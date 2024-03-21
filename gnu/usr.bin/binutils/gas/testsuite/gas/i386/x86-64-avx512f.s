# Check 64bit AVX512F instructions

	.allow_index_reg
	.text
_start:

	vaddpd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vaddpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vaddpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vaddpd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vaddpd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vaddpd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vaddpd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vaddpd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vaddpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vaddpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vaddpd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vaddpd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vaddpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vaddpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vaddpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vaddpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vaddpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vaddpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vaddps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vaddps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vaddps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vaddps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vaddps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vaddps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vaddps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vaddps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vaddps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vaddps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vaddps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vaddps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vaddps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vaddps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vaddps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vaddps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vaddps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vaddps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vaddsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vaddsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vaddsd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vaddsd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vaddsd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vaddsd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vaddsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vaddsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vaddsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vaddsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vaddsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vaddsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vaddss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vaddss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vaddss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vaddss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vaddss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vaddss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vaddss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vaddss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vaddss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vaddss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vaddss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vaddss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	valignd	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512F
	valignd	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	valignd	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	valignd	$123, %zmm28, %zmm29, %zmm30	 # AVX512F
	valignd	$123, (%rcx), %zmm29, %zmm30	 # AVX512F
	valignd	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	valignd	$123, (%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	valignd	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	valignd	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512F
	valignd	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	valignd	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512F
	valignd	$123, 508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	valignd	$123, 512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	valignd	$123, -512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	valignd	$123, -516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vblendmpd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vblendmpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vblendmpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vblendmpd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vblendmpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vblendmpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vblendmpd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vblendmpd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vblendmpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vblendmpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vblendmpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vblendmpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vblendmpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vblendmpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vblendmps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vblendmps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vblendmps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vblendmps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vblendmps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vblendmps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vblendmps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vblendmps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vblendmps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vblendmps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vblendmps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vblendmps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vblendmps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vblendmps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vbroadcastf32x4	(%rcx), %zmm30	 # AVX512F
	vbroadcastf32x4	(%rcx), %zmm30{%k7}	 # AVX512F
	vbroadcastf32x4	(%rcx), %zmm30{%k7}{z}	 # AVX512F
	vbroadcastf32x4	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vbroadcastf32x4	2032(%rdx), %zmm30	 # AVX512F Disp8
	vbroadcastf32x4	2048(%rdx), %zmm30	 # AVX512F
	vbroadcastf32x4	-2048(%rdx), %zmm30	 # AVX512F Disp8
	vbroadcastf32x4	-2064(%rdx), %zmm30	 # AVX512F

	vbroadcastf64x4	(%rcx), %zmm30	 # AVX512F
	vbroadcastf64x4	(%rcx), %zmm30{%k7}	 # AVX512F
	vbroadcastf64x4	(%rcx), %zmm30{%k7}{z}	 # AVX512F
	vbroadcastf64x4	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vbroadcastf64x4	4064(%rdx), %zmm30	 # AVX512F Disp8
	vbroadcastf64x4	4096(%rdx), %zmm30	 # AVX512F
	vbroadcastf64x4	-4096(%rdx), %zmm30	 # AVX512F Disp8
	vbroadcastf64x4	-4128(%rdx), %zmm30	 # AVX512F

	vbroadcasti32x4	(%rcx), %zmm30	 # AVX512F
	vbroadcasti32x4	(%rcx), %zmm30{%k7}	 # AVX512F
	vbroadcasti32x4	(%rcx), %zmm30{%k7}{z}	 # AVX512F
	vbroadcasti32x4	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vbroadcasti32x4	2032(%rdx), %zmm30	 # AVX512F Disp8
	vbroadcasti32x4	2048(%rdx), %zmm30	 # AVX512F
	vbroadcasti32x4	-2048(%rdx), %zmm30	 # AVX512F Disp8
	vbroadcasti32x4	-2064(%rdx), %zmm30	 # AVX512F

	vbroadcasti64x4	(%rcx), %zmm30	 # AVX512F
	vbroadcasti64x4	(%rcx), %zmm30{%k7}	 # AVX512F
	vbroadcasti64x4	(%rcx), %zmm30{%k7}{z}	 # AVX512F
	vbroadcasti64x4	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vbroadcasti64x4	4064(%rdx), %zmm30	 # AVX512F Disp8
	vbroadcasti64x4	4096(%rdx), %zmm30	 # AVX512F
	vbroadcasti64x4	-4096(%rdx), %zmm30	 # AVX512F Disp8
	vbroadcasti64x4	-4128(%rdx), %zmm30	 # AVX512F

	vbroadcastsd	(%rcx), %zmm30	 # AVX512F
	vbroadcastsd	(%rcx), %zmm30{%k7}	 # AVX512F
	vbroadcastsd	(%rcx), %zmm30{%k7}{z}	 # AVX512F
	vbroadcastsd	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vbroadcastsd	1016(%rdx), %zmm30	 # AVX512F Disp8
	vbroadcastsd	1024(%rdx), %zmm30	 # AVX512F
	vbroadcastsd	-1024(%rdx), %zmm30	 # AVX512F Disp8
	vbroadcastsd	-1032(%rdx), %zmm30	 # AVX512F

	vbroadcastsd	%xmm29, %zmm30{%k7}	 # AVX512F
	vbroadcastsd	%xmm29, %zmm30{%k7}{z}	 # AVX512F

	vbroadcastss	(%rcx), %zmm30	 # AVX512F
	vbroadcastss	(%rcx), %zmm30{%k7}	 # AVX512F
	vbroadcastss	(%rcx), %zmm30{%k7}{z}	 # AVX512F
	vbroadcastss	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vbroadcastss	508(%rdx), %zmm30	 # AVX512F Disp8
	vbroadcastss	512(%rdx), %zmm30	 # AVX512F
	vbroadcastss	-512(%rdx), %zmm30	 # AVX512F Disp8
	vbroadcastss	-516(%rdx), %zmm30	 # AVX512F

	vbroadcastss	%xmm29, %zmm30{%k7}	 # AVX512F
	vbroadcastss	%xmm29, %zmm30{%k7}{z}	 # AVX512F

	vcmppd	$0xab, %zmm29, %zmm30, %k5	 # AVX512F
	vcmppd	$0xab, %zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmppd	$0xab, {sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmppd	$123, %zmm29, %zmm30, %k5	 # AVX512F
	vcmppd	$123, {sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmppd	$123, (%rcx), %zmm30, %k5	 # AVX512F
	vcmppd	$123, 0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmppd	$123, (%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmppd	$123, 8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmppd	$123, 8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmppd	$123, -8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmppd	$123, -8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmppd	$123, 1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmppd	$123, 1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmppd	$123, -1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmppd	$123, -1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpeq_oqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_oqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpeq_oqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_oqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpeq_oqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpeq_oqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpeq_oqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_oqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_oqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_oqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_oqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_oqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpeq_oqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_oqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpeqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpeqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpeqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpeqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpeqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpeqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpeqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmplt_ospd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmplt_ospd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmplt_ospd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmplt_ospd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmplt_ospd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmplt_ospd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmplt_ospd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmplt_ospd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmplt_ospd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmplt_ospd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmplt_ospd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmplt_ospd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmplt_ospd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmplt_ospd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpltpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpltpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpltpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpltpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpltpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpltpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpltpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpltpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpltpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpltpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpltpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpltpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpltpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpltpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmple_ospd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmple_ospd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmple_ospd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmple_ospd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmple_ospd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmple_ospd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmple_ospd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmple_ospd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmple_ospd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmple_ospd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmple_ospd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmple_ospd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmple_ospd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmple_ospd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmplepd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmplepd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmplepd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmplepd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmplepd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmplepd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmplepd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmplepd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmplepd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmplepd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmplepd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmplepd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmplepd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmplepd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpunord_qpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpunord_qpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpunord_qpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpunord_qpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpunord_qpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpunord_qpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpunord_qpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_qpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpunord_qpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_qpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpunord_qpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_qpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpunord_qpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_qpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpunordpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpunordpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpunordpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpunordpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpunordpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpunordpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpunordpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpunordpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpunordpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpunordpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpunordpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpunordpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpunordpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpunordpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpneq_uqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_uqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpneq_uqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_uqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpneq_uqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpneq_uqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpneq_uqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_uqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_uqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_uqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_uqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_uqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpneq_uqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_uqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpneqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpneqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpneqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpneqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpneqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpneqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpneqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpnlt_uspd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnlt_uspd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnlt_uspd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnlt_uspd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnlt_uspd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnlt_uspd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnlt_uspd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_uspd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnlt_uspd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_uspd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnlt_uspd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_uspd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnlt_uspd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_uspd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpnltpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnltpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnltpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnltpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnltpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnltpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnltpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnltpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnltpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnltpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnltpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnltpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnltpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnltpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpnle_uspd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnle_uspd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnle_uspd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnle_uspd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnle_uspd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnle_uspd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnle_uspd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_uspd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnle_uspd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_uspd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnle_uspd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_uspd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnle_uspd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_uspd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpnlepd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnlepd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnlepd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnlepd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnlepd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnlepd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnlepd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnlepd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnlepd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnlepd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnlepd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnlepd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnlepd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnlepd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpord_qpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpord_qpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpord_qpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpord_qpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpord_qpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpord_qpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpord_qpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpord_qpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpord_qpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpord_qpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpord_qpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpord_qpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpord_qpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpord_qpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpordpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpordpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpordpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpordpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpordpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpordpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpordpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpordpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpordpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpordpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpordpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpordpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpordpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpordpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpeq_uqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_uqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpeq_uqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_uqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpeq_uqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpeq_uqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpeq_uqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_uqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_uqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_uqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_uqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_uqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpeq_uqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_uqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpnge_uspd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnge_uspd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnge_uspd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnge_uspd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnge_uspd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnge_uspd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnge_uspd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_uspd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnge_uspd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_uspd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnge_uspd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_uspd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnge_uspd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_uspd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpngepd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpngepd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpngepd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngepd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpngepd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpngepd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpngepd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngepd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngepd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngepd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngepd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngepd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpngepd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngepd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpngt_uspd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpngt_uspd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpngt_uspd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngt_uspd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpngt_uspd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpngt_uspd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpngt_uspd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_uspd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngt_uspd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_uspd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngt_uspd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_uspd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpngt_uspd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_uspd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpngtpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpngtpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpngtpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngtpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpngtpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpngtpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpngtpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngtpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngtpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngtpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngtpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngtpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpngtpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngtpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpfalse_oqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalse_oqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpfalse_oqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalse_oqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpfalse_oqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpfalse_oqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpfalse_oqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_oqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpfalse_oqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_oqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpfalse_oqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_oqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpfalse_oqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_oqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpfalsepd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalsepd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpfalsepd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalsepd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpfalsepd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpfalsepd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpfalsepd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpfalsepd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpfalsepd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpfalsepd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpfalsepd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpfalsepd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpfalsepd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpfalsepd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpneq_oqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_oqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpneq_oqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_oqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpneq_oqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpneq_oqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpneq_oqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_oqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_oqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_oqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_oqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_oqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpneq_oqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_oqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpge_ospd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpge_ospd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpge_ospd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpge_ospd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpge_ospd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpge_ospd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpge_ospd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpge_ospd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpge_ospd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpge_ospd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpge_ospd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpge_ospd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpge_ospd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpge_ospd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpgepd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpgepd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpgepd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgepd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpgepd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpgepd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpgepd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgepd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgepd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgepd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgepd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgepd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpgepd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgepd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpgt_ospd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpgt_ospd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpgt_ospd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgt_ospd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpgt_ospd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpgt_ospd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpgt_ospd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_ospd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgt_ospd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_ospd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgt_ospd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_ospd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpgt_ospd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_ospd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpgtpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpgtpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpgtpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgtpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpgtpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpgtpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpgtpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgtpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgtpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgtpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgtpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgtpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpgtpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgtpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmptrue_uqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmptrue_uqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmptrue_uqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmptrue_uqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmptrue_uqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmptrue_uqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmptrue_uqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_uqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmptrue_uqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_uqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmptrue_uqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_uqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmptrue_uqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_uqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmptruepd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmptruepd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmptruepd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmptruepd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmptruepd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmptruepd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmptruepd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmptruepd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmptruepd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmptruepd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmptruepd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmptruepd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmptruepd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmptruepd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpeq_ospd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_ospd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpeq_ospd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_ospd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpeq_ospd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpeq_ospd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpeq_ospd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_ospd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_ospd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_ospd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_ospd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_ospd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpeq_ospd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_ospd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmplt_oqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmplt_oqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmplt_oqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmplt_oqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmplt_oqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmplt_oqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmplt_oqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmplt_oqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmplt_oqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmplt_oqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmplt_oqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmplt_oqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmplt_oqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmplt_oqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmple_oqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmple_oqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmple_oqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmple_oqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmple_oqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmple_oqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmple_oqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmple_oqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmple_oqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmple_oqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmple_oqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmple_oqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmple_oqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmple_oqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpunord_spd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpunord_spd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpunord_spd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpunord_spd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpunord_spd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpunord_spd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpunord_spd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_spd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpunord_spd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_spd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpunord_spd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_spd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpunord_spd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_spd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpneq_uspd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_uspd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpneq_uspd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_uspd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpneq_uspd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpneq_uspd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpneq_uspd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_uspd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_uspd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_uspd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_uspd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_uspd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpneq_uspd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_uspd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpnlt_uqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnlt_uqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnlt_uqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnlt_uqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnlt_uqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnlt_uqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnlt_uqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_uqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnlt_uqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_uqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnlt_uqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_uqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnlt_uqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_uqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpnle_uqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnle_uqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnle_uqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnle_uqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnle_uqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnle_uqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnle_uqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_uqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnle_uqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_uqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnle_uqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_uqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnle_uqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_uqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpord_spd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpord_spd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpord_spd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpord_spd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpord_spd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpord_spd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpord_spd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpord_spd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpord_spd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpord_spd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpord_spd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpord_spd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpord_spd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpord_spd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpeq_uspd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_uspd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpeq_uspd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_uspd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpeq_uspd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpeq_uspd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpeq_uspd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_uspd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_uspd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_uspd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_uspd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_uspd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpeq_uspd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_uspd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpnge_uqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnge_uqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnge_uqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnge_uqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnge_uqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnge_uqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnge_uqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_uqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnge_uqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_uqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnge_uqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_uqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpnge_uqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_uqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpngt_uqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpngt_uqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpngt_uqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngt_uqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpngt_uqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpngt_uqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpngt_uqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_uqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngt_uqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_uqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngt_uqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_uqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpngt_uqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_uqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpfalse_ospd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalse_ospd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpfalse_ospd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalse_ospd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpfalse_ospd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpfalse_ospd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpfalse_ospd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_ospd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpfalse_ospd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_ospd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpfalse_ospd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_ospd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpfalse_ospd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_ospd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpneq_ospd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_ospd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpneq_ospd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_ospd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpneq_ospd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpneq_ospd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpneq_ospd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_ospd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_ospd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_ospd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_ospd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_ospd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpneq_ospd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_ospd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpge_oqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpge_oqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpge_oqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpge_oqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpge_oqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpge_oqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpge_oqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpge_oqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpge_oqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpge_oqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpge_oqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpge_oqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpge_oqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpge_oqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpgt_oqpd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpgt_oqpd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpgt_oqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgt_oqpd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpgt_oqpd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpgt_oqpd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpgt_oqpd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_oqpd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgt_oqpd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_oqpd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgt_oqpd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_oqpd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmpgt_oqpd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_oqpd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmptrue_uspd	%zmm29, %zmm30, %k5	 # AVX512F
	vcmptrue_uspd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmptrue_uspd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmptrue_uspd	(%rcx), %zmm30, %k5	 # AVX512F
	vcmptrue_uspd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmptrue_uspd	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vcmptrue_uspd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_uspd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmptrue_uspd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_uspd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmptrue_uspd	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_uspd	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vcmptrue_uspd	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_uspd	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vcmpps	$0xab, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpps	$0xab, %zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpps	$0xab, {sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpps	$123, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpps	$123, {sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpps	$123, (%rcx), %zmm30, %k5	 # AVX512F
	vcmpps	$123, 0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpps	$123, (%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpps	$123, 8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpps	$123, 8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpps	$123, -8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpps	$123, -8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpps	$123, 508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpps	$123, 512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpps	$123, -512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpps	$123, -516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpeq_oqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_oqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpeq_oqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_oqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpeq_oqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpeq_oqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpeq_oqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_oqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_oqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_oqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_oqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_oqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpeq_oqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_oqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpeqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpeqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpeqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpeqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpeqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpeqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpeqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmplt_osps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmplt_osps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmplt_osps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmplt_osps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmplt_osps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmplt_osps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmplt_osps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmplt_osps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmplt_osps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmplt_osps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmplt_osps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmplt_osps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmplt_osps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmplt_osps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpltps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpltps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpltps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpltps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpltps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpltps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpltps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpltps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpltps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpltps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpltps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpltps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpltps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpltps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmple_osps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmple_osps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmple_osps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmple_osps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmple_osps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmple_osps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmple_osps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmple_osps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmple_osps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmple_osps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmple_osps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmple_osps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmple_osps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmple_osps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpleps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpleps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpleps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpleps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpleps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpleps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpleps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpleps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpleps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpleps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpleps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpleps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpleps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpleps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpunord_qps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpunord_qps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpunord_qps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpunord_qps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpunord_qps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpunord_qps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpunord_qps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_qps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpunord_qps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_qps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpunord_qps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_qps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpunord_qps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_qps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpunordps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpunordps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpunordps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpunordps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpunordps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpunordps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpunordps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpunordps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpunordps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpunordps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpunordps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpunordps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpunordps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpunordps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpneq_uqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_uqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpneq_uqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_uqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpneq_uqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpneq_uqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpneq_uqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_uqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_uqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_uqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_uqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_uqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpneq_uqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_uqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpneqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpneqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpneqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpneqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpneqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpneqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpneqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpnlt_usps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnlt_usps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnlt_usps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnlt_usps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnlt_usps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnlt_usps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnlt_usps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_usps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnlt_usps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_usps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnlt_usps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_usps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnlt_usps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_usps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpnltps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnltps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnltps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnltps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnltps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnltps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnltps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnltps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnltps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnltps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnltps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnltps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnltps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnltps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpnle_usps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnle_usps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnle_usps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnle_usps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnle_usps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnle_usps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnle_usps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_usps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnle_usps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_usps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnle_usps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_usps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnle_usps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_usps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpnleps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnleps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnleps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnleps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnleps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnleps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnleps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnleps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnleps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnleps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnleps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnleps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnleps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnleps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpord_qps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpord_qps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpord_qps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpord_qps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpord_qps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpord_qps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpord_qps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpord_qps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpord_qps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpord_qps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpord_qps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpord_qps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpord_qps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpord_qps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpordps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpordps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpordps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpordps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpordps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpordps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpordps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpordps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpordps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpordps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpordps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpordps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpordps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpordps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpeq_uqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_uqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpeq_uqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_uqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpeq_uqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpeq_uqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpeq_uqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_uqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_uqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_uqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_uqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_uqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpeq_uqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_uqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpnge_usps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnge_usps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnge_usps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnge_usps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnge_usps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnge_usps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnge_usps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_usps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnge_usps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_usps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnge_usps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_usps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnge_usps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_usps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpngeps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpngeps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpngeps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngeps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpngeps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpngeps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpngeps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngeps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngeps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngeps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngeps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngeps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpngeps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngeps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpngt_usps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpngt_usps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpngt_usps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngt_usps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpngt_usps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpngt_usps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpngt_usps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_usps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngt_usps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_usps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngt_usps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_usps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpngt_usps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_usps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpngtps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpngtps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpngtps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngtps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpngtps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpngtps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpngtps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngtps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngtps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngtps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngtps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngtps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpngtps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngtps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpfalse_oqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalse_oqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpfalse_oqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalse_oqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpfalse_oqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpfalse_oqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpfalse_oqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_oqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpfalse_oqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_oqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpfalse_oqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_oqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpfalse_oqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_oqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpfalseps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalseps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpfalseps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalseps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpfalseps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpfalseps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpfalseps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpfalseps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpfalseps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpfalseps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpfalseps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpfalseps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpfalseps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpfalseps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpneq_oqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_oqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpneq_oqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_oqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpneq_oqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpneq_oqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpneq_oqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_oqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_oqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_oqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_oqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_oqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpneq_oqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_oqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpge_osps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpge_osps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpge_osps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpge_osps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpge_osps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpge_osps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpge_osps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpge_osps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpge_osps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpge_osps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpge_osps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpge_osps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpge_osps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpge_osps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpgeps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpgeps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpgeps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgeps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpgeps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpgeps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpgeps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgeps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgeps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgeps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgeps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgeps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpgeps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgeps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpgt_osps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpgt_osps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpgt_osps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgt_osps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpgt_osps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpgt_osps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpgt_osps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_osps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgt_osps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_osps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgt_osps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_osps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpgt_osps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_osps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpgtps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpgtps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpgtps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgtps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpgtps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpgtps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpgtps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgtps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgtps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgtps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgtps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgtps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpgtps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgtps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmptrue_uqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmptrue_uqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmptrue_uqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmptrue_uqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmptrue_uqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmptrue_uqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmptrue_uqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_uqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmptrue_uqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_uqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmptrue_uqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_uqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmptrue_uqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_uqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmptrueps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmptrueps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmptrueps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmptrueps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmptrueps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmptrueps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmptrueps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmptrueps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmptrueps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmptrueps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmptrueps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmptrueps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmptrueps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmptrueps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpeq_osps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_osps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpeq_osps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_osps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpeq_osps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpeq_osps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpeq_osps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_osps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_osps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_osps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_osps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_osps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpeq_osps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_osps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmplt_oqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmplt_oqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmplt_oqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmplt_oqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmplt_oqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmplt_oqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmplt_oqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmplt_oqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmplt_oqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmplt_oqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmplt_oqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmplt_oqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmplt_oqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmplt_oqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmple_oqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmple_oqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmple_oqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmple_oqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmple_oqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmple_oqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmple_oqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmple_oqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmple_oqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmple_oqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmple_oqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmple_oqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmple_oqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmple_oqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpunord_sps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpunord_sps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpunord_sps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpunord_sps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpunord_sps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpunord_sps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpunord_sps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_sps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpunord_sps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_sps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpunord_sps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_sps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpunord_sps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpunord_sps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpneq_usps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_usps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpneq_usps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_usps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpneq_usps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpneq_usps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpneq_usps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_usps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_usps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_usps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_usps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_usps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpneq_usps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_usps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpnlt_uqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnlt_uqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnlt_uqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnlt_uqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnlt_uqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnlt_uqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnlt_uqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_uqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnlt_uqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_uqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnlt_uqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_uqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnlt_uqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnlt_uqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpnle_uqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnle_uqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnle_uqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnle_uqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnle_uqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnle_uqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnle_uqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_uqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnle_uqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_uqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnle_uqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_uqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnle_uqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnle_uqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpord_sps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpord_sps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpord_sps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpord_sps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpord_sps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpord_sps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpord_sps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpord_sps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpord_sps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpord_sps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpord_sps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpord_sps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpord_sps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpord_sps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpeq_usps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_usps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpeq_usps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_usps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpeq_usps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpeq_usps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpeq_usps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_usps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_usps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_usps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpeq_usps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_usps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpeq_usps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpeq_usps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpnge_uqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpnge_uqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpnge_uqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnge_uqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpnge_uqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpnge_uqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnge_uqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_uqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnge_uqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_uqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpnge_uqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_uqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpnge_uqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpnge_uqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpngt_uqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpngt_uqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpngt_uqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngt_uqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpngt_uqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpngt_uqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpngt_uqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_uqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngt_uqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_uqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpngt_uqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_uqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpngt_uqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpngt_uqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpfalse_osps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalse_osps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpfalse_osps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalse_osps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpfalse_osps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpfalse_osps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpfalse_osps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_osps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpfalse_osps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_osps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpfalse_osps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_osps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpfalse_osps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpfalse_osps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpneq_osps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_osps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpneq_osps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_osps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpneq_osps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpneq_osps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpneq_osps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_osps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_osps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_osps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpneq_osps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_osps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpneq_osps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpneq_osps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpge_oqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpge_oqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpge_oqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpge_oqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpge_oqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpge_oqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpge_oqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpge_oqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpge_oqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpge_oqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpge_oqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpge_oqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpge_oqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpge_oqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpgt_oqps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmpgt_oqps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmpgt_oqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgt_oqps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmpgt_oqps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmpgt_oqps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpgt_oqps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_oqps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgt_oqps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_oqps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmpgt_oqps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_oqps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmpgt_oqps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmpgt_oqps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmptrue_usps	%zmm29, %zmm30, %k5	 # AVX512F
	vcmptrue_usps	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vcmptrue_usps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmptrue_usps	(%rcx), %zmm30, %k5	 # AVX512F
	vcmptrue_usps	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vcmptrue_usps	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vcmptrue_usps	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_usps	8192(%rdx), %zmm30, %k5	 # AVX512F
	vcmptrue_usps	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_usps	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vcmptrue_usps	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_usps	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vcmptrue_usps	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vcmptrue_usps	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vcmpsd	$0xab, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpsd	$0xab, {sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpsd	$123, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpsd	$123, {sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpsd	$123, (%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpsd	$123, 0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpsd	$123, 1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpsd	$123, 1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpsd	$123, -1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpsd	$123, -1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpeq_oqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_oqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_oqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_oqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_oqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_oqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_oqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_oqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpeqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmplt_ossd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_ossd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_ossd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_ossd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_ossd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmplt_ossd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_ossd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmplt_ossd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpltsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpltsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpltsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpltsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpltsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpltsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpltsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpltsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmple_ossd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmple_ossd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmple_ossd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmple_ossd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmple_ossd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmple_ossd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmple_ossd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmple_ossd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmplesd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmplesd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmplesd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmplesd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmplesd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmplesd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmplesd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmplesd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpunord_qsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_qsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_qsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_qsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_qsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_qsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_qsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_qsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpunordsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpunordsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpunordsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunordsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunordsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpunordsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunordsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpunordsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpneq_uqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_uqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_uqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_uqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_uqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_uqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_uqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_uqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpneqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnlt_ussd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_ussd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_ussd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_ussd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_ussd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_ussd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_ussd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_ussd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnltsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnltsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnltsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnltsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnltsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnltsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnltsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnltsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnle_ussd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_ussd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_ussd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_ussd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_ussd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_ussd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_ussd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_ussd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnlesd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlesd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlesd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlesd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlesd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnlesd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlesd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnlesd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpord_qsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_qsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_qsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_qsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_qsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpord_qsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_qsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpord_qsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpordsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpordsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpordsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpordsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpordsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpordsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpordsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpordsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpeq_uqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_uqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_uqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_uqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_uqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_uqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_uqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_uqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnge_ussd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_ussd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_ussd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_ussd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_ussd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_ussd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_ussd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_ussd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpngesd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngesd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngesd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngesd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngesd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngesd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngesd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngesd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpngt_ussd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_ussd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_ussd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_ussd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_ussd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_ussd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_ussd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_ussd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpngtsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngtsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngtsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngtsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngtsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngtsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngtsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngtsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpfalse_oqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_oqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_oqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_oqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_oqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_oqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_oqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_oqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpfalsesd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalsesd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalsesd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalsesd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalsesd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpfalsesd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalsesd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpfalsesd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpneq_oqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_oqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_oqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_oqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_oqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_oqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_oqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_oqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpge_ossd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_ossd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_ossd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_ossd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_ossd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpge_ossd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_ossd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpge_ossd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpgesd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgesd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgesd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgesd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgesd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgesd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgesd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgesd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpgt_ossd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_ossd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_ossd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_ossd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_ossd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_ossd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_ossd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_ossd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpgtsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgtsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgtsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgtsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgtsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgtsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgtsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgtsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmptrue_uqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_uqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_uqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_uqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_uqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_uqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_uqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_uqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmptruesd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmptruesd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmptruesd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmptruesd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmptruesd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmptruesd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmptruesd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmptruesd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpeq_ossd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_ossd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_ossd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_ossd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_ossd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_ossd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_ossd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_ossd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmplt_oqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_oqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_oqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_oqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_oqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmplt_oqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_oqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmplt_oqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmple_oqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmple_oqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmple_oqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmple_oqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmple_oqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmple_oqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmple_oqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmple_oqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpunord_ssd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_ssd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_ssd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_ssd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_ssd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_ssd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_ssd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_ssd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpneq_ussd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_ussd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_ussd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_ussd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_ussd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_ussd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_ussd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_ussd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnlt_uqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_uqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_uqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_uqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_uqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_uqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_uqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_uqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnle_uqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_uqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_uqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_uqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_uqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_uqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_uqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_uqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpord_ssd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_ssd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_ssd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_ssd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_ssd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpord_ssd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_ssd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpord_ssd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpeq_ussd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_ussd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_ussd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_ussd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_ussd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_ussd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_ussd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_ussd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnge_uqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_uqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_uqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_uqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_uqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_uqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_uqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_uqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpngt_uqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_uqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_uqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_uqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_uqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_uqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_uqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_uqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpfalse_ossd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_ossd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_ossd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_ossd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_ossd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_ossd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_ossd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_ossd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpneq_ossd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_ossd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_ossd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_ossd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_ossd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_ossd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_ossd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_ossd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpge_oqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_oqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_oqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_oqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_oqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpge_oqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_oqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpge_oqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpgt_oqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_oqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_oqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_oqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_oqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_oqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_oqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_oqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmptrue_ussd	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_ussd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_ussd	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_ussd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_ussd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_ussd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_ussd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_ussd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpss	$0xab, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpss	$0xab, {sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpss	$123, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpss	$123, {sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpss	$123, (%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpss	$123, 0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpss	$123, 508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpss	$123, 512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpss	$123, -512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpss	$123, -516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpeq_oqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_oqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_oqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_oqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_oqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_oqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_oqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_oqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpeqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmplt_osss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_osss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_osss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_osss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_osss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmplt_osss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_osss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmplt_osss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpltss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpltss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpltss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpltss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpltss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpltss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpltss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpltss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmple_osss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmple_osss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmple_osss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmple_osss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmple_osss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmple_osss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmple_osss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmple_osss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpless	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpless	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpless	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpless	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpless	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpless	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpless	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpless	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpunord_qss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_qss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_qss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_qss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_qss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_qss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_qss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_qss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpunordss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpunordss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpunordss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunordss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunordss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpunordss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunordss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpunordss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpneq_uqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_uqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_uqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_uqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_uqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_uqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_uqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_uqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpneqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnlt_usss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_usss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_usss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_usss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_usss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_usss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_usss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_usss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnltss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnltss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnltss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnltss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnltss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnltss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnltss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnltss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnle_usss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_usss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_usss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_usss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_usss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_usss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_usss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_usss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnless	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnless	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnless	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnless	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnless	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnless	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnless	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnless	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpord_qss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_qss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_qss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_qss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_qss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpord_qss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_qss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpord_qss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpordss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpordss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpordss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpordss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpordss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpordss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpordss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpordss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpeq_uqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_uqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_uqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_uqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_uqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_uqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_uqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_uqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnge_usss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_usss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_usss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_usss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_usss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_usss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_usss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_usss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpngess	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngess	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngess	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngess	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngess	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngess	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngess	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngess	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpngt_usss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_usss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_usss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_usss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_usss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_usss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_usss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_usss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpngtss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngtss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngtss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngtss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngtss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngtss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngtss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngtss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpfalse_oqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_oqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_oqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_oqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_oqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_oqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_oqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_oqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpfalsess	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalsess	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalsess	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalsess	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalsess	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpfalsess	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalsess	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpfalsess	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpneq_oqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_oqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_oqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_oqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_oqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_oqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_oqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_oqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpge_osss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_osss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_osss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_osss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_osss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpge_osss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_osss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpge_osss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpgess	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgess	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgess	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgess	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgess	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgess	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgess	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgess	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpgt_osss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_osss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_osss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_osss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_osss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_osss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_osss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_osss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpgtss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgtss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgtss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgtss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgtss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgtss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgtss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgtss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmptrue_uqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_uqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_uqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_uqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_uqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_uqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_uqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_uqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmptruess	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmptruess	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmptruess	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmptruess	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmptruess	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmptruess	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmptruess	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmptruess	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpeq_osss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_osss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_osss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_osss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_osss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_osss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_osss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_osss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmplt_oqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_oqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_oqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_oqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_oqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmplt_oqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmplt_oqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmplt_oqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmple_oqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmple_oqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmple_oqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmple_oqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmple_oqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmple_oqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmple_oqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmple_oqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpunord_sss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_sss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_sss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_sss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_sss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_sss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpunord_sss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_sss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpneq_usss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_usss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_usss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_usss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_usss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_usss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_usss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_usss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnlt_uqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_uqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_uqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_uqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_uqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_uqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnlt_uqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_uqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnle_uqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_uqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_uqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_uqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_uqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_uqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnle_uqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_uqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpord_sss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_sss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_sss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_sss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_sss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpord_sss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpord_sss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpord_sss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpeq_usss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_usss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_usss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_usss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_usss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_usss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpeq_usss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_usss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpnge_uqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_uqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_uqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_uqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_uqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_uqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpnge_uqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_uqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpngt_uqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_uqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_uqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_uqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_uqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_uqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpngt_uqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_uqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpfalse_osss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_osss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_osss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_osss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_osss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_osss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpfalse_osss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_osss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpneq_osss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_osss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_osss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_osss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_osss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_osss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpneq_osss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_osss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpge_oqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_oqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_oqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_oqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_oqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpge_oqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpge_oqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpge_oqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmpgt_oqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_oqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_oqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_oqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_oqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_oqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmpgt_oqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_oqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcmptrue_usss	%xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_usss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_usss	(%rcx), %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_usss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_usss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_usss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512F
	vcmptrue_usss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_usss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512F

	vcomisd	%xmm29, %xmm30	 # AVX512F
	vcomisd	{sae}, %xmm29, %xmm30	 # AVX512F
	vcomisd	(%rcx), %xmm30	 # AVX512F
	vcomisd	0x123(%rax,%r14,8), %xmm30	 # AVX512F
	vcomisd	1016(%rdx), %xmm30	 # AVX512F Disp8
	vcomisd	1024(%rdx), %xmm30	 # AVX512F
	vcomisd	-1024(%rdx), %xmm30	 # AVX512F Disp8
	vcomisd	-1032(%rdx), %xmm30	 # AVX512F

	vcomiss	%xmm29, %xmm30	 # AVX512F
	vcomiss	{sae}, %xmm29, %xmm30	 # AVX512F
	vcomiss	(%rcx), %xmm30	 # AVX512F
	vcomiss	0x123(%rax,%r14,8), %xmm30	 # AVX512F
	vcomiss	508(%rdx), %xmm30	 # AVX512F Disp8
	vcomiss	512(%rdx), %xmm30	 # AVX512F
	vcomiss	-512(%rdx), %xmm30	 # AVX512F Disp8
	vcomiss	-516(%rdx), %xmm30	 # AVX512F

	vcompresspd	%zmm30, (%rcx)	 # AVX512F
	vcompresspd	%zmm30, (%rcx){%k7}	 # AVX512F
	vcompresspd	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vcompresspd	%zmm30, 1016(%rdx)	 # AVX512F Disp8
	vcompresspd	%zmm30, 1024(%rdx)	 # AVX512F
	vcompresspd	%zmm30, -1024(%rdx)	 # AVX512F Disp8
	vcompresspd	%zmm30, -1032(%rdx)	 # AVX512F

	vcompresspd	%zmm29, %zmm30	 # AVX512F
	vcompresspd	%zmm29, %zmm30{%k7}	 # AVX512F
	vcompresspd	%zmm29, %zmm30{%k7}{z}	 # AVX512F

	vcompressps	%zmm30, (%rcx)	 # AVX512F
	vcompressps	%zmm30, (%rcx){%k7}	 # AVX512F
	vcompressps	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vcompressps	%zmm30, 508(%rdx)	 # AVX512F Disp8
	vcompressps	%zmm30, 512(%rdx)	 # AVX512F
	vcompressps	%zmm30, -512(%rdx)	 # AVX512F Disp8
	vcompressps	%zmm30, -516(%rdx)	 # AVX512F

	vcompressps	%zmm29, %zmm30	 # AVX512F
	vcompressps	%zmm29, %zmm30{%k7}	 # AVX512F
	vcompressps	%zmm29, %zmm30{%k7}{z}	 # AVX512F

	vcvtdq2pd	%ymm29, %zmm30{%k7}	 # AVX512F
	vcvtdq2pd	%ymm29, %zmm30{%k7}{z}	 # AVX512F
	vcvtdq2pd	(%rcx), %zmm30{%k7}	 # AVX512F
	vcvtdq2pd	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512F
	vcvtdq2pd	(%rcx){1to8}, %zmm30{%k7}	 # AVX512F
	vcvtdq2pd	4064(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vcvtdq2pd	4096(%rdx), %zmm30{%k7}	 # AVX512F
	vcvtdq2pd	-4096(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vcvtdq2pd	-4128(%rdx), %zmm30{%k7}	 # AVX512F
	vcvtdq2pd	508(%rdx){1to8}, %zmm30{%k7}	 # AVX512F Disp8
	vcvtdq2pd	512(%rdx){1to8}, %zmm30{%k7}	 # AVX512F
	vcvtdq2pd	-512(%rdx){1to8}, %zmm30{%k7}	 # AVX512F Disp8
	vcvtdq2pd	-516(%rdx){1to8}, %zmm30{%k7}	 # AVX512F

	vcvtdq2ps	%zmm29, %zmm30	 # AVX512F
	vcvtdq2ps	%zmm29, %zmm30{%k7}	 # AVX512F
	vcvtdq2ps	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vcvtdq2ps	{rn-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtdq2ps	{ru-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtdq2ps	{rd-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtdq2ps	{rz-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtdq2ps	(%rcx), %zmm30	 # AVX512F
	vcvtdq2ps	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vcvtdq2ps	(%rcx){1to16}, %zmm30	 # AVX512F
	vcvtdq2ps	8128(%rdx), %zmm30	 # AVX512F Disp8
	vcvtdq2ps	8192(%rdx), %zmm30	 # AVX512F
	vcvtdq2ps	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vcvtdq2ps	-8256(%rdx), %zmm30	 # AVX512F
	vcvtdq2ps	508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vcvtdq2ps	512(%rdx){1to16}, %zmm30	 # AVX512F
	vcvtdq2ps	-512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vcvtdq2ps	-516(%rdx){1to16}, %zmm30	 # AVX512F

	vcvtpd2dq	%zmm29, %ymm30{%k7}	 # AVX512F
	vcvtpd2dq	%zmm29, %ymm30{%k7}{z}	 # AVX512F
	vcvtpd2dq	{rn-sae}, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvtpd2dq	{ru-sae}, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvtpd2dq	{rd-sae}, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvtpd2dq	{rz-sae}, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvtpd2dq	(%rcx), %ymm30{%k7}	 # AVX512F
	vcvtpd2dq	0x123(%rax,%r14,8), %ymm30{%k7}	 # AVX512F
	vcvtpd2dq	(%rcx){1to8}, %ymm30{%k7}	 # AVX512F
	vcvtpd2dq	8128(%rdx), %ymm30{%k7}	 # AVX512F Disp8
	vcvtpd2dq	8192(%rdx), %ymm30{%k7}	 # AVX512F
	vcvtpd2dq	-8192(%rdx), %ymm30{%k7}	 # AVX512F Disp8
	vcvtpd2dq	-8256(%rdx), %ymm30{%k7}	 # AVX512F
	vcvtpd2dq	1016(%rdx){1to8}, %ymm30{%k7}	 # AVX512F Disp8
	vcvtpd2dq	1024(%rdx){1to8}, %ymm30{%k7}	 # AVX512F
	vcvtpd2dq	-1024(%rdx){1to8}, %ymm30{%k7}	 # AVX512F Disp8
	vcvtpd2dq	-1032(%rdx){1to8}, %ymm30{%k7}	 # AVX512F

	vcvtpd2ps	%zmm29, %ymm30{%k7}	 # AVX512F
	vcvtpd2ps	%zmm29, %ymm30{%k7}{z}	 # AVX512F
	vcvtpd2ps	{rn-sae}, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvtpd2ps	{ru-sae}, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvtpd2ps	{rd-sae}, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvtpd2ps	{rz-sae}, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvtpd2ps	(%rcx), %ymm30{%k7}	 # AVX512F
	vcvtpd2ps	0x123(%rax,%r14,8), %ymm30{%k7}	 # AVX512F
	vcvtpd2ps	(%rcx){1to8}, %ymm30{%k7}	 # AVX512F
	vcvtpd2ps	8128(%rdx), %ymm30{%k7}	 # AVX512F Disp8
	vcvtpd2ps	8192(%rdx), %ymm30{%k7}	 # AVX512F
	vcvtpd2ps	-8192(%rdx), %ymm30{%k7}	 # AVX512F Disp8
	vcvtpd2ps	-8256(%rdx), %ymm30{%k7}	 # AVX512F
	vcvtpd2ps	1016(%rdx){1to8}, %ymm30{%k7}	 # AVX512F Disp8
	vcvtpd2ps	1024(%rdx){1to8}, %ymm30{%k7}	 # AVX512F
	vcvtpd2ps	-1024(%rdx){1to8}, %ymm30{%k7}	 # AVX512F Disp8
	vcvtpd2ps	-1032(%rdx){1to8}, %ymm30{%k7}	 # AVX512F

	vcvtpd2udq	%zmm29, %ymm30{%k7}	 # AVX512F
	vcvtpd2udq	%zmm29, %ymm30{%k7}{z}	 # AVX512F
	vcvtpd2udq	{rn-sae}, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvtpd2udq	{ru-sae}, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvtpd2udq	{rd-sae}, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvtpd2udq	{rz-sae}, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvtpd2udq	(%rcx), %ymm30{%k7}	 # AVX512F
	vcvtpd2udq	0x123(%rax,%r14,8), %ymm30{%k7}	 # AVX512F
	vcvtpd2udq	(%rcx){1to8}, %ymm30{%k7}	 # AVX512F
	vcvtpd2udq	8128(%rdx), %ymm30{%k7}	 # AVX512F Disp8
	vcvtpd2udq	8192(%rdx), %ymm30{%k7}	 # AVX512F
	vcvtpd2udq	-8192(%rdx), %ymm30{%k7}	 # AVX512F Disp8
	vcvtpd2udq	-8256(%rdx), %ymm30{%k7}	 # AVX512F
	vcvtpd2udq	1016(%rdx){1to8}, %ymm30{%k7}	 # AVX512F Disp8
	vcvtpd2udq	1024(%rdx){1to8}, %ymm30{%k7}	 # AVX512F
	vcvtpd2udq	-1024(%rdx){1to8}, %ymm30{%k7}	 # AVX512F Disp8
	vcvtpd2udq	-1032(%rdx){1to8}, %ymm30{%k7}	 # AVX512F

	vcvtph2ps	%ymm29, %zmm30{%k7}	 # AVX512F
	vcvtph2ps	%ymm29, %zmm30{%k7}{z}	 # AVX512F
	vcvtph2ps	{sae}, %ymm29, %zmm30{%k7}	 # AVX512F
	vcvtph2ps	(%rcx), %zmm30{%k7}	 # AVX512F
	vcvtph2ps	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512F
	vcvtph2ps	4064(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vcvtph2ps	4096(%rdx), %zmm30{%k7}	 # AVX512F
	vcvtph2ps	-4096(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vcvtph2ps	-4128(%rdx), %zmm30{%k7}	 # AVX512F

	vcvtps2dq	%zmm29, %zmm30	 # AVX512F
	vcvtps2dq	%zmm29, %zmm30{%k7}	 # AVX512F
	vcvtps2dq	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vcvtps2dq	{rn-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtps2dq	{ru-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtps2dq	{rd-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtps2dq	{rz-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtps2dq	(%rcx), %zmm30	 # AVX512F
	vcvtps2dq	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vcvtps2dq	(%rcx){1to16}, %zmm30	 # AVX512F
	vcvtps2dq	8128(%rdx), %zmm30	 # AVX512F Disp8
	vcvtps2dq	8192(%rdx), %zmm30	 # AVX512F
	vcvtps2dq	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vcvtps2dq	-8256(%rdx), %zmm30	 # AVX512F
	vcvtps2dq	508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vcvtps2dq	512(%rdx){1to16}, %zmm30	 # AVX512F
	vcvtps2dq	-512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vcvtps2dq	-516(%rdx){1to16}, %zmm30	 # AVX512F

	vcvtps2pd	%ymm29, %zmm30{%k7}	 # AVX512F
	vcvtps2pd	%ymm29, %zmm30{%k7}{z}	 # AVX512F
	vcvtps2pd	{sae}, %ymm29, %zmm30{%k7}	 # AVX512F
	vcvtps2pd	(%rcx), %zmm30{%k7}	 # AVX512F
	vcvtps2pd	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512F
	vcvtps2pd	(%rcx){1to8}, %zmm30{%k7}	 # AVX512F
	vcvtps2pd	4064(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vcvtps2pd	4096(%rdx), %zmm30{%k7}	 # AVX512F
	vcvtps2pd	-4096(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vcvtps2pd	-4128(%rdx), %zmm30{%k7}	 # AVX512F
	vcvtps2pd	508(%rdx){1to8}, %zmm30{%k7}	 # AVX512F Disp8
	vcvtps2pd	512(%rdx){1to8}, %zmm30{%k7}	 # AVX512F
	vcvtps2pd	-512(%rdx){1to8}, %zmm30{%k7}	 # AVX512F Disp8
	vcvtps2pd	-516(%rdx){1to8}, %zmm30{%k7}	 # AVX512F

	vcvtps2ph	$0xab, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvtps2ph	$0xab, %zmm29, %ymm30{%k7}{z}	 # AVX512F
	vcvtps2ph	$0xab, {sae}, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvtps2ph	$123, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvtps2ph	$123, {sae}, %zmm29, %ymm30{%k7}	 # AVX512F

	vcvtps2udq	%zmm29, %zmm30	 # AVX512F
	vcvtps2udq	%zmm29, %zmm30{%k7}	 # AVX512F
	vcvtps2udq	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vcvtps2udq	{rn-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtps2udq	{ru-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtps2udq	{rd-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtps2udq	{rz-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtps2udq	(%rcx), %zmm30	 # AVX512F
	vcvtps2udq	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vcvtps2udq	(%rcx){1to16}, %zmm30	 # AVX512F
	vcvtps2udq	8128(%rdx), %zmm30	 # AVX512F Disp8
	vcvtps2udq	8192(%rdx), %zmm30	 # AVX512F
	vcvtps2udq	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vcvtps2udq	-8256(%rdx), %zmm30	 # AVX512F
	vcvtps2udq	508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vcvtps2udq	512(%rdx){1to16}, %zmm30	 # AVX512F
	vcvtps2udq	-512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vcvtps2udq	-516(%rdx){1to16}, %zmm30	 # AVX512F

	vcvtsd2si	{rn-sae}, %xmm30, %eax	 # AVX512F
	vcvtsd2si	{ru-sae}, %xmm30, %eax	 # AVX512F
	vcvtsd2si	{rd-sae}, %xmm30, %eax	 # AVX512F
	vcvtsd2si	{rz-sae}, %xmm30, %eax	 # AVX512F
	vcvtsd2si	{rn-sae}, %xmm30, %ebp	 # AVX512F
	vcvtsd2si	{ru-sae}, %xmm30, %ebp	 # AVX512F
	vcvtsd2si	{rd-sae}, %xmm30, %ebp	 # AVX512F
	vcvtsd2si	{rz-sae}, %xmm30, %ebp	 # AVX512F
	vcvtsd2si	{rn-sae}, %xmm30, %r13d	 # AVX512F
	vcvtsd2si	{ru-sae}, %xmm30, %r13d	 # AVX512F
	vcvtsd2si	{rd-sae}, %xmm30, %r13d	 # AVX512F
	vcvtsd2si	{rz-sae}, %xmm30, %r13d	 # AVX512F

	vcvtsd2si	{rn-sae}, %xmm30, %rax	 # AVX512F
	vcvtsd2si	{ru-sae}, %xmm30, %rax	 # AVX512F
	vcvtsd2si	{rd-sae}, %xmm30, %rax	 # AVX512F
	vcvtsd2si	{rz-sae}, %xmm30, %rax	 # AVX512F
	vcvtsd2si	{rn-sae}, %xmm30, %r8	 # AVX512F
	vcvtsd2si	{ru-sae}, %xmm30, %r8	 # AVX512F
	vcvtsd2si	{rd-sae}, %xmm30, %r8	 # AVX512F
	vcvtsd2si	{rz-sae}, %xmm30, %r8	 # AVX512F

	vcvtsd2ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vcvtsd2ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vcvtsd2ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vcvtsd2ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vcvtsd2ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vcvtsd2ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vcvtsd2ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vcvtsd2ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vcvtsd2ss	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vcvtsd2ss	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vcvtsd2ss	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vcvtsd2ss	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vcvtsi2sdl	%eax, %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdl	%ebp, %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdl	%r13d, %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdl	(%rcx), %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdl	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdl	508(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtsi2sdl	512(%rdx), %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdl	-512(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtsi2sdl	-516(%rdx), %xmm29, %xmm30	 # AVX512F

	vcvtsi2sdq	%rax, %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdq	%rax, {rn-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdq	%rax, {ru-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdq	%rax, {rd-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdq	%rax, {rz-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdq	%r8, %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdq	%r8, {rn-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdq	%r8, {ru-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdq	%r8, {rd-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdq	%r8, {rz-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdq	(%rcx), %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdq	1016(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtsi2sdq	1024(%rdx), %xmm29, %xmm30	 # AVX512F
	vcvtsi2sdq	-1024(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtsi2sdq	-1032(%rdx), %xmm29, %xmm30	 # AVX512F

	vcvtsi2ssl	%eax, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	%eax, {rn-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	%eax, {ru-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	%eax, {rd-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	%eax, {rz-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	%ebp, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	%ebp, {rn-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	%ebp, {ru-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	%ebp, {rd-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	%ebp, {rz-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	%r13d, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	%r13d, {rn-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	%r13d, {ru-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	%r13d, {rd-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	%r13d, {rz-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	(%rcx), %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	508(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtsi2ssl	512(%rdx), %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssl	-512(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtsi2ssl	-516(%rdx), %xmm29, %xmm30	 # AVX512F

	vcvtsi2ssq	%rax, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssq	%rax, {rn-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssq	%rax, {ru-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssq	%rax, {rd-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssq	%rax, {rz-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssq	%r8, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssq	%r8, {rn-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssq	%r8, {ru-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssq	%r8, {rd-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssq	%r8, {rz-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssq	(%rcx), %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssq	1016(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtsi2ssq	1024(%rdx), %xmm29, %xmm30	 # AVX512F
	vcvtsi2ssq	-1024(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtsi2ssq	-1032(%rdx), %xmm29, %xmm30	 # AVX512F

	vcvtss2sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vcvtss2sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vcvtss2sd	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vcvtss2sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vcvtss2sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vcvtss2sd	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vcvtss2sd	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vcvtss2sd	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vcvtss2sd	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vcvtss2si	{rn-sae}, %xmm30, %eax	 # AVX512F
	vcvtss2si	{ru-sae}, %xmm30, %eax	 # AVX512F
	vcvtss2si	{rd-sae}, %xmm30, %eax	 # AVX512F
	vcvtss2si	{rz-sae}, %xmm30, %eax	 # AVX512F
	vcvtss2si	{rn-sae}, %xmm30, %ebp	 # AVX512F
	vcvtss2si	{ru-sae}, %xmm30, %ebp	 # AVX512F
	vcvtss2si	{rd-sae}, %xmm30, %ebp	 # AVX512F
	vcvtss2si	{rz-sae}, %xmm30, %ebp	 # AVX512F
	vcvtss2si	{rn-sae}, %xmm30, %r13d	 # AVX512F
	vcvtss2si	{ru-sae}, %xmm30, %r13d	 # AVX512F
	vcvtss2si	{rd-sae}, %xmm30, %r13d	 # AVX512F
	vcvtss2si	{rz-sae}, %xmm30, %r13d	 # AVX512F

	vcvtss2si	{rn-sae}, %xmm30, %rax	 # AVX512F
	vcvtss2si	{ru-sae}, %xmm30, %rax	 # AVX512F
	vcvtss2si	{rd-sae}, %xmm30, %rax	 # AVX512F
	vcvtss2si	{rz-sae}, %xmm30, %rax	 # AVX512F
	vcvtss2si	{rn-sae}, %xmm30, %r8	 # AVX512F
	vcvtss2si	{ru-sae}, %xmm30, %r8	 # AVX512F
	vcvtss2si	{rd-sae}, %xmm30, %r8	 # AVX512F
	vcvtss2si	{rz-sae}, %xmm30, %r8	 # AVX512F

	vcvttpd2dq	%zmm29, %ymm30{%k7}	 # AVX512F
	vcvttpd2dq	%zmm29, %ymm30{%k7}{z}	 # AVX512F
	vcvttpd2dq	{sae}, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvttpd2dq	(%rcx), %ymm30{%k7}	 # AVX512F
	vcvttpd2dq	0x123(%rax,%r14,8), %ymm30{%k7}	 # AVX512F
	vcvttpd2dq	(%rcx){1to8}, %ymm30{%k7}	 # AVX512F
	vcvttpd2dq	8128(%rdx), %ymm30{%k7}	 # AVX512F Disp8
	vcvttpd2dq	8192(%rdx), %ymm30{%k7}	 # AVX512F
	vcvttpd2dq	-8192(%rdx), %ymm30{%k7}	 # AVX512F Disp8
	vcvttpd2dq	-8256(%rdx), %ymm30{%k7}	 # AVX512F
	vcvttpd2dq	1016(%rdx){1to8}, %ymm30{%k7}	 # AVX512F Disp8
	vcvttpd2dq	1024(%rdx){1to8}, %ymm30{%k7}	 # AVX512F
	vcvttpd2dq	-1024(%rdx){1to8}, %ymm30{%k7}	 # AVX512F Disp8
	vcvttpd2dq	-1032(%rdx){1to8}, %ymm30{%k7}	 # AVX512F

	vcvttps2dq	%zmm29, %zmm30	 # AVX512F
	vcvttps2dq	%zmm29, %zmm30{%k7}	 # AVX512F
	vcvttps2dq	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vcvttps2dq	{sae}, %zmm29, %zmm30	 # AVX512F
	vcvttps2dq	(%rcx), %zmm30	 # AVX512F
	vcvttps2dq	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vcvttps2dq	(%rcx){1to16}, %zmm30	 # AVX512F
	vcvttps2dq	8128(%rdx), %zmm30	 # AVX512F Disp8
	vcvttps2dq	8192(%rdx), %zmm30	 # AVX512F
	vcvttps2dq	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vcvttps2dq	-8256(%rdx), %zmm30	 # AVX512F
	vcvttps2dq	508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vcvttps2dq	512(%rdx){1to16}, %zmm30	 # AVX512F
	vcvttps2dq	-512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vcvttps2dq	-516(%rdx){1to16}, %zmm30	 # AVX512F

	vcvttsd2si	{sae}, %xmm30, %eax	 # AVX512F
	vcvttsd2si	{sae}, %xmm30, %ebp	 # AVX512F
	vcvttsd2si	{sae}, %xmm30, %r13d	 # AVX512F

	vcvttsd2si	{sae}, %xmm30, %rax	 # AVX512F
	vcvttsd2si	{sae}, %xmm30, %r8	 # AVX512F

	vcvttss2si	{sae}, %xmm30, %eax	 # AVX512F
	vcvttss2si	{sae}, %xmm30, %ebp	 # AVX512F
	vcvttss2si	{sae}, %xmm30, %r13d	 # AVX512F

	vcvttss2si	{sae}, %xmm30, %rax	 # AVX512F
	vcvttss2si	{sae}, %xmm30, %r8	 # AVX512F

	vcvtudq2pd	%ymm29, %zmm30{%k7}	 # AVX512F
	vcvtudq2pd	%ymm29, %zmm30{%k7}{z}	 # AVX512F
	vcvtudq2pd	(%rcx), %zmm30{%k7}	 # AVX512F
	vcvtudq2pd	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512F
	vcvtudq2pd	(%rcx){1to8}, %zmm30{%k7}	 # AVX512F
	vcvtudq2pd	4064(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vcvtudq2pd	4096(%rdx), %zmm30{%k7}	 # AVX512F
	vcvtudq2pd	-4096(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vcvtudq2pd	-4128(%rdx), %zmm30{%k7}	 # AVX512F
	vcvtudq2pd	508(%rdx){1to8}, %zmm30{%k7}	 # AVX512F Disp8
	vcvtudq2pd	512(%rdx){1to8}, %zmm30{%k7}	 # AVX512F
	vcvtudq2pd	-512(%rdx){1to8}, %zmm30{%k7}	 # AVX512F Disp8
	vcvtudq2pd	-516(%rdx){1to8}, %zmm30{%k7}	 # AVX512F

	vcvtudq2ps	%zmm29, %zmm30	 # AVX512F
	vcvtudq2ps	%zmm29, %zmm30{%k7}	 # AVX512F
	vcvtudq2ps	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vcvtudq2ps	{rn-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtudq2ps	{ru-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtudq2ps	{rd-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtudq2ps	{rz-sae}, %zmm29, %zmm30	 # AVX512F
	vcvtudq2ps	(%rcx), %zmm30	 # AVX512F
	vcvtudq2ps	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vcvtudq2ps	(%rcx){1to16}, %zmm30	 # AVX512F
	vcvtudq2ps	8128(%rdx), %zmm30	 # AVX512F Disp8
	vcvtudq2ps	8192(%rdx), %zmm30	 # AVX512F
	vcvtudq2ps	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vcvtudq2ps	-8256(%rdx), %zmm30	 # AVX512F
	vcvtudq2ps	508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vcvtudq2ps	512(%rdx){1to16}, %zmm30	 # AVX512F
	vcvtudq2ps	-512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vcvtudq2ps	-516(%rdx){1to16}, %zmm30	 # AVX512F

	vdivpd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vdivpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vdivpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vdivpd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vdivpd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vdivpd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vdivpd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vdivpd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vdivpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vdivpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vdivpd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vdivpd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vdivpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vdivpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vdivpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vdivpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vdivpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vdivpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vdivps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vdivps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vdivps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vdivps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vdivps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vdivps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vdivps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vdivps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vdivps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vdivps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vdivps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vdivps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vdivps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vdivps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vdivps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vdivps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vdivps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vdivps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vdivsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vdivsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vdivsd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vdivsd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vdivsd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vdivsd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vdivsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vdivsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vdivsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vdivsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vdivsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vdivsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vdivss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vdivss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vdivss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vdivss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vdivss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vdivss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vdivss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vdivss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vdivss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vdivss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vdivss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vdivss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vexpandpd	(%rcx), %zmm30	 # AVX512F
	vexpandpd	(%rcx), %zmm30{%k7}	 # AVX512F
	vexpandpd	(%rcx), %zmm30{%k7}{z}	 # AVX512F
	vexpandpd	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vexpandpd	1016(%rdx), %zmm30	 # AVX512F Disp8
	vexpandpd	1024(%rdx), %zmm30	 # AVX512F
	vexpandpd	-1024(%rdx), %zmm30	 # AVX512F Disp8
	vexpandpd	-1032(%rdx), %zmm30	 # AVX512F

	vexpandpd	%zmm29, %zmm30	 # AVX512F
	vexpandpd	%zmm29, %zmm30{%k7}	 # AVX512F
	vexpandpd	%zmm29, %zmm30{%k7}{z}	 # AVX512F

	vexpandps	(%rcx), %zmm30	 # AVX512F
	vexpandps	(%rcx), %zmm30{%k7}	 # AVX512F
	vexpandps	(%rcx), %zmm30{%k7}{z}	 # AVX512F
	vexpandps	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vexpandps	508(%rdx), %zmm30	 # AVX512F Disp8
	vexpandps	512(%rdx), %zmm30	 # AVX512F
	vexpandps	-512(%rdx), %zmm30	 # AVX512F Disp8
	vexpandps	-516(%rdx), %zmm30	 # AVX512F

	vexpandps	%zmm29, %zmm30	 # AVX512F
	vexpandps	%zmm29, %zmm30{%k7}	 # AVX512F
	vexpandps	%zmm29, %zmm30{%k7}{z}	 # AVX512F

	vextractf32x4	$0xab, %zmm29, %xmm30{%k7}	 # AVX512F
	vextractf32x4	$0xab, %zmm29, %xmm30{%k7}{z}	 # AVX512F
	vextractf32x4	$123, %zmm29, %xmm30{%k7}	 # AVX512F

	vextractf64x4	$0xab, %zmm29, %ymm30{%k7}	 # AVX512F
	vextractf64x4	$0xab, %zmm29, %ymm30{%k7}{z}	 # AVX512F
	vextractf64x4	$123, %zmm29, %ymm30{%k7}	 # AVX512F

	vextracti32x4	$0xab, %zmm29, %xmm30{%k7}	 # AVX512F
	vextracti32x4	$0xab, %zmm29, %xmm30{%k7}{z}	 # AVX512F
	vextracti32x4	$123, %zmm29, %xmm30{%k7}	 # AVX512F

	vextracti64x4	$0xab, %zmm29, %ymm30{%k7}	 # AVX512F
	vextracti64x4	$0xab, %zmm29, %ymm30{%k7}{z}	 # AVX512F
	vextracti64x4	$123, %zmm29, %ymm30{%k7}	 # AVX512F

	vextractps	$0xab, %xmm29, %eax	 # AVX512F
	vextractps	$123, %xmm29, %rax	 # AVX512F
	vextractps	$123, %xmm29, %r8	 # AVX512F
	vextractps	$123, %xmm29, (%rcx)	 # AVX512F
	vextractps	$123, %xmm29, 0x123(%rax,%r14,8)	 # AVX512F
	vextractps	$123, %xmm29, 508(%rdx)	 # AVX512F Disp8
	vextractps	$123, %xmm29, 512(%rdx)	 # AVX512F
	vextractps	$123, %xmm29, -512(%rdx)	 # AVX512F Disp8
	vextractps	$123, %xmm29, -516(%rdx)	 # AVX512F

	vfmadd132pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd132pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmadd132pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmadd132pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd132pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd132pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd132pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd132pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmadd132pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmadd132pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmadd132pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd132pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmadd132pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd132pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmadd132pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd132pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmadd132pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd132pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfmadd132ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd132ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmadd132ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmadd132ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd132ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd132ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd132ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd132ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmadd132ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmadd132ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmadd132ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd132ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmadd132ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd132ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmadd132ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd132ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmadd132ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd132ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfmadd132sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfmadd132sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmadd132sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmadd132sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfmadd132ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfmadd132ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmadd132ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd132ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmadd132ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfmadd213pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd213pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmadd213pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmadd213pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd213pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd213pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd213pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd213pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmadd213pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmadd213pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmadd213pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd213pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmadd213pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd213pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmadd213pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd213pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmadd213pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd213pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfmadd213ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd213ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmadd213ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmadd213ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd213ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd213ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd213ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd213ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmadd213ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmadd213ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmadd213ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd213ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmadd213ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd213ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmadd213ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd213ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmadd213ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd213ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfmadd213sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfmadd213sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmadd213sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmadd213sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfmadd213ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfmadd213ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmadd213ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd213ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmadd213ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfmadd231pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd231pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmadd231pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmadd231pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd231pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd231pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd231pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd231pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmadd231pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmadd231pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmadd231pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd231pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmadd231pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd231pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmadd231pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd231pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmadd231pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd231pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfmadd231ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd231ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmadd231ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmadd231ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd231ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd231ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd231ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmadd231ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmadd231ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmadd231ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmadd231ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd231ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmadd231ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd231ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmadd231ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd231ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmadd231ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmadd231ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfmadd231sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfmadd231sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmadd231sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmadd231sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfmadd231ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfmadd231ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmadd231ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmadd231ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmadd231ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfmaddsub132pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub132pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmaddsub132pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmaddsub132pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub132pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub132pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub132pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub132pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub132pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmaddsub132pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmaddsub132pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub132pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub132pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub132pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub132pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub132pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmaddsub132pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub132pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfmaddsub132ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub132ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmaddsub132ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmaddsub132ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub132ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub132ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub132ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub132ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub132ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmaddsub132ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmaddsub132ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub132ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub132ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub132ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub132ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub132ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmaddsub132ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub132ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfmaddsub213pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub213pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmaddsub213pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmaddsub213pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub213pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub213pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub213pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub213pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub213pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmaddsub213pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmaddsub213pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub213pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub213pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub213pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub213pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub213pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmaddsub213pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub213pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfmaddsub213ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub213ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmaddsub213ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmaddsub213ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub213ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub213ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub213ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub213ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub213ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmaddsub213ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmaddsub213ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub213ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub213ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub213ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub213ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub213ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmaddsub213ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub213ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfmaddsub231pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub231pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmaddsub231pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmaddsub231pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub231pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub231pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub231pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub231pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub231pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmaddsub231pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmaddsub231pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub231pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub231pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub231pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub231pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub231pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmaddsub231pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub231pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfmaddsub231ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub231ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmaddsub231ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmaddsub231ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub231ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub231ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub231ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmaddsub231ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub231ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmaddsub231ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmaddsub231ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub231ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub231ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub231ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmaddsub231ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub231ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmaddsub231ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmaddsub231ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfmsub132pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub132pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmsub132pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmsub132pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub132pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub132pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub132pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub132pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmsub132pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmsub132pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmsub132pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub132pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsub132pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub132pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsub132pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub132pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmsub132pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub132pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfmsub132ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub132ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmsub132ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmsub132ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub132ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub132ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub132ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub132ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmsub132ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmsub132ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmsub132ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub132ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsub132ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub132ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsub132ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub132ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmsub132ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub132ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfmsub132sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfmsub132sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmsub132sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmsub132sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfmsub132ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfmsub132ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmsub132ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub132ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmsub132ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfmsub213pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub213pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmsub213pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmsub213pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub213pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub213pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub213pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub213pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmsub213pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmsub213pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmsub213pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub213pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsub213pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub213pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsub213pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub213pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmsub213pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub213pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfmsub213ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub213ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmsub213ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmsub213ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub213ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub213ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub213ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub213ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmsub213ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmsub213ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmsub213ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub213ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsub213ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub213ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsub213ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub213ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmsub213ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub213ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfmsub213sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfmsub213sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmsub213sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmsub213sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfmsub213ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfmsub213ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmsub213ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub213ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmsub213ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfmsub231pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub231pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmsub231pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmsub231pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub231pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub231pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub231pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub231pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmsub231pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmsub231pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmsub231pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub231pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsub231pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub231pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsub231pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub231pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmsub231pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub231pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfmsub231ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub231ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmsub231ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmsub231ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub231ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub231ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub231ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsub231ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmsub231ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmsub231ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmsub231ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub231ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsub231ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub231ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsub231ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub231ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmsub231ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsub231ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfmsub231sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfmsub231sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmsub231sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmsub231sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfmsub231ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfmsub231ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmsub231ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfmsub231ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfmsub231ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfmsubadd132pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd132pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmsubadd132pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmsubadd132pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd132pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd132pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd132pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd132pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd132pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmsubadd132pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmsubadd132pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd132pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd132pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd132pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd132pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd132pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmsubadd132pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd132pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfmsubadd132ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd132ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmsubadd132ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmsubadd132ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd132ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd132ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd132ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd132ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd132ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmsubadd132ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmsubadd132ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd132ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd132ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd132ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd132ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd132ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmsubadd132ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd132ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfmsubadd213pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd213pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmsubadd213pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmsubadd213pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd213pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd213pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd213pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd213pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd213pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmsubadd213pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmsubadd213pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd213pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd213pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd213pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd213pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd213pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmsubadd213pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd213pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfmsubadd213ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd213ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmsubadd213ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmsubadd213ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd213ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd213ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd213ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd213ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd213ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmsubadd213ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmsubadd213ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd213ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd213ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd213ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd213ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd213ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmsubadd213ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd213ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfmsubadd231pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd231pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmsubadd231pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmsubadd231pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd231pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd231pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd231pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd231pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd231pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmsubadd231pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmsubadd231pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd231pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd231pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd231pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd231pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd231pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfmsubadd231pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd231pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfmsubadd231ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd231ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfmsubadd231ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfmsubadd231ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd231ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd231ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd231ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfmsubadd231ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd231ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfmsubadd231ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmsubadd231ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd231ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd231ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd231ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfmsubadd231ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd231ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfmsubadd231ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfmsubadd231ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfnmadd132pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd132pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfnmadd132pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfnmadd132pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd132pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd132pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd132pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd132pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfnmadd132pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfnmadd132pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfnmadd132pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd132pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmadd132pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd132pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmadd132pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd132pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfnmadd132pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd132pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfnmadd132ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd132ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfnmadd132ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfnmadd132ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd132ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd132ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd132ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd132ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfnmadd132ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfnmadd132ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfnmadd132ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd132ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmadd132ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd132ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmadd132ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd132ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfnmadd132ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd132ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfnmadd132sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfnmadd132sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmadd132sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmadd132sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfnmadd132ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfnmadd132ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmadd132ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd132ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmadd132ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfnmadd213pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd213pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfnmadd213pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfnmadd213pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd213pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd213pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd213pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd213pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfnmadd213pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfnmadd213pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfnmadd213pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd213pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmadd213pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd213pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmadd213pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd213pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfnmadd213pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd213pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfnmadd213ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd213ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfnmadd213ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfnmadd213ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd213ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd213ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd213ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd213ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfnmadd213ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfnmadd213ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfnmadd213ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd213ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmadd213ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd213ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmadd213ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd213ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfnmadd213ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd213ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfnmadd213sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfnmadd213sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmadd213sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmadd213sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfnmadd213ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfnmadd213ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmadd213ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd213ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmadd213ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfnmadd231pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd231pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfnmadd231pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfnmadd231pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd231pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd231pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd231pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd231pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfnmadd231pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfnmadd231pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfnmadd231pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd231pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmadd231pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd231pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmadd231pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd231pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfnmadd231pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd231pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfnmadd231ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd231ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfnmadd231ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfnmadd231ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd231ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd231ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd231ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmadd231ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfnmadd231ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfnmadd231ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfnmadd231ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd231ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmadd231ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd231ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmadd231ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd231ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfnmadd231ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmadd231ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfnmadd231sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfnmadd231sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmadd231sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmadd231sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfnmadd231ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfnmadd231ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmadd231ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmadd231ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmadd231ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfnmsub132pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub132pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfnmsub132pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfnmsub132pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub132pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub132pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub132pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub132pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfnmsub132pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfnmsub132pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfnmsub132pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub132pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmsub132pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub132pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmsub132pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub132pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfnmsub132pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub132pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfnmsub132ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub132ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfnmsub132ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfnmsub132ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub132ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub132ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub132ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub132ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfnmsub132ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfnmsub132ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfnmsub132ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub132ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmsub132ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub132ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmsub132ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub132ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfnmsub132ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub132ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfnmsub132sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfnmsub132sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmsub132sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmsub132sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfnmsub132ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfnmsub132ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmsub132ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub132ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmsub132ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfnmsub213pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub213pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfnmsub213pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfnmsub213pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub213pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub213pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub213pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub213pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfnmsub213pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfnmsub213pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfnmsub213pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub213pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmsub213pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub213pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmsub213pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub213pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfnmsub213pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub213pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfnmsub213ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub213ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfnmsub213ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfnmsub213ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub213ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub213ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub213ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub213ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfnmsub213ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfnmsub213ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfnmsub213ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub213ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmsub213ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub213ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmsub213ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub213ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfnmsub213ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub213ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfnmsub213sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfnmsub213sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmsub213sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmsub213sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfnmsub213ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfnmsub213ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmsub213ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub213ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmsub213ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfnmsub231pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub231pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfnmsub231pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfnmsub231pd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub231pd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub231pd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub231pd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub231pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfnmsub231pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfnmsub231pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfnmsub231pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub231pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmsub231pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub231pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmsub231pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub231pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfnmsub231pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub231pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfnmsub231ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub231ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfnmsub231ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfnmsub231ps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub231ps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub231ps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub231ps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfnmsub231ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vfnmsub231ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfnmsub231ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfnmsub231ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub231ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmsub231ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub231ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfnmsub231ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub231ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfnmsub231ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfnmsub231ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfnmsub231sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfnmsub231sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmsub231sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmsub231sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfnmsub231ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfnmsub231ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmsub231ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfnmsub231ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfnmsub231ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vgatherdpd	123(%r14,%ymm31,8), %zmm30{%k1}	 # AVX512F
	vgatherdpd	123(%r14,%ymm31,8), %zmm30{%k1}	 # AVX512F
	vgatherdpd	256(%r9,%ymm31), %zmm30{%k1}	 # AVX512F
	vgatherdpd	1024(%rcx,%ymm31,4), %zmm30{%k1}	 # AVX512F

	vgatherdps	123(%r14,%zmm31,8), %zmm30{%k1}	 # AVX512F
	vgatherdps	123(%r14,%zmm31,8), %zmm30{%k1}	 # AVX512F
	vgatherdps	256(%r9,%zmm31), %zmm30{%k1}	 # AVX512F
	vgatherdps	1024(%rcx,%zmm31,4), %zmm30{%k1}	 # AVX512F

	vgatherqpd	123(%r14,%zmm31,8), %zmm30{%k1}	 # AVX512F
	vgatherqpd	123(%r14,%zmm31,8), %zmm30{%k1}	 # AVX512F
	vgatherqpd	256(%r9,%zmm31), %zmm30{%k1}	 # AVX512F
	vgatherqpd	1024(%rcx,%zmm31,4), %zmm30{%k1}	 # AVX512F
	vgatherqpd	123(%r14,%zmm19,8), %zmm3{%k1}	 # AVX512F

	vgatherqps	123(%r14,%zmm31,8), %ymm30{%k1}	 # AVX512F
	vgatherqps	123(%r14,%zmm31,8), %ymm30{%k1}	 # AVX512F
	vgatherqps	256(%r9,%zmm31), %ymm30{%k1}	 # AVX512F
	vgatherqps	1024(%rcx,%zmm31,4), %ymm30{%k1}	 # AVX512F

	vgetexppd	%zmm29, %zmm30	 # AVX512F
	vgetexppd	%zmm29, %zmm30{%k7}	 # AVX512F
	vgetexppd	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vgetexppd	{sae}, %zmm29, %zmm30	 # AVX512F
	vgetexppd	(%rcx), %zmm30	 # AVX512F
	vgetexppd	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vgetexppd	(%rcx){1to8}, %zmm30	 # AVX512F
	vgetexppd	8128(%rdx), %zmm30	 # AVX512F Disp8
	vgetexppd	8192(%rdx), %zmm30	 # AVX512F
	vgetexppd	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vgetexppd	-8256(%rdx), %zmm30	 # AVX512F
	vgetexppd	1016(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vgetexppd	1024(%rdx){1to8}, %zmm30	 # AVX512F
	vgetexppd	-1024(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vgetexppd	-1032(%rdx){1to8}, %zmm30	 # AVX512F

	vgetexpps	%zmm29, %zmm30	 # AVX512F
	vgetexpps	%zmm29, %zmm30{%k7}	 # AVX512F
	vgetexpps	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vgetexpps	{sae}, %zmm29, %zmm30	 # AVX512F
	vgetexpps	(%rcx), %zmm30	 # AVX512F
	vgetexpps	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vgetexpps	(%rcx){1to16}, %zmm30	 # AVX512F
	vgetexpps	8128(%rdx), %zmm30	 # AVX512F Disp8
	vgetexpps	8192(%rdx), %zmm30	 # AVX512F
	vgetexpps	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vgetexpps	-8256(%rdx), %zmm30	 # AVX512F
	vgetexpps	508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vgetexpps	512(%rdx){1to16}, %zmm30	 # AVX512F
	vgetexpps	-512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vgetexpps	-516(%rdx){1to16}, %zmm30	 # AVX512F

	vgetexpsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vgetexpsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vgetexpsd	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vgetexpsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vgetexpsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vgetexpsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vgetexpsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vgetexpsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vgetexpsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vgetexpss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vgetexpss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vgetexpss	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vgetexpss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vgetexpss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vgetexpss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vgetexpss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vgetexpss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vgetexpss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vgetmantpd	$0xab, %zmm29, %zmm30	 # AVX512F
	vgetmantpd	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vgetmantpd	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vgetmantpd	$0xab, {sae}, %zmm29, %zmm30	 # AVX512F
	vgetmantpd	$123, %zmm29, %zmm30	 # AVX512F
	vgetmantpd	$123, {sae}, %zmm29, %zmm30	 # AVX512F
	vgetmantpd	$123, (%rcx), %zmm30	 # AVX512F
	vgetmantpd	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vgetmantpd	$123, (%rcx){1to8}, %zmm30	 # AVX512F
	vgetmantpd	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vgetmantpd	$123, 8192(%rdx), %zmm30	 # AVX512F
	vgetmantpd	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vgetmantpd	$123, -8256(%rdx), %zmm30	 # AVX512F
	vgetmantpd	$123, 1016(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vgetmantpd	$123, 1024(%rdx){1to8}, %zmm30	 # AVX512F
	vgetmantpd	$123, -1024(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vgetmantpd	$123, -1032(%rdx){1to8}, %zmm30	 # AVX512F

	vgetmantps	$0xab, %zmm29, %zmm30	 # AVX512F
	vgetmantps	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vgetmantps	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vgetmantps	$0xab, {sae}, %zmm29, %zmm30	 # AVX512F
	vgetmantps	$123, %zmm29, %zmm30	 # AVX512F
	vgetmantps	$123, {sae}, %zmm29, %zmm30	 # AVX512F
	vgetmantps	$123, (%rcx), %zmm30	 # AVX512F
	vgetmantps	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vgetmantps	$123, (%rcx){1to16}, %zmm30	 # AVX512F
	vgetmantps	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vgetmantps	$123, 8192(%rdx), %zmm30	 # AVX512F
	vgetmantps	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vgetmantps	$123, -8256(%rdx), %zmm30	 # AVX512F
	vgetmantps	$123, 508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vgetmantps	$123, 512(%rdx){1to16}, %zmm30	 # AVX512F
	vgetmantps	$123, -512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vgetmantps	$123, -516(%rdx){1to16}, %zmm30	 # AVX512F

	vgetmantsd	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vgetmantsd	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vgetmantsd	$0xab, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vgetmantsd	$123, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vgetmantsd	$123, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vgetmantsd	$123, (%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vgetmantsd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vgetmantsd	$123, 1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vgetmantsd	$123, 1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vgetmantsd	$123, -1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vgetmantsd	$123, -1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vgetmantss	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vgetmantss	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vgetmantss	$0xab, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vgetmantss	$123, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vgetmantss	$123, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vgetmantss	$123, (%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vgetmantss	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vgetmantss	$123, 508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vgetmantss	$123, 512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vgetmantss	$123, -512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vgetmantss	$123, -516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vinsertf32x4	$0xab, %xmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vinsertf32x4	$0xab, %xmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vinsertf32x4	$123, %xmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vinsertf32x4	$123, (%rcx), %zmm29, %zmm30{%k7}	 # AVX512F
	vinsertf32x4	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30{%k7}	 # AVX512F
	vinsertf32x4	$123, 2032(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vinsertf32x4	$123, 2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F
	vinsertf32x4	$123, -2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vinsertf32x4	$123, -2064(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F

	vinsertf64x4	$0xab, %ymm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vinsertf64x4	$0xab, %ymm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vinsertf64x4	$123, %ymm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vinsertf64x4	$123, (%rcx), %zmm29, %zmm30{%k7}	 # AVX512F
	vinsertf64x4	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30{%k7}	 # AVX512F
	vinsertf64x4	$123, 4064(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vinsertf64x4	$123, 4096(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F
	vinsertf64x4	$123, -4096(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vinsertf64x4	$123, -4128(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F

	vinserti32x4	$0xab, %xmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vinserti32x4	$0xab, %xmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vinserti32x4	$123, %xmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vinserti32x4	$123, (%rcx), %zmm29, %zmm30{%k7}	 # AVX512F
	vinserti32x4	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30{%k7}	 # AVX512F
	vinserti32x4	$123, 2032(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vinserti32x4	$123, 2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F
	vinserti32x4	$123, -2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vinserti32x4	$123, -2064(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F

	vinserti64x4	$0xab, %ymm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vinserti64x4	$0xab, %ymm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vinserti64x4	$123, %ymm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vinserti64x4	$123, (%rcx), %zmm29, %zmm30{%k7}	 # AVX512F
	vinserti64x4	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30{%k7}	 # AVX512F
	vinserti64x4	$123, 4064(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vinserti64x4	$123, 4096(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F
	vinserti64x4	$123, -4096(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vinserti64x4	$123, -4128(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F

	vinsertps	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512F
	vinsertps	$123, %xmm28, %xmm29, %xmm30	 # AVX512F
	vinsertps	$123, (%rcx), %xmm29, %xmm30	 # AVX512F
	vinsertps	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512F
	vinsertps	$123, 508(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vinsertps	$123, 512(%rdx), %xmm29, %xmm30	 # AVX512F
	vinsertps	$123, -512(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vinsertps	$123, -516(%rdx), %xmm29, %xmm30	 # AVX512F

	vmaxpd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vmaxpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vmaxpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmaxpd	{sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vmaxpd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vmaxpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vmaxpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vmaxpd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vmaxpd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vmaxpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vmaxpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vmaxpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vmaxpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vmaxpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vmaxpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vmaxps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vmaxps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vmaxps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmaxps	{sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vmaxps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vmaxps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vmaxps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vmaxps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vmaxps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vmaxps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vmaxps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vmaxps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vmaxps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vmaxps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vmaxps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vmaxsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmaxsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vmaxsd	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmaxsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vmaxsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vmaxsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vmaxsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vmaxsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vmaxsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vmaxss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmaxss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vmaxss	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmaxss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vmaxss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vmaxss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vmaxss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vmaxss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vmaxss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vminpd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vminpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vminpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vminpd	{sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vminpd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vminpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vminpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vminpd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vminpd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vminpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vminpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vminpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vminpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vminpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vminpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vminps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vminps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vminps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vminps	{sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vminps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vminps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vminps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vminps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vminps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vminps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vminps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vminps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vminps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vminps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vminps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vminsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vminsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vminsd	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vminsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vminsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vminsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vminsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vminsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vminsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vminss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vminss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vminss	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vminss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vminss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vminss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vminss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vminss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vminss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vmovapd	%zmm29, %zmm30	 # AVX512F
	vmovapd	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovapd	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovapd	(%rcx), %zmm30	 # AVX512F
	vmovapd	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vmovapd	8128(%rdx), %zmm30	 # AVX512F Disp8
	vmovapd	8192(%rdx), %zmm30	 # AVX512F
	vmovapd	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vmovapd	-8256(%rdx), %zmm30	 # AVX512F

	vmovaps	%zmm29, %zmm30	 # AVX512F
	vmovaps	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovaps	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovaps	(%rcx), %zmm30	 # AVX512F
	vmovaps	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vmovaps	8128(%rdx), %zmm30	 # AVX512F Disp8
	vmovaps	8192(%rdx), %zmm30	 # AVX512F
	vmovaps	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vmovaps	-8256(%rdx), %zmm30	 # AVX512F

	vmovd	%eax, %xmm30	 # AVX512F
	vmovd	%ebp, %xmm30	 # AVX512F
	vmovd	%r13d, %xmm30	 # AVX512F
	vmovd	(%rcx), %xmm30	 # AVX512F
	vmovd	0x123(%rax,%r14,8), %xmm30	 # AVX512F
	vmovd	508(%rdx), %xmm30	 # AVX512F Disp8
	vmovd	512(%rdx), %xmm30	 # AVX512F
	vmovd	-512(%rdx), %xmm30	 # AVX512F Disp8
	vmovd	-516(%rdx), %xmm30	 # AVX512F

	vmovd	%xmm30, (%rcx)	 # AVX512F
	vmovd	%xmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovd	%xmm30, 508(%rdx)	 # AVX512F Disp8
	vmovd	%xmm30, 512(%rdx)	 # AVX512F
	vmovd	%xmm30, -512(%rdx)	 # AVX512F Disp8
	vmovd	%xmm30, -516(%rdx)	 # AVX512F

	vmovddup	%zmm29, %zmm30	 # AVX512F
	vmovddup	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovddup	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovddup	(%rcx), %zmm30	 # AVX512F
	vmovddup	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vmovddup	8128(%rdx), %zmm30	 # AVX512F Disp8
	vmovddup	8192(%rdx), %zmm30	 # AVX512F
	vmovddup	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vmovddup	-8256(%rdx), %zmm30	 # AVX512F

	vmovdqa32	%zmm29, %zmm30	 # AVX512F
	vmovdqa32	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovdqa32	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovdqa32	(%rcx), %zmm30	 # AVX512F
	vmovdqa32	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vmovdqa32	8128(%rdx), %zmm30	 # AVX512F Disp8
	vmovdqa32	8192(%rdx), %zmm30	 # AVX512F
	vmovdqa32	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vmovdqa32	-8256(%rdx), %zmm30	 # AVX512F

	vmovdqa64	%zmm29, %zmm30	 # AVX512F
	vmovdqa64	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovdqa64	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovdqa64	(%rcx), %zmm30	 # AVX512F
	vmovdqa64	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vmovdqa64	8128(%rdx), %zmm30	 # AVX512F Disp8
	vmovdqa64	8192(%rdx), %zmm30	 # AVX512F
	vmovdqa64	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vmovdqa64	-8256(%rdx), %zmm30	 # AVX512F

	vmovdqu32	%zmm29, %zmm30	 # AVX512F
	vmovdqu32	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovdqu32	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovdqu32	(%rcx), %zmm30	 # AVX512F
	vmovdqu32	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vmovdqu32	8128(%rdx), %zmm30	 # AVX512F Disp8
	vmovdqu32	8192(%rdx), %zmm30	 # AVX512F
	vmovdqu32	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vmovdqu32	-8256(%rdx), %zmm30	 # AVX512F

	vmovdqu64	%zmm29, %zmm30	 # AVX512F
	vmovdqu64	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovdqu64	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovdqu64	(%rcx), %zmm30	 # AVX512F
	vmovdqu64	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vmovdqu64	8128(%rdx), %zmm30	 # AVX512F Disp8
	vmovdqu64	8192(%rdx), %zmm30	 # AVX512F
	vmovdqu64	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vmovdqu64	-8256(%rdx), %zmm30	 # AVX512F

	vmovhlps	%xmm28, %xmm29, %xmm30	 # AVX512F

	vmovhpd	(%rcx), %xmm30, %xmm29	 # AVX512F
	vmovhpd	0x123(%rax,%r14,8), %xmm30, %xmm29	 # AVX512F
	vmovhpd	1016(%rdx), %xmm30, %xmm29	 # AVX512F Disp8
	vmovhpd	1024(%rdx), %xmm30, %xmm29	 # AVX512F
	vmovhpd	-1024(%rdx), %xmm30, %xmm29	 # AVX512F Disp8
	vmovhpd	-1032(%rdx), %xmm30, %xmm29	 # AVX512F

	vmovhpd	%xmm30, (%rcx)	 # AVX512F
	vmovhpd	%xmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovhpd	%xmm30, 1016(%rdx)	 # AVX512F Disp8
	vmovhpd	%xmm30, 1024(%rdx)	 # AVX512F
	vmovhpd	%xmm30, -1024(%rdx)	 # AVX512F Disp8
	vmovhpd	%xmm30, -1032(%rdx)	 # AVX512F

	vmovhps	(%rcx), %xmm30, %xmm29	 # AVX512F
	vmovhps	0x123(%rax,%r14,8), %xmm30, %xmm29	 # AVX512F
	vmovhps	1016(%rdx), %xmm30, %xmm29	 # AVX512F Disp8
	vmovhps	1024(%rdx), %xmm30, %xmm29	 # AVX512F
	vmovhps	-1024(%rdx), %xmm30, %xmm29	 # AVX512F Disp8
	vmovhps	-1032(%rdx), %xmm30, %xmm29	 # AVX512F

	vmovhps	%xmm30, (%rcx)	 # AVX512F
	vmovhps	%xmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovhps	%xmm30, 1016(%rdx)	 # AVX512F Disp8
	vmovhps	%xmm30, 1024(%rdx)	 # AVX512F
	vmovhps	%xmm30, -1024(%rdx)	 # AVX512F Disp8
	vmovhps	%xmm30, -1032(%rdx)	 # AVX512F

	vmovlhps	%xmm28, %xmm29, %xmm30	 # AVX512F

	vmovlpd	(%rcx), %xmm30, %xmm29	 # AVX512F
	vmovlpd	0x123(%rax,%r14,8), %xmm30, %xmm29	 # AVX512F
	vmovlpd	1016(%rdx), %xmm30, %xmm29	 # AVX512F Disp8
	vmovlpd	1024(%rdx), %xmm30, %xmm29	 # AVX512F
	vmovlpd	-1024(%rdx), %xmm30, %xmm29	 # AVX512F Disp8
	vmovlpd	-1032(%rdx), %xmm30, %xmm29	 # AVX512F

	vmovlpd	%xmm30, (%rcx)	 # AVX512F
	vmovlpd	%xmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovlpd	%xmm30, 1016(%rdx)	 # AVX512F Disp8
	vmovlpd	%xmm30, 1024(%rdx)	 # AVX512F
	vmovlpd	%xmm30, -1024(%rdx)	 # AVX512F Disp8
	vmovlpd	%xmm30, -1032(%rdx)	 # AVX512F

	vmovlps	(%rcx), %xmm30, %xmm29	 # AVX512F
	vmovlps	0x123(%rax,%r14,8), %xmm30, %xmm29	 # AVX512F
	vmovlps	1016(%rdx), %xmm30, %xmm29	 # AVX512F Disp8
	vmovlps	1024(%rdx), %xmm30, %xmm29	 # AVX512F
	vmovlps	-1024(%rdx), %xmm30, %xmm29	 # AVX512F Disp8
	vmovlps	-1032(%rdx), %xmm30, %xmm29	 # AVX512F

	vmovlps	%xmm30, (%rcx)	 # AVX512F
	vmovlps	%xmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovlps	%xmm30, 1016(%rdx)	 # AVX512F Disp8
	vmovlps	%xmm30, 1024(%rdx)	 # AVX512F
	vmovlps	%xmm30, -1024(%rdx)	 # AVX512F Disp8
	vmovlps	%xmm30, -1032(%rdx)	 # AVX512F

	vmovntdq	%zmm30, (%rcx)	 # AVX512F
	vmovntdq	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovntdq	%zmm30, 8128(%rdx)	 # AVX512F Disp8
	vmovntdq	%zmm30, 8192(%rdx)	 # AVX512F
	vmovntdq	%zmm30, -8192(%rdx)	 # AVX512F Disp8
	vmovntdq	%zmm30, -8256(%rdx)	 # AVX512F

	vmovntdqa	(%rcx), %zmm30	 # AVX512F
	vmovntdqa	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vmovntdqa	8128(%rdx), %zmm30	 # AVX512F Disp8
	vmovntdqa	8192(%rdx), %zmm30	 # AVX512F
	vmovntdqa	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vmovntdqa	-8256(%rdx), %zmm30	 # AVX512F

	vmovntpd	%zmm30, (%rcx)	 # AVX512F
	vmovntpd	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovntpd	%zmm30, 8128(%rdx)	 # AVX512F Disp8
	vmovntpd	%zmm30, 8192(%rdx)	 # AVX512F
	vmovntpd	%zmm30, -8192(%rdx)	 # AVX512F Disp8
	vmovntpd	%zmm30, -8256(%rdx)	 # AVX512F

	vmovntps	%zmm30, (%rcx)	 # AVX512F
	vmovntps	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovntps	%zmm30, 8128(%rdx)	 # AVX512F Disp8
	vmovntps	%zmm30, 8192(%rdx)	 # AVX512F
	vmovntps	%zmm30, -8192(%rdx)	 # AVX512F Disp8
	vmovntps	%zmm30, -8256(%rdx)	 # AVX512F

	vmovq	%rax, %xmm30	 # AVX512F
	vmovq	%r8, %xmm30	 # AVX512F
	vmovq	(%rcx), %xmm30	 # AVX512F
	vmovq	0x123(%rax,%r14,8), %xmm30	 # AVX512F
	vmovq	1016(%rdx), %xmm30	 # AVX512F Disp8
	vmovq	1024(%rdx), %xmm30	 # AVX512F
	vmovq	-1024(%rdx), %xmm30	 # AVX512F Disp8
	vmovq	-1032(%rdx), %xmm30	 # AVX512F

	vmovq	%xmm30, (%rcx)	 # AVX512F
	vmovq	%xmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovq	%xmm30, 1016(%rdx)	 # AVX512F Disp8
	vmovq	%xmm30, 1024(%rdx)	 # AVX512F
	vmovq	%xmm30, -1024(%rdx)	 # AVX512F Disp8
	vmovq	%xmm30, -1032(%rdx)	 # AVX512F

	vmovq	%xmm29, %xmm30	 # AVX512F
	vmovq	(%rcx), %xmm30	 # AVX512F
	vmovq	0x123(%rax,%r14,8), %xmm30	 # AVX512F
	vmovq	1016(%rdx), %xmm30	 # AVX512F Disp8
	vmovq	1024(%rdx), %xmm30	 # AVX512F
	vmovq	-1024(%rdx), %xmm30	 # AVX512F Disp8
	vmovq	-1032(%rdx), %xmm30	 # AVX512F

	vmovq	%xmm29, (%rcx)	 # AVX512F
	vmovq	%xmm29, 0x123(%rax,%r14,8)	 # AVX512F
	vmovq	%xmm29, 1016(%rdx)	 # AVX512F Disp8
	vmovq	%xmm29, 1024(%rdx)	 # AVX512F
	vmovq	%xmm29, -1024(%rdx)	 # AVX512F Disp8
	vmovq	%xmm29, -1032(%rdx)	 # AVX512F

	vmovsd	(%rcx), %xmm30{%k7}	 # AVX512F
	vmovsd	(%rcx), %xmm30{%k7}{z}	 # AVX512F
	vmovsd	0x123(%rax,%r14,8), %xmm30{%k7}	 # AVX512F
	vmovsd	1016(%rdx), %xmm30{%k7}	 # AVX512F Disp8
	vmovsd	1024(%rdx), %xmm30{%k7}	 # AVX512F
	vmovsd	-1024(%rdx), %xmm30{%k7}	 # AVX512F Disp8
	vmovsd	-1032(%rdx), %xmm30{%k7}	 # AVX512F

	vmovsd	%xmm30, (%rcx){%k7}	 # AVX512F
	vmovsd	%xmm30, 0x123(%rax,%r14,8){%k7}	 # AVX512F
	vmovsd	%xmm30, 1016(%rdx){%k7}	 # AVX512F Disp8
	vmovsd	%xmm30, 1024(%rdx){%k7}	 # AVX512F
	vmovsd	%xmm30, -1024(%rdx){%k7}	 # AVX512F Disp8
	vmovsd	%xmm30, -1032(%rdx){%k7}	 # AVX512F

	vmovsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmovsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F

	vmovshdup	%zmm29, %zmm30	 # AVX512F
	vmovshdup	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovshdup	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovshdup	(%rcx), %zmm30	 # AVX512F
	vmovshdup	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vmovshdup	8128(%rdx), %zmm30	 # AVX512F Disp8
	vmovshdup	8192(%rdx), %zmm30	 # AVX512F
	vmovshdup	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vmovshdup	-8256(%rdx), %zmm30	 # AVX512F

	vmovsldup	%zmm29, %zmm30	 # AVX512F
	vmovsldup	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovsldup	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovsldup	(%rcx), %zmm30	 # AVX512F
	vmovsldup	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vmovsldup	8128(%rdx), %zmm30	 # AVX512F Disp8
	vmovsldup	8192(%rdx), %zmm30	 # AVX512F
	vmovsldup	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vmovsldup	-8256(%rdx), %zmm30	 # AVX512F

	vmovss	(%rcx), %xmm30{%k7}	 # AVX512F
	vmovss	(%rcx), %xmm30{%k7}{z}	 # AVX512F
	vmovss	0x123(%rax,%r14,8), %xmm30{%k7}	 # AVX512F
	vmovss	508(%rdx), %xmm30{%k7}	 # AVX512F Disp8
	vmovss	512(%rdx), %xmm30{%k7}	 # AVX512F
	vmovss	-512(%rdx), %xmm30{%k7}	 # AVX512F Disp8
	vmovss	-516(%rdx), %xmm30{%k7}	 # AVX512F

	vmovss	%xmm30, (%rcx){%k7}	 # AVX512F
	vmovss	%xmm30, 0x123(%rax,%r14,8){%k7}	 # AVX512F
	vmovss	%xmm30, 508(%rdx){%k7}	 # AVX512F Disp8
	vmovss	%xmm30, 512(%rdx){%k7}	 # AVX512F
	vmovss	%xmm30, -512(%rdx){%k7}	 # AVX512F Disp8
	vmovss	%xmm30, -516(%rdx){%k7}	 # AVX512F

	vmovss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmovss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F

	vmovupd	%zmm29, %zmm30	 # AVX512F
	vmovupd	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovupd	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovupd	(%rcx), %zmm30	 # AVX512F
	vmovupd	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vmovupd	8128(%rdx), %zmm30	 # AVX512F Disp8
	vmovupd	8192(%rdx), %zmm30	 # AVX512F
	vmovupd	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vmovupd	-8256(%rdx), %zmm30	 # AVX512F

	vmovups	%zmm29, %zmm30	 # AVX512F
	vmovups	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovups	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovups	(%rcx), %zmm30	 # AVX512F
	vmovups	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vmovups	8128(%rdx), %zmm30	 # AVX512F Disp8
	vmovups	8192(%rdx), %zmm30	 # AVX512F
	vmovups	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vmovups	-8256(%rdx), %zmm30	 # AVX512F

	vmulpd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vmulpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vmulpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmulpd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vmulpd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vmulpd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vmulpd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vmulpd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vmulpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vmulpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vmulpd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vmulpd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vmulpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vmulpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vmulpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vmulpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vmulpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vmulpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vmulps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vmulps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vmulps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmulps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vmulps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vmulps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vmulps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vmulps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vmulps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vmulps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vmulps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vmulps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vmulps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vmulps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vmulps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vmulps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vmulps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vmulps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vmulsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmulsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vmulsd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmulsd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmulsd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmulsd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmulsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vmulsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vmulsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vmulsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vmulsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vmulsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vmulss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmulss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vmulss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmulss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmulss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmulss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmulss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vmulss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vmulss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vmulss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vmulss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vmulss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vpabsd	%zmm29, %zmm30	 # AVX512F
	vpabsd	%zmm29, %zmm30{%k7}	 # AVX512F
	vpabsd	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpabsd	(%rcx), %zmm30	 # AVX512F
	vpabsd	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpabsd	(%rcx){1to16}, %zmm30	 # AVX512F
	vpabsd	8128(%rdx), %zmm30	 # AVX512F Disp8
	vpabsd	8192(%rdx), %zmm30	 # AVX512F
	vpabsd	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vpabsd	-8256(%rdx), %zmm30	 # AVX512F
	vpabsd	508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vpabsd	512(%rdx){1to16}, %zmm30	 # AVX512F
	vpabsd	-512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vpabsd	-516(%rdx){1to16}, %zmm30	 # AVX512F

	vpabsq	%zmm29, %zmm30	 # AVX512F
	vpabsq	%zmm29, %zmm30{%k7}	 # AVX512F
	vpabsq	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpabsq	(%rcx), %zmm30	 # AVX512F
	vpabsq	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpabsq	(%rcx){1to8}, %zmm30	 # AVX512F
	vpabsq	8128(%rdx), %zmm30	 # AVX512F Disp8
	vpabsq	8192(%rdx), %zmm30	 # AVX512F
	vpabsq	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vpabsq	-8256(%rdx), %zmm30	 # AVX512F
	vpabsq	1016(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vpabsq	1024(%rdx){1to8}, %zmm30	 # AVX512F
	vpabsq	-1024(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vpabsq	-1032(%rdx){1to8}, %zmm30	 # AVX512F

	vpaddd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpaddd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpaddd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpaddd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpaddd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpaddd	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpaddd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpaddd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpaddd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpaddd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpaddd	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpaddd	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpaddd	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpaddd	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpaddq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpaddq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpaddq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpaddq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpaddq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpaddq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpaddq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpaddq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpaddq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpaddq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpaddq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpaddq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpaddq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpaddq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpandd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpandd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpandd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpandd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpandd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpandd	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpandd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpandd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpandd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpandd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpandd	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpandd	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpandd	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpandd	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpandnd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpandnd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpandnd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpandnd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpandnd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpandnd	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpandnd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpandnd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpandnd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpandnd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpandnd	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpandnd	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpandnd	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpandnd	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpandnq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpandnq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpandnq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpandnq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpandnq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpandnq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpandnq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpandnq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpandnq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpandnq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpandnq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpandnq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpandnq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpandnq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpandq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpandq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpandq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpandq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpandq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpandq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpandq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpandq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpandq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpandq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpandq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpandq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpandq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpandq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpblendmd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpblendmd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpblendmd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpblendmd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpblendmd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpblendmd	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpblendmd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpblendmd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpblendmd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpblendmd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpblendmd	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpblendmd	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpblendmd	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpblendmd	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpbroadcastd	(%rcx), %zmm30	 # AVX512F
	vpbroadcastd	(%rcx), %zmm30{%k7}	 # AVX512F
	vpbroadcastd	(%rcx), %zmm30{%k7}{z}	 # AVX512F
	vpbroadcastd	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpbroadcastd	508(%rdx), %zmm30	 # AVX512F Disp8
	vpbroadcastd	512(%rdx), %zmm30	 # AVX512F
	vpbroadcastd	-512(%rdx), %zmm30	 # AVX512F Disp8
	vpbroadcastd	-516(%rdx), %zmm30	 # AVX512F

	vpbroadcastd	%xmm29, %zmm30{%k7}	 # AVX512F
	vpbroadcastd	%xmm29, %zmm30{%k7}{z}	 # AVX512F

	vpbroadcastd	%eax, %zmm30	 # AVX512F
	vpbroadcastd	%eax, %zmm30{%k7}	 # AVX512F
	vpbroadcastd	%eax, %zmm30{%k7}{z}	 # AVX512F
	vpbroadcastd	%ebp, %zmm30	 # AVX512F
	vpbroadcastd	%r13d, %zmm30	 # AVX512F

	vpbroadcastq	(%rcx), %zmm30	 # AVX512F
	vpbroadcastq	(%rcx), %zmm30{%k7}	 # AVX512F
	vpbroadcastq	(%rcx), %zmm30{%k7}{z}	 # AVX512F
	vpbroadcastq	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpbroadcastq	1016(%rdx), %zmm30	 # AVX512F Disp8
	vpbroadcastq	1024(%rdx), %zmm30	 # AVX512F
	vpbroadcastq	-1024(%rdx), %zmm30	 # AVX512F Disp8
	vpbroadcastq	-1032(%rdx), %zmm30	 # AVX512F

	vpbroadcastq	%xmm29, %zmm30{%k7}	 # AVX512F
	vpbroadcastq	%xmm29, %zmm30{%k7}{z}	 # AVX512F

	vpbroadcastq	%rax, %zmm30	 # AVX512F
	vpbroadcastq	%rax, %zmm30{%k7}	 # AVX512F
	vpbroadcastq	%rax, %zmm30{%k7}{z}	 # AVX512F
	vpbroadcastq	%r8, %zmm30	 # AVX512F

	vpcmpd	$0xab, %zmm29, %zmm30, %k5	 # AVX512F
	vpcmpd	$0xab, %zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpd	$123, %zmm29, %zmm30, %k5	 # AVX512F
	vpcmpd	$123, (%rcx), %zmm30, %k5	 # AVX512F
	vpcmpd	$123, 0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpd	$123, (%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpd	$123, 8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpd	$123, 8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpd	$123, -8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpd	$123, -8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpd	$123, 508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpd	$123, 512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpd	$123, -512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpd	$123, -516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vpcmpltd	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpltd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpltd	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpltd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpltd	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpltd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpltd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpltd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpltd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpltd	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpltd	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpltd	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpltd	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vpcmpled	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpled	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpled	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpled	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpled	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpled	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpled	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpled	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpled	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpled	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpled	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpled	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpled	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vpcmpneqd	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpneqd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpneqd	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpneqd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpneqd	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpneqd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpneqd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpneqd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpneqd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpneqd	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpneqd	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpneqd	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpneqd	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vpcmpnltd	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpnltd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpnltd	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpnltd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpnltd	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpnltd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnltd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnltd	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltd	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpnltd	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltd	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vpcmpnled	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpnled	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpnled	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpnled	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpnled	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpnled	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnled	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnled	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnled	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnled	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnled	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpnled	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnled	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vpcmpeqd	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpeqd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpeqd	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpeqd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpeqd	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpeqd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpeqd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpeqd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpeqd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpeqd	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpeqd	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpeqd	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpeqd	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vpcmpeqq	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpeqq	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpeqq	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpeqq	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpeqq	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpeqq	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpeqq	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpeqq	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpeqq	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpeqq	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpeqq	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpeqq	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpeqq	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpcmpgtd	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpgtd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpgtd	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpgtd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpgtd	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpgtd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpgtd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpgtd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpgtd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpgtd	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpgtd	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpgtd	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpgtd	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vpcmpgtq	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpgtq	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpgtq	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpgtq	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpgtq	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpgtq	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpgtq	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpgtq	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpgtq	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpgtq	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpgtq	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpgtq	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpgtq	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpcmpq	$0xab, %zmm29, %zmm30, %k5	 # AVX512F
	vpcmpq	$0xab, %zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpq	$123, %zmm29, %zmm30, %k5	 # AVX512F
	vpcmpq	$123, (%rcx), %zmm30, %k5	 # AVX512F
	vpcmpq	$123, 0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpq	$123, (%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpq	$123, 8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpq	$123, 8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpq	$123, -8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpq	$123, -8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpq	$123, 1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpq	$123, 1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpq	$123, -1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpq	$123, -1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpcmpltq	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpltq	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpltq	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpltq	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpltq	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpltq	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpltq	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpltq	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpltq	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpltq	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpltq	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpltq	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpltq	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpcmpleq	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpleq	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpleq	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpleq	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpleq	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpleq	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpleq	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpleq	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpleq	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpleq	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpleq	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpleq	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpleq	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpcmpneqq	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpneqq	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpneqq	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpneqq	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpneqq	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpneqq	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpneqq	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpneqq	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpneqq	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpneqq	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpneqq	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpneqq	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpneqq	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpcmpnltq	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpnltq	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpnltq	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpnltq	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpnltq	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpnltq	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltq	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnltq	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltq	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnltq	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltq	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpnltq	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltq	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpcmpnleq	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpnleq	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpnleq	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpnleq	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpnleq	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpnleq	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnleq	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnleq	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnleq	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnleq	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnleq	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpnleq	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnleq	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpcmpud	$0xab, %zmm29, %zmm30, %k5	 # AVX512F
	vpcmpud	$0xab, %zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpud	$123, %zmm29, %zmm30, %k5	 # AVX512F
	vpcmpud	$123, (%rcx), %zmm30, %k5	 # AVX512F
	vpcmpud	$123, 0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpud	$123, (%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpud	$123, 8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpud	$123, 8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpud	$123, -8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpud	$123, -8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpud	$123, 508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpud	$123, 512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpud	$123, -512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpud	$123, -516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vpcmpequd	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpequd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpequd	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpequd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpequd	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpequd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpequd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpequd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpequd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpequd	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpequd	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpequd	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpequd	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vpcmpltud	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpltud	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpltud	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpltud	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpltud	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpltud	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpltud	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpltud	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpltud	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpltud	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpltud	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpltud	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpltud	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vpcmpleud	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpleud	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpleud	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpleud	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpleud	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpleud	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpleud	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpleud	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpleud	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpleud	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpleud	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpleud	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpleud	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vpcmpnequd	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpnequd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpnequd	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpnequd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpnequd	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpnequd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnequd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnequd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnequd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnequd	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnequd	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpnequd	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnequd	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vpcmpnltud	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpnltud	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpnltud	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpnltud	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpnltud	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpnltud	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltud	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnltud	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltud	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnltud	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltud	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpnltud	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltud	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vpcmpnleud	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpnleud	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpnleud	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpnleud	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpnleud	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpnleud	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnleud	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnleud	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnleud	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnleud	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnleud	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vpcmpnleud	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnleud	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vpcmpuq	$0xab, %zmm29, %zmm30, %k5	 # AVX512F
	vpcmpuq	$0xab, %zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpuq	$123, %zmm29, %zmm30, %k5	 # AVX512F
	vpcmpuq	$123, (%rcx), %zmm30, %k5	 # AVX512F
	vpcmpuq	$123, 0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpuq	$123, (%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpuq	$123, 8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpuq	$123, 8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpuq	$123, -8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpuq	$123, -8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpuq	$123, 1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpuq	$123, 1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpuq	$123, -1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpuq	$123, -1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpcmpequq	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpequq	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpequq	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpequq	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpequq	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpequq	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpequq	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpequq	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpequq	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpequq	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpequq	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpequq	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpequq	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpcmpltuq	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpltuq	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpltuq	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpltuq	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpltuq	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpltuq	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpltuq	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpltuq	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpltuq	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpltuq	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpltuq	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpltuq	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpltuq	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpcmpleuq	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpleuq	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpleuq	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpleuq	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpleuq	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpleuq	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpleuq	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpleuq	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpleuq	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpleuq	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpleuq	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpleuq	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpleuq	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpcmpnequq	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpnequq	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpnequq	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpnequq	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpnequq	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpnequq	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnequq	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnequq	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnequq	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnequq	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnequq	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpnequq	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnequq	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpcmpnltuq	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpnltuq	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpnltuq	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpnltuq	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpnltuq	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpnltuq	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltuq	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnltuq	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltuq	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnltuq	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltuq	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpnltuq	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnltuq	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpcmpnleuq	%zmm29, %zmm30, %k5	 # AVX512F
	vpcmpnleuq	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vpcmpnleuq	(%rcx), %zmm30, %k5	 # AVX512F
	vpcmpnleuq	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vpcmpnleuq	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpnleuq	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnleuq	8192(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnleuq	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vpcmpnleuq	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vpcmpnleuq	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnleuq	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vpcmpnleuq	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vpcmpnleuq	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpblendmq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpblendmq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpblendmq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpblendmq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpblendmq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpblendmq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpblendmq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpblendmq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpblendmq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpblendmq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpblendmq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpblendmq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpblendmq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpblendmq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpcompressd	%zmm30, (%rcx)	 # AVX512F
	vpcompressd	%zmm30, (%rcx){%k7}	 # AVX512F
	vpcompressd	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpcompressd	%zmm30, 508(%rdx)	 # AVX512F Disp8
	vpcompressd	%zmm30, 512(%rdx)	 # AVX512F
	vpcompressd	%zmm30, -512(%rdx)	 # AVX512F Disp8
	vpcompressd	%zmm30, -516(%rdx)	 # AVX512F

	vpcompressd	%zmm29, %zmm30	 # AVX512F
	vpcompressd	%zmm29, %zmm30{%k7}	 # AVX512F
	vpcompressd	%zmm29, %zmm30{%k7}{z}	 # AVX512F

	vpermd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpermd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpermd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpermd	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpermd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermd	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermd	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpermd	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermd	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpermilpd	$0xab, %zmm29, %zmm30	 # AVX512F
	vpermilpd	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermilpd	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermilpd	$123, %zmm29, %zmm30	 # AVX512F
	vpermilpd	$123, (%rcx), %zmm30	 # AVX512F
	vpermilpd	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpermilpd	$123, (%rcx){1to8}, %zmm30	 # AVX512F
	vpermilpd	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vpermilpd	$123, 8192(%rdx), %zmm30	 # AVX512F
	vpermilpd	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vpermilpd	$123, -8256(%rdx), %zmm30	 # AVX512F
	vpermilpd	$123, 1016(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vpermilpd	$123, 1024(%rdx){1to8}, %zmm30	 # AVX512F
	vpermilpd	$123, -1024(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vpermilpd	$123, -1032(%rdx){1to8}, %zmm30	 # AVX512F

	vpermilpd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpermilpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermilpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermilpd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpermilpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpermilpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpermilpd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermilpd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermilpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermilpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermilpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermilpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpermilpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermilpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpermilps	$0xab, %zmm29, %zmm30	 # AVX512F
	vpermilps	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermilps	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermilps	$123, %zmm29, %zmm30	 # AVX512F
	vpermilps	$123, (%rcx), %zmm30	 # AVX512F
	vpermilps	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpermilps	$123, (%rcx){1to16}, %zmm30	 # AVX512F
	vpermilps	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vpermilps	$123, 8192(%rdx), %zmm30	 # AVX512F
	vpermilps	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vpermilps	$123, -8256(%rdx), %zmm30	 # AVX512F
	vpermilps	$123, 508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vpermilps	$123, 512(%rdx){1to16}, %zmm30	 # AVX512F
	vpermilps	$123, -512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vpermilps	$123, -516(%rdx){1to16}, %zmm30	 # AVX512F

	vpermilps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpermilps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermilps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermilps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpermilps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpermilps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpermilps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermilps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermilps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermilps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermilps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermilps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpermilps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermilps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpermpd	$0xab, %zmm29, %zmm30	 # AVX512F
	vpermpd	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermpd	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermpd	$123, %zmm29, %zmm30	 # AVX512F
	vpermpd	$123, (%rcx), %zmm30	 # AVX512F
	vpermpd	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpermpd	$123, (%rcx){1to8}, %zmm30	 # AVX512F
	vpermpd	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vpermpd	$123, 8192(%rdx), %zmm30	 # AVX512F
	vpermpd	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vpermpd	$123, -8256(%rdx), %zmm30	 # AVX512F
	vpermpd	$123, 1016(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vpermpd	$123, 1024(%rdx){1to8}, %zmm30	 # AVX512F
	vpermpd	$123, -1024(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vpermpd	$123, -1032(%rdx){1to8}, %zmm30	 # AVX512F

	vpermps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpermps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpermps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpermps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpermps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpermps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpermq	$0xab, %zmm29, %zmm30	 # AVX512F
	vpermq	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermq	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermq	$123, %zmm29, %zmm30	 # AVX512F
	vpermq	$123, (%rcx), %zmm30	 # AVX512F
	vpermq	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpermq	$123, (%rcx){1to8}, %zmm30	 # AVX512F
	vpermq	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vpermq	$123, 8192(%rdx), %zmm30	 # AVX512F
	vpermq	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vpermq	$123, -8256(%rdx), %zmm30	 # AVX512F
	vpermq	$123, 1016(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vpermq	$123, 1024(%rdx){1to8}, %zmm30	 # AVX512F
	vpermq	$123, -1024(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vpermq	$123, -1032(%rdx){1to8}, %zmm30	 # AVX512F

	vpexpandd	(%rcx), %zmm30	 # AVX512F
	vpexpandd	(%rcx), %zmm30{%k7}	 # AVX512F
	vpexpandd	(%rcx), %zmm30{%k7}{z}	 # AVX512F
	vpexpandd	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpexpandd	508(%rdx), %zmm30	 # AVX512F Disp8
	vpexpandd	512(%rdx), %zmm30	 # AVX512F
	vpexpandd	-512(%rdx), %zmm30	 # AVX512F Disp8
	vpexpandd	-516(%rdx), %zmm30	 # AVX512F

	vpexpandd	%zmm29, %zmm30	 # AVX512F
	vpexpandd	%zmm29, %zmm30{%k7}	 # AVX512F
	vpexpandd	%zmm29, %zmm30{%k7}{z}	 # AVX512F

	vpexpandq	(%rcx), %zmm30	 # AVX512F
	vpexpandq	(%rcx), %zmm30{%k7}	 # AVX512F
	vpexpandq	(%rcx), %zmm30{%k7}{z}	 # AVX512F
	vpexpandq	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpexpandq	1016(%rdx), %zmm30	 # AVX512F Disp8
	vpexpandq	1024(%rdx), %zmm30	 # AVX512F
	vpexpandq	-1024(%rdx), %zmm30	 # AVX512F Disp8
	vpexpandq	-1032(%rdx), %zmm30	 # AVX512F

	vpexpandq	%zmm29, %zmm30	 # AVX512F
	vpexpandq	%zmm29, %zmm30{%k7}	 # AVX512F
	vpexpandq	%zmm29, %zmm30{%k7}{z}	 # AVX512F

	vpgatherdd	123(%r14,%zmm31,8), %zmm30{%k1}	 # AVX512F
	vpgatherdd	123(%r14,%zmm31,8), %zmm30{%k1}	 # AVX512F
	vpgatherdd	256(%r9,%zmm31), %zmm30{%k1}	 # AVX512F
	vpgatherdd	1024(%rcx,%zmm31,4), %zmm30{%k1}	 # AVX512F

	vpgatherdq	123(%r14,%ymm31,8), %zmm30{%k1}	 # AVX512F
	vpgatherdq	123(%r14,%ymm31,8), %zmm30{%k1}	 # AVX512F
	vpgatherdq	256(%r9,%ymm31), %zmm30{%k1}	 # AVX512F
	vpgatherdq	1024(%rcx,%ymm31,4), %zmm30{%k1}	 # AVX512F

	vpgatherqd	123(%r14,%zmm31,8), %ymm30{%k1}	 # AVX512F
	vpgatherqd	123(%r14,%zmm31,8), %ymm30{%k1}	 # AVX512F
	vpgatherqd	256(%r9,%zmm31), %ymm30{%k1}	 # AVX512F
	vpgatherqd	1024(%rcx,%zmm31,4), %ymm30{%k1}	 # AVX512F

	vpgatherqq	123(%r14,%zmm31,8), %zmm30{%k1}	 # AVX512F
	vpgatherqq	123(%r14,%zmm31,8), %zmm30{%k1}	 # AVX512F
	vpgatherqq	256(%r9,%zmm31), %zmm30{%k1}	 # AVX512F
	vpgatherqq	1024(%rcx,%zmm31,4), %zmm30{%k1}	 # AVX512F

	vpmaxsd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpmaxsd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpmaxsd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpmaxsd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpmaxsd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpmaxsd	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpmaxsd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxsd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpmaxsd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxsd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpmaxsd	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxsd	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpmaxsd	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxsd	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpmaxsq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpmaxsq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpmaxsq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpmaxsq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpmaxsq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpmaxsq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpmaxsq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxsq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpmaxsq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxsq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpmaxsq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxsq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpmaxsq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxsq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpmaxud	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpmaxud	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpmaxud	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpmaxud	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpmaxud	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpmaxud	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpmaxud	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxud	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpmaxud	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxud	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpmaxud	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxud	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpmaxud	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxud	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpmaxuq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpmaxuq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpmaxuq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpmaxuq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpmaxuq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpmaxuq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpmaxuq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxuq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpmaxuq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxuq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpmaxuq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxuq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpmaxuq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpmaxuq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpminsd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpminsd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpminsd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpminsd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpminsd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpminsd	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpminsd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpminsd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpminsd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpminsd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpminsd	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpminsd	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpminsd	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpminsd	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpminsq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpminsq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpminsq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpminsq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpminsq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpminsq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpminsq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpminsq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpminsq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpminsq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpminsq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpminsq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpminsq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpminsq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpminud	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpminud	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpminud	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpminud	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpminud	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpminud	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpminud	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpminud	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpminud	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpminud	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpminud	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpminud	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpminud	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpminud	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpminuq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpminuq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpminuq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpminuq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpminuq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpminuq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpminuq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpminuq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpminuq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpminuq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpminuq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpminuq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpminuq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpminuq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpmovsxbd	%xmm29, %zmm30{%k7}	 # AVX512F
	vpmovsxbd	%xmm29, %zmm30{%k7}{z}	 # AVX512F
	vpmovsxbd	(%rcx), %zmm30{%k7}	 # AVX512F
	vpmovsxbd	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512F
	vpmovsxbd	2032(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovsxbd	2048(%rdx), %zmm30{%k7}	 # AVX512F
	vpmovsxbd	-2048(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovsxbd	-2064(%rdx), %zmm30{%k7}	 # AVX512F

	vpmovsxbq	%xmm29, %zmm30{%k7}	 # AVX512F
	vpmovsxbq	%xmm29, %zmm30{%k7}{z}	 # AVX512F
	vpmovsxbq	(%rcx), %zmm30{%k7}	 # AVX512F
	vpmovsxbq	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512F
	vpmovsxbq	1016(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovsxbq	1024(%rdx), %zmm30{%k7}	 # AVX512F
	vpmovsxbq	-1024(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovsxbq	-1032(%rdx), %zmm30{%k7}	 # AVX512F

	vpmovsxdq	%ymm29, %zmm30{%k7}	 # AVX512F
	vpmovsxdq	%ymm29, %zmm30{%k7}{z}	 # AVX512F
	vpmovsxdq	(%rcx), %zmm30{%k7}	 # AVX512F
	vpmovsxdq	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512F
	vpmovsxdq	4064(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovsxdq	4096(%rdx), %zmm30{%k7}	 # AVX512F
	vpmovsxdq	-4096(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovsxdq	-4128(%rdx), %zmm30{%k7}	 # AVX512F

	vpmovsxwd	%ymm29, %zmm30{%k7}	 # AVX512F
	vpmovsxwd	%ymm29, %zmm30{%k7}{z}	 # AVX512F
	vpmovsxwd	(%rcx), %zmm30{%k7}	 # AVX512F
	vpmovsxwd	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512F
	vpmovsxwd	4064(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovsxwd	4096(%rdx), %zmm30{%k7}	 # AVX512F
	vpmovsxwd	-4096(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovsxwd	-4128(%rdx), %zmm30{%k7}	 # AVX512F

	vpmovsxwq	%xmm29, %zmm30{%k7}	 # AVX512F
	vpmovsxwq	%xmm29, %zmm30{%k7}{z}	 # AVX512F
	vpmovsxwq	(%rcx), %zmm30{%k7}	 # AVX512F
	vpmovsxwq	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512F
	vpmovsxwq	2032(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovsxwq	2048(%rdx), %zmm30{%k7}	 # AVX512F
	vpmovsxwq	-2048(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovsxwq	-2064(%rdx), %zmm30{%k7}	 # AVX512F

	vpmovzxbd	%xmm29, %zmm30{%k7}	 # AVX512F
	vpmovzxbd	%xmm29, %zmm30{%k7}{z}	 # AVX512F
	vpmovzxbd	(%rcx), %zmm30{%k7}	 # AVX512F
	vpmovzxbd	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512F
	vpmovzxbd	2032(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovzxbd	2048(%rdx), %zmm30{%k7}	 # AVX512F
	vpmovzxbd	-2048(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovzxbd	-2064(%rdx), %zmm30{%k7}	 # AVX512F

	vpmovzxbq	%xmm29, %zmm30{%k7}	 # AVX512F
	vpmovzxbq	%xmm29, %zmm30{%k7}{z}	 # AVX512F
	vpmovzxbq	(%rcx), %zmm30{%k7}	 # AVX512F
	vpmovzxbq	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512F
	vpmovzxbq	1016(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovzxbq	1024(%rdx), %zmm30{%k7}	 # AVX512F
	vpmovzxbq	-1024(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovzxbq	-1032(%rdx), %zmm30{%k7}	 # AVX512F

	vpmovzxdq	%ymm29, %zmm30{%k7}	 # AVX512F
	vpmovzxdq	%ymm29, %zmm30{%k7}{z}	 # AVX512F
	vpmovzxdq	(%rcx), %zmm30{%k7}	 # AVX512F
	vpmovzxdq	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512F
	vpmovzxdq	4064(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovzxdq	4096(%rdx), %zmm30{%k7}	 # AVX512F
	vpmovzxdq	-4096(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovzxdq	-4128(%rdx), %zmm30{%k7}	 # AVX512F

	vpmovzxwd	%ymm29, %zmm30{%k7}	 # AVX512F
	vpmovzxwd	%ymm29, %zmm30{%k7}{z}	 # AVX512F
	vpmovzxwd	(%rcx), %zmm30{%k7}	 # AVX512F
	vpmovzxwd	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512F
	vpmovzxwd	4064(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovzxwd	4096(%rdx), %zmm30{%k7}	 # AVX512F
	vpmovzxwd	-4096(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovzxwd	-4128(%rdx), %zmm30{%k7}	 # AVX512F

	vpmovzxwq	%xmm29, %zmm30{%k7}	 # AVX512F
	vpmovzxwq	%xmm29, %zmm30{%k7}{z}	 # AVX512F
	vpmovzxwq	(%rcx), %zmm30{%k7}	 # AVX512F
	vpmovzxwq	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512F
	vpmovzxwq	2032(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovzxwq	2048(%rdx), %zmm30{%k7}	 # AVX512F
	vpmovzxwq	-2048(%rdx), %zmm30{%k7}	 # AVX512F Disp8
	vpmovzxwq	-2064(%rdx), %zmm30{%k7}	 # AVX512F

	vpmuldq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpmuldq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpmuldq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpmuldq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpmuldq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpmuldq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpmuldq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpmuldq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpmuldq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpmuldq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpmuldq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpmuldq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpmuldq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpmuldq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpmulld	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpmulld	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpmulld	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpmulld	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpmulld	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpmulld	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpmulld	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpmulld	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpmulld	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpmulld	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpmulld	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpmulld	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpmulld	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpmulld	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpmuludq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpmuludq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpmuludq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpmuludq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpmuludq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpmuludq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpmuludq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpmuludq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpmuludq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpmuludq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpmuludq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpmuludq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpmuludq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpmuludq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpord	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpord	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpord	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpord	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpord	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpord	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpord	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpord	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpord	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpord	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpord	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpord	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpord	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpord	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vporq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vporq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vporq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vporq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vporq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vporq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vporq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vporq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vporq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vporq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vporq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vporq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vporq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vporq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpscatterdd	%zmm30, 123(%r14,%zmm31,8){%k1}	 # AVX512F
	vpscatterdd	%zmm30, 123(%r14,%zmm31,8){%k1}	 # AVX512F
	vpscatterdd	%zmm30, 256(%r9,%zmm31){%k1}	 # AVX512F
	vpscatterdd	%zmm30, 1024(%rcx,%zmm31,4){%k1}	 # AVX512F

	vpscatterdq	%zmm30, 123(%r14,%ymm31,8){%k1}	 # AVX512F
	vpscatterdq	%zmm30, 123(%r14,%ymm31,8){%k1}	 # AVX512F
	vpscatterdq	%zmm30, 256(%r9,%ymm31){%k1}	 # AVX512F
	vpscatterdq	%zmm30, 1024(%rcx,%ymm31,4){%k1}	 # AVX512F

	vpscatterqd	%ymm30, 123(%r14,%zmm31,8){%k1}	 # AVX512F
	vpscatterqd	%ymm30, 123(%r14,%zmm31,8){%k1}	 # AVX512F
	vpscatterqd	%ymm30, 256(%r9,%zmm31){%k1}	 # AVX512F
	vpscatterqd	%ymm30, 1024(%rcx,%zmm31,4){%k1}	 # AVX512F

	vpscatterqq	%zmm30, 123(%r14,%zmm31,8){%k1}	 # AVX512F
	vpscatterqq	%zmm30, 123(%r14,%zmm31,8){%k1}	 # AVX512F
	vpscatterqq	%zmm30, 256(%r9,%zmm31){%k1}	 # AVX512F
	vpscatterqq	%zmm30, 1024(%rcx,%zmm31,4){%k1}	 # AVX512F

	vpshufd	$0xab, %zmm29, %zmm30	 # AVX512F
	vpshufd	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vpshufd	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpshufd	$123, %zmm29, %zmm30	 # AVX512F
	vpshufd	$123, (%rcx), %zmm30	 # AVX512F
	vpshufd	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpshufd	$123, (%rcx){1to16}, %zmm30	 # AVX512F
	vpshufd	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vpshufd	$123, 8192(%rdx), %zmm30	 # AVX512F
	vpshufd	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vpshufd	$123, -8256(%rdx), %zmm30	 # AVX512F
	vpshufd	$123, 508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vpshufd	$123, 512(%rdx){1to16}, %zmm30	 # AVX512F
	vpshufd	$123, -512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vpshufd	$123, -516(%rdx){1to16}, %zmm30	 # AVX512F

	vpslld	%xmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpslld	%xmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpslld	(%rcx), %zmm29, %zmm30{%k7}	 # AVX512F
	vpslld	0x123(%rax,%r14,8), %zmm29, %zmm30{%k7}	 # AVX512F
	vpslld	2032(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vpslld	2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F
	vpslld	-2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vpslld	-2064(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F

	vpsllq	%xmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsllq	%xmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsllq	(%rcx), %zmm29, %zmm30{%k7}	 # AVX512F
	vpsllq	0x123(%rax,%r14,8), %zmm29, %zmm30{%k7}	 # AVX512F
	vpsllq	2032(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vpsllq	2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F
	vpsllq	-2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vpsllq	-2064(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F

	vpsllvd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpsllvd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsllvd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsllvd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpsllvd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpsllvd	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpsllvd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsllvd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsllvd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsllvd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsllvd	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsllvd	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpsllvd	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsllvd	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpsllvq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpsllvq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsllvq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsllvq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpsllvq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpsllvq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpsllvq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsllvq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsllvq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsllvq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsllvq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsllvq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpsllvq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsllvq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpsrad	%xmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrad	%xmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsrad	(%rcx), %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrad	0x123(%rax,%r14,8), %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrad	2032(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vpsrad	2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrad	-2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vpsrad	-2064(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F

	vpsraq	%xmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsraq	%xmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsraq	(%rcx), %zmm29, %zmm30{%k7}	 # AVX512F
	vpsraq	0x123(%rax,%r14,8), %zmm29, %zmm30{%k7}	 # AVX512F
	vpsraq	2032(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vpsraq	2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F
	vpsraq	-2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vpsraq	-2064(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F

	vpsravd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpsravd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsravd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsravd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpsravd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpsravd	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpsravd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsravd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsravd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsravd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsravd	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsravd	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpsravd	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsravd	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpsravq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpsravq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsravq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsravq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpsravq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpsravq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpsravq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsravq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsravq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsravq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsravq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsravq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpsravq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsravq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpsrld	%xmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrld	%xmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsrld	(%rcx), %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrld	0x123(%rax,%r14,8), %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrld	2032(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vpsrld	2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrld	-2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vpsrld	-2064(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F

	vpsrlq	%xmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrlq	%xmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsrlq	(%rcx), %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrlq	0x123(%rax,%r14,8), %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrlq	2032(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vpsrlq	2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrlq	-2048(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F Disp8
	vpsrlq	-2064(%rdx), %zmm29, %zmm30{%k7}	 # AVX512F

	vpsrlvd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpsrlvd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrlvd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsrlvd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpsrlvd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpsrlvd	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpsrlvd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsrlvd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsrlvd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsrlvd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsrlvd	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsrlvd	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpsrlvd	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsrlvd	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpsrlvq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpsrlvq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrlvq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsrlvq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpsrlvq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpsrlvq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpsrlvq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsrlvq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsrlvq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsrlvq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsrlvq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsrlvq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpsrlvq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsrlvq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpsrld	$0xab, %zmm29, %zmm30	 # AVX512F
	vpsrld	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrld	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsrld	$123, %zmm29, %zmm30	 # AVX512F
	vpsrld	$123, (%rcx), %zmm30	 # AVX512F
	vpsrld	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpsrld	$123, (%rcx){1to16}, %zmm30	 # AVX512F
	vpsrld	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vpsrld	$123, 8192(%rdx), %zmm30	 # AVX512F
	vpsrld	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vpsrld	$123, -8256(%rdx), %zmm30	 # AVX512F
	vpsrld	$123, 508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vpsrld	$123, 512(%rdx){1to16}, %zmm30	 # AVX512F
	vpsrld	$123, -512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vpsrld	$123, -516(%rdx){1to16}, %zmm30	 # AVX512F

	vpsrlq	$0xab, %zmm29, %zmm30	 # AVX512F
	vpsrlq	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrlq	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsrlq	$123, %zmm29, %zmm30	 # AVX512F
	vpsrlq	$123, (%rcx), %zmm30	 # AVX512F
	vpsrlq	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpsrlq	$123, (%rcx){1to8}, %zmm30	 # AVX512F
	vpsrlq	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vpsrlq	$123, 8192(%rdx), %zmm30	 # AVX512F
	vpsrlq	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vpsrlq	$123, -8256(%rdx), %zmm30	 # AVX512F
	vpsrlq	$123, 1016(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vpsrlq	$123, 1024(%rdx){1to8}, %zmm30	 # AVX512F
	vpsrlq	$123, -1024(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vpsrlq	$123, -1032(%rdx){1to8}, %zmm30	 # AVX512F

	vpsubd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpsubd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsubd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsubd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpsubd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpsubd	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpsubd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsubd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsubd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsubd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsubd	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsubd	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpsubd	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsubd	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpsubq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpsubq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsubq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsubq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpsubq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpsubq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpsubq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsubq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsubq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpsubq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpsubq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsubq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpsubq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpsubq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vptestmd	%zmm29, %zmm30, %k5	 # AVX512F
	vptestmd	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vptestmd	(%rcx), %zmm30, %k5	 # AVX512F
	vptestmd	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vptestmd	(%rcx){1to16}, %zmm30, %k5	 # AVX512F
	vptestmd	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vptestmd	8192(%rdx), %zmm30, %k5	 # AVX512F
	vptestmd	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vptestmd	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vptestmd	508(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vptestmd	512(%rdx){1to16}, %zmm30, %k5	 # AVX512F
	vptestmd	-512(%rdx){1to16}, %zmm30, %k5	 # AVX512F Disp8
	vptestmd	-516(%rdx){1to16}, %zmm30, %k5	 # AVX512F

	vptestmq	%zmm29, %zmm30, %k5	 # AVX512F
	vptestmq	%zmm29, %zmm30, %k5{%k7}	 # AVX512F
	vptestmq	(%rcx), %zmm30, %k5	 # AVX512F
	vptestmq	0x123(%rax,%r14,8), %zmm30, %k5	 # AVX512F
	vptestmq	(%rcx){1to8}, %zmm30, %k5	 # AVX512F
	vptestmq	8128(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vptestmq	8192(%rdx), %zmm30, %k5	 # AVX512F
	vptestmq	-8192(%rdx), %zmm30, %k5	 # AVX512F Disp8
	vptestmq	-8256(%rdx), %zmm30, %k5	 # AVX512F
	vptestmq	1016(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vptestmq	1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F
	vptestmq	-1024(%rdx){1to8}, %zmm30, %k5	 # AVX512F Disp8
	vptestmq	-1032(%rdx){1to8}, %zmm30, %k5	 # AVX512F

	vpunpckhdq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpunpckhdq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpunpckhdq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpunpckhdq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpunpckhdq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpunpckhdq	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpunpckhdq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpunpckhdq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpunpckhdq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpunpckhdq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpunpckhdq	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpunpckhdq	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpunpckhdq	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpunpckhdq	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpunpckhqdq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpunpckhqdq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpunpckhqdq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpunpckhqdq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpunpckhqdq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpunpckhqdq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpunpckhqdq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpunpckhqdq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpunpckhqdq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpunpckhqdq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpunpckhqdq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpunpckhqdq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpunpckhqdq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpunpckhqdq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpunpckldq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpunpckldq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpunpckldq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpunpckldq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpunpckldq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpunpckldq	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpunpckldq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpunpckldq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpunpckldq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpunpckldq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpunpckldq	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpunpckldq	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpunpckldq	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpunpckldq	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpunpcklqdq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpunpcklqdq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpunpcklqdq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpunpcklqdq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpunpcklqdq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpunpcklqdq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpunpcklqdq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpunpcklqdq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpunpcklqdq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpunpcklqdq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpunpcklqdq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpunpcklqdq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpunpcklqdq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpunpcklqdq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpxord	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpxord	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpxord	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpxord	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpxord	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpxord	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpxord	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpxord	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpxord	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpxord	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpxord	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpxord	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpxord	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpxord	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpxorq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpxorq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpxorq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpxorq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpxorq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpxorq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpxorq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpxorq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpxorq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpxorq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpxorq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpxorq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpxorq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpxorq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vrcp14pd	%zmm29, %zmm30	 # AVX512F
	vrcp14pd	%zmm29, %zmm30{%k7}	 # AVX512F
	vrcp14pd	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vrcp14pd	(%rcx), %zmm30	 # AVX512F
	vrcp14pd	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vrcp14pd	(%rcx){1to8}, %zmm30	 # AVX512F
	vrcp14pd	8128(%rdx), %zmm30	 # AVX512F Disp8
	vrcp14pd	8192(%rdx), %zmm30	 # AVX512F
	vrcp14pd	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vrcp14pd	-8256(%rdx), %zmm30	 # AVX512F
	vrcp14pd	1016(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vrcp14pd	1024(%rdx){1to8}, %zmm30	 # AVX512F
	vrcp14pd	-1024(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vrcp14pd	-1032(%rdx){1to8}, %zmm30	 # AVX512F

	vrcp14ps	%zmm29, %zmm30	 # AVX512F
	vrcp14ps	%zmm29, %zmm30{%k7}	 # AVX512F
	vrcp14ps	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vrcp14ps	(%rcx), %zmm30	 # AVX512F
	vrcp14ps	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vrcp14ps	(%rcx){1to16}, %zmm30	 # AVX512F
	vrcp14ps	8128(%rdx), %zmm30	 # AVX512F Disp8
	vrcp14ps	8192(%rdx), %zmm30	 # AVX512F
	vrcp14ps	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vrcp14ps	-8256(%rdx), %zmm30	 # AVX512F
	vrcp14ps	508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vrcp14ps	512(%rdx){1to16}, %zmm30	 # AVX512F
	vrcp14ps	-512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vrcp14ps	-516(%rdx){1to16}, %zmm30	 # AVX512F

	vrcp14sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vrcp14sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vrcp14sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vrcp14sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vrcp14sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vrcp14sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vrcp14sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vrcp14sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vrcp14ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vrcp14ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vrcp14ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vrcp14ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vrcp14ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vrcp14ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vrcp14ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vrcp14ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vrsqrt14pd	%zmm29, %zmm30	 # AVX512F
	vrsqrt14pd	%zmm29, %zmm30{%k7}	 # AVX512F
	vrsqrt14pd	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vrsqrt14pd	(%rcx), %zmm30	 # AVX512F
	vrsqrt14pd	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vrsqrt14pd	(%rcx){1to8}, %zmm30	 # AVX512F
	vrsqrt14pd	8128(%rdx), %zmm30	 # AVX512F Disp8
	vrsqrt14pd	8192(%rdx), %zmm30	 # AVX512F
	vrsqrt14pd	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vrsqrt14pd	-8256(%rdx), %zmm30	 # AVX512F
	vrsqrt14pd	1016(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vrsqrt14pd	1024(%rdx){1to8}, %zmm30	 # AVX512F
	vrsqrt14pd	-1024(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vrsqrt14pd	-1032(%rdx){1to8}, %zmm30	 # AVX512F

	vrsqrt14ps	%zmm29, %zmm30	 # AVX512F
	vrsqrt14ps	%zmm29, %zmm30{%k7}	 # AVX512F
	vrsqrt14ps	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vrsqrt14ps	(%rcx), %zmm30	 # AVX512F
	vrsqrt14ps	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vrsqrt14ps	(%rcx){1to16}, %zmm30	 # AVX512F
	vrsqrt14ps	8128(%rdx), %zmm30	 # AVX512F Disp8
	vrsqrt14ps	8192(%rdx), %zmm30	 # AVX512F
	vrsqrt14ps	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vrsqrt14ps	-8256(%rdx), %zmm30	 # AVX512F
	vrsqrt14ps	508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vrsqrt14ps	512(%rdx){1to16}, %zmm30	 # AVX512F
	vrsqrt14ps	-512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vrsqrt14ps	-516(%rdx){1to16}, %zmm30	 # AVX512F

	vrsqrt14sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vrsqrt14sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vrsqrt14sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vrsqrt14sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vrsqrt14sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vrsqrt14sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vrsqrt14sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vrsqrt14sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vrsqrt14ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vrsqrt14ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vrsqrt14ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vrsqrt14ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vrsqrt14ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vrsqrt14ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vrsqrt14ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vrsqrt14ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vscatterdpd	%zmm30, 123(%r14,%ymm31,8){%k1}	 # AVX512F
	vscatterdpd	%zmm30, 123(%r14,%ymm31,8){%k1}	 # AVX512F
	vscatterdpd	%zmm30, 256(%r9,%ymm31){%k1}	 # AVX512F
	vscatterdpd	%zmm30, 1024(%rcx,%ymm31,4){%k1}	 # AVX512F

	vscatterdps	%zmm30, 123(%r14,%zmm31,8){%k1}	 # AVX512F
	vscatterdps	%zmm30, 123(%r14,%zmm31,8){%k1}	 # AVX512F
	vscatterdps	%zmm30, 256(%r9,%zmm31){%k1}	 # AVX512F
	vscatterdps	%zmm30, 1024(%rcx,%zmm31,4){%k1}	 # AVX512F

	vscatterqpd	%zmm30, 123(%r14,%zmm31,8){%k1}	 # AVX512F
	vscatterqpd	%zmm30, 123(%r14,%zmm31,8){%k1}	 # AVX512F
	vscatterqpd	%zmm30, 256(%r9,%zmm31){%k1}	 # AVX512F
	vscatterqpd	%zmm30, 1024(%rcx,%zmm31,4){%k1}	 # AVX512F

	vscatterqps	%ymm30, 123(%r14,%zmm31,8){%k1}	 # AVX512F
	vscatterqps	%ymm30, 123(%r14,%zmm31,8){%k1}	 # AVX512F
	vscatterqps	%ymm30, 256(%r9,%zmm31){%k1}	 # AVX512F
	vscatterqps	%ymm30, 1024(%rcx,%zmm31,4){%k1}	 # AVX512F

	vshufpd	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512F
	vshufpd	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vshufpd	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vshufpd	$123, %zmm28, %zmm29, %zmm30	 # AVX512F
	vshufpd	$123, (%rcx), %zmm29, %zmm30	 # AVX512F
	vshufpd	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vshufpd	$123, (%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vshufpd	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vshufpd	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vshufpd	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vshufpd	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vshufpd	$123, 1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vshufpd	$123, 1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vshufpd	$123, -1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vshufpd	$123, -1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vshufps	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512F
	vshufps	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vshufps	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vshufps	$123, %zmm28, %zmm29, %zmm30	 # AVX512F
	vshufps	$123, (%rcx), %zmm29, %zmm30	 # AVX512F
	vshufps	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vshufps	$123, (%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vshufps	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vshufps	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vshufps	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vshufps	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vshufps	$123, 508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vshufps	$123, 512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vshufps	$123, -512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vshufps	$123, -516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vsqrtpd	%zmm29, %zmm30	 # AVX512F
	vsqrtpd	%zmm29, %zmm30{%k7}	 # AVX512F
	vsqrtpd	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vsqrtpd	{rn-sae}, %zmm29, %zmm30	 # AVX512F
	vsqrtpd	{ru-sae}, %zmm29, %zmm30	 # AVX512F
	vsqrtpd	{rd-sae}, %zmm29, %zmm30	 # AVX512F
	vsqrtpd	{rz-sae}, %zmm29, %zmm30	 # AVX512F
	vsqrtpd	(%rcx), %zmm30	 # AVX512F
	vsqrtpd	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vsqrtpd	(%rcx){1to8}, %zmm30	 # AVX512F
	vsqrtpd	8128(%rdx), %zmm30	 # AVX512F Disp8
	vsqrtpd	8192(%rdx), %zmm30	 # AVX512F
	vsqrtpd	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vsqrtpd	-8256(%rdx), %zmm30	 # AVX512F
	vsqrtpd	1016(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vsqrtpd	1024(%rdx){1to8}, %zmm30	 # AVX512F
	vsqrtpd	-1024(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vsqrtpd	-1032(%rdx){1to8}, %zmm30	 # AVX512F

	vsqrtps	%zmm29, %zmm30	 # AVX512F
	vsqrtps	%zmm29, %zmm30{%k7}	 # AVX512F
	vsqrtps	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vsqrtps	{rn-sae}, %zmm29, %zmm30	 # AVX512F
	vsqrtps	{ru-sae}, %zmm29, %zmm30	 # AVX512F
	vsqrtps	{rd-sae}, %zmm29, %zmm30	 # AVX512F
	vsqrtps	{rz-sae}, %zmm29, %zmm30	 # AVX512F
	vsqrtps	(%rcx), %zmm30	 # AVX512F
	vsqrtps	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vsqrtps	(%rcx){1to16}, %zmm30	 # AVX512F
	vsqrtps	8128(%rdx), %zmm30	 # AVX512F Disp8
	vsqrtps	8192(%rdx), %zmm30	 # AVX512F
	vsqrtps	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vsqrtps	-8256(%rdx), %zmm30	 # AVX512F
	vsqrtps	508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vsqrtps	512(%rdx){1to16}, %zmm30	 # AVX512F
	vsqrtps	-512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vsqrtps	-516(%rdx){1to16}, %zmm30	 # AVX512F

	vsqrtsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vsqrtsd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtsd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtsd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtsd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vsqrtsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vsqrtsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vsqrtss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vsqrtss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vsqrtss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vsqrtss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vsqrtss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vsubpd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vsubpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vsubpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vsubpd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vsubpd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vsubpd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vsubpd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vsubpd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vsubpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vsubpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vsubpd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vsubpd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vsubpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vsubpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vsubpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vsubpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vsubpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vsubpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vsubps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vsubps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vsubps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vsubps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vsubps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vsubps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vsubps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vsubps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vsubps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vsubps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vsubps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vsubps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vsubps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vsubps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vsubps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vsubps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vsubps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vsubps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vsubsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsubsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vsubsd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsubsd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsubsd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsubsd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsubsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vsubsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vsubsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vsubsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vsubsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vsubsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vsubss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsubss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vsubss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsubss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsubss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsubss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vsubss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vsubss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vsubss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vsubss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vsubss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vsubss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vucomisd	%xmm29, %xmm30	 # AVX512F
	vucomisd	{sae}, %xmm29, %xmm30	 # AVX512F
	vucomisd	(%rcx), %xmm30	 # AVX512F
	vucomisd	0x123(%rax,%r14,8), %xmm30	 # AVX512F
	vucomisd	1016(%rdx), %xmm30	 # AVX512F Disp8
	vucomisd	1024(%rdx), %xmm30	 # AVX512F
	vucomisd	-1024(%rdx), %xmm30	 # AVX512F Disp8
	vucomisd	-1032(%rdx), %xmm30	 # AVX512F

	vucomiss	%xmm29, %xmm30	 # AVX512F
	vucomiss	{sae}, %xmm29, %xmm30	 # AVX512F
	vucomiss	(%rcx), %xmm30	 # AVX512F
	vucomiss	0x123(%rax,%r14,8), %xmm30	 # AVX512F
	vucomiss	508(%rdx), %xmm30	 # AVX512F Disp8
	vucomiss	512(%rdx), %xmm30	 # AVX512F
	vucomiss	-512(%rdx), %xmm30	 # AVX512F Disp8
	vucomiss	-516(%rdx), %xmm30	 # AVX512F

	vunpckhpd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vunpckhpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vunpckhpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vunpckhpd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vunpckhpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vunpckhpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vunpckhpd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vunpckhpd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vunpckhpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vunpckhpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vunpckhpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vunpckhpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vunpckhpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vunpckhpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vunpckhps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vunpckhps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vunpckhps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vunpckhps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vunpckhps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vunpckhps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vunpckhps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vunpckhps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vunpckhps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vunpckhps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vunpckhps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vunpckhps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vunpckhps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vunpckhps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vunpcklpd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vunpcklpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vunpcklpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vunpcklpd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vunpcklpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vunpcklpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vunpcklpd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vunpcklpd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vunpcklpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vunpcklpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vunpcklpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vunpcklpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vunpcklpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vunpcklpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vunpcklps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vunpcklps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vunpcklps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vunpcklps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vunpcklps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vunpcklps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vunpcklps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vunpcklps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vunpcklps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vunpcklps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vunpcklps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vunpcklps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vunpcklps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vunpcklps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpternlogd	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512F
	vpternlogd	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpternlogd	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpternlogd	$123, %zmm28, %zmm29, %zmm30	 # AVX512F
	vpternlogd	$123, (%rcx), %zmm29, %zmm30	 # AVX512F
	vpternlogd	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpternlogd	$123, (%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpternlogd	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpternlogd	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpternlogd	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpternlogd	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpternlogd	$123, 508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpternlogd	$123, 512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpternlogd	$123, -512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpternlogd	$123, -516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpternlogq	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512F
	vpternlogq	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpternlogq	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpternlogq	$123, %zmm28, %zmm29, %zmm30	 # AVX512F
	vpternlogq	$123, (%rcx), %zmm29, %zmm30	 # AVX512F
	vpternlogq	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpternlogq	$123, (%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpternlogq	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpternlogq	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpternlogq	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpternlogq	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpternlogq	$123, 1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpternlogq	$123, 1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpternlogq	$123, -1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpternlogq	$123, -1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpmovqb	%zmm29, %xmm30{%k7}	 # AVX512F
	vpmovqb	%zmm29, %xmm30{%k7}{z}	 # AVX512F

	vpmovsqb	%zmm29, %xmm30{%k7}	 # AVX512F
	vpmovsqb	%zmm29, %xmm30{%k7}{z}	 # AVX512F

	vpmovusqb	%zmm29, %xmm30{%k7}	 # AVX512F
	vpmovusqb	%zmm29, %xmm30{%k7}{z}	 # AVX512F

	vpmovqw	%zmm29, %xmm30{%k7}	 # AVX512F
	vpmovqw	%zmm29, %xmm30{%k7}{z}	 # AVX512F

	vpmovsqw	%zmm29, %xmm30{%k7}	 # AVX512F
	vpmovsqw	%zmm29, %xmm30{%k7}{z}	 # AVX512F

	vpmovusqw	%zmm29, %xmm30{%k7}	 # AVX512F
	vpmovusqw	%zmm29, %xmm30{%k7}{z}	 # AVX512F

	vpmovqd	%zmm29, %ymm30{%k7}	 # AVX512F
	vpmovqd	%zmm29, %ymm30{%k7}{z}	 # AVX512F

	vpmovsqd	%zmm29, %ymm30{%k7}	 # AVX512F
	vpmovsqd	%zmm29, %ymm30{%k7}{z}	 # AVX512F

	vpmovusqd	%zmm29, %ymm30{%k7}	 # AVX512F
	vpmovusqd	%zmm29, %ymm30{%k7}{z}	 # AVX512F

	vpmovdb	%zmm29, %xmm30{%k7}	 # AVX512F
	vpmovdb	%zmm29, %xmm30{%k7}{z}	 # AVX512F

	vpmovsdb	%zmm29, %xmm30{%k7}	 # AVX512F
	vpmovsdb	%zmm29, %xmm30{%k7}{z}	 # AVX512F

	vpmovusdb	%zmm29, %xmm30{%k7}	 # AVX512F
	vpmovusdb	%zmm29, %xmm30{%k7}{z}	 # AVX512F

	vpmovdw	%zmm29, %ymm30{%k7}	 # AVX512F
	vpmovdw	%zmm29, %ymm30{%k7}{z}	 # AVX512F

	vpmovsdw	%zmm29, %ymm30{%k7}	 # AVX512F
	vpmovsdw	%zmm29, %ymm30{%k7}{z}	 # AVX512F

	vpmovusdw	%zmm29, %ymm30{%k7}	 # AVX512F
	vpmovusdw	%zmm29, %ymm30{%k7}{z}	 # AVX512F

	vshuff32x4	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512F
	vshuff32x4	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vshuff32x4	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vshuff32x4	$123, %zmm28, %zmm29, %zmm30	 # AVX512F
	vshuff32x4	$123, (%rcx), %zmm29, %zmm30	 # AVX512F
	vshuff32x4	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vshuff32x4	$123, (%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vshuff32x4	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vshuff32x4	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vshuff32x4	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vshuff32x4	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vshuff32x4	$123, 508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vshuff32x4	$123, 512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vshuff32x4	$123, -512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vshuff32x4	$123, -516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vshuff64x2	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512F
	vshuff64x2	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vshuff64x2	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vshuff64x2	$123, %zmm28, %zmm29, %zmm30	 # AVX512F
	vshuff64x2	$123, (%rcx), %zmm29, %zmm30	 # AVX512F
	vshuff64x2	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vshuff64x2	$123, (%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vshuff64x2	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vshuff64x2	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vshuff64x2	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vshuff64x2	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vshuff64x2	$123, 1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vshuff64x2	$123, 1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vshuff64x2	$123, -1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vshuff64x2	$123, -1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vshufi32x4	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512F
	vshufi32x4	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vshufi32x4	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vshufi32x4	$123, %zmm28, %zmm29, %zmm30	 # AVX512F
	vshufi32x4	$123, (%rcx), %zmm29, %zmm30	 # AVX512F
	vshufi32x4	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vshufi32x4	$123, (%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vshufi32x4	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vshufi32x4	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vshufi32x4	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vshufi32x4	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vshufi32x4	$123, 508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vshufi32x4	$123, 512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vshufi32x4	$123, -512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vshufi32x4	$123, -516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vshufi64x2	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512F
	vshufi64x2	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vshufi64x2	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vshufi64x2	$123, %zmm28, %zmm29, %zmm30	 # AVX512F
	vshufi64x2	$123, (%rcx), %zmm29, %zmm30	 # AVX512F
	vshufi64x2	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vshufi64x2	$123, (%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vshufi64x2	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vshufi64x2	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vshufi64x2	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vshufi64x2	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vshufi64x2	$123, 1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vshufi64x2	$123, 1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vshufi64x2	$123, -1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vshufi64x2	$123, -1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpermq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpermq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpermq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpermq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpermq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpermq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpermpd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpermpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermpd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpermpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpermpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpermpd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermpd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpermpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpermt2d	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpermt2d	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermt2d	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermt2d	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpermt2d	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpermt2d	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpermt2d	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2d	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermt2d	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2d	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermt2d	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2d	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpermt2d	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2d	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpermt2q	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpermt2q	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermt2q	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermt2q	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpermt2q	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpermt2q	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpermt2q	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2q	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermt2q	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2q	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermt2q	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2q	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpermt2q	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2q	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpermt2ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpermt2ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermt2ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermt2ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpermt2ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpermt2ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpermt2ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermt2ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermt2ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpermt2ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpermt2pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpermt2pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermt2pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermt2pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpermt2pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpermt2pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpermt2pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermt2pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermt2pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpermt2pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermt2pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	valignq	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512F
	valignq	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	valignq	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	valignq	$123, %zmm28, %zmm29, %zmm30	 # AVX512F
	valignq	$123, (%rcx), %zmm29, %zmm30	 # AVX512F
	valignq	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	valignq	$123, (%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	valignq	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	valignq	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512F
	valignq	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	valignq	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512F
	valignq	$123, 1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	valignq	$123, 1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	valignq	$123, -1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	valignq	$123, -1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vcvtsd2usi	%xmm30, %eax	 # AVX512F
	vcvtsd2usi	{rn-sae}, %xmm30, %eax	 # AVX512F
	vcvtsd2usi	{ru-sae}, %xmm30, %eax	 # AVX512F
	vcvtsd2usi	{rd-sae}, %xmm30, %eax	 # AVX512F
	vcvtsd2usi	{rz-sae}, %xmm30, %eax	 # AVX512F
	vcvtsd2usi	(%rcx), %eax	 # AVX512F
	vcvtsd2usi	0x123(%rax,%r14,8), %eax	 # AVX512F
	vcvtsd2usi	1016(%rdx), %eax	 # AVX512F Disp8
	vcvtsd2usi	1024(%rdx), %eax	 # AVX512F
	vcvtsd2usi	-1024(%rdx), %eax	 # AVX512F Disp8
	vcvtsd2usi	-1032(%rdx), %eax	 # AVX512F
	vcvtsd2usi	%xmm30, %ebp	 # AVX512F
	vcvtsd2usi	{rn-sae}, %xmm30, %ebp	 # AVX512F
	vcvtsd2usi	{ru-sae}, %xmm30, %ebp	 # AVX512F
	vcvtsd2usi	{rd-sae}, %xmm30, %ebp	 # AVX512F
	vcvtsd2usi	{rz-sae}, %xmm30, %ebp	 # AVX512F
	vcvtsd2usi	(%rcx), %ebp	 # AVX512F
	vcvtsd2usi	0x123(%rax,%r14,8), %ebp	 # AVX512F
	vcvtsd2usi	1016(%rdx), %ebp	 # AVX512F Disp8
	vcvtsd2usi	1024(%rdx), %ebp	 # AVX512F
	vcvtsd2usi	-1024(%rdx), %ebp	 # AVX512F Disp8
	vcvtsd2usi	-1032(%rdx), %ebp	 # AVX512F
	vcvtsd2usi	%xmm30, %r13d	 # AVX512F
	vcvtsd2usi	{rn-sae}, %xmm30, %r13d	 # AVX512F
	vcvtsd2usi	{ru-sae}, %xmm30, %r13d	 # AVX512F
	vcvtsd2usi	{rd-sae}, %xmm30, %r13d	 # AVX512F
	vcvtsd2usi	{rz-sae}, %xmm30, %r13d	 # AVX512F
	vcvtsd2usi	(%rcx), %r13d	 # AVX512F
	vcvtsd2usi	0x123(%rax,%r14,8), %r13d	 # AVX512F
	vcvtsd2usi	1016(%rdx), %r13d	 # AVX512F Disp8
	vcvtsd2usi	1024(%rdx), %r13d	 # AVX512F
	vcvtsd2usi	-1024(%rdx), %r13d	 # AVX512F Disp8
	vcvtsd2usi	-1032(%rdx), %r13d	 # AVX512F

	vcvtsd2usi	%xmm30, %rax	 # AVX512F
	vcvtsd2usi	{rn-sae}, %xmm30, %rax	 # AVX512F
	vcvtsd2usi	{ru-sae}, %xmm30, %rax	 # AVX512F
	vcvtsd2usi	{rd-sae}, %xmm30, %rax	 # AVX512F
	vcvtsd2usi	{rz-sae}, %xmm30, %rax	 # AVX512F
	vcvtsd2usi	(%rcx), %rax	 # AVX512F
	vcvtsd2usi	0x123(%rax,%r14,8), %rax	 # AVX512F
	vcvtsd2usi	1016(%rdx), %rax	 # AVX512F Disp8
	vcvtsd2usi	1024(%rdx), %rax	 # AVX512F
	vcvtsd2usi	-1024(%rdx), %rax	 # AVX512F Disp8
	vcvtsd2usi	-1032(%rdx), %rax	 # AVX512F
	vcvtsd2usi	%xmm30, %r8	 # AVX512F
	vcvtsd2usi	{rn-sae}, %xmm30, %r8	 # AVX512F
	vcvtsd2usi	{ru-sae}, %xmm30, %r8	 # AVX512F
	vcvtsd2usi	{rd-sae}, %xmm30, %r8	 # AVX512F
	vcvtsd2usi	{rz-sae}, %xmm30, %r8	 # AVX512F
	vcvtsd2usi	(%rcx), %r8	 # AVX512F
	vcvtsd2usi	0x123(%rax,%r14,8), %r8	 # AVX512F
	vcvtsd2usi	1016(%rdx), %r8	 # AVX512F Disp8
	vcvtsd2usi	1024(%rdx), %r8	 # AVX512F
	vcvtsd2usi	-1024(%rdx), %r8	 # AVX512F Disp8
	vcvtsd2usi	-1032(%rdx), %r8	 # AVX512F

	vcvtss2usi	%xmm30, %eax	 # AVX512F
	vcvtss2usi	{rn-sae}, %xmm30, %eax	 # AVX512F
	vcvtss2usi	{ru-sae}, %xmm30, %eax	 # AVX512F
	vcvtss2usi	{rd-sae}, %xmm30, %eax	 # AVX512F
	vcvtss2usi	{rz-sae}, %xmm30, %eax	 # AVX512F
	vcvtss2usi	(%rcx), %eax	 # AVX512F
	vcvtss2usi	0x123(%rax,%r14,8), %eax	 # AVX512F
	vcvtss2usi	508(%rdx), %eax	 # AVX512F Disp8
	vcvtss2usi	512(%rdx), %eax	 # AVX512F
	vcvtss2usi	-512(%rdx), %eax	 # AVX512F Disp8
	vcvtss2usi	-516(%rdx), %eax	 # AVX512F
	vcvtss2usi	%xmm30, %ebp	 # AVX512F
	vcvtss2usi	{rn-sae}, %xmm30, %ebp	 # AVX512F
	vcvtss2usi	{ru-sae}, %xmm30, %ebp	 # AVX512F
	vcvtss2usi	{rd-sae}, %xmm30, %ebp	 # AVX512F
	vcvtss2usi	{rz-sae}, %xmm30, %ebp	 # AVX512F
	vcvtss2usi	(%rcx), %ebp	 # AVX512F
	vcvtss2usi	0x123(%rax,%r14,8), %ebp	 # AVX512F
	vcvtss2usi	508(%rdx), %ebp	 # AVX512F Disp8
	vcvtss2usi	512(%rdx), %ebp	 # AVX512F
	vcvtss2usi	-512(%rdx), %ebp	 # AVX512F Disp8
	vcvtss2usi	-516(%rdx), %ebp	 # AVX512F
	vcvtss2usi	%xmm30, %r13d	 # AVX512F
	vcvtss2usi	{rn-sae}, %xmm30, %r13d	 # AVX512F
	vcvtss2usi	{ru-sae}, %xmm30, %r13d	 # AVX512F
	vcvtss2usi	{rd-sae}, %xmm30, %r13d	 # AVX512F
	vcvtss2usi	{rz-sae}, %xmm30, %r13d	 # AVX512F
	vcvtss2usi	(%rcx), %r13d	 # AVX512F
	vcvtss2usi	0x123(%rax,%r14,8), %r13d	 # AVX512F
	vcvtss2usi	508(%rdx), %r13d	 # AVX512F Disp8
	vcvtss2usi	512(%rdx), %r13d	 # AVX512F
	vcvtss2usi	-512(%rdx), %r13d	 # AVX512F Disp8
	vcvtss2usi	-516(%rdx), %r13d	 # AVX512F

	vcvtss2usi	%xmm30, %rax	 # AVX512F
	vcvtss2usi	{rn-sae}, %xmm30, %rax	 # AVX512F
	vcvtss2usi	{ru-sae}, %xmm30, %rax	 # AVX512F
	vcvtss2usi	{rd-sae}, %xmm30, %rax	 # AVX512F
	vcvtss2usi	{rz-sae}, %xmm30, %rax	 # AVX512F
	vcvtss2usi	(%rcx), %rax	 # AVX512F
	vcvtss2usi	0x123(%rax,%r14,8), %rax	 # AVX512F
	vcvtss2usi	508(%rdx), %rax	 # AVX512F Disp8
	vcvtss2usi	512(%rdx), %rax	 # AVX512F
	vcvtss2usi	-512(%rdx), %rax	 # AVX512F Disp8
	vcvtss2usi	-516(%rdx), %rax	 # AVX512F
	vcvtss2usi	%xmm30, %r8	 # AVX512F
	vcvtss2usi	{rn-sae}, %xmm30, %r8	 # AVX512F
	vcvtss2usi	{ru-sae}, %xmm30, %r8	 # AVX512F
	vcvtss2usi	{rd-sae}, %xmm30, %r8	 # AVX512F
	vcvtss2usi	{rz-sae}, %xmm30, %r8	 # AVX512F
	vcvtss2usi	(%rcx), %r8	 # AVX512F
	vcvtss2usi	0x123(%rax,%r14,8), %r8	 # AVX512F
	vcvtss2usi	508(%rdx), %r8	 # AVX512F Disp8
	vcvtss2usi	512(%rdx), %r8	 # AVX512F
	vcvtss2usi	-512(%rdx), %r8	 # AVX512F Disp8
	vcvtss2usi	-516(%rdx), %r8	 # AVX512F

	vcvtusi2sdl	%eax, %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdl	%ebp, %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdl	%r13d, %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdl	(%rcx), %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdl	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdl	508(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtusi2sdl	512(%rdx), %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdl	-512(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtusi2sdl	-516(%rdx), %xmm29, %xmm30	 # AVX512F

	vcvtusi2sdq	%rax, %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdq	%rax, {rn-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdq	%rax, {ru-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdq	%rax, {rd-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdq	%rax, {rz-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdq	%r8, %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdq	%r8, {rn-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdq	%r8, {ru-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdq	%r8, {rd-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdq	%r8, {rz-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdq	(%rcx), %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdq	1016(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtusi2sdq	1024(%rdx), %xmm29, %xmm30	 # AVX512F
	vcvtusi2sdq	-1024(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtusi2sdq	-1032(%rdx), %xmm29, %xmm30	 # AVX512F

	vcvtusi2ssl	%eax, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	%eax, {rn-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	%eax, {ru-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	%eax, {rd-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	%eax, {rz-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	%ebp, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	%ebp, {rn-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	%ebp, {ru-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	%ebp, {rd-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	%ebp, {rz-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	%r13d, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	%r13d, {rn-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	%r13d, {ru-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	%r13d, {rd-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	%r13d, {rz-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	(%rcx), %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	508(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtusi2ssl	512(%rdx), %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssl	-512(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtusi2ssl	-516(%rdx), %xmm29, %xmm30	 # AVX512F

	vcvtusi2ssq	%rax, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssq	%rax, {rn-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssq	%rax, {ru-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssq	%rax, {rd-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssq	%rax, {rz-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssq	%r8, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssq	%r8, {rn-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssq	%r8, {ru-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssq	%r8, {rd-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssq	%r8, {rz-sae}, %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssq	(%rcx), %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssq	1016(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtusi2ssq	1024(%rdx), %xmm29, %xmm30	 # AVX512F
	vcvtusi2ssq	-1024(%rdx), %xmm29, %xmm30	 # AVX512F Disp8
	vcvtusi2ssq	-1032(%rdx), %xmm29, %xmm30	 # AVX512F

	vscalefpd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vscalefpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vscalefpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vscalefpd	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vscalefpd	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vscalefpd	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vscalefpd	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vscalefpd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vscalefpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vscalefpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vscalefpd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vscalefpd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vscalefpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vscalefpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vscalefpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vscalefpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vscalefpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vscalefpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vscalefps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vscalefps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vscalefps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vscalefps	{rn-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vscalefps	{ru-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vscalefps	{rd-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vscalefps	{rz-sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vscalefps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vscalefps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vscalefps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vscalefps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vscalefps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vscalefps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vscalefps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vscalefps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vscalefps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vscalefps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vscalefps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vscalefsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vscalefsd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefsd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefsd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefsd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vscalefsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vscalefsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vscalefss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vscalefss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vscalefss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vscalefss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vscalefss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfixupimmps	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfixupimmps	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfixupimmps	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfixupimmps	$0xab, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfixupimmps	$123, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfixupimmps	$123, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfixupimmps	$123, (%rcx), %zmm29, %zmm30	 # AVX512F
	vfixupimmps	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfixupimmps	$123, (%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfixupimmps	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfixupimmps	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfixupimmps	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfixupimmps	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfixupimmps	$123, 508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfixupimmps	$123, 512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vfixupimmps	$123, -512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vfixupimmps	$123, -516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vfixupimmpd	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfixupimmpd	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vfixupimmpd	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vfixupimmpd	$0xab, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfixupimmpd	$123, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfixupimmpd	$123, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfixupimmpd	$123, (%rcx), %zmm29, %zmm30	 # AVX512F
	vfixupimmpd	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vfixupimmpd	$123, (%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfixupimmpd	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfixupimmpd	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vfixupimmpd	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vfixupimmpd	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vfixupimmpd	$123, 1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfixupimmpd	$123, 1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vfixupimmpd	$123, -1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vfixupimmpd	$123, -1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vfixupimmss	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfixupimmss	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfixupimmss	$0xab, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfixupimmss	$123, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfixupimmss	$123, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfixupimmss	$123, (%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfixupimmss	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfixupimmss	$123, 508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfixupimmss	$123, 512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfixupimmss	$123, -512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfixupimmss	$123, -516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vfixupimmsd	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfixupimmsd	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vfixupimmsd	$0xab, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfixupimmsd	$123, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfixupimmsd	$123, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vfixupimmsd	$123, (%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfixupimmsd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vfixupimmsd	$123, 1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfixupimmsd	$123, 1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vfixupimmsd	$123, -1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vfixupimmsd	$123, -1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vpslld	$0xab, %zmm29, %zmm30	 # AVX512F
	vpslld	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vpslld	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpslld	$123, %zmm29, %zmm30	 # AVX512F
	vpslld	$123, (%rcx), %zmm30	 # AVX512F
	vpslld	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpslld	$123, (%rcx){1to16}, %zmm30	 # AVX512F
	vpslld	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vpslld	$123, 8192(%rdx), %zmm30	 # AVX512F
	vpslld	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vpslld	$123, -8256(%rdx), %zmm30	 # AVX512F
	vpslld	$123, 508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vpslld	$123, 512(%rdx){1to16}, %zmm30	 # AVX512F
	vpslld	$123, -512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vpslld	$123, -516(%rdx){1to16}, %zmm30	 # AVX512F

	vpsllq	$0xab, %zmm29, %zmm30	 # AVX512F
	vpsllq	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsllq	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsllq	$123, %zmm29, %zmm30	 # AVX512F
	vpsllq	$123, (%rcx), %zmm30	 # AVX512F
	vpsllq	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpsllq	$123, (%rcx){1to8}, %zmm30	 # AVX512F
	vpsllq	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vpsllq	$123, 8192(%rdx), %zmm30	 # AVX512F
	vpsllq	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vpsllq	$123, -8256(%rdx), %zmm30	 # AVX512F
	vpsllq	$123, 1016(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vpsllq	$123, 1024(%rdx){1to8}, %zmm30	 # AVX512F
	vpsllq	$123, -1024(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vpsllq	$123, -1032(%rdx){1to8}, %zmm30	 # AVX512F

	vpsrad	$0xab, %zmm29, %zmm30	 # AVX512F
	vpsrad	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsrad	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsrad	$123, %zmm29, %zmm30	 # AVX512F
	vpsrad	$123, (%rcx), %zmm30	 # AVX512F
	vpsrad	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpsrad	$123, (%rcx){1to16}, %zmm30	 # AVX512F
	vpsrad	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vpsrad	$123, 8192(%rdx), %zmm30	 # AVX512F
	vpsrad	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vpsrad	$123, -8256(%rdx), %zmm30	 # AVX512F
	vpsrad	$123, 508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vpsrad	$123, 512(%rdx){1to16}, %zmm30	 # AVX512F
	vpsrad	$123, -512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vpsrad	$123, -516(%rdx){1to16}, %zmm30	 # AVX512F

	vpsraq	$0xab, %zmm29, %zmm30	 # AVX512F
	vpsraq	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vpsraq	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpsraq	$123, %zmm29, %zmm30	 # AVX512F
	vpsraq	$123, (%rcx), %zmm30	 # AVX512F
	vpsraq	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vpsraq	$123, (%rcx){1to8}, %zmm30	 # AVX512F
	vpsraq	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vpsraq	$123, 8192(%rdx), %zmm30	 # AVX512F
	vpsraq	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vpsraq	$123, -8256(%rdx), %zmm30	 # AVX512F
	vpsraq	$123, 1016(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vpsraq	$123, 1024(%rdx){1to8}, %zmm30	 # AVX512F
	vpsraq	$123, -1024(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vpsraq	$123, -1032(%rdx){1to8}, %zmm30	 # AVX512F

	vprolvd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vprolvd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vprolvd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vprolvd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vprolvd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vprolvd	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vprolvd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vprolvd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vprolvd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vprolvd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vprolvd	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vprolvd	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vprolvd	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vprolvd	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vprold	$0xab, %zmm29, %zmm30	 # AVX512F
	vprold	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vprold	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vprold	$123, %zmm29, %zmm30	 # AVX512F
	vprold	$123, (%rcx), %zmm30	 # AVX512F
	vprold	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vprold	$123, (%rcx){1to16}, %zmm30	 # AVX512F
	vprold	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vprold	$123, 8192(%rdx), %zmm30	 # AVX512F
	vprold	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vprold	$123, -8256(%rdx), %zmm30	 # AVX512F
	vprold	$123, 508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vprold	$123, 512(%rdx){1to16}, %zmm30	 # AVX512F
	vprold	$123, -512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vprold	$123, -516(%rdx){1to16}, %zmm30	 # AVX512F

	vprolvq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vprolvq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vprolvq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vprolvq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vprolvq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vprolvq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vprolvq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vprolvq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vprolvq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vprolvq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vprolvq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vprolvq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vprolvq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vprolvq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vprolq	$0xab, %zmm29, %zmm30	 # AVX512F
	vprolq	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vprolq	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vprolq	$123, %zmm29, %zmm30	 # AVX512F
	vprolq	$123, (%rcx), %zmm30	 # AVX512F
	vprolq	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vprolq	$123, (%rcx){1to8}, %zmm30	 # AVX512F
	vprolq	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vprolq	$123, 8192(%rdx), %zmm30	 # AVX512F
	vprolq	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vprolq	$123, -8256(%rdx), %zmm30	 # AVX512F
	vprolq	$123, 1016(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vprolq	$123, 1024(%rdx){1to8}, %zmm30	 # AVX512F
	vprolq	$123, -1024(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vprolq	$123, -1032(%rdx){1to8}, %zmm30	 # AVX512F

	vprorvd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vprorvd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vprorvd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vprorvd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vprorvd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vprorvd	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vprorvd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vprorvd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vprorvd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vprorvd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vprorvd	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vprorvd	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vprorvd	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vprorvd	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vprord	$0xab, %zmm29, %zmm30	 # AVX512F
	vprord	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vprord	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vprord	$123, %zmm29, %zmm30	 # AVX512F
	vprord	$123, (%rcx), %zmm30	 # AVX512F
	vprord	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vprord	$123, (%rcx){1to16}, %zmm30	 # AVX512F
	vprord	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vprord	$123, 8192(%rdx), %zmm30	 # AVX512F
	vprord	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vprord	$123, -8256(%rdx), %zmm30	 # AVX512F
	vprord	$123, 508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vprord	$123, 512(%rdx){1to16}, %zmm30	 # AVX512F
	vprord	$123, -512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vprord	$123, -516(%rdx){1to16}, %zmm30	 # AVX512F

	vprorvq	%zmm28, %zmm29, %zmm30	 # AVX512F
	vprorvq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vprorvq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vprorvq	(%rcx), %zmm29, %zmm30	 # AVX512F
	vprorvq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vprorvq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vprorvq	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vprorvq	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vprorvq	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vprorvq	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vprorvq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vprorvq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vprorvq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vprorvq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vprorq	$0xab, %zmm29, %zmm30	 # AVX512F
	vprorq	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vprorq	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vprorq	$123, %zmm29, %zmm30	 # AVX512F
	vprorq	$123, (%rcx), %zmm30	 # AVX512F
	vprorq	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vprorq	$123, (%rcx){1to8}, %zmm30	 # AVX512F
	vprorq	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vprorq	$123, 8192(%rdx), %zmm30	 # AVX512F
	vprorq	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vprorq	$123, -8256(%rdx), %zmm30	 # AVX512F
	vprorq	$123, 1016(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vprorq	$123, 1024(%rdx){1to8}, %zmm30	 # AVX512F
	vprorq	$123, -1024(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vprorq	$123, -1032(%rdx){1to8}, %zmm30	 # AVX512F

	vrndscalepd	$0xab, %zmm29, %zmm30	 # AVX512F
	vrndscalepd	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vrndscalepd	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vrndscalepd	$0xab, {sae}, %zmm29, %zmm30	 # AVX512F
	vrndscalepd	$123, %zmm29, %zmm30	 # AVX512F
	vrndscalepd	$123, {sae}, %zmm29, %zmm30	 # AVX512F
	vrndscalepd	$123, (%rcx), %zmm30	 # AVX512F
	vrndscalepd	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vrndscalepd	$123, (%rcx){1to8}, %zmm30	 # AVX512F
	vrndscalepd	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vrndscalepd	$123, 8192(%rdx), %zmm30	 # AVX512F
	vrndscalepd	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vrndscalepd	$123, -8256(%rdx), %zmm30	 # AVX512F
	vrndscalepd	$123, 1016(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vrndscalepd	$123, 1024(%rdx){1to8}, %zmm30	 # AVX512F
	vrndscalepd	$123, -1024(%rdx){1to8}, %zmm30	 # AVX512F Disp8
	vrndscalepd	$123, -1032(%rdx){1to8}, %zmm30	 # AVX512F

	vrndscaleps	$0xab, %zmm29, %zmm30	 # AVX512F
	vrndscaleps	$0xab, %zmm29, %zmm30{%k7}	 # AVX512F
	vrndscaleps	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vrndscaleps	$0xab, {sae}, %zmm29, %zmm30	 # AVX512F
	vrndscaleps	$123, %zmm29, %zmm30	 # AVX512F
	vrndscaleps	$123, {sae}, %zmm29, %zmm30	 # AVX512F
	vrndscaleps	$123, (%rcx), %zmm30	 # AVX512F
	vrndscaleps	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vrndscaleps	$123, (%rcx){1to16}, %zmm30	 # AVX512F
	vrndscaleps	$123, 8128(%rdx), %zmm30	 # AVX512F Disp8
	vrndscaleps	$123, 8192(%rdx), %zmm30	 # AVX512F
	vrndscaleps	$123, -8192(%rdx), %zmm30	 # AVX512F Disp8
	vrndscaleps	$123, -8256(%rdx), %zmm30	 # AVX512F
	vrndscaleps	$123, 508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vrndscaleps	$123, 512(%rdx){1to16}, %zmm30	 # AVX512F
	vrndscaleps	$123, -512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vrndscaleps	$123, -516(%rdx){1to16}, %zmm30	 # AVX512F

	vrndscalesd	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vrndscalesd	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vrndscalesd	$0xab, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vrndscalesd	$123, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vrndscalesd	$123, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vrndscalesd	$123, (%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vrndscalesd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vrndscalesd	$123, 1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vrndscalesd	$123, 1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vrndscalesd	$123, -1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vrndscalesd	$123, -1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vrndscaless	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vrndscaless	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vrndscaless	$0xab, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vrndscaless	$123, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vrndscaless	$123, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vrndscaless	$123, (%rcx), %xmm29, %xmm30{%k7}	 # AVX512F
	vrndscaless	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512F
	vrndscaless	$123, 508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vrndscaless	$123, 512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F
	vrndscaless	$123, -512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F Disp8
	vrndscaless	$123, -516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512F

	vpcompressq	%zmm30, (%rcx)	 # AVX512F
	vpcompressq	%zmm30, (%rcx){%k7}	 # AVX512F
	vpcompressq	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpcompressq	%zmm30, 1016(%rdx)	 # AVX512F Disp8
	vpcompressq	%zmm30, 1024(%rdx)	 # AVX512F
	vpcompressq	%zmm30, -1024(%rdx)	 # AVX512F Disp8
	vpcompressq	%zmm30, -1032(%rdx)	 # AVX512F

	vpcompressq	%zmm29, %zmm30	 # AVX512F
	vpcompressq	%zmm29, %zmm30{%k7}	 # AVX512F
	vpcompressq	%zmm29, %zmm30{%k7}{z}	 # AVX512F

	kandw	%k7, %k6, %k5	 # AVX512F

	kandnw	%k7, %k6, %k5	 # AVX512F

	korw	%k7, %k6, %k5	 # AVX512F

	kxnorw	%k7, %k6, %k5	 # AVX512F

	kxorw	%k7, %k6, %k5	 # AVX512F

	knotw	%k6, %k5	 # AVX512F

	kortestw	%k6, %k5	 # AVX512F

	kshiftrw	$0xab, %k6, %k5	 # AVX512F
	kshiftrw	$123, %k6, %k5	 # AVX512F

	kshiftlw	$0xab, %k6, %k5	 # AVX512F
	kshiftlw	$123, %k6, %k5	 # AVX512F

	kmovw	%k6, %k5	 # AVX512F
	kmovw	(%rcx), %k5	 # AVX512F
	kmovw	0x123(%rax,%r14,8), %k5	 # AVX512F

	kmovw	%k5, (%rcx)	 # AVX512F
	kmovw	%k5, 0x123(%rax,%r14,8)	 # AVX512F

	kmovw	%eax, %k5	 # AVX512F
	kmovw	%ebp, %k5	 # AVX512F
	kmovw	%r13d, %k5	 # AVX512F

	kmovw	%k5, %eax	 # AVX512F
	kmovw	%k5, %ebp	 # AVX512F
	kmovw	%k5, %r13d	 # AVX512F

	kunpckbw	%k7, %k6, %k5	 # AVX512F

	vcvtps2ph	$0xab, %zmm30, (%rcx)	 # AVX512F
	vcvtps2ph	$0xab, %zmm30, (%rcx){%k7}	 # AVX512F
	vcvtps2ph	$123, %zmm30, (%rcx)	 # AVX512F
	vcvtps2ph	$123, %zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vcvtps2ph	$123, %zmm30, 4064(%rdx)	 # AVX512F Disp8
	vcvtps2ph	$123, %zmm30, 4096(%rdx)	 # AVX512F
	vcvtps2ph	$123, %zmm30, -4096(%rdx)	 # AVX512F Disp8
	vcvtps2ph	$123, %zmm30, -4128(%rdx)	 # AVX512F

	vextractf32x4	$0xab, %zmm30, (%rcx)	 # AVX512F
	vextractf32x4	$0xab, %zmm30, (%rcx){%k7}	 # AVX512F
	vextractf32x4	$123, %zmm30, (%rcx)	 # AVX512F
	vextractf32x4	$123, %zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vextractf32x4	$123, %zmm30, 2032(%rdx)	 # AVX512F Disp8
	vextractf32x4	$123, %zmm30, 2048(%rdx)	 # AVX512F
	vextractf32x4	$123, %zmm30, -2048(%rdx)	 # AVX512F Disp8
	vextractf32x4	$123, %zmm30, -2064(%rdx)	 # AVX512F

	vextractf64x4	$0xab, %zmm30, (%rcx)	 # AVX512F
	vextractf64x4	$0xab, %zmm30, (%rcx){%k7}	 # AVX512F
	vextractf64x4	$123, %zmm30, (%rcx)	 # AVX512F
	vextractf64x4	$123, %zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vextractf64x4	$123, %zmm30, 4064(%rdx)	 # AVX512F Disp8
	vextractf64x4	$123, %zmm30, 4096(%rdx)	 # AVX512F
	vextractf64x4	$123, %zmm30, -4096(%rdx)	 # AVX512F Disp8
	vextractf64x4	$123, %zmm30, -4128(%rdx)	 # AVX512F

	vextracti32x4	$0xab, %zmm30, (%rcx)	 # AVX512F
	vextracti32x4	$0xab, %zmm30, (%rcx){%k7}	 # AVX512F
	vextracti32x4	$123, %zmm30, (%rcx)	 # AVX512F
	vextracti32x4	$123, %zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vextracti32x4	$123, %zmm30, 2032(%rdx)	 # AVX512F Disp8
	vextracti32x4	$123, %zmm30, 2048(%rdx)	 # AVX512F
	vextracti32x4	$123, %zmm30, -2048(%rdx)	 # AVX512F Disp8
	vextracti32x4	$123, %zmm30, -2064(%rdx)	 # AVX512F

	vextracti64x4	$0xab, %zmm30, (%rcx)	 # AVX512F
	vextracti64x4	$0xab, %zmm30, (%rcx){%k7}	 # AVX512F
	vextracti64x4	$123, %zmm30, (%rcx)	 # AVX512F
	vextracti64x4	$123, %zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vextracti64x4	$123, %zmm30, 4064(%rdx)	 # AVX512F Disp8
	vextracti64x4	$123, %zmm30, 4096(%rdx)	 # AVX512F
	vextracti64x4	$123, %zmm30, -4096(%rdx)	 # AVX512F Disp8
	vextracti64x4	$123, %zmm30, -4128(%rdx)	 # AVX512F

	vmovapd	%zmm30, (%rcx)	 # AVX512F
	vmovapd	%zmm30, (%rcx){%k7}	 # AVX512F
	vmovapd	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovapd	%zmm30, 8128(%rdx)	 # AVX512F Disp8
	vmovapd	%zmm30, 8192(%rdx)	 # AVX512F
	vmovapd	%zmm30, -8192(%rdx)	 # AVX512F Disp8
	vmovapd	%zmm30, -8256(%rdx)	 # AVX512F

	vmovaps	%zmm30, (%rcx)	 # AVX512F
	vmovaps	%zmm30, (%rcx){%k7}	 # AVX512F
	vmovaps	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovaps	%zmm30, 8128(%rdx)	 # AVX512F Disp8
	vmovaps	%zmm30, 8192(%rdx)	 # AVX512F
	vmovaps	%zmm30, -8192(%rdx)	 # AVX512F Disp8
	vmovaps	%zmm30, -8256(%rdx)	 # AVX512F

	vmovdqa32	%zmm30, (%rcx)	 # AVX512F
	vmovdqa32	%zmm30, (%rcx){%k7}	 # AVX512F
	vmovdqa32	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovdqa32	%zmm30, 8128(%rdx)	 # AVX512F Disp8
	vmovdqa32	%zmm30, 8192(%rdx)	 # AVX512F
	vmovdqa32	%zmm30, -8192(%rdx)	 # AVX512F Disp8
	vmovdqa32	%zmm30, -8256(%rdx)	 # AVX512F

	vmovdqa64	%zmm30, (%rcx)	 # AVX512F
	vmovdqa64	%zmm30, (%rcx){%k7}	 # AVX512F
	vmovdqa64	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovdqa64	%zmm30, 8128(%rdx)	 # AVX512F Disp8
	vmovdqa64	%zmm30, 8192(%rdx)	 # AVX512F
	vmovdqa64	%zmm30, -8192(%rdx)	 # AVX512F Disp8
	vmovdqa64	%zmm30, -8256(%rdx)	 # AVX512F

	vmovdqu32	%zmm30, (%rcx)	 # AVX512F
	vmovdqu32	%zmm30, (%rcx){%k7}	 # AVX512F
	vmovdqu32	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovdqu32	%zmm30, 8128(%rdx)	 # AVX512F Disp8
	vmovdqu32	%zmm30, 8192(%rdx)	 # AVX512F
	vmovdqu32	%zmm30, -8192(%rdx)	 # AVX512F Disp8
	vmovdqu32	%zmm30, -8256(%rdx)	 # AVX512F

	vmovdqu64	%zmm30, (%rcx)	 # AVX512F
	vmovdqu64	%zmm30, (%rcx){%k7}	 # AVX512F
	vmovdqu64	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovdqu64	%zmm30, 8128(%rdx)	 # AVX512F Disp8
	vmovdqu64	%zmm30, 8192(%rdx)	 # AVX512F
	vmovdqu64	%zmm30, -8192(%rdx)	 # AVX512F Disp8
	vmovdqu64	%zmm30, -8256(%rdx)	 # AVX512F

	vmovupd	%zmm30, (%rcx)	 # AVX512F
	vmovupd	%zmm30, (%rcx){%k7}	 # AVX512F
	vmovupd	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovupd	%zmm30, 8128(%rdx)	 # AVX512F Disp8
	vmovupd	%zmm30, 8192(%rdx)	 # AVX512F
	vmovupd	%zmm30, -8192(%rdx)	 # AVX512F Disp8
	vmovupd	%zmm30, -8256(%rdx)	 # AVX512F

	vmovups	%zmm30, (%rcx)	 # AVX512F
	vmovups	%zmm30, (%rcx){%k7}	 # AVX512F
	vmovups	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vmovups	%zmm30, 8128(%rdx)	 # AVX512F Disp8
	vmovups	%zmm30, 8192(%rdx)	 # AVX512F
	vmovups	%zmm30, -8192(%rdx)	 # AVX512F Disp8
	vmovups	%zmm30, -8256(%rdx)	 # AVX512F

	vpmovqb	%zmm30, (%rcx)	 # AVX512F
	vpmovqb	%zmm30, (%rcx){%k7}	 # AVX512F
	vpmovqb	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpmovqb	%zmm30, 1016(%rdx)	 # AVX512F Disp8
	vpmovqb	%zmm30, 1024(%rdx)	 # AVX512F
	vpmovqb	%zmm30, -1024(%rdx)	 # AVX512F Disp8
	vpmovqb	%zmm30, -1032(%rdx)	 # AVX512F

	vpmovsqb	%zmm30, (%rcx)	 # AVX512F
	vpmovsqb	%zmm30, (%rcx){%k7}	 # AVX512F
	vpmovsqb	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpmovsqb	%zmm30, 1016(%rdx)	 # AVX512F Disp8
	vpmovsqb	%zmm30, 1024(%rdx)	 # AVX512F
	vpmovsqb	%zmm30, -1024(%rdx)	 # AVX512F Disp8
	vpmovsqb	%zmm30, -1032(%rdx)	 # AVX512F

	vpmovusqb	%zmm30, (%rcx)	 # AVX512F
	vpmovusqb	%zmm30, (%rcx){%k7}	 # AVX512F
	vpmovusqb	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpmovusqb	%zmm30, 1016(%rdx)	 # AVX512F Disp8
	vpmovusqb	%zmm30, 1024(%rdx)	 # AVX512F
	vpmovusqb	%zmm30, -1024(%rdx)	 # AVX512F Disp8
	vpmovusqb	%zmm30, -1032(%rdx)	 # AVX512F

	vpmovqw	%zmm30, (%rcx)	 # AVX512F
	vpmovqw	%zmm30, (%rcx){%k7}	 # AVX512F
	vpmovqw	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpmovqw	%zmm30, 2032(%rdx)	 # AVX512F Disp8
	vpmovqw	%zmm30, 2048(%rdx)	 # AVX512F
	vpmovqw	%zmm30, -2048(%rdx)	 # AVX512F Disp8
	vpmovqw	%zmm30, -2064(%rdx)	 # AVX512F

	vpmovsqw	%zmm30, (%rcx)	 # AVX512F
	vpmovsqw	%zmm30, (%rcx){%k7}	 # AVX512F
	vpmovsqw	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpmovsqw	%zmm30, 2032(%rdx)	 # AVX512F Disp8
	vpmovsqw	%zmm30, 2048(%rdx)	 # AVX512F
	vpmovsqw	%zmm30, -2048(%rdx)	 # AVX512F Disp8
	vpmovsqw	%zmm30, -2064(%rdx)	 # AVX512F

	vpmovusqw	%zmm30, (%rcx)	 # AVX512F
	vpmovusqw	%zmm30, (%rcx){%k7}	 # AVX512F
	vpmovusqw	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpmovusqw	%zmm30, 2032(%rdx)	 # AVX512F Disp8
	vpmovusqw	%zmm30, 2048(%rdx)	 # AVX512F
	vpmovusqw	%zmm30, -2048(%rdx)	 # AVX512F Disp8
	vpmovusqw	%zmm30, -2064(%rdx)	 # AVX512F

	vpmovqd	%zmm30, (%rcx)	 # AVX512F
	vpmovqd	%zmm30, (%rcx){%k7}	 # AVX512F
	vpmovqd	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpmovqd	%zmm30, 4064(%rdx)	 # AVX512F Disp8
	vpmovqd	%zmm30, 4096(%rdx)	 # AVX512F
	vpmovqd	%zmm30, -4096(%rdx)	 # AVX512F Disp8
	vpmovqd	%zmm30, -4128(%rdx)	 # AVX512F

	vpmovsqd	%zmm30, (%rcx)	 # AVX512F
	vpmovsqd	%zmm30, (%rcx){%k7}	 # AVX512F
	vpmovsqd	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpmovsqd	%zmm30, 4064(%rdx)	 # AVX512F Disp8
	vpmovsqd	%zmm30, 4096(%rdx)	 # AVX512F
	vpmovsqd	%zmm30, -4096(%rdx)	 # AVX512F Disp8
	vpmovsqd	%zmm30, -4128(%rdx)	 # AVX512F

	vpmovusqd	%zmm30, (%rcx)	 # AVX512F
	vpmovusqd	%zmm30, (%rcx){%k7}	 # AVX512F
	vpmovusqd	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpmovusqd	%zmm30, 4064(%rdx)	 # AVX512F Disp8
	vpmovusqd	%zmm30, 4096(%rdx)	 # AVX512F
	vpmovusqd	%zmm30, -4096(%rdx)	 # AVX512F Disp8
	vpmovusqd	%zmm30, -4128(%rdx)	 # AVX512F

	vpmovdb	%zmm30, (%rcx)	 # AVX512F
	vpmovdb	%zmm30, (%rcx){%k7}	 # AVX512F
	vpmovdb	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpmovdb	%zmm30, 2032(%rdx)	 # AVX512F Disp8
	vpmovdb	%zmm30, 2048(%rdx)	 # AVX512F
	vpmovdb	%zmm30, -2048(%rdx)	 # AVX512F Disp8
	vpmovdb	%zmm30, -2064(%rdx)	 # AVX512F

	vpmovsdb	%zmm30, (%rcx)	 # AVX512F
	vpmovsdb	%zmm30, (%rcx){%k7}	 # AVX512F
	vpmovsdb	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpmovsdb	%zmm30, 2032(%rdx)	 # AVX512F Disp8
	vpmovsdb	%zmm30, 2048(%rdx)	 # AVX512F
	vpmovsdb	%zmm30, -2048(%rdx)	 # AVX512F Disp8
	vpmovsdb	%zmm30, -2064(%rdx)	 # AVX512F

	vpmovusdb	%zmm30, (%rcx)	 # AVX512F
	vpmovusdb	%zmm30, (%rcx){%k7}	 # AVX512F
	vpmovusdb	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpmovusdb	%zmm30, 2032(%rdx)	 # AVX512F Disp8
	vpmovusdb	%zmm30, 2048(%rdx)	 # AVX512F
	vpmovusdb	%zmm30, -2048(%rdx)	 # AVX512F Disp8
	vpmovusdb	%zmm30, -2064(%rdx)	 # AVX512F

	vpmovdw	%zmm30, (%rcx)	 # AVX512F
	vpmovdw	%zmm30, (%rcx){%k7}	 # AVX512F
	vpmovdw	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpmovdw	%zmm30, 4064(%rdx)	 # AVX512F Disp8
	vpmovdw	%zmm30, 4096(%rdx)	 # AVX512F
	vpmovdw	%zmm30, -4096(%rdx)	 # AVX512F Disp8
	vpmovdw	%zmm30, -4128(%rdx)	 # AVX512F

	vpmovsdw	%zmm30, (%rcx)	 # AVX512F
	vpmovsdw	%zmm30, (%rcx){%k7}	 # AVX512F
	vpmovsdw	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpmovsdw	%zmm30, 4064(%rdx)	 # AVX512F Disp8
	vpmovsdw	%zmm30, 4096(%rdx)	 # AVX512F
	vpmovsdw	%zmm30, -4096(%rdx)	 # AVX512F Disp8
	vpmovsdw	%zmm30, -4128(%rdx)	 # AVX512F

	vpmovusdw	%zmm30, (%rcx)	 # AVX512F
	vpmovusdw	%zmm30, (%rcx){%k7}	 # AVX512F
	vpmovusdw	%zmm30, 0x123(%rax,%r14,8)	 # AVX512F
	vpmovusdw	%zmm30, 4064(%rdx)	 # AVX512F Disp8
	vpmovusdw	%zmm30, 4096(%rdx)	 # AVX512F
	vpmovusdw	%zmm30, -4096(%rdx)	 # AVX512F Disp8
	vpmovusdw	%zmm30, -4128(%rdx)	 # AVX512F

	vcvttpd2udq	%zmm29, %ymm30{%k7}	 # AVX512F
	vcvttpd2udq	%zmm29, %ymm30{%k7}{z}	 # AVX512F
	vcvttpd2udq	{sae}, %zmm29, %ymm30{%k7}	 # AVX512F
	vcvttpd2udq	(%rcx), %ymm30{%k7}	 # AVX512F
	vcvttpd2udq	0x123(%rax,%r14,8), %ymm30{%k7}	 # AVX512F
	vcvttpd2udq	(%rcx){1to8}, %ymm30{%k7}	 # AVX512F
	vcvttpd2udq	8128(%rdx), %ymm30{%k7}	 # AVX512F Disp8
	vcvttpd2udq	8192(%rdx), %ymm30{%k7}	 # AVX512F
	vcvttpd2udq	-8192(%rdx), %ymm30{%k7}	 # AVX512F Disp8
	vcvttpd2udq	-8256(%rdx), %ymm30{%k7}	 # AVX512F
	vcvttpd2udq	1016(%rdx){1to8}, %ymm30{%k7}	 # AVX512F Disp8
	vcvttpd2udq	1024(%rdx){1to8}, %ymm30{%k7}	 # AVX512F
	vcvttpd2udq	-1024(%rdx){1to8}, %ymm30{%k7}	 # AVX512F Disp8
	vcvttpd2udq	-1032(%rdx){1to8}, %ymm30{%k7}	 # AVX512F

	vcvttps2udq	%zmm29, %zmm30	 # AVX512F
	vcvttps2udq	%zmm29, %zmm30{%k7}	 # AVX512F
	vcvttps2udq	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vcvttps2udq	{sae}, %zmm29, %zmm30	 # AVX512F
	vcvttps2udq	(%rcx), %zmm30	 # AVX512F
	vcvttps2udq	0x123(%rax,%r14,8), %zmm30	 # AVX512F
	vcvttps2udq	(%rcx){1to16}, %zmm30	 # AVX512F
	vcvttps2udq	8128(%rdx), %zmm30	 # AVX512F Disp8
	vcvttps2udq	8192(%rdx), %zmm30	 # AVX512F
	vcvttps2udq	-8192(%rdx), %zmm30	 # AVX512F Disp8
	vcvttps2udq	-8256(%rdx), %zmm30	 # AVX512F
	vcvttps2udq	508(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vcvttps2udq	512(%rdx){1to16}, %zmm30	 # AVX512F
	vcvttps2udq	-512(%rdx){1to16}, %zmm30	 # AVX512F Disp8
	vcvttps2udq	-516(%rdx){1to16}, %zmm30	 # AVX512F

	vcvttsd2usi	%xmm30, %eax	 # AVX512F
	vcvttsd2usi	{sae}, %xmm30, %eax	 # AVX512F
	vcvttsd2usi	(%rcx), %eax	 # AVX512F
	vcvttsd2usi	0x123(%rax,%r14,8), %eax	 # AVX512F
	vcvttsd2usi	1016(%rdx), %eax	 # AVX512F Disp8
	vcvttsd2usi	1024(%rdx), %eax	 # AVX512F
	vcvttsd2usi	-1024(%rdx), %eax	 # AVX512F Disp8
	vcvttsd2usi	-1032(%rdx), %eax	 # AVX512F
	vcvttsd2usi	%xmm30, %ebp	 # AVX512F
	vcvttsd2usi	{sae}, %xmm30, %ebp	 # AVX512F
	vcvttsd2usi	(%rcx), %ebp	 # AVX512F
	vcvttsd2usi	0x123(%rax,%r14,8), %ebp	 # AVX512F
	vcvttsd2usi	1016(%rdx), %ebp	 # AVX512F Disp8
	vcvttsd2usi	1024(%rdx), %ebp	 # AVX512F
	vcvttsd2usi	-1024(%rdx), %ebp	 # AVX512F Disp8
	vcvttsd2usi	-1032(%rdx), %ebp	 # AVX512F
	vcvttsd2usi	%xmm30, %r13d	 # AVX512F
	vcvttsd2usi	{sae}, %xmm30, %r13d	 # AVX512F
	vcvttsd2usi	(%rcx), %r13d	 # AVX512F
	vcvttsd2usi	0x123(%rax,%r14,8), %r13d	 # AVX512F
	vcvttsd2usi	1016(%rdx), %r13d	 # AVX512F Disp8
	vcvttsd2usi	1024(%rdx), %r13d	 # AVX512F
	vcvttsd2usi	-1024(%rdx), %r13d	 # AVX512F Disp8
	vcvttsd2usi	-1032(%rdx), %r13d	 # AVX512F

	vcvttsd2usi	%xmm30, %rax	 # AVX512F
	vcvttsd2usi	{sae}, %xmm30, %rax	 # AVX512F
	vcvttsd2usi	(%rcx), %rax	 # AVX512F
	vcvttsd2usi	0x123(%rax,%r14,8), %rax	 # AVX512F
	vcvttsd2usi	1016(%rdx), %rax	 # AVX512F Disp8
	vcvttsd2usi	1024(%rdx), %rax	 # AVX512F
	vcvttsd2usi	-1024(%rdx), %rax	 # AVX512F Disp8
	vcvttsd2usi	-1032(%rdx), %rax	 # AVX512F
	vcvttsd2usi	%xmm30, %r8	 # AVX512F
	vcvttsd2usi	{sae}, %xmm30, %r8	 # AVX512F
	vcvttsd2usi	(%rcx), %r8	 # AVX512F
	vcvttsd2usi	0x123(%rax,%r14,8), %r8	 # AVX512F
	vcvttsd2usi	1016(%rdx), %r8	 # AVX512F Disp8
	vcvttsd2usi	1024(%rdx), %r8	 # AVX512F
	vcvttsd2usi	-1024(%rdx), %r8	 # AVX512F Disp8
	vcvttsd2usi	-1032(%rdx), %r8	 # AVX512F

	vcvttss2usi	%xmm30, %eax	 # AVX512F
	vcvttss2usi	{sae}, %xmm30, %eax	 # AVX512F
	vcvttss2usi	(%rcx), %eax	 # AVX512F
	vcvttss2usi	0x123(%rax,%r14,8), %eax	 # AVX512F
	vcvttss2usi	508(%rdx), %eax	 # AVX512F Disp8
	vcvttss2usi	512(%rdx), %eax	 # AVX512F
	vcvttss2usi	-512(%rdx), %eax	 # AVX512F Disp8
	vcvttss2usi	-516(%rdx), %eax	 # AVX512F
	vcvttss2usi	%xmm30, %ebp	 # AVX512F
	vcvttss2usi	{sae}, %xmm30, %ebp	 # AVX512F
	vcvttss2usi	(%rcx), %ebp	 # AVX512F
	vcvttss2usi	0x123(%rax,%r14,8), %ebp	 # AVX512F
	vcvttss2usi	508(%rdx), %ebp	 # AVX512F Disp8
	vcvttss2usi	512(%rdx), %ebp	 # AVX512F
	vcvttss2usi	-512(%rdx), %ebp	 # AVX512F Disp8
	vcvttss2usi	-516(%rdx), %ebp	 # AVX512F
	vcvttss2usi	%xmm30, %r13d	 # AVX512F
	vcvttss2usi	{sae}, %xmm30, %r13d	 # AVX512F
	vcvttss2usi	(%rcx), %r13d	 # AVX512F
	vcvttss2usi	0x123(%rax,%r14,8), %r13d	 # AVX512F
	vcvttss2usi	508(%rdx), %r13d	 # AVX512F Disp8
	vcvttss2usi	512(%rdx), %r13d	 # AVX512F
	vcvttss2usi	-512(%rdx), %r13d	 # AVX512F Disp8
	vcvttss2usi	-516(%rdx), %r13d	 # AVX512F

	vcvttss2usi	%xmm30, %rax	 # AVX512F
	vcvttss2usi	{sae}, %xmm30, %rax	 # AVX512F
	vcvttss2usi	(%rcx), %rax	 # AVX512F
	vcvttss2usi	0x123(%rax,%r14,8), %rax	 # AVX512F
	vcvttss2usi	508(%rdx), %rax	 # AVX512F Disp8
	vcvttss2usi	512(%rdx), %rax	 # AVX512F
	vcvttss2usi	-512(%rdx), %rax	 # AVX512F Disp8
	vcvttss2usi	-516(%rdx), %rax	 # AVX512F
	vcvttss2usi	%xmm30, %r8	 # AVX512F
	vcvttss2usi	{sae}, %xmm30, %r8	 # AVX512F
	vcvttss2usi	(%rcx), %r8	 # AVX512F
	vcvttss2usi	0x123(%rax,%r14,8), %r8	 # AVX512F
	vcvttss2usi	508(%rdx), %r8	 # AVX512F Disp8
	vcvttss2usi	512(%rdx), %r8	 # AVX512F
	vcvttss2usi	-512(%rdx), %r8	 # AVX512F Disp8
	vcvttss2usi	-516(%rdx), %r8	 # AVX512F

	vpermi2d	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpermi2d	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermi2d	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermi2d	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpermi2d	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpermi2d	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpermi2d	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2d	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermi2d	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2d	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermi2d	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2d	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpermi2d	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2d	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpermi2q	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpermi2q	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermi2q	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermi2q	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpermi2q	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpermi2q	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpermi2q	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2q	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermi2q	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2q	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermi2q	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2q	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpermi2q	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2q	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vpermi2ps	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpermi2ps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermi2ps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermi2ps	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpermi2ps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpermi2ps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpermi2ps	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2ps	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermi2ps	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2ps	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermi2ps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2ps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F
	vpermi2ps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2ps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512F

	vpermi2pd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vpermi2pd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F
	vpermi2pd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F
	vpermi2pd	(%rcx), %zmm29, %zmm30	 # AVX512F
	vpermi2pd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F
	vpermi2pd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpermi2pd	8128(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2pd	8192(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermi2pd	-8192(%rdx), %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2pd	-8256(%rdx), %zmm29, %zmm30	 # AVX512F
	vpermi2pd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2pd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F
	vpermi2pd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F Disp8
	vpermi2pd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F

	vptestnmd	%zmm28, %zmm29, %k5	 # AVX512CD
	vptestnmd	%zmm28, %zmm29, %k5{%k7}	 # AVX512CD
	vptestnmd	(%rcx), %zmm29, %k5	 # AVX512CD
	vptestnmd	0x123(%rax,%r14,8), %zmm29, %k5	 # AVX512CD
	vptestnmd	(%rcx){1to16}, %zmm29, %k5	 # AVX512CD
	vptestnmd	8128(%rdx), %zmm29, %k5	 # AVX512CD Disp8
	vptestnmd	8192(%rdx), %zmm29, %k5	 # AVX512CD
	vptestnmd	-8192(%rdx), %zmm29, %k5	 # AVX512CD Disp8
	vptestnmd	-8256(%rdx), %zmm29, %k5	 # AVX512CD
	vptestnmd	508(%rdx){1to16}, %zmm29, %k5	 # AVX512CD Disp8
	vptestnmd	512(%rdx){1to16}, %zmm29, %k5	 # AVX512CD
	vptestnmd	-512(%rdx){1to16}, %zmm29, %k5	 # AVX512CD Disp8
	vptestnmd	-516(%rdx){1to16}, %zmm29, %k5	 # AVX512CD

	vptestnmq	%zmm28, %zmm29, %k5	 # AVX512CD
	vptestnmq	%zmm28, %zmm29, %k5{%k7}	 # AVX512CD
	vptestnmq	(%rcx), %zmm29, %k5	 # AVX512CD
	vptestnmq	0x123(%rax,%r14,8), %zmm29, %k5	 # AVX512CD
	vptestnmq	(%rcx){1to8}, %zmm29, %k5	 # AVX512CD
	vptestnmq	8128(%rdx), %zmm29, %k5	 # AVX512CD Disp8
	vptestnmq	8192(%rdx), %zmm29, %k5	 # AVX512CD
	vptestnmq	-8192(%rdx), %zmm29, %k5	 # AVX512CD Disp8
	vptestnmq	-8256(%rdx), %zmm29, %k5	 # AVX512CD
	vptestnmq	1016(%rdx){1to8}, %zmm29, %k5	 # AVX512CD Disp8
	vptestnmq	1024(%rdx){1to8}, %zmm29, %k5	 # AVX512CD
	vptestnmq	-1024(%rdx){1to8}, %zmm29, %k5	 # AVX512CD Disp8
	vptestnmq	-1032(%rdx){1to8}, %zmm29, %k5	 # AVX512CD

	.intel_syntax noprefix
	vaddpd	zmm30, zmm29, zmm28	 # AVX512F
	vaddpd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vaddpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vaddpd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vaddpd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vaddpd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vaddpd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vaddpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vaddpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vaddpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vaddpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vaddpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vaddpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vaddpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vaddpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vaddpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vaddpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vaddpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vaddps	zmm30, zmm29, zmm28	 # AVX512F
	vaddps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vaddps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vaddps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vaddps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vaddps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vaddps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vaddps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vaddps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vaddps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vaddps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vaddps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vaddps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vaddps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vaddps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vaddps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vaddps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vaddps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vaddsd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vaddsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vaddsd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vaddsd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vaddsd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vaddsd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vaddsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vaddsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vaddsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vaddsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vaddsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vaddsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vaddss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vaddss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vaddss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vaddss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vaddss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vaddss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vaddss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vaddss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vaddss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vaddss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vaddss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vaddss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	valignd	zmm30, zmm29, zmm28, 0xab	 # AVX512F
	valignd	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512F
	valignd	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512F
	valignd	zmm30, zmm29, zmm28, 123	 # AVX512F
	valignd	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512F
	valignd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	valignd	zmm30, zmm29, dword bcst [rcx], 123	 # AVX512F
	valignd	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	valignd	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	valignd	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	valignd	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	valignd	zmm30, zmm29, dword bcst [rdx+508], 123	 # AVX512F Disp8
	valignd	zmm30, zmm29, dword bcst [rdx+512], 123	 # AVX512F
	valignd	zmm30, zmm29, dword bcst [rdx-512], 123	 # AVX512F Disp8
	valignd	zmm30, zmm29, dword bcst [rdx-516], 123	 # AVX512F

	vblendmpd	zmm30, zmm29, zmm28	 # AVX512F
	vblendmpd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vblendmpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vblendmpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vblendmpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vblendmpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vblendmpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vblendmpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vblendmpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vblendmpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vblendmpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vblendmpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vblendmpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vblendmpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vblendmps	zmm30, zmm29, zmm28	 # AVX512F
	vblendmps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vblendmps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vblendmps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vblendmps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vblendmps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vblendmps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vblendmps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vblendmps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vblendmps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vblendmps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vblendmps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vblendmps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vblendmps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vbroadcastf32x4	zmm30, XMMWORD PTR [rcx]	 # AVX512F
	vbroadcastf32x4	zmm30{k7}, XMMWORD PTR [rcx]	 # AVX512F
	vbroadcastf32x4	zmm30{k7}{z}, XMMWORD PTR [rcx]	 # AVX512F
	vbroadcastf32x4	zmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vbroadcastf32x4	zmm30, XMMWORD PTR [rdx+2032]	 # AVX512F Disp8
	vbroadcastf32x4	zmm30, XMMWORD PTR [rdx+2048]	 # AVX512F
	vbroadcastf32x4	zmm30, XMMWORD PTR [rdx-2048]	 # AVX512F Disp8
	vbroadcastf32x4	zmm30, XMMWORD PTR [rdx-2064]	 # AVX512F

	vbroadcastf64x4	zmm30, YMMWORD PTR [rcx]	 # AVX512F
	vbroadcastf64x4	zmm30{k7}, YMMWORD PTR [rcx]	 # AVX512F
	vbroadcastf64x4	zmm30{k7}{z}, YMMWORD PTR [rcx]	 # AVX512F
	vbroadcastf64x4	zmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vbroadcastf64x4	zmm30, YMMWORD PTR [rdx+4064]	 # AVX512F Disp8
	vbroadcastf64x4	zmm30, YMMWORD PTR [rdx+4096]	 # AVX512F
	vbroadcastf64x4	zmm30, YMMWORD PTR [rdx-4096]	 # AVX512F Disp8
	vbroadcastf64x4	zmm30, YMMWORD PTR [rdx-4128]	 # AVX512F

	vbroadcasti32x4	zmm30, XMMWORD PTR [rcx]	 # AVX512F
	vbroadcasti32x4	zmm30{k7}, XMMWORD PTR [rcx]	 # AVX512F
	vbroadcasti32x4	zmm30{k7}{z}, XMMWORD PTR [rcx]	 # AVX512F
	vbroadcasti32x4	zmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vbroadcasti32x4	zmm30, XMMWORD PTR [rdx+2032]	 # AVX512F Disp8
	vbroadcasti32x4	zmm30, XMMWORD PTR [rdx+2048]	 # AVX512F
	vbroadcasti32x4	zmm30, XMMWORD PTR [rdx-2048]	 # AVX512F Disp8
	vbroadcasti32x4	zmm30, XMMWORD PTR [rdx-2064]	 # AVX512F

	vbroadcasti64x4	zmm30, YMMWORD PTR [rcx]	 # AVX512F
	vbroadcasti64x4	zmm30{k7}, YMMWORD PTR [rcx]	 # AVX512F
	vbroadcasti64x4	zmm30{k7}{z}, YMMWORD PTR [rcx]	 # AVX512F
	vbroadcasti64x4	zmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vbroadcasti64x4	zmm30, YMMWORD PTR [rdx+4064]	 # AVX512F Disp8
	vbroadcasti64x4	zmm30, YMMWORD PTR [rdx+4096]	 # AVX512F
	vbroadcasti64x4	zmm30, YMMWORD PTR [rdx-4096]	 # AVX512F Disp8
	vbroadcasti64x4	zmm30, YMMWORD PTR [rdx-4128]	 # AVX512F

	vbroadcastsd	zmm30, QWORD PTR [rcx]	 # AVX512F
	vbroadcastsd	zmm30{k7}, QWORD PTR [rcx]	 # AVX512F
	vbroadcastsd	zmm30{k7}{z}, QWORD PTR [rcx]	 # AVX512F
	vbroadcastsd	zmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vbroadcastsd	zmm30, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vbroadcastsd	zmm30, QWORD PTR [rdx+1024]	 # AVX512F
	vbroadcastsd	zmm30, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vbroadcastsd	zmm30, QWORD PTR [rdx-1032]	 # AVX512F

	vbroadcastsd	zmm30{k7}, xmm29	 # AVX512F
	vbroadcastsd	zmm30{k7}{z}, xmm29	 # AVX512F

	vbroadcastss	zmm30, DWORD PTR [rcx]	 # AVX512F
	vbroadcastss	zmm30{k7}, DWORD PTR [rcx]	 # AVX512F
	vbroadcastss	zmm30{k7}{z}, DWORD PTR [rcx]	 # AVX512F
	vbroadcastss	zmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vbroadcastss	zmm30, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vbroadcastss	zmm30, DWORD PTR [rdx+512]	 # AVX512F
	vbroadcastss	zmm30, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vbroadcastss	zmm30, DWORD PTR [rdx-516]	 # AVX512F

	vbroadcastss	zmm30{k7}, xmm29	 # AVX512F
	vbroadcastss	zmm30{k7}{z}, xmm29	 # AVX512F

	vcmppd	k5, zmm30, zmm29, 0xab	 # AVX512F
	vcmppd	k5{k7}, zmm30, zmm29, 0xab	 # AVX512F
	vcmppd	k5, zmm30, zmm29{sae}, 0xab	 # AVX512F
	vcmppd	k5, zmm30, zmm29, 123	 # AVX512F
	vcmppd	k5, zmm30, zmm29{sae}, 123	 # AVX512F
	vcmppd	k5, zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vcmppd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vcmppd	k5, zmm30, qword bcst [rcx], 123	 # AVX512F
	vcmppd	k5, zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vcmppd	k5, zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vcmppd	k5, zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vcmppd	k5, zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vcmppd	k5, zmm30, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vcmppd	k5, zmm30, qword bcst [rdx+1024], 123	 # AVX512F
	vcmppd	k5, zmm30, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vcmppd	k5, zmm30, qword bcst [rdx-1032], 123	 # AVX512F

	vcmpeq_oqpd	k5, zmm30, zmm29	 # AVX512F
	vcmpeq_oqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpeq_oqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpeq_oqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpeq_oqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_oqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpeq_oqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpeq_oqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpeq_oqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpeq_oqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpeq_oqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpeq_oqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpeq_oqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpeq_oqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpeqpd	k5, zmm30, zmm29	 # AVX512F
	vcmpeqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpeqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpeqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpeqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpeqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpeqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpeqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpeqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpeqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpeqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpeqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpeqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmplt_ospd	k5, zmm30, zmm29	 # AVX512F
	vcmplt_ospd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmplt_ospd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmplt_ospd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmplt_ospd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmplt_ospd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmplt_ospd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmplt_ospd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmplt_ospd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmplt_ospd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmplt_ospd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmplt_ospd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmplt_ospd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmplt_ospd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpltpd	k5, zmm30, zmm29	 # AVX512F
	vcmpltpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpltpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpltpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpltpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpltpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpltpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpltpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpltpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpltpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpltpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpltpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpltpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpltpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmple_ospd	k5, zmm30, zmm29	 # AVX512F
	vcmple_ospd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmple_ospd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmple_ospd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmple_ospd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmple_ospd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmple_ospd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmple_ospd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmple_ospd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmple_ospd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmple_ospd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmple_ospd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmple_ospd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmple_ospd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmplepd	k5, zmm30, zmm29	 # AVX512F
	vcmplepd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmplepd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmplepd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmplepd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmplepd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmplepd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmplepd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmplepd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmplepd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmplepd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmplepd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmplepd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmplepd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpunord_qpd	k5, zmm30, zmm29	 # AVX512F
	vcmpunord_qpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpunord_qpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpunord_qpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpunord_qpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpunord_qpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpunord_qpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpunord_qpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpunord_qpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpunord_qpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpunord_qpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpunord_qpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpunord_qpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpunord_qpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpunordpd	k5, zmm30, zmm29	 # AVX512F
	vcmpunordpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpunordpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpunordpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpunordpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpunordpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpunordpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpunordpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpunordpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpunordpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpunordpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpunordpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpunordpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpunordpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpneq_uqpd	k5, zmm30, zmm29	 # AVX512F
	vcmpneq_uqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpneq_uqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpneq_uqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpneq_uqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_uqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpneq_uqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpneq_uqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpneq_uqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpneq_uqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpneq_uqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpneq_uqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpneq_uqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpneq_uqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpneqpd	k5, zmm30, zmm29	 # AVX512F
	vcmpneqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpneqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpneqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpneqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpneqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpneqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpneqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpneqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpneqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpneqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpneqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpneqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpnlt_uspd	k5, zmm30, zmm29	 # AVX512F
	vcmpnlt_uspd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnlt_uspd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnlt_uspd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnlt_uspd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnlt_uspd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpnlt_uspd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnlt_uspd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnlt_uspd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnlt_uspd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnlt_uspd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpnlt_uspd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpnlt_uspd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpnlt_uspd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpnltpd	k5, zmm30, zmm29	 # AVX512F
	vcmpnltpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnltpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnltpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnltpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnltpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpnltpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnltpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnltpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnltpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnltpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpnltpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpnltpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpnltpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpnle_uspd	k5, zmm30, zmm29	 # AVX512F
	vcmpnle_uspd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnle_uspd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnle_uspd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnle_uspd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnle_uspd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpnle_uspd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnle_uspd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnle_uspd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnle_uspd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnle_uspd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpnle_uspd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpnle_uspd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpnle_uspd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpnlepd	k5, zmm30, zmm29	 # AVX512F
	vcmpnlepd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnlepd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnlepd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnlepd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnlepd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpnlepd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnlepd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnlepd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnlepd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnlepd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpnlepd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpnlepd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpnlepd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpord_qpd	k5, zmm30, zmm29	 # AVX512F
	vcmpord_qpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpord_qpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpord_qpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpord_qpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpord_qpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpord_qpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpord_qpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpord_qpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpord_qpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpord_qpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpord_qpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpord_qpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpord_qpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpordpd	k5, zmm30, zmm29	 # AVX512F
	vcmpordpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpordpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpordpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpordpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpordpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpordpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpordpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpordpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpordpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpordpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpordpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpordpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpordpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpeq_uqpd	k5, zmm30, zmm29	 # AVX512F
	vcmpeq_uqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpeq_uqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpeq_uqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpeq_uqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_uqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpeq_uqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpeq_uqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpeq_uqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpeq_uqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpeq_uqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpeq_uqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpeq_uqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpeq_uqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpnge_uspd	k5, zmm30, zmm29	 # AVX512F
	vcmpnge_uspd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnge_uspd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnge_uspd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnge_uspd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnge_uspd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpnge_uspd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnge_uspd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnge_uspd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnge_uspd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnge_uspd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpnge_uspd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpnge_uspd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpnge_uspd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpngepd	k5, zmm30, zmm29	 # AVX512F
	vcmpngepd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpngepd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpngepd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpngepd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngepd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpngepd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpngepd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpngepd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpngepd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpngepd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpngepd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpngepd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpngepd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpngt_uspd	k5, zmm30, zmm29	 # AVX512F
	vcmpngt_uspd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpngt_uspd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpngt_uspd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpngt_uspd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngt_uspd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpngt_uspd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpngt_uspd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpngt_uspd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpngt_uspd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpngt_uspd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpngt_uspd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpngt_uspd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpngt_uspd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpngtpd	k5, zmm30, zmm29	 # AVX512F
	vcmpngtpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpngtpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpngtpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpngtpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngtpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpngtpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpngtpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpngtpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpngtpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpngtpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpngtpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpngtpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpngtpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpfalse_oqpd	k5, zmm30, zmm29	 # AVX512F
	vcmpfalse_oqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpfalse_oqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpfalse_oqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpfalse_oqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpfalse_oqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpfalse_oqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpfalse_oqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpfalse_oqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpfalse_oqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpfalse_oqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpfalse_oqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpfalse_oqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpfalse_oqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpfalsepd	k5, zmm30, zmm29	 # AVX512F
	vcmpfalsepd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpfalsepd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpfalsepd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpfalsepd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpfalsepd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpfalsepd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpfalsepd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpfalsepd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpfalsepd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpfalsepd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpfalsepd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpfalsepd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpfalsepd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpneq_oqpd	k5, zmm30, zmm29	 # AVX512F
	vcmpneq_oqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpneq_oqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpneq_oqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpneq_oqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_oqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpneq_oqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpneq_oqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpneq_oqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpneq_oqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpneq_oqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpneq_oqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpneq_oqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpneq_oqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpge_ospd	k5, zmm30, zmm29	 # AVX512F
	vcmpge_ospd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpge_ospd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpge_ospd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpge_ospd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpge_ospd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpge_ospd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpge_ospd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpge_ospd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpge_ospd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpge_ospd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpge_ospd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpge_ospd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpge_ospd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpgepd	k5, zmm30, zmm29	 # AVX512F
	vcmpgepd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpgepd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpgepd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpgepd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgepd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpgepd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpgepd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpgepd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpgepd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpgepd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpgepd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpgepd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpgepd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpgt_ospd	k5, zmm30, zmm29	 # AVX512F
	vcmpgt_ospd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpgt_ospd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpgt_ospd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpgt_ospd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgt_ospd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpgt_ospd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpgt_ospd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpgt_ospd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpgt_ospd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpgt_ospd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpgt_ospd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpgt_ospd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpgt_ospd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpgtpd	k5, zmm30, zmm29	 # AVX512F
	vcmpgtpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpgtpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpgtpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpgtpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgtpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpgtpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpgtpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpgtpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpgtpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpgtpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpgtpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpgtpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpgtpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmptrue_uqpd	k5, zmm30, zmm29	 # AVX512F
	vcmptrue_uqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmptrue_uqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmptrue_uqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmptrue_uqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmptrue_uqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmptrue_uqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmptrue_uqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmptrue_uqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmptrue_uqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmptrue_uqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmptrue_uqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmptrue_uqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmptrue_uqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmptruepd	k5, zmm30, zmm29	 # AVX512F
	vcmptruepd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmptruepd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmptruepd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmptruepd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmptruepd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmptruepd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmptruepd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmptruepd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmptruepd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmptruepd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmptruepd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmptruepd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmptruepd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpeq_ospd	k5, zmm30, zmm29	 # AVX512F
	vcmpeq_ospd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpeq_ospd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpeq_ospd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpeq_ospd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_ospd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpeq_ospd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpeq_ospd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpeq_ospd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpeq_ospd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpeq_ospd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpeq_ospd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpeq_ospd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpeq_ospd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmplt_oqpd	k5, zmm30, zmm29	 # AVX512F
	vcmplt_oqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmplt_oqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmplt_oqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmplt_oqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmplt_oqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmplt_oqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmplt_oqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmplt_oqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmplt_oqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmplt_oqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmplt_oqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmplt_oqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmplt_oqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmple_oqpd	k5, zmm30, zmm29	 # AVX512F
	vcmple_oqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmple_oqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmple_oqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmple_oqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmple_oqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmple_oqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmple_oqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmple_oqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmple_oqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmple_oqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmple_oqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmple_oqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmple_oqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpunord_spd	k5, zmm30, zmm29	 # AVX512F
	vcmpunord_spd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpunord_spd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpunord_spd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpunord_spd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpunord_spd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpunord_spd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpunord_spd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpunord_spd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpunord_spd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpunord_spd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpunord_spd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpunord_spd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpunord_spd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpneq_uspd	k5, zmm30, zmm29	 # AVX512F
	vcmpneq_uspd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpneq_uspd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpneq_uspd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpneq_uspd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_uspd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpneq_uspd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpneq_uspd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpneq_uspd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpneq_uspd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpneq_uspd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpneq_uspd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpneq_uspd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpneq_uspd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpnlt_uqpd	k5, zmm30, zmm29	 # AVX512F
	vcmpnlt_uqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnlt_uqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnlt_uqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnlt_uqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnlt_uqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpnlt_uqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnlt_uqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnlt_uqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnlt_uqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnlt_uqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpnlt_uqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpnlt_uqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpnlt_uqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpnle_uqpd	k5, zmm30, zmm29	 # AVX512F
	vcmpnle_uqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnle_uqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnle_uqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnle_uqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnle_uqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpnle_uqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnle_uqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnle_uqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnle_uqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnle_uqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpnle_uqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpnle_uqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpnle_uqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpord_spd	k5, zmm30, zmm29	 # AVX512F
	vcmpord_spd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpord_spd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpord_spd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpord_spd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpord_spd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpord_spd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpord_spd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpord_spd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpord_spd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpord_spd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpord_spd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpord_spd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpord_spd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpeq_uspd	k5, zmm30, zmm29	 # AVX512F
	vcmpeq_uspd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpeq_uspd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpeq_uspd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpeq_uspd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_uspd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpeq_uspd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpeq_uspd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpeq_uspd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpeq_uspd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpeq_uspd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpeq_uspd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpeq_uspd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpeq_uspd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpnge_uqpd	k5, zmm30, zmm29	 # AVX512F
	vcmpnge_uqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnge_uqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnge_uqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnge_uqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnge_uqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpnge_uqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnge_uqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnge_uqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnge_uqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnge_uqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpnge_uqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpnge_uqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpnge_uqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpngt_uqpd	k5, zmm30, zmm29	 # AVX512F
	vcmpngt_uqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpngt_uqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpngt_uqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpngt_uqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngt_uqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpngt_uqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpngt_uqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpngt_uqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpngt_uqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpngt_uqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpngt_uqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpngt_uqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpngt_uqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpfalse_ospd	k5, zmm30, zmm29	 # AVX512F
	vcmpfalse_ospd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpfalse_ospd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpfalse_ospd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpfalse_ospd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpfalse_ospd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpfalse_ospd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpfalse_ospd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpfalse_ospd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpfalse_ospd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpfalse_ospd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpfalse_ospd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpfalse_ospd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpfalse_ospd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpneq_ospd	k5, zmm30, zmm29	 # AVX512F
	vcmpneq_ospd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpneq_ospd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpneq_ospd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpneq_ospd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_ospd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpneq_ospd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpneq_ospd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpneq_ospd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpneq_ospd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpneq_ospd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpneq_ospd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpneq_ospd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpneq_ospd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpge_oqpd	k5, zmm30, zmm29	 # AVX512F
	vcmpge_oqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpge_oqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpge_oqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpge_oqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpge_oqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpge_oqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpge_oqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpge_oqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpge_oqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpge_oqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpge_oqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpge_oqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpge_oqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpgt_oqpd	k5, zmm30, zmm29	 # AVX512F
	vcmpgt_oqpd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpgt_oqpd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpgt_oqpd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpgt_oqpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgt_oqpd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmpgt_oqpd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpgt_oqpd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpgt_oqpd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpgt_oqpd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpgt_oqpd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmpgt_oqpd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmpgt_oqpd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmpgt_oqpd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmptrue_uspd	k5, zmm30, zmm29	 # AVX512F
	vcmptrue_uspd	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmptrue_uspd	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmptrue_uspd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmptrue_uspd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmptrue_uspd	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vcmptrue_uspd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmptrue_uspd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmptrue_uspd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmptrue_uspd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmptrue_uspd	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcmptrue_uspd	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vcmptrue_uspd	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcmptrue_uspd	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vcmpps	k5, zmm30, zmm29, 0xab	 # AVX512F
	vcmpps	k5{k7}, zmm30, zmm29, 0xab	 # AVX512F
	vcmpps	k5, zmm30, zmm29{sae}, 0xab	 # AVX512F
	vcmpps	k5, zmm30, zmm29, 123	 # AVX512F
	vcmpps	k5, zmm30, zmm29{sae}, 123	 # AVX512F
	vcmpps	k5, zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vcmpps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vcmpps	k5, zmm30, dword bcst [rcx], 123	 # AVX512F
	vcmpps	k5, zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vcmpps	k5, zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vcmpps	k5, zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vcmpps	k5, zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vcmpps	k5, zmm30, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vcmpps	k5, zmm30, dword bcst [rdx+512], 123	 # AVX512F
	vcmpps	k5, zmm30, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vcmpps	k5, zmm30, dword bcst [rdx-516], 123	 # AVX512F

	vcmpeq_oqps	k5, zmm30, zmm29	 # AVX512F
	vcmpeq_oqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpeq_oqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpeq_oqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpeq_oqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_oqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpeq_oqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpeq_oqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpeq_oqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpeq_oqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpeq_oqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpeq_oqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpeq_oqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpeq_oqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpeqps	k5, zmm30, zmm29	 # AVX512F
	vcmpeqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpeqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpeqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpeqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpeqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpeqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpeqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpeqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpeqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpeqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpeqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpeqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmplt_osps	k5, zmm30, zmm29	 # AVX512F
	vcmplt_osps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmplt_osps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmplt_osps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmplt_osps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmplt_osps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmplt_osps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmplt_osps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmplt_osps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmplt_osps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmplt_osps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmplt_osps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmplt_osps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmplt_osps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpltps	k5, zmm30, zmm29	 # AVX512F
	vcmpltps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpltps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpltps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpltps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpltps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpltps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpltps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpltps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpltps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpltps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpltps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpltps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpltps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmple_osps	k5, zmm30, zmm29	 # AVX512F
	vcmple_osps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmple_osps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmple_osps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmple_osps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmple_osps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmple_osps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmple_osps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmple_osps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmple_osps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmple_osps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmple_osps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmple_osps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmple_osps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpleps	k5, zmm30, zmm29	 # AVX512F
	vcmpleps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpleps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpleps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpleps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpleps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpleps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpleps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpleps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpleps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpleps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpleps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpleps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpleps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpunord_qps	k5, zmm30, zmm29	 # AVX512F
	vcmpunord_qps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpunord_qps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpunord_qps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpunord_qps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpunord_qps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpunord_qps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpunord_qps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpunord_qps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpunord_qps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpunord_qps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpunord_qps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpunord_qps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpunord_qps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpunordps	k5, zmm30, zmm29	 # AVX512F
	vcmpunordps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpunordps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpunordps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpunordps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpunordps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpunordps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpunordps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpunordps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpunordps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpunordps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpunordps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpunordps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpunordps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpneq_uqps	k5, zmm30, zmm29	 # AVX512F
	vcmpneq_uqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpneq_uqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpneq_uqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpneq_uqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_uqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpneq_uqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpneq_uqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpneq_uqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpneq_uqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpneq_uqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpneq_uqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpneq_uqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpneq_uqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpneqps	k5, zmm30, zmm29	 # AVX512F
	vcmpneqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpneqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpneqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpneqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpneqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpneqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpneqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpneqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpneqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpneqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpneqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpneqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpnlt_usps	k5, zmm30, zmm29	 # AVX512F
	vcmpnlt_usps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnlt_usps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnlt_usps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnlt_usps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnlt_usps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpnlt_usps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnlt_usps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnlt_usps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnlt_usps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnlt_usps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpnlt_usps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpnlt_usps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpnlt_usps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpnltps	k5, zmm30, zmm29	 # AVX512F
	vcmpnltps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnltps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnltps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnltps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnltps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpnltps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnltps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnltps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnltps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnltps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpnltps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpnltps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpnltps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpnle_usps	k5, zmm30, zmm29	 # AVX512F
	vcmpnle_usps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnle_usps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnle_usps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnle_usps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnle_usps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpnle_usps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnle_usps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnle_usps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnle_usps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnle_usps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpnle_usps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpnle_usps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpnle_usps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpnleps	k5, zmm30, zmm29	 # AVX512F
	vcmpnleps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnleps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnleps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnleps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnleps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpnleps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnleps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnleps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnleps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnleps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpnleps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpnleps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpnleps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpord_qps	k5, zmm30, zmm29	 # AVX512F
	vcmpord_qps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpord_qps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpord_qps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpord_qps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpord_qps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpord_qps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpord_qps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpord_qps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpord_qps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpord_qps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpord_qps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpord_qps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpord_qps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpordps	k5, zmm30, zmm29	 # AVX512F
	vcmpordps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpordps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpordps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpordps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpordps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpordps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpordps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpordps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpordps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpordps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpordps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpordps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpordps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpeq_uqps	k5, zmm30, zmm29	 # AVX512F
	vcmpeq_uqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpeq_uqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpeq_uqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpeq_uqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_uqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpeq_uqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpeq_uqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpeq_uqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpeq_uqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpeq_uqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpeq_uqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpeq_uqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpeq_uqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpnge_usps	k5, zmm30, zmm29	 # AVX512F
	vcmpnge_usps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnge_usps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnge_usps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnge_usps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnge_usps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpnge_usps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnge_usps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnge_usps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnge_usps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnge_usps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpnge_usps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpnge_usps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpnge_usps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpngeps	k5, zmm30, zmm29	 # AVX512F
	vcmpngeps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpngeps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpngeps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpngeps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngeps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpngeps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpngeps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpngeps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpngeps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpngeps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpngeps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpngeps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpngeps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpngt_usps	k5, zmm30, zmm29	 # AVX512F
	vcmpngt_usps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpngt_usps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpngt_usps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpngt_usps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngt_usps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpngt_usps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpngt_usps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpngt_usps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpngt_usps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpngt_usps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpngt_usps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpngt_usps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpngt_usps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpngtps	k5, zmm30, zmm29	 # AVX512F
	vcmpngtps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpngtps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpngtps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpngtps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngtps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpngtps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpngtps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpngtps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpngtps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpngtps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpngtps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpngtps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpngtps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpfalse_oqps	k5, zmm30, zmm29	 # AVX512F
	vcmpfalse_oqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpfalse_oqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpfalse_oqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpfalse_oqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpfalse_oqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpfalse_oqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpfalse_oqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpfalse_oqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpfalse_oqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpfalse_oqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpfalse_oqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpfalse_oqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpfalse_oqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpfalseps	k5, zmm30, zmm29	 # AVX512F
	vcmpfalseps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpfalseps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpfalseps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpfalseps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpfalseps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpfalseps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpfalseps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpfalseps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpfalseps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpfalseps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpfalseps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpfalseps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpfalseps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpneq_oqps	k5, zmm30, zmm29	 # AVX512F
	vcmpneq_oqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpneq_oqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpneq_oqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpneq_oqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_oqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpneq_oqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpneq_oqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpneq_oqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpneq_oqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpneq_oqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpneq_oqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpneq_oqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpneq_oqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpge_osps	k5, zmm30, zmm29	 # AVX512F
	vcmpge_osps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpge_osps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpge_osps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpge_osps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpge_osps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpge_osps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpge_osps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpge_osps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpge_osps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpge_osps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpge_osps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpge_osps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpge_osps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpgeps	k5, zmm30, zmm29	 # AVX512F
	vcmpgeps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpgeps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpgeps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpgeps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgeps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpgeps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpgeps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpgeps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpgeps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpgeps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpgeps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpgeps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpgeps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpgt_osps	k5, zmm30, zmm29	 # AVX512F
	vcmpgt_osps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpgt_osps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpgt_osps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpgt_osps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgt_osps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpgt_osps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpgt_osps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpgt_osps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpgt_osps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpgt_osps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpgt_osps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpgt_osps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpgt_osps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpgtps	k5, zmm30, zmm29	 # AVX512F
	vcmpgtps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpgtps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpgtps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpgtps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgtps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpgtps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpgtps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpgtps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpgtps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpgtps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpgtps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpgtps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpgtps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmptrue_uqps	k5, zmm30, zmm29	 # AVX512F
	vcmptrue_uqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmptrue_uqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmptrue_uqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmptrue_uqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmptrue_uqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmptrue_uqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmptrue_uqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmptrue_uqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmptrue_uqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmptrue_uqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmptrue_uqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmptrue_uqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmptrue_uqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmptrueps	k5, zmm30, zmm29	 # AVX512F
	vcmptrueps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmptrueps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmptrueps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmptrueps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmptrueps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmptrueps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmptrueps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmptrueps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmptrueps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmptrueps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmptrueps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmptrueps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmptrueps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpeq_osps	k5, zmm30, zmm29	 # AVX512F
	vcmpeq_osps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpeq_osps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpeq_osps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpeq_osps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_osps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpeq_osps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpeq_osps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpeq_osps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpeq_osps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpeq_osps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpeq_osps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpeq_osps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpeq_osps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmplt_oqps	k5, zmm30, zmm29	 # AVX512F
	vcmplt_oqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmplt_oqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmplt_oqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmplt_oqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmplt_oqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmplt_oqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmplt_oqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmplt_oqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmplt_oqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmplt_oqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmplt_oqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmplt_oqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmplt_oqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmple_oqps	k5, zmm30, zmm29	 # AVX512F
	vcmple_oqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmple_oqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmple_oqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmple_oqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmple_oqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmple_oqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmple_oqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmple_oqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmple_oqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmple_oqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmple_oqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmple_oqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmple_oqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpunord_sps	k5, zmm30, zmm29	 # AVX512F
	vcmpunord_sps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpunord_sps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpunord_sps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpunord_sps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpunord_sps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpunord_sps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpunord_sps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpunord_sps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpunord_sps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpunord_sps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpunord_sps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpunord_sps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpunord_sps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpneq_usps	k5, zmm30, zmm29	 # AVX512F
	vcmpneq_usps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpneq_usps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpneq_usps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpneq_usps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_usps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpneq_usps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpneq_usps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpneq_usps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpneq_usps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpneq_usps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpneq_usps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpneq_usps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpneq_usps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpnlt_uqps	k5, zmm30, zmm29	 # AVX512F
	vcmpnlt_uqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnlt_uqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnlt_uqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnlt_uqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnlt_uqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpnlt_uqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnlt_uqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnlt_uqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnlt_uqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnlt_uqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpnlt_uqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpnlt_uqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpnlt_uqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpnle_uqps	k5, zmm30, zmm29	 # AVX512F
	vcmpnle_uqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnle_uqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnle_uqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnle_uqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnle_uqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpnle_uqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnle_uqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnle_uqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnle_uqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnle_uqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpnle_uqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpnle_uqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpnle_uqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpord_sps	k5, zmm30, zmm29	 # AVX512F
	vcmpord_sps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpord_sps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpord_sps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpord_sps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpord_sps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpord_sps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpord_sps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpord_sps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpord_sps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpord_sps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpord_sps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpord_sps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpord_sps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpeq_usps	k5, zmm30, zmm29	 # AVX512F
	vcmpeq_usps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpeq_usps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpeq_usps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpeq_usps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_usps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpeq_usps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpeq_usps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpeq_usps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpeq_usps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpeq_usps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpeq_usps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpeq_usps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpeq_usps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpnge_uqps	k5, zmm30, zmm29	 # AVX512F
	vcmpnge_uqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpnge_uqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpnge_uqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpnge_uqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnge_uqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpnge_uqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpnge_uqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpnge_uqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpnge_uqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpnge_uqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpnge_uqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpnge_uqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpnge_uqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpngt_uqps	k5, zmm30, zmm29	 # AVX512F
	vcmpngt_uqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpngt_uqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpngt_uqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpngt_uqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngt_uqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpngt_uqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpngt_uqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpngt_uqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpngt_uqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpngt_uqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpngt_uqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpngt_uqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpngt_uqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpfalse_osps	k5, zmm30, zmm29	 # AVX512F
	vcmpfalse_osps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpfalse_osps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpfalse_osps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpfalse_osps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpfalse_osps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpfalse_osps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpfalse_osps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpfalse_osps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpfalse_osps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpfalse_osps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpfalse_osps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpfalse_osps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpfalse_osps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpneq_osps	k5, zmm30, zmm29	 # AVX512F
	vcmpneq_osps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpneq_osps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpneq_osps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpneq_osps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_osps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpneq_osps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpneq_osps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpneq_osps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpneq_osps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpneq_osps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpneq_osps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpneq_osps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpneq_osps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpge_oqps	k5, zmm30, zmm29	 # AVX512F
	vcmpge_oqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpge_oqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpge_oqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpge_oqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpge_oqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpge_oqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpge_oqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpge_oqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpge_oqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpge_oqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpge_oqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpge_oqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpge_oqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpgt_oqps	k5, zmm30, zmm29	 # AVX512F
	vcmpgt_oqps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmpgt_oqps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmpgt_oqps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmpgt_oqps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgt_oqps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmpgt_oqps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmpgt_oqps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmpgt_oqps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmpgt_oqps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmpgt_oqps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmpgt_oqps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmpgt_oqps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmpgt_oqps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmptrue_usps	k5, zmm30, zmm29	 # AVX512F
	vcmptrue_usps	k5{k7}, zmm30, zmm29	 # AVX512F
	vcmptrue_usps	k5, zmm30, zmm29{sae}	 # AVX512F
	vcmptrue_usps	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcmptrue_usps	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmptrue_usps	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vcmptrue_usps	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcmptrue_usps	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcmptrue_usps	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcmptrue_usps	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcmptrue_usps	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcmptrue_usps	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vcmptrue_usps	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcmptrue_usps	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vcmpsd	k5{k7}, xmm29, xmm28, 0xab	 # AVX512F
	vcmpsd	k5{k7}, xmm29, xmm28{sae}, 0xab	 # AVX512F
	vcmpsd	k5{k7}, xmm29, xmm28, 123	 # AVX512F
	vcmpsd	k5{k7}, xmm29, xmm28{sae}, 123	 # AVX512F
	vcmpsd	k5{k7}, xmm29, QWORD PTR [rcx], 123	 # AVX512F
	vcmpsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vcmpsd	k5{k7}, xmm29, QWORD PTR [rdx+1016], 123	 # AVX512F Disp8
	vcmpsd	k5{k7}, xmm29, QWORD PTR [rdx+1024], 123	 # AVX512F
	vcmpsd	k5{k7}, xmm29, QWORD PTR [rdx-1024], 123	 # AVX512F Disp8
	vcmpsd	k5{k7}, xmm29, QWORD PTR [rdx-1032], 123	 # AVX512F

	vcmpeq_oqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpeq_oqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpeq_oqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpeq_oqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpeq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpeq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpeq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpeqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpeqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpeqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpeqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpeqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpeqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpeqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmplt_ossd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmplt_ossd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmplt_ossd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmplt_ossd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmplt_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmplt_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmplt_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmplt_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpltsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpltsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpltsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpltsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpltsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpltsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpltsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpltsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmple_ossd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmple_ossd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmple_ossd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmple_ossd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmple_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmple_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmple_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmple_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmplesd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmplesd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmplesd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmplesd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmplesd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmplesd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmplesd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmplesd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpunord_qsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpunord_qsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpunord_qsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpunord_qsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpunord_qsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpunord_qsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpunord_qsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpunord_qsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpunordsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpunordsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpunordsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpunordsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpunordsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpunordsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpunordsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpunordsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpneq_uqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpneq_uqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpneq_uqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpneq_uqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpneq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpneq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpneq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpneqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpneqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpneqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpneqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpneqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpneqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpneqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpnlt_ussd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnlt_ussd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnlt_ussd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpnlt_ussd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnlt_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpnlt_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpnlt_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpnlt_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpnltsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnltsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnltsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpnltsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnltsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpnltsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpnltsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpnltsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpnle_ussd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnle_ussd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnle_ussd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpnle_ussd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnle_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpnle_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpnle_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpnle_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpnlesd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnlesd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnlesd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpnlesd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnlesd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpnlesd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpnlesd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpnlesd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpord_qsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpord_qsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpord_qsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpord_qsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpord_qsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpord_qsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpord_qsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpord_qsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpordsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpordsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpordsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpordsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpordsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpordsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpordsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpordsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpeq_uqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpeq_uqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpeq_uqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpeq_uqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpeq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpeq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpeq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpnge_ussd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnge_ussd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnge_ussd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpnge_ussd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnge_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpnge_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpnge_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpnge_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpngesd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpngesd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpngesd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpngesd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngesd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpngesd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpngesd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpngesd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpngt_ussd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpngt_ussd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpngt_ussd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpngt_ussd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngt_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpngt_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpngt_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpngt_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpngtsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpngtsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpngtsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpngtsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngtsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpngtsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpngtsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpngtsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpfalse_oqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpfalse_oqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpfalse_oqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpfalse_oqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpfalse_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpfalse_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpfalse_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpfalse_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpfalsesd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpfalsesd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpfalsesd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpfalsesd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpfalsesd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpfalsesd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpfalsesd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpfalsesd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpneq_oqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpneq_oqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpneq_oqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpneq_oqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpneq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpneq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpneq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpge_ossd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpge_ossd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpge_ossd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpge_ossd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpge_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpge_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpge_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpge_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpgesd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpgesd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpgesd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpgesd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgesd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpgesd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpgesd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpgesd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpgt_ossd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpgt_ossd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpgt_ossd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpgt_ossd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgt_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpgt_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpgt_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpgt_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpgtsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpgtsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpgtsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpgtsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgtsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpgtsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpgtsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpgtsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmptrue_uqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmptrue_uqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmptrue_uqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmptrue_uqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmptrue_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmptrue_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmptrue_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmptrue_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmptruesd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmptruesd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmptruesd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmptruesd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmptruesd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmptruesd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmptruesd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmptruesd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpeq_ossd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpeq_ossd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpeq_ossd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpeq_ossd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpeq_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpeq_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpeq_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmplt_oqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmplt_oqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmplt_oqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmplt_oqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmplt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmplt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmplt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmplt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmple_oqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmple_oqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmple_oqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmple_oqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmple_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmple_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmple_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmple_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpunord_ssd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpunord_ssd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpunord_ssd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpunord_ssd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpunord_ssd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpunord_ssd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpunord_ssd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpunord_ssd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpneq_ussd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpneq_ussd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpneq_ussd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpneq_ussd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpneq_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpneq_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpneq_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpnlt_uqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnlt_uqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnlt_uqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpnlt_uqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnlt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpnlt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpnlt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpnlt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpnle_uqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnle_uqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnle_uqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpnle_uqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnle_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpnle_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpnle_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpnle_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpord_ssd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpord_ssd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpord_ssd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpord_ssd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpord_ssd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpord_ssd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpord_ssd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpord_ssd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpeq_ussd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpeq_ussd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpeq_ussd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpeq_ussd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpeq_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpeq_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpeq_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpnge_uqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnge_uqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnge_uqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpnge_uqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnge_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpnge_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpnge_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpnge_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpngt_uqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpngt_uqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpngt_uqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpngt_uqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpngt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpngt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpngt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpfalse_ossd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpfalse_ossd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpfalse_ossd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpfalse_ossd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpfalse_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpfalse_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpfalse_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpfalse_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpneq_ossd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpneq_ossd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpneq_ossd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpneq_ossd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpneq_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpneq_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpneq_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpge_oqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpge_oqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpge_oqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpge_oqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpge_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpge_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpge_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpge_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpgt_oqsd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpgt_oqsd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpgt_oqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmpgt_oqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmpgt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmpgt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmpgt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmptrue_ussd	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmptrue_ussd	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmptrue_ussd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcmptrue_ussd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmptrue_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcmptrue_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcmptrue_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcmptrue_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcmpss	k5{k7}, xmm29, xmm28, 0xab	 # AVX512F
	vcmpss	k5{k7}, xmm29, xmm28{sae}, 0xab	 # AVX512F
	vcmpss	k5{k7}, xmm29, xmm28, 123	 # AVX512F
	vcmpss	k5{k7}, xmm29, xmm28{sae}, 123	 # AVX512F
	vcmpss	k5{k7}, xmm29, DWORD PTR [rcx], 123	 # AVX512F
	vcmpss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vcmpss	k5{k7}, xmm29, DWORD PTR [rdx+508], 123	 # AVX512F Disp8
	vcmpss	k5{k7}, xmm29, DWORD PTR [rdx+512], 123	 # AVX512F
	vcmpss	k5{k7}, xmm29, DWORD PTR [rdx-512], 123	 # AVX512F Disp8
	vcmpss	k5{k7}, xmm29, DWORD PTR [rdx-516], 123	 # AVX512F

	vcmpeq_oqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpeq_oqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpeq_oqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpeq_oqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_oqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpeq_oqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpeq_oqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpeq_oqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpeqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpeqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpeqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpeqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpeqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpeqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpeqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmplt_osss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmplt_osss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmplt_osss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmplt_osss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmplt_osss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmplt_osss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmplt_osss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmplt_osss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpltss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpltss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpltss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpltss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpltss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpltss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpltss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpltss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmple_osss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmple_osss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmple_osss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmple_osss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmple_osss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmple_osss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmple_osss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmple_osss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpless	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpless	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpless	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpless	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpless	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpless	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpless	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpless	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpunord_qss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpunord_qss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpunord_qss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpunord_qss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpunord_qss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpunord_qss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpunord_qss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpunord_qss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpunordss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpunordss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpunordss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpunordss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpunordss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpunordss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpunordss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpunordss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpneq_uqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpneq_uqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpneq_uqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpneq_uqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_uqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpneq_uqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpneq_uqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpneq_uqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpneqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpneqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpneqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpneqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpneqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpneqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpneqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpnlt_usss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnlt_usss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnlt_usss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpnlt_usss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnlt_usss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpnlt_usss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpnlt_usss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpnlt_usss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpnltss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnltss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnltss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpnltss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnltss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpnltss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpnltss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpnltss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpnle_usss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnle_usss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnle_usss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpnle_usss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnle_usss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpnle_usss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpnle_usss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpnle_usss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpnless	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnless	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnless	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpnless	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnless	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpnless	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpnless	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpnless	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpord_qss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpord_qss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpord_qss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpord_qss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpord_qss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpord_qss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpord_qss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpord_qss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpordss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpordss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpordss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpordss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpordss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpordss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpordss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpordss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpeq_uqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpeq_uqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpeq_uqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpeq_uqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_uqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpeq_uqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpeq_uqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpeq_uqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpnge_usss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnge_usss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnge_usss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpnge_usss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnge_usss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpnge_usss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpnge_usss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpnge_usss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpngess	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpngess	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpngess	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpngess	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngess	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpngess	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpngess	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpngess	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpngt_usss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpngt_usss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpngt_usss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpngt_usss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngt_usss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpngt_usss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpngt_usss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpngt_usss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpngtss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpngtss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpngtss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpngtss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngtss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpngtss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpngtss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpngtss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpfalse_oqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpfalse_oqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpfalse_oqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpfalse_oqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpfalse_oqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpfalse_oqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpfalse_oqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpfalse_oqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpfalsess	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpfalsess	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpfalsess	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpfalsess	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpfalsess	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpfalsess	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpfalsess	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpfalsess	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpneq_oqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpneq_oqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpneq_oqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpneq_oqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_oqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpneq_oqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpneq_oqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpneq_oqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpge_osss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpge_osss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpge_osss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpge_osss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpge_osss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpge_osss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpge_osss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpge_osss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpgess	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpgess	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpgess	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpgess	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgess	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpgess	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpgess	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpgess	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpgt_osss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpgt_osss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpgt_osss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpgt_osss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgt_osss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpgt_osss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpgt_osss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpgt_osss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpgtss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpgtss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpgtss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpgtss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgtss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpgtss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpgtss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpgtss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmptrue_uqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmptrue_uqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmptrue_uqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmptrue_uqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmptrue_uqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmptrue_uqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmptrue_uqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmptrue_uqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmptruess	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmptruess	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmptruess	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmptruess	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmptruess	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmptruess	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmptruess	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmptruess	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpeq_osss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpeq_osss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpeq_osss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpeq_osss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_osss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpeq_osss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpeq_osss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpeq_osss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmplt_oqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmplt_oqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmplt_oqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmplt_oqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmplt_oqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmplt_oqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmplt_oqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmplt_oqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmple_oqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmple_oqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmple_oqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmple_oqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmple_oqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmple_oqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmple_oqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmple_oqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpunord_sss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpunord_sss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpunord_sss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpunord_sss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpunord_sss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpunord_sss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpunord_sss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpunord_sss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpneq_usss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpneq_usss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpneq_usss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpneq_usss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_usss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpneq_usss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpneq_usss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpneq_usss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpnlt_uqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnlt_uqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnlt_uqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpnlt_uqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnlt_uqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpnlt_uqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpnlt_uqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpnlt_uqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpnle_uqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnle_uqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnle_uqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpnle_uqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnle_uqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpnle_uqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpnle_uqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpnle_uqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpord_sss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpord_sss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpord_sss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpord_sss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpord_sss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpord_sss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpord_sss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpord_sss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpeq_usss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpeq_usss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpeq_usss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpeq_usss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpeq_usss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpeq_usss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpeq_usss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpeq_usss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpnge_uqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpnge_uqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpnge_uqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpnge_uqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpnge_uqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpnge_uqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpnge_uqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpnge_uqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpngt_uqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpngt_uqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpngt_uqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpngt_uqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpngt_uqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpngt_uqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpngt_uqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpngt_uqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpfalse_osss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpfalse_osss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpfalse_osss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpfalse_osss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpfalse_osss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpfalse_osss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpfalse_osss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpfalse_osss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpneq_osss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpneq_osss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpneq_osss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpneq_osss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpneq_osss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpneq_osss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpneq_osss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpneq_osss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpge_oqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpge_oqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpge_oqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpge_oqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpge_oqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpge_oqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpge_oqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpge_oqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmpgt_oqss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmpgt_oqss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmpgt_oqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmpgt_oqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmpgt_oqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmpgt_oqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmpgt_oqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmpgt_oqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcmptrue_usss	k5{k7}, xmm29, xmm28	 # AVX512F
	vcmptrue_usss	k5{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcmptrue_usss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcmptrue_usss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcmptrue_usss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcmptrue_usss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcmptrue_usss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcmptrue_usss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcomisd	xmm30, xmm29	 # AVX512F
	vcomisd	xmm30, xmm29{sae}	 # AVX512F
	vcomisd	xmm30, QWORD PTR [rcx]	 # AVX512F
	vcomisd	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcomisd	xmm30, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcomisd	xmm30, QWORD PTR [rdx+1024]	 # AVX512F
	vcomisd	xmm30, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcomisd	xmm30, QWORD PTR [rdx-1032]	 # AVX512F

	vcomiss	xmm30, xmm29	 # AVX512F
	vcomiss	xmm30, xmm29{sae}	 # AVX512F
	vcomiss	xmm30, DWORD PTR [rcx]	 # AVX512F
	vcomiss	xmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcomiss	xmm30, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcomiss	xmm30, DWORD PTR [rdx+512]	 # AVX512F
	vcomiss	xmm30, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcomiss	xmm30, DWORD PTR [rdx-516]	 # AVX512F

	vcompresspd	ZMMWORD PTR [rcx], zmm30	 # AVX512F
	vcompresspd	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vcompresspd	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vcompresspd	ZMMWORD PTR [rdx+1016], zmm30	 # AVX512F Disp8
	vcompresspd	ZMMWORD PTR [rdx+1024], zmm30	 # AVX512F
	vcompresspd	ZMMWORD PTR [rdx-1024], zmm30	 # AVX512F Disp8
	vcompresspd	ZMMWORD PTR [rdx-1032], zmm30	 # AVX512F

	vcompresspd	zmm30, zmm29	 # AVX512F
	vcompresspd	zmm30{k7}, zmm29	 # AVX512F
	vcompresspd	zmm30{k7}{z}, zmm29	 # AVX512F

	vcompressps	ZMMWORD PTR [rcx], zmm30	 # AVX512F
	vcompressps	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vcompressps	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vcompressps	ZMMWORD PTR [rdx+508], zmm30	 # AVX512F Disp8
	vcompressps	ZMMWORD PTR [rdx+512], zmm30	 # AVX512F
	vcompressps	ZMMWORD PTR [rdx-512], zmm30	 # AVX512F Disp8
	vcompressps	ZMMWORD PTR [rdx-516], zmm30	 # AVX512F

	vcompressps	zmm30, zmm29	 # AVX512F
	vcompressps	zmm30{k7}, zmm29	 # AVX512F
	vcompressps	zmm30{k7}{z}, zmm29	 # AVX512F

	vcvtdq2pd	zmm30{k7}, ymm29	 # AVX512F
	vcvtdq2pd	zmm30{k7}{z}, ymm29	 # AVX512F
	vcvtdq2pd	zmm30{k7}, YMMWORD PTR [rcx]	 # AVX512F
	vcvtdq2pd	zmm30{k7}, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtdq2pd	zmm30{k7}, dword bcst [rcx]	 # AVX512F
	vcvtdq2pd	zmm30{k7}, YMMWORD PTR [rdx+4064]	 # AVX512F Disp8
	vcvtdq2pd	zmm30{k7}, YMMWORD PTR [rdx+4096]	 # AVX512F
	vcvtdq2pd	zmm30{k7}, YMMWORD PTR [rdx-4096]	 # AVX512F Disp8
	vcvtdq2pd	zmm30{k7}, YMMWORD PTR [rdx-4128]	 # AVX512F
	vcvtdq2pd	zmm30{k7}, dword bcst [rdx+508]	 # AVX512F Disp8
	vcvtdq2pd	zmm30{k7}, dword bcst [rdx+512]	 # AVX512F
	vcvtdq2pd	zmm30{k7}, dword bcst [rdx-512]	 # AVX512F Disp8
	vcvtdq2pd	zmm30{k7}, dword bcst [rdx-516]	 # AVX512F

	vcvtdq2ps	zmm30, zmm29	 # AVX512F
	vcvtdq2ps	zmm30{k7}, zmm29	 # AVX512F
	vcvtdq2ps	zmm30{k7}{z}, zmm29	 # AVX512F
	vcvtdq2ps	zmm30, zmm29{rn-sae}	 # AVX512F
	vcvtdq2ps	zmm30, zmm29{ru-sae}	 # AVX512F
	vcvtdq2ps	zmm30, zmm29{rd-sae}	 # AVX512F
	vcvtdq2ps	zmm30, zmm29{rz-sae}	 # AVX512F
	vcvtdq2ps	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcvtdq2ps	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtdq2ps	zmm30, dword bcst [rcx]	 # AVX512F
	vcvtdq2ps	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcvtdq2ps	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcvtdq2ps	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcvtdq2ps	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcvtdq2ps	zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcvtdq2ps	zmm30, dword bcst [rdx+512]	 # AVX512F
	vcvtdq2ps	zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcvtdq2ps	zmm30, dword bcst [rdx-516]	 # AVX512F

	vcvtpd2dq	ymm30{k7}, zmm29	 # AVX512F
	vcvtpd2dq	ymm30{k7}{z}, zmm29	 # AVX512F
	vcvtpd2dq	ymm30{k7}, zmm29{rn-sae}	 # AVX512F
	vcvtpd2dq	ymm30{k7}, zmm29{ru-sae}	 # AVX512F
	vcvtpd2dq	ymm30{k7}, zmm29{rd-sae}	 # AVX512F
	vcvtpd2dq	ymm30{k7}, zmm29{rz-sae}	 # AVX512F
	vcvtpd2dq	ymm30{k7}, ZMMWORD PTR [rcx]	 # AVX512F
	vcvtpd2dq	ymm30{k7}, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtpd2dq	ymm30{k7}, qword bcst [rcx]	 # AVX512F
	vcvtpd2dq	ymm30{k7}, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcvtpd2dq	ymm30{k7}, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcvtpd2dq	ymm30{k7}, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcvtpd2dq	ymm30{k7}, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcvtpd2dq	ymm30{k7}, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcvtpd2dq	ymm30{k7}, qword bcst [rdx+1024]	 # AVX512F
	vcvtpd2dq	ymm30{k7}, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcvtpd2dq	ymm30{k7}, qword bcst [rdx-1032]	 # AVX512F

	vcvtpd2ps	ymm30{k7}, zmm29	 # AVX512F
	vcvtpd2ps	ymm30{k7}{z}, zmm29	 # AVX512F
	vcvtpd2ps	ymm30{k7}, zmm29{rn-sae}	 # AVX512F
	vcvtpd2ps	ymm30{k7}, zmm29{ru-sae}	 # AVX512F
	vcvtpd2ps	ymm30{k7}, zmm29{rd-sae}	 # AVX512F
	vcvtpd2ps	ymm30{k7}, zmm29{rz-sae}	 # AVX512F
	vcvtpd2ps	ymm30{k7}, ZMMWORD PTR [rcx]	 # AVX512F
	vcvtpd2ps	ymm30{k7}, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtpd2ps	ymm30{k7}, qword bcst [rcx]	 # AVX512F
	vcvtpd2ps	ymm30{k7}, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcvtpd2ps	ymm30{k7}, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcvtpd2ps	ymm30{k7}, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcvtpd2ps	ymm30{k7}, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcvtpd2ps	ymm30{k7}, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcvtpd2ps	ymm30{k7}, qword bcst [rdx+1024]	 # AVX512F
	vcvtpd2ps	ymm30{k7}, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcvtpd2ps	ymm30{k7}, qword bcst [rdx-1032]	 # AVX512F

	vcvtpd2udq	ymm30{k7}, zmm29	 # AVX512F
	vcvtpd2udq	ymm30{k7}{z}, zmm29	 # AVX512F
	vcvtpd2udq	ymm30{k7}, zmm29{rn-sae}	 # AVX512F
	vcvtpd2udq	ymm30{k7}, zmm29{ru-sae}	 # AVX512F
	vcvtpd2udq	ymm30{k7}, zmm29{rd-sae}	 # AVX512F
	vcvtpd2udq	ymm30{k7}, zmm29{rz-sae}	 # AVX512F
	vcvtpd2udq	ymm30{k7}, ZMMWORD PTR [rcx]	 # AVX512F
	vcvtpd2udq	ymm30{k7}, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtpd2udq	ymm30{k7}, qword bcst [rcx]	 # AVX512F
	vcvtpd2udq	ymm30{k7}, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcvtpd2udq	ymm30{k7}, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcvtpd2udq	ymm30{k7}, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcvtpd2udq	ymm30{k7}, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcvtpd2udq	ymm30{k7}, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcvtpd2udq	ymm30{k7}, qword bcst [rdx+1024]	 # AVX512F
	vcvtpd2udq	ymm30{k7}, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcvtpd2udq	ymm30{k7}, qword bcst [rdx-1032]	 # AVX512F

	vcvtph2ps	zmm30{k7}, ymm29	 # AVX512F
	vcvtph2ps	zmm30{k7}{z}, ymm29	 # AVX512F
	vcvtph2ps	zmm30{k7}, ymm29{sae}	 # AVX512F
	vcvtph2ps	zmm30{k7}, YMMWORD PTR [rcx]	 # AVX512F
	vcvtph2ps	zmm30{k7}, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtph2ps	zmm30{k7}, YMMWORD PTR [rdx+4064]	 # AVX512F Disp8
	vcvtph2ps	zmm30{k7}, YMMWORD PTR [rdx+4096]	 # AVX512F
	vcvtph2ps	zmm30{k7}, YMMWORD PTR [rdx-4096]	 # AVX512F Disp8
	vcvtph2ps	zmm30{k7}, YMMWORD PTR [rdx-4128]	 # AVX512F

	vcvtps2dq	zmm30, zmm29	 # AVX512F
	vcvtps2dq	zmm30{k7}, zmm29	 # AVX512F
	vcvtps2dq	zmm30{k7}{z}, zmm29	 # AVX512F
	vcvtps2dq	zmm30, zmm29{rn-sae}	 # AVX512F
	vcvtps2dq	zmm30, zmm29{ru-sae}	 # AVX512F
	vcvtps2dq	zmm30, zmm29{rd-sae}	 # AVX512F
	vcvtps2dq	zmm30, zmm29{rz-sae}	 # AVX512F
	vcvtps2dq	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcvtps2dq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtps2dq	zmm30, dword bcst [rcx]	 # AVX512F
	vcvtps2dq	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcvtps2dq	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcvtps2dq	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcvtps2dq	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcvtps2dq	zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcvtps2dq	zmm30, dword bcst [rdx+512]	 # AVX512F
	vcvtps2dq	zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcvtps2dq	zmm30, dword bcst [rdx-516]	 # AVX512F

	vcvtps2pd	zmm30{k7}, ymm29	 # AVX512F
	vcvtps2pd	zmm30{k7}{z}, ymm29	 # AVX512F
	vcvtps2pd	zmm30{k7}, ymm29{sae}	 # AVX512F
	vcvtps2pd	zmm30{k7}, YMMWORD PTR [rcx]	 # AVX512F
	vcvtps2pd	zmm30{k7}, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtps2pd	zmm30{k7}, dword bcst [rcx]	 # AVX512F
	vcvtps2pd	zmm30{k7}, YMMWORD PTR [rdx+4064]	 # AVX512F Disp8
	vcvtps2pd	zmm30{k7}, YMMWORD PTR [rdx+4096]	 # AVX512F
	vcvtps2pd	zmm30{k7}, YMMWORD PTR [rdx-4096]	 # AVX512F Disp8
	vcvtps2pd	zmm30{k7}, YMMWORD PTR [rdx-4128]	 # AVX512F
	vcvtps2pd	zmm30{k7}, dword bcst [rdx+508]	 # AVX512F Disp8
	vcvtps2pd	zmm30{k7}, dword bcst [rdx+512]	 # AVX512F
	vcvtps2pd	zmm30{k7}, dword bcst [rdx-512]	 # AVX512F Disp8
	vcvtps2pd	zmm30{k7}, dword bcst [rdx-516]	 # AVX512F

	vcvtps2ph	ymm30{k7}, zmm29, 0xab	 # AVX512F
	vcvtps2ph	ymm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vcvtps2ph	ymm30{k7}, zmm29{sae}, 0xab	 # AVX512F
	vcvtps2ph	ymm30{k7}, zmm29, 123	 # AVX512F
	vcvtps2ph	ymm30{k7}, zmm29{sae}, 123	 # AVX512F

	vcvtps2udq	zmm30, zmm29	 # AVX512F
	vcvtps2udq	zmm30{k7}, zmm29	 # AVX512F
	vcvtps2udq	zmm30{k7}{z}, zmm29	 # AVX512F
	vcvtps2udq	zmm30, zmm29{rn-sae}	 # AVX512F
	vcvtps2udq	zmm30, zmm29{ru-sae}	 # AVX512F
	vcvtps2udq	zmm30, zmm29{rd-sae}	 # AVX512F
	vcvtps2udq	zmm30, zmm29{rz-sae}	 # AVX512F
	vcvtps2udq	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcvtps2udq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtps2udq	zmm30, dword bcst [rcx]	 # AVX512F
	vcvtps2udq	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcvtps2udq	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcvtps2udq	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcvtps2udq	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcvtps2udq	zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcvtps2udq	zmm30, dword bcst [rdx+512]	 # AVX512F
	vcvtps2udq	zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcvtps2udq	zmm30, dword bcst [rdx-516]	 # AVX512F

	vcvtsd2si	eax, xmm30{rn-sae}	 # AVX512F
	vcvtsd2si	eax, xmm30{ru-sae}	 # AVX512F
	vcvtsd2si	eax, xmm30{rd-sae}	 # AVX512F
	vcvtsd2si	eax, xmm30{rz-sae}	 # AVX512F
	vcvtsd2si	ebp, xmm30{rn-sae}	 # AVX512F
	vcvtsd2si	ebp, xmm30{ru-sae}	 # AVX512F
	vcvtsd2si	ebp, xmm30{rd-sae}	 # AVX512F
	vcvtsd2si	ebp, xmm30{rz-sae}	 # AVX512F
	vcvtsd2si	r13d, xmm30{rn-sae}	 # AVX512F
	vcvtsd2si	r13d, xmm30{ru-sae}	 # AVX512F
	vcvtsd2si	r13d, xmm30{rd-sae}	 # AVX512F
	vcvtsd2si	r13d, xmm30{rz-sae}	 # AVX512F

	vcvtsd2si	rax, xmm30{rn-sae}	 # AVX512F
	vcvtsd2si	rax, xmm30{ru-sae}	 # AVX512F
	vcvtsd2si	rax, xmm30{rd-sae}	 # AVX512F
	vcvtsd2si	rax, xmm30{rz-sae}	 # AVX512F
	vcvtsd2si	r8, xmm30{rn-sae}	 # AVX512F
	vcvtsd2si	r8, xmm30{ru-sae}	 # AVX512F
	vcvtsd2si	r8, xmm30{rd-sae}	 # AVX512F
	vcvtsd2si	r8, xmm30{rz-sae}	 # AVX512F

	vcvtsd2ss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vcvtsd2ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vcvtsd2ss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vcvtsd2ss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vcvtsd2ss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vcvtsd2ss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vcvtsd2ss	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcvtsd2ss	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtsd2ss	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcvtsd2ss	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcvtsd2ss	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcvtsd2ss	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcvtsi2sd	xmm30, xmm29, eax	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, ebp	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, r13d	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcvtsi2sd	xmm30, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcvtsi2sd	xmm30, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcvtsi2sd	xmm30, xmm29, rax	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, rax{rn-sae}	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, rax{ru-sae}	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, rax{rd-sae}	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, rax{rz-sae}	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, r8	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, r8{rn-sae}	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, r8{ru-sae}	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, r8{rd-sae}	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, r8{rz-sae}	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcvtsi2sd	xmm30, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcvtsi2sd	xmm30, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcvtsi2sd	xmm30, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcvtsi2ss	xmm30, xmm29, eax	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, eax{rn-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, eax{ru-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, eax{rd-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, eax{rz-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, ebp	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, ebp{rn-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, ebp{ru-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, ebp{rd-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, ebp{rz-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, r13d	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, r13d{rn-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, r13d{ru-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, r13d{rd-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, r13d{rz-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcvtsi2ss	xmm30, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcvtsi2ss	xmm30, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcvtsi2ss	xmm30, xmm29, rax	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, rax{rn-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, rax{ru-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, rax{rd-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, rax{rz-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, r8	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, r8{rn-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, r8{ru-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, r8{rd-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, r8{rz-sae}	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcvtsi2ss	xmm30, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcvtsi2ss	xmm30, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcvtsi2ss	xmm30, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcvtss2sd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vcvtss2sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vcvtss2sd	xmm30{k7}, xmm29, xmm28{sae}	 # AVX512F
	vcvtss2sd	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcvtss2sd	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtss2sd	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcvtss2sd	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcvtss2sd	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcvtss2sd	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcvtss2si	eax, xmm30{rn-sae}	 # AVX512F
	vcvtss2si	eax, xmm30{ru-sae}	 # AVX512F
	vcvtss2si	eax, xmm30{rd-sae}	 # AVX512F
	vcvtss2si	eax, xmm30{rz-sae}	 # AVX512F
	vcvtss2si	ebp, xmm30{rn-sae}	 # AVX512F
	vcvtss2si	ebp, xmm30{ru-sae}	 # AVX512F
	vcvtss2si	ebp, xmm30{rd-sae}	 # AVX512F
	vcvtss2si	ebp, xmm30{rz-sae}	 # AVX512F
	vcvtss2si	r13d, xmm30{rn-sae}	 # AVX512F
	vcvtss2si	r13d, xmm30{ru-sae}	 # AVX512F
	vcvtss2si	r13d, xmm30{rd-sae}	 # AVX512F
	vcvtss2si	r13d, xmm30{rz-sae}	 # AVX512F

	vcvtss2si	rax, xmm30{rn-sae}	 # AVX512F
	vcvtss2si	rax, xmm30{ru-sae}	 # AVX512F
	vcvtss2si	rax, xmm30{rd-sae}	 # AVX512F
	vcvtss2si	rax, xmm30{rz-sae}	 # AVX512F
	vcvtss2si	r8, xmm30{rn-sae}	 # AVX512F
	vcvtss2si	r8, xmm30{ru-sae}	 # AVX512F
	vcvtss2si	r8, xmm30{rd-sae}	 # AVX512F
	vcvtss2si	r8, xmm30{rz-sae}	 # AVX512F

	vcvttpd2dq	ymm30{k7}, zmm29	 # AVX512F
	vcvttpd2dq	ymm30{k7}{z}, zmm29	 # AVX512F
	vcvttpd2dq	ymm30{k7}, zmm29{sae}	 # AVX512F
	vcvttpd2dq	ymm30{k7}, ZMMWORD PTR [rcx]	 # AVX512F
	vcvttpd2dq	ymm30{k7}, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvttpd2dq	ymm30{k7}, qword bcst [rcx]	 # AVX512F
	vcvttpd2dq	ymm30{k7}, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcvttpd2dq	ymm30{k7}, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcvttpd2dq	ymm30{k7}, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcvttpd2dq	ymm30{k7}, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcvttpd2dq	ymm30{k7}, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcvttpd2dq	ymm30{k7}, qword bcst [rdx+1024]	 # AVX512F
	vcvttpd2dq	ymm30{k7}, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcvttpd2dq	ymm30{k7}, qword bcst [rdx-1032]	 # AVX512F

	vcvttps2dq	zmm30, zmm29	 # AVX512F
	vcvttps2dq	zmm30{k7}, zmm29	 # AVX512F
	vcvttps2dq	zmm30{k7}{z}, zmm29	 # AVX512F
	vcvttps2dq	zmm30, zmm29{sae}	 # AVX512F
	vcvttps2dq	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcvttps2dq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvttps2dq	zmm30, dword bcst [rcx]	 # AVX512F
	vcvttps2dq	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcvttps2dq	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcvttps2dq	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcvttps2dq	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcvttps2dq	zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcvttps2dq	zmm30, dword bcst [rdx+512]	 # AVX512F
	vcvttps2dq	zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcvttps2dq	zmm30, dword bcst [rdx-516]	 # AVX512F

	vcvttsd2si	eax, xmm30{sae}	 # AVX512F
	vcvttsd2si	ebp, xmm30{sae}	 # AVX512F
	vcvttsd2si	r13d, xmm30{sae}	 # AVX512F

	vcvttsd2si	rax, xmm30{sae}	 # AVX512F
	vcvttsd2si	r8, xmm30{sae}	 # AVX512F

	vcvttss2si	eax, xmm30{sae}	 # AVX512F
	vcvttss2si	ebp, xmm30{sae}	 # AVX512F
	vcvttss2si	r13d, xmm30{sae}	 # AVX512F

	vcvttss2si	rax, xmm30{sae}	 # AVX512F
	vcvttss2si	r8, xmm30{sae}	 # AVX512F

	vcvtudq2pd	zmm30{k7}, ymm29	 # AVX512F
	vcvtudq2pd	zmm30{k7}{z}, ymm29	 # AVX512F
	vcvtudq2pd	zmm30{k7}, YMMWORD PTR [rcx]	 # AVX512F
	vcvtudq2pd	zmm30{k7}, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtudq2pd	zmm30{k7}, dword bcst [rcx]	 # AVX512F
	vcvtudq2pd	zmm30{k7}, YMMWORD PTR [rdx+4064]	 # AVX512F Disp8
	vcvtudq2pd	zmm30{k7}, YMMWORD PTR [rdx+4096]	 # AVX512F
	vcvtudq2pd	zmm30{k7}, YMMWORD PTR [rdx-4096]	 # AVX512F Disp8
	vcvtudq2pd	zmm30{k7}, YMMWORD PTR [rdx-4128]	 # AVX512F
	vcvtudq2pd	zmm30{k7}, dword bcst [rdx+508]	 # AVX512F Disp8
	vcvtudq2pd	zmm30{k7}, dword bcst [rdx+512]	 # AVX512F
	vcvtudq2pd	zmm30{k7}, dword bcst [rdx-512]	 # AVX512F Disp8
	vcvtudq2pd	zmm30{k7}, dword bcst [rdx-516]	 # AVX512F

	vcvtudq2ps	zmm30, zmm29	 # AVX512F
	vcvtudq2ps	zmm30{k7}, zmm29	 # AVX512F
	vcvtudq2ps	zmm30{k7}{z}, zmm29	 # AVX512F
	vcvtudq2ps	zmm30, zmm29{rn-sae}	 # AVX512F
	vcvtudq2ps	zmm30, zmm29{ru-sae}	 # AVX512F
	vcvtudq2ps	zmm30, zmm29{rd-sae}	 # AVX512F
	vcvtudq2ps	zmm30, zmm29{rz-sae}	 # AVX512F
	vcvtudq2ps	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcvtudq2ps	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtudq2ps	zmm30, dword bcst [rcx]	 # AVX512F
	vcvtudq2ps	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcvtudq2ps	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcvtudq2ps	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcvtudq2ps	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcvtudq2ps	zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcvtudq2ps	zmm30, dword bcst [rdx+512]	 # AVX512F
	vcvtudq2ps	zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcvtudq2ps	zmm30, dword bcst [rdx-516]	 # AVX512F

	vdivpd	zmm30, zmm29, zmm28	 # AVX512F
	vdivpd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vdivpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vdivpd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vdivpd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vdivpd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vdivpd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vdivpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vdivpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vdivpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vdivpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vdivpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vdivpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vdivpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vdivpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vdivpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vdivpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vdivpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vdivps	zmm30, zmm29, zmm28	 # AVX512F
	vdivps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vdivps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vdivps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vdivps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vdivps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vdivps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vdivps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vdivps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vdivps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vdivps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vdivps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vdivps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vdivps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vdivps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vdivps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vdivps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vdivps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vdivsd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vdivsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vdivsd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vdivsd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vdivsd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vdivsd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vdivsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vdivsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vdivsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vdivsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vdivsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vdivsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vdivss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vdivss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vdivss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vdivss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vdivss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vdivss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vdivss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vdivss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vdivss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vdivss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vdivss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vdivss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vexpandpd	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vexpandpd	zmm30{k7}, ZMMWORD PTR [rcx]	 # AVX512F
	vexpandpd	zmm30{k7}{z}, ZMMWORD PTR [rcx]	 # AVX512F
	vexpandpd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vexpandpd	zmm30, ZMMWORD PTR [rdx+1016]	 # AVX512F Disp8
	vexpandpd	zmm30, ZMMWORD PTR [rdx+1024]	 # AVX512F
	vexpandpd	zmm30, ZMMWORD PTR [rdx-1024]	 # AVX512F Disp8
	vexpandpd	zmm30, ZMMWORD PTR [rdx-1032]	 # AVX512F

	vexpandpd	zmm30, zmm29	 # AVX512F
	vexpandpd	zmm30{k7}, zmm29	 # AVX512F
	vexpandpd	zmm30{k7}{z}, zmm29	 # AVX512F

	vexpandps	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vexpandps	zmm30{k7}, ZMMWORD PTR [rcx]	 # AVX512F
	vexpandps	zmm30{k7}{z}, ZMMWORD PTR [rcx]	 # AVX512F
	vexpandps	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vexpandps	zmm30, ZMMWORD PTR [rdx+508]	 # AVX512F Disp8
	vexpandps	zmm30, ZMMWORD PTR [rdx+512]	 # AVX512F
	vexpandps	zmm30, ZMMWORD PTR [rdx-512]	 # AVX512F Disp8
	vexpandps	zmm30, ZMMWORD PTR [rdx-516]	 # AVX512F

	vexpandps	zmm30, zmm29	 # AVX512F
	vexpandps	zmm30{k7}, zmm29	 # AVX512F
	vexpandps	zmm30{k7}{z}, zmm29	 # AVX512F

	vextractf32x4	xmm30{k7}, zmm29, 0xab	 # AVX512F
	vextractf32x4	xmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vextractf32x4	xmm30{k7}, zmm29, 123	 # AVX512F

	vextractf64x4	ymm30{k7}, zmm29, 0xab	 # AVX512F
	vextractf64x4	ymm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vextractf64x4	ymm30{k7}, zmm29, 123	 # AVX512F

	vextracti32x4	xmm30{k7}, zmm29, 0xab	 # AVX512F
	vextracti32x4	xmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vextracti32x4	xmm30{k7}, zmm29, 123	 # AVX512F

	vextracti64x4	ymm30{k7}, zmm29, 0xab	 # AVX512F
	vextracti64x4	ymm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vextracti64x4	ymm30{k7}, zmm29, 123	 # AVX512F

	vextractps	eax, xmm29, 0xab	 # AVX512F
	vextractps	rax, xmm29, 123	 # AVX512F
	vextractps	r8, xmm29, 123	 # AVX512F
	vextractps	DWORD PTR [rcx], xmm29, 123	 # AVX512F
	vextractps	DWORD PTR [rax+r14*8+0x1234], xmm29, 123	 # AVX512F
	vextractps	DWORD PTR [rdx+508], xmm29, 123	 # AVX512F Disp8
	vextractps	DWORD PTR [rdx+512], xmm29, 123	 # AVX512F
	vextractps	DWORD PTR [rdx-512], xmm29, 123	 # AVX512F Disp8
	vextractps	DWORD PTR [rdx-516], xmm29, 123	 # AVX512F

	vfmadd132pd	zmm30, zmm29, zmm28	 # AVX512F
	vfmadd132pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmadd132pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmadd132pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmadd132pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmadd132pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmadd132pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmadd132pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmadd132pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmadd132pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfmadd132pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmadd132pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmadd132pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmadd132pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmadd132pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfmadd132pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfmadd132pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfmadd132pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfmadd132ps	zmm30, zmm29, zmm28	 # AVX512F
	vfmadd132ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmadd132ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmadd132ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmadd132ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmadd132ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmadd132ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmadd132ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmadd132ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmadd132ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfmadd132ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmadd132ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmadd132ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmadd132ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmadd132ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfmadd132ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfmadd132ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfmadd132ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfmadd132sd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfmadd132sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfmadd132sd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfmadd132sd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfmadd132sd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfmadd132sd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vfmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vfmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vfmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vfmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vfmadd132ss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfmadd132ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfmadd132ss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfmadd132ss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfmadd132ss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfmadd132ss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vfmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vfmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vfmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vfmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vfmadd213pd	zmm30, zmm29, zmm28	 # AVX512F
	vfmadd213pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmadd213pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmadd213pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmadd213pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmadd213pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmadd213pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmadd213pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmadd213pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmadd213pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfmadd213pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmadd213pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmadd213pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmadd213pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmadd213pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfmadd213pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfmadd213pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfmadd213pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfmadd213ps	zmm30, zmm29, zmm28	 # AVX512F
	vfmadd213ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmadd213ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmadd213ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmadd213ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmadd213ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmadd213ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmadd213ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmadd213ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmadd213ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfmadd213ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmadd213ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmadd213ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmadd213ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmadd213ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfmadd213ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfmadd213ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfmadd213ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfmadd213sd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfmadd213sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfmadd213sd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfmadd213sd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfmadd213sd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfmadd213sd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vfmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vfmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vfmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vfmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vfmadd213ss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfmadd213ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfmadd213ss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfmadd213ss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfmadd213ss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfmadd213ss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vfmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vfmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vfmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vfmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vfmadd231pd	zmm30, zmm29, zmm28	 # AVX512F
	vfmadd231pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmadd231pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmadd231pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmadd231pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmadd231pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmadd231pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmadd231pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmadd231pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmadd231pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfmadd231pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmadd231pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmadd231pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmadd231pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmadd231pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfmadd231pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfmadd231pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfmadd231pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfmadd231ps	zmm30, zmm29, zmm28	 # AVX512F
	vfmadd231ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmadd231ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmadd231ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmadd231ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmadd231ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmadd231ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmadd231ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmadd231ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmadd231ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfmadd231ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmadd231ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmadd231ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmadd231ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmadd231ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfmadd231ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfmadd231ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfmadd231ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfmadd231sd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfmadd231sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfmadd231sd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfmadd231sd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfmadd231sd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfmadd231sd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vfmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vfmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vfmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vfmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vfmadd231ss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfmadd231ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfmadd231ss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfmadd231ss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfmadd231ss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfmadd231ss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vfmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vfmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vfmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vfmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vfmaddsub132pd	zmm30, zmm29, zmm28	 # AVX512F
	vfmaddsub132pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmaddsub132pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmaddsub132pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmaddsub132pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmaddsub132pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmaddsub132pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmaddsub132pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmaddsub132pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmaddsub132pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfmaddsub132pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmaddsub132pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmaddsub132pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmaddsub132pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmaddsub132pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfmaddsub132pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfmaddsub132pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfmaddsub132pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfmaddsub132ps	zmm30, zmm29, zmm28	 # AVX512F
	vfmaddsub132ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmaddsub132ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmaddsub132ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmaddsub132ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmaddsub132ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmaddsub132ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmaddsub132ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmaddsub132ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmaddsub132ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfmaddsub132ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmaddsub132ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmaddsub132ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmaddsub132ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmaddsub132ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfmaddsub132ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfmaddsub132ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfmaddsub132ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfmaddsub213pd	zmm30, zmm29, zmm28	 # AVX512F
	vfmaddsub213pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmaddsub213pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmaddsub213pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmaddsub213pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmaddsub213pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmaddsub213pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmaddsub213pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmaddsub213pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmaddsub213pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfmaddsub213pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmaddsub213pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmaddsub213pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmaddsub213pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmaddsub213pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfmaddsub213pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfmaddsub213pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfmaddsub213pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfmaddsub213ps	zmm30, zmm29, zmm28	 # AVX512F
	vfmaddsub213ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmaddsub213ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmaddsub213ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmaddsub213ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmaddsub213ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmaddsub213ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmaddsub213ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmaddsub213ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmaddsub213ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfmaddsub213ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmaddsub213ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmaddsub213ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmaddsub213ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmaddsub213ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfmaddsub213ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfmaddsub213ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfmaddsub213ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfmaddsub231pd	zmm30, zmm29, zmm28	 # AVX512F
	vfmaddsub231pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmaddsub231pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmaddsub231pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmaddsub231pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmaddsub231pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmaddsub231pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmaddsub231pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmaddsub231pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmaddsub231pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfmaddsub231pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmaddsub231pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmaddsub231pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmaddsub231pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmaddsub231pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfmaddsub231pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfmaddsub231pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfmaddsub231pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfmaddsub231ps	zmm30, zmm29, zmm28	 # AVX512F
	vfmaddsub231ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmaddsub231ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmaddsub231ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmaddsub231ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmaddsub231ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmaddsub231ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmaddsub231ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmaddsub231ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmaddsub231ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfmaddsub231ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmaddsub231ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmaddsub231ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmaddsub231ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmaddsub231ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfmaddsub231ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfmaddsub231ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfmaddsub231ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfmsub132pd	zmm30, zmm29, zmm28	 # AVX512F
	vfmsub132pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmsub132pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmsub132pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmsub132pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmsub132pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmsub132pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmsub132pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmsub132pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsub132pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfmsub132pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmsub132pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmsub132pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmsub132pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmsub132pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfmsub132pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfmsub132pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfmsub132pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfmsub132ps	zmm30, zmm29, zmm28	 # AVX512F
	vfmsub132ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmsub132ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmsub132ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmsub132ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmsub132ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmsub132ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmsub132ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmsub132ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsub132ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfmsub132ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmsub132ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmsub132ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmsub132ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmsub132ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfmsub132ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfmsub132ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfmsub132ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfmsub132sd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfmsub132sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfmsub132sd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfmsub132sd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfmsub132sd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfmsub132sd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vfmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vfmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vfmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vfmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vfmsub132ss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfmsub132ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfmsub132ss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfmsub132ss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfmsub132ss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfmsub132ss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vfmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vfmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vfmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vfmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vfmsub213pd	zmm30, zmm29, zmm28	 # AVX512F
	vfmsub213pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmsub213pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmsub213pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmsub213pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmsub213pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmsub213pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmsub213pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmsub213pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsub213pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfmsub213pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmsub213pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmsub213pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmsub213pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmsub213pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfmsub213pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfmsub213pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfmsub213pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfmsub213ps	zmm30, zmm29, zmm28	 # AVX512F
	vfmsub213ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmsub213ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmsub213ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmsub213ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmsub213ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmsub213ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmsub213ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmsub213ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsub213ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfmsub213ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmsub213ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmsub213ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmsub213ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmsub213ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfmsub213ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfmsub213ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfmsub213ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfmsub213sd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfmsub213sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfmsub213sd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfmsub213sd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfmsub213sd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfmsub213sd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vfmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vfmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vfmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vfmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vfmsub213ss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfmsub213ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfmsub213ss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfmsub213ss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfmsub213ss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfmsub213ss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vfmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vfmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vfmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vfmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vfmsub231pd	zmm30, zmm29, zmm28	 # AVX512F
	vfmsub231pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmsub231pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmsub231pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmsub231pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmsub231pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmsub231pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmsub231pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmsub231pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsub231pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfmsub231pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmsub231pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmsub231pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmsub231pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmsub231pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfmsub231pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfmsub231pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfmsub231pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfmsub231ps	zmm30, zmm29, zmm28	 # AVX512F
	vfmsub231ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmsub231ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmsub231ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmsub231ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmsub231ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmsub231ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmsub231ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmsub231ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsub231ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfmsub231ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmsub231ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmsub231ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmsub231ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmsub231ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfmsub231ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfmsub231ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfmsub231ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfmsub231sd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfmsub231sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfmsub231sd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfmsub231sd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfmsub231sd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfmsub231sd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vfmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vfmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vfmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vfmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vfmsub231ss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfmsub231ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfmsub231ss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfmsub231ss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfmsub231ss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfmsub231ss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vfmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vfmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vfmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vfmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vfmsubadd132pd	zmm30, zmm29, zmm28	 # AVX512F
	vfmsubadd132pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmsubadd132pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmsubadd132pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmsubadd132pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmsubadd132pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmsubadd132pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmsubadd132pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmsubadd132pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsubadd132pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfmsubadd132pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmsubadd132pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmsubadd132pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmsubadd132pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmsubadd132pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfmsubadd132pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfmsubadd132pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfmsubadd132pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfmsubadd132ps	zmm30, zmm29, zmm28	 # AVX512F
	vfmsubadd132ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmsubadd132ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmsubadd132ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmsubadd132ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmsubadd132ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmsubadd132ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmsubadd132ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmsubadd132ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsubadd132ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfmsubadd132ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmsubadd132ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmsubadd132ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmsubadd132ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmsubadd132ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfmsubadd132ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfmsubadd132ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfmsubadd132ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfmsubadd213pd	zmm30, zmm29, zmm28	 # AVX512F
	vfmsubadd213pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmsubadd213pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmsubadd213pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmsubadd213pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmsubadd213pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmsubadd213pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmsubadd213pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmsubadd213pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsubadd213pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfmsubadd213pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmsubadd213pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmsubadd213pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmsubadd213pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmsubadd213pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfmsubadd213pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfmsubadd213pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfmsubadd213pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfmsubadd213ps	zmm30, zmm29, zmm28	 # AVX512F
	vfmsubadd213ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmsubadd213ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmsubadd213ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmsubadd213ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmsubadd213ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmsubadd213ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmsubadd213ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmsubadd213ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsubadd213ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfmsubadd213ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmsubadd213ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmsubadd213ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmsubadd213ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmsubadd213ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfmsubadd213ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfmsubadd213ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfmsubadd213ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfmsubadd231pd	zmm30, zmm29, zmm28	 # AVX512F
	vfmsubadd231pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmsubadd231pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmsubadd231pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmsubadd231pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmsubadd231pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmsubadd231pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmsubadd231pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmsubadd231pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsubadd231pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfmsubadd231pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmsubadd231pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmsubadd231pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmsubadd231pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmsubadd231pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfmsubadd231pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfmsubadd231pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfmsubadd231pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfmsubadd231ps	zmm30, zmm29, zmm28	 # AVX512F
	vfmsubadd231ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfmsubadd231ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfmsubadd231ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfmsubadd231ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfmsubadd231ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfmsubadd231ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfmsubadd231ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfmsubadd231ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfmsubadd231ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfmsubadd231ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfmsubadd231ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfmsubadd231ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfmsubadd231ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfmsubadd231ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfmsubadd231ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfmsubadd231ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfmsubadd231ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfnmadd132pd	zmm30, zmm29, zmm28	 # AVX512F
	vfnmadd132pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfnmadd132pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfnmadd132pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfnmadd132pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfnmadd132pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfnmadd132pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfnmadd132pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfnmadd132pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmadd132pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfnmadd132pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfnmadd132pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfnmadd132pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfnmadd132pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfnmadd132pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfnmadd132pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfnmadd132pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfnmadd132pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfnmadd132ps	zmm30, zmm29, zmm28	 # AVX512F
	vfnmadd132ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfnmadd132ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfnmadd132ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfnmadd132ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfnmadd132ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfnmadd132ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfnmadd132ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfnmadd132ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmadd132ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfnmadd132ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfnmadd132ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfnmadd132ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfnmadd132ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfnmadd132ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfnmadd132ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfnmadd132ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfnmadd132ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfnmadd132sd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfnmadd132sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfnmadd132sd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfnmadd132sd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfnmadd132sd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfnmadd132sd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfnmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vfnmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vfnmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vfnmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vfnmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vfnmadd132ss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfnmadd132ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfnmadd132ss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfnmadd132ss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfnmadd132ss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfnmadd132ss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfnmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vfnmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vfnmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vfnmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vfnmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vfnmadd213pd	zmm30, zmm29, zmm28	 # AVX512F
	vfnmadd213pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfnmadd213pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfnmadd213pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfnmadd213pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfnmadd213pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfnmadd213pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfnmadd213pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfnmadd213pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmadd213pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfnmadd213pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfnmadd213pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfnmadd213pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfnmadd213pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfnmadd213pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfnmadd213pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfnmadd213pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfnmadd213pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfnmadd213ps	zmm30, zmm29, zmm28	 # AVX512F
	vfnmadd213ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfnmadd213ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfnmadd213ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfnmadd213ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfnmadd213ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfnmadd213ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfnmadd213ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfnmadd213ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmadd213ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfnmadd213ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfnmadd213ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfnmadd213ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfnmadd213ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfnmadd213ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfnmadd213ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfnmadd213ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfnmadd213ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfnmadd213sd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfnmadd213sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfnmadd213sd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfnmadd213sd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfnmadd213sd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfnmadd213sd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfnmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vfnmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vfnmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vfnmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vfnmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vfnmadd213ss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfnmadd213ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfnmadd213ss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfnmadd213ss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfnmadd213ss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfnmadd213ss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfnmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vfnmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vfnmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vfnmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vfnmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vfnmadd231pd	zmm30, zmm29, zmm28	 # AVX512F
	vfnmadd231pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfnmadd231pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfnmadd231pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfnmadd231pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfnmadd231pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfnmadd231pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfnmadd231pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfnmadd231pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmadd231pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfnmadd231pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfnmadd231pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfnmadd231pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfnmadd231pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfnmadd231pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfnmadd231pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfnmadd231pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfnmadd231pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfnmadd231ps	zmm30, zmm29, zmm28	 # AVX512F
	vfnmadd231ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfnmadd231ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfnmadd231ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfnmadd231ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfnmadd231ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfnmadd231ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfnmadd231ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfnmadd231ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmadd231ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfnmadd231ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfnmadd231ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfnmadd231ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfnmadd231ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfnmadd231ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfnmadd231ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfnmadd231ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfnmadd231ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfnmadd231sd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfnmadd231sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfnmadd231sd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfnmadd231sd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfnmadd231sd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfnmadd231sd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfnmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vfnmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vfnmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vfnmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vfnmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vfnmadd231ss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfnmadd231ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfnmadd231ss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfnmadd231ss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfnmadd231ss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfnmadd231ss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfnmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vfnmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vfnmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vfnmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vfnmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vfnmsub132pd	zmm30, zmm29, zmm28	 # AVX512F
	vfnmsub132pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfnmsub132pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfnmsub132pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfnmsub132pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfnmsub132pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfnmsub132pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfnmsub132pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfnmsub132pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmsub132pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfnmsub132pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfnmsub132pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfnmsub132pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfnmsub132pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfnmsub132pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfnmsub132pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfnmsub132pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfnmsub132pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfnmsub132ps	zmm30, zmm29, zmm28	 # AVX512F
	vfnmsub132ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfnmsub132ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfnmsub132ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfnmsub132ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfnmsub132ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfnmsub132ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfnmsub132ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfnmsub132ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmsub132ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfnmsub132ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfnmsub132ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfnmsub132ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfnmsub132ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfnmsub132ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfnmsub132ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfnmsub132ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfnmsub132ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfnmsub132sd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfnmsub132sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfnmsub132sd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfnmsub132sd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfnmsub132sd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfnmsub132sd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfnmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vfnmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vfnmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vfnmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vfnmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vfnmsub132ss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfnmsub132ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfnmsub132ss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfnmsub132ss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfnmsub132ss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfnmsub132ss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfnmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vfnmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vfnmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vfnmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vfnmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vfnmsub213pd	zmm30, zmm29, zmm28	 # AVX512F
	vfnmsub213pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfnmsub213pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfnmsub213pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfnmsub213pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfnmsub213pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfnmsub213pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfnmsub213pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfnmsub213pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmsub213pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfnmsub213pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfnmsub213pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfnmsub213pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfnmsub213pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfnmsub213pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfnmsub213pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfnmsub213pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfnmsub213pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfnmsub213ps	zmm30, zmm29, zmm28	 # AVX512F
	vfnmsub213ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfnmsub213ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfnmsub213ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfnmsub213ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfnmsub213ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfnmsub213ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfnmsub213ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfnmsub213ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmsub213ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfnmsub213ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfnmsub213ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfnmsub213ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfnmsub213ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfnmsub213ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfnmsub213ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfnmsub213ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfnmsub213ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfnmsub213sd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfnmsub213sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfnmsub213sd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfnmsub213sd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfnmsub213sd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfnmsub213sd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfnmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vfnmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vfnmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vfnmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vfnmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vfnmsub213ss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfnmsub213ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfnmsub213ss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfnmsub213ss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfnmsub213ss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfnmsub213ss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfnmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vfnmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vfnmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vfnmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vfnmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vfnmsub231pd	zmm30, zmm29, zmm28	 # AVX512F
	vfnmsub231pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfnmsub231pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfnmsub231pd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfnmsub231pd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfnmsub231pd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfnmsub231pd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfnmsub231pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfnmsub231pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmsub231pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vfnmsub231pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfnmsub231pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfnmsub231pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfnmsub231pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfnmsub231pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vfnmsub231pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vfnmsub231pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vfnmsub231pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vfnmsub231ps	zmm30, zmm29, zmm28	 # AVX512F
	vfnmsub231ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vfnmsub231ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vfnmsub231ps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vfnmsub231ps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vfnmsub231ps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vfnmsub231ps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vfnmsub231ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vfnmsub231ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmsub231ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vfnmsub231ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vfnmsub231ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vfnmsub231ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vfnmsub231ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vfnmsub231ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vfnmsub231ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vfnmsub231ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vfnmsub231ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vfnmsub231sd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfnmsub231sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfnmsub231sd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfnmsub231sd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfnmsub231sd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfnmsub231sd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfnmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vfnmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vfnmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vfnmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vfnmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vfnmsub231ss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vfnmsub231ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vfnmsub231ss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vfnmsub231ss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vfnmsub231ss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vfnmsub231ss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vfnmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vfnmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vfnmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vfnmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vfnmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vfnmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vgatherdpd	zmm30{k1}, [r14+ymm31*8-123]	 # AVX512F
	vgatherdpd	zmm30{k1}, qword ptr [r14+ymm31*8-123]	 # AVX512F
	vgatherdpd	zmm30{k1}, [r9+ymm31+256]	 # AVX512F
	vgatherdpd	zmm30{k1}, [rcx+ymm31*4+1024]	 # AVX512F

	vgatherdps	zmm30{k1}, [r14+zmm31*8-123]	 # AVX512F
	vgatherdps	zmm30{k1}, dword ptr [r14+zmm31*8-123]	 # AVX512F
	vgatherdps	zmm30{k1}, [r9+zmm31+256]	 # AVX512F
	vgatherdps	zmm30{k1}, [rcx+zmm31*4+1024]	 # AVX512F

	vgatherqpd	zmm30{k1}, [r14+zmm31*8-123]	 # AVX512F
	vgatherqpd	zmm30{k1}, qword ptr [r14+zmm31*8-123]	 # AVX512F
	vgatherqpd	zmm30{k1}, [r9+zmm31+256]	 # AVX512F
	vgatherqpd	zmm30{k1}, [rcx+zmm31*4+1024]	 # AVX512F
	vgatherqpd	zmm3{k1}, [r14+zmm19*8+123]	 # AVX512F

	vgatherqps	ymm30{k1}, [r14+zmm31*8-123]	 # AVX512F
	vgatherqps	ymm30{k1}, dword ptr [r14+zmm31*8-123]	 # AVX512F
	vgatherqps	ymm30{k1}, [r9+zmm31+256]	 # AVX512F
	vgatherqps	ymm30{k1}, [rcx+zmm31*4+1024]	 # AVX512F

	vgetexppd	zmm30, zmm29	 # AVX512F
	vgetexppd	zmm30{k7}, zmm29	 # AVX512F
	vgetexppd	zmm30{k7}{z}, zmm29	 # AVX512F
	vgetexppd	zmm30, zmm29{sae}	 # AVX512F
	vgetexppd	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vgetexppd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vgetexppd	zmm30, qword bcst [rcx]	 # AVX512F
	vgetexppd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vgetexppd	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vgetexppd	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vgetexppd	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vgetexppd	zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vgetexppd	zmm30, qword bcst [rdx+1024]	 # AVX512F
	vgetexppd	zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vgetexppd	zmm30, qword bcst [rdx-1032]	 # AVX512F

	vgetexpps	zmm30, zmm29	 # AVX512F
	vgetexpps	zmm30{k7}, zmm29	 # AVX512F
	vgetexpps	zmm30{k7}{z}, zmm29	 # AVX512F
	vgetexpps	zmm30, zmm29{sae}	 # AVX512F
	vgetexpps	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vgetexpps	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vgetexpps	zmm30, dword bcst [rcx]	 # AVX512F
	vgetexpps	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vgetexpps	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vgetexpps	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vgetexpps	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vgetexpps	zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vgetexpps	zmm30, dword bcst [rdx+512]	 # AVX512F
	vgetexpps	zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vgetexpps	zmm30, dword bcst [rdx-516]	 # AVX512F

	vgetexpsd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vgetexpsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vgetexpsd	xmm30{k7}, xmm29, xmm28{sae}	 # AVX512F
	vgetexpsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vgetexpsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vgetexpsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vgetexpsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vgetexpsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vgetexpsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vgetexpss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vgetexpss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vgetexpss	xmm30{k7}, xmm29, xmm28{sae}	 # AVX512F
	vgetexpss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vgetexpss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vgetexpss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vgetexpss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vgetexpss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vgetexpss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vgetmantpd	zmm30, zmm29, 0xab	 # AVX512F
	vgetmantpd	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vgetmantpd	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vgetmantpd	zmm30, zmm29{sae}, 0xab	 # AVX512F
	vgetmantpd	zmm30, zmm29, 123	 # AVX512F
	vgetmantpd	zmm30, zmm29{sae}, 123	 # AVX512F
	vgetmantpd	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vgetmantpd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vgetmantpd	zmm30, qword bcst [rcx], 123	 # AVX512F
	vgetmantpd	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vgetmantpd	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vgetmantpd	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vgetmantpd	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vgetmantpd	zmm30, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vgetmantpd	zmm30, qword bcst [rdx+1024], 123	 # AVX512F
	vgetmantpd	zmm30, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vgetmantpd	zmm30, qword bcst [rdx-1032], 123	 # AVX512F

	vgetmantps	zmm30, zmm29, 0xab	 # AVX512F
	vgetmantps	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vgetmantps	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vgetmantps	zmm30, zmm29{sae}, 0xab	 # AVX512F
	vgetmantps	zmm30, zmm29, 123	 # AVX512F
	vgetmantps	zmm30, zmm29{sae}, 123	 # AVX512F
	vgetmantps	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vgetmantps	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vgetmantps	zmm30, dword bcst [rcx], 123	 # AVX512F
	vgetmantps	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vgetmantps	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vgetmantps	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vgetmantps	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vgetmantps	zmm30, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vgetmantps	zmm30, dword bcst [rdx+512], 123	 # AVX512F
	vgetmantps	zmm30, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vgetmantps	zmm30, dword bcst [rdx-516], 123	 # AVX512F

	vgetmantsd	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512F
	vgetmantsd	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512F
	vgetmantsd	xmm30{k7}, xmm29, xmm28{sae}, 0xab	 # AVX512F
	vgetmantsd	xmm30{k7}, xmm29, xmm28, 123	 # AVX512F
	vgetmantsd	xmm30{k7}, xmm29, xmm28{sae}, 123	 # AVX512F
	vgetmantsd	xmm30{k7}, xmm29, QWORD PTR [rcx], 123	 # AVX512F
	vgetmantsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vgetmantsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016], 123	 # AVX512F Disp8
	vgetmantsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024], 123	 # AVX512F
	vgetmantsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024], 123	 # AVX512F Disp8
	vgetmantsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032], 123	 # AVX512F

	vgetmantss	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512F
	vgetmantss	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512F
	vgetmantss	xmm30{k7}, xmm29, xmm28{sae}, 0xab	 # AVX512F
	vgetmantss	xmm30{k7}, xmm29, xmm28, 123	 # AVX512F
	vgetmantss	xmm30{k7}, xmm29, xmm28{sae}, 123	 # AVX512F
	vgetmantss	xmm30{k7}, xmm29, DWORD PTR [rcx], 123	 # AVX512F
	vgetmantss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vgetmantss	xmm30{k7}, xmm29, DWORD PTR [rdx+508], 123	 # AVX512F Disp8
	vgetmantss	xmm30{k7}, xmm29, DWORD PTR [rdx+512], 123	 # AVX512F
	vgetmantss	xmm30{k7}, xmm29, DWORD PTR [rdx-512], 123	 # AVX512F Disp8
	vgetmantss	xmm30{k7}, xmm29, DWORD PTR [rdx-516], 123	 # AVX512F

	vinsertf32x4	zmm30{k7}, zmm29, xmm28, 0xab	 # AVX512F
	vinsertf32x4	zmm30{k7}{z}, zmm29, xmm28, 0xab	 # AVX512F
	vinsertf32x4	zmm30{k7}, zmm29, xmm28, 123	 # AVX512F
	vinsertf32x4	zmm30{k7}, zmm29, XMMWORD PTR [rcx], 123	 # AVX512F
	vinsertf32x4	zmm30{k7}, zmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vinsertf32x4	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512F Disp8
	vinsertf32x4	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512F
	vinsertf32x4	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512F Disp8
	vinsertf32x4	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512F

	vinsertf64x4	zmm30{k7}, zmm29, ymm28, 0xab	 # AVX512F
	vinsertf64x4	zmm30{k7}{z}, zmm29, ymm28, 0xab	 # AVX512F
	vinsertf64x4	zmm30{k7}, zmm29, ymm28, 123	 # AVX512F
	vinsertf64x4	zmm30{k7}, zmm29, YMMWORD PTR [rcx], 123	 # AVX512F
	vinsertf64x4	zmm30{k7}, zmm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vinsertf64x4	zmm30{k7}, zmm29, YMMWORD PTR [rdx+4064], 123	 # AVX512F Disp8
	vinsertf64x4	zmm30{k7}, zmm29, YMMWORD PTR [rdx+4096], 123	 # AVX512F
	vinsertf64x4	zmm30{k7}, zmm29, YMMWORD PTR [rdx-4096], 123	 # AVX512F Disp8
	vinsertf64x4	zmm30{k7}, zmm29, YMMWORD PTR [rdx-4128], 123	 # AVX512F

	vinserti32x4	zmm30{k7}, zmm29, xmm28, 0xab	 # AVX512F
	vinserti32x4	zmm30{k7}{z}, zmm29, xmm28, 0xab	 # AVX512F
	vinserti32x4	zmm30{k7}, zmm29, xmm28, 123	 # AVX512F
	vinserti32x4	zmm30{k7}, zmm29, XMMWORD PTR [rcx], 123	 # AVX512F
	vinserti32x4	zmm30{k7}, zmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vinserti32x4	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512F Disp8
	vinserti32x4	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512F
	vinserti32x4	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512F Disp8
	vinserti32x4	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512F

	vinserti64x4	zmm30{k7}, zmm29, ymm28, 0xab	 # AVX512F
	vinserti64x4	zmm30{k7}{z}, zmm29, ymm28, 0xab	 # AVX512F
	vinserti64x4	zmm30{k7}, zmm29, ymm28, 123	 # AVX512F
	vinserti64x4	zmm30{k7}, zmm29, YMMWORD PTR [rcx], 123	 # AVX512F
	vinserti64x4	zmm30{k7}, zmm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vinserti64x4	zmm30{k7}, zmm29, YMMWORD PTR [rdx+4064], 123	 # AVX512F Disp8
	vinserti64x4	zmm30{k7}, zmm29, YMMWORD PTR [rdx+4096], 123	 # AVX512F
	vinserti64x4	zmm30{k7}, zmm29, YMMWORD PTR [rdx-4096], 123	 # AVX512F Disp8
	vinserti64x4	zmm30{k7}, zmm29, YMMWORD PTR [rdx-4128], 123	 # AVX512F

	vinsertps	xmm30, xmm29, xmm28, 0xab	 # AVX512F
	vinsertps	xmm30, xmm29, xmm28, 123	 # AVX512F
	vinsertps	xmm30, xmm29, DWORD PTR [rcx], 123	 # AVX512F
	vinsertps	xmm30, xmm29, DWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vinsertps	xmm30, xmm29, DWORD PTR [rdx+508], 123	 # AVX512F Disp8
	vinsertps	xmm30, xmm29, DWORD PTR [rdx+512], 123	 # AVX512F
	vinsertps	xmm30, xmm29, DWORD PTR [rdx-512], 123	 # AVX512F Disp8
	vinsertps	xmm30, xmm29, DWORD PTR [rdx-516], 123	 # AVX512F

	vmaxpd	zmm30, zmm29, zmm28	 # AVX512F
	vmaxpd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vmaxpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vmaxpd	zmm30, zmm29, zmm28{sae}	 # AVX512F
	vmaxpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vmaxpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmaxpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vmaxpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmaxpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmaxpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmaxpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vmaxpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vmaxpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vmaxpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vmaxpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vmaxps	zmm30, zmm29, zmm28	 # AVX512F
	vmaxps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vmaxps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vmaxps	zmm30, zmm29, zmm28{sae}	 # AVX512F
	vmaxps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vmaxps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmaxps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vmaxps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmaxps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmaxps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmaxps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vmaxps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vmaxps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vmaxps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vmaxps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vmaxsd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vmaxsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vmaxsd	xmm30{k7}, xmm29, xmm28{sae}	 # AVX512F
	vmaxsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vmaxsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmaxsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vmaxsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vmaxsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vmaxsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vmaxss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vmaxss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vmaxss	xmm30{k7}, xmm29, xmm28{sae}	 # AVX512F
	vmaxss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vmaxss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmaxss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vmaxss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vmaxss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vmaxss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vminpd	zmm30, zmm29, zmm28	 # AVX512F
	vminpd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vminpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vminpd	zmm30, zmm29, zmm28{sae}	 # AVX512F
	vminpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vminpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vminpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vminpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vminpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vminpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vminpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vminpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vminpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vminpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vminpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vminps	zmm30, zmm29, zmm28	 # AVX512F
	vminps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vminps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vminps	zmm30, zmm29, zmm28{sae}	 # AVX512F
	vminps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vminps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vminps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vminps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vminps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vminps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vminps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vminps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vminps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vminps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vminps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vminsd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vminsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vminsd	xmm30{k7}, xmm29, xmm28{sae}	 # AVX512F
	vminsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vminsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vminsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vminsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vminsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vminsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vminss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vminss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vminss	xmm30{k7}, xmm29, xmm28{sae}	 # AVX512F
	vminss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vminss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vminss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vminss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vminss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vminss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vmovapd	zmm30, zmm29	 # AVX512F
	vmovapd	zmm30{k7}, zmm29	 # AVX512F
	vmovapd	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovapd	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vmovapd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovapd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmovapd	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmovapd	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmovapd	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F

	vmovaps	zmm30, zmm29	 # AVX512F
	vmovaps	zmm30{k7}, zmm29	 # AVX512F
	vmovaps	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovaps	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vmovaps	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovaps	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmovaps	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmovaps	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmovaps	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F

	vmovd	xmm30, eax	 # AVX512F
	vmovd	xmm30, ebp	 # AVX512F
	vmovd	xmm30, r13d	 # AVX512F
	vmovd	xmm30, DWORD PTR [rcx]	 # AVX512F
	vmovd	xmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovd	xmm30, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vmovd	xmm30, DWORD PTR [rdx+512]	 # AVX512F
	vmovd	xmm30, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vmovd	xmm30, DWORD PTR [rdx-516]	 # AVX512F

	vmovd	DWORD PTR [rcx], xmm30	 # AVX512F
	vmovd	DWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512F
	vmovd	DWORD PTR [rdx+508], xmm30	 # AVX512F Disp8
	vmovd	DWORD PTR [rdx+512], xmm30	 # AVX512F
	vmovd	DWORD PTR [rdx-512], xmm30	 # AVX512F Disp8
	vmovd	DWORD PTR [rdx-516], xmm30	 # AVX512F

	vmovddup	zmm30, zmm29	 # AVX512F
	vmovddup	zmm30{k7}, zmm29	 # AVX512F
	vmovddup	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovddup	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vmovddup	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovddup	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmovddup	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmovddup	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmovddup	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F

	vmovdqa32	zmm30, zmm29	 # AVX512F
	vmovdqa32	zmm30{k7}, zmm29	 # AVX512F
	vmovdqa32	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovdqa32	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vmovdqa32	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovdqa32	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmovdqa32	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmovdqa32	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmovdqa32	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F

	vmovdqa64	zmm30, zmm29	 # AVX512F
	vmovdqa64	zmm30{k7}, zmm29	 # AVX512F
	vmovdqa64	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovdqa64	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vmovdqa64	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovdqa64	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmovdqa64	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmovdqa64	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmovdqa64	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F

	vmovdqu32	zmm30, zmm29	 # AVX512F
	vmovdqu32	zmm30{k7}, zmm29	 # AVX512F
	vmovdqu32	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovdqu32	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vmovdqu32	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovdqu32	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmovdqu32	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmovdqu32	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmovdqu32	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F

	vmovdqu64	zmm30, zmm29	 # AVX512F
	vmovdqu64	zmm30{k7}, zmm29	 # AVX512F
	vmovdqu64	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovdqu64	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vmovdqu64	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovdqu64	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmovdqu64	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmovdqu64	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmovdqu64	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F

	vmovhlps	xmm30, xmm29, xmm28	 # AVX512F

	vmovhpd	xmm29, xmm30, QWORD PTR [rcx]	 # AVX512F
	vmovhpd	xmm29, xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovhpd	xmm29, xmm30, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vmovhpd	xmm29, xmm30, QWORD PTR [rdx+1024]	 # AVX512F
	vmovhpd	xmm29, xmm30, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vmovhpd	xmm29, xmm30, QWORD PTR [rdx-1032]	 # AVX512F

	vmovhpd	QWORD PTR [rcx], xmm30	 # AVX512F
	vmovhpd	QWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512F
	vmovhpd	QWORD PTR [rdx+1016], xmm30	 # AVX512F Disp8
	vmovhpd	QWORD PTR [rdx+1024], xmm30	 # AVX512F
	vmovhpd	QWORD PTR [rdx-1024], xmm30	 # AVX512F Disp8
	vmovhpd	QWORD PTR [rdx-1032], xmm30	 # AVX512F

	vmovhps	xmm29, xmm30, QWORD PTR [rcx]	 # AVX512F
	vmovhps	xmm29, xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovhps	xmm29, xmm30, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vmovhps	xmm29, xmm30, QWORD PTR [rdx+1024]	 # AVX512F
	vmovhps	xmm29, xmm30, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vmovhps	xmm29, xmm30, QWORD PTR [rdx-1032]	 # AVX512F

	vmovhps	QWORD PTR [rcx], xmm30	 # AVX512F
	vmovhps	QWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512F
	vmovhps	QWORD PTR [rdx+1016], xmm30	 # AVX512F Disp8
	vmovhps	QWORD PTR [rdx+1024], xmm30	 # AVX512F
	vmovhps	QWORD PTR [rdx-1024], xmm30	 # AVX512F Disp8
	vmovhps	QWORD PTR [rdx-1032], xmm30	 # AVX512F

	vmovlhps	xmm30, xmm29, xmm28	 # AVX512F

	vmovlpd	xmm29, xmm30, QWORD PTR [rcx]	 # AVX512F
	vmovlpd	xmm29, xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovlpd	xmm29, xmm30, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vmovlpd	xmm29, xmm30, QWORD PTR [rdx+1024]	 # AVX512F
	vmovlpd	xmm29, xmm30, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vmovlpd	xmm29, xmm30, QWORD PTR [rdx-1032]	 # AVX512F

	vmovlpd	QWORD PTR [rcx], xmm30	 # AVX512F
	vmovlpd	QWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512F
	vmovlpd	QWORD PTR [rdx+1016], xmm30	 # AVX512F Disp8
	vmovlpd	QWORD PTR [rdx+1024], xmm30	 # AVX512F
	vmovlpd	QWORD PTR [rdx-1024], xmm30	 # AVX512F Disp8
	vmovlpd	QWORD PTR [rdx-1032], xmm30	 # AVX512F

	vmovlps	xmm29, xmm30, QWORD PTR [rcx]	 # AVX512F
	vmovlps	xmm29, xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovlps	xmm29, xmm30, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vmovlps	xmm29, xmm30, QWORD PTR [rdx+1024]	 # AVX512F
	vmovlps	xmm29, xmm30, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vmovlps	xmm29, xmm30, QWORD PTR [rdx-1032]	 # AVX512F

	vmovlps	QWORD PTR [rcx], xmm30	 # AVX512F
	vmovlps	QWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512F
	vmovlps	QWORD PTR [rdx+1016], xmm30	 # AVX512F Disp8
	vmovlps	QWORD PTR [rdx+1024], xmm30	 # AVX512F
	vmovlps	QWORD PTR [rdx-1024], xmm30	 # AVX512F Disp8
	vmovlps	QWORD PTR [rdx-1032], xmm30	 # AVX512F

	vmovntdq	ZMMWORD PTR [rcx], zmm30	 # AVX512F
	vmovntdq	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vmovntdq	ZMMWORD PTR [rdx+8128], zmm30	 # AVX512F Disp8
	vmovntdq	ZMMWORD PTR [rdx+8192], zmm30	 # AVX512F
	vmovntdq	ZMMWORD PTR [rdx-8192], zmm30	 # AVX512F Disp8
	vmovntdq	ZMMWORD PTR [rdx-8256], zmm30	 # AVX512F

	vmovntdqa	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vmovntdqa	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovntdqa	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmovntdqa	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmovntdqa	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmovntdqa	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F

	vmovntpd	ZMMWORD PTR [rcx], zmm30	 # AVX512F
	vmovntpd	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vmovntpd	ZMMWORD PTR [rdx+8128], zmm30	 # AVX512F Disp8
	vmovntpd	ZMMWORD PTR [rdx+8192], zmm30	 # AVX512F
	vmovntpd	ZMMWORD PTR [rdx-8192], zmm30	 # AVX512F Disp8
	vmovntpd	ZMMWORD PTR [rdx-8256], zmm30	 # AVX512F

	vmovntps	ZMMWORD PTR [rcx], zmm30	 # AVX512F
	vmovntps	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vmovntps	ZMMWORD PTR [rdx+8128], zmm30	 # AVX512F Disp8
	vmovntps	ZMMWORD PTR [rdx+8192], zmm30	 # AVX512F
	vmovntps	ZMMWORD PTR [rdx-8192], zmm30	 # AVX512F Disp8
	vmovntps	ZMMWORD PTR [rdx-8256], zmm30	 # AVX512F

	vmovq	xmm30, rax	 # AVX512F
	vmovq	xmm30, r8	 # AVX512F
	vmovq	xmm30, QWORD PTR [rcx]	 # AVX512F
	vmovq	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovq	xmm30, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vmovq	xmm30, QWORD PTR [rdx+1024]	 # AVX512F
	vmovq	xmm30, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vmovq	xmm30, QWORD PTR [rdx-1032]	 # AVX512F

	vmovq	QWORD PTR [rcx], xmm30	 # AVX512F
	vmovq	QWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512F
	vmovq	QWORD PTR [rdx+1016], xmm30	 # AVX512F Disp8
	vmovq	QWORD PTR [rdx+1024], xmm30	 # AVX512F
	vmovq	QWORD PTR [rdx-1024], xmm30	 # AVX512F Disp8
	vmovq	QWORD PTR [rdx-1032], xmm30	 # AVX512F

	vmovq	xmm30, xmm29	 # AVX512F
	vmovq	xmm30, QWORD PTR [rcx]	 # AVX512F
	vmovq	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovq	xmm30, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vmovq	xmm30, QWORD PTR [rdx+1024]	 # AVX512F
	vmovq	xmm30, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vmovq	xmm30, QWORD PTR [rdx-1032]	 # AVX512F

	vmovq	QWORD PTR [rcx], xmm29	 # AVX512F
	vmovq	QWORD PTR [rax+r14*8+0x1234], xmm29	 # AVX512F
	vmovq	QWORD PTR [rdx+1016], xmm29	 # AVX512F Disp8
	vmovq	QWORD PTR [rdx+1024], xmm29	 # AVX512F
	vmovq	QWORD PTR [rdx-1024], xmm29	 # AVX512F Disp8
	vmovq	QWORD PTR [rdx-1032], xmm29	 # AVX512F

	vmovsd	xmm30{k7}, QWORD PTR [rcx]	 # AVX512F
	vmovsd	xmm30{k7}{z}, QWORD PTR [rcx]	 # AVX512F
	vmovsd	xmm30{k7}, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovsd	xmm30{k7}, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vmovsd	xmm30{k7}, QWORD PTR [rdx+1024]	 # AVX512F
	vmovsd	xmm30{k7}, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vmovsd	xmm30{k7}, QWORD PTR [rdx-1032]	 # AVX512F

	vmovsd	QWORD PTR [rcx]{k7}, xmm30	 # AVX512F
	vmovsd	QWORD PTR [rax+r14*8+0x1234]{k7}, xmm30	 # AVX512F
	vmovsd	QWORD PTR [rdx+1016]{k7}, xmm30	 # AVX512F Disp8
	vmovsd	QWORD PTR [rdx+1024]{k7}, xmm30	 # AVX512F
	vmovsd	QWORD PTR [rdx-1024]{k7}, xmm30	 # AVX512F Disp8
	vmovsd	QWORD PTR [rdx-1032]{k7}, xmm30	 # AVX512F

	vmovsd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vmovsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F

	vmovshdup	zmm30, zmm29	 # AVX512F
	vmovshdup	zmm30{k7}, zmm29	 # AVX512F
	vmovshdup	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovshdup	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vmovshdup	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovshdup	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmovshdup	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmovshdup	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmovshdup	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F

	vmovsldup	zmm30, zmm29	 # AVX512F
	vmovsldup	zmm30{k7}, zmm29	 # AVX512F
	vmovsldup	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovsldup	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vmovsldup	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovsldup	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmovsldup	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmovsldup	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmovsldup	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F

	vmovss	xmm30{k7}, DWORD PTR [rcx]	 # AVX512F
	vmovss	xmm30{k7}{z}, DWORD PTR [rcx]	 # AVX512F
	vmovss	xmm30{k7}, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovss	xmm30{k7}, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vmovss	xmm30{k7}, DWORD PTR [rdx+512]	 # AVX512F
	vmovss	xmm30{k7}, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vmovss	xmm30{k7}, DWORD PTR [rdx-516]	 # AVX512F

	vmovss	DWORD PTR [rcx]{k7}, xmm30	 # AVX512F
	vmovss	DWORD PTR [rax+r14*8+0x1234]{k7}, xmm30	 # AVX512F
	vmovss	DWORD PTR [rdx+508]{k7}, xmm30	 # AVX512F Disp8
	vmovss	DWORD PTR [rdx+512]{k7}, xmm30	 # AVX512F
	vmovss	DWORD PTR [rdx-512]{k7}, xmm30	 # AVX512F Disp8
	vmovss	DWORD PTR [rdx-516]{k7}, xmm30	 # AVX512F

	vmovss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vmovss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F

	vmovupd	zmm30, zmm29	 # AVX512F
	vmovupd	zmm30{k7}, zmm29	 # AVX512F
	vmovupd	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovupd	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vmovupd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovupd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmovupd	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmovupd	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmovupd	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F

	vmovups	zmm30, zmm29	 # AVX512F
	vmovups	zmm30{k7}, zmm29	 # AVX512F
	vmovups	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovups	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vmovups	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmovups	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmovups	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmovups	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmovups	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F

	vmulpd	zmm30, zmm29, zmm28	 # AVX512F
	vmulpd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vmulpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vmulpd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vmulpd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vmulpd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vmulpd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vmulpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vmulpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmulpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vmulpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmulpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmulpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmulpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vmulpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vmulpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vmulpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vmulpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vmulps	zmm30, zmm29, zmm28	 # AVX512F
	vmulps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vmulps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vmulps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vmulps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vmulps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vmulps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vmulps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vmulps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmulps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vmulps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vmulps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vmulps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vmulps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vmulps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vmulps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vmulps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vmulps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vmulsd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vmulsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vmulsd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vmulsd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vmulsd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vmulsd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vmulsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vmulsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmulsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vmulsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vmulsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vmulsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vmulss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vmulss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vmulss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vmulss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vmulss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vmulss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vmulss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vmulss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vmulss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vmulss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vmulss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vmulss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vpabsd	zmm30, zmm29	 # AVX512F
	vpabsd	zmm30{k7}, zmm29	 # AVX512F
	vpabsd	zmm30{k7}{z}, zmm29	 # AVX512F
	vpabsd	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpabsd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpabsd	zmm30, dword bcst [rcx]	 # AVX512F
	vpabsd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpabsd	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpabsd	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpabsd	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpabsd	zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vpabsd	zmm30, dword bcst [rdx+512]	 # AVX512F
	vpabsd	zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vpabsd	zmm30, dword bcst [rdx-516]	 # AVX512F

	vpabsq	zmm30, zmm29	 # AVX512F
	vpabsq	zmm30{k7}, zmm29	 # AVX512F
	vpabsq	zmm30{k7}{z}, zmm29	 # AVX512F
	vpabsq	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpabsq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpabsq	zmm30, qword bcst [rcx]	 # AVX512F
	vpabsq	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpabsq	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpabsq	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpabsq	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpabsq	zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpabsq	zmm30, qword bcst [rdx+1024]	 # AVX512F
	vpabsq	zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpabsq	zmm30, qword bcst [rdx-1032]	 # AVX512F

	vpaddd	zmm30, zmm29, zmm28	 # AVX512F
	vpaddd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpaddd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpaddd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpaddd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpaddd	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpaddd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpaddd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpaddd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpaddd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpaddd	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpaddd	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpaddd	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpaddd	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpaddq	zmm30, zmm29, zmm28	 # AVX512F
	vpaddq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpaddq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpaddq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpaddq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpaddq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpaddq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpaddq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpaddq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpaddq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpaddq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpaddq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpaddq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpaddq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpandd	zmm30, zmm29, zmm28	 # AVX512F
	vpandd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpandd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpandd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpandd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpandd	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpandd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpandd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpandd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpandd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpandd	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpandd	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpandd	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpandd	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpandnd	zmm30, zmm29, zmm28	 # AVX512F
	vpandnd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpandnd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpandnd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpandnd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpandnd	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpandnd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpandnd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpandnd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpandnd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpandnd	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpandnd	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpandnd	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpandnd	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpandnq	zmm30, zmm29, zmm28	 # AVX512F
	vpandnq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpandnq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpandnq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpandnq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpandnq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpandnq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpandnq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpandnq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpandnq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpandnq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpandnq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpandnq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpandnq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpandq	zmm30, zmm29, zmm28	 # AVX512F
	vpandq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpandq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpandq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpandq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpandq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpandq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpandq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpandq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpandq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpandq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpandq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpandq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpandq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpblendmd	zmm30, zmm29, zmm28	 # AVX512F
	vpblendmd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpblendmd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpblendmd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpblendmd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpblendmd	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpblendmd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpblendmd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpblendmd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpblendmd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpblendmd	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpblendmd	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpblendmd	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpblendmd	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpbroadcastd	zmm30, DWORD PTR [rcx]	 # AVX512F
	vpbroadcastd	zmm30{k7}, DWORD PTR [rcx]	 # AVX512F
	vpbroadcastd	zmm30{k7}{z}, DWORD PTR [rcx]	 # AVX512F
	vpbroadcastd	zmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpbroadcastd	zmm30, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vpbroadcastd	zmm30, DWORD PTR [rdx+512]	 # AVX512F
	vpbroadcastd	zmm30, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vpbroadcastd	zmm30, DWORD PTR [rdx-516]	 # AVX512F

	vpbroadcastd	zmm30{k7}, xmm29	 # AVX512F
	vpbroadcastd	zmm30{k7}{z}, xmm29	 # AVX512F

	vpbroadcastd	zmm30, eax	 # AVX512F
	vpbroadcastd	zmm30{k7}, eax	 # AVX512F
	vpbroadcastd	zmm30{k7}{z}, eax	 # AVX512F
	vpbroadcastd	zmm30, ebp	 # AVX512F
	vpbroadcastd	zmm30, r13d	 # AVX512F

	vpbroadcastq	zmm30, QWORD PTR [rcx]	 # AVX512F
	vpbroadcastq	zmm30{k7}, QWORD PTR [rcx]	 # AVX512F
	vpbroadcastq	zmm30{k7}{z}, QWORD PTR [rcx]	 # AVX512F
	vpbroadcastq	zmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpbroadcastq	zmm30, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vpbroadcastq	zmm30, QWORD PTR [rdx+1024]	 # AVX512F
	vpbroadcastq	zmm30, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vpbroadcastq	zmm30, QWORD PTR [rdx-1032]	 # AVX512F

	vpbroadcastq	zmm30{k7}, xmm29	 # AVX512F
	vpbroadcastq	zmm30{k7}{z}, xmm29	 # AVX512F

	vpbroadcastq	zmm30, rax	 # AVX512F
	vpbroadcastq	zmm30{k7}, rax	 # AVX512F
	vpbroadcastq	zmm30{k7}{z}, rax	 # AVX512F
	vpbroadcastq	zmm30, r8	 # AVX512F

	vpcmpd	k5, zmm30, zmm29, 0xab	 # AVX512F
	vpcmpd	k5{k7}, zmm30, zmm29, 0xab	 # AVX512F
	vpcmpd	k5, zmm30, zmm29, 123	 # AVX512F
	vpcmpd	k5, zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpcmpd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpcmpd	k5, zmm30, dword bcst [rcx], 123	 # AVX512F
	vpcmpd	k5, zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpcmpd	k5, zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpcmpd	k5, zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpcmpd	k5, zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpcmpd	k5, zmm30, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vpcmpd	k5, zmm30, dword bcst [rdx+512], 123	 # AVX512F
	vpcmpd	k5, zmm30, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vpcmpd	k5, zmm30, dword bcst [rdx-516], 123	 # AVX512F

	vpcmpltd	k5, zmm30, zmm29	 # AVX512F
	vpcmpltd	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpltd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpltd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpltd	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vpcmpltd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpltd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpltd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpltd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpltd	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vpcmpltd	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vpcmpltd	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vpcmpltd	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vpcmpled	k5, zmm30, zmm29	 # AVX512F
	vpcmpled	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpled	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpled	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpled	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vpcmpled	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpled	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpled	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpled	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpled	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vpcmpled	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vpcmpled	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vpcmpled	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vpcmpneqd	k5, zmm30, zmm29	 # AVX512F
	vpcmpneqd	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpneqd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpneqd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpneqd	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vpcmpneqd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpneqd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpneqd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpneqd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpneqd	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vpcmpneqd	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vpcmpneqd	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vpcmpneqd	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vpcmpnltd	k5, zmm30, zmm29	 # AVX512F
	vpcmpnltd	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpnltd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpnltd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpnltd	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vpcmpnltd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpnltd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpnltd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpnltd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpnltd	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vpcmpnltd	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vpcmpnltd	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vpcmpnltd	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vpcmpnled	k5, zmm30, zmm29	 # AVX512F
	vpcmpnled	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpnled	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpnled	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpnled	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vpcmpnled	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpnled	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpnled	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpnled	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpnled	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vpcmpnled	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vpcmpnled	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vpcmpnled	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vpcmpeqd	k5, zmm30, zmm29	 # AVX512F
	vpcmpeqd	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpeqd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpeqd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpeqd	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vpcmpeqd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpeqd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpeqd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpeqd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpeqd	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vpcmpeqd	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vpcmpeqd	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vpcmpeqd	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vpcmpeqq	k5, zmm30, zmm29	 # AVX512F
	vpcmpeqq	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpeqq	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpeqq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpeqq	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vpcmpeqq	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpeqq	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpeqq	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpeqq	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpeqq	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpcmpeqq	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vpcmpeqq	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpcmpeqq	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vpcmpgtd	k5, zmm30, zmm29	 # AVX512F
	vpcmpgtd	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpgtd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpgtd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpgtd	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vpcmpgtd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpgtd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpgtd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpgtd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpgtd	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vpcmpgtd	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vpcmpgtd	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vpcmpgtd	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vpcmpgtq	k5, zmm30, zmm29	 # AVX512F
	vpcmpgtq	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpgtq	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpgtq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpgtq	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vpcmpgtq	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpgtq	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpgtq	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpgtq	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpgtq	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpcmpgtq	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vpcmpgtq	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpcmpgtq	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vpcmpq	k5, zmm30, zmm29, 0xab	 # AVX512F
	vpcmpq	k5{k7}, zmm30, zmm29, 0xab	 # AVX512F
	vpcmpq	k5, zmm30, zmm29, 123	 # AVX512F
	vpcmpq	k5, zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpcmpq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpcmpq	k5, zmm30, qword bcst [rcx], 123	 # AVX512F
	vpcmpq	k5, zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpcmpq	k5, zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpcmpq	k5, zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpcmpq	k5, zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpcmpq	k5, zmm30, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vpcmpq	k5, zmm30, qword bcst [rdx+1024], 123	 # AVX512F
	vpcmpq	k5, zmm30, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vpcmpq	k5, zmm30, qword bcst [rdx-1032], 123	 # AVX512F

	vpcmpltq	k5, zmm30, zmm29	 # AVX512F
	vpcmpltq	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpltq	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpltq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpltq	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vpcmpltq	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpltq	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpltq	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpltq	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpltq	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpcmpltq	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vpcmpltq	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpcmpltq	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vpcmpleq	k5, zmm30, zmm29	 # AVX512F
	vpcmpleq	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpleq	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpleq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpleq	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vpcmpleq	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpleq	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpleq	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpleq	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpleq	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpcmpleq	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vpcmpleq	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpcmpleq	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vpcmpneqq	k5, zmm30, zmm29	 # AVX512F
	vpcmpneqq	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpneqq	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpneqq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpneqq	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vpcmpneqq	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpneqq	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpneqq	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpneqq	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpneqq	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpcmpneqq	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vpcmpneqq	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpcmpneqq	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vpcmpnltq	k5, zmm30, zmm29	 # AVX512F
	vpcmpnltq	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpnltq	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpnltq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpnltq	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vpcmpnltq	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpnltq	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpnltq	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpnltq	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpnltq	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpcmpnltq	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vpcmpnltq	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpcmpnltq	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vpcmpnleq	k5, zmm30, zmm29	 # AVX512F
	vpcmpnleq	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpnleq	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpnleq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpnleq	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vpcmpnleq	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpnleq	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpnleq	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpnleq	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpnleq	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpcmpnleq	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vpcmpnleq	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpcmpnleq	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vpcmpud	k5, zmm30, zmm29, 0xab	 # AVX512F
	vpcmpud	k5{k7}, zmm30, zmm29, 0xab	 # AVX512F
	vpcmpud	k5, zmm30, zmm29, 123	 # AVX512F
	vpcmpud	k5, zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpcmpud	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpcmpud	k5, zmm30, dword bcst [rcx], 123	 # AVX512F
	vpcmpud	k5, zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpcmpud	k5, zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpcmpud	k5, zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpcmpud	k5, zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpcmpud	k5, zmm30, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vpcmpud	k5, zmm30, dword bcst [rdx+512], 123	 # AVX512F
	vpcmpud	k5, zmm30, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vpcmpud	k5, zmm30, dword bcst [rdx-516], 123	 # AVX512F

	vpcmpequd	k5, zmm30, zmm29	 # AVX512F
	vpcmpequd	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpequd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpequd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpequd	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vpcmpequd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpequd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpequd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpequd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpequd	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vpcmpequd	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vpcmpequd	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vpcmpequd	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vpcmpltud	k5, zmm30, zmm29	 # AVX512F
	vpcmpltud	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpltud	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpltud	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpltud	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vpcmpltud	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpltud	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpltud	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpltud	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpltud	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vpcmpltud	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vpcmpltud	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vpcmpltud	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vpcmpleud	k5, zmm30, zmm29	 # AVX512F
	vpcmpleud	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpleud	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpleud	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpleud	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vpcmpleud	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpleud	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpleud	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpleud	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpleud	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vpcmpleud	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vpcmpleud	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vpcmpleud	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vpcmpnequd	k5, zmm30, zmm29	 # AVX512F
	vpcmpnequd	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpnequd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpnequd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpnequd	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vpcmpnequd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpnequd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpnequd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpnequd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpnequd	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vpcmpnequd	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vpcmpnequd	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vpcmpnequd	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vpcmpnltud	k5, zmm30, zmm29	 # AVX512F
	vpcmpnltud	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpnltud	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpnltud	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpnltud	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vpcmpnltud	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpnltud	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpnltud	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpnltud	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpnltud	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vpcmpnltud	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vpcmpnltud	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vpcmpnltud	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vpcmpnleud	k5, zmm30, zmm29	 # AVX512F
	vpcmpnleud	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpnleud	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpnleud	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpnleud	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vpcmpnleud	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpnleud	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpnleud	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpnleud	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpnleud	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vpcmpnleud	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vpcmpnleud	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vpcmpnleud	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vpcmpuq	k5, zmm30, zmm29, 0xab	 # AVX512F
	vpcmpuq	k5{k7}, zmm30, zmm29, 0xab	 # AVX512F
	vpcmpuq	k5, zmm30, zmm29, 123	 # AVX512F
	vpcmpuq	k5, zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpcmpuq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpcmpuq	k5, zmm30, qword bcst [rcx], 123	 # AVX512F
	vpcmpuq	k5, zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpcmpuq	k5, zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpcmpuq	k5, zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpcmpuq	k5, zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpcmpuq	k5, zmm30, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vpcmpuq	k5, zmm30, qword bcst [rdx+1024], 123	 # AVX512F
	vpcmpuq	k5, zmm30, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vpcmpuq	k5, zmm30, qword bcst [rdx-1032], 123	 # AVX512F

	vpcmpequq	k5, zmm30, zmm29	 # AVX512F
	vpcmpequq	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpequq	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpequq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpequq	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vpcmpequq	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpequq	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpequq	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpequq	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpequq	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpcmpequq	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vpcmpequq	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpcmpequq	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vpcmpltuq	k5, zmm30, zmm29	 # AVX512F
	vpcmpltuq	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpltuq	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpltuq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpltuq	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vpcmpltuq	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpltuq	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpltuq	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpltuq	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpltuq	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpcmpltuq	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vpcmpltuq	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpcmpltuq	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vpcmpleuq	k5, zmm30, zmm29	 # AVX512F
	vpcmpleuq	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpleuq	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpleuq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpleuq	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vpcmpleuq	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpleuq	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpleuq	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpleuq	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpleuq	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpcmpleuq	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vpcmpleuq	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpcmpleuq	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vpcmpnequq	k5, zmm30, zmm29	 # AVX512F
	vpcmpnequq	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpnequq	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpnequq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpnequq	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vpcmpnequq	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpnequq	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpnequq	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpnequq	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpnequq	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpcmpnequq	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vpcmpnequq	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpcmpnequq	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vpcmpnltuq	k5, zmm30, zmm29	 # AVX512F
	vpcmpnltuq	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpnltuq	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpnltuq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpnltuq	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vpcmpnltuq	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpnltuq	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpnltuq	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpnltuq	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpnltuq	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpcmpnltuq	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vpcmpnltuq	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpcmpnltuq	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vpcmpnleuq	k5, zmm30, zmm29	 # AVX512F
	vpcmpnleuq	k5{k7}, zmm30, zmm29	 # AVX512F
	vpcmpnleuq	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpcmpnleuq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpcmpnleuq	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vpcmpnleuq	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpcmpnleuq	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpcmpnleuq	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpcmpnleuq	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpcmpnleuq	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpcmpnleuq	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vpcmpnleuq	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpcmpnleuq	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vpblendmq	zmm30, zmm29, zmm28	 # AVX512F
	vpblendmq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpblendmq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpblendmq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpblendmq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpblendmq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpblendmq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpblendmq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpblendmq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpblendmq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpblendmq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpblendmq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpblendmq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpblendmq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpcompressd	ZMMWORD PTR [rcx], zmm30	 # AVX512F
	vpcompressd	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpcompressd	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpcompressd	ZMMWORD PTR [rdx+508], zmm30	 # AVX512F Disp8
	vpcompressd	ZMMWORD PTR [rdx+512], zmm30	 # AVX512F
	vpcompressd	ZMMWORD PTR [rdx-512], zmm30	 # AVX512F Disp8
	vpcompressd	ZMMWORD PTR [rdx-516], zmm30	 # AVX512F

	vpcompressd	zmm30, zmm29	 # AVX512F
	vpcompressd	zmm30{k7}, zmm29	 # AVX512F
	vpcompressd	zmm30{k7}{z}, zmm29	 # AVX512F

	vpermd	zmm30, zmm29, zmm28	 # AVX512F
	vpermd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpermd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpermd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpermd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpermd	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpermd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpermd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpermd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpermd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpermd	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpermd	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpermd	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpermd	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpermilpd	zmm30, zmm29, 0xab	 # AVX512F
	vpermilpd	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vpermilpd	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vpermilpd	zmm30, zmm29, 123	 # AVX512F
	vpermilpd	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpermilpd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpermilpd	zmm30, qword bcst [rcx], 123	 # AVX512F
	vpermilpd	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpermilpd	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpermilpd	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpermilpd	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpermilpd	zmm30, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vpermilpd	zmm30, qword bcst [rdx+1024], 123	 # AVX512F
	vpermilpd	zmm30, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vpermilpd	zmm30, qword bcst [rdx-1032], 123	 # AVX512F

	vpermilpd	zmm30, zmm29, zmm28	 # AVX512F
	vpermilpd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpermilpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpermilpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpermilpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpermilpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpermilpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpermilpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpermilpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpermilpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpermilpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpermilpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpermilpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpermilpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpermilps	zmm30, zmm29, 0xab	 # AVX512F
	vpermilps	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vpermilps	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vpermilps	zmm30, zmm29, 123	 # AVX512F
	vpermilps	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpermilps	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpermilps	zmm30, dword bcst [rcx], 123	 # AVX512F
	vpermilps	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpermilps	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpermilps	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpermilps	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpermilps	zmm30, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vpermilps	zmm30, dword bcst [rdx+512], 123	 # AVX512F
	vpermilps	zmm30, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vpermilps	zmm30, dword bcst [rdx-516], 123	 # AVX512F

	vpermilps	zmm30, zmm29, zmm28	 # AVX512F
	vpermilps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpermilps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpermilps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpermilps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpermilps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpermilps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpermilps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpermilps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpermilps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpermilps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpermilps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpermilps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpermilps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpermpd	zmm30, zmm29, 0xab	 # AVX512F
	vpermpd	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vpermpd	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vpermpd	zmm30, zmm29, 123	 # AVX512F
	vpermpd	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpermpd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpermpd	zmm30, qword bcst [rcx], 123	 # AVX512F
	vpermpd	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpermpd	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpermpd	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpermpd	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpermpd	zmm30, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vpermpd	zmm30, qword bcst [rdx+1024], 123	 # AVX512F
	vpermpd	zmm30, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vpermpd	zmm30, qword bcst [rdx-1032], 123	 # AVX512F

	vpermps	zmm30, zmm29, zmm28	 # AVX512F
	vpermps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpermps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpermps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpermps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpermps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpermps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpermps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpermps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpermps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpermps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpermps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpermps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpermps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpermq	zmm30, zmm29, 0xab	 # AVX512F
	vpermq	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vpermq	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vpermq	zmm30, zmm29, 123	 # AVX512F
	vpermq	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpermq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpermq	zmm30, qword bcst [rcx], 123	 # AVX512F
	vpermq	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpermq	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpermq	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpermq	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpermq	zmm30, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vpermq	zmm30, qword bcst [rdx+1024], 123	 # AVX512F
	vpermq	zmm30, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vpermq	zmm30, qword bcst [rdx-1032], 123	 # AVX512F

	vpexpandd	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpexpandd	zmm30{k7}, ZMMWORD PTR [rcx]	 # AVX512F
	vpexpandd	zmm30{k7}{z}, ZMMWORD PTR [rcx]	 # AVX512F
	vpexpandd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpexpandd	zmm30, ZMMWORD PTR [rdx+508]	 # AVX512F Disp8
	vpexpandd	zmm30, ZMMWORD PTR [rdx+512]	 # AVX512F
	vpexpandd	zmm30, ZMMWORD PTR [rdx-512]	 # AVX512F Disp8
	vpexpandd	zmm30, ZMMWORD PTR [rdx-516]	 # AVX512F

	vpexpandd	zmm30, zmm29	 # AVX512F
	vpexpandd	zmm30{k7}, zmm29	 # AVX512F
	vpexpandd	zmm30{k7}{z}, zmm29	 # AVX512F

	vpexpandq	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vpexpandq	zmm30{k7}, ZMMWORD PTR [rcx]	 # AVX512F
	vpexpandq	zmm30{k7}{z}, ZMMWORD PTR [rcx]	 # AVX512F
	vpexpandq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpexpandq	zmm30, ZMMWORD PTR [rdx+1016]	 # AVX512F Disp8
	vpexpandq	zmm30, ZMMWORD PTR [rdx+1024]	 # AVX512F
	vpexpandq	zmm30, ZMMWORD PTR [rdx-1024]	 # AVX512F Disp8
	vpexpandq	zmm30, ZMMWORD PTR [rdx-1032]	 # AVX512F

	vpexpandq	zmm30, zmm29	 # AVX512F
	vpexpandq	zmm30{k7}, zmm29	 # AVX512F
	vpexpandq	zmm30{k7}{z}, zmm29	 # AVX512F

	vpgatherdd	zmm30{k1}, [r14+zmm31*8-123]	 # AVX512F
	vpgatherdd	zmm30{k1}, dword ptr [r14+zmm31*8-123]	 # AVX512F
	vpgatherdd	zmm30{k1}, [r9+zmm31+256]	 # AVX512F
	vpgatherdd	zmm30{k1}, [rcx+zmm31*4+1024]	 # AVX512F

	vpgatherdq	zmm30{k1}, [r14+ymm31*8-123]	 # AVX512F
	vpgatherdq	zmm30{k1}, qword ptr [r14+ymm31*8-123]	 # AVX512F
	vpgatherdq	zmm30{k1}, [r9+ymm31+256]	 # AVX512F
	vpgatherdq	zmm30{k1}, [rcx+ymm31*4+1024]	 # AVX512F

	vpgatherqd	ymm30{k1}, [r14+zmm31*8-123]	 # AVX512F
	vpgatherqd	ymm30{k1}, dword ptr [r14+zmm31*8-123]	 # AVX512F
	vpgatherqd	ymm30{k1}, [r9+zmm31+256]	 # AVX512F
	vpgatherqd	ymm30{k1}, [rcx+zmm31*4+1024]	 # AVX512F

	vpgatherqq	zmm30{k1}, [r14+zmm31*8-123]	 # AVX512F
	vpgatherqq	zmm30{k1}, qword ptr [r14+zmm31*8-123]	 # AVX512F
	vpgatherqq	zmm30{k1}, [r9+zmm31+256]	 # AVX512F
	vpgatherqq	zmm30{k1}, [rcx+zmm31*4+1024]	 # AVX512F

	vpmaxsd	zmm30, zmm29, zmm28	 # AVX512F
	vpmaxsd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpmaxsd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpmaxsd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpmaxsd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmaxsd	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpmaxsd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpmaxsd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpmaxsd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpmaxsd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpmaxsd	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpmaxsd	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpmaxsd	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpmaxsd	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpmaxsq	zmm30, zmm29, zmm28	 # AVX512F
	vpmaxsq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpmaxsq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpmaxsq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpmaxsq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmaxsq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpmaxsq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpmaxsq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpmaxsq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpmaxsq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpmaxsq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpmaxsq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpmaxsq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpmaxsq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpmaxud	zmm30, zmm29, zmm28	 # AVX512F
	vpmaxud	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpmaxud	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpmaxud	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpmaxud	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmaxud	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpmaxud	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpmaxud	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpmaxud	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpmaxud	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpmaxud	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpmaxud	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpmaxud	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpmaxud	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpmaxuq	zmm30, zmm29, zmm28	 # AVX512F
	vpmaxuq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpmaxuq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpmaxuq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpmaxuq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmaxuq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpmaxuq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpmaxuq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpmaxuq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpmaxuq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpmaxuq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpmaxuq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpmaxuq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpmaxuq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpminsd	zmm30, zmm29, zmm28	 # AVX512F
	vpminsd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpminsd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpminsd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpminsd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpminsd	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpminsd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpminsd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpminsd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpminsd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpminsd	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpminsd	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpminsd	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpminsd	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpminsq	zmm30, zmm29, zmm28	 # AVX512F
	vpminsq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpminsq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpminsq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpminsq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpminsq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpminsq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpminsq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpminsq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpminsq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpminsq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpminsq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpminsq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpminsq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpminud	zmm30, zmm29, zmm28	 # AVX512F
	vpminud	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpminud	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpminud	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpminud	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpminud	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpminud	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpminud	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpminud	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpminud	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpminud	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpminud	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpminud	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpminud	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpminuq	zmm30, zmm29, zmm28	 # AVX512F
	vpminuq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpminuq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpminuq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpminuq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpminuq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpminuq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpminuq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpminuq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpminuq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpminuq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpminuq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpminuq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpminuq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpmovsxbd	zmm30{k7}, xmm29	 # AVX512F
	vpmovsxbd	zmm30{k7}{z}, xmm29	 # AVX512F
	vpmovsxbd	zmm30{k7}, XMMWORD PTR [rcx]	 # AVX512F
	vpmovsxbd	zmm30{k7}, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmovsxbd	zmm30{k7}, XMMWORD PTR [rdx+2032]	 # AVX512F Disp8
	vpmovsxbd	zmm30{k7}, XMMWORD PTR [rdx+2048]	 # AVX512F
	vpmovsxbd	zmm30{k7}, XMMWORD PTR [rdx-2048]	 # AVX512F Disp8
	vpmovsxbd	zmm30{k7}, XMMWORD PTR [rdx-2064]	 # AVX512F

	vpmovsxbq	zmm30{k7}, xmm29	 # AVX512F
	vpmovsxbq	zmm30{k7}{z}, xmm29	 # AVX512F
	vpmovsxbq	zmm30{k7}, QWORD PTR [rcx]	 # AVX512F
	vpmovsxbq	zmm30{k7}, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmovsxbq	zmm30{k7}, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vpmovsxbq	zmm30{k7}, QWORD PTR [rdx+1024]	 # AVX512F
	vpmovsxbq	zmm30{k7}, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vpmovsxbq	zmm30{k7}, QWORD PTR [rdx-1032]	 # AVX512F

	vpmovsxdq	zmm30{k7}, ymm29	 # AVX512F
	vpmovsxdq	zmm30{k7}{z}, ymm29	 # AVX512F
	vpmovsxdq	zmm30{k7}, YMMWORD PTR [rcx]	 # AVX512F
	vpmovsxdq	zmm30{k7}, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmovsxdq	zmm30{k7}, YMMWORD PTR [rdx+4064]	 # AVX512F Disp8
	vpmovsxdq	zmm30{k7}, YMMWORD PTR [rdx+4096]	 # AVX512F
	vpmovsxdq	zmm30{k7}, YMMWORD PTR [rdx-4096]	 # AVX512F Disp8
	vpmovsxdq	zmm30{k7}, YMMWORD PTR [rdx-4128]	 # AVX512F

	vpmovsxwd	zmm30{k7}, ymm29	 # AVX512F
	vpmovsxwd	zmm30{k7}{z}, ymm29	 # AVX512F
	vpmovsxwd	zmm30{k7}, YMMWORD PTR [rcx]	 # AVX512F
	vpmovsxwd	zmm30{k7}, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmovsxwd	zmm30{k7}, YMMWORD PTR [rdx+4064]	 # AVX512F Disp8
	vpmovsxwd	zmm30{k7}, YMMWORD PTR [rdx+4096]	 # AVX512F
	vpmovsxwd	zmm30{k7}, YMMWORD PTR [rdx-4096]	 # AVX512F Disp8
	vpmovsxwd	zmm30{k7}, YMMWORD PTR [rdx-4128]	 # AVX512F

	vpmovsxwq	zmm30{k7}, xmm29	 # AVX512F
	vpmovsxwq	zmm30{k7}{z}, xmm29	 # AVX512F
	vpmovsxwq	zmm30{k7}, XMMWORD PTR [rcx]	 # AVX512F
	vpmovsxwq	zmm30{k7}, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmovsxwq	zmm30{k7}, XMMWORD PTR [rdx+2032]	 # AVX512F Disp8
	vpmovsxwq	zmm30{k7}, XMMWORD PTR [rdx+2048]	 # AVX512F
	vpmovsxwq	zmm30{k7}, XMMWORD PTR [rdx-2048]	 # AVX512F Disp8
	vpmovsxwq	zmm30{k7}, XMMWORD PTR [rdx-2064]	 # AVX512F

	vpmovzxbd	zmm30{k7}, xmm29	 # AVX512F
	vpmovzxbd	zmm30{k7}{z}, xmm29	 # AVX512F
	vpmovzxbd	zmm30{k7}, XMMWORD PTR [rcx]	 # AVX512F
	vpmovzxbd	zmm30{k7}, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmovzxbd	zmm30{k7}, XMMWORD PTR [rdx+2032]	 # AVX512F Disp8
	vpmovzxbd	zmm30{k7}, XMMWORD PTR [rdx+2048]	 # AVX512F
	vpmovzxbd	zmm30{k7}, XMMWORD PTR [rdx-2048]	 # AVX512F Disp8
	vpmovzxbd	zmm30{k7}, XMMWORD PTR [rdx-2064]	 # AVX512F

	vpmovzxbq	zmm30{k7}, xmm29	 # AVX512F
	vpmovzxbq	zmm30{k7}{z}, xmm29	 # AVX512F
	vpmovzxbq	zmm30{k7}, QWORD PTR [rcx]	 # AVX512F
	vpmovzxbq	zmm30{k7}, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmovzxbq	zmm30{k7}, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vpmovzxbq	zmm30{k7}, QWORD PTR [rdx+1024]	 # AVX512F
	vpmovzxbq	zmm30{k7}, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vpmovzxbq	zmm30{k7}, QWORD PTR [rdx-1032]	 # AVX512F

	vpmovzxdq	zmm30{k7}, ymm29	 # AVX512F
	vpmovzxdq	zmm30{k7}{z}, ymm29	 # AVX512F
	vpmovzxdq	zmm30{k7}, YMMWORD PTR [rcx]	 # AVX512F
	vpmovzxdq	zmm30{k7}, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmovzxdq	zmm30{k7}, YMMWORD PTR [rdx+4064]	 # AVX512F Disp8
	vpmovzxdq	zmm30{k7}, YMMWORD PTR [rdx+4096]	 # AVX512F
	vpmovzxdq	zmm30{k7}, YMMWORD PTR [rdx-4096]	 # AVX512F Disp8
	vpmovzxdq	zmm30{k7}, YMMWORD PTR [rdx-4128]	 # AVX512F

	vpmovzxwd	zmm30{k7}, ymm29	 # AVX512F
	vpmovzxwd	zmm30{k7}{z}, ymm29	 # AVX512F
	vpmovzxwd	zmm30{k7}, YMMWORD PTR [rcx]	 # AVX512F
	vpmovzxwd	zmm30{k7}, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmovzxwd	zmm30{k7}, YMMWORD PTR [rdx+4064]	 # AVX512F Disp8
	vpmovzxwd	zmm30{k7}, YMMWORD PTR [rdx+4096]	 # AVX512F
	vpmovzxwd	zmm30{k7}, YMMWORD PTR [rdx-4096]	 # AVX512F Disp8
	vpmovzxwd	zmm30{k7}, YMMWORD PTR [rdx-4128]	 # AVX512F

	vpmovzxwq	zmm30{k7}, xmm29	 # AVX512F
	vpmovzxwq	zmm30{k7}{z}, xmm29	 # AVX512F
	vpmovzxwq	zmm30{k7}, XMMWORD PTR [rcx]	 # AVX512F
	vpmovzxwq	zmm30{k7}, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmovzxwq	zmm30{k7}, XMMWORD PTR [rdx+2032]	 # AVX512F Disp8
	vpmovzxwq	zmm30{k7}, XMMWORD PTR [rdx+2048]	 # AVX512F
	vpmovzxwq	zmm30{k7}, XMMWORD PTR [rdx-2048]	 # AVX512F Disp8
	vpmovzxwq	zmm30{k7}, XMMWORD PTR [rdx-2064]	 # AVX512F

	vpmuldq	zmm30, zmm29, zmm28	 # AVX512F
	vpmuldq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpmuldq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpmuldq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpmuldq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmuldq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpmuldq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpmuldq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpmuldq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpmuldq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpmuldq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpmuldq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpmuldq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpmuldq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpmulld	zmm30, zmm29, zmm28	 # AVX512F
	vpmulld	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpmulld	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpmulld	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpmulld	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmulld	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpmulld	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpmulld	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpmulld	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpmulld	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpmulld	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpmulld	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpmulld	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpmulld	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpmuludq	zmm30, zmm29, zmm28	 # AVX512F
	vpmuludq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpmuludq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpmuludq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpmuludq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpmuludq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpmuludq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpmuludq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpmuludq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpmuludq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpmuludq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpmuludq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpmuludq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpmuludq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpord	zmm30, zmm29, zmm28	 # AVX512F
	vpord	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpord	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpord	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpord	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpord	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpord	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpord	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpord	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpord	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpord	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpord	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpord	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpord	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vporq	zmm30, zmm29, zmm28	 # AVX512F
	vporq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vporq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vporq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vporq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vporq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vporq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vporq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vporq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vporq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vporq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vporq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vporq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vporq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpscatterdd	[r14+zmm31*8-123]{k1}, zmm30	 # AVX512F
	vpscatterdd	dword ptr [r14+zmm31*8-123]{k1}, zmm30	 # AVX512F
	vpscatterdd	[r9+zmm31+256]{k1}, zmm30	 # AVX512F
	vpscatterdd	[rcx+zmm31*4+1024]{k1}, zmm30	 # AVX512F

	vpscatterdq	[r14+ymm31*8-123]{k1}, zmm30	 # AVX512F
	vpscatterdq	qword ptr [r14+ymm31*8-123]{k1}, zmm30	 # AVX512F
	vpscatterdq	[r9+ymm31+256]{k1}, zmm30	 # AVX512F
	vpscatterdq	[rcx+ymm31*4+1024]{k1}, zmm30	 # AVX512F

	vpscatterqd	[r14+zmm31*8-123]{k1}, ymm30	 # AVX512F
	vpscatterqd	dword ptr [r14+zmm31*8-123]{k1}, ymm30	 # AVX512F
	vpscatterqd	[r9+zmm31+256]{k1}, ymm30	 # AVX512F
	vpscatterqd	[rcx+zmm31*4+1024]{k1}, ymm30	 # AVX512F

	vpscatterqq	[r14+zmm31*8-123]{k1}, zmm30	 # AVX512F
	vpscatterqq	qword ptr [r14+zmm31*8-123]{k1}, zmm30	 # AVX512F
	vpscatterqq	[r9+zmm31+256]{k1}, zmm30	 # AVX512F
	vpscatterqq	[rcx+zmm31*4+1024]{k1}, zmm30	 # AVX512F

	vpshufd	zmm30, zmm29, 0xab	 # AVX512F
	vpshufd	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vpshufd	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vpshufd	zmm30, zmm29, 123	 # AVX512F
	vpshufd	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpshufd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpshufd	zmm30, dword bcst [rcx], 123	 # AVX512F
	vpshufd	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpshufd	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpshufd	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpshufd	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpshufd	zmm30, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vpshufd	zmm30, dword bcst [rdx+512], 123	 # AVX512F
	vpshufd	zmm30, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vpshufd	zmm30, dword bcst [rdx-516], 123	 # AVX512F

	vpslld	zmm30{k7}, zmm29, xmm28	 # AVX512F
	vpslld	zmm30{k7}{z}, zmm29, xmm28	 # AVX512F
	vpslld	zmm30{k7}, zmm29, XMMWORD PTR [rcx]	 # AVX512F
	vpslld	zmm30{k7}, zmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpslld	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2032]	 # AVX512F Disp8
	vpslld	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2048]	 # AVX512F
	vpslld	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2048]	 # AVX512F Disp8
	vpslld	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2064]	 # AVX512F

	vpsllq	zmm30{k7}, zmm29, xmm28	 # AVX512F
	vpsllq	zmm30{k7}{z}, zmm29, xmm28	 # AVX512F
	vpsllq	zmm30{k7}, zmm29, XMMWORD PTR [rcx]	 # AVX512F
	vpsllq	zmm30{k7}, zmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpsllq	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2032]	 # AVX512F Disp8
	vpsllq	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2048]	 # AVX512F
	vpsllq	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2048]	 # AVX512F Disp8
	vpsllq	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2064]	 # AVX512F

	vpsllvd	zmm30, zmm29, zmm28	 # AVX512F
	vpsllvd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpsllvd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpsllvd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpsllvd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpsllvd	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpsllvd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpsllvd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpsllvd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpsllvd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpsllvd	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpsllvd	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpsllvd	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpsllvd	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpsllvq	zmm30, zmm29, zmm28	 # AVX512F
	vpsllvq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpsllvq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpsllvq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpsllvq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpsllvq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpsllvq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpsllvq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpsllvq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpsllvq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpsllvq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpsllvq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpsllvq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpsllvq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpsrad	zmm30{k7}, zmm29, xmm28	 # AVX512F
	vpsrad	zmm30{k7}{z}, zmm29, xmm28	 # AVX512F
	vpsrad	zmm30{k7}, zmm29, XMMWORD PTR [rcx]	 # AVX512F
	vpsrad	zmm30{k7}, zmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpsrad	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2032]	 # AVX512F Disp8
	vpsrad	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2048]	 # AVX512F
	vpsrad	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2048]	 # AVX512F Disp8
	vpsrad	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2064]	 # AVX512F

	vpsraq	zmm30{k7}, zmm29, xmm28	 # AVX512F
	vpsraq	zmm30{k7}{z}, zmm29, xmm28	 # AVX512F
	vpsraq	zmm30{k7}, zmm29, XMMWORD PTR [rcx]	 # AVX512F
	vpsraq	zmm30{k7}, zmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpsraq	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2032]	 # AVX512F Disp8
	vpsraq	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2048]	 # AVX512F
	vpsraq	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2048]	 # AVX512F Disp8
	vpsraq	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2064]	 # AVX512F

	vpsravd	zmm30, zmm29, zmm28	 # AVX512F
	vpsravd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpsravd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpsravd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpsravd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpsravd	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpsravd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpsravd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpsravd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpsravd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpsravd	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpsravd	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpsravd	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpsravd	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpsravq	zmm30, zmm29, zmm28	 # AVX512F
	vpsravq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpsravq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpsravq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpsravq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpsravq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpsravq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpsravq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpsravq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpsravq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpsravq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpsravq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpsravq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpsravq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpsrld	zmm30{k7}, zmm29, xmm28	 # AVX512F
	vpsrld	zmm30{k7}{z}, zmm29, xmm28	 # AVX512F
	vpsrld	zmm30{k7}, zmm29, XMMWORD PTR [rcx]	 # AVX512F
	vpsrld	zmm30{k7}, zmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpsrld	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2032]	 # AVX512F Disp8
	vpsrld	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2048]	 # AVX512F
	vpsrld	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2048]	 # AVX512F Disp8
	vpsrld	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2064]	 # AVX512F

	vpsrlq	zmm30{k7}, zmm29, xmm28	 # AVX512F
	vpsrlq	zmm30{k7}{z}, zmm29, xmm28	 # AVX512F
	vpsrlq	zmm30{k7}, zmm29, XMMWORD PTR [rcx]	 # AVX512F
	vpsrlq	zmm30{k7}, zmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpsrlq	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2032]	 # AVX512F Disp8
	vpsrlq	zmm30{k7}, zmm29, XMMWORD PTR [rdx+2048]	 # AVX512F
	vpsrlq	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2048]	 # AVX512F Disp8
	vpsrlq	zmm30{k7}, zmm29, XMMWORD PTR [rdx-2064]	 # AVX512F

	vpsrlvd	zmm30, zmm29, zmm28	 # AVX512F
	vpsrlvd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpsrlvd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpsrlvd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpsrlvd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpsrlvd	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpsrlvd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpsrlvd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpsrlvd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpsrlvd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpsrlvd	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpsrlvd	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpsrlvd	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpsrlvd	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpsrlvq	zmm30, zmm29, zmm28	 # AVX512F
	vpsrlvq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpsrlvq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpsrlvq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpsrlvq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpsrlvq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpsrlvq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpsrlvq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpsrlvq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpsrlvq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpsrlvq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpsrlvq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpsrlvq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpsrlvq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpsrld	zmm30, zmm29, 0xab	 # AVX512F
	vpsrld	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vpsrld	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vpsrld	zmm30, zmm29, 123	 # AVX512F
	vpsrld	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpsrld	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpsrld	zmm30, dword bcst [rcx], 123	 # AVX512F
	vpsrld	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpsrld	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpsrld	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpsrld	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpsrld	zmm30, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vpsrld	zmm30, dword bcst [rdx+512], 123	 # AVX512F
	vpsrld	zmm30, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vpsrld	zmm30, dword bcst [rdx-516], 123	 # AVX512F

	vpsrlq	zmm30, zmm29, 0xab	 # AVX512F
	vpsrlq	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vpsrlq	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vpsrlq	zmm30, zmm29, 123	 # AVX512F
	vpsrlq	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpsrlq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpsrlq	zmm30, qword bcst [rcx], 123	 # AVX512F
	vpsrlq	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpsrlq	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpsrlq	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpsrlq	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpsrlq	zmm30, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vpsrlq	zmm30, qword bcst [rdx+1024], 123	 # AVX512F
	vpsrlq	zmm30, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vpsrlq	zmm30, qword bcst [rdx-1032], 123	 # AVX512F

	vpsubd	zmm30, zmm29, zmm28	 # AVX512F
	vpsubd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpsubd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpsubd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpsubd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpsubd	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpsubd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpsubd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpsubd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpsubd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpsubd	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpsubd	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpsubd	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpsubd	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpsubq	zmm30, zmm29, zmm28	 # AVX512F
	vpsubq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpsubq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpsubq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpsubq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpsubq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpsubq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpsubq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpsubq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpsubq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpsubq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpsubq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpsubq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpsubq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vptestmd	k5, zmm30, zmm29	 # AVX512F
	vptestmd	k5{k7}, zmm30, zmm29	 # AVX512F
	vptestmd	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vptestmd	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vptestmd	k5, zmm30, dword bcst [rcx]	 # AVX512F
	vptestmd	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vptestmd	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vptestmd	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vptestmd	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vptestmd	k5, zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vptestmd	k5, zmm30, dword bcst [rdx+512]	 # AVX512F
	vptestmd	k5, zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vptestmd	k5, zmm30, dword bcst [rdx-516]	 # AVX512F

	vptestmq	k5, zmm30, zmm29	 # AVX512F
	vptestmq	k5{k7}, zmm30, zmm29	 # AVX512F
	vptestmq	k5, zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vptestmq	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vptestmq	k5, zmm30, qword bcst [rcx]	 # AVX512F
	vptestmq	k5, zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vptestmq	k5, zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vptestmq	k5, zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vptestmq	k5, zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vptestmq	k5, zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vptestmq	k5, zmm30, qword bcst [rdx+1024]	 # AVX512F
	vptestmq	k5, zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vptestmq	k5, zmm30, qword bcst [rdx-1032]	 # AVX512F

	vpunpckhdq	zmm30, zmm29, zmm28	 # AVX512F
	vpunpckhdq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpunpckhdq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpunpckhdq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpunpckhdq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpunpckhdq	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpunpckhdq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpunpckhdq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpunpckhdq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpunpckhdq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpunpckhdq	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpunpckhdq	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpunpckhdq	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpunpckhdq	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpunpckhqdq	zmm30, zmm29, zmm28	 # AVX512F
	vpunpckhqdq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpunpckhqdq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpunpckhqdq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpunpckhqdq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpunpckhqdq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpunpckhqdq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpunpckhqdq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpunpckhqdq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpunpckhqdq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpunpckhqdq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpunpckhqdq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpunpckhqdq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpunpckhqdq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpunpckldq	zmm30, zmm29, zmm28	 # AVX512F
	vpunpckldq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpunpckldq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpunpckldq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpunpckldq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpunpckldq	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpunpckldq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpunpckldq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpunpckldq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpunpckldq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpunpckldq	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpunpckldq	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpunpckldq	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpunpckldq	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpunpcklqdq	zmm30, zmm29, zmm28	 # AVX512F
	vpunpcklqdq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpunpcklqdq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpunpcklqdq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpunpcklqdq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpunpcklqdq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpunpcklqdq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpunpcklqdq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpunpcklqdq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpunpcklqdq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpunpcklqdq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpunpcklqdq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpunpcklqdq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpunpcklqdq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpxord	zmm30, zmm29, zmm28	 # AVX512F
	vpxord	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpxord	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpxord	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpxord	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpxord	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpxord	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpxord	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpxord	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpxord	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpxord	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpxord	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpxord	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpxord	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpxorq	zmm30, zmm29, zmm28	 # AVX512F
	vpxorq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpxorq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpxorq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpxorq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpxorq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpxorq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpxorq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpxorq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpxorq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpxorq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpxorq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpxorq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpxorq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vrcp14pd	zmm30, zmm29	 # AVX512F
	vrcp14pd	zmm30{k7}, zmm29	 # AVX512F
	vrcp14pd	zmm30{k7}{z}, zmm29	 # AVX512F
	vrcp14pd	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vrcp14pd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vrcp14pd	zmm30, qword bcst [rcx]	 # AVX512F
	vrcp14pd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vrcp14pd	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vrcp14pd	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vrcp14pd	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vrcp14pd	zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vrcp14pd	zmm30, qword bcst [rdx+1024]	 # AVX512F
	vrcp14pd	zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vrcp14pd	zmm30, qword bcst [rdx-1032]	 # AVX512F

	vrcp14ps	zmm30, zmm29	 # AVX512F
	vrcp14ps	zmm30{k7}, zmm29	 # AVX512F
	vrcp14ps	zmm30{k7}{z}, zmm29	 # AVX512F
	vrcp14ps	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vrcp14ps	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vrcp14ps	zmm30, dword bcst [rcx]	 # AVX512F
	vrcp14ps	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vrcp14ps	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vrcp14ps	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vrcp14ps	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vrcp14ps	zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vrcp14ps	zmm30, dword bcst [rdx+512]	 # AVX512F
	vrcp14ps	zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vrcp14ps	zmm30, dword bcst [rdx-516]	 # AVX512F

	vrcp14sd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vrcp14sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vrcp14sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vrcp14sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vrcp14sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vrcp14sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vrcp14sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vrcp14sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vrcp14ss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vrcp14ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vrcp14ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vrcp14ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vrcp14ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vrcp14ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vrcp14ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vrcp14ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vrsqrt14pd	zmm30, zmm29	 # AVX512F
	vrsqrt14pd	zmm30{k7}, zmm29	 # AVX512F
	vrsqrt14pd	zmm30{k7}{z}, zmm29	 # AVX512F
	vrsqrt14pd	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vrsqrt14pd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vrsqrt14pd	zmm30, qword bcst [rcx]	 # AVX512F
	vrsqrt14pd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vrsqrt14pd	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vrsqrt14pd	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vrsqrt14pd	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vrsqrt14pd	zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vrsqrt14pd	zmm30, qword bcst [rdx+1024]	 # AVX512F
	vrsqrt14pd	zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vrsqrt14pd	zmm30, qword bcst [rdx-1032]	 # AVX512F

	vrsqrt14ps	zmm30, zmm29	 # AVX512F
	vrsqrt14ps	zmm30{k7}, zmm29	 # AVX512F
	vrsqrt14ps	zmm30{k7}{z}, zmm29	 # AVX512F
	vrsqrt14ps	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vrsqrt14ps	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vrsqrt14ps	zmm30, dword bcst [rcx]	 # AVX512F
	vrsqrt14ps	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vrsqrt14ps	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vrsqrt14ps	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vrsqrt14ps	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vrsqrt14ps	zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vrsqrt14ps	zmm30, dword bcst [rdx+512]	 # AVX512F
	vrsqrt14ps	zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vrsqrt14ps	zmm30, dword bcst [rdx-516]	 # AVX512F

	vrsqrt14sd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vrsqrt14sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vrsqrt14sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vrsqrt14sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vrsqrt14sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vrsqrt14sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vrsqrt14sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vrsqrt14sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vrsqrt14ss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vrsqrt14ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vrsqrt14ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vrsqrt14ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vrsqrt14ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vrsqrt14ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vrsqrt14ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vrsqrt14ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vscatterdpd	[r14+ymm31*8-123]{k1}, zmm30	 # AVX512F
	vscatterdpd	qword ptr [r14+ymm31*8-123]{k1}, zmm30	 # AVX512F
	vscatterdpd	[r9+ymm31+256]{k1}, zmm30	 # AVX512F
	vscatterdpd	[rcx+ymm31*4+1024]{k1}, zmm30	 # AVX512F

	vscatterdps	[r14+zmm31*8-123]{k1}, zmm30	 # AVX512F
	vscatterdps	dword ptr [r14+zmm31*8-123]{k1}, zmm30	 # AVX512F
	vscatterdps	[r9+zmm31+256]{k1}, zmm30	 # AVX512F
	vscatterdps	[rcx+zmm31*4+1024]{k1}, zmm30	 # AVX512F

	vscatterqpd	[r14+zmm31*8-123]{k1}, zmm30	 # AVX512F
	vscatterqpd	qword ptr [r14+zmm31*8-123]{k1}, zmm30	 # AVX512F
	vscatterqpd	[r9+zmm31+256]{k1}, zmm30	 # AVX512F
	vscatterqpd	[rcx+zmm31*4+1024]{k1}, zmm30	 # AVX512F

	vscatterqps	[r14+zmm31*8-123]{k1}, ymm30	 # AVX512F
	vscatterqps	dword ptr [r14+zmm31*8-123]{k1}, ymm30	 # AVX512F
	vscatterqps	[r9+zmm31+256]{k1}, ymm30	 # AVX512F
	vscatterqps	[rcx+zmm31*4+1024]{k1}, ymm30	 # AVX512F

	vshufpd	zmm30, zmm29, zmm28, 0xab	 # AVX512F
	vshufpd	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512F
	vshufpd	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512F
	vshufpd	zmm30, zmm29, zmm28, 123	 # AVX512F
	vshufpd	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512F
	vshufpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vshufpd	zmm30, zmm29, qword bcst [rcx], 123	 # AVX512F
	vshufpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vshufpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vshufpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vshufpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vshufpd	zmm30, zmm29, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vshufpd	zmm30, zmm29, qword bcst [rdx+1024], 123	 # AVX512F
	vshufpd	zmm30, zmm29, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vshufpd	zmm30, zmm29, qword bcst [rdx-1032], 123	 # AVX512F

	vshufps	zmm30, zmm29, zmm28, 0xab	 # AVX512F
	vshufps	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512F
	vshufps	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512F
	vshufps	zmm30, zmm29, zmm28, 123	 # AVX512F
	vshufps	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512F
	vshufps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vshufps	zmm30, zmm29, dword bcst [rcx], 123	 # AVX512F
	vshufps	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vshufps	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vshufps	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vshufps	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vshufps	zmm30, zmm29, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vshufps	zmm30, zmm29, dword bcst [rdx+512], 123	 # AVX512F
	vshufps	zmm30, zmm29, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vshufps	zmm30, zmm29, dword bcst [rdx-516], 123	 # AVX512F

	vsqrtpd	zmm30, zmm29	 # AVX512F
	vsqrtpd	zmm30{k7}, zmm29	 # AVX512F
	vsqrtpd	zmm30{k7}{z}, zmm29	 # AVX512F
	vsqrtpd	zmm30, zmm29{rn-sae}	 # AVX512F
	vsqrtpd	zmm30, zmm29{ru-sae}	 # AVX512F
	vsqrtpd	zmm30, zmm29{rd-sae}	 # AVX512F
	vsqrtpd	zmm30, zmm29{rz-sae}	 # AVX512F
	vsqrtpd	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vsqrtpd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vsqrtpd	zmm30, qword bcst [rcx]	 # AVX512F
	vsqrtpd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vsqrtpd	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vsqrtpd	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vsqrtpd	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vsqrtpd	zmm30, qword bcst [rdx+1016]	 # AVX512F Disp8
	vsqrtpd	zmm30, qword bcst [rdx+1024]	 # AVX512F
	vsqrtpd	zmm30, qword bcst [rdx-1024]	 # AVX512F Disp8
	vsqrtpd	zmm30, qword bcst [rdx-1032]	 # AVX512F

	vsqrtps	zmm30, zmm29	 # AVX512F
	vsqrtps	zmm30{k7}, zmm29	 # AVX512F
	vsqrtps	zmm30{k7}{z}, zmm29	 # AVX512F
	vsqrtps	zmm30, zmm29{rn-sae}	 # AVX512F
	vsqrtps	zmm30, zmm29{ru-sae}	 # AVX512F
	vsqrtps	zmm30, zmm29{rd-sae}	 # AVX512F
	vsqrtps	zmm30, zmm29{rz-sae}	 # AVX512F
	vsqrtps	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vsqrtps	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vsqrtps	zmm30, dword bcst [rcx]	 # AVX512F
	vsqrtps	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vsqrtps	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vsqrtps	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vsqrtps	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vsqrtps	zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vsqrtps	zmm30, dword bcst [rdx+512]	 # AVX512F
	vsqrtps	zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vsqrtps	zmm30, dword bcst [rdx-516]	 # AVX512F

	vsqrtsd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vsqrtsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vsqrtsd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vsqrtsd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vsqrtsd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vsqrtsd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vsqrtsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vsqrtsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vsqrtsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vsqrtsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vsqrtsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vsqrtsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vsqrtss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vsqrtss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vsqrtss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vsqrtss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vsqrtss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vsqrtss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vsqrtss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vsqrtss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vsqrtss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vsqrtss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vsqrtss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vsqrtss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vsubpd	zmm30, zmm29, zmm28	 # AVX512F
	vsubpd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vsubpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vsubpd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vsubpd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vsubpd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vsubpd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vsubpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vsubpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vsubpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vsubpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vsubpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vsubpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vsubpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vsubpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vsubpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vsubpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vsubpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vsubps	zmm30, zmm29, zmm28	 # AVX512F
	vsubps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vsubps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vsubps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vsubps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vsubps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vsubps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vsubps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vsubps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vsubps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vsubps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vsubps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vsubps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vsubps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vsubps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vsubps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vsubps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vsubps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vsubsd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vsubsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vsubsd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vsubsd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vsubsd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vsubsd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vsubsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vsubsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vsubsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vsubsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vsubsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vsubsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vsubss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vsubss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vsubss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vsubss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vsubss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vsubss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vsubss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vsubss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vsubss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vsubss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vsubss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vsubss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vucomisd	xmm30, xmm29	 # AVX512F
	vucomisd	xmm30, xmm29{sae}	 # AVX512F
	vucomisd	xmm30, QWORD PTR [rcx]	 # AVX512F
	vucomisd	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vucomisd	xmm30, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vucomisd	xmm30, QWORD PTR [rdx+1024]	 # AVX512F
	vucomisd	xmm30, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vucomisd	xmm30, QWORD PTR [rdx-1032]	 # AVX512F

	vucomiss	xmm30, xmm29	 # AVX512F
	vucomiss	xmm30, xmm29{sae}	 # AVX512F
	vucomiss	xmm30, DWORD PTR [rcx]	 # AVX512F
	vucomiss	xmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vucomiss	xmm30, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vucomiss	xmm30, DWORD PTR [rdx+512]	 # AVX512F
	vucomiss	xmm30, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vucomiss	xmm30, DWORD PTR [rdx-516]	 # AVX512F

	vunpckhpd	zmm30, zmm29, zmm28	 # AVX512F
	vunpckhpd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vunpckhpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vunpckhpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vunpckhpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vunpckhpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vunpckhpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vunpckhpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vunpckhpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vunpckhpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vunpckhpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vunpckhpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vunpckhpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vunpckhpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vunpckhps	zmm30, zmm29, zmm28	 # AVX512F
	vunpckhps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vunpckhps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vunpckhps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vunpckhps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vunpckhps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vunpckhps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vunpckhps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vunpckhps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vunpckhps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vunpckhps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vunpckhps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vunpckhps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vunpckhps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vunpcklpd	zmm30, zmm29, zmm28	 # AVX512F
	vunpcklpd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vunpcklpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vunpcklpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vunpcklpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vunpcklpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vunpcklpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vunpcklpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vunpcklpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vunpcklpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vunpcklpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vunpcklpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vunpcklpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vunpcklpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vunpcklps	zmm30, zmm29, zmm28	 # AVX512F
	vunpcklps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vunpcklps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vunpcklps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vunpcklps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vunpcklps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vunpcklps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vunpcklps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vunpcklps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vunpcklps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vunpcklps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vunpcklps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vunpcklps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vunpcklps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpternlogd	zmm30, zmm29, zmm28, 0xab	 # AVX512F
	vpternlogd	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512F
	vpternlogd	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512F
	vpternlogd	zmm30, zmm29, zmm28, 123	 # AVX512F
	vpternlogd	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpternlogd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpternlogd	zmm30, zmm29, dword bcst [rcx], 123	 # AVX512F
	vpternlogd	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpternlogd	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpternlogd	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpternlogd	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpternlogd	zmm30, zmm29, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vpternlogd	zmm30, zmm29, dword bcst [rdx+512], 123	 # AVX512F
	vpternlogd	zmm30, zmm29, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vpternlogd	zmm30, zmm29, dword bcst [rdx-516], 123	 # AVX512F

	vpternlogq	zmm30, zmm29, zmm28, 0xab	 # AVX512F
	vpternlogq	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512F
	vpternlogq	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512F
	vpternlogq	zmm30, zmm29, zmm28, 123	 # AVX512F
	vpternlogq	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpternlogq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpternlogq	zmm30, zmm29, qword bcst [rcx], 123	 # AVX512F
	vpternlogq	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpternlogq	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpternlogq	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpternlogq	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpternlogq	zmm30, zmm29, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vpternlogq	zmm30, zmm29, qword bcst [rdx+1024], 123	 # AVX512F
	vpternlogq	zmm30, zmm29, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vpternlogq	zmm30, zmm29, qword bcst [rdx-1032], 123	 # AVX512F

	vpmovqb	xmm30{k7}, zmm29	 # AVX512F
	vpmovqb	xmm30{k7}{z}, zmm29	 # AVX512F

	vpmovsqb	xmm30{k7}, zmm29	 # AVX512F
	vpmovsqb	xmm30{k7}{z}, zmm29	 # AVX512F

	vpmovusqb	xmm30{k7}, zmm29	 # AVX512F
	vpmovusqb	xmm30{k7}{z}, zmm29	 # AVX512F

	vpmovqw	xmm30{k7}, zmm29	 # AVX512F
	vpmovqw	xmm30{k7}{z}, zmm29	 # AVX512F

	vpmovsqw	xmm30{k7}, zmm29	 # AVX512F
	vpmovsqw	xmm30{k7}{z}, zmm29	 # AVX512F

	vpmovusqw	xmm30{k7}, zmm29	 # AVX512F
	vpmovusqw	xmm30{k7}{z}, zmm29	 # AVX512F

	vpmovqd	ymm30{k7}, zmm29	 # AVX512F
	vpmovqd	ymm30{k7}{z}, zmm29	 # AVX512F

	vpmovsqd	ymm30{k7}, zmm29	 # AVX512F
	vpmovsqd	ymm30{k7}{z}, zmm29	 # AVX512F

	vpmovusqd	ymm30{k7}, zmm29	 # AVX512F
	vpmovusqd	ymm30{k7}{z}, zmm29	 # AVX512F

	vpmovdb	xmm30{k7}, zmm29	 # AVX512F
	vpmovdb	xmm30{k7}{z}, zmm29	 # AVX512F

	vpmovsdb	xmm30{k7}, zmm29	 # AVX512F
	vpmovsdb	xmm30{k7}{z}, zmm29	 # AVX512F

	vpmovusdb	xmm30{k7}, zmm29	 # AVX512F
	vpmovusdb	xmm30{k7}{z}, zmm29	 # AVX512F

	vpmovdw	ymm30{k7}, zmm29	 # AVX512F
	vpmovdw	ymm30{k7}{z}, zmm29	 # AVX512F

	vpmovsdw	ymm30{k7}, zmm29	 # AVX512F
	vpmovsdw	ymm30{k7}{z}, zmm29	 # AVX512F

	vpmovusdw	ymm30{k7}, zmm29	 # AVX512F
	vpmovusdw	ymm30{k7}{z}, zmm29	 # AVX512F

	vshuff32x4	zmm30, zmm29, zmm28, 0xab	 # AVX512F
	vshuff32x4	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512F
	vshuff32x4	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512F
	vshuff32x4	zmm30, zmm29, zmm28, 123	 # AVX512F
	vshuff32x4	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512F
	vshuff32x4	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vshuff32x4	zmm30, zmm29, dword bcst [rcx], 123	 # AVX512F
	vshuff32x4	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vshuff32x4	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vshuff32x4	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vshuff32x4	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vshuff32x4	zmm30, zmm29, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vshuff32x4	zmm30, zmm29, dword bcst [rdx+512], 123	 # AVX512F
	vshuff32x4	zmm30, zmm29, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vshuff32x4	zmm30, zmm29, dword bcst [rdx-516], 123	 # AVX512F

	vshuff64x2	zmm30, zmm29, zmm28, 0xab	 # AVX512F
	vshuff64x2	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512F
	vshuff64x2	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512F
	vshuff64x2	zmm30, zmm29, zmm28, 123	 # AVX512F
	vshuff64x2	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512F
	vshuff64x2	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vshuff64x2	zmm30, zmm29, qword bcst [rcx], 123	 # AVX512F
	vshuff64x2	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vshuff64x2	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vshuff64x2	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vshuff64x2	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vshuff64x2	zmm30, zmm29, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vshuff64x2	zmm30, zmm29, qword bcst [rdx+1024], 123	 # AVX512F
	vshuff64x2	zmm30, zmm29, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vshuff64x2	zmm30, zmm29, qword bcst [rdx-1032], 123	 # AVX512F

	vshufi32x4	zmm30, zmm29, zmm28, 0xab	 # AVX512F
	vshufi32x4	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512F
	vshufi32x4	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512F
	vshufi32x4	zmm30, zmm29, zmm28, 123	 # AVX512F
	vshufi32x4	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512F
	vshufi32x4	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vshufi32x4	zmm30, zmm29, dword bcst [rcx], 123	 # AVX512F
	vshufi32x4	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vshufi32x4	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vshufi32x4	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vshufi32x4	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vshufi32x4	zmm30, zmm29, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vshufi32x4	zmm30, zmm29, dword bcst [rdx+512], 123	 # AVX512F
	vshufi32x4	zmm30, zmm29, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vshufi32x4	zmm30, zmm29, dword bcst [rdx-516], 123	 # AVX512F

	vshufi64x2	zmm30, zmm29, zmm28, 0xab	 # AVX512F
	vshufi64x2	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512F
	vshufi64x2	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512F
	vshufi64x2	zmm30, zmm29, zmm28, 123	 # AVX512F
	vshufi64x2	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512F
	vshufi64x2	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vshufi64x2	zmm30, zmm29, qword bcst [rcx], 123	 # AVX512F
	vshufi64x2	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vshufi64x2	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vshufi64x2	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vshufi64x2	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vshufi64x2	zmm30, zmm29, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vshufi64x2	zmm30, zmm29, qword bcst [rdx+1024], 123	 # AVX512F
	vshufi64x2	zmm30, zmm29, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vshufi64x2	zmm30, zmm29, qword bcst [rdx-1032], 123	 # AVX512F

	vpermq	zmm30, zmm29, zmm28	 # AVX512F
	vpermq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpermq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpermq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpermq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpermq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpermq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpermq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpermq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpermq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpermq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpermq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpermq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpermq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpermpd	zmm30, zmm29, zmm28	 # AVX512F
	vpermpd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpermpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpermpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpermpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpermpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpermpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpermpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpermpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpermpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpermpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpermpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpermpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpermpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpermt2d	zmm30, zmm29, zmm28	 # AVX512F
	vpermt2d	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpermt2d	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpermt2d	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpermt2d	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpermt2d	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpermt2d	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpermt2d	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpermt2d	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpermt2d	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpermt2d	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpermt2d	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpermt2d	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpermt2d	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpermt2q	zmm30, zmm29, zmm28	 # AVX512F
	vpermt2q	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpermt2q	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpermt2q	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpermt2q	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpermt2q	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpermt2q	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpermt2q	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpermt2q	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpermt2q	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpermt2q	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpermt2q	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpermt2q	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpermt2q	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpermt2ps	zmm30, zmm29, zmm28	 # AVX512F
	vpermt2ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpermt2ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpermt2ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpermt2ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpermt2ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpermt2ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpermt2ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpermt2ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpermt2ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpermt2ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpermt2ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpermt2ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpermt2ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpermt2pd	zmm30, zmm29, zmm28	 # AVX512F
	vpermt2pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpermt2pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpermt2pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpermt2pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpermt2pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpermt2pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpermt2pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpermt2pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpermt2pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpermt2pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpermt2pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpermt2pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpermt2pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	valignq	zmm30, zmm29, zmm28, 0xab	 # AVX512F
	valignq	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512F
	valignq	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512F
	valignq	zmm30, zmm29, zmm28, 123	 # AVX512F
	valignq	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512F
	valignq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	valignq	zmm30, zmm29, qword bcst [rcx], 123	 # AVX512F
	valignq	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	valignq	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	valignq	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	valignq	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	valignq	zmm30, zmm29, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	valignq	zmm30, zmm29, qword bcst [rdx+1024], 123	 # AVX512F
	valignq	zmm30, zmm29, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	valignq	zmm30, zmm29, qword bcst [rdx-1032], 123	 # AVX512F

	vcvtsd2usi	eax, xmm30	 # AVX512F
	vcvtsd2usi	eax, xmm30{rn-sae}	 # AVX512F
	vcvtsd2usi	eax, xmm30{ru-sae}	 # AVX512F
	vcvtsd2usi	eax, xmm30{rd-sae}	 # AVX512F
	vcvtsd2usi	eax, xmm30{rz-sae}	 # AVX512F
	vcvtsd2usi	eax, QWORD PTR [rcx]	 # AVX512F
	vcvtsd2usi	eax, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtsd2usi	eax, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcvtsd2usi	eax, QWORD PTR [rdx+1024]	 # AVX512F
	vcvtsd2usi	eax, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcvtsd2usi	eax, QWORD PTR [rdx-1032]	 # AVX512F
	vcvtsd2usi	ebp, xmm30	 # AVX512F
	vcvtsd2usi	ebp, xmm30{rn-sae}	 # AVX512F
	vcvtsd2usi	ebp, xmm30{ru-sae}	 # AVX512F
	vcvtsd2usi	ebp, xmm30{rd-sae}	 # AVX512F
	vcvtsd2usi	ebp, xmm30{rz-sae}	 # AVX512F
	vcvtsd2usi	ebp, QWORD PTR [rcx]	 # AVX512F
	vcvtsd2usi	ebp, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtsd2usi	ebp, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcvtsd2usi	ebp, QWORD PTR [rdx+1024]	 # AVX512F
	vcvtsd2usi	ebp, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcvtsd2usi	ebp, QWORD PTR [rdx-1032]	 # AVX512F
	vcvtsd2usi	r13d, xmm30	 # AVX512F
	vcvtsd2usi	r13d, xmm30{rn-sae}	 # AVX512F
	vcvtsd2usi	r13d, xmm30{ru-sae}	 # AVX512F
	vcvtsd2usi	r13d, xmm30{rd-sae}	 # AVX512F
	vcvtsd2usi	r13d, xmm30{rz-sae}	 # AVX512F
	vcvtsd2usi	r13d, QWORD PTR [rcx]	 # AVX512F
	vcvtsd2usi	r13d, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtsd2usi	r13d, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcvtsd2usi	r13d, QWORD PTR [rdx+1024]	 # AVX512F
	vcvtsd2usi	r13d, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcvtsd2usi	r13d, QWORD PTR [rdx-1032]	 # AVX512F

	vcvtsd2usi	rax, xmm30	 # AVX512F
	vcvtsd2usi	rax, xmm30{rn-sae}	 # AVX512F
	vcvtsd2usi	rax, xmm30{ru-sae}	 # AVX512F
	vcvtsd2usi	rax, xmm30{rd-sae}	 # AVX512F
	vcvtsd2usi	rax, xmm30{rz-sae}	 # AVX512F
	vcvtsd2usi	rax, QWORD PTR [rcx]	 # AVX512F
	vcvtsd2usi	rax, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtsd2usi	rax, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcvtsd2usi	rax, QWORD PTR [rdx+1024]	 # AVX512F
	vcvtsd2usi	rax, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcvtsd2usi	rax, QWORD PTR [rdx-1032]	 # AVX512F
	vcvtsd2usi	r8, xmm30	 # AVX512F
	vcvtsd2usi	r8, xmm30{rn-sae}	 # AVX512F
	vcvtsd2usi	r8, xmm30{ru-sae}	 # AVX512F
	vcvtsd2usi	r8, xmm30{rd-sae}	 # AVX512F
	vcvtsd2usi	r8, xmm30{rz-sae}	 # AVX512F
	vcvtsd2usi	r8, QWORD PTR [rcx]	 # AVX512F
	vcvtsd2usi	r8, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtsd2usi	r8, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcvtsd2usi	r8, QWORD PTR [rdx+1024]	 # AVX512F
	vcvtsd2usi	r8, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcvtsd2usi	r8, QWORD PTR [rdx-1032]	 # AVX512F

	vcvtss2usi	eax, xmm30	 # AVX512F
	vcvtss2usi	eax, xmm30{rn-sae}	 # AVX512F
	vcvtss2usi	eax, xmm30{ru-sae}	 # AVX512F
	vcvtss2usi	eax, xmm30{rd-sae}	 # AVX512F
	vcvtss2usi	eax, xmm30{rz-sae}	 # AVX512F
	vcvtss2usi	eax, DWORD PTR [rcx]	 # AVX512F
	vcvtss2usi	eax, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtss2usi	eax, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcvtss2usi	eax, DWORD PTR [rdx+512]	 # AVX512F
	vcvtss2usi	eax, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcvtss2usi	eax, DWORD PTR [rdx-516]	 # AVX512F
	vcvtss2usi	ebp, xmm30	 # AVX512F
	vcvtss2usi	ebp, xmm30{rn-sae}	 # AVX512F
	vcvtss2usi	ebp, xmm30{ru-sae}	 # AVX512F
	vcvtss2usi	ebp, xmm30{rd-sae}	 # AVX512F
	vcvtss2usi	ebp, xmm30{rz-sae}	 # AVX512F
	vcvtss2usi	ebp, DWORD PTR [rcx]	 # AVX512F
	vcvtss2usi	ebp, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtss2usi	ebp, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcvtss2usi	ebp, DWORD PTR [rdx+512]	 # AVX512F
	vcvtss2usi	ebp, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcvtss2usi	ebp, DWORD PTR [rdx-516]	 # AVX512F
	vcvtss2usi	r13d, xmm30	 # AVX512F
	vcvtss2usi	r13d, xmm30{rn-sae}	 # AVX512F
	vcvtss2usi	r13d, xmm30{ru-sae}	 # AVX512F
	vcvtss2usi	r13d, xmm30{rd-sae}	 # AVX512F
	vcvtss2usi	r13d, xmm30{rz-sae}	 # AVX512F
	vcvtss2usi	r13d, DWORD PTR [rcx]	 # AVX512F
	vcvtss2usi	r13d, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtss2usi	r13d, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcvtss2usi	r13d, DWORD PTR [rdx+512]	 # AVX512F
	vcvtss2usi	r13d, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcvtss2usi	r13d, DWORD PTR [rdx-516]	 # AVX512F

	vcvtss2usi	rax, xmm30	 # AVX512F
	vcvtss2usi	rax, xmm30{rn-sae}	 # AVX512F
	vcvtss2usi	rax, xmm30{ru-sae}	 # AVX512F
	vcvtss2usi	rax, xmm30{rd-sae}	 # AVX512F
	vcvtss2usi	rax, xmm30{rz-sae}	 # AVX512F
	vcvtss2usi	rax, DWORD PTR [rcx]	 # AVX512F
	vcvtss2usi	rax, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtss2usi	rax, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcvtss2usi	rax, DWORD PTR [rdx+512]	 # AVX512F
	vcvtss2usi	rax, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcvtss2usi	rax, DWORD PTR [rdx-516]	 # AVX512F
	vcvtss2usi	r8, xmm30	 # AVX512F
	vcvtss2usi	r8, xmm30{rn-sae}	 # AVX512F
	vcvtss2usi	r8, xmm30{ru-sae}	 # AVX512F
	vcvtss2usi	r8, xmm30{rd-sae}	 # AVX512F
	vcvtss2usi	r8, xmm30{rz-sae}	 # AVX512F
	vcvtss2usi	r8, DWORD PTR [rcx]	 # AVX512F
	vcvtss2usi	r8, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtss2usi	r8, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcvtss2usi	r8, DWORD PTR [rdx+512]	 # AVX512F
	vcvtss2usi	r8, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcvtss2usi	r8, DWORD PTR [rdx-516]	 # AVX512F

	vcvtusi2sd	xmm30, xmm29, eax	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, ebp	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, r13d	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcvtusi2sd	xmm30, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcvtusi2sd	xmm30, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcvtusi2sd	xmm30, xmm29, rax	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, rax{rn-sae}	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, rax{ru-sae}	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, rax{rd-sae}	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, rax{rz-sae}	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, r8	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, r8{rn-sae}	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, r8{ru-sae}	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, r8{rd-sae}	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, r8{rz-sae}	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcvtusi2sd	xmm30, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcvtusi2sd	xmm30, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcvtusi2sd	xmm30, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vcvtusi2ss	xmm30, xmm29, eax	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, eax{rn-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, eax{ru-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, eax{rd-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, eax{rz-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, ebp	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, ebp{rn-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, ebp{ru-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, ebp{rd-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, ebp{rz-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, r13d	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, r13d{rn-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, r13d{ru-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, r13d{rd-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, r13d{rz-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, DWORD PTR [rcx]	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcvtusi2ss	xmm30, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcvtusi2ss	xmm30, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vcvtusi2ss	xmm30, xmm29, rax	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, rax{rn-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, rax{ru-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, rax{rd-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, rax{rz-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, r8	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, r8{rn-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, r8{ru-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, r8{rd-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, r8{rz-sae}	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, QWORD PTR [rcx]	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcvtusi2ss	xmm30, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vcvtusi2ss	xmm30, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcvtusi2ss	xmm30, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vscalefpd	zmm30, zmm29, zmm28	 # AVX512F
	vscalefpd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vscalefpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vscalefpd	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vscalefpd	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vscalefpd	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vscalefpd	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vscalefpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vscalefpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vscalefpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vscalefpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vscalefpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vscalefpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vscalefpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vscalefpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vscalefpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vscalefpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vscalefpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vscalefps	zmm30, zmm29, zmm28	 # AVX512F
	vscalefps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vscalefps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vscalefps	zmm30, zmm29, zmm28{rn-sae}	 # AVX512F
	vscalefps	zmm30, zmm29, zmm28{ru-sae}	 # AVX512F
	vscalefps	zmm30, zmm29, zmm28{rd-sae}	 # AVX512F
	vscalefps	zmm30, zmm29, zmm28{rz-sae}	 # AVX512F
	vscalefps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vscalefps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vscalefps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vscalefps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vscalefps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vscalefps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vscalefps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vscalefps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vscalefps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vscalefps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vscalefps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vscalefsd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vscalefsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vscalefsd	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vscalefsd	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vscalefsd	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vscalefsd	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vscalefsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512F
	vscalefsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vscalefsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vscalefsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512F
	vscalefsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vscalefsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512F

	vscalefss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vscalefss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vscalefss	xmm30{k7}, xmm29, xmm28{rn-sae}	 # AVX512F
	vscalefss	xmm30{k7}, xmm29, xmm28{ru-sae}	 # AVX512F
	vscalefss	xmm30{k7}, xmm29, xmm28{rd-sae}	 # AVX512F
	vscalefss	xmm30{k7}, xmm29, xmm28{rz-sae}	 # AVX512F
	vscalefss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512F
	vscalefss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vscalefss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vscalefss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512F
	vscalefss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vscalefss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512F

	vfixupimmps	zmm30, zmm29, zmm28, 0xab	 # AVX512F
	vfixupimmps	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512F
	vfixupimmps	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512F
	vfixupimmps	zmm30, zmm29, zmm28{sae}, 0xab	 # AVX512F
	vfixupimmps	zmm30, zmm29, zmm28, 123	 # AVX512F
	vfixupimmps	zmm30, zmm29, zmm28{sae}, 123	 # AVX512F
	vfixupimmps	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512F
	vfixupimmps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vfixupimmps	zmm30, zmm29, dword bcst [rcx], 123	 # AVX512F
	vfixupimmps	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vfixupimmps	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vfixupimmps	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vfixupimmps	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vfixupimmps	zmm30, zmm29, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vfixupimmps	zmm30, zmm29, dword bcst [rdx+512], 123	 # AVX512F
	vfixupimmps	zmm30, zmm29, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vfixupimmps	zmm30, zmm29, dword bcst [rdx-516], 123	 # AVX512F

	vfixupimmpd	zmm30, zmm29, zmm28, 0xab	 # AVX512F
	vfixupimmpd	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512F
	vfixupimmpd	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512F
	vfixupimmpd	zmm30, zmm29, zmm28{sae}, 0xab	 # AVX512F
	vfixupimmpd	zmm30, zmm29, zmm28, 123	 # AVX512F
	vfixupimmpd	zmm30, zmm29, zmm28{sae}, 123	 # AVX512F
	vfixupimmpd	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512F
	vfixupimmpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vfixupimmpd	zmm30, zmm29, qword bcst [rcx], 123	 # AVX512F
	vfixupimmpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vfixupimmpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vfixupimmpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vfixupimmpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vfixupimmpd	zmm30, zmm29, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vfixupimmpd	zmm30, zmm29, qword bcst [rdx+1024], 123	 # AVX512F
	vfixupimmpd	zmm30, zmm29, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vfixupimmpd	zmm30, zmm29, qword bcst [rdx-1032], 123	 # AVX512F

	vfixupimmss	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512F
	vfixupimmss	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512F
	vfixupimmss	xmm30{k7}, xmm29, xmm28{sae}, 0xab	 # AVX512F
	vfixupimmss	xmm30{k7}, xmm29, xmm28, 123	 # AVX512F
	vfixupimmss	xmm30{k7}, xmm29, xmm28{sae}, 123	 # AVX512F
	vfixupimmss	xmm30{k7}, xmm29, DWORD PTR [rcx], 123	 # AVX512F
	vfixupimmss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vfixupimmss	xmm30{k7}, xmm29, DWORD PTR [rdx+508], 123	 # AVX512F Disp8
	vfixupimmss	xmm30{k7}, xmm29, DWORD PTR [rdx+512], 123	 # AVX512F
	vfixupimmss	xmm30{k7}, xmm29, DWORD PTR [rdx-512], 123	 # AVX512F Disp8
	vfixupimmss	xmm30{k7}, xmm29, DWORD PTR [rdx-516], 123	 # AVX512F

	vfixupimmsd	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512F
	vfixupimmsd	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512F
	vfixupimmsd	xmm30{k7}, xmm29, xmm28{sae}, 0xab	 # AVX512F
	vfixupimmsd	xmm30{k7}, xmm29, xmm28, 123	 # AVX512F
	vfixupimmsd	xmm30{k7}, xmm29, xmm28{sae}, 123	 # AVX512F
	vfixupimmsd	xmm30{k7}, xmm29, QWORD PTR [rcx], 123	 # AVX512F
	vfixupimmsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vfixupimmsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016], 123	 # AVX512F Disp8
	vfixupimmsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024], 123	 # AVX512F
	vfixupimmsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024], 123	 # AVX512F Disp8
	vfixupimmsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032], 123	 # AVX512F

	vpslld	zmm30, zmm29, 0xab	 # AVX512F
	vpslld	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vpslld	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vpslld	zmm30, zmm29, 123	 # AVX512F
	vpslld	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpslld	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpslld	zmm30, dword bcst [rcx], 123	 # AVX512F
	vpslld	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpslld	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpslld	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpslld	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpslld	zmm30, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vpslld	zmm30, dword bcst [rdx+512], 123	 # AVX512F
	vpslld	zmm30, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vpslld	zmm30, dword bcst [rdx-516], 123	 # AVX512F

	vpsllq	zmm30, zmm29, 0xab	 # AVX512F
	vpsllq	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vpsllq	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vpsllq	zmm30, zmm29, 123	 # AVX512F
	vpsllq	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpsllq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpsllq	zmm30, qword bcst [rcx], 123	 # AVX512F
	vpsllq	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpsllq	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpsllq	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpsllq	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpsllq	zmm30, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vpsllq	zmm30, qword bcst [rdx+1024], 123	 # AVX512F
	vpsllq	zmm30, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vpsllq	zmm30, qword bcst [rdx-1032], 123	 # AVX512F

	vpsrad	zmm30, zmm29, 0xab	 # AVX512F
	vpsrad	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vpsrad	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vpsrad	zmm30, zmm29, 123	 # AVX512F
	vpsrad	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpsrad	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpsrad	zmm30, dword bcst [rcx], 123	 # AVX512F
	vpsrad	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpsrad	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpsrad	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpsrad	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpsrad	zmm30, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vpsrad	zmm30, dword bcst [rdx+512], 123	 # AVX512F
	vpsrad	zmm30, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vpsrad	zmm30, dword bcst [rdx-516], 123	 # AVX512F

	vpsraq	zmm30, zmm29, 0xab	 # AVX512F
	vpsraq	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vpsraq	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vpsraq	zmm30, zmm29, 123	 # AVX512F
	vpsraq	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vpsraq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vpsraq	zmm30, qword bcst [rcx], 123	 # AVX512F
	vpsraq	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vpsraq	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vpsraq	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vpsraq	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vpsraq	zmm30, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vpsraq	zmm30, qword bcst [rdx+1024], 123	 # AVX512F
	vpsraq	zmm30, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vpsraq	zmm30, qword bcst [rdx-1032], 123	 # AVX512F

	vprolvd	zmm30, zmm29, zmm28	 # AVX512F
	vprolvd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vprolvd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vprolvd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vprolvd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vprolvd	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vprolvd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vprolvd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vprolvd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vprolvd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vprolvd	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vprolvd	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vprolvd	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vprolvd	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vprold	zmm30, zmm29, 0xab	 # AVX512F
	vprold	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vprold	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vprold	zmm30, zmm29, 123	 # AVX512F
	vprold	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vprold	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vprold	zmm30, dword bcst [rcx], 123	 # AVX512F
	vprold	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vprold	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vprold	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vprold	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vprold	zmm30, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vprold	zmm30, dword bcst [rdx+512], 123	 # AVX512F
	vprold	zmm30, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vprold	zmm30, dword bcst [rdx-516], 123	 # AVX512F

	vprolvq	zmm30, zmm29, zmm28	 # AVX512F
	vprolvq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vprolvq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vprolvq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vprolvq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vprolvq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vprolvq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vprolvq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vprolvq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vprolvq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vprolvq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vprolvq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vprolvq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vprolvq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vprolq	zmm30, zmm29, 0xab	 # AVX512F
	vprolq	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vprolq	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vprolq	zmm30, zmm29, 123	 # AVX512F
	vprolq	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vprolq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vprolq	zmm30, qword bcst [rcx], 123	 # AVX512F
	vprolq	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vprolq	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vprolq	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vprolq	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vprolq	zmm30, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vprolq	zmm30, qword bcst [rdx+1024], 123	 # AVX512F
	vprolq	zmm30, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vprolq	zmm30, qword bcst [rdx-1032], 123	 # AVX512F

	vprorvd	zmm30, zmm29, zmm28	 # AVX512F
	vprorvd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vprorvd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vprorvd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vprorvd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vprorvd	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vprorvd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vprorvd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vprorvd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vprorvd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vprorvd	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vprorvd	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vprorvd	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vprorvd	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vprord	zmm30, zmm29, 0xab	 # AVX512F
	vprord	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vprord	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vprord	zmm30, zmm29, 123	 # AVX512F
	vprord	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vprord	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vprord	zmm30, dword bcst [rcx], 123	 # AVX512F
	vprord	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vprord	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vprord	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vprord	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vprord	zmm30, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vprord	zmm30, dword bcst [rdx+512], 123	 # AVX512F
	vprord	zmm30, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vprord	zmm30, dword bcst [rdx-516], 123	 # AVX512F

	vprorvq	zmm30, zmm29, zmm28	 # AVX512F
	vprorvq	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vprorvq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vprorvq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vprorvq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vprorvq	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vprorvq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vprorvq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vprorvq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vprorvq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vprorvq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vprorvq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vprorvq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vprorvq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vprorq	zmm30, zmm29, 0xab	 # AVX512F
	vprorq	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vprorq	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vprorq	zmm30, zmm29, 123	 # AVX512F
	vprorq	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vprorq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vprorq	zmm30, qword bcst [rcx], 123	 # AVX512F
	vprorq	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vprorq	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vprorq	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vprorq	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vprorq	zmm30, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vprorq	zmm30, qword bcst [rdx+1024], 123	 # AVX512F
	vprorq	zmm30, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vprorq	zmm30, qword bcst [rdx-1032], 123	 # AVX512F

	vrndscalepd	zmm30, zmm29, 0xab	 # AVX512F
	vrndscalepd	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vrndscalepd	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vrndscalepd	zmm30, zmm29{sae}, 0xab	 # AVX512F
	vrndscalepd	zmm30, zmm29, 123	 # AVX512F
	vrndscalepd	zmm30, zmm29{sae}, 123	 # AVX512F
	vrndscalepd	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vrndscalepd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vrndscalepd	zmm30, qword bcst [rcx], 123	 # AVX512F
	vrndscalepd	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vrndscalepd	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vrndscalepd	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vrndscalepd	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vrndscalepd	zmm30, qword bcst [rdx+1016], 123	 # AVX512F Disp8
	vrndscalepd	zmm30, qword bcst [rdx+1024], 123	 # AVX512F
	vrndscalepd	zmm30, qword bcst [rdx-1024], 123	 # AVX512F Disp8
	vrndscalepd	zmm30, qword bcst [rdx-1032], 123	 # AVX512F

	vrndscaleps	zmm30, zmm29, 0xab	 # AVX512F
	vrndscaleps	zmm30{k7}, zmm29, 0xab	 # AVX512F
	vrndscaleps	zmm30{k7}{z}, zmm29, 0xab	 # AVX512F
	vrndscaleps	zmm30, zmm29{sae}, 0xab	 # AVX512F
	vrndscaleps	zmm30, zmm29, 123	 # AVX512F
	vrndscaleps	zmm30, zmm29{sae}, 123	 # AVX512F
	vrndscaleps	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512F
	vrndscaleps	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vrndscaleps	zmm30, dword bcst [rcx], 123	 # AVX512F
	vrndscaleps	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512F Disp8
	vrndscaleps	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512F
	vrndscaleps	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512F Disp8
	vrndscaleps	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512F
	vrndscaleps	zmm30, dword bcst [rdx+508], 123	 # AVX512F Disp8
	vrndscaleps	zmm30, dword bcst [rdx+512], 123	 # AVX512F
	vrndscaleps	zmm30, dword bcst [rdx-512], 123	 # AVX512F Disp8
	vrndscaleps	zmm30, dword bcst [rdx-516], 123	 # AVX512F

	vrndscalesd	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512F
	vrndscalesd	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512F
	vrndscalesd	xmm30{k7}, xmm29, xmm28{sae}, 0xab	 # AVX512F
	vrndscalesd	xmm30{k7}, xmm29, xmm28, 123	 # AVX512F
	vrndscalesd	xmm30{k7}, xmm29, xmm28{sae}, 123	 # AVX512F
	vrndscalesd	xmm30{k7}, xmm29, QWORD PTR [rcx], 123	 # AVX512F
	vrndscalesd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vrndscalesd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016], 123	 # AVX512F Disp8
	vrndscalesd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024], 123	 # AVX512F
	vrndscalesd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024], 123	 # AVX512F Disp8
	vrndscalesd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032], 123	 # AVX512F

	vrndscaless	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512F
	vrndscaless	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512F
	vrndscaless	xmm30{k7}, xmm29, xmm28{sae}, 0xab	 # AVX512F
	vrndscaless	xmm30{k7}, xmm29, xmm28, 123	 # AVX512F
	vrndscaless	xmm30{k7}, xmm29, xmm28{sae}, 123	 # AVX512F
	vrndscaless	xmm30{k7}, xmm29, DWORD PTR [rcx], 123	 # AVX512F
	vrndscaless	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F
	vrndscaless	xmm30{k7}, xmm29, DWORD PTR [rdx+508], 123	 # AVX512F Disp8
	vrndscaless	xmm30{k7}, xmm29, DWORD PTR [rdx+512], 123	 # AVX512F
	vrndscaless	xmm30{k7}, xmm29, DWORD PTR [rdx-512], 123	 # AVX512F Disp8
	vrndscaless	xmm30{k7}, xmm29, DWORD PTR [rdx-516], 123	 # AVX512F

	vpcompressq	ZMMWORD PTR [rcx], zmm30	 # AVX512F
	vpcompressq	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpcompressq	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpcompressq	ZMMWORD PTR [rdx+1016], zmm30	 # AVX512F Disp8
	vpcompressq	ZMMWORD PTR [rdx+1024], zmm30	 # AVX512F
	vpcompressq	ZMMWORD PTR [rdx-1024], zmm30	 # AVX512F Disp8
	vpcompressq	ZMMWORD PTR [rdx-1032], zmm30	 # AVX512F

	vpcompressq	zmm30, zmm29	 # AVX512F
	vpcompressq	zmm30{k7}, zmm29	 # AVX512F
	vpcompressq	zmm30{k7}{z}, zmm29	 # AVX512F

	kandw	k5, k6, k7	 # AVX512F

	kandnw	k5, k6, k7	 # AVX512F

	korw	k5, k6, k7	 # AVX512F

	kxnorw	k5, k6, k7	 # AVX512F

	kxorw	k5, k6, k7	 # AVX512F

	knotw	k5, k6	 # AVX512F

	kortestw	k5, k6	 # AVX512F

	kshiftrw	k5, k6, 0xab	 # AVX512F
	kshiftrw	k5, k6, 123	 # AVX512F

	kshiftlw	k5, k6, 0xab	 # AVX512F
	kshiftlw	k5, k6, 123	 # AVX512F

	kmovw	k5, k6	 # AVX512F
	kmovw	k5, WORD PTR [rcx]	 # AVX512F
	kmovw	k5, WORD PTR [rax+r14*8+0x1234]	 # AVX512F

	kmovw	WORD PTR [rcx], k5	 # AVX512F
	kmovw	WORD PTR [rax+r14*8+0x1234], k5	 # AVX512F

	kmovw	k5, eax	 # AVX512F
	kmovw	k5, ebp	 # AVX512F
	kmovw	k5, r13d	 # AVX512F

	kmovw	eax, k5	 # AVX512F
	kmovw	ebp, k5	 # AVX512F
	kmovw	r13d, k5	 # AVX512F

	kunpckbw	k5, k6, k7	 # AVX512F

	vcvtps2ph	YMMWORD PTR [rcx], zmm30, 0xab	 # AVX512F
	vcvtps2ph	YMMWORD PTR [rcx]{k7}, zmm30, 0xab	 # AVX512F
	vcvtps2ph	YMMWORD PTR [rcx], zmm30, 123	 # AVX512F
	vcvtps2ph	YMMWORD PTR [rax+r14*8+0x1234], zmm30, 123	 # AVX512F
	vcvtps2ph	YMMWORD PTR [rdx+4064], zmm30, 123	 # AVX512F Disp8
	vcvtps2ph	YMMWORD PTR [rdx+4096], zmm30, 123	 # AVX512F
	vcvtps2ph	YMMWORD PTR [rdx-4096], zmm30, 123	 # AVX512F Disp8
	vcvtps2ph	YMMWORD PTR [rdx-4128], zmm30, 123	 # AVX512F

	vextractf32x4	XMMWORD PTR [rcx], zmm30, 0xab	 # AVX512F
	vextractf32x4	XMMWORD PTR [rcx]{k7}, zmm30, 0xab	 # AVX512F
	vextractf32x4	XMMWORD PTR [rcx], zmm30, 123	 # AVX512F
	vextractf32x4	XMMWORD PTR [rax+r14*8+0x1234], zmm30, 123	 # AVX512F
	vextractf32x4	XMMWORD PTR [rdx+2032], zmm30, 123	 # AVX512F Disp8
	vextractf32x4	XMMWORD PTR [rdx+2048], zmm30, 123	 # AVX512F
	vextractf32x4	XMMWORD PTR [rdx-2048], zmm30, 123	 # AVX512F Disp8
	vextractf32x4	XMMWORD PTR [rdx-2064], zmm30, 123	 # AVX512F

	vextractf64x4	YMMWORD PTR [rcx], zmm30, 0xab	 # AVX512F
	vextractf64x4	YMMWORD PTR [rcx]{k7}, zmm30, 0xab	 # AVX512F
	vextractf64x4	YMMWORD PTR [rcx], zmm30, 123	 # AVX512F
	vextractf64x4	YMMWORD PTR [rax+r14*8+0x1234], zmm30, 123	 # AVX512F
	vextractf64x4	YMMWORD PTR [rdx+4064], zmm30, 123	 # AVX512F Disp8
	vextractf64x4	YMMWORD PTR [rdx+4096], zmm30, 123	 # AVX512F
	vextractf64x4	YMMWORD PTR [rdx-4096], zmm30, 123	 # AVX512F Disp8
	vextractf64x4	YMMWORD PTR [rdx-4128], zmm30, 123	 # AVX512F

	vextracti32x4	XMMWORD PTR [rcx], zmm30, 0xab	 # AVX512F
	vextracti32x4	XMMWORD PTR [rcx]{k7}, zmm30, 0xab	 # AVX512F
	vextracti32x4	XMMWORD PTR [rcx], zmm30, 123	 # AVX512F
	vextracti32x4	XMMWORD PTR [rax+r14*8+0x1234], zmm30, 123	 # AVX512F
	vextracti32x4	XMMWORD PTR [rdx+2032], zmm30, 123	 # AVX512F Disp8
	vextracti32x4	XMMWORD PTR [rdx+2048], zmm30, 123	 # AVX512F
	vextracti32x4	XMMWORD PTR [rdx-2048], zmm30, 123	 # AVX512F Disp8
	vextracti32x4	XMMWORD PTR [rdx-2064], zmm30, 123	 # AVX512F

	vextracti64x4	YMMWORD PTR [rcx], zmm30, 0xab	 # AVX512F
	vextracti64x4	YMMWORD PTR [rcx]{k7}, zmm30, 0xab	 # AVX512F
	vextracti64x4	YMMWORD PTR [rcx], zmm30, 123	 # AVX512F
	vextracti64x4	YMMWORD PTR [rax+r14*8+0x1234], zmm30, 123	 # AVX512F
	vextracti64x4	YMMWORD PTR [rdx+4064], zmm30, 123	 # AVX512F Disp8
	vextracti64x4	YMMWORD PTR [rdx+4096], zmm30, 123	 # AVX512F
	vextracti64x4	YMMWORD PTR [rdx-4096], zmm30, 123	 # AVX512F Disp8
	vextracti64x4	YMMWORD PTR [rdx-4128], zmm30, 123	 # AVX512F

	vmovapd	ZMMWORD PTR [rcx], zmm30	 # AVX512F
	vmovapd	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vmovapd	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vmovapd	ZMMWORD PTR [rdx+8128], zmm30	 # AVX512F Disp8
	vmovapd	ZMMWORD PTR [rdx+8192], zmm30	 # AVX512F
	vmovapd	ZMMWORD PTR [rdx-8192], zmm30	 # AVX512F Disp8
	vmovapd	ZMMWORD PTR [rdx-8256], zmm30	 # AVX512F

	vmovaps	ZMMWORD PTR [rcx], zmm30	 # AVX512F
	vmovaps	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vmovaps	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vmovaps	ZMMWORD PTR [rdx+8128], zmm30	 # AVX512F Disp8
	vmovaps	ZMMWORD PTR [rdx+8192], zmm30	 # AVX512F
	vmovaps	ZMMWORD PTR [rdx-8192], zmm30	 # AVX512F Disp8
	vmovaps	ZMMWORD PTR [rdx-8256], zmm30	 # AVX512F

	vmovdqa32	ZMMWORD PTR [rcx], zmm30	 # AVX512F
	vmovdqa32	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vmovdqa32	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vmovdqa32	ZMMWORD PTR [rdx+8128], zmm30	 # AVX512F Disp8
	vmovdqa32	ZMMWORD PTR [rdx+8192], zmm30	 # AVX512F
	vmovdqa32	ZMMWORD PTR [rdx-8192], zmm30	 # AVX512F Disp8
	vmovdqa32	ZMMWORD PTR [rdx-8256], zmm30	 # AVX512F

	vmovdqa64	ZMMWORD PTR [rcx], zmm30	 # AVX512F
	vmovdqa64	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vmovdqa64	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vmovdqa64	ZMMWORD PTR [rdx+8128], zmm30	 # AVX512F Disp8
	vmovdqa64	ZMMWORD PTR [rdx+8192], zmm30	 # AVX512F
	vmovdqa64	ZMMWORD PTR [rdx-8192], zmm30	 # AVX512F Disp8
	vmovdqa64	ZMMWORD PTR [rdx-8256], zmm30	 # AVX512F

	vmovdqu32	ZMMWORD PTR [rcx], zmm30	 # AVX512F
	vmovdqu32	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vmovdqu32	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vmovdqu32	ZMMWORD PTR [rdx+8128], zmm30	 # AVX512F Disp8
	vmovdqu32	ZMMWORD PTR [rdx+8192], zmm30	 # AVX512F
	vmovdqu32	ZMMWORD PTR [rdx-8192], zmm30	 # AVX512F Disp8
	vmovdqu32	ZMMWORD PTR [rdx-8256], zmm30	 # AVX512F

	vmovdqu64	ZMMWORD PTR [rcx], zmm30	 # AVX512F
	vmovdqu64	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vmovdqu64	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vmovdqu64	ZMMWORD PTR [rdx+8128], zmm30	 # AVX512F Disp8
	vmovdqu64	ZMMWORD PTR [rdx+8192], zmm30	 # AVX512F
	vmovdqu64	ZMMWORD PTR [rdx-8192], zmm30	 # AVX512F Disp8
	vmovdqu64	ZMMWORD PTR [rdx-8256], zmm30	 # AVX512F

	vmovupd	ZMMWORD PTR [rcx], zmm30	 # AVX512F
	vmovupd	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vmovupd	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vmovupd	ZMMWORD PTR [rdx+8128], zmm30	 # AVX512F Disp8
	vmovupd	ZMMWORD PTR [rdx+8192], zmm30	 # AVX512F
	vmovupd	ZMMWORD PTR [rdx-8192], zmm30	 # AVX512F Disp8
	vmovupd	ZMMWORD PTR [rdx-8256], zmm30	 # AVX512F

	vmovups	ZMMWORD PTR [rcx], zmm30	 # AVX512F
	vmovups	ZMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vmovups	ZMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vmovups	ZMMWORD PTR [rdx+8128], zmm30	 # AVX512F Disp8
	vmovups	ZMMWORD PTR [rdx+8192], zmm30	 # AVX512F
	vmovups	ZMMWORD PTR [rdx-8192], zmm30	 # AVX512F Disp8
	vmovups	ZMMWORD PTR [rdx-8256], zmm30	 # AVX512F

	vpmovqb	QWORD PTR [rcx], zmm30	 # AVX512F
	vpmovqb	QWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpmovqb	QWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpmovqb	QWORD PTR [rdx+1016], zmm30	 # AVX512F Disp8
	vpmovqb	QWORD PTR [rdx+1024], zmm30	 # AVX512F
	vpmovqb	QWORD PTR [rdx-1024], zmm30	 # AVX512F Disp8
	vpmovqb	QWORD PTR [rdx-1032], zmm30	 # AVX512F

	vpmovsqb	QWORD PTR [rcx], zmm30	 # AVX512F
	vpmovsqb	QWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpmovsqb	QWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpmovsqb	QWORD PTR [rdx+1016], zmm30	 # AVX512F Disp8
	vpmovsqb	QWORD PTR [rdx+1024], zmm30	 # AVX512F
	vpmovsqb	QWORD PTR [rdx-1024], zmm30	 # AVX512F Disp8
	vpmovsqb	QWORD PTR [rdx-1032], zmm30	 # AVX512F

	vpmovusqb	QWORD PTR [rcx], zmm30	 # AVX512F
	vpmovusqb	QWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpmovusqb	QWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpmovusqb	QWORD PTR [rdx+1016], zmm30	 # AVX512F Disp8
	vpmovusqb	QWORD PTR [rdx+1024], zmm30	 # AVX512F
	vpmovusqb	QWORD PTR [rdx-1024], zmm30	 # AVX512F Disp8
	vpmovusqb	QWORD PTR [rdx-1032], zmm30	 # AVX512F

	vpmovqw	XMMWORD PTR [rcx], zmm30	 # AVX512F
	vpmovqw	XMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpmovqw	XMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpmovqw	XMMWORD PTR [rdx+2032], zmm30	 # AVX512F Disp8
	vpmovqw	XMMWORD PTR [rdx+2048], zmm30	 # AVX512F
	vpmovqw	XMMWORD PTR [rdx-2048], zmm30	 # AVX512F Disp8
	vpmovqw	XMMWORD PTR [rdx-2064], zmm30	 # AVX512F

	vpmovsqw	XMMWORD PTR [rcx], zmm30	 # AVX512F
	vpmovsqw	XMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpmovsqw	XMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpmovsqw	XMMWORD PTR [rdx+2032], zmm30	 # AVX512F Disp8
	vpmovsqw	XMMWORD PTR [rdx+2048], zmm30	 # AVX512F
	vpmovsqw	XMMWORD PTR [rdx-2048], zmm30	 # AVX512F Disp8
	vpmovsqw	XMMWORD PTR [rdx-2064], zmm30	 # AVX512F

	vpmovusqw	XMMWORD PTR [rcx], zmm30	 # AVX512F
	vpmovusqw	XMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpmovusqw	XMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpmovusqw	XMMWORD PTR [rdx+2032], zmm30	 # AVX512F Disp8
	vpmovusqw	XMMWORD PTR [rdx+2048], zmm30	 # AVX512F
	vpmovusqw	XMMWORD PTR [rdx-2048], zmm30	 # AVX512F Disp8
	vpmovusqw	XMMWORD PTR [rdx-2064], zmm30	 # AVX512F

	vpmovqd	YMMWORD PTR [rcx], zmm30	 # AVX512F
	vpmovqd	YMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpmovqd	YMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpmovqd	YMMWORD PTR [rdx+4064], zmm30	 # AVX512F Disp8
	vpmovqd	YMMWORD PTR [rdx+4096], zmm30	 # AVX512F
	vpmovqd	YMMWORD PTR [rdx-4096], zmm30	 # AVX512F Disp8
	vpmovqd	YMMWORD PTR [rdx-4128], zmm30	 # AVX512F

	vpmovsqd	YMMWORD PTR [rcx], zmm30	 # AVX512F
	vpmovsqd	YMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpmovsqd	YMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpmovsqd	YMMWORD PTR [rdx+4064], zmm30	 # AVX512F Disp8
	vpmovsqd	YMMWORD PTR [rdx+4096], zmm30	 # AVX512F
	vpmovsqd	YMMWORD PTR [rdx-4096], zmm30	 # AVX512F Disp8
	vpmovsqd	YMMWORD PTR [rdx-4128], zmm30	 # AVX512F

	vpmovusqd	YMMWORD PTR [rcx], zmm30	 # AVX512F
	vpmovusqd	YMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpmovusqd	YMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpmovusqd	YMMWORD PTR [rdx+4064], zmm30	 # AVX512F Disp8
	vpmovusqd	YMMWORD PTR [rdx+4096], zmm30	 # AVX512F
	vpmovusqd	YMMWORD PTR [rdx-4096], zmm30	 # AVX512F Disp8
	vpmovusqd	YMMWORD PTR [rdx-4128], zmm30	 # AVX512F

	vpmovdb	XMMWORD PTR [rcx], zmm30	 # AVX512F
	vpmovdb	XMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpmovdb	XMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpmovdb	XMMWORD PTR [rdx+2032], zmm30	 # AVX512F Disp8
	vpmovdb	XMMWORD PTR [rdx+2048], zmm30	 # AVX512F
	vpmovdb	XMMWORD PTR [rdx-2048], zmm30	 # AVX512F Disp8
	vpmovdb	XMMWORD PTR [rdx-2064], zmm30	 # AVX512F

	vpmovsdb	XMMWORD PTR [rcx], zmm30	 # AVX512F
	vpmovsdb	XMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpmovsdb	XMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpmovsdb	XMMWORD PTR [rdx+2032], zmm30	 # AVX512F Disp8
	vpmovsdb	XMMWORD PTR [rdx+2048], zmm30	 # AVX512F
	vpmovsdb	XMMWORD PTR [rdx-2048], zmm30	 # AVX512F Disp8
	vpmovsdb	XMMWORD PTR [rdx-2064], zmm30	 # AVX512F

	vpmovusdb	XMMWORD PTR [rcx], zmm30	 # AVX512F
	vpmovusdb	XMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpmovusdb	XMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpmovusdb	XMMWORD PTR [rdx+2032], zmm30	 # AVX512F Disp8
	vpmovusdb	XMMWORD PTR [rdx+2048], zmm30	 # AVX512F
	vpmovusdb	XMMWORD PTR [rdx-2048], zmm30	 # AVX512F Disp8
	vpmovusdb	XMMWORD PTR [rdx-2064], zmm30	 # AVX512F

	vpmovdw	YMMWORD PTR [rcx], zmm30	 # AVX512F
	vpmovdw	YMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpmovdw	YMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpmovdw	YMMWORD PTR [rdx+4064], zmm30	 # AVX512F Disp8
	vpmovdw	YMMWORD PTR [rdx+4096], zmm30	 # AVX512F
	vpmovdw	YMMWORD PTR [rdx-4096], zmm30	 # AVX512F Disp8
	vpmovdw	YMMWORD PTR [rdx-4128], zmm30	 # AVX512F

	vpmovsdw	YMMWORD PTR [rcx], zmm30	 # AVX512F
	vpmovsdw	YMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpmovsdw	YMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpmovsdw	YMMWORD PTR [rdx+4064], zmm30	 # AVX512F Disp8
	vpmovsdw	YMMWORD PTR [rdx+4096], zmm30	 # AVX512F
	vpmovsdw	YMMWORD PTR [rdx-4096], zmm30	 # AVX512F Disp8
	vpmovsdw	YMMWORD PTR [rdx-4128], zmm30	 # AVX512F

	vpmovusdw	YMMWORD PTR [rcx], zmm30	 # AVX512F
	vpmovusdw	YMMWORD PTR [rcx]{k7}, zmm30	 # AVX512F
	vpmovusdw	YMMWORD PTR [rax+r14*8+0x1234], zmm30	 # AVX512F
	vpmovusdw	YMMWORD PTR [rdx+4064], zmm30	 # AVX512F Disp8
	vpmovusdw	YMMWORD PTR [rdx+4096], zmm30	 # AVX512F
	vpmovusdw	YMMWORD PTR [rdx-4096], zmm30	 # AVX512F Disp8
	vpmovusdw	YMMWORD PTR [rdx-4128], zmm30	 # AVX512F

	vcvttpd2udq	ymm30{k7}, zmm29	 # AVX512F
	vcvttpd2udq	ymm30{k7}{z}, zmm29	 # AVX512F
	vcvttpd2udq	ymm30{k7}, zmm29{sae}	 # AVX512F
	vcvttpd2udq	ymm30{k7}, ZMMWORD PTR [rcx]	 # AVX512F
	vcvttpd2udq	ymm30{k7}, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvttpd2udq	ymm30{k7}, qword bcst [rcx]	 # AVX512F
	vcvttpd2udq	ymm30{k7}, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcvttpd2udq	ymm30{k7}, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcvttpd2udq	ymm30{k7}, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcvttpd2udq	ymm30{k7}, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcvttpd2udq	ymm30{k7}, qword bcst [rdx+1016]	 # AVX512F Disp8
	vcvttpd2udq	ymm30{k7}, qword bcst [rdx+1024]	 # AVX512F
	vcvttpd2udq	ymm30{k7}, qword bcst [rdx-1024]	 # AVX512F Disp8
	vcvttpd2udq	ymm30{k7}, qword bcst [rdx-1032]	 # AVX512F

	vcvttps2udq	zmm30, zmm29	 # AVX512F
	vcvttps2udq	zmm30{k7}, zmm29	 # AVX512F
	vcvttps2udq	zmm30{k7}{z}, zmm29	 # AVX512F
	vcvttps2udq	zmm30, zmm29{sae}	 # AVX512F
	vcvttps2udq	zmm30, ZMMWORD PTR [rcx]	 # AVX512F
	vcvttps2udq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvttps2udq	zmm30, dword bcst [rcx]	 # AVX512F
	vcvttps2udq	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vcvttps2udq	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vcvttps2udq	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vcvttps2udq	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vcvttps2udq	zmm30, dword bcst [rdx+508]	 # AVX512F Disp8
	vcvttps2udq	zmm30, dword bcst [rdx+512]	 # AVX512F
	vcvttps2udq	zmm30, dword bcst [rdx-512]	 # AVX512F Disp8
	vcvttps2udq	zmm30, dword bcst [rdx-516]	 # AVX512F

	vcvttsd2usi	eax, xmm30	 # AVX512F
	vcvttsd2usi	eax, xmm30{sae}	 # AVX512F
	vcvttsd2usi	eax, QWORD PTR [rcx]	 # AVX512F
	vcvttsd2usi	eax, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvttsd2usi	eax, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcvttsd2usi	eax, QWORD PTR [rdx+1024]	 # AVX512F
	vcvttsd2usi	eax, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcvttsd2usi	eax, QWORD PTR [rdx-1032]	 # AVX512F
	vcvttsd2usi	ebp, xmm30	 # AVX512F
	vcvttsd2usi	ebp, xmm30{sae}	 # AVX512F
	vcvttsd2usi	ebp, QWORD PTR [rcx]	 # AVX512F
	vcvttsd2usi	ebp, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvttsd2usi	ebp, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcvttsd2usi	ebp, QWORD PTR [rdx+1024]	 # AVX512F
	vcvttsd2usi	ebp, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcvttsd2usi	ebp, QWORD PTR [rdx-1032]	 # AVX512F
	vcvttsd2usi	r13d, xmm30	 # AVX512F
	vcvttsd2usi	r13d, xmm30{sae}	 # AVX512F
	vcvttsd2usi	r13d, QWORD PTR [rcx]	 # AVX512F
	vcvttsd2usi	r13d, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvttsd2usi	r13d, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcvttsd2usi	r13d, QWORD PTR [rdx+1024]	 # AVX512F
	vcvttsd2usi	r13d, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcvttsd2usi	r13d, QWORD PTR [rdx-1032]	 # AVX512F

	vcvttsd2usi	rax, xmm30	 # AVX512F
	vcvttsd2usi	rax, xmm30{sae}	 # AVX512F
	vcvttsd2usi	rax, QWORD PTR [rcx]	 # AVX512F
	vcvttsd2usi	rax, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvttsd2usi	rax, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcvttsd2usi	rax, QWORD PTR [rdx+1024]	 # AVX512F
	vcvttsd2usi	rax, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcvttsd2usi	rax, QWORD PTR [rdx-1032]	 # AVX512F
	vcvttsd2usi	r8, xmm30	 # AVX512F
	vcvttsd2usi	r8, xmm30{sae}	 # AVX512F
	vcvttsd2usi	r8, QWORD PTR [rcx]	 # AVX512F
	vcvttsd2usi	r8, QWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvttsd2usi	r8, QWORD PTR [rdx+1016]	 # AVX512F Disp8
	vcvttsd2usi	r8, QWORD PTR [rdx+1024]	 # AVX512F
	vcvttsd2usi	r8, QWORD PTR [rdx-1024]	 # AVX512F Disp8
	vcvttsd2usi	r8, QWORD PTR [rdx-1032]	 # AVX512F

	vcvttss2usi	eax, xmm30	 # AVX512F
	vcvttss2usi	eax, xmm30{sae}	 # AVX512F
	vcvttss2usi	eax, DWORD PTR [rcx]	 # AVX512F
	vcvttss2usi	eax, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvttss2usi	eax, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcvttss2usi	eax, DWORD PTR [rdx+512]	 # AVX512F
	vcvttss2usi	eax, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcvttss2usi	eax, DWORD PTR [rdx-516]	 # AVX512F
	vcvttss2usi	ebp, xmm30	 # AVX512F
	vcvttss2usi	ebp, xmm30{sae}	 # AVX512F
	vcvttss2usi	ebp, DWORD PTR [rcx]	 # AVX512F
	vcvttss2usi	ebp, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvttss2usi	ebp, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcvttss2usi	ebp, DWORD PTR [rdx+512]	 # AVX512F
	vcvttss2usi	ebp, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcvttss2usi	ebp, DWORD PTR [rdx-516]	 # AVX512F
	vcvttss2usi	r13d, xmm30	 # AVX512F
	vcvttss2usi	r13d, xmm30{sae}	 # AVX512F
	vcvttss2usi	r13d, DWORD PTR [rcx]	 # AVX512F
	vcvttss2usi	r13d, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvttss2usi	r13d, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcvttss2usi	r13d, DWORD PTR [rdx+512]	 # AVX512F
	vcvttss2usi	r13d, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcvttss2usi	r13d, DWORD PTR [rdx-516]	 # AVX512F

	vcvttss2usi	rax, xmm30	 # AVX512F
	vcvttss2usi	rax, xmm30{sae}	 # AVX512F
	vcvttss2usi	rax, DWORD PTR [rcx]	 # AVX512F
	vcvttss2usi	rax, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvttss2usi	rax, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcvttss2usi	rax, DWORD PTR [rdx+512]	 # AVX512F
	vcvttss2usi	rax, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcvttss2usi	rax, DWORD PTR [rdx-516]	 # AVX512F
	vcvttss2usi	r8, xmm30	 # AVX512F
	vcvttss2usi	r8, xmm30{sae}	 # AVX512F
	vcvttss2usi	r8, DWORD PTR [rcx]	 # AVX512F
	vcvttss2usi	r8, DWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vcvttss2usi	r8, DWORD PTR [rdx+508]	 # AVX512F Disp8
	vcvttss2usi	r8, DWORD PTR [rdx+512]	 # AVX512F
	vcvttss2usi	r8, DWORD PTR [rdx-512]	 # AVX512F Disp8
	vcvttss2usi	r8, DWORD PTR [rdx-516]	 # AVX512F

	vpermi2d	zmm30, zmm29, zmm28	 # AVX512F
	vpermi2d	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpermi2d	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpermi2d	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpermi2d	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpermi2d	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpermi2d	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpermi2d	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpermi2d	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpermi2d	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpermi2d	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpermi2d	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpermi2d	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpermi2d	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpermi2q	zmm30, zmm29, zmm28	 # AVX512F
	vpermi2q	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpermi2q	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpermi2q	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpermi2q	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpermi2q	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpermi2q	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpermi2q	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpermi2q	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpermi2q	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpermi2q	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpermi2q	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpermi2q	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpermi2q	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vpermi2ps	zmm30, zmm29, zmm28	 # AVX512F
	vpermi2ps	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpermi2ps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpermi2ps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpermi2ps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpermi2ps	zmm30, zmm29, dword bcst [rcx]	 # AVX512F
	vpermi2ps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpermi2ps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpermi2ps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpermi2ps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpermi2ps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512F Disp8
	vpermi2ps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512F
	vpermi2ps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512F Disp8
	vpermi2ps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512F

	vpermi2pd	zmm30, zmm29, zmm28	 # AVX512F
	vpermi2pd	zmm30{k7}, zmm29, zmm28	 # AVX512F
	vpermi2pd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F
	vpermi2pd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512F
	vpermi2pd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F
	vpermi2pd	zmm30, zmm29, qword bcst [rcx]	 # AVX512F
	vpermi2pd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F Disp8
	vpermi2pd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512F
	vpermi2pd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512F Disp8
	vpermi2pd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512F
	vpermi2pd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512F Disp8
	vpermi2pd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512F
	vpermi2pd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512F Disp8
	vpermi2pd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512F

	vptestnmd	k5, zmm29, zmm28	 # AVX512CD
	vptestnmd	k5{k7}, zmm29, zmm28	 # AVX512CD
	vptestnmd	k5, zmm29, ZMMWORD PTR [rcx]	 # AVX512CD
	vptestnmd	k5, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512CD
	vptestnmd	k5, zmm29, dword bcst [rcx]	 # AVX512CD
	vptestnmd	k5, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512CD Disp8
	vptestnmd	k5, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512CD
	vptestnmd	k5, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512CD Disp8
	vptestnmd	k5, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512CD
	vptestnmd	k5, zmm29, dword bcst [rdx+508]	 # AVX512CD Disp8
	vptestnmd	k5, zmm29, dword bcst [rdx+512]	 # AVX512CD
	vptestnmd	k5, zmm29, dword bcst [rdx-512]	 # AVX512CD Disp8
	vptestnmd	k5, zmm29, dword bcst [rdx-516]	 # AVX512CD

	vptestnmq	k5, zmm29, zmm28	 # AVX512CD
	vptestnmq	k5{k7}, zmm29, zmm28	 # AVX512CD
	vptestnmq	k5, zmm29, ZMMWORD PTR [rcx]	 # AVX512CD
	vptestnmq	k5, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512CD
	vptestnmq	k5, zmm29, qword bcst [rcx]	 # AVX512CD
	vptestnmq	k5, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512CD Disp8
	vptestnmq	k5, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512CD
	vptestnmq	k5, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512CD Disp8
	vptestnmq	k5, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512CD
	vptestnmq	k5, zmm29, qword bcst [rdx+1016]	 # AVX512CD Disp8
	vptestnmq	k5, zmm29, qword bcst [rdx+1024]	 # AVX512CD
	vptestnmq	k5, zmm29, qword bcst [rdx-1024]	 # AVX512CD Disp8
	vptestnmq	k5, zmm29, qword bcst [rdx-1032]	 # AVX512CD
