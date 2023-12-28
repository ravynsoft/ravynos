# Check 32bit AVX512F instructions

	.allow_index_reg
	.text
_start:

	vaddpd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vaddpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vaddpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vaddpd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vaddpd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vaddpd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vaddpd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vaddpd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vaddpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vaddpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vaddpd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vaddpd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vaddpd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vaddpd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vaddpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vaddpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vaddpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vaddpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vaddps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vaddps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vaddps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vaddps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vaddps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vaddps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vaddps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vaddps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vaddps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vaddps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vaddps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vaddps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vaddps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vaddps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vaddps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vaddps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vaddps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vaddps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vaddsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vaddsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vaddsd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vaddsd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vaddsd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vaddsd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vaddsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vaddsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vaddsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vaddsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vaddsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vaddsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vaddss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vaddss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vaddss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vaddss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vaddss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vaddss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vaddss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vaddss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vaddss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vaddss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vaddss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vaddss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	valignd	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512F
	valignd	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	valignd	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	valignd	$123, %zmm4, %zmm5, %zmm6	 # AVX512F
	valignd	$123, (%ecx), %zmm5, %zmm6	 # AVX512F
	valignd	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	valignd	$123, (%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	valignd	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	valignd	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512F
	valignd	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	valignd	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512F
	valignd	$123, 508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	valignd	$123, 512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	valignd	$123, -512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	valignd	$123, -516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vblendmpd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vblendmpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vblendmpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vblendmpd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vblendmpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vblendmpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vblendmpd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vblendmpd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vblendmpd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vblendmpd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vblendmpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vblendmpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vblendmpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vblendmpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vblendmps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vblendmps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vblendmps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vblendmps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vblendmps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vblendmps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vblendmps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vblendmps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vblendmps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vblendmps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vblendmps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vblendmps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vblendmps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vblendmps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vbroadcastf32x4	(%ecx), %zmm6	 # AVX512F
	vbroadcastf32x4	(%ecx), %zmm6{%k7}	 # AVX512F
	vbroadcastf32x4	(%ecx), %zmm6{%k7}{z}	 # AVX512F
	vbroadcastf32x4	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vbroadcastf32x4	2032(%edx), %zmm6	 # AVX512F Disp8
	vbroadcastf32x4	2048(%edx), %zmm6	 # AVX512F
	vbroadcastf32x4	-2048(%edx), %zmm6	 # AVX512F Disp8
	vbroadcastf32x4	-2064(%edx), %zmm6	 # AVX512F

	vbroadcastf64x4	(%ecx), %zmm6	 # AVX512F
	vbroadcastf64x4	(%ecx), %zmm6{%k7}	 # AVX512F
	vbroadcastf64x4	(%ecx), %zmm6{%k7}{z}	 # AVX512F
	vbroadcastf64x4	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vbroadcastf64x4	4064(%edx), %zmm6	 # AVX512F Disp8
	vbroadcastf64x4	4096(%edx), %zmm6	 # AVX512F
	vbroadcastf64x4	-4096(%edx), %zmm6	 # AVX512F Disp8
	vbroadcastf64x4	-4128(%edx), %zmm6	 # AVX512F

	vbroadcasti32x4	(%ecx), %zmm6	 # AVX512F
	vbroadcasti32x4	(%ecx), %zmm6{%k7}	 # AVX512F
	vbroadcasti32x4	(%ecx), %zmm6{%k7}{z}	 # AVX512F
	vbroadcasti32x4	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vbroadcasti32x4	2032(%edx), %zmm6	 # AVX512F Disp8
	vbroadcasti32x4	2048(%edx), %zmm6	 # AVX512F
	vbroadcasti32x4	-2048(%edx), %zmm6	 # AVX512F Disp8
	vbroadcasti32x4	-2064(%edx), %zmm6	 # AVX512F

	vbroadcasti64x4	(%ecx), %zmm6	 # AVX512F
	vbroadcasti64x4	(%ecx), %zmm6{%k7}	 # AVX512F
	vbroadcasti64x4	(%ecx), %zmm6{%k7}{z}	 # AVX512F
	vbroadcasti64x4	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vbroadcasti64x4	4064(%edx), %zmm6	 # AVX512F Disp8
	vbroadcasti64x4	4096(%edx), %zmm6	 # AVX512F
	vbroadcasti64x4	-4096(%edx), %zmm6	 # AVX512F Disp8
	vbroadcasti64x4	-4128(%edx), %zmm6	 # AVX512F

	vbroadcastsd	(%ecx), %zmm6	 # AVX512F
	vbroadcastsd	(%ecx), %zmm6{%k7}	 # AVX512F
	vbroadcastsd	(%ecx), %zmm6{%k7}{z}	 # AVX512F
	vbroadcastsd	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vbroadcastsd	1016(%edx), %zmm6	 # AVX512F Disp8
	vbroadcastsd	1024(%edx), %zmm6	 # AVX512F
	vbroadcastsd	-1024(%edx), %zmm6	 # AVX512F Disp8
	vbroadcastsd	-1032(%edx), %zmm6	 # AVX512F

	vbroadcastsd	%xmm5, %zmm6{%k7}	 # AVX512F
	vbroadcastsd	%xmm5, %zmm6{%k7}{z}	 # AVX512F

	vbroadcastss	(%ecx), %zmm6	 # AVX512F
	vbroadcastss	(%ecx), %zmm6{%k7}	 # AVX512F
	vbroadcastss	(%ecx), %zmm6{%k7}{z}	 # AVX512F
	vbroadcastss	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vbroadcastss	508(%edx), %zmm6	 # AVX512F Disp8
	vbroadcastss	512(%edx), %zmm6	 # AVX512F
	vbroadcastss	-512(%edx), %zmm6	 # AVX512F Disp8
	vbroadcastss	-516(%edx), %zmm6	 # AVX512F

	vbroadcastss	%xmm5, %zmm6{%k7}	 # AVX512F
	vbroadcastss	%xmm5, %zmm6{%k7}{z}	 # AVX512F

	vcmppd	$0xab, %zmm5, %zmm6, %k5	 # AVX512F
	vcmppd	$0xab, %zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmppd	$0xab, {sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmppd	$123, %zmm5, %zmm6, %k5	 # AVX512F
	vcmppd	$123, {sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmppd	$123, (%ecx), %zmm6, %k5	 # AVX512F
	vcmppd	$123, -123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmppd	$123, (%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmppd	$123, 8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmppd	$123, 8192(%edx), %zmm6, %k5	 # AVX512F
	vcmppd	$123, -8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmppd	$123, -8256(%edx), %zmm6, %k5	 # AVX512F
	vcmppd	$123, 1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmppd	$123, 1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmppd	$123, -1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmppd	$123, -1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpeq_oqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_oqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpeq_oqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_oqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpeq_oqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpeq_oqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpeq_oqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_oqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_oqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_oqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_oqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_oqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpeq_oqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_oqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpeqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpeqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpeqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpeqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpeqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpeqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpeqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpeqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpeqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpeqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmplt_ospd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmplt_ospd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmplt_ospd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmplt_ospd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmplt_ospd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmplt_ospd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmplt_ospd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmplt_ospd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmplt_ospd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmplt_ospd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmplt_ospd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmplt_ospd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmplt_ospd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmplt_ospd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpltpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpltpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpltpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpltpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpltpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpltpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpltpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpltpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpltpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpltpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpltpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpltpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpltpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpltpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmple_ospd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmple_ospd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmple_ospd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmple_ospd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmple_ospd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmple_ospd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmple_ospd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmple_ospd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmple_ospd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmple_ospd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmple_ospd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmple_ospd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmple_ospd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmple_ospd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmplepd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmplepd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmplepd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmplepd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmplepd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmplepd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmplepd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmplepd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmplepd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmplepd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmplepd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmplepd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmplepd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmplepd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpunord_qpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpunord_qpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpunord_qpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpunord_qpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpunord_qpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpunord_qpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpunord_qpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_qpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpunord_qpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_qpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpunord_qpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_qpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpunord_qpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_qpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpunordpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpunordpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpunordpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpunordpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpunordpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpunordpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpunordpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpunordpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpunordpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpunordpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpunordpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpunordpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpunordpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpunordpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpneq_uqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_uqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpneq_uqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_uqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpneq_uqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpneq_uqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpneq_uqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_uqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_uqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_uqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_uqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_uqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpneq_uqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_uqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpneqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpneqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpneqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpneqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpneqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpneqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpneqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpneqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpneqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpneqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpnlt_uspd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnlt_uspd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnlt_uspd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnlt_uspd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnlt_uspd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnlt_uspd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnlt_uspd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_uspd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnlt_uspd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_uspd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnlt_uspd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_uspd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnlt_uspd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_uspd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpnltpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnltpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnltpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnltpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnltpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnltpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnltpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnltpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnltpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnltpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnltpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnltpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnltpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnltpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpnle_uspd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnle_uspd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnle_uspd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnle_uspd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnle_uspd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnle_uspd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnle_uspd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_uspd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnle_uspd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_uspd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnle_uspd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_uspd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnle_uspd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_uspd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpnlepd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnlepd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnlepd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnlepd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnlepd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnlepd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnlepd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnlepd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnlepd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnlepd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnlepd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnlepd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnlepd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnlepd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpord_qpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpord_qpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpord_qpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpord_qpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpord_qpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpord_qpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpord_qpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpord_qpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpord_qpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpord_qpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpord_qpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpord_qpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpord_qpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpord_qpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpordpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpordpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpordpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpordpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpordpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpordpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpordpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpordpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpordpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpordpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpordpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpordpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpordpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpordpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpeq_uqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_uqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpeq_uqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_uqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpeq_uqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpeq_uqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpeq_uqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_uqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_uqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_uqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_uqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_uqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpeq_uqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_uqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpnge_uspd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnge_uspd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnge_uspd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnge_uspd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnge_uspd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnge_uspd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnge_uspd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_uspd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnge_uspd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_uspd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnge_uspd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_uspd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnge_uspd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_uspd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpngepd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpngepd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpngepd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpngepd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpngepd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpngepd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpngepd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngepd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpngepd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngepd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpngepd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngepd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpngepd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngepd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpngt_uspd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpngt_uspd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpngt_uspd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpngt_uspd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpngt_uspd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpngt_uspd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpngt_uspd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_uspd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpngt_uspd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_uspd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpngt_uspd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_uspd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpngt_uspd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_uspd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpngtpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpngtpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpngtpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpngtpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpngtpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpngtpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpngtpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngtpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpngtpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngtpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpngtpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngtpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpngtpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngtpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpfalse_oqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpfalse_oqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpfalse_oqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpfalse_oqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpfalse_oqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpfalse_oqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpfalse_oqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_oqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpfalse_oqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_oqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpfalse_oqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_oqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpfalse_oqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_oqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpfalsepd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpfalsepd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpfalsepd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpfalsepd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpfalsepd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpfalsepd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpfalsepd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpfalsepd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpfalsepd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpfalsepd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpfalsepd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpfalsepd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpfalsepd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpfalsepd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpneq_oqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_oqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpneq_oqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_oqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpneq_oqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpneq_oqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpneq_oqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_oqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_oqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_oqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_oqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_oqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpneq_oqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_oqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpge_ospd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpge_ospd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpge_ospd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpge_ospd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpge_ospd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpge_ospd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpge_ospd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpge_ospd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpge_ospd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpge_ospd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpge_ospd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpge_ospd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpge_ospd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpge_ospd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpgepd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpgepd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpgepd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpgepd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpgepd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpgepd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpgepd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgepd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpgepd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgepd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpgepd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgepd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpgepd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgepd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpgt_ospd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpgt_ospd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpgt_ospd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpgt_ospd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpgt_ospd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpgt_ospd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpgt_ospd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_ospd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpgt_ospd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_ospd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpgt_ospd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_ospd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpgt_ospd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_ospd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpgtpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpgtpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpgtpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpgtpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpgtpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpgtpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpgtpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgtpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpgtpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgtpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpgtpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgtpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpgtpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgtpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmptrue_uqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmptrue_uqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmptrue_uqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmptrue_uqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmptrue_uqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmptrue_uqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmptrue_uqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_uqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmptrue_uqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_uqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmptrue_uqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_uqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmptrue_uqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_uqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmptruepd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmptruepd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmptruepd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmptruepd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmptruepd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmptruepd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmptruepd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmptruepd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmptruepd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmptruepd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmptruepd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmptruepd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmptruepd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmptruepd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpeq_ospd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_ospd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpeq_ospd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_ospd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpeq_ospd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpeq_ospd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpeq_ospd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_ospd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_ospd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_ospd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_ospd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_ospd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpeq_ospd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_ospd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmplt_oqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmplt_oqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmplt_oqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmplt_oqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmplt_oqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmplt_oqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmplt_oqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmplt_oqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmplt_oqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmplt_oqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmplt_oqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmplt_oqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmplt_oqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmplt_oqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmple_oqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmple_oqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmple_oqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmple_oqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmple_oqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmple_oqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmple_oqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmple_oqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmple_oqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmple_oqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmple_oqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmple_oqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmple_oqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmple_oqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpunord_spd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpunord_spd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpunord_spd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpunord_spd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpunord_spd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpunord_spd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpunord_spd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_spd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpunord_spd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_spd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpunord_spd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_spd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpunord_spd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_spd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpneq_uspd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_uspd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpneq_uspd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_uspd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpneq_uspd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpneq_uspd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpneq_uspd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_uspd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_uspd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_uspd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_uspd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_uspd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpneq_uspd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_uspd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpnlt_uqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnlt_uqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnlt_uqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnlt_uqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnlt_uqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnlt_uqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnlt_uqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_uqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnlt_uqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_uqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnlt_uqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_uqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnlt_uqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_uqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpnle_uqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnle_uqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnle_uqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnle_uqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnle_uqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnle_uqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnle_uqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_uqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnle_uqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_uqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnle_uqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_uqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnle_uqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_uqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpord_spd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpord_spd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpord_spd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpord_spd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpord_spd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpord_spd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpord_spd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpord_spd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpord_spd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpord_spd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpord_spd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpord_spd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpord_spd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpord_spd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpeq_uspd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_uspd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpeq_uspd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_uspd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpeq_uspd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpeq_uspd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpeq_uspd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_uspd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_uspd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_uspd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_uspd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_uspd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpeq_uspd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_uspd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpnge_uqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnge_uqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnge_uqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnge_uqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnge_uqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnge_uqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnge_uqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_uqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnge_uqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_uqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnge_uqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_uqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpnge_uqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_uqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpngt_uqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpngt_uqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpngt_uqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpngt_uqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpngt_uqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpngt_uqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpngt_uqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_uqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpngt_uqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_uqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpngt_uqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_uqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpngt_uqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_uqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpfalse_ospd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpfalse_ospd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpfalse_ospd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpfalse_ospd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpfalse_ospd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpfalse_ospd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpfalse_ospd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_ospd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpfalse_ospd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_ospd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpfalse_ospd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_ospd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpfalse_ospd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_ospd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpneq_ospd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_ospd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpneq_ospd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_ospd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpneq_ospd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpneq_ospd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpneq_ospd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_ospd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_ospd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_ospd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_ospd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_ospd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpneq_ospd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_ospd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpge_oqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpge_oqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpge_oqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpge_oqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpge_oqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpge_oqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpge_oqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpge_oqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpge_oqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpge_oqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpge_oqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpge_oqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpge_oqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpge_oqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpgt_oqpd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpgt_oqpd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpgt_oqpd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpgt_oqpd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpgt_oqpd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpgt_oqpd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmpgt_oqpd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_oqpd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpgt_oqpd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_oqpd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpgt_oqpd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_oqpd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmpgt_oqpd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_oqpd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmptrue_uspd	%zmm5, %zmm6, %k5	 # AVX512F
	vcmptrue_uspd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmptrue_uspd	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmptrue_uspd	(%ecx), %zmm6, %k5	 # AVX512F
	vcmptrue_uspd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmptrue_uspd	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vcmptrue_uspd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_uspd	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmptrue_uspd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_uspd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmptrue_uspd	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_uspd	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vcmptrue_uspd	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_uspd	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vcmpps	$0xab, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpps	$0xab, %zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpps	$0xab, {sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpps	$123, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpps	$123, {sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpps	$123, (%ecx), %zmm6, %k5	 # AVX512F
	vcmpps	$123, -123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpps	$123, (%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpps	$123, 8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpps	$123, 8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpps	$123, -8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpps	$123, -8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpps	$123, 508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpps	$123, 512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpps	$123, -512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpps	$123, -516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpeq_oqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_oqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpeq_oqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_oqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpeq_oqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpeq_oqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpeq_oqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_oqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_oqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_oqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_oqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_oqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpeq_oqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_oqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpeqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpeqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpeqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpeqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpeqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpeqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpeqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpeqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpeqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpeqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmplt_osps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmplt_osps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmplt_osps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmplt_osps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmplt_osps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmplt_osps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmplt_osps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmplt_osps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmplt_osps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmplt_osps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmplt_osps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmplt_osps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmplt_osps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmplt_osps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpltps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpltps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpltps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpltps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpltps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpltps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpltps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpltps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpltps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpltps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpltps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpltps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpltps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpltps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmple_osps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmple_osps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmple_osps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmple_osps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmple_osps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmple_osps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmple_osps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmple_osps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmple_osps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmple_osps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmple_osps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmple_osps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmple_osps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmple_osps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpleps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpleps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpleps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpleps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpleps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpleps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpleps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpleps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpleps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpleps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpleps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpleps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpleps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpleps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpunord_qps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpunord_qps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpunord_qps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpunord_qps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpunord_qps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpunord_qps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpunord_qps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_qps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpunord_qps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_qps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpunord_qps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_qps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpunord_qps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_qps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpunordps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpunordps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpunordps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpunordps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpunordps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpunordps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpunordps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpunordps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpunordps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpunordps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpunordps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpunordps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpunordps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpunordps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpneq_uqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_uqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpneq_uqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_uqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpneq_uqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpneq_uqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpneq_uqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_uqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_uqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_uqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_uqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_uqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpneq_uqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_uqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpneqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpneqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpneqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpneqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpneqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpneqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpneqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpneqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpneqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpneqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpnlt_usps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnlt_usps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnlt_usps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnlt_usps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnlt_usps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnlt_usps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnlt_usps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_usps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnlt_usps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_usps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnlt_usps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_usps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnlt_usps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_usps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpnltps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnltps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnltps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnltps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnltps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnltps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnltps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnltps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnltps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnltps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnltps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnltps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnltps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnltps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpnle_usps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnle_usps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnle_usps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnle_usps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnle_usps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnle_usps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnle_usps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_usps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnle_usps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_usps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnle_usps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_usps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnle_usps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_usps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpnleps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnleps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnleps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnleps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnleps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnleps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnleps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnleps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnleps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnleps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnleps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnleps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnleps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnleps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpord_qps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpord_qps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpord_qps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpord_qps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpord_qps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpord_qps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpord_qps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpord_qps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpord_qps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpord_qps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpord_qps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpord_qps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpord_qps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpord_qps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpordps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpordps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpordps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpordps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpordps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpordps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpordps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpordps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpordps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpordps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpordps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpordps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpordps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpordps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpeq_uqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_uqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpeq_uqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_uqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpeq_uqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpeq_uqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpeq_uqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_uqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_uqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_uqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_uqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_uqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpeq_uqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_uqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpnge_usps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnge_usps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnge_usps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnge_usps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnge_usps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnge_usps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnge_usps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_usps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnge_usps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_usps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnge_usps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_usps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnge_usps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_usps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpngeps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpngeps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpngeps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpngeps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpngeps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpngeps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpngeps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngeps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpngeps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngeps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpngeps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngeps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpngeps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngeps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpngt_usps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpngt_usps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpngt_usps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpngt_usps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpngt_usps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpngt_usps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpngt_usps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_usps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpngt_usps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_usps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpngt_usps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_usps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpngt_usps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_usps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpngtps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpngtps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpngtps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpngtps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpngtps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpngtps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpngtps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngtps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpngtps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngtps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpngtps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngtps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpngtps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngtps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpfalse_oqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpfalse_oqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpfalse_oqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpfalse_oqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpfalse_oqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpfalse_oqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpfalse_oqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_oqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpfalse_oqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_oqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpfalse_oqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_oqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpfalse_oqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_oqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpfalseps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpfalseps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpfalseps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpfalseps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpfalseps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpfalseps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpfalseps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpfalseps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpfalseps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpfalseps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpfalseps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpfalseps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpfalseps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpfalseps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpneq_oqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_oqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpneq_oqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_oqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpneq_oqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpneq_oqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpneq_oqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_oqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_oqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_oqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_oqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_oqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpneq_oqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_oqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpge_osps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpge_osps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpge_osps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpge_osps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpge_osps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpge_osps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpge_osps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpge_osps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpge_osps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpge_osps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpge_osps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpge_osps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpge_osps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpge_osps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpgeps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpgeps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpgeps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpgeps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpgeps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpgeps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpgeps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgeps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpgeps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgeps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpgeps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgeps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpgeps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgeps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpgt_osps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpgt_osps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpgt_osps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpgt_osps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpgt_osps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpgt_osps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpgt_osps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_osps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpgt_osps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_osps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpgt_osps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_osps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpgt_osps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_osps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpgtps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpgtps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpgtps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpgtps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpgtps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpgtps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpgtps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgtps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpgtps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgtps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpgtps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgtps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpgtps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgtps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmptrue_uqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmptrue_uqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmptrue_uqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmptrue_uqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmptrue_uqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmptrue_uqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmptrue_uqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_uqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmptrue_uqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_uqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmptrue_uqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_uqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmptrue_uqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_uqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmptrueps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmptrueps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmptrueps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmptrueps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmptrueps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmptrueps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmptrueps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmptrueps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmptrueps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmptrueps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmptrueps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmptrueps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmptrueps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmptrueps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpeq_osps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_osps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpeq_osps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_osps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpeq_osps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpeq_osps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpeq_osps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_osps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_osps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_osps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_osps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_osps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpeq_osps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_osps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmplt_oqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmplt_oqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmplt_oqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmplt_oqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmplt_oqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmplt_oqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmplt_oqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmplt_oqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmplt_oqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmplt_oqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmplt_oqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmplt_oqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmplt_oqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmplt_oqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmple_oqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmple_oqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmple_oqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmple_oqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmple_oqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmple_oqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmple_oqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmple_oqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmple_oqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmple_oqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmple_oqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmple_oqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmple_oqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmple_oqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpunord_sps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpunord_sps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpunord_sps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpunord_sps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpunord_sps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpunord_sps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpunord_sps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_sps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpunord_sps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_sps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpunord_sps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_sps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpunord_sps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpunord_sps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpneq_usps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_usps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpneq_usps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_usps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpneq_usps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpneq_usps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpneq_usps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_usps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_usps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_usps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_usps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_usps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpneq_usps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_usps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpnlt_uqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnlt_uqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnlt_uqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnlt_uqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnlt_uqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnlt_uqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnlt_uqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_uqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnlt_uqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_uqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnlt_uqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_uqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnlt_uqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnlt_uqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpnle_uqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnle_uqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnle_uqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnle_uqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnle_uqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnle_uqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnle_uqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_uqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnle_uqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_uqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnle_uqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_uqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnle_uqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnle_uqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpord_sps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpord_sps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpord_sps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpord_sps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpord_sps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpord_sps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpord_sps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpord_sps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpord_sps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpord_sps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpord_sps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpord_sps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpord_sps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpord_sps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpeq_usps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_usps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpeq_usps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpeq_usps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpeq_usps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpeq_usps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpeq_usps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_usps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_usps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_usps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpeq_usps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_usps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpeq_usps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpeq_usps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpnge_uqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpnge_uqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpnge_uqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpnge_uqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpnge_uqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpnge_uqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnge_uqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_uqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpnge_uqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_uqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpnge_uqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_uqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpnge_uqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpnge_uqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpngt_uqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpngt_uqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpngt_uqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpngt_uqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpngt_uqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpngt_uqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpngt_uqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_uqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpngt_uqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_uqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpngt_uqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_uqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpngt_uqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpngt_uqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpfalse_osps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpfalse_osps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpfalse_osps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpfalse_osps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpfalse_osps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpfalse_osps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpfalse_osps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_osps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpfalse_osps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_osps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpfalse_osps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_osps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpfalse_osps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpfalse_osps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpneq_osps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_osps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpneq_osps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpneq_osps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpneq_osps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpneq_osps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpneq_osps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_osps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_osps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_osps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpneq_osps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_osps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpneq_osps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpneq_osps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpge_oqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpge_oqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpge_oqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpge_oqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpge_oqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpge_oqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpge_oqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpge_oqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpge_oqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpge_oqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpge_oqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpge_oqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpge_oqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpge_oqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpgt_oqps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmpgt_oqps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmpgt_oqps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmpgt_oqps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmpgt_oqps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmpgt_oqps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmpgt_oqps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_oqps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmpgt_oqps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_oqps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmpgt_oqps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_oqps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmpgt_oqps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmpgt_oqps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmptrue_usps	%zmm5, %zmm6, %k5	 # AVX512F
	vcmptrue_usps	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vcmptrue_usps	{sae}, %zmm5, %zmm6, %k5	 # AVX512F
	vcmptrue_usps	(%ecx), %zmm6, %k5	 # AVX512F
	vcmptrue_usps	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vcmptrue_usps	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vcmptrue_usps	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_usps	8192(%edx), %zmm6, %k5	 # AVX512F
	vcmptrue_usps	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_usps	-8256(%edx), %zmm6, %k5	 # AVX512F
	vcmptrue_usps	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_usps	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vcmptrue_usps	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vcmptrue_usps	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vcmpsd	$0xab, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpsd	$0xab, {sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpsd	$123, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpsd	$123, {sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpsd	$123, (%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpsd	$123, -123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpsd	$123, 1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpsd	$123, 1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpsd	$123, -1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpsd	$123, -1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpeq_oqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_oqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_oqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_oqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_oqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_oqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_oqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_oqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpeqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmplt_ossd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_ossd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_ossd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_ossd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_ossd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmplt_ossd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_ossd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmplt_ossd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpltsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpltsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpltsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpltsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpltsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpltsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpltsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpltsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmple_ossd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmple_ossd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmple_ossd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmple_ossd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmple_ossd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmple_ossd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmple_ossd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmple_ossd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmplesd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmplesd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmplesd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmplesd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmplesd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmplesd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmplesd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmplesd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpunord_qsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_qsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_qsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_qsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_qsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_qsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_qsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_qsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpunordsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpunordsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpunordsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunordsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunordsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpunordsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunordsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpunordsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpneq_uqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_uqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_uqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_uqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_uqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_uqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_uqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_uqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpneqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnlt_ussd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_ussd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_ussd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_ussd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_ussd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_ussd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_ussd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_ussd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnltsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnltsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnltsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnltsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnltsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnltsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnltsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnltsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnle_ussd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_ussd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_ussd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_ussd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_ussd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_ussd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_ussd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_ussd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnlesd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlesd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlesd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlesd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlesd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnlesd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlesd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnlesd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpord_qsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_qsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_qsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_qsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_qsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpord_qsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_qsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpord_qsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpordsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpordsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpordsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpordsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpordsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpordsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpordsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpordsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpeq_uqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_uqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_uqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_uqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_uqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_uqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_uqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_uqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnge_ussd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_ussd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_ussd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_ussd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_ussd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_ussd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_ussd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_ussd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpngesd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngesd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngesd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngesd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngesd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngesd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngesd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngesd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpngt_ussd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_ussd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_ussd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_ussd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_ussd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_ussd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_ussd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_ussd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpngtsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngtsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngtsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngtsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngtsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngtsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngtsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngtsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpfalse_oqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_oqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_oqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_oqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_oqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_oqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_oqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_oqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpfalsesd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalsesd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalsesd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalsesd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalsesd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpfalsesd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalsesd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpfalsesd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpneq_oqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_oqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_oqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_oqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_oqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_oqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_oqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_oqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpge_ossd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_ossd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_ossd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_ossd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_ossd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpge_ossd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_ossd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpge_ossd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpgesd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgesd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgesd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgesd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgesd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgesd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgesd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgesd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpgt_ossd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_ossd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_ossd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_ossd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_ossd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_ossd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_ossd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_ossd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpgtsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgtsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgtsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgtsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgtsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgtsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgtsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgtsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmptrue_uqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_uqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_uqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_uqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_uqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_uqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_uqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_uqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmptruesd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmptruesd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmptruesd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmptruesd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmptruesd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmptruesd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmptruesd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmptruesd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpeq_ossd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_ossd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_ossd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_ossd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_ossd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_ossd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_ossd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_ossd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmplt_oqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_oqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_oqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_oqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_oqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmplt_oqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_oqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmplt_oqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmple_oqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmple_oqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmple_oqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmple_oqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmple_oqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmple_oqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmple_oqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmple_oqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpunord_ssd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_ssd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_ssd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_ssd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_ssd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_ssd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_ssd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_ssd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpneq_ussd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_ussd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_ussd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_ussd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_ussd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_ussd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_ussd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_ussd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnlt_uqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_uqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_uqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_uqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_uqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_uqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_uqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_uqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnle_uqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_uqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_uqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_uqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_uqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_uqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_uqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_uqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpord_ssd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_ssd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_ssd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_ssd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_ssd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpord_ssd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_ssd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpord_ssd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpeq_ussd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_ussd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_ussd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_ussd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_ussd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_ussd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_ussd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_ussd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnge_uqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_uqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_uqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_uqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_uqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_uqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_uqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_uqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpngt_uqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_uqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_uqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_uqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_uqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_uqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_uqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_uqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpfalse_ossd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_ossd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_ossd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_ossd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_ossd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_ossd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_ossd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_ossd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpneq_ossd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_ossd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_ossd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_ossd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_ossd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_ossd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_ossd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_ossd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpge_oqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_oqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_oqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_oqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_oqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpge_oqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_oqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpge_oqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpgt_oqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_oqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_oqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_oqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_oqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_oqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_oqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_oqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmptrue_ussd	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_ussd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_ussd	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_ussd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_ussd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_ussd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_ussd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_ussd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpss	$0xab, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpss	$0xab, {sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpss	$123, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpss	$123, {sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpss	$123, (%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpss	$123, -123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpss	$123, 508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpss	$123, 512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpss	$123, -512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpss	$123, -516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpeq_oqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_oqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_oqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_oqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_oqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_oqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_oqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_oqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpeqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmplt_osss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_osss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_osss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_osss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_osss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmplt_osss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_osss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmplt_osss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpltss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpltss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpltss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpltss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpltss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpltss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpltss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpltss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmple_osss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmple_osss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmple_osss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmple_osss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmple_osss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmple_osss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmple_osss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmple_osss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpless	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpless	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpless	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpless	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpless	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpless	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpless	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpless	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpunord_qss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_qss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_qss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_qss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_qss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_qss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_qss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_qss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpunordss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpunordss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpunordss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunordss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunordss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpunordss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunordss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpunordss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpneq_uqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_uqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_uqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_uqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_uqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_uqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_uqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_uqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpneqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnlt_usss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_usss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_usss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_usss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_usss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_usss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_usss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_usss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnltss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnltss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnltss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnltss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnltss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnltss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnltss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnltss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnle_usss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_usss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_usss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_usss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_usss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_usss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_usss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_usss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnless	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnless	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnless	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnless	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnless	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnless	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnless	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnless	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpord_qss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_qss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_qss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_qss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_qss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpord_qss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_qss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpord_qss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpordss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpordss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpordss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpordss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpordss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpordss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpordss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpordss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpeq_uqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_uqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_uqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_uqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_uqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_uqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_uqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_uqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnge_usss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_usss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_usss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_usss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_usss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_usss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_usss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_usss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpngess	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngess	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngess	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngess	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngess	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngess	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngess	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngess	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpngt_usss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_usss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_usss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_usss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_usss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_usss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_usss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_usss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpngtss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngtss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngtss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngtss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngtss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngtss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngtss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngtss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpfalse_oqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_oqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_oqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_oqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_oqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_oqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_oqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_oqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpfalsess	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalsess	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalsess	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalsess	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalsess	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpfalsess	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalsess	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpfalsess	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpneq_oqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_oqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_oqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_oqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_oqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_oqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_oqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_oqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpge_osss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_osss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_osss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_osss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_osss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpge_osss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_osss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpge_osss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpgess	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgess	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgess	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgess	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgess	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgess	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgess	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgess	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpgt_osss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_osss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_osss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_osss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_osss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_osss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_osss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_osss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpgtss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgtss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgtss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgtss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgtss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgtss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgtss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgtss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmptrue_uqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_uqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_uqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_uqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_uqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_uqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_uqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_uqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmptruess	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmptruess	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmptruess	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmptruess	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmptruess	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmptruess	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmptruess	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmptruess	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpeq_osss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_osss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_osss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_osss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_osss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_osss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_osss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_osss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmplt_oqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_oqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_oqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_oqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_oqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmplt_oqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmplt_oqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmplt_oqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmple_oqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmple_oqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmple_oqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmple_oqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmple_oqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmple_oqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmple_oqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmple_oqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpunord_sss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_sss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_sss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_sss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_sss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_sss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpunord_sss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpunord_sss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpneq_usss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_usss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_usss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_usss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_usss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_usss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_usss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_usss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnlt_uqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_uqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_uqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_uqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_uqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_uqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnlt_uqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnlt_uqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnle_uqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_uqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_uqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_uqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_uqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_uqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnle_uqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnle_uqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpord_sss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_sss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_sss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_sss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_sss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpord_sss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpord_sss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpord_sss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpeq_usss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_usss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_usss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_usss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_usss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_usss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpeq_usss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpeq_usss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpnge_uqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_uqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_uqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_uqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_uqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_uqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpnge_uqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpnge_uqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpngt_uqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_uqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_uqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_uqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_uqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_uqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpngt_uqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpngt_uqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpfalse_osss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_osss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_osss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_osss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_osss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_osss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpfalse_osss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpfalse_osss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpneq_osss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_osss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_osss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_osss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_osss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_osss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpneq_osss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpneq_osss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpge_oqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_oqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_oqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_oqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_oqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpge_oqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpge_oqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpge_oqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmpgt_oqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_oqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_oqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_oqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_oqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_oqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmpgt_oqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmpgt_oqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcmptrue_usss	%xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_usss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_usss	(%ecx), %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_usss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_usss	508(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_usss	512(%edx), %xmm5, %k5{%k7}	 # AVX512F
	vcmptrue_usss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512F Disp8
	vcmptrue_usss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512F

	vcomisd	{sae}, %xmm5, %xmm6	 # AVX512F

	vcomiss	{sae}, %xmm5, %xmm6	 # AVX512F

	vcompresspd	%zmm6, (%ecx)	 # AVX512F
	vcompresspd	%zmm6, (%ecx){%k7}	 # AVX512F
	vcompresspd	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vcompresspd	%zmm6, 1016(%edx)	 # AVX512F Disp8
	vcompresspd	%zmm6, 1024(%edx)	 # AVX512F
	vcompresspd	%zmm6, -1024(%edx)	 # AVX512F Disp8
	vcompresspd	%zmm6, -1032(%edx)	 # AVX512F

	vcompresspd	%zmm5, %zmm6	 # AVX512F
	vcompresspd	%zmm5, %zmm6{%k7}	 # AVX512F
	vcompresspd	%zmm5, %zmm6{%k7}{z}	 # AVX512F

	vcompressps	%zmm6, (%ecx)	 # AVX512F
	vcompressps	%zmm6, (%ecx){%k7}	 # AVX512F
	vcompressps	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vcompressps	%zmm6, 508(%edx)	 # AVX512F Disp8
	vcompressps	%zmm6, 512(%edx)	 # AVX512F
	vcompressps	%zmm6, -512(%edx)	 # AVX512F Disp8
	vcompressps	%zmm6, -516(%edx)	 # AVX512F

	vcompressps	%zmm5, %zmm6	 # AVX512F
	vcompressps	%zmm5, %zmm6{%k7}	 # AVX512F
	vcompressps	%zmm5, %zmm6{%k7}{z}	 # AVX512F

	vcvtdq2pd	%ymm5, %zmm6{%k7}	 # AVX512F
	vcvtdq2pd	%ymm5, %zmm6{%k7}{z}	 # AVX512F
	vcvtdq2pd	(%ecx), %zmm6{%k7}	 # AVX512F
	vcvtdq2pd	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512F
	vcvtdq2pd	(%eax){1to8}, %zmm6{%k7}	 # AVX512F
	vcvtdq2pd	4064(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vcvtdq2pd	4096(%edx), %zmm6{%k7}	 # AVX512F
	vcvtdq2pd	-4096(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vcvtdq2pd	-4128(%edx), %zmm6{%k7}	 # AVX512F
	vcvtdq2pd	508(%edx){1to8}, %zmm6{%k7}	 # AVX512F Disp8
	vcvtdq2pd	512(%edx){1to8}, %zmm6{%k7}	 # AVX512F
	vcvtdq2pd	-512(%edx){1to8}, %zmm6{%k7}	 # AVX512F Disp8
	vcvtdq2pd	-516(%edx){1to8}, %zmm6{%k7}	 # AVX512F

	vcvtdq2ps	%zmm5, %zmm6	 # AVX512F
	vcvtdq2ps	%zmm5, %zmm6{%k7}	 # AVX512F
	vcvtdq2ps	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vcvtdq2ps	{rn-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtdq2ps	{ru-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtdq2ps	{rd-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtdq2ps	{rz-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtdq2ps	(%ecx), %zmm6	 # AVX512F
	vcvtdq2ps	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vcvtdq2ps	(%eax){1to16}, %zmm6	 # AVX512F
	vcvtdq2ps	8128(%edx), %zmm6	 # AVX512F Disp8
	vcvtdq2ps	8192(%edx), %zmm6	 # AVX512F
	vcvtdq2ps	-8192(%edx), %zmm6	 # AVX512F Disp8
	vcvtdq2ps	-8256(%edx), %zmm6	 # AVX512F
	vcvtdq2ps	508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vcvtdq2ps	512(%edx){1to16}, %zmm6	 # AVX512F
	vcvtdq2ps	-512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vcvtdq2ps	-516(%edx){1to16}, %zmm6	 # AVX512F

	vcvtpd2dq	%zmm5, %ymm6{%k7}	 # AVX512F
	vcvtpd2dq	%zmm5, %ymm6{%k7}{z}	 # AVX512F
	vcvtpd2dq	{rn-sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtpd2dq	{ru-sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtpd2dq	{rd-sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtpd2dq	{rz-sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtpd2dq	(%ecx), %ymm6{%k7}	 # AVX512F
	vcvtpd2dq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512F
	vcvtpd2dq	(%eax){1to8}, %ymm6{%k7}	 # AVX512F
	vcvtpd2dq	8128(%edx), %ymm6{%k7}	 # AVX512F Disp8
	vcvtpd2dq	8192(%edx), %ymm6{%k7}	 # AVX512F
	vcvtpd2dq	-8192(%edx), %ymm6{%k7}	 # AVX512F Disp8
	vcvtpd2dq	-8256(%edx), %ymm6{%k7}	 # AVX512F
	vcvtpd2dq	1016(%edx){1to8}, %ymm6{%k7}	 # AVX512F Disp8
	vcvtpd2dq	1024(%edx){1to8}, %ymm6{%k7}	 # AVX512F
	vcvtpd2dq	-1024(%edx){1to8}, %ymm6{%k7}	 # AVX512F Disp8
	vcvtpd2dq	-1032(%edx){1to8}, %ymm6{%k7}	 # AVX512F

	vcvtpd2ps	%zmm5, %ymm6{%k7}	 # AVX512F
	vcvtpd2ps	%zmm5, %ymm6{%k7}{z}	 # AVX512F
	vcvtpd2ps	{rn-sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtpd2ps	{ru-sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtpd2ps	{rd-sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtpd2ps	{rz-sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtpd2ps	(%ecx), %ymm6{%k7}	 # AVX512F
	vcvtpd2ps	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512F
	vcvtpd2ps	(%eax){1to8}, %ymm6{%k7}	 # AVX512F
	vcvtpd2ps	8128(%edx), %ymm6{%k7}	 # AVX512F Disp8
	vcvtpd2ps	8192(%edx), %ymm6{%k7}	 # AVX512F
	vcvtpd2ps	-8192(%edx), %ymm6{%k7}	 # AVX512F Disp8
	vcvtpd2ps	-8256(%edx), %ymm6{%k7}	 # AVX512F
	vcvtpd2ps	1016(%edx){1to8}, %ymm6{%k7}	 # AVX512F Disp8
	vcvtpd2ps	1024(%edx){1to8}, %ymm6{%k7}	 # AVX512F
	vcvtpd2ps	-1024(%edx){1to8}, %ymm6{%k7}	 # AVX512F Disp8
	vcvtpd2ps	-1032(%edx){1to8}, %ymm6{%k7}	 # AVX512F

	vcvtpd2udq	%zmm5, %ymm6{%k7}	 # AVX512F
	vcvtpd2udq	%zmm5, %ymm6{%k7}{z}	 # AVX512F
	vcvtpd2udq	{rn-sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtpd2udq	{ru-sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtpd2udq	{rd-sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtpd2udq	{rz-sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtpd2udq	(%ecx), %ymm6{%k7}	 # AVX512F
	vcvtpd2udq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512F
	vcvtpd2udq	(%eax){1to8}, %ymm6{%k7}	 # AVX512F
	vcvtpd2udq	8128(%edx), %ymm6{%k7}	 # AVX512F Disp8
	vcvtpd2udq	8192(%edx), %ymm6{%k7}	 # AVX512F
	vcvtpd2udq	-8192(%edx), %ymm6{%k7}	 # AVX512F Disp8
	vcvtpd2udq	-8256(%edx), %ymm6{%k7}	 # AVX512F
	vcvtpd2udq	1016(%edx){1to8}, %ymm6{%k7}	 # AVX512F Disp8
	vcvtpd2udq	1024(%edx){1to8}, %ymm6{%k7}	 # AVX512F
	vcvtpd2udq	-1024(%edx){1to8}, %ymm6{%k7}	 # AVX512F Disp8
	vcvtpd2udq	-1032(%edx){1to8}, %ymm6{%k7}	 # AVX512F

	vcvtph2ps	%ymm5, %zmm6{%k7}	 # AVX512F
	vcvtph2ps	%ymm5, %zmm6{%k7}{z}	 # AVX512F
	vcvtph2ps	{sae}, %ymm5, %zmm6{%k7}	 # AVX512F
	vcvtph2ps	(%ecx), %zmm6{%k7}	 # AVX512F
	vcvtph2ps	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512F
	vcvtph2ps	4064(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vcvtph2ps	4096(%edx), %zmm6{%k7}	 # AVX512F
	vcvtph2ps	-4096(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vcvtph2ps	-4128(%edx), %zmm6{%k7}	 # AVX512F

	vcvtps2dq	%zmm5, %zmm6	 # AVX512F
	vcvtps2dq	%zmm5, %zmm6{%k7}	 # AVX512F
	vcvtps2dq	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vcvtps2dq	{rn-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtps2dq	{ru-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtps2dq	{rd-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtps2dq	{rz-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtps2dq	(%ecx), %zmm6	 # AVX512F
	vcvtps2dq	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vcvtps2dq	(%eax){1to16}, %zmm6	 # AVX512F
	vcvtps2dq	8128(%edx), %zmm6	 # AVX512F Disp8
	vcvtps2dq	8192(%edx), %zmm6	 # AVX512F
	vcvtps2dq	-8192(%edx), %zmm6	 # AVX512F Disp8
	vcvtps2dq	-8256(%edx), %zmm6	 # AVX512F
	vcvtps2dq	508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vcvtps2dq	512(%edx){1to16}, %zmm6	 # AVX512F
	vcvtps2dq	-512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vcvtps2dq	-516(%edx){1to16}, %zmm6	 # AVX512F

	vcvtps2pd	%ymm5, %zmm6{%k7}	 # AVX512F
	vcvtps2pd	%ymm5, %zmm6{%k7}{z}	 # AVX512F
	vcvtps2pd	{sae}, %ymm5, %zmm6{%k7}	 # AVX512F
	vcvtps2pd	(%ecx), %zmm6{%k7}	 # AVX512F
	vcvtps2pd	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512F
	vcvtps2pd	(%eax){1to8}, %zmm6{%k7}	 # AVX512F
	vcvtps2pd	4064(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vcvtps2pd	4096(%edx), %zmm6{%k7}	 # AVX512F
	vcvtps2pd	-4096(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vcvtps2pd	-4128(%edx), %zmm6{%k7}	 # AVX512F
	vcvtps2pd	508(%edx){1to8}, %zmm6{%k7}	 # AVX512F Disp8
	vcvtps2pd	512(%edx){1to8}, %zmm6{%k7}	 # AVX512F
	vcvtps2pd	-512(%edx){1to8}, %zmm6{%k7}	 # AVX512F Disp8
	vcvtps2pd	-516(%edx){1to8}, %zmm6{%k7}	 # AVX512F

	vcvtps2ph	$0xab, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtps2ph	$0xab, %zmm5, %ymm6{%k7}{z}	 # AVX512F
	vcvtps2ph	$0xab, {sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtps2ph	$123, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvtps2ph	$123, {sae}, %zmm5, %ymm6{%k7}	 # AVX512F

	vcvtps2udq	%zmm5, %zmm6	 # AVX512F
	vcvtps2udq	%zmm5, %zmm6{%k7}	 # AVX512F
	vcvtps2udq	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vcvtps2udq	{rn-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtps2udq	{ru-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtps2udq	{rd-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtps2udq	{rz-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtps2udq	(%ecx), %zmm6	 # AVX512F
	vcvtps2udq	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vcvtps2udq	(%eax){1to16}, %zmm6	 # AVX512F
	vcvtps2udq	8128(%edx), %zmm6	 # AVX512F Disp8
	vcvtps2udq	8192(%edx), %zmm6	 # AVX512F
	vcvtps2udq	-8192(%edx), %zmm6	 # AVX512F Disp8
	vcvtps2udq	-8256(%edx), %zmm6	 # AVX512F
	vcvtps2udq	508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vcvtps2udq	512(%edx){1to16}, %zmm6	 # AVX512F
	vcvtps2udq	-512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vcvtps2udq	-516(%edx){1to16}, %zmm6	 # AVX512F

	vcvtsd2si	{rn-sae}, %xmm6, %eax	 # AVX512F
	vcvtsd2si	{ru-sae}, %xmm6, %eax	 # AVX512F
	vcvtsd2si	{rd-sae}, %xmm6, %eax	 # AVX512F
	vcvtsd2si	{rz-sae}, %xmm6, %eax	 # AVX512F
	vcvtsd2si	{rn-sae}, %xmm6, %ebp	 # AVX512F
	vcvtsd2si	{ru-sae}, %xmm6, %ebp	 # AVX512F
	vcvtsd2si	{rd-sae}, %xmm6, %ebp	 # AVX512F
	vcvtsd2si	{rz-sae}, %xmm6, %ebp	 # AVX512F

	vcvtsd2ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vcvtsd2ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vcvtsd2ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vcvtsd2ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vcvtsd2ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vcvtsd2ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vcvtsd2ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vcvtsd2ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vcvtsd2ss	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vcvtsd2ss	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vcvtsd2ss	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vcvtsd2ss	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F


	vcvtsi2ssl	%eax, {rn-sae}, %xmm5, %xmm6	 # AVX512F
	vcvtsi2ssl	%eax, {ru-sae}, %xmm5, %xmm6	 # AVX512F
	vcvtsi2ssl	%eax, {rd-sae}, %xmm5, %xmm6	 # AVX512F
	vcvtsi2ssl	%eax, {rz-sae}, %xmm5, %xmm6	 # AVX512F
	vcvtsi2ssl	%ebp, {rn-sae}, %xmm5, %xmm6	 # AVX512F
	vcvtsi2ssl	%ebp, {ru-sae}, %xmm5, %xmm6	 # AVX512F
	vcvtsi2ssl	%ebp, {rd-sae}, %xmm5, %xmm6	 # AVX512F
	vcvtsi2ssl	%ebp, {rz-sae}, %xmm5, %xmm6	 # AVX512F

	vcvtss2sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vcvtss2sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vcvtss2sd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vcvtss2sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vcvtss2sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vcvtss2sd	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vcvtss2sd	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vcvtss2sd	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vcvtss2sd	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vcvtss2si	{rn-sae}, %xmm6, %eax	 # AVX512F
	vcvtss2si	{ru-sae}, %xmm6, %eax	 # AVX512F
	vcvtss2si	{rd-sae}, %xmm6, %eax	 # AVX512F
	vcvtss2si	{rz-sae}, %xmm6, %eax	 # AVX512F
	vcvtss2si	{rn-sae}, %xmm6, %ebp	 # AVX512F
	vcvtss2si	{ru-sae}, %xmm6, %ebp	 # AVX512F
	vcvtss2si	{rd-sae}, %xmm6, %ebp	 # AVX512F
	vcvtss2si	{rz-sae}, %xmm6, %ebp	 # AVX512F

	vcvttpd2dq	%zmm5, %ymm6{%k7}	 # AVX512F
	vcvttpd2dq	%zmm5, %ymm6{%k7}{z}	 # AVX512F
	vcvttpd2dq	{sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvttpd2dq	(%ecx), %ymm6{%k7}	 # AVX512F
	vcvttpd2dq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512F
	vcvttpd2dq	(%eax){1to8}, %ymm6{%k7}	 # AVX512F
	vcvttpd2dq	8128(%edx), %ymm6{%k7}	 # AVX512F Disp8
	vcvttpd2dq	8192(%edx), %ymm6{%k7}	 # AVX512F
	vcvttpd2dq	-8192(%edx), %ymm6{%k7}	 # AVX512F Disp8
	vcvttpd2dq	-8256(%edx), %ymm6{%k7}	 # AVX512F
	vcvttpd2dq	1016(%edx){1to8}, %ymm6{%k7}	 # AVX512F Disp8
	vcvttpd2dq	1024(%edx){1to8}, %ymm6{%k7}	 # AVX512F
	vcvttpd2dq	-1024(%edx){1to8}, %ymm6{%k7}	 # AVX512F Disp8
	vcvttpd2dq	-1032(%edx){1to8}, %ymm6{%k7}	 # AVX512F

	vcvttps2dq	%zmm5, %zmm6	 # AVX512F
	vcvttps2dq	%zmm5, %zmm6{%k7}	 # AVX512F
	vcvttps2dq	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vcvttps2dq	{sae}, %zmm5, %zmm6	 # AVX512F
	vcvttps2dq	(%ecx), %zmm6	 # AVX512F
	vcvttps2dq	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vcvttps2dq	(%eax){1to16}, %zmm6	 # AVX512F
	vcvttps2dq	8128(%edx), %zmm6	 # AVX512F Disp8
	vcvttps2dq	8192(%edx), %zmm6	 # AVX512F
	vcvttps2dq	-8192(%edx), %zmm6	 # AVX512F Disp8
	vcvttps2dq	-8256(%edx), %zmm6	 # AVX512F
	vcvttps2dq	508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vcvttps2dq	512(%edx){1to16}, %zmm6	 # AVX512F
	vcvttps2dq	-512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vcvttps2dq	-516(%edx){1to16}, %zmm6	 # AVX512F

	vcvttsd2si	{sae}, %xmm6, %eax	 # AVX512F
	vcvttsd2si	{sae}, %xmm6, %ebp	 # AVX512F

	vcvttss2si	{sae}, %xmm6, %eax	 # AVX512F
	vcvttss2si	{sae}, %xmm6, %ebp	 # AVX512F

	vcvtudq2pd	%ymm5, %zmm6{%k7}	 # AVX512F
	vcvtudq2pd	%ymm5, %zmm6{%k7}{z}	 # AVX512F
	vcvtudq2pd	(%ecx), %zmm6{%k7}	 # AVX512F
	vcvtudq2pd	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512F
	vcvtudq2pd	(%eax){1to8}, %zmm6{%k7}	 # AVX512F
	vcvtudq2pd	4064(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vcvtudq2pd	4096(%edx), %zmm6{%k7}	 # AVX512F
	vcvtudq2pd	-4096(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vcvtudq2pd	-4128(%edx), %zmm6{%k7}	 # AVX512F
	vcvtudq2pd	508(%edx){1to8}, %zmm6{%k7}	 # AVX512F Disp8
	vcvtudq2pd	512(%edx){1to8}, %zmm6{%k7}	 # AVX512F
	vcvtudq2pd	-512(%edx){1to8}, %zmm6{%k7}	 # AVX512F Disp8
	vcvtudq2pd	-516(%edx){1to8}, %zmm6{%k7}	 # AVX512F

	vcvtudq2ps	%zmm5, %zmm6	 # AVX512F
	vcvtudq2ps	%zmm5, %zmm6{%k7}	 # AVX512F
	vcvtudq2ps	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vcvtudq2ps	{rn-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtudq2ps	{ru-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtudq2ps	{rd-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtudq2ps	{rz-sae}, %zmm5, %zmm6	 # AVX512F
	vcvtudq2ps	(%ecx), %zmm6	 # AVX512F
	vcvtudq2ps	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vcvtudq2ps	(%eax){1to16}, %zmm6	 # AVX512F
	vcvtudq2ps	8128(%edx), %zmm6	 # AVX512F Disp8
	vcvtudq2ps	8192(%edx), %zmm6	 # AVX512F
	vcvtudq2ps	-8192(%edx), %zmm6	 # AVX512F Disp8
	vcvtudq2ps	-8256(%edx), %zmm6	 # AVX512F
	vcvtudq2ps	508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vcvtudq2ps	512(%edx){1to16}, %zmm6	 # AVX512F
	vcvtudq2ps	-512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vcvtudq2ps	-516(%edx){1to16}, %zmm6	 # AVX512F

	vdivpd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vdivpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vdivpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vdivpd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vdivpd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vdivpd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vdivpd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vdivpd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vdivpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vdivpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vdivpd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vdivpd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vdivpd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vdivpd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vdivpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vdivpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vdivpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vdivpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vdivps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vdivps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vdivps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vdivps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vdivps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vdivps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vdivps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vdivps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vdivps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vdivps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vdivps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vdivps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vdivps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vdivps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vdivps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vdivps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vdivps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vdivps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vdivsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vdivsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vdivsd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vdivsd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vdivsd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vdivsd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vdivsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vdivsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vdivsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vdivsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vdivsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vdivsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vdivss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vdivss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vdivss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vdivss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vdivss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vdivss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vdivss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vdivss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vdivss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vdivss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vdivss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vdivss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vexpandpd	(%ecx), %zmm6	 # AVX512F
	vexpandpd	(%ecx), %zmm6{%k7}	 # AVX512F
	vexpandpd	(%ecx), %zmm6{%k7}{z}	 # AVX512F
	vexpandpd	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vexpandpd	1016(%edx), %zmm6	 # AVX512F Disp8
	vexpandpd	1024(%edx), %zmm6	 # AVX512F
	vexpandpd	-1024(%edx), %zmm6	 # AVX512F Disp8
	vexpandpd	-1032(%edx), %zmm6	 # AVX512F

	vexpandpd	%zmm5, %zmm6	 # AVX512F
	vexpandpd	%zmm5, %zmm6{%k7}	 # AVX512F
	vexpandpd	%zmm5, %zmm6{%k7}{z}	 # AVX512F

	vexpandps	(%ecx), %zmm6	 # AVX512F
	vexpandps	(%ecx), %zmm6{%k7}	 # AVX512F
	vexpandps	(%ecx), %zmm6{%k7}{z}	 # AVX512F
	vexpandps	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vexpandps	508(%edx), %zmm6	 # AVX512F Disp8
	vexpandps	512(%edx), %zmm6	 # AVX512F
	vexpandps	-512(%edx), %zmm6	 # AVX512F Disp8
	vexpandps	-516(%edx), %zmm6	 # AVX512F

	vexpandps	%zmm5, %zmm6	 # AVX512F
	vexpandps	%zmm5, %zmm6{%k7}	 # AVX512F
	vexpandps	%zmm5, %zmm6{%k7}{z}	 # AVX512F

	vextractf32x4	$0xab, %zmm5, %xmm6{%k7}	 # AVX512F
	vextractf32x4	$0xab, %zmm5, %xmm6{%k7}{z}	 # AVX512F
	vextractf32x4	$123, %zmm5, %xmm6{%k7}	 # AVX512F

	vextractf64x4	$0xab, %zmm5, %ymm6{%k7}	 # AVX512F
	vextractf64x4	$0xab, %zmm5, %ymm6{%k7}{z}	 # AVX512F
	vextractf64x4	$123, %zmm5, %ymm6{%k7}	 # AVX512F

	vextracti32x4	$0xab, %zmm5, %xmm6{%k7}	 # AVX512F
	vextracti32x4	$0xab, %zmm5, %xmm6{%k7}{z}	 # AVX512F
	vextracti32x4	$123, %zmm5, %xmm6{%k7}	 # AVX512F

	vextracti64x4	$0xab, %zmm5, %ymm6{%k7}	 # AVX512F
	vextracti64x4	$0xab, %zmm5, %ymm6{%k7}{z}	 # AVX512F
	vextracti64x4	$123, %zmm5, %ymm6{%k7}	 # AVX512F

	vfmadd132pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd132pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmadd132pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmadd132pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd132pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd132pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd132pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd132pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmadd132pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmadd132pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmadd132pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd132pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmadd132pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd132pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmadd132pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd132pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmadd132pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd132pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfmadd132ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd132ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmadd132ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmadd132ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd132ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd132ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd132ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd132ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmadd132ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmadd132ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmadd132ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd132ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmadd132ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd132ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmadd132ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd132ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmadd132ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd132ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfmadd132sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfmadd132sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmadd132sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmadd132sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfmadd132ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfmadd132ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmadd132ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd132ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmadd132ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfmadd213pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd213pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmadd213pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmadd213pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd213pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd213pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd213pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd213pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmadd213pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmadd213pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmadd213pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd213pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmadd213pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd213pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmadd213pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd213pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmadd213pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd213pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfmadd213ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd213ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmadd213ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmadd213ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd213ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd213ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd213ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd213ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmadd213ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmadd213ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmadd213ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd213ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmadd213ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd213ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmadd213ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd213ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmadd213ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd213ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfmadd213sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfmadd213sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmadd213sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmadd213sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfmadd213ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfmadd213ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmadd213ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd213ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmadd213ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfmadd231pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd231pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmadd231pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmadd231pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd231pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd231pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd231pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd231pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmadd231pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmadd231pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmadd231pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd231pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmadd231pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd231pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmadd231pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd231pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmadd231pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd231pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfmadd231ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd231ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmadd231ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmadd231ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd231ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd231ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd231ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmadd231ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmadd231ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmadd231ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmadd231ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd231ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmadd231ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd231ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmadd231ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd231ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmadd231ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmadd231ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfmadd231sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfmadd231sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmadd231sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmadd231sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfmadd231ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfmadd231ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmadd231ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmadd231ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmadd231ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfmaddsub132pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub132pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmaddsub132pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmaddsub132pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub132pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub132pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub132pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub132pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub132pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmaddsub132pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmaddsub132pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub132pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub132pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub132pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub132pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub132pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmaddsub132pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub132pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfmaddsub132ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub132ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmaddsub132ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmaddsub132ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub132ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub132ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub132ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub132ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub132ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmaddsub132ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmaddsub132ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub132ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub132ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub132ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub132ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub132ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmaddsub132ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub132ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfmaddsub213pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub213pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmaddsub213pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmaddsub213pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub213pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub213pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub213pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub213pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub213pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmaddsub213pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmaddsub213pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub213pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub213pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub213pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub213pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub213pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmaddsub213pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub213pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfmaddsub213ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub213ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmaddsub213ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmaddsub213ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub213ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub213ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub213ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub213ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub213ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmaddsub213ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmaddsub213ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub213ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub213ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub213ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub213ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub213ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmaddsub213ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub213ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfmaddsub231pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub231pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmaddsub231pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmaddsub231pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub231pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub231pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub231pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub231pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub231pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmaddsub231pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmaddsub231pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub231pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub231pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub231pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub231pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub231pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmaddsub231pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub231pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfmaddsub231ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub231ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmaddsub231ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmaddsub231ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub231ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub231ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub231ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmaddsub231ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub231ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmaddsub231ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmaddsub231ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub231ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub231ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub231ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmaddsub231ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub231ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmaddsub231ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmaddsub231ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfmsub132pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub132pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmsub132pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmsub132pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub132pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub132pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub132pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub132pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmsub132pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmsub132pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmsub132pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub132pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsub132pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub132pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsub132pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub132pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmsub132pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub132pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfmsub132ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub132ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmsub132ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmsub132ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub132ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub132ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub132ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub132ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmsub132ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmsub132ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmsub132ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub132ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsub132ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub132ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsub132ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub132ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmsub132ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub132ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfmsub132sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfmsub132sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmsub132sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmsub132sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfmsub132ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfmsub132ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmsub132ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub132ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmsub132ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfmsub213pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub213pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmsub213pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmsub213pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub213pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub213pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub213pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub213pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmsub213pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmsub213pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmsub213pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub213pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsub213pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub213pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsub213pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub213pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmsub213pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub213pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfmsub213ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub213ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmsub213ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmsub213ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub213ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub213ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub213ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub213ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmsub213ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmsub213ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmsub213ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub213ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsub213ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub213ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsub213ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub213ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmsub213ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub213ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfmsub213sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfmsub213sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmsub213sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmsub213sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfmsub213ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfmsub213ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmsub213ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub213ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmsub213ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfmsub231pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub231pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmsub231pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmsub231pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub231pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub231pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub231pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub231pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmsub231pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmsub231pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmsub231pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub231pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsub231pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub231pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsub231pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub231pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmsub231pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub231pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfmsub231ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub231ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmsub231ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmsub231ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub231ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub231ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub231ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsub231ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmsub231ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmsub231ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmsub231ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub231ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsub231ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub231ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsub231ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub231ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmsub231ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsub231ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfmsub231sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfmsub231sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmsub231sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmsub231sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfmsub231ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfmsub231ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmsub231ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfmsub231ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfmsub231ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfmsubadd132pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd132pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmsubadd132pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmsubadd132pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd132pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd132pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd132pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd132pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd132pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmsubadd132pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmsubadd132pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd132pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd132pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd132pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd132pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd132pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmsubadd132pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd132pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfmsubadd132ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd132ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmsubadd132ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmsubadd132ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd132ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd132ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd132ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd132ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd132ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmsubadd132ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmsubadd132ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd132ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd132ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd132ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd132ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd132ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmsubadd132ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd132ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfmsubadd213pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd213pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmsubadd213pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmsubadd213pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd213pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd213pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd213pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd213pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd213pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmsubadd213pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmsubadd213pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd213pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd213pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd213pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd213pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd213pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmsubadd213pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd213pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfmsubadd213ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd213ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmsubadd213ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmsubadd213ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd213ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd213ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd213ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd213ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd213ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmsubadd213ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmsubadd213ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd213ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd213ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd213ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd213ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd213ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmsubadd213ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd213ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfmsubadd231pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd231pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmsubadd231pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmsubadd231pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd231pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd231pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd231pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd231pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd231pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmsubadd231pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmsubadd231pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd231pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd231pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd231pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd231pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd231pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfmsubadd231pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd231pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfmsubadd231ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd231ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfmsubadd231ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfmsubadd231ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd231ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd231ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd231ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfmsubadd231ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd231ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfmsubadd231ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmsubadd231ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd231ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd231ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd231ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfmsubadd231ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd231ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfmsubadd231ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfmsubadd231ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfnmadd132pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd132pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfnmadd132pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfnmadd132pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd132pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd132pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd132pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd132pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfnmadd132pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfnmadd132pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfnmadd132pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd132pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmadd132pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd132pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmadd132pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd132pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfnmadd132pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd132pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfnmadd132ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd132ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfnmadd132ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfnmadd132ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd132ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd132ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd132ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd132ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfnmadd132ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfnmadd132ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfnmadd132ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd132ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmadd132ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd132ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmadd132ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd132ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfnmadd132ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd132ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfnmadd132sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfnmadd132sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmadd132sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmadd132sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfnmadd132ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfnmadd132ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmadd132ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd132ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmadd132ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfnmadd213pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd213pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfnmadd213pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfnmadd213pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd213pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd213pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd213pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd213pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfnmadd213pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfnmadd213pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfnmadd213pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd213pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmadd213pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd213pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmadd213pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd213pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfnmadd213pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd213pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfnmadd213ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd213ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfnmadd213ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfnmadd213ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd213ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd213ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd213ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd213ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfnmadd213ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfnmadd213ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfnmadd213ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd213ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmadd213ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd213ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmadd213ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd213ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfnmadd213ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd213ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfnmadd213sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfnmadd213sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmadd213sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmadd213sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfnmadd213ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfnmadd213ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmadd213ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd213ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmadd213ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfnmadd231pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd231pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfnmadd231pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfnmadd231pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd231pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd231pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd231pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd231pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfnmadd231pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfnmadd231pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfnmadd231pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd231pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmadd231pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd231pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmadd231pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd231pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfnmadd231pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd231pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfnmadd231ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd231ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfnmadd231ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfnmadd231ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd231ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd231ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd231ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmadd231ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfnmadd231ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfnmadd231ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfnmadd231ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd231ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmadd231ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd231ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmadd231ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd231ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfnmadd231ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmadd231ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfnmadd231sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfnmadd231sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmadd231sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmadd231sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfnmadd231ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfnmadd231ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmadd231ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmadd231ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmadd231ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfnmsub132pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub132pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfnmsub132pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfnmsub132pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub132pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub132pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub132pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub132pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfnmsub132pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfnmsub132pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfnmsub132pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub132pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmsub132pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub132pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmsub132pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub132pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfnmsub132pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub132pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfnmsub132ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub132ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfnmsub132ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfnmsub132ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub132ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub132ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub132ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub132ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfnmsub132ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfnmsub132ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfnmsub132ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub132ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmsub132ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub132ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmsub132ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub132ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfnmsub132ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub132ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfnmsub132sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfnmsub132sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmsub132sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmsub132sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfnmsub132ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfnmsub132ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmsub132ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub132ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmsub132ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfnmsub213pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub213pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfnmsub213pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfnmsub213pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub213pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub213pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub213pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub213pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfnmsub213pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfnmsub213pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfnmsub213pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub213pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmsub213pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub213pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmsub213pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub213pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfnmsub213pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub213pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfnmsub213ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub213ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfnmsub213ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfnmsub213ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub213ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub213ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub213ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub213ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfnmsub213ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfnmsub213ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfnmsub213ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub213ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmsub213ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub213ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmsub213ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub213ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfnmsub213ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub213ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfnmsub213sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfnmsub213sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmsub213sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmsub213sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfnmsub213ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfnmsub213ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmsub213ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub213ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmsub213ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfnmsub231pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub231pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfnmsub231pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfnmsub231pd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub231pd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub231pd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub231pd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub231pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfnmsub231pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfnmsub231pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfnmsub231pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub231pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmsub231pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub231pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmsub231pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub231pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfnmsub231pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub231pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfnmsub231ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub231ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfnmsub231ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfnmsub231ps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub231ps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub231ps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub231ps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfnmsub231ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vfnmsub231ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfnmsub231ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfnmsub231ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub231ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmsub231ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub231ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfnmsub231ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub231ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfnmsub231ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfnmsub231ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfnmsub231sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfnmsub231sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmsub231sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmsub231sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfnmsub231ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfnmsub231ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmsub231ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfnmsub231ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfnmsub231ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vgatherdpd	123(%ebp,%ymm7,8), %zmm6{%k1}	 # AVX512F
	vgatherdpd	123(%ebp,%ymm7,8), %zmm6{%k1}	 # AVX512F
	vgatherdpd	256(%eax,%ymm7), %zmm6{%k1}	 # AVX512F
	vgatherdpd	1024(%ecx,%ymm7,4), %zmm6{%k1}	 # AVX512F

	vgatherdps	123(%ebp,%zmm7,8), %zmm6{%k1}	 # AVX512F
	vgatherdps	123(%ebp,%zmm7,8), %zmm6{%k1}	 # AVX512F
	vgatherdps	256(%eax,%zmm7), %zmm6{%k1}	 # AVX512F
	vgatherdps	1024(%ecx,%zmm7,4), %zmm6{%k1}	 # AVX512F

	vgatherqpd	123(%ebp,%zmm7,8), %zmm6{%k1}	 # AVX512F
	vgatherqpd	123(%ebp,%zmm7,8), %zmm6{%k1}	 # AVX512F
	vgatherqpd	256(%eax,%zmm7), %zmm6{%k1}	 # AVX512F
	vgatherqpd	1024(%ecx,%zmm7,4), %zmm6{%k1}	 # AVX512F

	vgatherqps	123(%ebp,%zmm7,8), %ymm6{%k1}	 # AVX512F
	vgatherqps	123(%ebp,%zmm7,8), %ymm6{%k1}	 # AVX512F
	vgatherqps	256(%eax,%zmm7), %ymm6{%k1}	 # AVX512F
	vgatherqps	1024(%ecx,%zmm7,4), %ymm6{%k1}	 # AVX512F

	vgetexppd	%zmm5, %zmm6	 # AVX512F
	vgetexppd	%zmm5, %zmm6{%k7}	 # AVX512F
	vgetexppd	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vgetexppd	{sae}, %zmm5, %zmm6	 # AVX512F
	vgetexppd	(%ecx), %zmm6	 # AVX512F
	vgetexppd	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vgetexppd	(%eax){1to8}, %zmm6	 # AVX512F
	vgetexppd	8128(%edx), %zmm6	 # AVX512F Disp8
	vgetexppd	8192(%edx), %zmm6	 # AVX512F
	vgetexppd	-8192(%edx), %zmm6	 # AVX512F Disp8
	vgetexppd	-8256(%edx), %zmm6	 # AVX512F
	vgetexppd	1016(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vgetexppd	1024(%edx){1to8}, %zmm6	 # AVX512F
	vgetexppd	-1024(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vgetexppd	-1032(%edx){1to8}, %zmm6	 # AVX512F

	vgetexpps	%zmm5, %zmm6	 # AVX512F
	vgetexpps	%zmm5, %zmm6{%k7}	 # AVX512F
	vgetexpps	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vgetexpps	{sae}, %zmm5, %zmm6	 # AVX512F
	vgetexpps	(%ecx), %zmm6	 # AVX512F
	vgetexpps	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vgetexpps	(%eax){1to16}, %zmm6	 # AVX512F
	vgetexpps	8128(%edx), %zmm6	 # AVX512F Disp8
	vgetexpps	8192(%edx), %zmm6	 # AVX512F
	vgetexpps	-8192(%edx), %zmm6	 # AVX512F Disp8
	vgetexpps	-8256(%edx), %zmm6	 # AVX512F
	vgetexpps	508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vgetexpps	512(%edx){1to16}, %zmm6	 # AVX512F
	vgetexpps	-512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vgetexpps	-516(%edx){1to16}, %zmm6	 # AVX512F

	vgetexpsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetexpsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vgetexpsd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetexpsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vgetexpsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vgetexpsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vgetexpsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vgetexpsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vgetexpsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vgetexpss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetexpss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vgetexpss	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetexpss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vgetexpss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vgetexpss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vgetexpss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vgetexpss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vgetexpss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vgetmantpd	$0xab, %zmm5, %zmm6	 # AVX512F
	vgetmantpd	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vgetmantpd	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vgetmantpd	$0xab, {sae}, %zmm5, %zmm6	 # AVX512F
	vgetmantpd	$123, %zmm5, %zmm6	 # AVX512F
	vgetmantpd	$123, {sae}, %zmm5, %zmm6	 # AVX512F
	vgetmantpd	$123, (%ecx), %zmm6	 # AVX512F
	vgetmantpd	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vgetmantpd	$123, (%eax){1to8}, %zmm6	 # AVX512F
	vgetmantpd	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vgetmantpd	$123, 8192(%edx), %zmm6	 # AVX512F
	vgetmantpd	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vgetmantpd	$123, -8256(%edx), %zmm6	 # AVX512F
	vgetmantpd	$123, 1016(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vgetmantpd	$123, 1024(%edx){1to8}, %zmm6	 # AVX512F
	vgetmantpd	$123, -1024(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vgetmantpd	$123, -1032(%edx){1to8}, %zmm6	 # AVX512F

	vgetmantps	$0xab, %zmm5, %zmm6	 # AVX512F
	vgetmantps	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vgetmantps	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vgetmantps	$0xab, {sae}, %zmm5, %zmm6	 # AVX512F
	vgetmantps	$123, %zmm5, %zmm6	 # AVX512F
	vgetmantps	$123, {sae}, %zmm5, %zmm6	 # AVX512F
	vgetmantps	$123, (%ecx), %zmm6	 # AVX512F
	vgetmantps	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vgetmantps	$123, (%eax){1to16}, %zmm6	 # AVX512F
	vgetmantps	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vgetmantps	$123, 8192(%edx), %zmm6	 # AVX512F
	vgetmantps	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vgetmantps	$123, -8256(%edx), %zmm6	 # AVX512F
	vgetmantps	$123, 508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vgetmantps	$123, 512(%edx){1to16}, %zmm6	 # AVX512F
	vgetmantps	$123, -512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vgetmantps	$123, -516(%edx){1to16}, %zmm6	 # AVX512F

	vgetmantsd	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantsd	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vgetmantsd	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantsd	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantsd	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantsd	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantsd	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantsd	$123, 1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vgetmantsd	$123, 1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantsd	$123, -1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vgetmantsd	$123, -1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vgetmantss	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantss	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vgetmantss	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantss	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantss	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantss	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantss	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantss	$123, 508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vgetmantss	$123, 512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vgetmantss	$123, -512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vgetmantss	$123, -516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vinsertf32x4	$0xab, %xmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vinsertf32x4	$0xab, %xmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vinsertf32x4	$123, %xmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vinsertf32x4	$123, (%ecx), %zmm5, %zmm6{%k7}	 # AVX512F
	vinsertf32x4	$123, -123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512F
	vinsertf32x4	$123, 2032(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vinsertf32x4	$123, 2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F
	vinsertf32x4	$123, -2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vinsertf32x4	$123, -2064(%edx), %zmm5, %zmm6{%k7}	 # AVX512F

	vinsertf64x4	$0xab, %ymm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vinsertf64x4	$0xab, %ymm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vinsertf64x4	$123, %ymm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vinsertf64x4	$123, (%ecx), %zmm5, %zmm6{%k7}	 # AVX512F
	vinsertf64x4	$123, -123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512F
	vinsertf64x4	$123, 4064(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vinsertf64x4	$123, 4096(%edx), %zmm5, %zmm6{%k7}	 # AVX512F
	vinsertf64x4	$123, -4096(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vinsertf64x4	$123, -4128(%edx), %zmm5, %zmm6{%k7}	 # AVX512F

	vinserti32x4	$0xab, %xmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vinserti32x4	$0xab, %xmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vinserti32x4	$123, %xmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vinserti32x4	$123, (%ecx), %zmm5, %zmm6{%k7}	 # AVX512F
	vinserti32x4	$123, -123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512F
	vinserti32x4	$123, 2032(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vinserti32x4	$123, 2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F
	vinserti32x4	$123, -2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vinserti32x4	$123, -2064(%edx), %zmm5, %zmm6{%k7}	 # AVX512F

	vinserti64x4	$0xab, %ymm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vinserti64x4	$0xab, %ymm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vinserti64x4	$123, %ymm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vinserti64x4	$123, (%ecx), %zmm5, %zmm6{%k7}	 # AVX512F
	vinserti64x4	$123, -123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512F
	vinserti64x4	$123, 4064(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vinserti64x4	$123, 4096(%edx), %zmm5, %zmm6{%k7}	 # AVX512F
	vinserti64x4	$123, -4096(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vinserti64x4	$123, -4128(%edx), %zmm5, %zmm6{%k7}	 # AVX512F


	vmaxpd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vmaxpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vmaxpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmaxpd	{sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vmaxpd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vmaxpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vmaxpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vmaxpd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vmaxpd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vmaxpd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vmaxpd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vmaxpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vmaxpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vmaxpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vmaxpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vmaxps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vmaxps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vmaxps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmaxps	{sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vmaxps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vmaxps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vmaxps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vmaxps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vmaxps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vmaxps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vmaxps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vmaxps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vmaxps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vmaxps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vmaxps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vmaxsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmaxsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vmaxsd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmaxsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vmaxsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vmaxsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vmaxsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vmaxsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vmaxsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vmaxss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmaxss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vmaxss	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmaxss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vmaxss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vmaxss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vmaxss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vmaxss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vmaxss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vminpd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vminpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vminpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vminpd	{sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vminpd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vminpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vminpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vminpd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vminpd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vminpd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vminpd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vminpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vminpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vminpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vminpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vminps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vminps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vminps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vminps	{sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vminps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vminps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vminps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vminps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vminps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vminps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vminps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vminps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vminps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vminps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vminps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vminsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vminsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vminsd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vminsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vminsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vminsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vminsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vminsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vminsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vminss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vminss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vminss	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vminss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vminss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vminss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vminss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vminss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vminss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vmovapd	%zmm5, %zmm6	 # AVX512F
	vmovapd	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovapd	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovapd	(%ecx), %zmm6	 # AVX512F
	vmovapd	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vmovapd	8128(%edx), %zmm6	 # AVX512F Disp8
	vmovapd	8192(%edx), %zmm6	 # AVX512F
	vmovapd	-8192(%edx), %zmm6	 # AVX512F Disp8
	vmovapd	-8256(%edx), %zmm6	 # AVX512F

	vmovaps	%zmm5, %zmm6	 # AVX512F
	vmovaps	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovaps	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovaps	(%ecx), %zmm6	 # AVX512F
	vmovaps	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vmovaps	8128(%edx), %zmm6	 # AVX512F Disp8
	vmovaps	8192(%edx), %zmm6	 # AVX512F
	vmovaps	-8192(%edx), %zmm6	 # AVX512F Disp8
	vmovaps	-8256(%edx), %zmm6	 # AVX512F



	vmovddup	%zmm5, %zmm6	 # AVX512F
	vmovddup	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovddup	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovddup	(%ecx), %zmm6	 # AVX512F
	vmovddup	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vmovddup	8128(%edx), %zmm6	 # AVX512F Disp8
	vmovddup	8192(%edx), %zmm6	 # AVX512F
	vmovddup	-8192(%edx), %zmm6	 # AVX512F Disp8
	vmovddup	-8256(%edx), %zmm6	 # AVX512F

	vmovdqa32	%zmm5, %zmm6	 # AVX512F
	vmovdqa32	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovdqa32	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovdqa32	(%ecx), %zmm6	 # AVX512F
	vmovdqa32	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vmovdqa32	8128(%edx), %zmm6	 # AVX512F Disp8
	vmovdqa32	8192(%edx), %zmm6	 # AVX512F
	vmovdqa32	-8192(%edx), %zmm6	 # AVX512F Disp8
	vmovdqa32	-8256(%edx), %zmm6	 # AVX512F

	vmovdqa64	%zmm5, %zmm6	 # AVX512F
	vmovdqa64	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovdqa64	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovdqa64	(%ecx), %zmm6	 # AVX512F
	vmovdqa64	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vmovdqa64	8128(%edx), %zmm6	 # AVX512F Disp8
	vmovdqa64	8192(%edx), %zmm6	 # AVX512F
	vmovdqa64	-8192(%edx), %zmm6	 # AVX512F Disp8
	vmovdqa64	-8256(%edx), %zmm6	 # AVX512F

	vmovdqu32	%zmm5, %zmm6	 # AVX512F
	vmovdqu32	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovdqu32	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovdqu32	(%ecx), %zmm6	 # AVX512F
	vmovdqu32	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vmovdqu32	8128(%edx), %zmm6	 # AVX512F Disp8
	vmovdqu32	8192(%edx), %zmm6	 # AVX512F
	vmovdqu32	-8192(%edx), %zmm6	 # AVX512F Disp8
	vmovdqu32	-8256(%edx), %zmm6	 # AVX512F

	vmovdqu64	%zmm5, %zmm6	 # AVX512F
	vmovdqu64	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovdqu64	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovdqu64	(%ecx), %zmm6	 # AVX512F
	vmovdqu64	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vmovdqu64	8128(%edx), %zmm6	 # AVX512F Disp8
	vmovdqu64	8192(%edx), %zmm6	 # AVX512F
	vmovdqu64	-8192(%edx), %zmm6	 # AVX512F Disp8
	vmovdqu64	-8256(%edx), %zmm6	 # AVX512F











	vmovntdq	%zmm6, (%ecx)	 # AVX512F
	vmovntdq	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vmovntdq	%zmm6, 8128(%edx)	 # AVX512F Disp8
	vmovntdq	%zmm6, 8192(%edx)	 # AVX512F
	vmovntdq	%zmm6, -8192(%edx)	 # AVX512F Disp8
	vmovntdq	%zmm6, -8256(%edx)	 # AVX512F

	vmovntdqa	(%ecx), %zmm6	 # AVX512F
	vmovntdqa	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vmovntdqa	8128(%edx), %zmm6	 # AVX512F Disp8
	vmovntdqa	8192(%edx), %zmm6	 # AVX512F
	vmovntdqa	-8192(%edx), %zmm6	 # AVX512F Disp8
	vmovntdqa	-8256(%edx), %zmm6	 # AVX512F

	vmovntpd	%zmm6, (%ecx)	 # AVX512F
	vmovntpd	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vmovntpd	%zmm6, 8128(%edx)	 # AVX512F Disp8
	vmovntpd	%zmm6, 8192(%edx)	 # AVX512F
	vmovntpd	%zmm6, -8192(%edx)	 # AVX512F Disp8
	vmovntpd	%zmm6, -8256(%edx)	 # AVX512F

	vmovntps	%zmm6, (%ecx)	 # AVX512F
	vmovntps	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vmovntps	%zmm6, 8128(%edx)	 # AVX512F Disp8
	vmovntps	%zmm6, 8192(%edx)	 # AVX512F
	vmovntps	%zmm6, -8192(%edx)	 # AVX512F Disp8
	vmovntps	%zmm6, -8256(%edx)	 # AVX512F

	vmovsd	(%ecx), %xmm6{%k7}	 # AVX512F
	vmovsd	(%ecx), %xmm6{%k7}{z}	 # AVX512F
	vmovsd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512F
	vmovsd	1016(%edx), %xmm6{%k7}	 # AVX512F Disp8
	vmovsd	1024(%edx), %xmm6{%k7}	 # AVX512F
	vmovsd	-1024(%edx), %xmm6{%k7}	 # AVX512F Disp8
	vmovsd	-1032(%edx), %xmm6{%k7}	 # AVX512F

	vmovsd	%xmm6, (%ecx){%k7}	 # AVX512F
	vmovsd	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512F
	vmovsd	%xmm6, 1016(%edx){%k7}	 # AVX512F Disp8
	vmovsd	%xmm6, 1024(%edx){%k7}	 # AVX512F
	vmovsd	%xmm6, -1024(%edx){%k7}	 # AVX512F Disp8
	vmovsd	%xmm6, -1032(%edx){%k7}	 # AVX512F

	vmovsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmovsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F

	vmovshdup	%zmm5, %zmm6	 # AVX512F
	vmovshdup	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovshdup	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovshdup	(%ecx), %zmm6	 # AVX512F
	vmovshdup	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vmovshdup	8128(%edx), %zmm6	 # AVX512F Disp8
	vmovshdup	8192(%edx), %zmm6	 # AVX512F
	vmovshdup	-8192(%edx), %zmm6	 # AVX512F Disp8
	vmovshdup	-8256(%edx), %zmm6	 # AVX512F

	vmovsldup	%zmm5, %zmm6	 # AVX512F
	vmovsldup	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovsldup	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovsldup	(%ecx), %zmm6	 # AVX512F
	vmovsldup	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vmovsldup	8128(%edx), %zmm6	 # AVX512F Disp8
	vmovsldup	8192(%edx), %zmm6	 # AVX512F
	vmovsldup	-8192(%edx), %zmm6	 # AVX512F Disp8
	vmovsldup	-8256(%edx), %zmm6	 # AVX512F

	vmovss	(%ecx), %xmm6{%k7}	 # AVX512F
	vmovss	(%ecx), %xmm6{%k7}{z}	 # AVX512F
	vmovss	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512F
	vmovss	508(%edx), %xmm6{%k7}	 # AVX512F Disp8
	vmovss	512(%edx), %xmm6{%k7}	 # AVX512F
	vmovss	-512(%edx), %xmm6{%k7}	 # AVX512F Disp8
	vmovss	-516(%edx), %xmm6{%k7}	 # AVX512F

	vmovss	%xmm6, (%ecx){%k7}	 # AVX512F
	vmovss	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512F
	vmovss	%xmm6, 508(%edx){%k7}	 # AVX512F Disp8
	vmovss	%xmm6, 512(%edx){%k7}	 # AVX512F
	vmovss	%xmm6, -512(%edx){%k7}	 # AVX512F Disp8
	vmovss	%xmm6, -516(%edx){%k7}	 # AVX512F

	vmovss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmovss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F

	vmovupd	%zmm5, %zmm6	 # AVX512F
	vmovupd	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovupd	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovupd	(%ecx), %zmm6	 # AVX512F
	vmovupd	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vmovupd	8128(%edx), %zmm6	 # AVX512F Disp8
	vmovupd	8192(%edx), %zmm6	 # AVX512F
	vmovupd	-8192(%edx), %zmm6	 # AVX512F Disp8
	vmovupd	-8256(%edx), %zmm6	 # AVX512F

	vmovups	%zmm5, %zmm6	 # AVX512F
	vmovups	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovups	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovups	(%ecx), %zmm6	 # AVX512F
	vmovups	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vmovups	8128(%edx), %zmm6	 # AVX512F Disp8
	vmovups	8192(%edx), %zmm6	 # AVX512F
	vmovups	-8192(%edx), %zmm6	 # AVX512F Disp8
	vmovups	-8256(%edx), %zmm6	 # AVX512F

	vmulpd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vmulpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vmulpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmulpd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vmulpd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vmulpd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vmulpd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vmulpd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vmulpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vmulpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vmulpd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vmulpd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vmulpd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vmulpd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vmulpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vmulpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vmulpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vmulpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vmulps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vmulps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vmulps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmulps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vmulps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vmulps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vmulps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vmulps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vmulps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vmulps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vmulps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vmulps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vmulps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vmulps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vmulps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vmulps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vmulps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vmulps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vmulsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmulsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vmulsd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmulsd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmulsd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmulsd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmulsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vmulsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vmulsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vmulsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vmulsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vmulsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vmulss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmulss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vmulss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmulss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmulss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmulss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmulss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vmulss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vmulss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vmulss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vmulss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vmulss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vpabsd	%zmm5, %zmm6	 # AVX512F
	vpabsd	%zmm5, %zmm6{%k7}	 # AVX512F
	vpabsd	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpabsd	(%ecx), %zmm6	 # AVX512F
	vpabsd	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpabsd	(%eax){1to16}, %zmm6	 # AVX512F
	vpabsd	8128(%edx), %zmm6	 # AVX512F Disp8
	vpabsd	8192(%edx), %zmm6	 # AVX512F
	vpabsd	-8192(%edx), %zmm6	 # AVX512F Disp8
	vpabsd	-8256(%edx), %zmm6	 # AVX512F
	vpabsd	508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vpabsd	512(%edx){1to16}, %zmm6	 # AVX512F
	vpabsd	-512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vpabsd	-516(%edx){1to16}, %zmm6	 # AVX512F

	vpabsq	%zmm5, %zmm6	 # AVX512F
	vpabsq	%zmm5, %zmm6{%k7}	 # AVX512F
	vpabsq	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpabsq	(%ecx), %zmm6	 # AVX512F
	vpabsq	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpabsq	(%eax){1to8}, %zmm6	 # AVX512F
	vpabsq	8128(%edx), %zmm6	 # AVX512F Disp8
	vpabsq	8192(%edx), %zmm6	 # AVX512F
	vpabsq	-8192(%edx), %zmm6	 # AVX512F Disp8
	vpabsq	-8256(%edx), %zmm6	 # AVX512F
	vpabsq	1016(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vpabsq	1024(%edx){1to8}, %zmm6	 # AVX512F
	vpabsq	-1024(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vpabsq	-1032(%edx){1to8}, %zmm6	 # AVX512F

	vpaddd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpaddd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpaddd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpaddd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpaddd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpaddd	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpaddd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpaddd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpaddd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpaddd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpaddd	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpaddd	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpaddd	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpaddd	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpaddq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpaddq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpaddq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpaddq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpaddq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpaddq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpaddq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpaddq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpaddq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpaddq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpaddq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpaddq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpaddq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpaddq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpandd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpandd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpandd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpandd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpandd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpandd	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpandd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpandd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpandd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpandd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpandd	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpandd	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpandd	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpandd	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpandnd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpandnd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpandnd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpandnd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpandnd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpandnd	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpandnd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpandnd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpandnd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpandnd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpandnd	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpandnd	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpandnd	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpandnd	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpandnq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpandnq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpandnq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpandnq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpandnq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpandnq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpandnq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpandnq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpandnq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpandnq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpandnq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpandnq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpandnq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpandnq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpandq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpandq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpandq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpandq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpandq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpandq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpandq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpandq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpandq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpandq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpandq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpandq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpandq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpandq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpblendmd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpblendmd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpblendmd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpblendmd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpblendmd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpblendmd	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpblendmd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpblendmd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpblendmd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpblendmd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpblendmd	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpblendmd	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpblendmd	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpblendmd	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpbroadcastd	(%ecx), %zmm6	 # AVX512F
	vpbroadcastd	(%ecx), %zmm6{%k7}	 # AVX512F
	vpbroadcastd	(%ecx), %zmm6{%k7}{z}	 # AVX512F
	vpbroadcastd	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpbroadcastd	508(%edx), %zmm6	 # AVX512F Disp8
	vpbroadcastd	512(%edx), %zmm6	 # AVX512F
	vpbroadcastd	-512(%edx), %zmm6	 # AVX512F Disp8
	vpbroadcastd	-516(%edx), %zmm6	 # AVX512F

	vpbroadcastd	%xmm5, %zmm6{%k7}	 # AVX512F
	vpbroadcastd	%xmm5, %zmm6{%k7}{z}	 # AVX512F

	vpbroadcastd	%eax, %zmm6	 # AVX512F
	vpbroadcastd	%eax, %zmm6{%k7}	 # AVX512F
	vpbroadcastd	%eax, %zmm6{%k7}{z}	 # AVX512F
	vpbroadcastd	%ebp, %zmm6	 # AVX512F

	vpbroadcastq	(%ecx), %zmm6	 # AVX512F
	vpbroadcastq	(%ecx), %zmm6{%k7}	 # AVX512F
	vpbroadcastq	(%ecx), %zmm6{%k7}{z}	 # AVX512F
	vpbroadcastq	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpbroadcastq	1016(%edx), %zmm6	 # AVX512F Disp8
	vpbroadcastq	1024(%edx), %zmm6	 # AVX512F
	vpbroadcastq	-1024(%edx), %zmm6	 # AVX512F Disp8
	vpbroadcastq	-1032(%edx), %zmm6	 # AVX512F

	vpbroadcastq	%xmm5, %zmm6{%k7}	 # AVX512F
	vpbroadcastq	%xmm5, %zmm6{%k7}{z}	 # AVX512F

	vpcmpd	$0xab, %zmm5, %zmm6, %k5	 # AVX512F
	vpcmpd	$0xab, %zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpd	$123, %zmm5, %zmm6, %k5	 # AVX512F
	vpcmpd	$123, (%ecx), %zmm6, %k5	 # AVX512F
	vpcmpd	$123, -123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpd	$123, (%eax){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpd	$123, 8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpd	$123, 8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpd	$123, -8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpd	$123, -8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpd	$123, 508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpd	$123, 512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpd	$123, -512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpd	$123, -516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vpcmpltd	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpltd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpltd	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpltd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpltd	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpltd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpltd	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpltd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpltd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpltd	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpltd	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpltd	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpltd	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vpcmpled	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpled	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpled	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpled	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpled	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpled	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpled	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpled	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpled	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpled	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpled	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpled	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpled	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vpcmpneqd	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpneqd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpneqd	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpneqd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpneqd	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpneqd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpneqd	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpneqd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpneqd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpneqd	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpneqd	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpneqd	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpneqd	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vpcmpnltd	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpnltd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpnltd	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpnltd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpnltd	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpnltd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltd	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnltd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnltd	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltd	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpnltd	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltd	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vpcmpnled	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpnled	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpnled	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpnled	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpnled	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpnled	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnled	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnled	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnled	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnled	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnled	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpnled	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnled	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vpcmpeqd	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpeqd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpeqd	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpeqd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpeqd	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpeqd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpeqd	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpeqd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpeqd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpeqd	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpeqd	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpeqd	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpeqd	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vpcmpeqq	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpeqq	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpeqq	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpeqq	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpeqq	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpeqq	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpeqq	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpeqq	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpeqq	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpeqq	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpeqq	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpeqq	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpeqq	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpcmpgtd	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpgtd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpgtd	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpgtd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpgtd	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpgtd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpgtd	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpgtd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpgtd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpgtd	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpgtd	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpgtd	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpgtd	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vpcmpgtq	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpgtq	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpgtq	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpgtq	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpgtq	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpgtq	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpgtq	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpgtq	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpgtq	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpgtq	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpgtq	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpgtq	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpgtq	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpcmpq	$0xab, %zmm5, %zmm6, %k5	 # AVX512F
	vpcmpq	$0xab, %zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpq	$123, %zmm5, %zmm6, %k5	 # AVX512F
	vpcmpq	$123, (%ecx), %zmm6, %k5	 # AVX512F
	vpcmpq	$123, -123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpq	$123, (%eax){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpq	$123, 8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpq	$123, 8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpq	$123, -8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpq	$123, -8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpq	$123, 1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpq	$123, 1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpq	$123, -1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpq	$123, -1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpcmpltq	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpltq	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpltq	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpltq	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpltq	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpltq	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpltq	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpltq	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpltq	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpltq	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpltq	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpltq	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpltq	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpcmpleq	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpleq	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpleq	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpleq	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpleq	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpleq	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpleq	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpleq	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpleq	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpleq	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpleq	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpleq	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpleq	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpcmpneqq	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpneqq	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpneqq	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpneqq	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpneqq	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpneqq	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpneqq	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpneqq	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpneqq	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpneqq	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpneqq	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpneqq	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpneqq	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpcmpnltq	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpnltq	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpnltq	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpnltq	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpnltq	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpnltq	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltq	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnltq	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltq	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnltq	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltq	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpnltq	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltq	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpcmpnleq	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpnleq	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpnleq	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpnleq	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpnleq	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpnleq	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnleq	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnleq	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnleq	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnleq	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnleq	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpnleq	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnleq	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpcmpud	$0xab, %zmm5, %zmm6, %k5	 # AVX512F
	vpcmpud	$0xab, %zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpud	$123, %zmm5, %zmm6, %k5	 # AVX512F
	vpcmpud	$123, (%ecx), %zmm6, %k5	 # AVX512F
	vpcmpud	$123, -123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpud	$123, (%eax){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpud	$123, 8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpud	$123, 8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpud	$123, -8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpud	$123, -8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpud	$123, 508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpud	$123, 512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpud	$123, -512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpud	$123, -516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vpcmpequd	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpequd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpequd	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpequd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpequd	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpequd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpequd	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpequd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpequd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpequd	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpequd	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpequd	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpequd	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vpcmpltud	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpltud	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpltud	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpltud	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpltud	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpltud	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpltud	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpltud	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpltud	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpltud	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpltud	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpltud	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpltud	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vpcmpleud	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpleud	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpleud	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpleud	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpleud	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpleud	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpleud	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpleud	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpleud	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpleud	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpleud	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpleud	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpleud	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vpcmpnequd	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpnequd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpnequd	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpnequd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpnequd	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpnequd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnequd	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnequd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnequd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnequd	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnequd	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpnequd	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnequd	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vpcmpnltud	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpnltud	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpnltud	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpnltud	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpnltud	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpnltud	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltud	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnltud	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltud	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnltud	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltud	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpnltud	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltud	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vpcmpnleud	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpnleud	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpnleud	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpnleud	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpnleud	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpnleud	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnleud	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnleud	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnleud	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnleud	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnleud	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vpcmpnleud	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnleud	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vpcmpuq	$0xab, %zmm5, %zmm6, %k5	 # AVX512F
	vpcmpuq	$0xab, %zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpuq	$123, %zmm5, %zmm6, %k5	 # AVX512F
	vpcmpuq	$123, (%ecx), %zmm6, %k5	 # AVX512F
	vpcmpuq	$123, -123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpuq	$123, (%eax){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpuq	$123, 8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpuq	$123, 8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpuq	$123, -8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpuq	$123, -8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpuq	$123, 1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpuq	$123, 1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpuq	$123, -1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpuq	$123, -1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpcmpequq	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpequq	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpequq	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpequq	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpequq	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpequq	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpequq	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpequq	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpequq	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpequq	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpequq	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpequq	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpequq	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpcmpltuq	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpltuq	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpltuq	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpltuq	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpltuq	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpltuq	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpltuq	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpltuq	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpltuq	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpltuq	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpltuq	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpltuq	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpltuq	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpcmpleuq	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpleuq	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpleuq	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpleuq	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpleuq	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpleuq	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpleuq	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpleuq	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpleuq	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpleuq	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpleuq	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpleuq	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpleuq	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpcmpnequq	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpnequq	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpnequq	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpnequq	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpnequq	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpnequq	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnequq	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnequq	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnequq	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnequq	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnequq	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpnequq	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnequq	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpcmpnltuq	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpnltuq	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpnltuq	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpnltuq	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpnltuq	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpnltuq	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltuq	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnltuq	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltuq	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnltuq	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltuq	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpnltuq	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnltuq	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpcmpnleuq	%zmm5, %zmm6, %k5	 # AVX512F
	vpcmpnleuq	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vpcmpnleuq	(%ecx), %zmm6, %k5	 # AVX512F
	vpcmpnleuq	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vpcmpnleuq	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpnleuq	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnleuq	8192(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnleuq	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vpcmpnleuq	-8256(%edx), %zmm6, %k5	 # AVX512F
	vpcmpnleuq	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnleuq	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vpcmpnleuq	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vpcmpnleuq	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpblendmq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpblendmq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpblendmq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpblendmq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpblendmq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpblendmq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpblendmq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpblendmq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpblendmq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpblendmq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpblendmq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpblendmq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpblendmq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpblendmq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpcompressd	%zmm6, (%ecx)	 # AVX512F
	vpcompressd	%zmm6, (%ecx){%k7}	 # AVX512F
	vpcompressd	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpcompressd	%zmm6, 508(%edx)	 # AVX512F Disp8
	vpcompressd	%zmm6, 512(%edx)	 # AVX512F
	vpcompressd	%zmm6, -512(%edx)	 # AVX512F Disp8
	vpcompressd	%zmm6, -516(%edx)	 # AVX512F

	vpcompressd	%zmm5, %zmm6	 # AVX512F
	vpcompressd	%zmm5, %zmm6{%k7}	 # AVX512F
	vpcompressd	%zmm5, %zmm6{%k7}{z}	 # AVX512F

	vpermd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpermd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpermd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpermd	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpermd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpermd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpermd	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermd	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpermd	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermd	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpermilpd	$0xab, %zmm5, %zmm6	 # AVX512F
	vpermilpd	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermilpd	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermilpd	$123, %zmm5, %zmm6	 # AVX512F
	vpermilpd	$123, (%ecx), %zmm6	 # AVX512F
	vpermilpd	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpermilpd	$123, (%eax){1to8}, %zmm6	 # AVX512F
	vpermilpd	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vpermilpd	$123, 8192(%edx), %zmm6	 # AVX512F
	vpermilpd	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vpermilpd	$123, -8256(%edx), %zmm6	 # AVX512F
	vpermilpd	$123, 1016(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vpermilpd	$123, 1024(%edx){1to8}, %zmm6	 # AVX512F
	vpermilpd	$123, -1024(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vpermilpd	$123, -1032(%edx){1to8}, %zmm6	 # AVX512F

	vpermilpd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpermilpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermilpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermilpd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpermilpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpermilpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpermilpd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermilpd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpermilpd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermilpd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpermilpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermilpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpermilpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermilpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpermilps	$0xab, %zmm5, %zmm6	 # AVX512F
	vpermilps	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermilps	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermilps	$123, %zmm5, %zmm6	 # AVX512F
	vpermilps	$123, (%ecx), %zmm6	 # AVX512F
	vpermilps	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpermilps	$123, (%eax){1to16}, %zmm6	 # AVX512F
	vpermilps	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vpermilps	$123, 8192(%edx), %zmm6	 # AVX512F
	vpermilps	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vpermilps	$123, -8256(%edx), %zmm6	 # AVX512F
	vpermilps	$123, 508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vpermilps	$123, 512(%edx){1to16}, %zmm6	 # AVX512F
	vpermilps	$123, -512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vpermilps	$123, -516(%edx){1to16}, %zmm6	 # AVX512F

	vpermilps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpermilps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermilps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermilps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpermilps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpermilps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpermilps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermilps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpermilps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermilps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpermilps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermilps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpermilps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermilps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpermpd	$0xab, %zmm5, %zmm6	 # AVX512F
	vpermpd	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermpd	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermpd	$123, %zmm5, %zmm6	 # AVX512F
	vpermpd	$123, (%ecx), %zmm6	 # AVX512F
	vpermpd	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpermpd	$123, (%eax){1to8}, %zmm6	 # AVX512F
	vpermpd	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vpermpd	$123, 8192(%edx), %zmm6	 # AVX512F
	vpermpd	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vpermpd	$123, -8256(%edx), %zmm6	 # AVX512F
	vpermpd	$123, 1016(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vpermpd	$123, 1024(%edx){1to8}, %zmm6	 # AVX512F
	vpermpd	$123, -1024(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vpermpd	$123, -1032(%edx){1to8}, %zmm6	 # AVX512F

	vpermps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpermps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpermps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpermps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpermps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpermps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpermps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpermps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpermq	$0xab, %zmm5, %zmm6	 # AVX512F
	vpermq	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermq	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermq	$123, %zmm5, %zmm6	 # AVX512F
	vpermq	$123, (%ecx), %zmm6	 # AVX512F
	vpermq	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpermq	$123, (%eax){1to8}, %zmm6	 # AVX512F
	vpermq	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vpermq	$123, 8192(%edx), %zmm6	 # AVX512F
	vpermq	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vpermq	$123, -8256(%edx), %zmm6	 # AVX512F
	vpermq	$123, 1016(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vpermq	$123, 1024(%edx){1to8}, %zmm6	 # AVX512F
	vpermq	$123, -1024(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vpermq	$123, -1032(%edx){1to8}, %zmm6	 # AVX512F

	vpexpandd	(%ecx), %zmm6	 # AVX512F
	vpexpandd	(%ecx), %zmm6{%k7}	 # AVX512F
	vpexpandd	(%ecx), %zmm6{%k7}{z}	 # AVX512F
	vpexpandd	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpexpandd	508(%edx), %zmm6	 # AVX512F Disp8
	vpexpandd	512(%edx), %zmm6	 # AVX512F
	vpexpandd	-512(%edx), %zmm6	 # AVX512F Disp8
	vpexpandd	-516(%edx), %zmm6	 # AVX512F

	vpexpandd	%zmm5, %zmm6	 # AVX512F
	vpexpandd	%zmm5, %zmm6{%k7}	 # AVX512F
	vpexpandd	%zmm5, %zmm6{%k7}{z}	 # AVX512F

	vpexpandq	(%ecx), %zmm6	 # AVX512F
	vpexpandq	(%ecx), %zmm6{%k7}	 # AVX512F
	vpexpandq	(%ecx), %zmm6{%k7}{z}	 # AVX512F
	vpexpandq	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpexpandq	1016(%edx), %zmm6	 # AVX512F Disp8
	vpexpandq	1024(%edx), %zmm6	 # AVX512F
	vpexpandq	-1024(%edx), %zmm6	 # AVX512F Disp8
	vpexpandq	-1032(%edx), %zmm6	 # AVX512F

	vpexpandq	%zmm5, %zmm6	 # AVX512F
	vpexpandq	%zmm5, %zmm6{%k7}	 # AVX512F
	vpexpandq	%zmm5, %zmm6{%k7}{z}	 # AVX512F

	vpgatherdd	123(%ebp,%zmm7,8), %zmm6{%k1}	 # AVX512F
	vpgatherdd	123(%ebp,%zmm7,8), %zmm6{%k1}	 # AVX512F
	vpgatherdd	256(%eax,%zmm7), %zmm6{%k1}	 # AVX512F
	vpgatherdd	1024(%ecx,%zmm7,4), %zmm6{%k1}	 # AVX512F

	vpgatherdq	123(%ebp,%ymm7,8), %zmm6{%k1}	 # AVX512F
	vpgatherdq	123(%ebp,%ymm7,8), %zmm6{%k1}	 # AVX512F
	vpgatherdq	256(%eax,%ymm7), %zmm6{%k1}	 # AVX512F
	vpgatherdq	1024(%ecx,%ymm7,4), %zmm6{%k1}	 # AVX512F

	vpgatherqd	123(%ebp,%zmm7,8), %ymm6{%k1}	 # AVX512F
	vpgatherqd	123(%ebp,%zmm7,8), %ymm6{%k1}	 # AVX512F
	vpgatherqd	256(%eax,%zmm7), %ymm6{%k1}	 # AVX512F
	vpgatherqd	1024(%ecx,%zmm7,4), %ymm6{%k1}	 # AVX512F

	vpgatherqq	123(%ebp,%zmm7,8), %zmm6{%k1}	 # AVX512F
	vpgatherqq	123(%ebp,%zmm7,8), %zmm6{%k1}	 # AVX512F
	vpgatherqq	256(%eax,%zmm7), %zmm6{%k1}	 # AVX512F
	vpgatherqq	1024(%ecx,%zmm7,4), %zmm6{%k1}	 # AVX512F

	vpmaxsd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpmaxsd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpmaxsd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpmaxsd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpmaxsd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpmaxsd	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpmaxsd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxsd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpmaxsd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxsd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpmaxsd	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxsd	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpmaxsd	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxsd	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpmaxsq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpmaxsq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpmaxsq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpmaxsq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpmaxsq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpmaxsq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpmaxsq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxsq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpmaxsq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxsq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpmaxsq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxsq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpmaxsq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxsq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpmaxud	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpmaxud	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpmaxud	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpmaxud	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpmaxud	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpmaxud	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpmaxud	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxud	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpmaxud	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxud	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpmaxud	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxud	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpmaxud	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxud	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpmaxuq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpmaxuq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpmaxuq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpmaxuq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpmaxuq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpmaxuq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpmaxuq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxuq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpmaxuq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxuq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpmaxuq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxuq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpmaxuq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpmaxuq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpminsd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpminsd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpminsd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpminsd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpminsd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpminsd	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpminsd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpminsd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpminsd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpminsd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpminsd	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpminsd	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpminsd	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpminsd	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpminsq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpminsq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpminsq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpminsq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpminsq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpminsq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpminsq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpminsq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpminsq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpminsq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpminsq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpminsq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpminsq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpminsq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpminud	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpminud	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpminud	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpminud	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpminud	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpminud	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpminud	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpminud	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpminud	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpminud	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpminud	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpminud	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpminud	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpminud	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpminuq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpminuq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpminuq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpminuq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpminuq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpminuq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpminuq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpminuq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpminuq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpminuq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpminuq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpminuq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpminuq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpminuq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpmovsxbd	%xmm5, %zmm6{%k7}	 # AVX512F
	vpmovsxbd	%xmm5, %zmm6{%k7}{z}	 # AVX512F
	vpmovsxbd	(%ecx), %zmm6{%k7}	 # AVX512F
	vpmovsxbd	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512F
	vpmovsxbd	2032(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovsxbd	2048(%edx), %zmm6{%k7}	 # AVX512F
	vpmovsxbd	-2048(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovsxbd	-2064(%edx), %zmm6{%k7}	 # AVX512F

	vpmovsxbq	%xmm5, %zmm6{%k7}	 # AVX512F
	vpmovsxbq	%xmm5, %zmm6{%k7}{z}	 # AVX512F
	vpmovsxbq	(%ecx), %zmm6{%k7}	 # AVX512F
	vpmovsxbq	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512F
	vpmovsxbq	1016(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovsxbq	1024(%edx), %zmm6{%k7}	 # AVX512F
	vpmovsxbq	-1024(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovsxbq	-1032(%edx), %zmm6{%k7}	 # AVX512F

	vpmovsxdq	%ymm5, %zmm6{%k7}	 # AVX512F
	vpmovsxdq	%ymm5, %zmm6{%k7}{z}	 # AVX512F
	vpmovsxdq	(%ecx), %zmm6{%k7}	 # AVX512F
	vpmovsxdq	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512F
	vpmovsxdq	4064(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovsxdq	4096(%edx), %zmm6{%k7}	 # AVX512F
	vpmovsxdq	-4096(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovsxdq	-4128(%edx), %zmm6{%k7}	 # AVX512F

	vpmovsxwd	%ymm5, %zmm6{%k7}	 # AVX512F
	vpmovsxwd	%ymm5, %zmm6{%k7}{z}	 # AVX512F
	vpmovsxwd	(%ecx), %zmm6{%k7}	 # AVX512F
	vpmovsxwd	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512F
	vpmovsxwd	4064(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovsxwd	4096(%edx), %zmm6{%k7}	 # AVX512F
	vpmovsxwd	-4096(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovsxwd	-4128(%edx), %zmm6{%k7}	 # AVX512F

	vpmovsxwq	%xmm5, %zmm6{%k7}	 # AVX512F
	vpmovsxwq	%xmm5, %zmm6{%k7}{z}	 # AVX512F
	vpmovsxwq	(%ecx), %zmm6{%k7}	 # AVX512F
	vpmovsxwq	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512F
	vpmovsxwq	2032(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovsxwq	2048(%edx), %zmm6{%k7}	 # AVX512F
	vpmovsxwq	-2048(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovsxwq	-2064(%edx), %zmm6{%k7}	 # AVX512F

	vpmovzxbd	%xmm5, %zmm6{%k7}	 # AVX512F
	vpmovzxbd	%xmm5, %zmm6{%k7}{z}	 # AVX512F
	vpmovzxbd	(%ecx), %zmm6{%k7}	 # AVX512F
	vpmovzxbd	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512F
	vpmovzxbd	2032(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovzxbd	2048(%edx), %zmm6{%k7}	 # AVX512F
	vpmovzxbd	-2048(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovzxbd	-2064(%edx), %zmm6{%k7}	 # AVX512F

	vpmovzxbq	%xmm5, %zmm6{%k7}	 # AVX512F
	vpmovzxbq	%xmm5, %zmm6{%k7}{z}	 # AVX512F
	vpmovzxbq	(%ecx), %zmm6{%k7}	 # AVX512F
	vpmovzxbq	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512F
	vpmovzxbq	1016(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovzxbq	1024(%edx), %zmm6{%k7}	 # AVX512F
	vpmovzxbq	-1024(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovzxbq	-1032(%edx), %zmm6{%k7}	 # AVX512F

	vpmovzxdq	%ymm5, %zmm6{%k7}	 # AVX512F
	vpmovzxdq	%ymm5, %zmm6{%k7}{z}	 # AVX512F
	vpmovzxdq	(%ecx), %zmm6{%k7}	 # AVX512F
	vpmovzxdq	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512F
	vpmovzxdq	4064(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovzxdq	4096(%edx), %zmm6{%k7}	 # AVX512F
	vpmovzxdq	-4096(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovzxdq	-4128(%edx), %zmm6{%k7}	 # AVX512F

	vpmovzxwd	%ymm5, %zmm6{%k7}	 # AVX512F
	vpmovzxwd	%ymm5, %zmm6{%k7}{z}	 # AVX512F
	vpmovzxwd	(%ecx), %zmm6{%k7}	 # AVX512F
	vpmovzxwd	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512F
	vpmovzxwd	4064(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovzxwd	4096(%edx), %zmm6{%k7}	 # AVX512F
	vpmovzxwd	-4096(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovzxwd	-4128(%edx), %zmm6{%k7}	 # AVX512F

	vpmovzxwq	%xmm5, %zmm6{%k7}	 # AVX512F
	vpmovzxwq	%xmm5, %zmm6{%k7}{z}	 # AVX512F
	vpmovzxwq	(%ecx), %zmm6{%k7}	 # AVX512F
	vpmovzxwq	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512F
	vpmovzxwq	2032(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovzxwq	2048(%edx), %zmm6{%k7}	 # AVX512F
	vpmovzxwq	-2048(%edx), %zmm6{%k7}	 # AVX512F Disp8
	vpmovzxwq	-2064(%edx), %zmm6{%k7}	 # AVX512F

	vpmuldq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpmuldq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpmuldq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpmuldq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpmuldq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpmuldq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpmuldq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpmuldq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpmuldq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpmuldq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpmuldq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpmuldq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpmuldq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpmuldq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpmulld	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpmulld	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpmulld	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpmulld	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpmulld	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpmulld	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpmulld	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpmulld	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpmulld	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpmulld	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpmulld	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpmulld	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpmulld	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpmulld	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpmuludq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpmuludq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpmuludq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpmuludq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpmuludq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpmuludq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpmuludq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpmuludq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpmuludq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpmuludq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpmuludq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpmuludq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpmuludq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpmuludq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpord	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpord	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpord	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpord	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpord	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpord	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpord	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpord	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpord	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpord	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpord	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpord	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpord	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpord	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vporq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vporq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vporq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vporq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vporq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vporq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vporq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vporq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vporq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vporq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vporq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vporq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vporq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vporq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpscatterdd	%zmm6, 123(%ebp,%zmm7,8){%k1}	 # AVX512F
	vpscatterdd	%zmm6, 123(%ebp,%zmm7,8){%k1}	 # AVX512F
	vpscatterdd	%zmm6, 256(%eax,%zmm7){%k1}	 # AVX512F
	vpscatterdd	%zmm6, 1024(%ecx,%zmm7,4){%k1}	 # AVX512F

	vpscatterdq	%zmm6, 123(%ebp,%ymm7,8){%k1}	 # AVX512F
	vpscatterdq	%zmm6, 123(%ebp,%ymm7,8){%k1}	 # AVX512F
	vpscatterdq	%zmm6, 256(%eax,%ymm7){%k1}	 # AVX512F
	vpscatterdq	%zmm6, 1024(%ecx,%ymm7,4){%k1}	 # AVX512F

	vpscatterqd	%ymm6, 123(%ebp,%zmm7,8){%k1}	 # AVX512F
	vpscatterqd	%ymm6, 123(%ebp,%zmm7,8){%k1}	 # AVX512F
	vpscatterqd	%ymm6, 256(%eax,%zmm7){%k1}	 # AVX512F
	vpscatterqd	%ymm6, 1024(%ecx,%zmm7,4){%k1}	 # AVX512F

	vpscatterqq	%zmm6, 123(%ebp,%zmm7,8){%k1}	 # AVX512F
	vpscatterqq	%zmm6, 123(%ebp,%zmm7,8){%k1}	 # AVX512F
	vpscatterqq	%zmm6, 256(%eax,%zmm7){%k1}	 # AVX512F
	vpscatterqq	%zmm6, 1024(%ecx,%zmm7,4){%k1}	 # AVX512F

	vpshufd	$0xab, %zmm5, %zmm6	 # AVX512F
	vpshufd	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vpshufd	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpshufd	$123, %zmm5, %zmm6	 # AVX512F
	vpshufd	$123, (%ecx), %zmm6	 # AVX512F
	vpshufd	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpshufd	$123, (%eax){1to16}, %zmm6	 # AVX512F
	vpshufd	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vpshufd	$123, 8192(%edx), %zmm6	 # AVX512F
	vpshufd	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vpshufd	$123, -8256(%edx), %zmm6	 # AVX512F
	vpshufd	$123, 508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vpshufd	$123, 512(%edx){1to16}, %zmm6	 # AVX512F
	vpshufd	$123, -512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vpshufd	$123, -516(%edx){1to16}, %zmm6	 # AVX512F

	vpslld	%xmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpslld	%xmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpslld	(%ecx), %zmm5, %zmm6{%k7}	 # AVX512F
	vpslld	-123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512F
	vpslld	2032(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vpslld	2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F
	vpslld	-2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vpslld	-2064(%edx), %zmm5, %zmm6{%k7}	 # AVX512F

	vpsllq	%xmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsllq	%xmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsllq	(%ecx), %zmm5, %zmm6{%k7}	 # AVX512F
	vpsllq	-123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512F
	vpsllq	2032(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vpsllq	2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F
	vpsllq	-2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vpsllq	-2064(%edx), %zmm5, %zmm6{%k7}	 # AVX512F

	vpsllvd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpsllvd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsllvd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsllvd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpsllvd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpsllvd	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpsllvd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsllvd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpsllvd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsllvd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpsllvd	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsllvd	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpsllvd	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsllvd	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpsllvq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpsllvq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsllvq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsllvq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpsllvq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpsllvq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpsllvq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsllvq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpsllvq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsllvq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpsllvq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsllvq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpsllvq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsllvq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpsrad	%xmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrad	%xmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsrad	(%ecx), %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrad	-123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrad	2032(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vpsrad	2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrad	-2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vpsrad	-2064(%edx), %zmm5, %zmm6{%k7}	 # AVX512F

	vpsraq	%xmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsraq	%xmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsraq	(%ecx), %zmm5, %zmm6{%k7}	 # AVX512F
	vpsraq	-123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512F
	vpsraq	2032(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vpsraq	2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F
	vpsraq	-2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vpsraq	-2064(%edx), %zmm5, %zmm6{%k7}	 # AVX512F

	vpsravd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpsravd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsravd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsravd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpsravd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpsravd	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpsravd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsravd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpsravd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsravd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpsravd	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsravd	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpsravd	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsravd	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpsravq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpsravq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsravq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsravq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpsravq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpsravq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpsravq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsravq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpsravq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsravq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpsravq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsravq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpsravq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsravq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpsrld	%xmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrld	%xmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsrld	(%ecx), %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrld	-123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrld	2032(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vpsrld	2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrld	-2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vpsrld	-2064(%edx), %zmm5, %zmm6{%k7}	 # AVX512F

	vpsrlq	%xmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrlq	%xmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsrlq	(%ecx), %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrlq	-123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrlq	2032(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vpsrlq	2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrlq	-2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512F Disp8
	vpsrlq	-2064(%edx), %zmm5, %zmm6{%k7}	 # AVX512F

	vpsrlvd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpsrlvd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrlvd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsrlvd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpsrlvd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpsrlvd	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpsrlvd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsrlvd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpsrlvd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsrlvd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpsrlvd	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsrlvd	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpsrlvd	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsrlvd	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpsrlvq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpsrlvq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrlvq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsrlvq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpsrlvq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpsrlvq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpsrlvq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsrlvq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpsrlvq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsrlvq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpsrlvq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsrlvq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpsrlvq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsrlvq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpsrld	$0xab, %zmm5, %zmm6	 # AVX512F
	vpsrld	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrld	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsrld	$123, %zmm5, %zmm6	 # AVX512F
	vpsrld	$123, (%ecx), %zmm6	 # AVX512F
	vpsrld	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpsrld	$123, (%eax){1to16}, %zmm6	 # AVX512F
	vpsrld	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vpsrld	$123, 8192(%edx), %zmm6	 # AVX512F
	vpsrld	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vpsrld	$123, -8256(%edx), %zmm6	 # AVX512F
	vpsrld	$123, 508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vpsrld	$123, 512(%edx){1to16}, %zmm6	 # AVX512F
	vpsrld	$123, -512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vpsrld	$123, -516(%edx){1to16}, %zmm6	 # AVX512F

	vpsrlq	$0xab, %zmm5, %zmm6	 # AVX512F
	vpsrlq	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrlq	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsrlq	$123, %zmm5, %zmm6	 # AVX512F
	vpsrlq	$123, (%ecx), %zmm6	 # AVX512F
	vpsrlq	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpsrlq	$123, (%eax){1to8}, %zmm6	 # AVX512F
	vpsrlq	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vpsrlq	$123, 8192(%edx), %zmm6	 # AVX512F
	vpsrlq	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vpsrlq	$123, -8256(%edx), %zmm6	 # AVX512F
	vpsrlq	$123, 1016(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vpsrlq	$123, 1024(%edx){1to8}, %zmm6	 # AVX512F
	vpsrlq	$123, -1024(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vpsrlq	$123, -1032(%edx){1to8}, %zmm6	 # AVX512F

	vpsubd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpsubd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsubd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsubd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpsubd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpsubd	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpsubd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsubd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpsubd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsubd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpsubd	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsubd	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpsubd	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsubd	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpsubq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpsubq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsubq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsubq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpsubq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpsubq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpsubq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsubq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpsubq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpsubq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpsubq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsubq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpsubq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpsubq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vptestmd	%zmm5, %zmm6, %k5	 # AVX512F
	vptestmd	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vptestmd	(%ecx), %zmm6, %k5	 # AVX512F
	vptestmd	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vptestmd	(%eax){1to16}, %zmm6, %k5	 # AVX512F
	vptestmd	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vptestmd	8192(%edx), %zmm6, %k5	 # AVX512F
	vptestmd	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vptestmd	-8256(%edx), %zmm6, %k5	 # AVX512F
	vptestmd	508(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vptestmd	512(%edx){1to16}, %zmm6, %k5	 # AVX512F
	vptestmd	-512(%edx){1to16}, %zmm6, %k5	 # AVX512F Disp8
	vptestmd	-516(%edx){1to16}, %zmm6, %k5	 # AVX512F

	vptestmq	%zmm5, %zmm6, %k5	 # AVX512F
	vptestmq	%zmm5, %zmm6, %k5{%k7}	 # AVX512F
	vptestmq	(%ecx), %zmm6, %k5	 # AVX512F
	vptestmq	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512F
	vptestmq	(%eax){1to8}, %zmm6, %k5	 # AVX512F
	vptestmq	8128(%edx), %zmm6, %k5	 # AVX512F Disp8
	vptestmq	8192(%edx), %zmm6, %k5	 # AVX512F
	vptestmq	-8192(%edx), %zmm6, %k5	 # AVX512F Disp8
	vptestmq	-8256(%edx), %zmm6, %k5	 # AVX512F
	vptestmq	1016(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vptestmq	1024(%edx){1to8}, %zmm6, %k5	 # AVX512F
	vptestmq	-1024(%edx){1to8}, %zmm6, %k5	 # AVX512F Disp8
	vptestmq	-1032(%edx){1to8}, %zmm6, %k5	 # AVX512F

	vpunpckhdq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpunpckhdq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpunpckhdq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpunpckhdq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpunpckhdq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpunpckhdq	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpunpckhdq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpunpckhdq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpunpckhdq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpunpckhdq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpunpckhdq	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpunpckhdq	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpunpckhdq	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpunpckhdq	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpunpckhqdq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpunpckhqdq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpunpckhqdq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpunpckhqdq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpunpckhqdq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpunpckhqdq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpunpckhqdq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpunpckhqdq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpunpckhqdq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpunpckhqdq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpunpckhqdq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpunpckhqdq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpunpckhqdq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpunpckhqdq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpunpckldq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpunpckldq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpunpckldq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpunpckldq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpunpckldq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpunpckldq	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpunpckldq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpunpckldq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpunpckldq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpunpckldq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpunpckldq	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpunpckldq	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpunpckldq	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpunpckldq	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpunpcklqdq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpunpcklqdq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpunpcklqdq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpunpcklqdq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpunpcklqdq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpunpcklqdq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpunpcklqdq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpunpcklqdq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpunpcklqdq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpunpcklqdq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpunpcklqdq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpunpcklqdq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpunpcklqdq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpunpcklqdq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpxord	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpxord	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpxord	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpxord	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpxord	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpxord	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpxord	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpxord	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpxord	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpxord	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpxord	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpxord	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpxord	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpxord	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpxorq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpxorq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpxorq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpxorq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpxorq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpxorq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpxorq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpxorq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpxorq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpxorq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpxorq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpxorq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpxorq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpxorq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vrcp14pd	%zmm5, %zmm6	 # AVX512F
	vrcp14pd	%zmm5, %zmm6{%k7}	 # AVX512F
	vrcp14pd	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vrcp14pd	(%ecx), %zmm6	 # AVX512F
	vrcp14pd	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vrcp14pd	(%eax){1to8}, %zmm6	 # AVX512F
	vrcp14pd	8128(%edx), %zmm6	 # AVX512F Disp8
	vrcp14pd	8192(%edx), %zmm6	 # AVX512F
	vrcp14pd	-8192(%edx), %zmm6	 # AVX512F Disp8
	vrcp14pd	-8256(%edx), %zmm6	 # AVX512F
	vrcp14pd	1016(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vrcp14pd	1024(%edx){1to8}, %zmm6	 # AVX512F
	vrcp14pd	-1024(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vrcp14pd	-1032(%edx){1to8}, %zmm6	 # AVX512F

	vrcp14ps	%zmm5, %zmm6	 # AVX512F
	vrcp14ps	%zmm5, %zmm6{%k7}	 # AVX512F
	vrcp14ps	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vrcp14ps	(%ecx), %zmm6	 # AVX512F
	vrcp14ps	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vrcp14ps	(%eax){1to16}, %zmm6	 # AVX512F
	vrcp14ps	8128(%edx), %zmm6	 # AVX512F Disp8
	vrcp14ps	8192(%edx), %zmm6	 # AVX512F
	vrcp14ps	-8192(%edx), %zmm6	 # AVX512F Disp8
	vrcp14ps	-8256(%edx), %zmm6	 # AVX512F
	vrcp14ps	508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vrcp14ps	512(%edx){1to16}, %zmm6	 # AVX512F
	vrcp14ps	-512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vrcp14ps	-516(%edx){1to16}, %zmm6	 # AVX512F

	vrcp14sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrcp14sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vrcp14sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vrcp14sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vrcp14sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vrcp14sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vrcp14sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vrcp14sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vrcp14ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrcp14ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vrcp14ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vrcp14ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vrcp14ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vrcp14ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vrcp14ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vrcp14ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vrsqrt14pd	%zmm5, %zmm6	 # AVX512F
	vrsqrt14pd	%zmm5, %zmm6{%k7}	 # AVX512F
	vrsqrt14pd	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vrsqrt14pd	(%ecx), %zmm6	 # AVX512F
	vrsqrt14pd	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vrsqrt14pd	(%eax){1to8}, %zmm6	 # AVX512F
	vrsqrt14pd	8128(%edx), %zmm6	 # AVX512F Disp8
	vrsqrt14pd	8192(%edx), %zmm6	 # AVX512F
	vrsqrt14pd	-8192(%edx), %zmm6	 # AVX512F Disp8
	vrsqrt14pd	-8256(%edx), %zmm6	 # AVX512F
	vrsqrt14pd	1016(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vrsqrt14pd	1024(%edx){1to8}, %zmm6	 # AVX512F
	vrsqrt14pd	-1024(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vrsqrt14pd	-1032(%edx){1to8}, %zmm6	 # AVX512F

	vrsqrt14ps	%zmm5, %zmm6	 # AVX512F
	vrsqrt14ps	%zmm5, %zmm6{%k7}	 # AVX512F
	vrsqrt14ps	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vrsqrt14ps	(%ecx), %zmm6	 # AVX512F
	vrsqrt14ps	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vrsqrt14ps	(%eax){1to16}, %zmm6	 # AVX512F
	vrsqrt14ps	8128(%edx), %zmm6	 # AVX512F Disp8
	vrsqrt14ps	8192(%edx), %zmm6	 # AVX512F
	vrsqrt14ps	-8192(%edx), %zmm6	 # AVX512F Disp8
	vrsqrt14ps	-8256(%edx), %zmm6	 # AVX512F
	vrsqrt14ps	508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vrsqrt14ps	512(%edx){1to16}, %zmm6	 # AVX512F
	vrsqrt14ps	-512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vrsqrt14ps	-516(%edx){1to16}, %zmm6	 # AVX512F

	vrsqrt14sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrsqrt14sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vrsqrt14sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vrsqrt14sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vrsqrt14sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vrsqrt14sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vrsqrt14sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vrsqrt14sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vrsqrt14ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrsqrt14ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vrsqrt14ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vrsqrt14ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vrsqrt14ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vrsqrt14ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vrsqrt14ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vrsqrt14ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vscatterdpd	%zmm6, 123(%ebp,%ymm7,8){%k1}	 # AVX512F
	vscatterdpd	%zmm6, 123(%ebp,%ymm7,8){%k1}	 # AVX512F
	vscatterdpd	%zmm6, 256(%eax,%ymm7){%k1}	 # AVX512F
	vscatterdpd	%zmm6, 1024(%ecx,%ymm7,4){%k1}	 # AVX512F

	vscatterdps	%zmm6, 123(%ebp,%zmm7,8){%k1}	 # AVX512F
	vscatterdps	%zmm6, 123(%ebp,%zmm7,8){%k1}	 # AVX512F
	vscatterdps	%zmm6, 256(%eax,%zmm7){%k1}	 # AVX512F
	vscatterdps	%zmm6, 1024(%ecx,%zmm7,4){%k1}	 # AVX512F

	vscatterqpd	%zmm6, 123(%ebp,%zmm7,8){%k1}	 # AVX512F
	vscatterqpd	%zmm6, 123(%ebp,%zmm7,8){%k1}	 # AVX512F
	vscatterqpd	%zmm6, 256(%eax,%zmm7){%k1}	 # AVX512F
	vscatterqpd	%zmm6, 1024(%ecx,%zmm7,4){%k1}	 # AVX512F

	vscatterqps	%ymm6, 123(%ebp,%zmm7,8){%k1}	 # AVX512F
	vscatterqps	%ymm6, 123(%ebp,%zmm7,8){%k1}	 # AVX512F
	vscatterqps	%ymm6, 256(%eax,%zmm7){%k1}	 # AVX512F
	vscatterqps	%ymm6, 1024(%ecx,%zmm7,4){%k1}	 # AVX512F

	vshufpd	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512F
	vshufpd	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vshufpd	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vshufpd	$123, %zmm4, %zmm5, %zmm6	 # AVX512F
	vshufpd	$123, (%ecx), %zmm5, %zmm6	 # AVX512F
	vshufpd	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vshufpd	$123, (%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vshufpd	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vshufpd	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512F
	vshufpd	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vshufpd	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512F
	vshufpd	$123, 1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vshufpd	$123, 1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vshufpd	$123, -1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vshufpd	$123, -1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vshufps	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512F
	vshufps	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vshufps	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vshufps	$123, %zmm4, %zmm5, %zmm6	 # AVX512F
	vshufps	$123, (%ecx), %zmm5, %zmm6	 # AVX512F
	vshufps	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vshufps	$123, (%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vshufps	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vshufps	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512F
	vshufps	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vshufps	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512F
	vshufps	$123, 508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vshufps	$123, 512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vshufps	$123, -512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vshufps	$123, -516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vsqrtpd	%zmm5, %zmm6	 # AVX512F
	vsqrtpd	%zmm5, %zmm6{%k7}	 # AVX512F
	vsqrtpd	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vsqrtpd	{rn-sae}, %zmm5, %zmm6	 # AVX512F
	vsqrtpd	{ru-sae}, %zmm5, %zmm6	 # AVX512F
	vsqrtpd	{rd-sae}, %zmm5, %zmm6	 # AVX512F
	vsqrtpd	{rz-sae}, %zmm5, %zmm6	 # AVX512F
	vsqrtpd	(%ecx), %zmm6	 # AVX512F
	vsqrtpd	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vsqrtpd	(%eax){1to8}, %zmm6	 # AVX512F
	vsqrtpd	8128(%edx), %zmm6	 # AVX512F Disp8
	vsqrtpd	8192(%edx), %zmm6	 # AVX512F
	vsqrtpd	-8192(%edx), %zmm6	 # AVX512F Disp8
	vsqrtpd	-8256(%edx), %zmm6	 # AVX512F
	vsqrtpd	1016(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vsqrtpd	1024(%edx){1to8}, %zmm6	 # AVX512F
	vsqrtpd	-1024(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vsqrtpd	-1032(%edx){1to8}, %zmm6	 # AVX512F

	vsqrtps	%zmm5, %zmm6	 # AVX512F
	vsqrtps	%zmm5, %zmm6{%k7}	 # AVX512F
	vsqrtps	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vsqrtps	{rn-sae}, %zmm5, %zmm6	 # AVX512F
	vsqrtps	{ru-sae}, %zmm5, %zmm6	 # AVX512F
	vsqrtps	{rd-sae}, %zmm5, %zmm6	 # AVX512F
	vsqrtps	{rz-sae}, %zmm5, %zmm6	 # AVX512F
	vsqrtps	(%ecx), %zmm6	 # AVX512F
	vsqrtps	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vsqrtps	(%eax){1to16}, %zmm6	 # AVX512F
	vsqrtps	8128(%edx), %zmm6	 # AVX512F Disp8
	vsqrtps	8192(%edx), %zmm6	 # AVX512F
	vsqrtps	-8192(%edx), %zmm6	 # AVX512F Disp8
	vsqrtps	-8256(%edx), %zmm6	 # AVX512F
	vsqrtps	508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vsqrtps	512(%edx){1to16}, %zmm6	 # AVX512F
	vsqrtps	-512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vsqrtps	-516(%edx){1to16}, %zmm6	 # AVX512F

	vsqrtsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vsqrtsd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtsd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtsd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtsd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vsqrtsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vsqrtsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vsqrtss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vsqrtss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vsqrtss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vsqrtss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vsqrtss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vsubpd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vsubpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vsubpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vsubpd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vsubpd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vsubpd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vsubpd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vsubpd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vsubpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vsubpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vsubpd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vsubpd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vsubpd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vsubpd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vsubpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vsubpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vsubpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vsubpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vsubps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vsubps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vsubps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vsubps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vsubps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vsubps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vsubps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vsubps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vsubps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vsubps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vsubps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vsubps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vsubps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vsubps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vsubps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vsubps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vsubps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vsubps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vsubsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsubsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vsubsd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsubsd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsubsd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsubsd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsubsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vsubsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vsubsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vsubsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vsubsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vsubsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vsubss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsubss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vsubss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsubss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsubss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsubss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vsubss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vsubss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vsubss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vsubss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vsubss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vsubss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vucomisd	{sae}, %xmm5, %xmm6	 # AVX512F

	vucomiss	{sae}, %xmm5, %xmm6	 # AVX512F

	vunpckhpd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vunpckhpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vunpckhpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vunpckhpd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vunpckhpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vunpckhpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vunpckhpd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vunpckhpd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vunpckhpd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vunpckhpd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vunpckhpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vunpckhpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vunpckhpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vunpckhpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vunpckhps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vunpckhps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vunpckhps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vunpckhps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vunpckhps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vunpckhps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vunpckhps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vunpckhps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vunpckhps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vunpckhps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vunpckhps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vunpckhps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vunpckhps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vunpckhps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vunpcklpd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vunpcklpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vunpcklpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vunpcklpd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vunpcklpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vunpcklpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vunpcklpd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vunpcklpd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vunpcklpd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vunpcklpd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vunpcklpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vunpcklpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vunpcklpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vunpcklpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vunpcklps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vunpcklps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vunpcklps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vunpcklps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vunpcklps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vunpcklps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vunpcklps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vunpcklps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vunpcklps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vunpcklps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vunpcklps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vunpcklps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vunpcklps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vunpcklps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpternlogd	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512F
	vpternlogd	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpternlogd	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpternlogd	$123, %zmm4, %zmm5, %zmm6	 # AVX512F
	vpternlogd	$123, (%ecx), %zmm5, %zmm6	 # AVX512F
	vpternlogd	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpternlogd	$123, (%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpternlogd	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpternlogd	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpternlogd	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpternlogd	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpternlogd	$123, 508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpternlogd	$123, 512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpternlogd	$123, -512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpternlogd	$123, -516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpternlogq	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512F
	vpternlogq	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpternlogq	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpternlogq	$123, %zmm4, %zmm5, %zmm6	 # AVX512F
	vpternlogq	$123, (%ecx), %zmm5, %zmm6	 # AVX512F
	vpternlogq	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpternlogq	$123, (%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpternlogq	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpternlogq	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpternlogq	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpternlogq	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpternlogq	$123, 1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpternlogq	$123, 1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpternlogq	$123, -1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpternlogq	$123, -1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpmovqb	%zmm5, %xmm6{%k7}	 # AVX512F
	vpmovqb	%zmm5, %xmm6{%k7}{z}	 # AVX512F

	vpmovsqb	%zmm5, %xmm6{%k7}	 # AVX512F
	vpmovsqb	%zmm5, %xmm6{%k7}{z}	 # AVX512F

	vpmovusqb	%zmm5, %xmm6{%k7}	 # AVX512F
	vpmovusqb	%zmm5, %xmm6{%k7}{z}	 # AVX512F

	vpmovqw	%zmm5, %xmm6{%k7}	 # AVX512F
	vpmovqw	%zmm5, %xmm6{%k7}{z}	 # AVX512F

	vpmovsqw	%zmm5, %xmm6{%k7}	 # AVX512F
	vpmovsqw	%zmm5, %xmm6{%k7}{z}	 # AVX512F

	vpmovusqw	%zmm5, %xmm6{%k7}	 # AVX512F
	vpmovusqw	%zmm5, %xmm6{%k7}{z}	 # AVX512F

	vpmovqd	%zmm5, %ymm6{%k7}	 # AVX512F
	vpmovqd	%zmm5, %ymm6{%k7}{z}	 # AVX512F

	vpmovsqd	%zmm5, %ymm6{%k7}	 # AVX512F
	vpmovsqd	%zmm5, %ymm6{%k7}{z}	 # AVX512F

	vpmovusqd	%zmm5, %ymm6{%k7}	 # AVX512F
	vpmovusqd	%zmm5, %ymm6{%k7}{z}	 # AVX512F

	vpmovdb	%zmm5, %xmm6{%k7}	 # AVX512F
	vpmovdb	%zmm5, %xmm6{%k7}{z}	 # AVX512F

	vpmovsdb	%zmm5, %xmm6{%k7}	 # AVX512F
	vpmovsdb	%zmm5, %xmm6{%k7}{z}	 # AVX512F

	vpmovusdb	%zmm5, %xmm6{%k7}	 # AVX512F
	vpmovusdb	%zmm5, %xmm6{%k7}{z}	 # AVX512F

	vpmovdw	%zmm5, %ymm6{%k7}	 # AVX512F
	vpmovdw	%zmm5, %ymm6{%k7}{z}	 # AVX512F

	vpmovsdw	%zmm5, %ymm6{%k7}	 # AVX512F
	vpmovsdw	%zmm5, %ymm6{%k7}{z}	 # AVX512F

	vpmovusdw	%zmm5, %ymm6{%k7}	 # AVX512F
	vpmovusdw	%zmm5, %ymm6{%k7}{z}	 # AVX512F

	vshuff32x4	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512F
	vshuff32x4	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vshuff32x4	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vshuff32x4	$123, %zmm4, %zmm5, %zmm6	 # AVX512F
	vshuff32x4	$123, (%ecx), %zmm5, %zmm6	 # AVX512F
	vshuff32x4	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vshuff32x4	$123, (%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vshuff32x4	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vshuff32x4	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512F
	vshuff32x4	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vshuff32x4	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512F
	vshuff32x4	$123, 508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vshuff32x4	$123, 512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vshuff32x4	$123, -512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vshuff32x4	$123, -516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vshuff64x2	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512F
	vshuff64x2	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vshuff64x2	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vshuff64x2	$123, %zmm4, %zmm5, %zmm6	 # AVX512F
	vshuff64x2	$123, (%ecx), %zmm5, %zmm6	 # AVX512F
	vshuff64x2	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vshuff64x2	$123, (%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vshuff64x2	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vshuff64x2	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512F
	vshuff64x2	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vshuff64x2	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512F
	vshuff64x2	$123, 1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vshuff64x2	$123, 1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vshuff64x2	$123, -1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vshuff64x2	$123, -1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vshufi32x4	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512F
	vshufi32x4	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vshufi32x4	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vshufi32x4	$123, %zmm4, %zmm5, %zmm6	 # AVX512F
	vshufi32x4	$123, (%ecx), %zmm5, %zmm6	 # AVX512F
	vshufi32x4	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vshufi32x4	$123, (%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vshufi32x4	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vshufi32x4	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512F
	vshufi32x4	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vshufi32x4	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512F
	vshufi32x4	$123, 508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vshufi32x4	$123, 512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vshufi32x4	$123, -512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vshufi32x4	$123, -516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vshufi64x2	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512F
	vshufi64x2	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vshufi64x2	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vshufi64x2	$123, %zmm4, %zmm5, %zmm6	 # AVX512F
	vshufi64x2	$123, (%ecx), %zmm5, %zmm6	 # AVX512F
	vshufi64x2	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vshufi64x2	$123, (%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vshufi64x2	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vshufi64x2	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512F
	vshufi64x2	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vshufi64x2	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512F
	vshufi64x2	$123, 1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vshufi64x2	$123, 1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vshufi64x2	$123, -1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vshufi64x2	$123, -1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpermq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpermq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpermq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpermq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpermq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpermq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpermq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpermq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpermpd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpermpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermpd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpermpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpermpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpermpd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermpd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpermpd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermpd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpermpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpermpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpermt2d	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpermt2d	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermt2d	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermt2d	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpermt2d	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpermt2d	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpermt2d	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2d	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpermt2d	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2d	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpermt2d	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2d	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpermt2d	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2d	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpermt2q	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpermt2q	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermt2q	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermt2q	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpermt2q	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpermt2q	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpermt2q	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2q	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpermt2q	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2q	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpermt2q	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2q	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpermt2q	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2q	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpermt2ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpermt2ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermt2ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermt2ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpermt2ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpermt2ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpermt2ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpermt2ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpermt2ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpermt2ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpermt2pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpermt2pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermt2pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermt2pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpermt2pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpermt2pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpermt2pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpermt2pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpermt2pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpermt2pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermt2pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	valignq	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512F
	valignq	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	valignq	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	valignq	$123, %zmm4, %zmm5, %zmm6	 # AVX512F
	valignq	$123, (%ecx), %zmm5, %zmm6	 # AVX512F
	valignq	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	valignq	$123, (%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	valignq	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	valignq	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512F
	valignq	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	valignq	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512F
	valignq	$123, 1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	valignq	$123, 1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	valignq	$123, -1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	valignq	$123, -1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vcvtsd2usi	%xmm6, %eax	 # AVX512F
	vcvtsd2usi	{rn-sae}, %xmm6, %eax	 # AVX512F
	vcvtsd2usi	{ru-sae}, %xmm6, %eax	 # AVX512F
	vcvtsd2usi	{rd-sae}, %xmm6, %eax	 # AVX512F
	vcvtsd2usi	{rz-sae}, %xmm6, %eax	 # AVX512F
	vcvtsd2usi	(%ecx), %eax	 # AVX512F
	vcvtsd2usi	-123456(%esp,%esi,8), %eax	 # AVX512F
	vcvtsd2usi	1016(%edx), %eax	 # AVX512F Disp8
	vcvtsd2usi	1024(%edx), %eax	 # AVX512F
	vcvtsd2usi	-1024(%edx), %eax	 # AVX512F Disp8
	vcvtsd2usi	-1032(%edx), %eax	 # AVX512F
	vcvtsd2usi	%xmm6, %ebp	 # AVX512F
	vcvtsd2usi	{rn-sae}, %xmm6, %ebp	 # AVX512F
	vcvtsd2usi	{ru-sae}, %xmm6, %ebp	 # AVX512F
	vcvtsd2usi	{rd-sae}, %xmm6, %ebp	 # AVX512F
	vcvtsd2usi	{rz-sae}, %xmm6, %ebp	 # AVX512F
	vcvtsd2usi	(%ecx), %ebp	 # AVX512F
	vcvtsd2usi	-123456(%esp,%esi,8), %ebp	 # AVX512F
	vcvtsd2usi	1016(%edx), %ebp	 # AVX512F Disp8
	vcvtsd2usi	1024(%edx), %ebp	 # AVX512F
	vcvtsd2usi	-1024(%edx), %ebp	 # AVX512F Disp8
	vcvtsd2usi	-1032(%edx), %ebp	 # AVX512F

	vcvtss2usi	%xmm6, %eax	 # AVX512F
	vcvtss2usi	{rn-sae}, %xmm6, %eax	 # AVX512F
	vcvtss2usi	{ru-sae}, %xmm6, %eax	 # AVX512F
	vcvtss2usi	{rd-sae}, %xmm6, %eax	 # AVX512F
	vcvtss2usi	{rz-sae}, %xmm6, %eax	 # AVX512F
	vcvtss2usi	(%ecx), %eax	 # AVX512F
	vcvtss2usi	-123456(%esp,%esi,8), %eax	 # AVX512F
	vcvtss2usi	508(%edx), %eax	 # AVX512F Disp8
	vcvtss2usi	512(%edx), %eax	 # AVX512F
	vcvtss2usi	-512(%edx), %eax	 # AVX512F Disp8
	vcvtss2usi	-516(%edx), %eax	 # AVX512F
	vcvtss2usi	%xmm6, %ebp	 # AVX512F
	vcvtss2usi	{rn-sae}, %xmm6, %ebp	 # AVX512F
	vcvtss2usi	{ru-sae}, %xmm6, %ebp	 # AVX512F
	vcvtss2usi	{rd-sae}, %xmm6, %ebp	 # AVX512F
	vcvtss2usi	{rz-sae}, %xmm6, %ebp	 # AVX512F
	vcvtss2usi	(%ecx), %ebp	 # AVX512F
	vcvtss2usi	-123456(%esp,%esi,8), %ebp	 # AVX512F
	vcvtss2usi	508(%edx), %ebp	 # AVX512F Disp8
	vcvtss2usi	512(%edx), %ebp	 # AVX512F
	vcvtss2usi	-512(%edx), %ebp	 # AVX512F Disp8
	vcvtss2usi	-516(%edx), %ebp	 # AVX512F

	vcvtusi2sdl	%eax, %xmm5, %xmm6	 # AVX512F
	vcvtusi2sdl	%ebp, %xmm5, %xmm6	 # AVX512F
	vcvtusi2sdl	(%ecx), %xmm5, %xmm6	 # AVX512F
	vcvtusi2sdl	-123456(%esp,%esi,8), %xmm5, %xmm6	 # AVX512F
	vcvtusi2sdl	508(%edx), %xmm5, %xmm6	 # AVX512F Disp8
	vcvtusi2sdl	512(%edx), %xmm5, %xmm6	 # AVX512F
	vcvtusi2sdl	-512(%edx), %xmm5, %xmm6	 # AVX512F Disp8
	vcvtusi2sdl	-516(%edx), %xmm5, %xmm6	 # AVX512F

	vcvtusi2ssl	%eax, %xmm5, %xmm6	 # AVX512F
	vcvtusi2ssl	%eax, {rn-sae}, %xmm5, %xmm6	 # AVX512F
	vcvtusi2ssl	%eax, {ru-sae}, %xmm5, %xmm6	 # AVX512F
	vcvtusi2ssl	%eax, {rd-sae}, %xmm5, %xmm6	 # AVX512F
	vcvtusi2ssl	%eax, {rz-sae}, %xmm5, %xmm6	 # AVX512F
	vcvtusi2ssl	%ebp, %xmm5, %xmm6	 # AVX512F
	vcvtusi2ssl	%ebp, {rn-sae}, %xmm5, %xmm6	 # AVX512F
	vcvtusi2ssl	%ebp, {ru-sae}, %xmm5, %xmm6	 # AVX512F
	vcvtusi2ssl	%ebp, {rd-sae}, %xmm5, %xmm6	 # AVX512F
	vcvtusi2ssl	%ebp, {rz-sae}, %xmm5, %xmm6	 # AVX512F
	vcvtusi2ssl	(%ecx), %xmm5, %xmm6	 # AVX512F
	vcvtusi2ssl	-123456(%esp,%esi,8), %xmm5, %xmm6	 # AVX512F
	vcvtusi2ssl	508(%edx), %xmm5, %xmm6	 # AVX512F Disp8
	vcvtusi2ssl	512(%edx), %xmm5, %xmm6	 # AVX512F
	vcvtusi2ssl	-512(%edx), %xmm5, %xmm6	 # AVX512F Disp8
	vcvtusi2ssl	-516(%edx), %xmm5, %xmm6	 # AVX512F

	vscalefpd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vscalefpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vscalefpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vscalefpd	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vscalefpd	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vscalefpd	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vscalefpd	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vscalefpd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vscalefpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vscalefpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vscalefpd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vscalefpd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vscalefpd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vscalefpd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vscalefpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vscalefpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vscalefpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vscalefpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vscalefps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vscalefps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vscalefps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vscalefps	{rn-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vscalefps	{ru-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vscalefps	{rd-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vscalefps	{rz-sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vscalefps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vscalefps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vscalefps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vscalefps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vscalefps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vscalefps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vscalefps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vscalefps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vscalefps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vscalefps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vscalefps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vscalefsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vscalefsd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefsd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefsd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefsd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vscalefsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vscalefsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vscalefss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vscalefss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vscalefss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vscalefss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vscalefss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfixupimmps	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfixupimmps	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfixupimmps	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfixupimmps	$0xab, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfixupimmps	$123, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfixupimmps	$123, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfixupimmps	$123, (%ecx), %zmm5, %zmm6	 # AVX512F
	vfixupimmps	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfixupimmps	$123, (%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vfixupimmps	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfixupimmps	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfixupimmps	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfixupimmps	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfixupimmps	$123, 508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfixupimmps	$123, 512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vfixupimmps	$123, -512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vfixupimmps	$123, -516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vfixupimmpd	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfixupimmpd	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vfixupimmpd	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vfixupimmpd	$0xab, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfixupimmpd	$123, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfixupimmpd	$123, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512F
	vfixupimmpd	$123, (%ecx), %zmm5, %zmm6	 # AVX512F
	vfixupimmpd	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vfixupimmpd	$123, (%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vfixupimmpd	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfixupimmpd	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512F
	vfixupimmpd	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vfixupimmpd	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512F
	vfixupimmpd	$123, 1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfixupimmpd	$123, 1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vfixupimmpd	$123, -1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vfixupimmpd	$123, -1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vfixupimmss	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmss	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfixupimmss	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmss	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmss	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmss	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmss	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmss	$123, 508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfixupimmss	$123, 512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmss	$123, -512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfixupimmss	$123, -516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vfixupimmsd	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmsd	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vfixupimmsd	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmsd	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmsd	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmsd	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmsd	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmsd	$123, 1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfixupimmsd	$123, 1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vfixupimmsd	$123, -1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vfixupimmsd	$123, -1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vpslld	$0xab, %zmm5, %zmm6	 # AVX512F
	vpslld	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vpslld	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpslld	$123, %zmm5, %zmm6	 # AVX512F
	vpslld	$123, (%ecx), %zmm6	 # AVX512F
	vpslld	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpslld	$123, (%eax){1to16}, %zmm6	 # AVX512F
	vpslld	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vpslld	$123, 8192(%edx), %zmm6	 # AVX512F
	vpslld	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vpslld	$123, -8256(%edx), %zmm6	 # AVX512F
	vpslld	$123, 508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vpslld	$123, 512(%edx){1to16}, %zmm6	 # AVX512F
	vpslld	$123, -512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vpslld	$123, -516(%edx){1to16}, %zmm6	 # AVX512F

	vpsllq	$0xab, %zmm5, %zmm6	 # AVX512F
	vpsllq	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsllq	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsllq	$123, %zmm5, %zmm6	 # AVX512F
	vpsllq	$123, (%ecx), %zmm6	 # AVX512F
	vpsllq	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpsllq	$123, (%eax){1to8}, %zmm6	 # AVX512F
	vpsllq	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vpsllq	$123, 8192(%edx), %zmm6	 # AVX512F
	vpsllq	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vpsllq	$123, -8256(%edx), %zmm6	 # AVX512F
	vpsllq	$123, 1016(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vpsllq	$123, 1024(%edx){1to8}, %zmm6	 # AVX512F
	vpsllq	$123, -1024(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vpsllq	$123, -1032(%edx){1to8}, %zmm6	 # AVX512F

	vpsrad	$0xab, %zmm5, %zmm6	 # AVX512F
	vpsrad	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsrad	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsrad	$123, %zmm5, %zmm6	 # AVX512F
	vpsrad	$123, (%ecx), %zmm6	 # AVX512F
	vpsrad	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpsrad	$123, (%eax){1to16}, %zmm6	 # AVX512F
	vpsrad	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vpsrad	$123, 8192(%edx), %zmm6	 # AVX512F
	vpsrad	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vpsrad	$123, -8256(%edx), %zmm6	 # AVX512F
	vpsrad	$123, 508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vpsrad	$123, 512(%edx){1to16}, %zmm6	 # AVX512F
	vpsrad	$123, -512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vpsrad	$123, -516(%edx){1to16}, %zmm6	 # AVX512F

	vpsraq	$0xab, %zmm5, %zmm6	 # AVX512F
	vpsraq	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vpsraq	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpsraq	$123, %zmm5, %zmm6	 # AVX512F
	vpsraq	$123, (%ecx), %zmm6	 # AVX512F
	vpsraq	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vpsraq	$123, (%eax){1to8}, %zmm6	 # AVX512F
	vpsraq	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vpsraq	$123, 8192(%edx), %zmm6	 # AVX512F
	vpsraq	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vpsraq	$123, -8256(%edx), %zmm6	 # AVX512F
	vpsraq	$123, 1016(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vpsraq	$123, 1024(%edx){1to8}, %zmm6	 # AVX512F
	vpsraq	$123, -1024(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vpsraq	$123, -1032(%edx){1to8}, %zmm6	 # AVX512F

	vprolvd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vprolvd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vprolvd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vprolvd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vprolvd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vprolvd	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vprolvd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vprolvd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vprolvd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vprolvd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vprolvd	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vprolvd	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vprolvd	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vprolvd	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vprold	$0xab, %zmm5, %zmm6	 # AVX512F
	vprold	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vprold	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vprold	$123, %zmm5, %zmm6	 # AVX512F
	vprold	$123, (%ecx), %zmm6	 # AVX512F
	vprold	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vprold	$123, (%eax){1to16}, %zmm6	 # AVX512F
	vprold	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vprold	$123, 8192(%edx), %zmm6	 # AVX512F
	vprold	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vprold	$123, -8256(%edx), %zmm6	 # AVX512F
	vprold	$123, 508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vprold	$123, 512(%edx){1to16}, %zmm6	 # AVX512F
	vprold	$123, -512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vprold	$123, -516(%edx){1to16}, %zmm6	 # AVX512F

	vprolvq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vprolvq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vprolvq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vprolvq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vprolvq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vprolvq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vprolvq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vprolvq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vprolvq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vprolvq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vprolvq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vprolvq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vprolvq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vprolvq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vprolq	$0xab, %zmm5, %zmm6	 # AVX512F
	vprolq	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vprolq	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vprolq	$123, %zmm5, %zmm6	 # AVX512F
	vprolq	$123, (%ecx), %zmm6	 # AVX512F
	vprolq	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vprolq	$123, (%eax){1to8}, %zmm6	 # AVX512F
	vprolq	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vprolq	$123, 8192(%edx), %zmm6	 # AVX512F
	vprolq	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vprolq	$123, -8256(%edx), %zmm6	 # AVX512F
	vprolq	$123, 1016(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vprolq	$123, 1024(%edx){1to8}, %zmm6	 # AVX512F
	vprolq	$123, -1024(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vprolq	$123, -1032(%edx){1to8}, %zmm6	 # AVX512F

	vprorvd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vprorvd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vprorvd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vprorvd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vprorvd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vprorvd	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vprorvd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vprorvd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vprorvd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vprorvd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vprorvd	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vprorvd	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vprorvd	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vprorvd	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vprord	$0xab, %zmm5, %zmm6	 # AVX512F
	vprord	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vprord	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vprord	$123, %zmm5, %zmm6	 # AVX512F
	vprord	$123, (%ecx), %zmm6	 # AVX512F
	vprord	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vprord	$123, (%eax){1to16}, %zmm6	 # AVX512F
	vprord	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vprord	$123, 8192(%edx), %zmm6	 # AVX512F
	vprord	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vprord	$123, -8256(%edx), %zmm6	 # AVX512F
	vprord	$123, 508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vprord	$123, 512(%edx){1to16}, %zmm6	 # AVX512F
	vprord	$123, -512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vprord	$123, -516(%edx){1to16}, %zmm6	 # AVX512F

	vprorvq	%zmm4, %zmm5, %zmm6	 # AVX512F
	vprorvq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vprorvq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vprorvq	(%ecx), %zmm5, %zmm6	 # AVX512F
	vprorvq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vprorvq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vprorvq	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vprorvq	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vprorvq	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vprorvq	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vprorvq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vprorvq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vprorvq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vprorvq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vprorq	$0xab, %zmm5, %zmm6	 # AVX512F
	vprorq	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vprorq	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vprorq	$123, %zmm5, %zmm6	 # AVX512F
	vprorq	$123, (%ecx), %zmm6	 # AVX512F
	vprorq	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vprorq	$123, (%eax){1to8}, %zmm6	 # AVX512F
	vprorq	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vprorq	$123, 8192(%edx), %zmm6	 # AVX512F
	vprorq	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vprorq	$123, -8256(%edx), %zmm6	 # AVX512F
	vprorq	$123, 1016(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vprorq	$123, 1024(%edx){1to8}, %zmm6	 # AVX512F
	vprorq	$123, -1024(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vprorq	$123, -1032(%edx){1to8}, %zmm6	 # AVX512F

	vrndscalepd	$0xab, %zmm5, %zmm6	 # AVX512F
	vrndscalepd	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vrndscalepd	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vrndscalepd	$0xab, {sae}, %zmm5, %zmm6	 # AVX512F
	vrndscalepd	$123, %zmm5, %zmm6	 # AVX512F
	vrndscalepd	$123, {sae}, %zmm5, %zmm6	 # AVX512F
	vrndscalepd	$123, (%ecx), %zmm6	 # AVX512F
	vrndscalepd	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vrndscalepd	$123, (%eax){1to8}, %zmm6	 # AVX512F
	vrndscalepd	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vrndscalepd	$123, 8192(%edx), %zmm6	 # AVX512F
	vrndscalepd	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vrndscalepd	$123, -8256(%edx), %zmm6	 # AVX512F
	vrndscalepd	$123, 1016(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vrndscalepd	$123, 1024(%edx){1to8}, %zmm6	 # AVX512F
	vrndscalepd	$123, -1024(%edx){1to8}, %zmm6	 # AVX512F Disp8
	vrndscalepd	$123, -1032(%edx){1to8}, %zmm6	 # AVX512F

	vrndscaleps	$0xab, %zmm5, %zmm6	 # AVX512F
	vrndscaleps	$0xab, %zmm5, %zmm6{%k7}	 # AVX512F
	vrndscaleps	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vrndscaleps	$0xab, {sae}, %zmm5, %zmm6	 # AVX512F
	vrndscaleps	$123, %zmm5, %zmm6	 # AVX512F
	vrndscaleps	$123, {sae}, %zmm5, %zmm6	 # AVX512F
	vrndscaleps	$123, (%ecx), %zmm6	 # AVX512F
	vrndscaleps	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512F
	vrndscaleps	$123, (%eax){1to16}, %zmm6	 # AVX512F
	vrndscaleps	$123, 8128(%edx), %zmm6	 # AVX512F Disp8
	vrndscaleps	$123, 8192(%edx), %zmm6	 # AVX512F
	vrndscaleps	$123, -8192(%edx), %zmm6	 # AVX512F Disp8
	vrndscaleps	$123, -8256(%edx), %zmm6	 # AVX512F
	vrndscaleps	$123, 508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vrndscaleps	$123, 512(%edx){1to16}, %zmm6	 # AVX512F
	vrndscaleps	$123, -512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vrndscaleps	$123, -516(%edx){1to16}, %zmm6	 # AVX512F

	vrndscalesd	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscalesd	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vrndscalesd	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscalesd	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscalesd	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscalesd	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscalesd	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscalesd	$123, 1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vrndscalesd	$123, 1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscalesd	$123, -1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vrndscalesd	$123, -1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vrndscaless	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscaless	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vrndscaless	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscaless	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscaless	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscaless	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscaless	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscaless	$123, 508(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vrndscaless	$123, 512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F
	vrndscaless	$123, -512(%edx), %xmm5, %xmm6{%k7}	 # AVX512F Disp8
	vrndscaless	$123, -516(%edx), %xmm5, %xmm6{%k7}	 # AVX512F

	vpcompressq	%zmm6, (%ecx)	 # AVX512F
	vpcompressq	%zmm6, (%ecx){%k7}	 # AVX512F
	vpcompressq	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpcompressq	%zmm6, 1016(%edx)	 # AVX512F Disp8
	vpcompressq	%zmm6, 1024(%edx)	 # AVX512F
	vpcompressq	%zmm6, -1024(%edx)	 # AVX512F Disp8
	vpcompressq	%zmm6, -1032(%edx)	 # AVX512F

	vpcompressq	%zmm5, %zmm6	 # AVX512F
	vpcompressq	%zmm5, %zmm6{%k7}	 # AVX512F
	vpcompressq	%zmm5, %zmm6{%k7}{z}	 # AVX512F

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
	kmovw	(%ecx), %k5	 # AVX512F
	kmovw	-123456(%esp,%esi,8), %k5	 # AVX512F

	kmovw	%k5, (%ecx)	 # AVX512F
	kmovw	%k5, -123456(%esp,%esi,8)	 # AVX512F

	kmovw	%eax, %k5	 # AVX512F
	kmovw	%ebp, %k5	 # AVX512F

	kmovw	%k5, %eax	 # AVX512F
	kmovw	%k5, %ebp	 # AVX512F

	kunpckbw	%k7, %k6, %k5	 # AVX512F

	vcvtps2ph	$0xab, %zmm6, (%ecx)	 # AVX512F
	vcvtps2ph	$0xab, %zmm6, (%ecx){%k7}	 # AVX512F
	vcvtps2ph	$123, %zmm6, (%ecx)	 # AVX512F
	vcvtps2ph	$123, %zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vcvtps2ph	$123, %zmm6, 4064(%edx)	 # AVX512F Disp8
	vcvtps2ph	$123, %zmm6, 4096(%edx)	 # AVX512F
	vcvtps2ph	$123, %zmm6, -4096(%edx)	 # AVX512F Disp8
	vcvtps2ph	$123, %zmm6, -4128(%edx)	 # AVX512F

	vextractf32x4	$0xab, %zmm6, (%ecx)	 # AVX512F
	vextractf32x4	$0xab, %zmm6, (%ecx){%k7}	 # AVX512F
	vextractf32x4	$123, %zmm6, (%ecx)	 # AVX512F
	vextractf32x4	$123, %zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vextractf32x4	$123, %zmm6, 2032(%edx)	 # AVX512F Disp8
	vextractf32x4	$123, %zmm6, 2048(%edx)	 # AVX512F
	vextractf32x4	$123, %zmm6, -2048(%edx)	 # AVX512F Disp8
	vextractf32x4	$123, %zmm6, -2064(%edx)	 # AVX512F

	vextractf64x4	$0xab, %zmm6, (%ecx)	 # AVX512F
	vextractf64x4	$0xab, %zmm6, (%ecx){%k7}	 # AVX512F
	vextractf64x4	$123, %zmm6, (%ecx)	 # AVX512F
	vextractf64x4	$123, %zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vextractf64x4	$123, %zmm6, 4064(%edx)	 # AVX512F Disp8
	vextractf64x4	$123, %zmm6, 4096(%edx)	 # AVX512F
	vextractf64x4	$123, %zmm6, -4096(%edx)	 # AVX512F Disp8
	vextractf64x4	$123, %zmm6, -4128(%edx)	 # AVX512F

	vextracti32x4	$0xab, %zmm6, (%ecx)	 # AVX512F
	vextracti32x4	$0xab, %zmm6, (%ecx){%k7}	 # AVX512F
	vextracti32x4	$123, %zmm6, (%ecx)	 # AVX512F
	vextracti32x4	$123, %zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vextracti32x4	$123, %zmm6, 2032(%edx)	 # AVX512F Disp8
	vextracti32x4	$123, %zmm6, 2048(%edx)	 # AVX512F
	vextracti32x4	$123, %zmm6, -2048(%edx)	 # AVX512F Disp8
	vextracti32x4	$123, %zmm6, -2064(%edx)	 # AVX512F

	vextracti64x4	$0xab, %zmm6, (%ecx)	 # AVX512F
	vextracti64x4	$0xab, %zmm6, (%ecx){%k7}	 # AVX512F
	vextracti64x4	$123, %zmm6, (%ecx)	 # AVX512F
	vextracti64x4	$123, %zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vextracti64x4	$123, %zmm6, 4064(%edx)	 # AVX512F Disp8
	vextracti64x4	$123, %zmm6, 4096(%edx)	 # AVX512F
	vextracti64x4	$123, %zmm6, -4096(%edx)	 # AVX512F Disp8
	vextracti64x4	$123, %zmm6, -4128(%edx)	 # AVX512F

	vmovapd	%zmm6, (%ecx)	 # AVX512F
	vmovapd	%zmm6, (%ecx){%k7}	 # AVX512F
	vmovapd	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vmovapd	%zmm6, 8128(%edx)	 # AVX512F Disp8
	vmovapd	%zmm6, 8192(%edx)	 # AVX512F
	vmovapd	%zmm6, -8192(%edx)	 # AVX512F Disp8
	vmovapd	%zmm6, -8256(%edx)	 # AVX512F

	vmovaps	%zmm6, (%ecx)	 # AVX512F
	vmovaps	%zmm6, (%ecx){%k7}	 # AVX512F
	vmovaps	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vmovaps	%zmm6, 8128(%edx)	 # AVX512F Disp8
	vmovaps	%zmm6, 8192(%edx)	 # AVX512F
	vmovaps	%zmm6, -8192(%edx)	 # AVX512F Disp8
	vmovaps	%zmm6, -8256(%edx)	 # AVX512F

	vmovdqa32	%zmm6, (%ecx)	 # AVX512F
	vmovdqa32	%zmm6, (%ecx){%k7}	 # AVX512F
	vmovdqa32	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vmovdqa32	%zmm6, 8128(%edx)	 # AVX512F Disp8
	vmovdqa32	%zmm6, 8192(%edx)	 # AVX512F
	vmovdqa32	%zmm6, -8192(%edx)	 # AVX512F Disp8
	vmovdqa32	%zmm6, -8256(%edx)	 # AVX512F

	vmovdqa64	%zmm6, (%ecx)	 # AVX512F
	vmovdqa64	%zmm6, (%ecx){%k7}	 # AVX512F
	vmovdqa64	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vmovdqa64	%zmm6, 8128(%edx)	 # AVX512F Disp8
	vmovdqa64	%zmm6, 8192(%edx)	 # AVX512F
	vmovdqa64	%zmm6, -8192(%edx)	 # AVX512F Disp8
	vmovdqa64	%zmm6, -8256(%edx)	 # AVX512F

	vmovdqu32	%zmm6, (%ecx)	 # AVX512F
	vmovdqu32	%zmm6, (%ecx){%k7}	 # AVX512F
	vmovdqu32	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vmovdqu32	%zmm6, 8128(%edx)	 # AVX512F Disp8
	vmovdqu32	%zmm6, 8192(%edx)	 # AVX512F
	vmovdqu32	%zmm6, -8192(%edx)	 # AVX512F Disp8
	vmovdqu32	%zmm6, -8256(%edx)	 # AVX512F

	vmovdqu64	%zmm6, (%ecx)	 # AVX512F
	vmovdqu64	%zmm6, (%ecx){%k7}	 # AVX512F
	vmovdqu64	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vmovdqu64	%zmm6, 8128(%edx)	 # AVX512F Disp8
	vmovdqu64	%zmm6, 8192(%edx)	 # AVX512F
	vmovdqu64	%zmm6, -8192(%edx)	 # AVX512F Disp8
	vmovdqu64	%zmm6, -8256(%edx)	 # AVX512F

	vmovupd	%zmm6, (%ecx)	 # AVX512F
	vmovupd	%zmm6, (%ecx){%k7}	 # AVX512F
	vmovupd	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vmovupd	%zmm6, 8128(%edx)	 # AVX512F Disp8
	vmovupd	%zmm6, 8192(%edx)	 # AVX512F
	vmovupd	%zmm6, -8192(%edx)	 # AVX512F Disp8
	vmovupd	%zmm6, -8256(%edx)	 # AVX512F

	vmovups	%zmm6, (%ecx)	 # AVX512F
	vmovups	%zmm6, (%ecx){%k7}	 # AVX512F
	vmovups	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vmovups	%zmm6, 8128(%edx)	 # AVX512F Disp8
	vmovups	%zmm6, 8192(%edx)	 # AVX512F
	vmovups	%zmm6, -8192(%edx)	 # AVX512F Disp8
	vmovups	%zmm6, -8256(%edx)	 # AVX512F

	vpmovqb	%zmm6, (%ecx)	 # AVX512F
	vpmovqb	%zmm6, (%ecx){%k7}	 # AVX512F
	vpmovqb	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpmovqb	%zmm6, 1016(%edx)	 # AVX512F Disp8
	vpmovqb	%zmm6, 1024(%edx)	 # AVX512F
	vpmovqb	%zmm6, -1024(%edx)	 # AVX512F Disp8
	vpmovqb	%zmm6, -1032(%edx)	 # AVX512F

	vpmovsqb	%zmm6, (%ecx)	 # AVX512F
	vpmovsqb	%zmm6, (%ecx){%k7}	 # AVX512F
	vpmovsqb	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpmovsqb	%zmm6, 1016(%edx)	 # AVX512F Disp8
	vpmovsqb	%zmm6, 1024(%edx)	 # AVX512F
	vpmovsqb	%zmm6, -1024(%edx)	 # AVX512F Disp8
	vpmovsqb	%zmm6, -1032(%edx)	 # AVX512F

	vpmovusqb	%zmm6, (%ecx)	 # AVX512F
	vpmovusqb	%zmm6, (%ecx){%k7}	 # AVX512F
	vpmovusqb	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpmovusqb	%zmm6, 1016(%edx)	 # AVX512F Disp8
	vpmovusqb	%zmm6, 1024(%edx)	 # AVX512F
	vpmovusqb	%zmm6, -1024(%edx)	 # AVX512F Disp8
	vpmovusqb	%zmm6, -1032(%edx)	 # AVX512F

	vpmovqw	%zmm6, (%ecx)	 # AVX512F
	vpmovqw	%zmm6, (%ecx){%k7}	 # AVX512F
	vpmovqw	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpmovqw	%zmm6, 2032(%edx)	 # AVX512F Disp8
	vpmovqw	%zmm6, 2048(%edx)	 # AVX512F
	vpmovqw	%zmm6, -2048(%edx)	 # AVX512F Disp8
	vpmovqw	%zmm6, -2064(%edx)	 # AVX512F

	vpmovsqw	%zmm6, (%ecx)	 # AVX512F
	vpmovsqw	%zmm6, (%ecx){%k7}	 # AVX512F
	vpmovsqw	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpmovsqw	%zmm6, 2032(%edx)	 # AVX512F Disp8
	vpmovsqw	%zmm6, 2048(%edx)	 # AVX512F
	vpmovsqw	%zmm6, -2048(%edx)	 # AVX512F Disp8
	vpmovsqw	%zmm6, -2064(%edx)	 # AVX512F

	vpmovusqw	%zmm6, (%ecx)	 # AVX512F
	vpmovusqw	%zmm6, (%ecx){%k7}	 # AVX512F
	vpmovusqw	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpmovusqw	%zmm6, 2032(%edx)	 # AVX512F Disp8
	vpmovusqw	%zmm6, 2048(%edx)	 # AVX512F
	vpmovusqw	%zmm6, -2048(%edx)	 # AVX512F Disp8
	vpmovusqw	%zmm6, -2064(%edx)	 # AVX512F

	vpmovqd	%zmm6, (%ecx)	 # AVX512F
	vpmovqd	%zmm6, (%ecx){%k7}	 # AVX512F
	vpmovqd	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpmovqd	%zmm6, 4064(%edx)	 # AVX512F Disp8
	vpmovqd	%zmm6, 4096(%edx)	 # AVX512F
	vpmovqd	%zmm6, -4096(%edx)	 # AVX512F Disp8
	vpmovqd	%zmm6, -4128(%edx)	 # AVX512F

	vpmovsqd	%zmm6, (%ecx)	 # AVX512F
	vpmovsqd	%zmm6, (%ecx){%k7}	 # AVX512F
	vpmovsqd	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpmovsqd	%zmm6, 4064(%edx)	 # AVX512F Disp8
	vpmovsqd	%zmm6, 4096(%edx)	 # AVX512F
	vpmovsqd	%zmm6, -4096(%edx)	 # AVX512F Disp8
	vpmovsqd	%zmm6, -4128(%edx)	 # AVX512F

	vpmovusqd	%zmm6, (%ecx)	 # AVX512F
	vpmovusqd	%zmm6, (%ecx){%k7}	 # AVX512F
	vpmovusqd	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpmovusqd	%zmm6, 4064(%edx)	 # AVX512F Disp8
	vpmovusqd	%zmm6, 4096(%edx)	 # AVX512F
	vpmovusqd	%zmm6, -4096(%edx)	 # AVX512F Disp8
	vpmovusqd	%zmm6, -4128(%edx)	 # AVX512F

	vpmovdb	%zmm6, (%ecx)	 # AVX512F
	vpmovdb	%zmm6, (%ecx){%k7}	 # AVX512F
	vpmovdb	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpmovdb	%zmm6, 2032(%edx)	 # AVX512F Disp8
	vpmovdb	%zmm6, 2048(%edx)	 # AVX512F
	vpmovdb	%zmm6, -2048(%edx)	 # AVX512F Disp8
	vpmovdb	%zmm6, -2064(%edx)	 # AVX512F

	vpmovsdb	%zmm6, (%ecx)	 # AVX512F
	vpmovsdb	%zmm6, (%ecx){%k7}	 # AVX512F
	vpmovsdb	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpmovsdb	%zmm6, 2032(%edx)	 # AVX512F Disp8
	vpmovsdb	%zmm6, 2048(%edx)	 # AVX512F
	vpmovsdb	%zmm6, -2048(%edx)	 # AVX512F Disp8
	vpmovsdb	%zmm6, -2064(%edx)	 # AVX512F

	vpmovusdb	%zmm6, (%ecx)	 # AVX512F
	vpmovusdb	%zmm6, (%ecx){%k7}	 # AVX512F
	vpmovusdb	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpmovusdb	%zmm6, 2032(%edx)	 # AVX512F Disp8
	vpmovusdb	%zmm6, 2048(%edx)	 # AVX512F
	vpmovusdb	%zmm6, -2048(%edx)	 # AVX512F Disp8
	vpmovusdb	%zmm6, -2064(%edx)	 # AVX512F

	vpmovdw	%zmm6, (%ecx)	 # AVX512F
	vpmovdw	%zmm6, (%ecx){%k7}	 # AVX512F
	vpmovdw	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpmovdw	%zmm6, 4064(%edx)	 # AVX512F Disp8
	vpmovdw	%zmm6, 4096(%edx)	 # AVX512F
	vpmovdw	%zmm6, -4096(%edx)	 # AVX512F Disp8
	vpmovdw	%zmm6, -4128(%edx)	 # AVX512F

	vpmovsdw	%zmm6, (%ecx)	 # AVX512F
	vpmovsdw	%zmm6, (%ecx){%k7}	 # AVX512F
	vpmovsdw	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpmovsdw	%zmm6, 4064(%edx)	 # AVX512F Disp8
	vpmovsdw	%zmm6, 4096(%edx)	 # AVX512F
	vpmovsdw	%zmm6, -4096(%edx)	 # AVX512F Disp8
	vpmovsdw	%zmm6, -4128(%edx)	 # AVX512F

	vpmovusdw	%zmm6, (%ecx)	 # AVX512F
	vpmovusdw	%zmm6, (%ecx){%k7}	 # AVX512F
	vpmovusdw	%zmm6, -123456(%esp,%esi,8)	 # AVX512F
	vpmovusdw	%zmm6, 4064(%edx)	 # AVX512F Disp8
	vpmovusdw	%zmm6, 4096(%edx)	 # AVX512F
	vpmovusdw	%zmm6, -4096(%edx)	 # AVX512F Disp8
	vpmovusdw	%zmm6, -4128(%edx)	 # AVX512F

	vcvttpd2udq	%zmm5, %ymm6{%k7}	 # AVX512F
	vcvttpd2udq	%zmm5, %ymm6{%k7}{z}	 # AVX512F
	vcvttpd2udq	{sae}, %zmm5, %ymm6{%k7}	 # AVX512F
	vcvttpd2udq	(%ecx), %ymm6{%k7}	 # AVX512F
	vcvttpd2udq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512F
	vcvttpd2udq	(%eax){1to8}, %ymm6{%k7}	 # AVX512F
	vcvttpd2udq	8128(%edx), %ymm6{%k7}	 # AVX512F Disp8
	vcvttpd2udq	8192(%edx), %ymm6{%k7}	 # AVX512F
	vcvttpd2udq	-8192(%edx), %ymm6{%k7}	 # AVX512F Disp8
	vcvttpd2udq	-8256(%edx), %ymm6{%k7}	 # AVX512F
	vcvttpd2udq	1016(%edx){1to8}, %ymm6{%k7}	 # AVX512F Disp8
	vcvttpd2udq	1024(%edx){1to8}, %ymm6{%k7}	 # AVX512F
	vcvttpd2udq	-1024(%edx){1to8}, %ymm6{%k7}	 # AVX512F Disp8
	vcvttpd2udq	-1032(%edx){1to8}, %ymm6{%k7}	 # AVX512F

	vcvttps2udq	%zmm5, %zmm6	 # AVX512F
	vcvttps2udq	%zmm5, %zmm6{%k7}	 # AVX512F
	vcvttps2udq	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vcvttps2udq	{sae}, %zmm5, %zmm6	 # AVX512F
	vcvttps2udq	(%ecx), %zmm6	 # AVX512F
	vcvttps2udq	-123456(%esp,%esi,8), %zmm6	 # AVX512F
	vcvttps2udq	(%eax){1to16}, %zmm6	 # AVX512F
	vcvttps2udq	8128(%edx), %zmm6	 # AVX512F Disp8
	vcvttps2udq	8192(%edx), %zmm6	 # AVX512F
	vcvttps2udq	-8192(%edx), %zmm6	 # AVX512F Disp8
	vcvttps2udq	-8256(%edx), %zmm6	 # AVX512F
	vcvttps2udq	508(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vcvttps2udq	512(%edx){1to16}, %zmm6	 # AVX512F
	vcvttps2udq	-512(%edx){1to16}, %zmm6	 # AVX512F Disp8
	vcvttps2udq	-516(%edx){1to16}, %zmm6	 # AVX512F

	vcvttsd2usi	%xmm6, %eax	 # AVX512F
	vcvttsd2usi	{sae}, %xmm6, %eax	 # AVX512F
	vcvttsd2usi	(%ecx), %eax	 # AVX512F
	vcvttsd2usi	-123456(%esp,%esi,8), %eax	 # AVX512F
	vcvttsd2usi	1016(%edx), %eax	 # AVX512F Disp8
	vcvttsd2usi	1024(%edx), %eax	 # AVX512F
	vcvttsd2usi	-1024(%edx), %eax	 # AVX512F Disp8
	vcvttsd2usi	-1032(%edx), %eax	 # AVX512F
	vcvttsd2usi	%xmm6, %ebp	 # AVX512F
	vcvttsd2usi	{sae}, %xmm6, %ebp	 # AVX512F
	vcvttsd2usi	(%ecx), %ebp	 # AVX512F
	vcvttsd2usi	-123456(%esp,%esi,8), %ebp	 # AVX512F
	vcvttsd2usi	1016(%edx), %ebp	 # AVX512F Disp8
	vcvttsd2usi	1024(%edx), %ebp	 # AVX512F
	vcvttsd2usi	-1024(%edx), %ebp	 # AVX512F Disp8
	vcvttsd2usi	-1032(%edx), %ebp	 # AVX512F

	vcvttss2usi	%xmm6, %eax	 # AVX512F
	vcvttss2usi	{sae}, %xmm6, %eax	 # AVX512F
	vcvttss2usi	(%ecx), %eax	 # AVX512F
	vcvttss2usi	-123456(%esp,%esi,8), %eax	 # AVX512F
	vcvttss2usi	508(%edx), %eax	 # AVX512F Disp8
	vcvttss2usi	512(%edx), %eax	 # AVX512F
	vcvttss2usi	-512(%edx), %eax	 # AVX512F Disp8
	vcvttss2usi	-516(%edx), %eax	 # AVX512F
	vcvttss2usi	%xmm6, %ebp	 # AVX512F
	vcvttss2usi	{sae}, %xmm6, %ebp	 # AVX512F
	vcvttss2usi	(%ecx), %ebp	 # AVX512F
	vcvttss2usi	-123456(%esp,%esi,8), %ebp	 # AVX512F
	vcvttss2usi	508(%edx), %ebp	 # AVX512F Disp8
	vcvttss2usi	512(%edx), %ebp	 # AVX512F
	vcvttss2usi	-512(%edx), %ebp	 # AVX512F Disp8
	vcvttss2usi	-516(%edx), %ebp	 # AVX512F

	vpermi2d	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpermi2d	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermi2d	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermi2d	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpermi2d	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpermi2d	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpermi2d	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2d	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpermi2d	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2d	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpermi2d	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2d	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpermi2d	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2d	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpermi2q	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpermi2q	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermi2q	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermi2q	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpermi2q	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpermi2q	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpermi2q	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2q	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpermi2q	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2q	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpermi2q	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2q	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpermi2q	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2q	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vpermi2ps	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpermi2ps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermi2ps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermi2ps	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpermi2ps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpermi2ps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512F
	vpermi2ps	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2ps	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpermi2ps	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2ps	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpermi2ps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2ps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F
	vpermi2ps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2ps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512F

	vpermi2pd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vpermi2pd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F
	vpermi2pd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F
	vpermi2pd	(%ecx), %zmm5, %zmm6	 # AVX512F
	vpermi2pd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F
	vpermi2pd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512F
	vpermi2pd	8128(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2pd	8192(%edx), %zmm5, %zmm6	 # AVX512F
	vpermi2pd	-8192(%edx), %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2pd	-8256(%edx), %zmm5, %zmm6	 # AVX512F
	vpermi2pd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2pd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F
	vpermi2pd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512F Disp8
	vpermi2pd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512F

	vptestnmd	%zmm4, %zmm5, %k5	 # AVX512F
	vptestnmd	%zmm4, %zmm5, %k5{%k7}	 # AVX512F
	vptestnmd	(%ecx), %zmm5, %k5	 # AVX512F
	vptestnmd	-123456(%esp,%esi,8), %zmm5, %k5	 # AVX512F
	vptestnmd	(%eax){1to16}, %zmm5, %k5	 # AVX512F
	vptestnmd	8128(%edx), %zmm5, %k5	 # AVX512F Disp8
	vptestnmd	8192(%edx), %zmm5, %k5	 # AVX512F
	vptestnmd	-8192(%edx), %zmm5, %k5	 # AVX512F Disp8
	vptestnmd	-8256(%edx), %zmm5, %k5	 # AVX512F
	vptestnmd	508(%edx){1to16}, %zmm5, %k5	 # AVX512F Disp8
	vptestnmd	512(%edx){1to16}, %zmm5, %k5	 # AVX512F
	vptestnmd	-512(%edx){1to16}, %zmm5, %k5	 # AVX512F Disp8
	vptestnmd	-516(%edx){1to16}, %zmm5, %k5	 # AVX512F

	vptestnmq	%zmm4, %zmm5, %k5	 # AVX512F
	vptestnmq	%zmm4, %zmm5, %k5{%k7}	 # AVX512F
	vptestnmq	(%ecx), %zmm5, %k5	 # AVX512F
	vptestnmq	-123456(%esp,%esi,8), %zmm5, %k5	 # AVX512F
	vptestnmq	(%eax){1to8}, %zmm5, %k5	 # AVX512F
	vptestnmq	8128(%edx), %zmm5, %k5	 # AVX512F Disp8
	vptestnmq	8192(%edx), %zmm5, %k5	 # AVX512F
	vptestnmq	-8192(%edx), %zmm5, %k5	 # AVX512F Disp8
	vptestnmq	-8256(%edx), %zmm5, %k5	 # AVX512F
	vptestnmq	1016(%edx){1to8}, %zmm5, %k5	 # AVX512F Disp8
	vptestnmq	1024(%edx){1to8}, %zmm5, %k5	 # AVX512F
	vptestnmq	-1024(%edx){1to8}, %zmm5, %k5	 # AVX512F Disp8
	vptestnmq	-1032(%edx){1to8}, %zmm5, %k5	 # AVX512F

	vaddps		(%bx), %zmm0, %zmm0
	vaddps		0x40(%bx), %zmm0, %zmm0
	vaddps		0x1234(%bx), %zmm0, %zmm0

	.intel_syntax noprefix
	vaddpd	zmm6, zmm5, zmm4	 # AVX512F
	vaddpd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vaddpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vaddpd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vaddpd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vaddpd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vaddpd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vaddpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vaddpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vaddpd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vaddpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vaddpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vaddpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vaddpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vaddpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vaddpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vaddpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vaddpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vaddps	zmm6, zmm5, zmm4	 # AVX512F
	vaddps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vaddps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vaddps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vaddps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vaddps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vaddps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vaddps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vaddps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vaddps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vaddps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vaddps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vaddps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vaddps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vaddps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vaddps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vaddps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vaddps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vaddsd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vaddsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vaddsd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vaddsd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vaddsd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vaddsd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vaddsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vaddsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vaddsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vaddsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vaddsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vaddsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vaddss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vaddss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vaddss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vaddss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vaddss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vaddss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vaddss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vaddss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vaddss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vaddss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vaddss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vaddss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	valignd	zmm6, zmm5, zmm4, 0xab	 # AVX512F
	valignd	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512F
	valignd	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512F
	valignd	zmm6, zmm5, zmm4, 123	 # AVX512F
	valignd	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512F
	valignd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	valignd	zmm6, zmm5, dword bcst [eax], 123	 # AVX512F
	valignd	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	valignd	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	valignd	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	valignd	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	valignd	zmm6, zmm5, dword bcst [edx+508], 123	 # AVX512F Disp8
	valignd	zmm6, zmm5, dword bcst [edx+512], 123	 # AVX512F
	valignd	zmm6, zmm5, dword bcst [edx-512], 123	 # AVX512F Disp8
	valignd	zmm6, zmm5, dword bcst [edx-516], 123	 # AVX512F

	vblendmpd	zmm6, zmm5, zmm4	 # AVX512F
	vblendmpd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vblendmpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vblendmpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vblendmpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vblendmpd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vblendmpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vblendmpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vblendmpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vblendmpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vblendmpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vblendmpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vblendmpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vblendmpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vblendmps	zmm6, zmm5, zmm4	 # AVX512F
	vblendmps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vblendmps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vblendmps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vblendmps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vblendmps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vblendmps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vblendmps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vblendmps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vblendmps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vblendmps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vblendmps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vblendmps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vblendmps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vbroadcastf32x4	zmm6, XMMWORD PTR [ecx]	 # AVX512F
	vbroadcastf32x4	zmm6{k7}, XMMWORD PTR [ecx]	 # AVX512F
	vbroadcastf32x4	zmm6{k7}{z}, XMMWORD PTR [ecx]	 # AVX512F
	vbroadcastf32x4	zmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vbroadcastf32x4	zmm6, XMMWORD PTR [edx+2032]	 # AVX512F Disp8
	vbroadcastf32x4	zmm6, XMMWORD PTR [edx+2048]	 # AVX512F
	vbroadcastf32x4	zmm6, XMMWORD PTR [edx-2048]	 # AVX512F Disp8
	vbroadcastf32x4	zmm6, XMMWORD PTR [edx-2064]	 # AVX512F

	vbroadcastf64x4	zmm6, YMMWORD PTR [ecx]	 # AVX512F
	vbroadcastf64x4	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512F
	vbroadcastf64x4	zmm6{k7}{z}, YMMWORD PTR [ecx]	 # AVX512F
	vbroadcastf64x4	zmm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vbroadcastf64x4	zmm6, YMMWORD PTR [edx+4064]	 # AVX512F Disp8
	vbroadcastf64x4	zmm6, YMMWORD PTR [edx+4096]	 # AVX512F
	vbroadcastf64x4	zmm6, YMMWORD PTR [edx-4096]	 # AVX512F Disp8
	vbroadcastf64x4	zmm6, YMMWORD PTR [edx-4128]	 # AVX512F

	vbroadcasti32x4	zmm6, XMMWORD PTR [ecx]	 # AVX512F
	vbroadcasti32x4	zmm6{k7}, XMMWORD PTR [ecx]	 # AVX512F
	vbroadcasti32x4	zmm6{k7}{z}, XMMWORD PTR [ecx]	 # AVX512F
	vbroadcasti32x4	zmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vbroadcasti32x4	zmm6, XMMWORD PTR [edx+2032]	 # AVX512F Disp8
	vbroadcasti32x4	zmm6, XMMWORD PTR [edx+2048]	 # AVX512F
	vbroadcasti32x4	zmm6, XMMWORD PTR [edx-2048]	 # AVX512F Disp8
	vbroadcasti32x4	zmm6, XMMWORD PTR [edx-2064]	 # AVX512F

	vbroadcasti64x4	zmm6, YMMWORD PTR [ecx]	 # AVX512F
	vbroadcasti64x4	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512F
	vbroadcasti64x4	zmm6{k7}{z}, YMMWORD PTR [ecx]	 # AVX512F
	vbroadcasti64x4	zmm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vbroadcasti64x4	zmm6, YMMWORD PTR [edx+4064]	 # AVX512F Disp8
	vbroadcasti64x4	zmm6, YMMWORD PTR [edx+4096]	 # AVX512F
	vbroadcasti64x4	zmm6, YMMWORD PTR [edx-4096]	 # AVX512F Disp8
	vbroadcasti64x4	zmm6, YMMWORD PTR [edx-4128]	 # AVX512F

	vbroadcastsd	zmm6, QWORD PTR [ecx]	 # AVX512F
	vbroadcastsd	zmm6{k7}, QWORD PTR [ecx]	 # AVX512F
	vbroadcastsd	zmm6{k7}{z}, QWORD PTR [ecx]	 # AVX512F
	vbroadcastsd	zmm6, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vbroadcastsd	zmm6, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vbroadcastsd	zmm6, QWORD PTR [edx+1024]	 # AVX512F
	vbroadcastsd	zmm6, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vbroadcastsd	zmm6, QWORD PTR [edx-1032]	 # AVX512F

	vbroadcastsd	zmm6{k7}, xmm5	 # AVX512F
	vbroadcastsd	zmm6{k7}{z}, xmm5	 # AVX512F

	vbroadcastss	zmm6, DWORD PTR [ecx]	 # AVX512F
	vbroadcastss	zmm6{k7}, DWORD PTR [ecx]	 # AVX512F
	vbroadcastss	zmm6{k7}{z}, DWORD PTR [ecx]	 # AVX512F
	vbroadcastss	zmm6, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vbroadcastss	zmm6, DWORD PTR [edx+508]	 # AVX512F Disp8
	vbroadcastss	zmm6, DWORD PTR [edx+512]	 # AVX512F
	vbroadcastss	zmm6, DWORD PTR [edx-512]	 # AVX512F Disp8
	vbroadcastss	zmm6, DWORD PTR [edx-516]	 # AVX512F

	vbroadcastss	zmm6{k7}, xmm5	 # AVX512F
	vbroadcastss	zmm6{k7}{z}, xmm5	 # AVX512F

	vcmppd	k5, zmm6, zmm5, 0xab	 # AVX512F
	vcmppd	k5{k7}, zmm6, zmm5, 0xab	 # AVX512F
	vcmppd	k5, zmm6, zmm5{sae}, 0xab	 # AVX512F
	vcmppd	k5, zmm6, zmm5, 123	 # AVX512F
	vcmppd	k5, zmm6, zmm5{sae}, 123	 # AVX512F
	vcmppd	k5, zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vcmppd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vcmppd	k5, zmm6, qword bcst [eax], 123	 # AVX512F
	vcmppd	k5, zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vcmppd	k5, zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vcmppd	k5, zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vcmppd	k5, zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vcmppd	k5, zmm6, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vcmppd	k5, zmm6, qword bcst [edx+1024], 123	 # AVX512F
	vcmppd	k5, zmm6, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vcmppd	k5, zmm6, qword bcst [edx-1032], 123	 # AVX512F

	vcmpeq_oqpd	k5, zmm6, zmm5	 # AVX512F
	vcmpeq_oqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpeq_oqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpeq_oqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpeq_oqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_oqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpeq_oqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpeq_oqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpeq_oqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpeq_oqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpeq_oqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpeq_oqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpeq_oqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpeq_oqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpeqpd	k5, zmm6, zmm5	 # AVX512F
	vcmpeqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpeqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpeqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpeqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpeqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpeqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpeqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpeqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpeqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpeqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpeqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpeqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmplt_ospd	k5, zmm6, zmm5	 # AVX512F
	vcmplt_ospd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmplt_ospd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmplt_ospd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmplt_ospd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmplt_ospd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmplt_ospd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmplt_ospd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmplt_ospd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmplt_ospd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmplt_ospd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmplt_ospd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmplt_ospd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmplt_ospd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpltpd	k5, zmm6, zmm5	 # AVX512F
	vcmpltpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpltpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpltpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpltpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpltpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpltpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpltpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpltpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpltpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpltpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpltpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpltpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpltpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmple_ospd	k5, zmm6, zmm5	 # AVX512F
	vcmple_ospd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmple_ospd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmple_ospd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmple_ospd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmple_ospd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmple_ospd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmple_ospd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmple_ospd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmple_ospd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmple_ospd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmple_ospd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmple_ospd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmple_ospd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmplepd	k5, zmm6, zmm5	 # AVX512F
	vcmplepd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmplepd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmplepd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmplepd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmplepd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmplepd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmplepd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmplepd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmplepd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmplepd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmplepd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmplepd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmplepd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpunord_qpd	k5, zmm6, zmm5	 # AVX512F
	vcmpunord_qpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpunord_qpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpunord_qpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpunord_qpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpunord_qpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpunord_qpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpunord_qpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpunord_qpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpunord_qpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpunord_qpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpunord_qpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpunord_qpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpunord_qpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpunordpd	k5, zmm6, zmm5	 # AVX512F
	vcmpunordpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpunordpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpunordpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpunordpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpunordpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpunordpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpunordpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpunordpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpunordpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpunordpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpunordpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpunordpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpunordpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpneq_uqpd	k5, zmm6, zmm5	 # AVX512F
	vcmpneq_uqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpneq_uqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpneq_uqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpneq_uqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_uqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpneq_uqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpneq_uqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpneq_uqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpneq_uqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpneq_uqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpneq_uqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpneq_uqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpneq_uqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpneqpd	k5, zmm6, zmm5	 # AVX512F
	vcmpneqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpneqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpneqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpneqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpneqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpneqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpneqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpneqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpneqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpneqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpneqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpneqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpnlt_uspd	k5, zmm6, zmm5	 # AVX512F
	vcmpnlt_uspd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnlt_uspd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnlt_uspd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnlt_uspd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnlt_uspd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpnlt_uspd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnlt_uspd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnlt_uspd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnlt_uspd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnlt_uspd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpnlt_uspd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpnlt_uspd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpnlt_uspd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpnltpd	k5, zmm6, zmm5	 # AVX512F
	vcmpnltpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnltpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnltpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnltpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnltpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpnltpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnltpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnltpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnltpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnltpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpnltpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpnltpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpnltpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpnle_uspd	k5, zmm6, zmm5	 # AVX512F
	vcmpnle_uspd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnle_uspd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnle_uspd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnle_uspd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnle_uspd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpnle_uspd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnle_uspd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnle_uspd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnle_uspd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnle_uspd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpnle_uspd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpnle_uspd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpnle_uspd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpnlepd	k5, zmm6, zmm5	 # AVX512F
	vcmpnlepd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnlepd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnlepd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnlepd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnlepd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpnlepd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnlepd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnlepd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnlepd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnlepd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpnlepd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpnlepd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpnlepd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpord_qpd	k5, zmm6, zmm5	 # AVX512F
	vcmpord_qpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpord_qpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpord_qpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpord_qpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpord_qpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpord_qpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpord_qpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpord_qpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpord_qpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpord_qpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpord_qpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpord_qpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpord_qpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpordpd	k5, zmm6, zmm5	 # AVX512F
	vcmpordpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpordpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpordpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpordpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpordpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpordpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpordpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpordpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpordpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpordpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpordpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpordpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpordpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpeq_uqpd	k5, zmm6, zmm5	 # AVX512F
	vcmpeq_uqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpeq_uqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpeq_uqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpeq_uqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_uqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpeq_uqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpeq_uqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpeq_uqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpeq_uqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpeq_uqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpeq_uqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpeq_uqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpeq_uqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpnge_uspd	k5, zmm6, zmm5	 # AVX512F
	vcmpnge_uspd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnge_uspd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnge_uspd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnge_uspd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnge_uspd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpnge_uspd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnge_uspd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnge_uspd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnge_uspd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnge_uspd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpnge_uspd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpnge_uspd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpnge_uspd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpngepd	k5, zmm6, zmm5	 # AVX512F
	vcmpngepd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpngepd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpngepd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpngepd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngepd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpngepd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpngepd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpngepd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpngepd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpngepd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpngepd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpngepd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpngepd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpngt_uspd	k5, zmm6, zmm5	 # AVX512F
	vcmpngt_uspd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpngt_uspd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpngt_uspd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpngt_uspd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngt_uspd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpngt_uspd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpngt_uspd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpngt_uspd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpngt_uspd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpngt_uspd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpngt_uspd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpngt_uspd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpngt_uspd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpngtpd	k5, zmm6, zmm5	 # AVX512F
	vcmpngtpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpngtpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpngtpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpngtpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngtpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpngtpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpngtpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpngtpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpngtpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpngtpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpngtpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpngtpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpngtpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpfalse_oqpd	k5, zmm6, zmm5	 # AVX512F
	vcmpfalse_oqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpfalse_oqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpfalse_oqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpfalse_oqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpfalse_oqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpfalse_oqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpfalse_oqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpfalse_oqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpfalse_oqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpfalse_oqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpfalse_oqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpfalse_oqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpfalse_oqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpfalsepd	k5, zmm6, zmm5	 # AVX512F
	vcmpfalsepd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpfalsepd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpfalsepd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpfalsepd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpfalsepd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpfalsepd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpfalsepd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpfalsepd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpfalsepd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpfalsepd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpfalsepd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpfalsepd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpfalsepd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpneq_oqpd	k5, zmm6, zmm5	 # AVX512F
	vcmpneq_oqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpneq_oqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpneq_oqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpneq_oqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_oqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpneq_oqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpneq_oqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpneq_oqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpneq_oqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpneq_oqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpneq_oqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpneq_oqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpneq_oqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpge_ospd	k5, zmm6, zmm5	 # AVX512F
	vcmpge_ospd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpge_ospd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpge_ospd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpge_ospd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpge_ospd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpge_ospd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpge_ospd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpge_ospd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpge_ospd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpge_ospd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpge_ospd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpge_ospd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpge_ospd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpgepd	k5, zmm6, zmm5	 # AVX512F
	vcmpgepd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpgepd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpgepd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpgepd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgepd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpgepd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpgepd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpgepd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpgepd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpgepd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpgepd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpgepd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpgepd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpgt_ospd	k5, zmm6, zmm5	 # AVX512F
	vcmpgt_ospd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpgt_ospd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpgt_ospd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpgt_ospd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgt_ospd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpgt_ospd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpgt_ospd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpgt_ospd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpgt_ospd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpgt_ospd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpgt_ospd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpgt_ospd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpgt_ospd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpgtpd	k5, zmm6, zmm5	 # AVX512F
	vcmpgtpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpgtpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpgtpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpgtpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgtpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpgtpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpgtpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpgtpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpgtpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpgtpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpgtpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpgtpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpgtpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmptrue_uqpd	k5, zmm6, zmm5	 # AVX512F
	vcmptrue_uqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmptrue_uqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmptrue_uqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmptrue_uqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmptrue_uqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmptrue_uqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmptrue_uqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmptrue_uqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmptrue_uqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmptrue_uqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmptrue_uqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmptrue_uqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmptrue_uqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmptruepd	k5, zmm6, zmm5	 # AVX512F
	vcmptruepd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmptruepd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmptruepd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmptruepd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmptruepd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmptruepd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmptruepd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmptruepd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmptruepd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmptruepd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmptruepd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmptruepd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmptruepd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpeq_ospd	k5, zmm6, zmm5	 # AVX512F
	vcmpeq_ospd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpeq_ospd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpeq_ospd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpeq_ospd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_ospd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpeq_ospd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpeq_ospd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpeq_ospd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpeq_ospd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpeq_ospd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpeq_ospd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpeq_ospd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpeq_ospd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmplt_oqpd	k5, zmm6, zmm5	 # AVX512F
	vcmplt_oqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmplt_oqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmplt_oqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmplt_oqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmplt_oqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmplt_oqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmplt_oqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmplt_oqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmplt_oqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmplt_oqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmplt_oqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmplt_oqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmplt_oqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmple_oqpd	k5, zmm6, zmm5	 # AVX512F
	vcmple_oqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmple_oqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmple_oqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmple_oqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmple_oqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmple_oqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmple_oqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmple_oqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmple_oqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmple_oqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmple_oqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmple_oqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmple_oqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpunord_spd	k5, zmm6, zmm5	 # AVX512F
	vcmpunord_spd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpunord_spd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpunord_spd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpunord_spd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpunord_spd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpunord_spd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpunord_spd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpunord_spd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpunord_spd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpunord_spd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpunord_spd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpunord_spd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpunord_spd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpneq_uspd	k5, zmm6, zmm5	 # AVX512F
	vcmpneq_uspd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpneq_uspd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpneq_uspd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpneq_uspd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_uspd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpneq_uspd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpneq_uspd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpneq_uspd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpneq_uspd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpneq_uspd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpneq_uspd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpneq_uspd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpneq_uspd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpnlt_uqpd	k5, zmm6, zmm5	 # AVX512F
	vcmpnlt_uqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnlt_uqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnlt_uqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnlt_uqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnlt_uqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpnlt_uqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnlt_uqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnlt_uqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnlt_uqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnlt_uqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpnlt_uqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpnlt_uqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpnlt_uqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpnle_uqpd	k5, zmm6, zmm5	 # AVX512F
	vcmpnle_uqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnle_uqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnle_uqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnle_uqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnle_uqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpnle_uqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnle_uqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnle_uqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnle_uqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnle_uqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpnle_uqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpnle_uqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpnle_uqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpord_spd	k5, zmm6, zmm5	 # AVX512F
	vcmpord_spd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpord_spd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpord_spd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpord_spd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpord_spd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpord_spd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpord_spd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpord_spd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpord_spd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpord_spd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpord_spd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpord_spd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpord_spd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpeq_uspd	k5, zmm6, zmm5	 # AVX512F
	vcmpeq_uspd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpeq_uspd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpeq_uspd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpeq_uspd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_uspd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpeq_uspd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpeq_uspd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpeq_uspd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpeq_uspd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpeq_uspd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpeq_uspd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpeq_uspd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpeq_uspd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpnge_uqpd	k5, zmm6, zmm5	 # AVX512F
	vcmpnge_uqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnge_uqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnge_uqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnge_uqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnge_uqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpnge_uqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnge_uqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnge_uqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnge_uqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnge_uqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpnge_uqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpnge_uqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpnge_uqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpngt_uqpd	k5, zmm6, zmm5	 # AVX512F
	vcmpngt_uqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpngt_uqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpngt_uqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpngt_uqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngt_uqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpngt_uqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpngt_uqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpngt_uqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpngt_uqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpngt_uqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpngt_uqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpngt_uqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpngt_uqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpfalse_ospd	k5, zmm6, zmm5	 # AVX512F
	vcmpfalse_ospd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpfalse_ospd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpfalse_ospd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpfalse_ospd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpfalse_ospd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpfalse_ospd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpfalse_ospd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpfalse_ospd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpfalse_ospd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpfalse_ospd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpfalse_ospd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpfalse_ospd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpfalse_ospd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpneq_ospd	k5, zmm6, zmm5	 # AVX512F
	vcmpneq_ospd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpneq_ospd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpneq_ospd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpneq_ospd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_ospd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpneq_ospd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpneq_ospd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpneq_ospd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpneq_ospd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpneq_ospd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpneq_ospd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpneq_ospd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpneq_ospd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpge_oqpd	k5, zmm6, zmm5	 # AVX512F
	vcmpge_oqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpge_oqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpge_oqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpge_oqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpge_oqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpge_oqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpge_oqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpge_oqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpge_oqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpge_oqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpge_oqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpge_oqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpge_oqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpgt_oqpd	k5, zmm6, zmm5	 # AVX512F
	vcmpgt_oqpd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpgt_oqpd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpgt_oqpd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpgt_oqpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgt_oqpd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmpgt_oqpd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpgt_oqpd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpgt_oqpd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpgt_oqpd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpgt_oqpd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmpgt_oqpd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmpgt_oqpd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmpgt_oqpd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmptrue_uspd	k5, zmm6, zmm5	 # AVX512F
	vcmptrue_uspd	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmptrue_uspd	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmptrue_uspd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmptrue_uspd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmptrue_uspd	k5, zmm6, qword bcst [eax]	 # AVX512F
	vcmptrue_uspd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmptrue_uspd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmptrue_uspd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmptrue_uspd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmptrue_uspd	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vcmptrue_uspd	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vcmptrue_uspd	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vcmptrue_uspd	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vcmpps	k5, zmm6, zmm5, 0xab	 # AVX512F
	vcmpps	k5{k7}, zmm6, zmm5, 0xab	 # AVX512F
	vcmpps	k5, zmm6, zmm5{sae}, 0xab	 # AVX512F
	vcmpps	k5, zmm6, zmm5, 123	 # AVX512F
	vcmpps	k5, zmm6, zmm5{sae}, 123	 # AVX512F
	vcmpps	k5, zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vcmpps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vcmpps	k5, zmm6, dword bcst [eax], 123	 # AVX512F
	vcmpps	k5, zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vcmpps	k5, zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vcmpps	k5, zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vcmpps	k5, zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vcmpps	k5, zmm6, dword bcst [edx+508], 123	 # AVX512F Disp8
	vcmpps	k5, zmm6, dword bcst [edx+512], 123	 # AVX512F
	vcmpps	k5, zmm6, dword bcst [edx-512], 123	 # AVX512F Disp8
	vcmpps	k5, zmm6, dword bcst [edx-516], 123	 # AVX512F

	vcmpeq_oqps	k5, zmm6, zmm5	 # AVX512F
	vcmpeq_oqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpeq_oqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpeq_oqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpeq_oqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_oqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpeq_oqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpeq_oqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpeq_oqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpeq_oqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpeq_oqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpeq_oqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpeq_oqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpeq_oqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpeqps	k5, zmm6, zmm5	 # AVX512F
	vcmpeqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpeqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpeqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpeqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpeqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpeqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpeqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpeqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpeqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpeqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpeqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpeqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmplt_osps	k5, zmm6, zmm5	 # AVX512F
	vcmplt_osps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmplt_osps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmplt_osps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmplt_osps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmplt_osps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmplt_osps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmplt_osps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmplt_osps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmplt_osps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmplt_osps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmplt_osps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmplt_osps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmplt_osps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpltps	k5, zmm6, zmm5	 # AVX512F
	vcmpltps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpltps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpltps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpltps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpltps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpltps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpltps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpltps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpltps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpltps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpltps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpltps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpltps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmple_osps	k5, zmm6, zmm5	 # AVX512F
	vcmple_osps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmple_osps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmple_osps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmple_osps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmple_osps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmple_osps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmple_osps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmple_osps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmple_osps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmple_osps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmple_osps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmple_osps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmple_osps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpleps	k5, zmm6, zmm5	 # AVX512F
	vcmpleps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpleps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpleps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpleps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpleps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpleps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpleps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpleps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpleps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpleps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpleps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpleps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpleps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpunord_qps	k5, zmm6, zmm5	 # AVX512F
	vcmpunord_qps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpunord_qps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpunord_qps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpunord_qps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpunord_qps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpunord_qps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpunord_qps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpunord_qps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpunord_qps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpunord_qps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpunord_qps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpunord_qps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpunord_qps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpunordps	k5, zmm6, zmm5	 # AVX512F
	vcmpunordps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpunordps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpunordps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpunordps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpunordps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpunordps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpunordps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpunordps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpunordps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpunordps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpunordps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpunordps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpunordps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpneq_uqps	k5, zmm6, zmm5	 # AVX512F
	vcmpneq_uqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpneq_uqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpneq_uqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpneq_uqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_uqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpneq_uqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpneq_uqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpneq_uqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpneq_uqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpneq_uqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpneq_uqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpneq_uqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpneq_uqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpneqps	k5, zmm6, zmm5	 # AVX512F
	vcmpneqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpneqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpneqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpneqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpneqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpneqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpneqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpneqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpneqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpneqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpneqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpneqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpnlt_usps	k5, zmm6, zmm5	 # AVX512F
	vcmpnlt_usps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnlt_usps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnlt_usps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnlt_usps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnlt_usps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpnlt_usps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnlt_usps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnlt_usps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnlt_usps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnlt_usps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpnlt_usps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpnlt_usps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpnlt_usps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpnltps	k5, zmm6, zmm5	 # AVX512F
	vcmpnltps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnltps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnltps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnltps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnltps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpnltps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnltps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnltps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnltps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnltps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpnltps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpnltps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpnltps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpnle_usps	k5, zmm6, zmm5	 # AVX512F
	vcmpnle_usps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnle_usps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnle_usps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnle_usps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnle_usps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpnle_usps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnle_usps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnle_usps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnle_usps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnle_usps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpnle_usps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpnle_usps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpnle_usps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpnleps	k5, zmm6, zmm5	 # AVX512F
	vcmpnleps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnleps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnleps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnleps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnleps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpnleps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnleps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnleps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnleps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnleps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpnleps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpnleps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpnleps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpord_qps	k5, zmm6, zmm5	 # AVX512F
	vcmpord_qps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpord_qps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpord_qps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpord_qps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpord_qps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpord_qps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpord_qps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpord_qps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpord_qps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpord_qps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpord_qps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpord_qps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpord_qps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpordps	k5, zmm6, zmm5	 # AVX512F
	vcmpordps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpordps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpordps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpordps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpordps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpordps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpordps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpordps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpordps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpordps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpordps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpordps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpordps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpeq_uqps	k5, zmm6, zmm5	 # AVX512F
	vcmpeq_uqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpeq_uqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpeq_uqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpeq_uqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_uqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpeq_uqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpeq_uqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpeq_uqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpeq_uqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpeq_uqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpeq_uqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpeq_uqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpeq_uqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpnge_usps	k5, zmm6, zmm5	 # AVX512F
	vcmpnge_usps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnge_usps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnge_usps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnge_usps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnge_usps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpnge_usps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnge_usps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnge_usps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnge_usps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnge_usps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpnge_usps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpnge_usps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpnge_usps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpngeps	k5, zmm6, zmm5	 # AVX512F
	vcmpngeps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpngeps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpngeps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpngeps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngeps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpngeps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpngeps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpngeps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpngeps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpngeps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpngeps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpngeps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpngeps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpngt_usps	k5, zmm6, zmm5	 # AVX512F
	vcmpngt_usps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpngt_usps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpngt_usps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpngt_usps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngt_usps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpngt_usps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpngt_usps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpngt_usps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpngt_usps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpngt_usps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpngt_usps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpngt_usps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpngt_usps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpngtps	k5, zmm6, zmm5	 # AVX512F
	vcmpngtps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpngtps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpngtps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpngtps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngtps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpngtps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpngtps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpngtps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpngtps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpngtps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpngtps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpngtps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpngtps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpfalse_oqps	k5, zmm6, zmm5	 # AVX512F
	vcmpfalse_oqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpfalse_oqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpfalse_oqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpfalse_oqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpfalse_oqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpfalse_oqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpfalse_oqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpfalse_oqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpfalse_oqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpfalse_oqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpfalse_oqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpfalse_oqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpfalse_oqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpfalseps	k5, zmm6, zmm5	 # AVX512F
	vcmpfalseps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpfalseps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpfalseps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpfalseps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpfalseps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpfalseps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpfalseps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpfalseps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpfalseps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpfalseps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpfalseps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpfalseps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpfalseps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpneq_oqps	k5, zmm6, zmm5	 # AVX512F
	vcmpneq_oqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpneq_oqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpneq_oqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpneq_oqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_oqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpneq_oqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpneq_oqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpneq_oqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpneq_oqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpneq_oqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpneq_oqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpneq_oqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpneq_oqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpge_osps	k5, zmm6, zmm5	 # AVX512F
	vcmpge_osps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpge_osps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpge_osps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpge_osps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpge_osps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpge_osps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpge_osps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpge_osps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpge_osps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpge_osps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpge_osps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpge_osps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpge_osps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpgeps	k5, zmm6, zmm5	 # AVX512F
	vcmpgeps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpgeps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpgeps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpgeps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgeps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpgeps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpgeps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpgeps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpgeps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpgeps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpgeps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpgeps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpgeps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpgt_osps	k5, zmm6, zmm5	 # AVX512F
	vcmpgt_osps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpgt_osps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpgt_osps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpgt_osps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgt_osps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpgt_osps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpgt_osps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpgt_osps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpgt_osps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpgt_osps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpgt_osps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpgt_osps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpgt_osps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpgtps	k5, zmm6, zmm5	 # AVX512F
	vcmpgtps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpgtps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpgtps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpgtps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgtps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpgtps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpgtps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpgtps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpgtps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpgtps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpgtps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpgtps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpgtps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmptrue_uqps	k5, zmm6, zmm5	 # AVX512F
	vcmptrue_uqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmptrue_uqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmptrue_uqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmptrue_uqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmptrue_uqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmptrue_uqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmptrue_uqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmptrue_uqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmptrue_uqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmptrue_uqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmptrue_uqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmptrue_uqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmptrue_uqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmptrueps	k5, zmm6, zmm5	 # AVX512F
	vcmptrueps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmptrueps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmptrueps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmptrueps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmptrueps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmptrueps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmptrueps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmptrueps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmptrueps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmptrueps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmptrueps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmptrueps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmptrueps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpeq_osps	k5, zmm6, zmm5	 # AVX512F
	vcmpeq_osps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpeq_osps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpeq_osps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpeq_osps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_osps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpeq_osps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpeq_osps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpeq_osps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpeq_osps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpeq_osps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpeq_osps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpeq_osps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpeq_osps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmplt_oqps	k5, zmm6, zmm5	 # AVX512F
	vcmplt_oqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmplt_oqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmplt_oqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmplt_oqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmplt_oqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmplt_oqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmplt_oqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmplt_oqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmplt_oqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmplt_oqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmplt_oqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmplt_oqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmplt_oqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmple_oqps	k5, zmm6, zmm5	 # AVX512F
	vcmple_oqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmple_oqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmple_oqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmple_oqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmple_oqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmple_oqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmple_oqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmple_oqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmple_oqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmple_oqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmple_oqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmple_oqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmple_oqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpunord_sps	k5, zmm6, zmm5	 # AVX512F
	vcmpunord_sps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpunord_sps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpunord_sps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpunord_sps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpunord_sps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpunord_sps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpunord_sps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpunord_sps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpunord_sps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpunord_sps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpunord_sps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpunord_sps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpunord_sps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpneq_usps	k5, zmm6, zmm5	 # AVX512F
	vcmpneq_usps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpneq_usps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpneq_usps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpneq_usps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_usps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpneq_usps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpneq_usps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpneq_usps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpneq_usps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpneq_usps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpneq_usps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpneq_usps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpneq_usps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpnlt_uqps	k5, zmm6, zmm5	 # AVX512F
	vcmpnlt_uqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnlt_uqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnlt_uqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnlt_uqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnlt_uqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpnlt_uqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnlt_uqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnlt_uqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnlt_uqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnlt_uqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpnlt_uqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpnlt_uqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpnlt_uqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpnle_uqps	k5, zmm6, zmm5	 # AVX512F
	vcmpnle_uqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnle_uqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnle_uqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnle_uqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnle_uqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpnle_uqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnle_uqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnle_uqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnle_uqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnle_uqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpnle_uqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpnle_uqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpnle_uqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpord_sps	k5, zmm6, zmm5	 # AVX512F
	vcmpord_sps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpord_sps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpord_sps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpord_sps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpord_sps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpord_sps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpord_sps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpord_sps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpord_sps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpord_sps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpord_sps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpord_sps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpord_sps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpeq_usps	k5, zmm6, zmm5	 # AVX512F
	vcmpeq_usps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpeq_usps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpeq_usps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpeq_usps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_usps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpeq_usps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpeq_usps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpeq_usps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpeq_usps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpeq_usps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpeq_usps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpeq_usps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpeq_usps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpnge_uqps	k5, zmm6, zmm5	 # AVX512F
	vcmpnge_uqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpnge_uqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpnge_uqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpnge_uqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnge_uqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpnge_uqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpnge_uqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpnge_uqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpnge_uqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpnge_uqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpnge_uqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpnge_uqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpnge_uqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpngt_uqps	k5, zmm6, zmm5	 # AVX512F
	vcmpngt_uqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpngt_uqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpngt_uqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpngt_uqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngt_uqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpngt_uqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpngt_uqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpngt_uqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpngt_uqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpngt_uqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpngt_uqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpngt_uqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpngt_uqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpfalse_osps	k5, zmm6, zmm5	 # AVX512F
	vcmpfalse_osps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpfalse_osps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpfalse_osps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpfalse_osps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpfalse_osps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpfalse_osps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpfalse_osps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpfalse_osps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpfalse_osps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpfalse_osps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpfalse_osps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpfalse_osps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpfalse_osps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpneq_osps	k5, zmm6, zmm5	 # AVX512F
	vcmpneq_osps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpneq_osps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpneq_osps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpneq_osps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_osps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpneq_osps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpneq_osps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpneq_osps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpneq_osps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpneq_osps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpneq_osps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpneq_osps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpneq_osps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpge_oqps	k5, zmm6, zmm5	 # AVX512F
	vcmpge_oqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpge_oqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpge_oqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpge_oqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpge_oqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpge_oqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpge_oqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpge_oqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpge_oqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpge_oqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpge_oqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpge_oqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpge_oqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpgt_oqps	k5, zmm6, zmm5	 # AVX512F
	vcmpgt_oqps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmpgt_oqps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmpgt_oqps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmpgt_oqps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgt_oqps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmpgt_oqps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmpgt_oqps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmpgt_oqps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmpgt_oqps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmpgt_oqps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmpgt_oqps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmpgt_oqps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmpgt_oqps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmptrue_usps	k5, zmm6, zmm5	 # AVX512F
	vcmptrue_usps	k5{k7}, zmm6, zmm5	 # AVX512F
	vcmptrue_usps	k5, zmm6, zmm5{sae}	 # AVX512F
	vcmptrue_usps	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcmptrue_usps	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmptrue_usps	k5, zmm6, dword bcst [eax]	 # AVX512F
	vcmptrue_usps	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcmptrue_usps	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcmptrue_usps	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcmptrue_usps	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcmptrue_usps	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcmptrue_usps	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vcmptrue_usps	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcmptrue_usps	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vcmpsd	k5{k7}, xmm5, xmm4, 0xab	 # AVX512F
	vcmpsd	k5{k7}, xmm5, xmm4{sae}, 0xab	 # AVX512F
	vcmpsd	k5{k7}, xmm5, xmm4, 123	 # AVX512F
	vcmpsd	k5{k7}, xmm5, xmm4{sae}, 123	 # AVX512F
	vcmpsd	k5{k7}, xmm5, QWORD PTR [ecx], 123	 # AVX512F
	vcmpsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vcmpsd	k5{k7}, xmm5, QWORD PTR [edx+1016], 123	 # AVX512F Disp8
	vcmpsd	k5{k7}, xmm5, QWORD PTR [edx+1024], 123	 # AVX512F
	vcmpsd	k5{k7}, xmm5, QWORD PTR [edx-1024], 123	 # AVX512F Disp8
	vcmpsd	k5{k7}, xmm5, QWORD PTR [edx-1032], 123	 # AVX512F

	vcmpeq_oqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpeq_oqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpeq_oqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpeq_oqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpeq_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpeq_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpeq_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpeqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpeqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpeqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpeqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpeqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpeqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpeqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmplt_ossd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmplt_ossd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmplt_ossd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmplt_ossd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmplt_ossd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmplt_ossd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmplt_ossd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmplt_ossd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpltsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpltsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpltsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpltsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpltsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpltsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpltsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpltsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmple_ossd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmple_ossd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmple_ossd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmple_ossd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmple_ossd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmple_ossd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmple_ossd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmple_ossd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmplesd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmplesd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmplesd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmplesd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmplesd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmplesd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmplesd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmplesd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpunord_qsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpunord_qsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpunord_qsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpunord_qsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpunord_qsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpunord_qsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpunord_qsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpunord_qsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpunordsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpunordsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpunordsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpunordsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpunordsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpunordsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpunordsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpunordsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpneq_uqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpneq_uqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpneq_uqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpneq_uqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpneq_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpneq_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpneq_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpneqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpneqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpneqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpneqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpneqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpneqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpneqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpnlt_ussd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnlt_ussd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnlt_ussd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpnlt_ussd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnlt_ussd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpnlt_ussd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpnlt_ussd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpnlt_ussd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpnltsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnltsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnltsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpnltsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnltsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpnltsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpnltsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpnltsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpnle_ussd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnle_ussd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnle_ussd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpnle_ussd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnle_ussd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpnle_ussd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpnle_ussd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpnle_ussd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpnlesd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnlesd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnlesd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpnlesd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnlesd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpnlesd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpnlesd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpnlesd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpord_qsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpord_qsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpord_qsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpord_qsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpord_qsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpord_qsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpord_qsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpord_qsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpordsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpordsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpordsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpordsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpordsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpordsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpordsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpordsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpeq_uqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpeq_uqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpeq_uqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpeq_uqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpeq_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpeq_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpeq_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpnge_ussd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnge_ussd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnge_ussd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpnge_ussd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnge_ussd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpnge_ussd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpnge_ussd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpnge_ussd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpngesd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpngesd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpngesd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpngesd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngesd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpngesd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpngesd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpngesd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpngt_ussd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpngt_ussd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpngt_ussd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpngt_ussd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngt_ussd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpngt_ussd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpngt_ussd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpngt_ussd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpngtsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpngtsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpngtsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpngtsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngtsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpngtsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpngtsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpngtsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpfalse_oqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpfalse_oqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpfalse_oqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpfalse_oqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpfalse_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpfalse_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpfalse_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpfalse_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpfalsesd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpfalsesd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpfalsesd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpfalsesd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpfalsesd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpfalsesd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpfalsesd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpfalsesd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpneq_oqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpneq_oqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpneq_oqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpneq_oqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpneq_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpneq_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpneq_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpge_ossd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpge_ossd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpge_ossd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpge_ossd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpge_ossd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpge_ossd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpge_ossd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpge_ossd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpgesd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpgesd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpgesd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpgesd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgesd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpgesd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpgesd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpgesd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpgt_ossd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpgt_ossd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpgt_ossd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpgt_ossd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgt_ossd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpgt_ossd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpgt_ossd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpgt_ossd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpgtsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpgtsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpgtsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpgtsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgtsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpgtsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpgtsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpgtsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmptrue_uqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmptrue_uqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmptrue_uqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmptrue_uqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmptrue_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmptrue_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmptrue_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmptrue_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmptruesd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmptruesd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmptruesd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmptruesd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmptruesd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmptruesd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmptruesd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmptruesd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpeq_ossd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpeq_ossd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpeq_ossd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpeq_ossd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_ossd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpeq_ossd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpeq_ossd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpeq_ossd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmplt_oqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmplt_oqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmplt_oqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmplt_oqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmplt_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmplt_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmplt_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmplt_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmple_oqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmple_oqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmple_oqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmple_oqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmple_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmple_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmple_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmple_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpunord_ssd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpunord_ssd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpunord_ssd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpunord_ssd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpunord_ssd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpunord_ssd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpunord_ssd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpunord_ssd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpneq_ussd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpneq_ussd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpneq_ussd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpneq_ussd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_ussd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpneq_ussd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpneq_ussd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpneq_ussd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpnlt_uqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnlt_uqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnlt_uqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpnlt_uqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnlt_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpnlt_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpnlt_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpnlt_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpnle_uqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnle_uqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnle_uqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpnle_uqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnle_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpnle_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpnle_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpnle_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpord_ssd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpord_ssd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpord_ssd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpord_ssd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpord_ssd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpord_ssd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpord_ssd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpord_ssd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpeq_ussd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpeq_ussd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpeq_ussd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpeq_ussd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_ussd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpeq_ussd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpeq_ussd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpeq_ussd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpnge_uqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnge_uqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnge_uqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpnge_uqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnge_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpnge_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpnge_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpnge_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpngt_uqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpngt_uqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpngt_uqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpngt_uqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngt_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpngt_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpngt_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpngt_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpfalse_ossd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpfalse_ossd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpfalse_ossd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpfalse_ossd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpfalse_ossd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpfalse_ossd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpfalse_ossd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpfalse_ossd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpneq_ossd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpneq_ossd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpneq_ossd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpneq_ossd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_ossd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpneq_ossd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpneq_ossd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpneq_ossd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpge_oqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpge_oqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpge_oqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpge_oqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpge_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpge_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpge_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpge_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpgt_oqsd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpgt_oqsd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpgt_oqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmpgt_oqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgt_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmpgt_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmpgt_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmpgt_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmptrue_ussd	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmptrue_ussd	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmptrue_ussd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcmptrue_ussd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmptrue_ussd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcmptrue_ussd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcmptrue_ussd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcmptrue_ussd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vcmpss	k5{k7}, xmm5, xmm4, 0xab	 # AVX512F
	vcmpss	k5{k7}, xmm5, xmm4{sae}, 0xab	 # AVX512F
	vcmpss	k5{k7}, xmm5, xmm4, 123	 # AVX512F
	vcmpss	k5{k7}, xmm5, xmm4{sae}, 123	 # AVX512F
	vcmpss	k5{k7}, xmm5, DWORD PTR [ecx], 123	 # AVX512F
	vcmpss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vcmpss	k5{k7}, xmm5, DWORD PTR [edx+508], 123	 # AVX512F Disp8
	vcmpss	k5{k7}, xmm5, DWORD PTR [edx+512], 123	 # AVX512F
	vcmpss	k5{k7}, xmm5, DWORD PTR [edx-512], 123	 # AVX512F Disp8
	vcmpss	k5{k7}, xmm5, DWORD PTR [edx-516], 123	 # AVX512F

	vcmpeq_oqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpeq_oqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpeq_oqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpeq_oqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_oqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpeq_oqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpeq_oqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpeq_oqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpeqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpeqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpeqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpeqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpeqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpeqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpeqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmplt_osss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmplt_osss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmplt_osss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmplt_osss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmplt_osss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmplt_osss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmplt_osss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmplt_osss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpltss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpltss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpltss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpltss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpltss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpltss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpltss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpltss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmple_osss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmple_osss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmple_osss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmple_osss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmple_osss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmple_osss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmple_osss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmple_osss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpless	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpless	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpless	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpless	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpless	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpless	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpless	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpless	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpunord_qss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpunord_qss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpunord_qss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpunord_qss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpunord_qss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpunord_qss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpunord_qss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpunord_qss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpunordss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpunordss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpunordss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpunordss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpunordss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpunordss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpunordss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpunordss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpneq_uqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpneq_uqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpneq_uqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpneq_uqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_uqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpneq_uqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpneq_uqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpneq_uqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpneqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpneqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpneqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpneqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpneqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpneqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpneqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpnlt_usss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnlt_usss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnlt_usss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpnlt_usss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnlt_usss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpnlt_usss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpnlt_usss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpnlt_usss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpnltss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnltss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnltss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpnltss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnltss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpnltss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpnltss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpnltss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpnle_usss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnle_usss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnle_usss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpnle_usss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnle_usss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpnle_usss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpnle_usss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpnle_usss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpnless	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnless	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnless	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpnless	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnless	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpnless	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpnless	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpnless	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpord_qss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpord_qss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpord_qss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpord_qss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpord_qss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpord_qss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpord_qss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpord_qss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpordss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpordss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpordss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpordss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpordss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpordss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpordss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpordss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpeq_uqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpeq_uqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpeq_uqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpeq_uqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_uqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpeq_uqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpeq_uqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpeq_uqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpnge_usss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnge_usss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnge_usss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpnge_usss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnge_usss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpnge_usss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpnge_usss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpnge_usss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpngess	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpngess	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpngess	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpngess	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngess	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpngess	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpngess	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpngess	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpngt_usss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpngt_usss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpngt_usss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpngt_usss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngt_usss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpngt_usss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpngt_usss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpngt_usss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpngtss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpngtss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpngtss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpngtss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngtss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpngtss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpngtss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpngtss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpfalse_oqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpfalse_oqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpfalse_oqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpfalse_oqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpfalse_oqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpfalse_oqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpfalse_oqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpfalse_oqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpfalsess	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpfalsess	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpfalsess	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpfalsess	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpfalsess	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpfalsess	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpfalsess	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpfalsess	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpneq_oqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpneq_oqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpneq_oqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpneq_oqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_oqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpneq_oqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpneq_oqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpneq_oqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpge_osss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpge_osss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpge_osss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpge_osss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpge_osss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpge_osss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpge_osss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpge_osss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpgess	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpgess	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpgess	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpgess	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgess	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpgess	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpgess	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpgess	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpgt_osss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpgt_osss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpgt_osss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpgt_osss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgt_osss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpgt_osss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpgt_osss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpgt_osss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpgtss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpgtss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpgtss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpgtss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgtss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpgtss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpgtss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpgtss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmptrue_uqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmptrue_uqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmptrue_uqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmptrue_uqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmptrue_uqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmptrue_uqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmptrue_uqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmptrue_uqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmptruess	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmptruess	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmptruess	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmptruess	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmptruess	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmptruess	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmptruess	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmptruess	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpeq_osss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpeq_osss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpeq_osss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpeq_osss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_osss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpeq_osss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpeq_osss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpeq_osss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmplt_oqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmplt_oqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmplt_oqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmplt_oqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmplt_oqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmplt_oqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmplt_oqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmplt_oqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmple_oqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmple_oqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmple_oqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmple_oqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmple_oqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmple_oqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmple_oqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmple_oqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpunord_sss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpunord_sss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpunord_sss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpunord_sss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpunord_sss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpunord_sss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpunord_sss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpunord_sss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpneq_usss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpneq_usss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpneq_usss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpneq_usss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_usss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpneq_usss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpneq_usss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpneq_usss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpnlt_uqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnlt_uqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnlt_uqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpnlt_uqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnlt_uqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpnlt_uqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpnlt_uqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpnlt_uqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpnle_uqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnle_uqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnle_uqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpnle_uqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnle_uqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpnle_uqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpnle_uqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpnle_uqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpord_sss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpord_sss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpord_sss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpord_sss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpord_sss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpord_sss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpord_sss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpord_sss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpeq_usss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpeq_usss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpeq_usss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpeq_usss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpeq_usss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpeq_usss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpeq_usss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpeq_usss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpnge_uqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpnge_uqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpnge_uqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpnge_uqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpnge_uqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpnge_uqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpnge_uqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpnge_uqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpngt_uqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpngt_uqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpngt_uqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpngt_uqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpngt_uqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpngt_uqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpngt_uqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpngt_uqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpfalse_osss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpfalse_osss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpfalse_osss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpfalse_osss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpfalse_osss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpfalse_osss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpfalse_osss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpfalse_osss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpneq_osss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpneq_osss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpneq_osss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpneq_osss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpneq_osss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpneq_osss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpneq_osss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpneq_osss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpge_oqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpge_oqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpge_oqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpge_oqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpge_oqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpge_oqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpge_oqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpge_oqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmpgt_oqss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmpgt_oqss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmpgt_oqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmpgt_oqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmpgt_oqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmpgt_oqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmpgt_oqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmpgt_oqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcmptrue_usss	k5{k7}, xmm5, xmm4	 # AVX512F
	vcmptrue_usss	k5{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcmptrue_usss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcmptrue_usss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcmptrue_usss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcmptrue_usss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcmptrue_usss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcmptrue_usss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcomisd	xmm6, xmm5{sae}	 # AVX512F

	vcomiss	xmm6, xmm5{sae}	 # AVX512F

	vcompresspd	ZMMWORD PTR [ecx], zmm6	 # AVX512F
	vcompresspd	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vcompresspd	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vcompresspd	ZMMWORD PTR [edx+1016], zmm6	 # AVX512F Disp8
	vcompresspd	ZMMWORD PTR [edx+1024], zmm6	 # AVX512F
	vcompresspd	ZMMWORD PTR [edx-1024], zmm6	 # AVX512F Disp8
	vcompresspd	ZMMWORD PTR [edx-1032], zmm6	 # AVX512F

	vcompresspd	zmm6, zmm5	 # AVX512F
	vcompresspd	zmm6{k7}, zmm5	 # AVX512F
	vcompresspd	zmm6{k7}{z}, zmm5	 # AVX512F

	vcompressps	ZMMWORD PTR [ecx], zmm6	 # AVX512F
	vcompressps	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vcompressps	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vcompressps	ZMMWORD PTR [edx+508], zmm6	 # AVX512F Disp8
	vcompressps	ZMMWORD PTR [edx+512], zmm6	 # AVX512F
	vcompressps	ZMMWORD PTR [edx-512], zmm6	 # AVX512F Disp8
	vcompressps	ZMMWORD PTR [edx-516], zmm6	 # AVX512F

	vcompressps	zmm6, zmm5	 # AVX512F
	vcompressps	zmm6{k7}, zmm5	 # AVX512F
	vcompressps	zmm6{k7}{z}, zmm5	 # AVX512F

	vcvtdq2pd	zmm6{k7}, ymm5	 # AVX512F
	vcvtdq2pd	zmm6{k7}{z}, ymm5	 # AVX512F
	vcvtdq2pd	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512F
	vcvtdq2pd	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtdq2pd	zmm6{k7}, dword bcst [eax]	 # AVX512F
	vcvtdq2pd	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512F Disp8
	vcvtdq2pd	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512F
	vcvtdq2pd	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512F Disp8
	vcvtdq2pd	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512F
	vcvtdq2pd	zmm6{k7}, dword bcst [edx+508]	 # AVX512F Disp8
	vcvtdq2pd	zmm6{k7}, dword bcst [edx+512]	 # AVX512F
	vcvtdq2pd	zmm6{k7}, dword bcst [edx-512]	 # AVX512F Disp8
	vcvtdq2pd	zmm6{k7}, dword bcst [edx-516]	 # AVX512F

	vcvtdq2ps	zmm6, zmm5	 # AVX512F
	vcvtdq2ps	zmm6{k7}, zmm5	 # AVX512F
	vcvtdq2ps	zmm6{k7}{z}, zmm5	 # AVX512F
	vcvtdq2ps	zmm6, zmm5{rn-sae}	 # AVX512F
	vcvtdq2ps	zmm6, zmm5{ru-sae}	 # AVX512F
	vcvtdq2ps	zmm6, zmm5{rd-sae}	 # AVX512F
	vcvtdq2ps	zmm6, zmm5{rz-sae}	 # AVX512F
	vcvtdq2ps	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcvtdq2ps	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtdq2ps	zmm6, dword bcst [eax]	 # AVX512F
	vcvtdq2ps	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcvtdq2ps	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcvtdq2ps	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcvtdq2ps	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcvtdq2ps	zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcvtdq2ps	zmm6, dword bcst [edx+512]	 # AVX512F
	vcvtdq2ps	zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcvtdq2ps	zmm6, dword bcst [edx-516]	 # AVX512F

	vcvtpd2dq	ymm6{k7}, zmm5	 # AVX512F
	vcvtpd2dq	ymm6{k7}{z}, zmm5	 # AVX512F
	vcvtpd2dq	ymm6{k7}, zmm5{rn-sae}	 # AVX512F
	vcvtpd2dq	ymm6{k7}, zmm5{ru-sae}	 # AVX512F
	vcvtpd2dq	ymm6{k7}, zmm5{rd-sae}	 # AVX512F
	vcvtpd2dq	ymm6{k7}, zmm5{rz-sae}	 # AVX512F
	vcvtpd2dq	ymm6{k7}, ZMMWORD PTR [ecx]	 # AVX512F
	vcvtpd2dq	ymm6{k7}, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtpd2dq	ymm6{k7}, qword bcst [eax]	 # AVX512F
	vcvtpd2dq	ymm6{k7}, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcvtpd2dq	ymm6{k7}, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcvtpd2dq	ymm6{k7}, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcvtpd2dq	ymm6{k7}, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcvtpd2dq	ymm6{k7}, qword bcst [edx+1016]	 # AVX512F Disp8
	vcvtpd2dq	ymm6{k7}, qword bcst [edx+1024]	 # AVX512F
	vcvtpd2dq	ymm6{k7}, qword bcst [edx-1024]	 # AVX512F Disp8
	vcvtpd2dq	ymm6{k7}, qword bcst [edx-1032]	 # AVX512F

	vcvtpd2ps	ymm6{k7}, zmm5	 # AVX512F
	vcvtpd2ps	ymm6{k7}{z}, zmm5	 # AVX512F
	vcvtpd2ps	ymm6{k7}, zmm5{rn-sae}	 # AVX512F
	vcvtpd2ps	ymm6{k7}, zmm5{ru-sae}	 # AVX512F
	vcvtpd2ps	ymm6{k7}, zmm5{rd-sae}	 # AVX512F
	vcvtpd2ps	ymm6{k7}, zmm5{rz-sae}	 # AVX512F
	vcvtpd2ps	ymm6{k7}, ZMMWORD PTR [ecx]	 # AVX512F
	vcvtpd2ps	ymm6{k7}, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtpd2ps	ymm6{k7}, qword bcst [eax]	 # AVX512F
	vcvtpd2ps	ymm6{k7}, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcvtpd2ps	ymm6{k7}, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcvtpd2ps	ymm6{k7}, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcvtpd2ps	ymm6{k7}, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcvtpd2ps	ymm6{k7}, qword bcst [edx+1016]	 # AVX512F Disp8
	vcvtpd2ps	ymm6{k7}, qword bcst [edx+1024]	 # AVX512F
	vcvtpd2ps	ymm6{k7}, qword bcst [edx-1024]	 # AVX512F Disp8
	vcvtpd2ps	ymm6{k7}, qword bcst [edx-1032]	 # AVX512F

	vcvtpd2udq	ymm6{k7}, zmm5	 # AVX512F
	vcvtpd2udq	ymm6{k7}{z}, zmm5	 # AVX512F
	vcvtpd2udq	ymm6{k7}, zmm5{rn-sae}	 # AVX512F
	vcvtpd2udq	ymm6{k7}, zmm5{ru-sae}	 # AVX512F
	vcvtpd2udq	ymm6{k7}, zmm5{rd-sae}	 # AVX512F
	vcvtpd2udq	ymm6{k7}, zmm5{rz-sae}	 # AVX512F
	vcvtpd2udq	ymm6{k7}, ZMMWORD PTR [ecx]	 # AVX512F
	vcvtpd2udq	ymm6{k7}, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtpd2udq	ymm6{k7}, qword bcst [eax]	 # AVX512F
	vcvtpd2udq	ymm6{k7}, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcvtpd2udq	ymm6{k7}, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcvtpd2udq	ymm6{k7}, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcvtpd2udq	ymm6{k7}, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcvtpd2udq	ymm6{k7}, qword bcst [edx+1016]	 # AVX512F Disp8
	vcvtpd2udq	ymm6{k7}, qword bcst [edx+1024]	 # AVX512F
	vcvtpd2udq	ymm6{k7}, qword bcst [edx-1024]	 # AVX512F Disp8
	vcvtpd2udq	ymm6{k7}, qword bcst [edx-1032]	 # AVX512F

	vcvtph2ps	zmm6{k7}, ymm5	 # AVX512F
	vcvtph2ps	zmm6{k7}{z}, ymm5	 # AVX512F
	vcvtph2ps	zmm6{k7}, ymm5{sae}	 # AVX512F
	vcvtph2ps	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512F
	vcvtph2ps	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtph2ps	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512F Disp8
	vcvtph2ps	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512F
	vcvtph2ps	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512F Disp8
	vcvtph2ps	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512F

	vcvtps2dq	zmm6, zmm5	 # AVX512F
	vcvtps2dq	zmm6{k7}, zmm5	 # AVX512F
	vcvtps2dq	zmm6{k7}{z}, zmm5	 # AVX512F
	vcvtps2dq	zmm6, zmm5{rn-sae}	 # AVX512F
	vcvtps2dq	zmm6, zmm5{ru-sae}	 # AVX512F
	vcvtps2dq	zmm6, zmm5{rd-sae}	 # AVX512F
	vcvtps2dq	zmm6, zmm5{rz-sae}	 # AVX512F
	vcvtps2dq	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcvtps2dq	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtps2dq	zmm6, dword bcst [eax]	 # AVX512F
	vcvtps2dq	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcvtps2dq	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcvtps2dq	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcvtps2dq	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcvtps2dq	zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcvtps2dq	zmm6, dword bcst [edx+512]	 # AVX512F
	vcvtps2dq	zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcvtps2dq	zmm6, dword bcst [edx-516]	 # AVX512F

	vcvtps2pd	zmm6{k7}, ymm5	 # AVX512F
	vcvtps2pd	zmm6{k7}{z}, ymm5	 # AVX512F
	vcvtps2pd	zmm6{k7}, ymm5{sae}	 # AVX512F
	vcvtps2pd	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512F
	vcvtps2pd	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtps2pd	zmm6{k7}, dword bcst [eax]	 # AVX512F
	vcvtps2pd	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512F Disp8
	vcvtps2pd	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512F
	vcvtps2pd	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512F Disp8
	vcvtps2pd	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512F
	vcvtps2pd	zmm6{k7}, dword bcst [edx+508]	 # AVX512F Disp8
	vcvtps2pd	zmm6{k7}, dword bcst [edx+512]	 # AVX512F
	vcvtps2pd	zmm6{k7}, dword bcst [edx-512]	 # AVX512F Disp8
	vcvtps2pd	zmm6{k7}, dword bcst [edx-516]	 # AVX512F

	vcvtps2ph	ymm6{k7}, zmm5, 0xab	 # AVX512F
	vcvtps2ph	ymm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vcvtps2ph	ymm6{k7}, zmm5{sae}, 0xab	 # AVX512F
	vcvtps2ph	ymm6{k7}, zmm5, 123	 # AVX512F
	vcvtps2ph	ymm6{k7}, zmm5{sae}, 123	 # AVX512F

	vcvtps2udq	zmm6, zmm5	 # AVX512F
	vcvtps2udq	zmm6{k7}, zmm5	 # AVX512F
	vcvtps2udq	zmm6{k7}{z}, zmm5	 # AVX512F
	vcvtps2udq	zmm6, zmm5{rn-sae}	 # AVX512F
	vcvtps2udq	zmm6, zmm5{ru-sae}	 # AVX512F
	vcvtps2udq	zmm6, zmm5{rd-sae}	 # AVX512F
	vcvtps2udq	zmm6, zmm5{rz-sae}	 # AVX512F
	vcvtps2udq	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcvtps2udq	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtps2udq	zmm6, dword bcst [eax]	 # AVX512F
	vcvtps2udq	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcvtps2udq	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcvtps2udq	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcvtps2udq	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcvtps2udq	zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcvtps2udq	zmm6, dword bcst [edx+512]	 # AVX512F
	vcvtps2udq	zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcvtps2udq	zmm6, dword bcst [edx-516]	 # AVX512F

	vcvtsd2si	eax, xmm6{rn-sae}	 # AVX512F
	vcvtsd2si	eax, xmm6{ru-sae}	 # AVX512F
	vcvtsd2si	eax, xmm6{rd-sae}	 # AVX512F
	vcvtsd2si	eax, xmm6{rz-sae}	 # AVX512F
	vcvtsd2si	ebp, xmm6{rn-sae}	 # AVX512F
	vcvtsd2si	ebp, xmm6{ru-sae}	 # AVX512F
	vcvtsd2si	ebp, xmm6{rd-sae}	 # AVX512F
	vcvtsd2si	ebp, xmm6{rz-sae}	 # AVX512F

	vcvtsd2ss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vcvtsd2ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vcvtsd2ss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vcvtsd2ss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vcvtsd2ss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vcvtsd2ss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vcvtsd2ss	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vcvtsd2ss	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtsd2ss	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcvtsd2ss	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vcvtsd2ss	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcvtsd2ss	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F


	vcvtsi2ss	xmm6, xmm5, eax{rn-sae}	 # AVX512F
	vcvtsi2ss	xmm6, xmm5, eax{ru-sae}	 # AVX512F
	vcvtsi2ss	xmm6, xmm5, eax{rd-sae}	 # AVX512F
	vcvtsi2ss	xmm6, xmm5, eax{rz-sae}	 # AVX512F
	vcvtsi2ss	xmm6, xmm5, ebp{rn-sae}	 # AVX512F
	vcvtsi2ss	xmm6, xmm5, ebp{ru-sae}	 # AVX512F
	vcvtsi2ss	xmm6, xmm5, ebp{rd-sae}	 # AVX512F
	vcvtsi2ss	xmm6, xmm5, ebp{rz-sae}	 # AVX512F

	vcvtss2sd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vcvtss2sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vcvtss2sd	xmm6{k7}, xmm5, xmm4{sae}	 # AVX512F
	vcvtss2sd	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcvtss2sd	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtss2sd	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcvtss2sd	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcvtss2sd	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcvtss2sd	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcvtss2si	eax, xmm6{rn-sae}	 # AVX512F
	vcvtss2si	eax, xmm6{ru-sae}	 # AVX512F
	vcvtss2si	eax, xmm6{rd-sae}	 # AVX512F
	vcvtss2si	eax, xmm6{rz-sae}	 # AVX512F
	vcvtss2si	ebp, xmm6{rn-sae}	 # AVX512F
	vcvtss2si	ebp, xmm6{ru-sae}	 # AVX512F
	vcvtss2si	ebp, xmm6{rd-sae}	 # AVX512F
	vcvtss2si	ebp, xmm6{rz-sae}	 # AVX512F

	vcvttpd2dq	ymm6{k7}, zmm5	 # AVX512F
	vcvttpd2dq	ymm6{k7}{z}, zmm5	 # AVX512F
	vcvttpd2dq	ymm6{k7}, zmm5{sae}	 # AVX512F
	vcvttpd2dq	ymm6{k7}, ZMMWORD PTR [ecx]	 # AVX512F
	vcvttpd2dq	ymm6{k7}, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvttpd2dq	ymm6{k7}, qword bcst [eax]	 # AVX512F
	vcvttpd2dq	ymm6{k7}, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcvttpd2dq	ymm6{k7}, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcvttpd2dq	ymm6{k7}, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcvttpd2dq	ymm6{k7}, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcvttpd2dq	ymm6{k7}, qword bcst [edx+1016]	 # AVX512F Disp8
	vcvttpd2dq	ymm6{k7}, qword bcst [edx+1024]	 # AVX512F
	vcvttpd2dq	ymm6{k7}, qword bcst [edx-1024]	 # AVX512F Disp8
	vcvttpd2dq	ymm6{k7}, qword bcst [edx-1032]	 # AVX512F

	vcvttps2dq	zmm6, zmm5	 # AVX512F
	vcvttps2dq	zmm6{k7}, zmm5	 # AVX512F
	vcvttps2dq	zmm6{k7}{z}, zmm5	 # AVX512F
	vcvttps2dq	zmm6, zmm5{sae}	 # AVX512F
	vcvttps2dq	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcvttps2dq	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvttps2dq	zmm6, dword bcst [eax]	 # AVX512F
	vcvttps2dq	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcvttps2dq	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcvttps2dq	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcvttps2dq	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcvttps2dq	zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcvttps2dq	zmm6, dword bcst [edx+512]	 # AVX512F
	vcvttps2dq	zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcvttps2dq	zmm6, dword bcst [edx-516]	 # AVX512F

	vcvttsd2si	eax, xmm6{sae}	 # AVX512F
	vcvttsd2si	ebp, xmm6{sae}	 # AVX512F

	vcvttss2si	eax, xmm6{sae}	 # AVX512F
	vcvttss2si	ebp, xmm6{sae}	 # AVX512F

	vcvtudq2pd	zmm6{k7}, ymm5	 # AVX512F
	vcvtudq2pd	zmm6{k7}{z}, ymm5	 # AVX512F
	vcvtudq2pd	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512F
	vcvtudq2pd	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtudq2pd	zmm6{k7}, dword bcst [eax]	 # AVX512F
	vcvtudq2pd	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512F Disp8
	vcvtudq2pd	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512F
	vcvtudq2pd	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512F Disp8
	vcvtudq2pd	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512F
	vcvtudq2pd	zmm6{k7}, dword bcst [edx+508]	 # AVX512F Disp8
	vcvtudq2pd	zmm6{k7}, dword bcst [edx+512]	 # AVX512F
	vcvtudq2pd	zmm6{k7}, dword bcst [edx-512]	 # AVX512F Disp8
	vcvtudq2pd	zmm6{k7}, dword bcst [edx-516]	 # AVX512F

	vcvtudq2ps	zmm6, zmm5	 # AVX512F
	vcvtudq2ps	zmm6{k7}, zmm5	 # AVX512F
	vcvtudq2ps	zmm6{k7}{z}, zmm5	 # AVX512F
	vcvtudq2ps	zmm6, zmm5{rn-sae}	 # AVX512F
	vcvtudq2ps	zmm6, zmm5{ru-sae}	 # AVX512F
	vcvtudq2ps	zmm6, zmm5{rd-sae}	 # AVX512F
	vcvtudq2ps	zmm6, zmm5{rz-sae}	 # AVX512F
	vcvtudq2ps	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcvtudq2ps	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtudq2ps	zmm6, dword bcst [eax]	 # AVX512F
	vcvtudq2ps	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcvtudq2ps	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcvtudq2ps	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcvtudq2ps	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcvtudq2ps	zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcvtudq2ps	zmm6, dword bcst [edx+512]	 # AVX512F
	vcvtudq2ps	zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcvtudq2ps	zmm6, dword bcst [edx-516]	 # AVX512F

	vdivpd	zmm6, zmm5, zmm4	 # AVX512F
	vdivpd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vdivpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vdivpd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vdivpd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vdivpd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vdivpd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vdivpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vdivpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vdivpd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vdivpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vdivpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vdivpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vdivpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vdivpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vdivpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vdivpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vdivpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vdivps	zmm6, zmm5, zmm4	 # AVX512F
	vdivps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vdivps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vdivps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vdivps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vdivps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vdivps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vdivps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vdivps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vdivps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vdivps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vdivps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vdivps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vdivps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vdivps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vdivps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vdivps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vdivps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vdivsd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vdivsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vdivsd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vdivsd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vdivsd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vdivsd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vdivsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vdivsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vdivsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vdivsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vdivsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vdivsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vdivss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vdivss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vdivss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vdivss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vdivss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vdivss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vdivss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vdivss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vdivss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vdivss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vdivss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vdivss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vexpandpd	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vexpandpd	zmm6{k7}, ZMMWORD PTR [ecx]	 # AVX512F
	vexpandpd	zmm6{k7}{z}, ZMMWORD PTR [ecx]	 # AVX512F
	vexpandpd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vexpandpd	zmm6, ZMMWORD PTR [edx+1016]	 # AVX512F Disp8
	vexpandpd	zmm6, ZMMWORD PTR [edx+1024]	 # AVX512F
	vexpandpd	zmm6, ZMMWORD PTR [edx-1024]	 # AVX512F Disp8
	vexpandpd	zmm6, ZMMWORD PTR [edx-1032]	 # AVX512F

	vexpandpd	zmm6, zmm5	 # AVX512F
	vexpandpd	zmm6{k7}, zmm5	 # AVX512F
	vexpandpd	zmm6{k7}{z}, zmm5	 # AVX512F

	vexpandps	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vexpandps	zmm6{k7}, ZMMWORD PTR [ecx]	 # AVX512F
	vexpandps	zmm6{k7}{z}, ZMMWORD PTR [ecx]	 # AVX512F
	vexpandps	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vexpandps	zmm6, ZMMWORD PTR [edx+508]	 # AVX512F Disp8
	vexpandps	zmm6, ZMMWORD PTR [edx+512]	 # AVX512F
	vexpandps	zmm6, ZMMWORD PTR [edx-512]	 # AVX512F Disp8
	vexpandps	zmm6, ZMMWORD PTR [edx-516]	 # AVX512F

	vexpandps	zmm6, zmm5	 # AVX512F
	vexpandps	zmm6{k7}, zmm5	 # AVX512F
	vexpandps	zmm6{k7}{z}, zmm5	 # AVX512F

	vextractf32x4	xmm6{k7}, zmm5, 0xab	 # AVX512F
	vextractf32x4	xmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vextractf32x4	xmm6{k7}, zmm5, 123	 # AVX512F

	vextractf64x4	ymm6{k7}, zmm5, 0xab	 # AVX512F
	vextractf64x4	ymm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vextractf64x4	ymm6{k7}, zmm5, 123	 # AVX512F

	vextracti32x4	xmm6{k7}, zmm5, 0xab	 # AVX512F
	vextracti32x4	xmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vextracti32x4	xmm6{k7}, zmm5, 123	 # AVX512F

	vextracti64x4	ymm6{k7}, zmm5, 0xab	 # AVX512F
	vextracti64x4	ymm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vextracti64x4	ymm6{k7}, zmm5, 123	 # AVX512F

	vfmadd132pd	zmm6, zmm5, zmm4	 # AVX512F
	vfmadd132pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmadd132pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmadd132pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmadd132pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmadd132pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmadd132pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmadd132pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmadd132pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmadd132pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfmadd132pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmadd132pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmadd132pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmadd132pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmadd132pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfmadd132pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfmadd132pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfmadd132pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfmadd132ps	zmm6, zmm5, zmm4	 # AVX512F
	vfmadd132ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmadd132ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmadd132ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmadd132ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmadd132ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmadd132ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmadd132ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmadd132ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmadd132ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfmadd132ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmadd132ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmadd132ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmadd132ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmadd132ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfmadd132ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfmadd132ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfmadd132ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfmadd132sd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfmadd132sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfmadd132sd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfmadd132sd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfmadd132sd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfmadd132sd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfmadd132sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vfmadd132sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vfmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vfmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vfmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vfmadd132ss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfmadd132ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfmadd132ss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfmadd132ss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfmadd132ss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfmadd132ss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfmadd132ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vfmadd132ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vfmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vfmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vfmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vfmadd213pd	zmm6, zmm5, zmm4	 # AVX512F
	vfmadd213pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmadd213pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmadd213pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmadd213pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmadd213pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmadd213pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmadd213pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmadd213pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmadd213pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfmadd213pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmadd213pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmadd213pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmadd213pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmadd213pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfmadd213pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfmadd213pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfmadd213pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfmadd213ps	zmm6, zmm5, zmm4	 # AVX512F
	vfmadd213ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmadd213ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmadd213ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmadd213ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmadd213ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmadd213ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmadd213ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmadd213ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmadd213ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfmadd213ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmadd213ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmadd213ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmadd213ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmadd213ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfmadd213ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfmadd213ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfmadd213ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfmadd213sd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfmadd213sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfmadd213sd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfmadd213sd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfmadd213sd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfmadd213sd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfmadd213sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vfmadd213sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vfmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vfmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vfmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vfmadd213ss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfmadd213ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfmadd213ss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfmadd213ss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfmadd213ss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfmadd213ss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfmadd213ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vfmadd213ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vfmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vfmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vfmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vfmadd231pd	zmm6, zmm5, zmm4	 # AVX512F
	vfmadd231pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmadd231pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmadd231pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmadd231pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmadd231pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmadd231pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmadd231pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmadd231pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmadd231pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfmadd231pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmadd231pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmadd231pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmadd231pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmadd231pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfmadd231pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfmadd231pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfmadd231pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfmadd231ps	zmm6, zmm5, zmm4	 # AVX512F
	vfmadd231ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmadd231ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmadd231ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmadd231ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmadd231ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmadd231ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmadd231ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmadd231ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmadd231ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfmadd231ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmadd231ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmadd231ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmadd231ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmadd231ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfmadd231ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfmadd231ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfmadd231ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfmadd231sd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfmadd231sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfmadd231sd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfmadd231sd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfmadd231sd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfmadd231sd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfmadd231sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vfmadd231sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vfmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vfmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vfmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vfmadd231ss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfmadd231ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfmadd231ss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfmadd231ss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfmadd231ss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfmadd231ss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfmadd231ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vfmadd231ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vfmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vfmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vfmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vfmaddsub132pd	zmm6, zmm5, zmm4	 # AVX512F
	vfmaddsub132pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmaddsub132pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmaddsub132pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmaddsub132pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmaddsub132pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmaddsub132pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmaddsub132pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmaddsub132pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmaddsub132pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfmaddsub132pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmaddsub132pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmaddsub132pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmaddsub132pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmaddsub132pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfmaddsub132pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfmaddsub132pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfmaddsub132pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfmaddsub132ps	zmm6, zmm5, zmm4	 # AVX512F
	vfmaddsub132ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmaddsub132ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmaddsub132ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmaddsub132ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmaddsub132ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmaddsub132ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmaddsub132ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmaddsub132ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmaddsub132ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfmaddsub132ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmaddsub132ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmaddsub132ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmaddsub132ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmaddsub132ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfmaddsub132ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfmaddsub132ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfmaddsub132ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfmaddsub213pd	zmm6, zmm5, zmm4	 # AVX512F
	vfmaddsub213pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmaddsub213pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmaddsub213pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmaddsub213pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmaddsub213pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmaddsub213pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmaddsub213pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmaddsub213pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmaddsub213pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfmaddsub213pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmaddsub213pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmaddsub213pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmaddsub213pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmaddsub213pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfmaddsub213pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfmaddsub213pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfmaddsub213pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfmaddsub213ps	zmm6, zmm5, zmm4	 # AVX512F
	vfmaddsub213ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmaddsub213ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmaddsub213ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmaddsub213ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmaddsub213ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmaddsub213ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmaddsub213ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmaddsub213ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmaddsub213ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfmaddsub213ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmaddsub213ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmaddsub213ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmaddsub213ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmaddsub213ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfmaddsub213ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfmaddsub213ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfmaddsub213ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfmaddsub231pd	zmm6, zmm5, zmm4	 # AVX512F
	vfmaddsub231pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmaddsub231pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmaddsub231pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmaddsub231pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmaddsub231pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmaddsub231pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmaddsub231pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmaddsub231pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmaddsub231pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfmaddsub231pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmaddsub231pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmaddsub231pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmaddsub231pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmaddsub231pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfmaddsub231pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfmaddsub231pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfmaddsub231pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfmaddsub231ps	zmm6, zmm5, zmm4	 # AVX512F
	vfmaddsub231ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmaddsub231ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmaddsub231ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmaddsub231ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmaddsub231ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmaddsub231ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmaddsub231ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmaddsub231ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmaddsub231ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfmaddsub231ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmaddsub231ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmaddsub231ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmaddsub231ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmaddsub231ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfmaddsub231ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfmaddsub231ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfmaddsub231ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfmsub132pd	zmm6, zmm5, zmm4	 # AVX512F
	vfmsub132pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmsub132pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmsub132pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmsub132pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmsub132pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmsub132pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmsub132pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmsub132pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsub132pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfmsub132pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmsub132pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmsub132pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmsub132pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmsub132pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfmsub132pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfmsub132pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfmsub132pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfmsub132ps	zmm6, zmm5, zmm4	 # AVX512F
	vfmsub132ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmsub132ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmsub132ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmsub132ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmsub132ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmsub132ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmsub132ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmsub132ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsub132ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfmsub132ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmsub132ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmsub132ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmsub132ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmsub132ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfmsub132ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfmsub132ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfmsub132ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfmsub132sd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfmsub132sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfmsub132sd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfmsub132sd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfmsub132sd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfmsub132sd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfmsub132sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vfmsub132sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vfmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vfmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vfmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vfmsub132ss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfmsub132ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfmsub132ss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfmsub132ss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfmsub132ss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfmsub132ss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfmsub132ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vfmsub132ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vfmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vfmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vfmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vfmsub213pd	zmm6, zmm5, zmm4	 # AVX512F
	vfmsub213pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmsub213pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmsub213pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmsub213pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmsub213pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmsub213pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmsub213pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmsub213pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsub213pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfmsub213pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmsub213pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmsub213pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmsub213pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmsub213pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfmsub213pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfmsub213pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfmsub213pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfmsub213ps	zmm6, zmm5, zmm4	 # AVX512F
	vfmsub213ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmsub213ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmsub213ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmsub213ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmsub213ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmsub213ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmsub213ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmsub213ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsub213ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfmsub213ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmsub213ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmsub213ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmsub213ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmsub213ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfmsub213ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfmsub213ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfmsub213ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfmsub213sd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfmsub213sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfmsub213sd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfmsub213sd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfmsub213sd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfmsub213sd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfmsub213sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vfmsub213sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vfmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vfmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vfmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vfmsub213ss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfmsub213ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfmsub213ss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfmsub213ss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfmsub213ss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfmsub213ss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfmsub213ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vfmsub213ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vfmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vfmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vfmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vfmsub231pd	zmm6, zmm5, zmm4	 # AVX512F
	vfmsub231pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmsub231pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmsub231pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmsub231pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmsub231pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmsub231pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmsub231pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmsub231pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsub231pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfmsub231pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmsub231pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmsub231pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmsub231pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmsub231pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfmsub231pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfmsub231pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfmsub231pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfmsub231ps	zmm6, zmm5, zmm4	 # AVX512F
	vfmsub231ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmsub231ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmsub231ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmsub231ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmsub231ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmsub231ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmsub231ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmsub231ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsub231ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfmsub231ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmsub231ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmsub231ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmsub231ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmsub231ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfmsub231ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfmsub231ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfmsub231ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfmsub231sd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfmsub231sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfmsub231sd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfmsub231sd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfmsub231sd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfmsub231sd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfmsub231sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vfmsub231sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vfmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vfmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vfmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vfmsub231ss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfmsub231ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfmsub231ss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfmsub231ss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfmsub231ss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfmsub231ss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfmsub231ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vfmsub231ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vfmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vfmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vfmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vfmsubadd132pd	zmm6, zmm5, zmm4	 # AVX512F
	vfmsubadd132pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmsubadd132pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmsubadd132pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmsubadd132pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmsubadd132pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmsubadd132pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmsubadd132pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmsubadd132pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsubadd132pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfmsubadd132pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmsubadd132pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmsubadd132pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmsubadd132pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmsubadd132pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfmsubadd132pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfmsubadd132pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfmsubadd132pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfmsubadd132ps	zmm6, zmm5, zmm4	 # AVX512F
	vfmsubadd132ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmsubadd132ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmsubadd132ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmsubadd132ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmsubadd132ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmsubadd132ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmsubadd132ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmsubadd132ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsubadd132ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfmsubadd132ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmsubadd132ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmsubadd132ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmsubadd132ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmsubadd132ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfmsubadd132ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfmsubadd132ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfmsubadd132ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfmsubadd213pd	zmm6, zmm5, zmm4	 # AVX512F
	vfmsubadd213pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmsubadd213pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmsubadd213pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmsubadd213pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmsubadd213pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmsubadd213pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmsubadd213pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmsubadd213pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsubadd213pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfmsubadd213pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmsubadd213pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmsubadd213pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmsubadd213pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmsubadd213pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfmsubadd213pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfmsubadd213pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfmsubadd213pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfmsubadd213ps	zmm6, zmm5, zmm4	 # AVX512F
	vfmsubadd213ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmsubadd213ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmsubadd213ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmsubadd213ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmsubadd213ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmsubadd213ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmsubadd213ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmsubadd213ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsubadd213ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfmsubadd213ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmsubadd213ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmsubadd213ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmsubadd213ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmsubadd213ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfmsubadd213ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfmsubadd213ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfmsubadd213ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfmsubadd231pd	zmm6, zmm5, zmm4	 # AVX512F
	vfmsubadd231pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmsubadd231pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmsubadd231pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmsubadd231pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmsubadd231pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmsubadd231pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmsubadd231pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmsubadd231pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsubadd231pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfmsubadd231pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmsubadd231pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmsubadd231pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmsubadd231pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmsubadd231pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfmsubadd231pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfmsubadd231pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfmsubadd231pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfmsubadd231ps	zmm6, zmm5, zmm4	 # AVX512F
	vfmsubadd231ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfmsubadd231ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfmsubadd231ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfmsubadd231ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfmsubadd231ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfmsubadd231ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfmsubadd231ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfmsubadd231ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfmsubadd231ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfmsubadd231ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfmsubadd231ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfmsubadd231ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfmsubadd231ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfmsubadd231ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfmsubadd231ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfmsubadd231ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfmsubadd231ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfnmadd132pd	zmm6, zmm5, zmm4	 # AVX512F
	vfnmadd132pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfnmadd132pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfnmadd132pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfnmadd132pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfnmadd132pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfnmadd132pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfnmadd132pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfnmadd132pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmadd132pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfnmadd132pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfnmadd132pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfnmadd132pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfnmadd132pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfnmadd132pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfnmadd132pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfnmadd132pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfnmadd132pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfnmadd132ps	zmm6, zmm5, zmm4	 # AVX512F
	vfnmadd132ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfnmadd132ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfnmadd132ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfnmadd132ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfnmadd132ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfnmadd132ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfnmadd132ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfnmadd132ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmadd132ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfnmadd132ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfnmadd132ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfnmadd132ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfnmadd132ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfnmadd132ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfnmadd132ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfnmadd132ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfnmadd132ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfnmadd132sd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfnmadd132sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfnmadd132sd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfnmadd132sd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfnmadd132sd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfnmadd132sd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfnmadd132sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vfnmadd132sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vfnmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vfnmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vfnmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vfnmadd132ss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfnmadd132ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfnmadd132ss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfnmadd132ss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfnmadd132ss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfnmadd132ss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfnmadd132ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vfnmadd132ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vfnmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vfnmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vfnmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vfnmadd213pd	zmm6, zmm5, zmm4	 # AVX512F
	vfnmadd213pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfnmadd213pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfnmadd213pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfnmadd213pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfnmadd213pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfnmadd213pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfnmadd213pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfnmadd213pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmadd213pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfnmadd213pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfnmadd213pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfnmadd213pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfnmadd213pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfnmadd213pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfnmadd213pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfnmadd213pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfnmadd213pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfnmadd213ps	zmm6, zmm5, zmm4	 # AVX512F
	vfnmadd213ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfnmadd213ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfnmadd213ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfnmadd213ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfnmadd213ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfnmadd213ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfnmadd213ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfnmadd213ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmadd213ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfnmadd213ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfnmadd213ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfnmadd213ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfnmadd213ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfnmadd213ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfnmadd213ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfnmadd213ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfnmadd213ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfnmadd213sd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfnmadd213sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfnmadd213sd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfnmadd213sd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfnmadd213sd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfnmadd213sd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfnmadd213sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vfnmadd213sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vfnmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vfnmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vfnmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vfnmadd213ss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfnmadd213ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfnmadd213ss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfnmadd213ss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfnmadd213ss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfnmadd213ss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfnmadd213ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vfnmadd213ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vfnmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vfnmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vfnmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vfnmadd231pd	zmm6, zmm5, zmm4	 # AVX512F
	vfnmadd231pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfnmadd231pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfnmadd231pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfnmadd231pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfnmadd231pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfnmadd231pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfnmadd231pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfnmadd231pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmadd231pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfnmadd231pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfnmadd231pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfnmadd231pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfnmadd231pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfnmadd231pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfnmadd231pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfnmadd231pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfnmadd231pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfnmadd231ps	zmm6, zmm5, zmm4	 # AVX512F
	vfnmadd231ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfnmadd231ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfnmadd231ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfnmadd231ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfnmadd231ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfnmadd231ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfnmadd231ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfnmadd231ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmadd231ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfnmadd231ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfnmadd231ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfnmadd231ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfnmadd231ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfnmadd231ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfnmadd231ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfnmadd231ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfnmadd231ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfnmadd231sd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfnmadd231sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfnmadd231sd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfnmadd231sd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfnmadd231sd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfnmadd231sd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfnmadd231sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vfnmadd231sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vfnmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vfnmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vfnmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vfnmadd231ss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfnmadd231ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfnmadd231ss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfnmadd231ss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfnmadd231ss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfnmadd231ss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfnmadd231ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vfnmadd231ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vfnmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vfnmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vfnmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vfnmsub132pd	zmm6, zmm5, zmm4	 # AVX512F
	vfnmsub132pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfnmsub132pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfnmsub132pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfnmsub132pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfnmsub132pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfnmsub132pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfnmsub132pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfnmsub132pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmsub132pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfnmsub132pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfnmsub132pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfnmsub132pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfnmsub132pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfnmsub132pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfnmsub132pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfnmsub132pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfnmsub132pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfnmsub132ps	zmm6, zmm5, zmm4	 # AVX512F
	vfnmsub132ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfnmsub132ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfnmsub132ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfnmsub132ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfnmsub132ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfnmsub132ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfnmsub132ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfnmsub132ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmsub132ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfnmsub132ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfnmsub132ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfnmsub132ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfnmsub132ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfnmsub132ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfnmsub132ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfnmsub132ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfnmsub132ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfnmsub132sd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfnmsub132sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfnmsub132sd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfnmsub132sd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfnmsub132sd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfnmsub132sd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfnmsub132sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vfnmsub132sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vfnmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vfnmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vfnmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vfnmsub132ss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfnmsub132ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfnmsub132ss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfnmsub132ss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfnmsub132ss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfnmsub132ss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfnmsub132ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vfnmsub132ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vfnmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vfnmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vfnmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vfnmsub213pd	zmm6, zmm5, zmm4	 # AVX512F
	vfnmsub213pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfnmsub213pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfnmsub213pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfnmsub213pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfnmsub213pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfnmsub213pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfnmsub213pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfnmsub213pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmsub213pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfnmsub213pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfnmsub213pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfnmsub213pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfnmsub213pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfnmsub213pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfnmsub213pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfnmsub213pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfnmsub213pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfnmsub213ps	zmm6, zmm5, zmm4	 # AVX512F
	vfnmsub213ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfnmsub213ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfnmsub213ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfnmsub213ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfnmsub213ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfnmsub213ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfnmsub213ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfnmsub213ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmsub213ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfnmsub213ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfnmsub213ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfnmsub213ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfnmsub213ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfnmsub213ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfnmsub213ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfnmsub213ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfnmsub213ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfnmsub213sd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfnmsub213sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfnmsub213sd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfnmsub213sd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfnmsub213sd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfnmsub213sd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfnmsub213sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vfnmsub213sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vfnmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vfnmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vfnmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vfnmsub213ss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfnmsub213ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfnmsub213ss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfnmsub213ss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfnmsub213ss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfnmsub213ss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfnmsub213ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vfnmsub213ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vfnmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vfnmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vfnmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vfnmsub231pd	zmm6, zmm5, zmm4	 # AVX512F
	vfnmsub231pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfnmsub231pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfnmsub231pd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfnmsub231pd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfnmsub231pd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfnmsub231pd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfnmsub231pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfnmsub231pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmsub231pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vfnmsub231pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfnmsub231pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfnmsub231pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfnmsub231pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfnmsub231pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vfnmsub231pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vfnmsub231pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vfnmsub231pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vfnmsub231ps	zmm6, zmm5, zmm4	 # AVX512F
	vfnmsub231ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vfnmsub231ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vfnmsub231ps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vfnmsub231ps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vfnmsub231ps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vfnmsub231ps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vfnmsub231ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vfnmsub231ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmsub231ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vfnmsub231ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vfnmsub231ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vfnmsub231ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vfnmsub231ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vfnmsub231ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vfnmsub231ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vfnmsub231ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vfnmsub231ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vfnmsub231sd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfnmsub231sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfnmsub231sd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfnmsub231sd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfnmsub231sd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfnmsub231sd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfnmsub231sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vfnmsub231sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vfnmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vfnmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vfnmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vfnmsub231ss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vfnmsub231ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vfnmsub231ss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vfnmsub231ss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vfnmsub231ss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vfnmsub231ss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vfnmsub231ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vfnmsub231ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vfnmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vfnmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vfnmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vfnmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vgatherdpd	zmm6{k1}, [ebp+ymm7*8-123]	 # AVX512F
	vgatherdpd	zmm6{k1}, qword ptr [ebp+ymm7*8-123]	 # AVX512F
	vgatherdpd	zmm6{k1}, [eax+ymm7+256]	 # AVX512F
	vgatherdpd	zmm6{k1}, [ecx+ymm7*4+1024]	 # AVX512F

	vgatherdps	zmm6{k1}, [ebp+zmm7*8-123]	 # AVX512F
	vgatherdps	zmm6{k1}, dword ptr [ebp+zmm7*8-123]	 # AVX512F
	vgatherdps	zmm6{k1}, [eax+zmm7+256]	 # AVX512F
	vgatherdps	zmm6{k1}, [ecx+zmm7*4+1024]	 # AVX512F

	vgatherqpd	zmm6{k1}, [ebp+zmm7*8-123]	 # AVX512F
	vgatherqpd	zmm6{k1}, qword ptr [ebp+zmm7*8-123]	 # AVX512F
	vgatherqpd	zmm6{k1}, [eax+zmm7+256]	 # AVX512F
	vgatherqpd	zmm6{k1}, [ecx+zmm7*4+1024]	 # AVX512F

	vgatherqps	ymm6{k1}, [ebp+zmm7*8-123]	 # AVX512F
	vgatherqps	ymm6{k1}, dword ptr [ebp+zmm7*8-123]	 # AVX512F
	vgatherqps	ymm6{k1}, [eax+zmm7+256]	 # AVX512F
	vgatherqps	ymm6{k1}, [ecx+zmm7*4+1024]	 # AVX512F

	vgetexppd	zmm6, zmm5	 # AVX512F
	vgetexppd	zmm6{k7}, zmm5	 # AVX512F
	vgetexppd	zmm6{k7}{z}, zmm5	 # AVX512F
	vgetexppd	zmm6, zmm5{sae}	 # AVX512F
	vgetexppd	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vgetexppd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vgetexppd	zmm6, qword bcst [eax]	 # AVX512F
	vgetexppd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vgetexppd	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vgetexppd	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vgetexppd	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vgetexppd	zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vgetexppd	zmm6, qword bcst [edx+1024]	 # AVX512F
	vgetexppd	zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vgetexppd	zmm6, qword bcst [edx-1032]	 # AVX512F

	vgetexpps	zmm6, zmm5	 # AVX512F
	vgetexpps	zmm6{k7}, zmm5	 # AVX512F
	vgetexpps	zmm6{k7}{z}, zmm5	 # AVX512F
	vgetexpps	zmm6, zmm5{sae}	 # AVX512F
	vgetexpps	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vgetexpps	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vgetexpps	zmm6, dword bcst [eax]	 # AVX512F
	vgetexpps	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vgetexpps	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vgetexpps	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vgetexpps	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vgetexpps	zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vgetexpps	zmm6, dword bcst [edx+512]	 # AVX512F
	vgetexpps	zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vgetexpps	zmm6, dword bcst [edx-516]	 # AVX512F

	vgetexpsd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vgetexpsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vgetexpsd	xmm6{k7}, xmm5, xmm4{sae}	 # AVX512F
	vgetexpsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vgetexpsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vgetexpsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vgetexpsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vgetexpsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vgetexpsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vgetexpss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vgetexpss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vgetexpss	xmm6{k7}, xmm5, xmm4{sae}	 # AVX512F
	vgetexpss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vgetexpss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vgetexpss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vgetexpss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vgetexpss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vgetexpss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vgetmantpd	zmm6, zmm5, 0xab	 # AVX512F
	vgetmantpd	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vgetmantpd	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vgetmantpd	zmm6, zmm5{sae}, 0xab	 # AVX512F
	vgetmantpd	zmm6, zmm5, 123	 # AVX512F
	vgetmantpd	zmm6, zmm5{sae}, 123	 # AVX512F
	vgetmantpd	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vgetmantpd	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vgetmantpd	zmm6, qword bcst [eax], 123	 # AVX512F
	vgetmantpd	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vgetmantpd	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vgetmantpd	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vgetmantpd	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vgetmantpd	zmm6, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vgetmantpd	zmm6, qword bcst [edx+1024], 123	 # AVX512F
	vgetmantpd	zmm6, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vgetmantpd	zmm6, qword bcst [edx-1032], 123	 # AVX512F

	vgetmantps	zmm6, zmm5, 0xab	 # AVX512F
	vgetmantps	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vgetmantps	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vgetmantps	zmm6, zmm5{sae}, 0xab	 # AVX512F
	vgetmantps	zmm6, zmm5, 123	 # AVX512F
	vgetmantps	zmm6, zmm5{sae}, 123	 # AVX512F
	vgetmantps	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vgetmantps	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vgetmantps	zmm6, dword bcst [eax], 123	 # AVX512F
	vgetmantps	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vgetmantps	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vgetmantps	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vgetmantps	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vgetmantps	zmm6, dword bcst [edx+508], 123	 # AVX512F Disp8
	vgetmantps	zmm6, dword bcst [edx+512], 123	 # AVX512F
	vgetmantps	zmm6, dword bcst [edx-512], 123	 # AVX512F Disp8
	vgetmantps	zmm6, dword bcst [edx-516], 123	 # AVX512F

	vgetmantsd	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512F
	vgetmantsd	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512F
	vgetmantsd	xmm6{k7}, xmm5, xmm4{sae}, 0xab	 # AVX512F
	vgetmantsd	xmm6{k7}, xmm5, xmm4, 123	 # AVX512F
	vgetmantsd	xmm6{k7}, xmm5, xmm4{sae}, 123	 # AVX512F
	vgetmantsd	xmm6{k7}, xmm5, QWORD PTR [ecx], 123	 # AVX512F
	vgetmantsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vgetmantsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016], 123	 # AVX512F Disp8
	vgetmantsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024], 123	 # AVX512F
	vgetmantsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024], 123	 # AVX512F Disp8
	vgetmantsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032], 123	 # AVX512F

	vgetmantss	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512F
	vgetmantss	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512F
	vgetmantss	xmm6{k7}, xmm5, xmm4{sae}, 0xab	 # AVX512F
	vgetmantss	xmm6{k7}, xmm5, xmm4, 123	 # AVX512F
	vgetmantss	xmm6{k7}, xmm5, xmm4{sae}, 123	 # AVX512F
	vgetmantss	xmm6{k7}, xmm5, DWORD PTR [ecx], 123	 # AVX512F
	vgetmantss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vgetmantss	xmm6{k7}, xmm5, DWORD PTR [edx+508], 123	 # AVX512F Disp8
	vgetmantss	xmm6{k7}, xmm5, DWORD PTR [edx+512], 123	 # AVX512F
	vgetmantss	xmm6{k7}, xmm5, DWORD PTR [edx-512], 123	 # AVX512F Disp8
	vgetmantss	xmm6{k7}, xmm5, DWORD PTR [edx-516], 123	 # AVX512F

	vinsertf32x4	zmm6{k7}, zmm5, xmm4, 0xab	 # AVX512F
	vinsertf32x4	zmm6{k7}{z}, zmm5, xmm4, 0xab	 # AVX512F
	vinsertf32x4	zmm6{k7}, zmm5, xmm4, 123	 # AVX512F
	vinsertf32x4	zmm6{k7}, zmm5, XMMWORD PTR [ecx], 123	 # AVX512F
	vinsertf32x4	zmm6{k7}, zmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vinsertf32x4	zmm6{k7}, zmm5, XMMWORD PTR [edx+2032], 123	 # AVX512F Disp8
	vinsertf32x4	zmm6{k7}, zmm5, XMMWORD PTR [edx+2048], 123	 # AVX512F
	vinsertf32x4	zmm6{k7}, zmm5, XMMWORD PTR [edx-2048], 123	 # AVX512F Disp8
	vinsertf32x4	zmm6{k7}, zmm5, XMMWORD PTR [edx-2064], 123	 # AVX512F

	vinsertf64x4	zmm6{k7}, zmm5, ymm4, 0xab	 # AVX512F
	vinsertf64x4	zmm6{k7}{z}, zmm5, ymm4, 0xab	 # AVX512F
	vinsertf64x4	zmm6{k7}, zmm5, ymm4, 123	 # AVX512F
	vinsertf64x4	zmm6{k7}, zmm5, YMMWORD PTR [ecx], 123	 # AVX512F
	vinsertf64x4	zmm6{k7}, zmm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vinsertf64x4	zmm6{k7}, zmm5, YMMWORD PTR [edx+4064], 123	 # AVX512F Disp8
	vinsertf64x4	zmm6{k7}, zmm5, YMMWORD PTR [edx+4096], 123	 # AVX512F
	vinsertf64x4	zmm6{k7}, zmm5, YMMWORD PTR [edx-4096], 123	 # AVX512F Disp8
	vinsertf64x4	zmm6{k7}, zmm5, YMMWORD PTR [edx-4128], 123	 # AVX512F

	vinserti32x4	zmm6{k7}, zmm5, xmm4, 0xab	 # AVX512F
	vinserti32x4	zmm6{k7}{z}, zmm5, xmm4, 0xab	 # AVX512F
	vinserti32x4	zmm6{k7}, zmm5, xmm4, 123	 # AVX512F
	vinserti32x4	zmm6{k7}, zmm5, XMMWORD PTR [ecx], 123	 # AVX512F
	vinserti32x4	zmm6{k7}, zmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vinserti32x4	zmm6{k7}, zmm5, XMMWORD PTR [edx+2032], 123	 # AVX512F Disp8
	vinserti32x4	zmm6{k7}, zmm5, XMMWORD PTR [edx+2048], 123	 # AVX512F
	vinserti32x4	zmm6{k7}, zmm5, XMMWORD PTR [edx-2048], 123	 # AVX512F Disp8
	vinserti32x4	zmm6{k7}, zmm5, XMMWORD PTR [edx-2064], 123	 # AVX512F

	vinserti64x4	zmm6{k7}, zmm5, ymm4, 0xab	 # AVX512F
	vinserti64x4	zmm6{k7}{z}, zmm5, ymm4, 0xab	 # AVX512F
	vinserti64x4	zmm6{k7}, zmm5, ymm4, 123	 # AVX512F
	vinserti64x4	zmm6{k7}, zmm5, YMMWORD PTR [ecx], 123	 # AVX512F
	vinserti64x4	zmm6{k7}, zmm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vinserti64x4	zmm6{k7}, zmm5, YMMWORD PTR [edx+4064], 123	 # AVX512F Disp8
	vinserti64x4	zmm6{k7}, zmm5, YMMWORD PTR [edx+4096], 123	 # AVX512F
	vinserti64x4	zmm6{k7}, zmm5, YMMWORD PTR [edx-4096], 123	 # AVX512F Disp8
	vinserti64x4	zmm6{k7}, zmm5, YMMWORD PTR [edx-4128], 123	 # AVX512F


	vmaxpd	zmm6, zmm5, zmm4	 # AVX512F
	vmaxpd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vmaxpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vmaxpd	zmm6, zmm5, zmm4{sae}	 # AVX512F
	vmaxpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vmaxpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmaxpd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vmaxpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmaxpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmaxpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmaxpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vmaxpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vmaxpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vmaxpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vmaxpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vmaxps	zmm6, zmm5, zmm4	 # AVX512F
	vmaxps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vmaxps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vmaxps	zmm6, zmm5, zmm4{sae}	 # AVX512F
	vmaxps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vmaxps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmaxps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vmaxps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmaxps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmaxps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmaxps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vmaxps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vmaxps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vmaxps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vmaxps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vmaxsd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vmaxsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vmaxsd	xmm6{k7}, xmm5, xmm4{sae}	 # AVX512F
	vmaxsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vmaxsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmaxsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vmaxsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vmaxsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vmaxsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vmaxss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vmaxss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vmaxss	xmm6{k7}, xmm5, xmm4{sae}	 # AVX512F
	vmaxss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vmaxss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmaxss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vmaxss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vmaxss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vmaxss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vminpd	zmm6, zmm5, zmm4	 # AVX512F
	vminpd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vminpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vminpd	zmm6, zmm5, zmm4{sae}	 # AVX512F
	vminpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vminpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vminpd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vminpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vminpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vminpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vminpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vminpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vminpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vminpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vminpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vminps	zmm6, zmm5, zmm4	 # AVX512F
	vminps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vminps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vminps	zmm6, zmm5, zmm4{sae}	 # AVX512F
	vminps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vminps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vminps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vminps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vminps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vminps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vminps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vminps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vminps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vminps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vminps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vminsd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vminsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vminsd	xmm6{k7}, xmm5, xmm4{sae}	 # AVX512F
	vminsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vminsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vminsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vminsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vminsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vminsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vminss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vminss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vminss	xmm6{k7}, xmm5, xmm4{sae}	 # AVX512F
	vminss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vminss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vminss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vminss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vminss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vminss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vmovapd	zmm6, zmm5	 # AVX512F
	vmovapd	zmm6{k7}, zmm5	 # AVX512F
	vmovapd	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovapd	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vmovapd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmovapd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmovapd	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmovapd	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmovapd	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F

	vmovaps	zmm6, zmm5	 # AVX512F
	vmovaps	zmm6{k7}, zmm5	 # AVX512F
	vmovaps	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovaps	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vmovaps	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmovaps	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmovaps	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmovaps	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmovaps	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F



	vmovddup	zmm6, zmm5	 # AVX512F
	vmovddup	zmm6{k7}, zmm5	 # AVX512F
	vmovddup	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovddup	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vmovddup	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmovddup	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmovddup	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmovddup	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmovddup	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F

	vmovdqa32	zmm6, zmm5	 # AVX512F
	vmovdqa32	zmm6{k7}, zmm5	 # AVX512F
	vmovdqa32	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovdqa32	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vmovdqa32	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmovdqa32	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmovdqa32	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmovdqa32	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmovdqa32	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F

	vmovdqa64	zmm6, zmm5	 # AVX512F
	vmovdqa64	zmm6{k7}, zmm5	 # AVX512F
	vmovdqa64	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovdqa64	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vmovdqa64	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmovdqa64	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmovdqa64	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmovdqa64	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmovdqa64	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F

	vmovdqu32	zmm6, zmm5	 # AVX512F
	vmovdqu32	zmm6{k7}, zmm5	 # AVX512F
	vmovdqu32	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovdqu32	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vmovdqu32	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmovdqu32	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmovdqu32	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmovdqu32	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmovdqu32	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F

	vmovdqu64	zmm6, zmm5	 # AVX512F
	vmovdqu64	zmm6{k7}, zmm5	 # AVX512F
	vmovdqu64	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovdqu64	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vmovdqu64	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmovdqu64	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmovdqu64	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmovdqu64	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmovdqu64	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F











	vmovntdq	ZMMWORD PTR [ecx], zmm6	 # AVX512F
	vmovntdq	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vmovntdq	ZMMWORD PTR [edx+8128], zmm6	 # AVX512F Disp8
	vmovntdq	ZMMWORD PTR [edx+8192], zmm6	 # AVX512F
	vmovntdq	ZMMWORD PTR [edx-8192], zmm6	 # AVX512F Disp8
	vmovntdq	ZMMWORD PTR [edx-8256], zmm6	 # AVX512F

	vmovntdqa	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vmovntdqa	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmovntdqa	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmovntdqa	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmovntdqa	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmovntdqa	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F

	vmovntpd	ZMMWORD PTR [ecx], zmm6	 # AVX512F
	vmovntpd	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vmovntpd	ZMMWORD PTR [edx+8128], zmm6	 # AVX512F Disp8
	vmovntpd	ZMMWORD PTR [edx+8192], zmm6	 # AVX512F
	vmovntpd	ZMMWORD PTR [edx-8192], zmm6	 # AVX512F Disp8
	vmovntpd	ZMMWORD PTR [edx-8256], zmm6	 # AVX512F

	vmovntps	ZMMWORD PTR [ecx], zmm6	 # AVX512F
	vmovntps	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vmovntps	ZMMWORD PTR [edx+8128], zmm6	 # AVX512F Disp8
	vmovntps	ZMMWORD PTR [edx+8192], zmm6	 # AVX512F
	vmovntps	ZMMWORD PTR [edx-8192], zmm6	 # AVX512F Disp8
	vmovntps	ZMMWORD PTR [edx-8256], zmm6	 # AVX512F

	vmovsd	xmm6{k7}, QWORD PTR [ecx]	 # AVX512F
	vmovsd	xmm6{k7}{z}, QWORD PTR [ecx]	 # AVX512F
	vmovsd	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmovsd	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vmovsd	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512F
	vmovsd	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vmovsd	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512F

	vmovsd	QWORD PTR [ecx]{k7}, xmm6	 # AVX512F
	vmovsd	QWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512F
	vmovsd	QWORD PTR [edx+1016]{k7}, xmm6	 # AVX512F Disp8
	vmovsd	QWORD PTR [edx+1024]{k7}, xmm6	 # AVX512F
	vmovsd	QWORD PTR [edx-1024]{k7}, xmm6	 # AVX512F Disp8
	vmovsd	QWORD PTR [edx-1032]{k7}, xmm6	 # AVX512F

	vmovsd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vmovsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F

	vmovshdup	zmm6, zmm5	 # AVX512F
	vmovshdup	zmm6{k7}, zmm5	 # AVX512F
	vmovshdup	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovshdup	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vmovshdup	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmovshdup	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmovshdup	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmovshdup	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmovshdup	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F

	vmovsldup	zmm6, zmm5	 # AVX512F
	vmovsldup	zmm6{k7}, zmm5	 # AVX512F
	vmovsldup	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovsldup	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vmovsldup	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmovsldup	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmovsldup	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmovsldup	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmovsldup	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F

	vmovss	xmm6{k7}, DWORD PTR [ecx]	 # AVX512F
	vmovss	xmm6{k7}{z}, DWORD PTR [ecx]	 # AVX512F
	vmovss	xmm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmovss	xmm6{k7}, DWORD PTR [edx+508]	 # AVX512F Disp8
	vmovss	xmm6{k7}, DWORD PTR [edx+512]	 # AVX512F
	vmovss	xmm6{k7}, DWORD PTR [edx-512]	 # AVX512F Disp8
	vmovss	xmm6{k7}, DWORD PTR [edx-516]	 # AVX512F

	vmovss	DWORD PTR [ecx]{k7}, xmm6	 # AVX512F
	vmovss	DWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512F
	vmovss	DWORD PTR [edx+508]{k7}, xmm6	 # AVX512F Disp8
	vmovss	DWORD PTR [edx+512]{k7}, xmm6	 # AVX512F
	vmovss	DWORD PTR [edx-512]{k7}, xmm6	 # AVX512F Disp8
	vmovss	DWORD PTR [edx-516]{k7}, xmm6	 # AVX512F

	vmovss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vmovss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F

	vmovupd	zmm6, zmm5	 # AVX512F
	vmovupd	zmm6{k7}, zmm5	 # AVX512F
	vmovupd	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovupd	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vmovupd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmovupd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmovupd	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmovupd	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmovupd	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F

	vmovups	zmm6, zmm5	 # AVX512F
	vmovups	zmm6{k7}, zmm5	 # AVX512F
	vmovups	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovups	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vmovups	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmovups	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmovups	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmovups	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmovups	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F

	vmulpd	zmm6, zmm5, zmm4	 # AVX512F
	vmulpd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vmulpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vmulpd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vmulpd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vmulpd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vmulpd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vmulpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vmulpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmulpd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vmulpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmulpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmulpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmulpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vmulpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vmulpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vmulpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vmulpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vmulps	zmm6, zmm5, zmm4	 # AVX512F
	vmulps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vmulps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vmulps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vmulps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vmulps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vmulps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vmulps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vmulps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmulps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vmulps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vmulps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vmulps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vmulps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vmulps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vmulps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vmulps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vmulps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vmulsd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vmulsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vmulsd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vmulsd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vmulsd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vmulsd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vmulsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vmulsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmulsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vmulsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vmulsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vmulsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vmulss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vmulss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vmulss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vmulss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vmulss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vmulss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vmulss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vmulss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vmulss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vmulss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vmulss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vmulss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vpabsd	zmm6, zmm5	 # AVX512F
	vpabsd	zmm6{k7}, zmm5	 # AVX512F
	vpabsd	zmm6{k7}{z}, zmm5	 # AVX512F
	vpabsd	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpabsd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpabsd	zmm6, dword bcst [eax]	 # AVX512F
	vpabsd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpabsd	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpabsd	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpabsd	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpabsd	zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vpabsd	zmm6, dword bcst [edx+512]	 # AVX512F
	vpabsd	zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vpabsd	zmm6, dword bcst [edx-516]	 # AVX512F

	vpabsq	zmm6, zmm5	 # AVX512F
	vpabsq	zmm6{k7}, zmm5	 # AVX512F
	vpabsq	zmm6{k7}{z}, zmm5	 # AVX512F
	vpabsq	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpabsq	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpabsq	zmm6, qword bcst [eax]	 # AVX512F
	vpabsq	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpabsq	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpabsq	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpabsq	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpabsq	zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vpabsq	zmm6, qword bcst [edx+1024]	 # AVX512F
	vpabsq	zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vpabsq	zmm6, qword bcst [edx-1032]	 # AVX512F

	vpaddd	zmm6, zmm5, zmm4	 # AVX512F
	vpaddd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpaddd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpaddd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpaddd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpaddd	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpaddd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpaddd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpaddd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpaddd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpaddd	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpaddd	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpaddd	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpaddd	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpaddq	zmm6, zmm5, zmm4	 # AVX512F
	vpaddq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpaddq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpaddq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpaddq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpaddq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpaddq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpaddq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpaddq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpaddq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpaddq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpaddq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpaddq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpaddq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpandd	zmm6, zmm5, zmm4	 # AVX512F
	vpandd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpandd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpandd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpandd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpandd	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpandd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpandd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpandd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpandd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpandd	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpandd	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpandd	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpandd	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpandnd	zmm6, zmm5, zmm4	 # AVX512F
	vpandnd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpandnd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpandnd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpandnd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpandnd	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpandnd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpandnd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpandnd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpandnd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpandnd	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpandnd	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpandnd	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpandnd	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpandnq	zmm6, zmm5, zmm4	 # AVX512F
	vpandnq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpandnq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpandnq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpandnq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpandnq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpandnq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpandnq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpandnq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpandnq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpandnq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpandnq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpandnq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpandnq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpandq	zmm6, zmm5, zmm4	 # AVX512F
	vpandq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpandq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpandq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpandq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpandq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpandq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpandq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpandq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpandq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpandq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpandq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpandq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpandq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpblendmd	zmm6, zmm5, zmm4	 # AVX512F
	vpblendmd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpblendmd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpblendmd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpblendmd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpblendmd	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpblendmd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpblendmd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpblendmd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpblendmd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpblendmd	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpblendmd	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpblendmd	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpblendmd	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpbroadcastd	zmm6, DWORD PTR [ecx]	 # AVX512F
	vpbroadcastd	zmm6{k7}, DWORD PTR [ecx]	 # AVX512F
	vpbroadcastd	zmm6{k7}{z}, DWORD PTR [ecx]	 # AVX512F
	vpbroadcastd	zmm6, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpbroadcastd	zmm6, DWORD PTR [edx+508]	 # AVX512F Disp8
	vpbroadcastd	zmm6, DWORD PTR [edx+512]	 # AVX512F
	vpbroadcastd	zmm6, DWORD PTR [edx-512]	 # AVX512F Disp8
	vpbroadcastd	zmm6, DWORD PTR [edx-516]	 # AVX512F

	vpbroadcastd	zmm6{k7}, xmm5	 # AVX512F
	vpbroadcastd	zmm6{k7}{z}, xmm5	 # AVX512F

	vpbroadcastd	zmm6, eax	 # AVX512F
	vpbroadcastd	zmm6{k7}, eax	 # AVX512F
	vpbroadcastd	zmm6{k7}{z}, eax	 # AVX512F
	vpbroadcastd	zmm6, ebp	 # AVX512F

	vpbroadcastq	zmm6, QWORD PTR [ecx]	 # AVX512F
	vpbroadcastq	zmm6{k7}, QWORD PTR [ecx]	 # AVX512F
	vpbroadcastq	zmm6{k7}{z}, QWORD PTR [ecx]	 # AVX512F
	vpbroadcastq	zmm6, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpbroadcastq	zmm6, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vpbroadcastq	zmm6, QWORD PTR [edx+1024]	 # AVX512F
	vpbroadcastq	zmm6, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vpbroadcastq	zmm6, QWORD PTR [edx-1032]	 # AVX512F

	vpbroadcastq	zmm6{k7}, xmm5	 # AVX512F
	vpbroadcastq	zmm6{k7}{z}, xmm5	 # AVX512F

	vpcmpd	k5, zmm6, zmm5, 0xab	 # AVX512F
	vpcmpd	k5{k7}, zmm6, zmm5, 0xab	 # AVX512F
	vpcmpd	k5, zmm6, zmm5, 123	 # AVX512F
	vpcmpd	k5, zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpcmpd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpcmpd	k5, zmm6, dword bcst [eax], 123	 # AVX512F
	vpcmpd	k5, zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpcmpd	k5, zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpcmpd	k5, zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpcmpd	k5, zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpcmpd	k5, zmm6, dword bcst [edx+508], 123	 # AVX512F Disp8
	vpcmpd	k5, zmm6, dword bcst [edx+512], 123	 # AVX512F
	vpcmpd	k5, zmm6, dword bcst [edx-512], 123	 # AVX512F Disp8
	vpcmpd	k5, zmm6, dword bcst [edx-516], 123	 # AVX512F

	vpcmpltd	k5, zmm6, zmm5	 # AVX512F
	vpcmpltd	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpltd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpltd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpltd	k5, zmm6, dword bcst [eax]	 # AVX512F
	vpcmpltd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpltd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpltd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpltd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpltd	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vpcmpltd	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vpcmpltd	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vpcmpltd	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vpcmpled	k5, zmm6, zmm5	 # AVX512F
	vpcmpled	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpled	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpled	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpled	k5, zmm6, dword bcst [eax]	 # AVX512F
	vpcmpled	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpled	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpled	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpled	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpled	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vpcmpled	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vpcmpled	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vpcmpled	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vpcmpneqd	k5, zmm6, zmm5	 # AVX512F
	vpcmpneqd	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpneqd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpneqd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpneqd	k5, zmm6, dword bcst [eax]	 # AVX512F
	vpcmpneqd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpneqd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpneqd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpneqd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpneqd	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vpcmpneqd	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vpcmpneqd	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vpcmpneqd	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vpcmpnltd	k5, zmm6, zmm5	 # AVX512F
	vpcmpnltd	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpnltd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpnltd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpnltd	k5, zmm6, dword bcst [eax]	 # AVX512F
	vpcmpnltd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpnltd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpnltd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpnltd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpnltd	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vpcmpnltd	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vpcmpnltd	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vpcmpnltd	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vpcmpnled	k5, zmm6, zmm5	 # AVX512F
	vpcmpnled	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpnled	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpnled	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpnled	k5, zmm6, dword bcst [eax]	 # AVX512F
	vpcmpnled	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpnled	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpnled	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpnled	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpnled	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vpcmpnled	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vpcmpnled	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vpcmpnled	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vpcmpeqd	k5, zmm6, zmm5	 # AVX512F
	vpcmpeqd	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpeqd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpeqd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpeqd	k5, zmm6, dword bcst [eax]	 # AVX512F
	vpcmpeqd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpeqd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpeqd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpeqd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpeqd	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vpcmpeqd	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vpcmpeqd	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vpcmpeqd	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vpcmpeqq	k5, zmm6, zmm5	 # AVX512F
	vpcmpeqq	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpeqq	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpeqq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpeqq	k5, zmm6, qword bcst [eax]	 # AVX512F
	vpcmpeqq	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpeqq	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpeqq	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpeqq	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpeqq	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vpcmpeqq	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vpcmpeqq	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vpcmpeqq	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vpcmpgtd	k5, zmm6, zmm5	 # AVX512F
	vpcmpgtd	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpgtd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpgtd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpgtd	k5, zmm6, dword bcst [eax]	 # AVX512F
	vpcmpgtd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpgtd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpgtd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpgtd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpgtd	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vpcmpgtd	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vpcmpgtd	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vpcmpgtd	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vpcmpgtq	k5, zmm6, zmm5	 # AVX512F
	vpcmpgtq	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpgtq	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpgtq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpgtq	k5, zmm6, qword bcst [eax]	 # AVX512F
	vpcmpgtq	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpgtq	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpgtq	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpgtq	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpgtq	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vpcmpgtq	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vpcmpgtq	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vpcmpgtq	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vpcmpq	k5, zmm6, zmm5, 0xab	 # AVX512F
	vpcmpq	k5{k7}, zmm6, zmm5, 0xab	 # AVX512F
	vpcmpq	k5, zmm6, zmm5, 123	 # AVX512F
	vpcmpq	k5, zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpcmpq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpcmpq	k5, zmm6, qword bcst [eax], 123	 # AVX512F
	vpcmpq	k5, zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpcmpq	k5, zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpcmpq	k5, zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpcmpq	k5, zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpcmpq	k5, zmm6, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vpcmpq	k5, zmm6, qword bcst [edx+1024], 123	 # AVX512F
	vpcmpq	k5, zmm6, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vpcmpq	k5, zmm6, qword bcst [edx-1032], 123	 # AVX512F

	vpcmpltq	k5, zmm6, zmm5	 # AVX512F
	vpcmpltq	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpltq	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpltq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpltq	k5, zmm6, qword bcst [eax]	 # AVX512F
	vpcmpltq	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpltq	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpltq	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpltq	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpltq	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vpcmpltq	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vpcmpltq	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vpcmpltq	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vpcmpleq	k5, zmm6, zmm5	 # AVX512F
	vpcmpleq	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpleq	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpleq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpleq	k5, zmm6, qword bcst [eax]	 # AVX512F
	vpcmpleq	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpleq	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpleq	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpleq	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpleq	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vpcmpleq	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vpcmpleq	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vpcmpleq	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vpcmpneqq	k5, zmm6, zmm5	 # AVX512F
	vpcmpneqq	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpneqq	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpneqq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpneqq	k5, zmm6, qword bcst [eax]	 # AVX512F
	vpcmpneqq	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpneqq	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpneqq	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpneqq	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpneqq	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vpcmpneqq	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vpcmpneqq	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vpcmpneqq	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vpcmpnltq	k5, zmm6, zmm5	 # AVX512F
	vpcmpnltq	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpnltq	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpnltq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpnltq	k5, zmm6, qword bcst [eax]	 # AVX512F
	vpcmpnltq	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpnltq	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpnltq	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpnltq	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpnltq	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vpcmpnltq	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vpcmpnltq	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vpcmpnltq	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vpcmpnleq	k5, zmm6, zmm5	 # AVX512F
	vpcmpnleq	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpnleq	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpnleq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpnleq	k5, zmm6, qword bcst [eax]	 # AVX512F
	vpcmpnleq	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpnleq	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpnleq	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpnleq	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpnleq	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vpcmpnleq	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vpcmpnleq	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vpcmpnleq	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vpcmpud	k5, zmm6, zmm5, 0xab	 # AVX512F
	vpcmpud	k5{k7}, zmm6, zmm5, 0xab	 # AVX512F
	vpcmpud	k5, zmm6, zmm5, 123	 # AVX512F
	vpcmpud	k5, zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpcmpud	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpcmpud	k5, zmm6, dword bcst [eax], 123	 # AVX512F
	vpcmpud	k5, zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpcmpud	k5, zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpcmpud	k5, zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpcmpud	k5, zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpcmpud	k5, zmm6, dword bcst [edx+508], 123	 # AVX512F Disp8
	vpcmpud	k5, zmm6, dword bcst [edx+512], 123	 # AVX512F
	vpcmpud	k5, zmm6, dword bcst [edx-512], 123	 # AVX512F Disp8
	vpcmpud	k5, zmm6, dword bcst [edx-516], 123	 # AVX512F

	vpcmpequd	k5, zmm6, zmm5	 # AVX512F
	vpcmpequd	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpequd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpequd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpequd	k5, zmm6, dword bcst [eax]	 # AVX512F
	vpcmpequd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpequd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpequd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpequd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpequd	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vpcmpequd	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vpcmpequd	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vpcmpequd	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vpcmpltud	k5, zmm6, zmm5	 # AVX512F
	vpcmpltud	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpltud	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpltud	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpltud	k5, zmm6, dword bcst [eax]	 # AVX512F
	vpcmpltud	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpltud	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpltud	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpltud	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpltud	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vpcmpltud	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vpcmpltud	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vpcmpltud	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vpcmpleud	k5, zmm6, zmm5	 # AVX512F
	vpcmpleud	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpleud	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpleud	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpleud	k5, zmm6, dword bcst [eax]	 # AVX512F
	vpcmpleud	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpleud	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpleud	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpleud	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpleud	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vpcmpleud	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vpcmpleud	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vpcmpleud	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vpcmpnequd	k5, zmm6, zmm5	 # AVX512F
	vpcmpnequd	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpnequd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpnequd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpnequd	k5, zmm6, dword bcst [eax]	 # AVX512F
	vpcmpnequd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpnequd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpnequd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpnequd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpnequd	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vpcmpnequd	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vpcmpnequd	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vpcmpnequd	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vpcmpnltud	k5, zmm6, zmm5	 # AVX512F
	vpcmpnltud	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpnltud	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpnltud	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpnltud	k5, zmm6, dword bcst [eax]	 # AVX512F
	vpcmpnltud	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpnltud	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpnltud	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpnltud	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpnltud	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vpcmpnltud	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vpcmpnltud	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vpcmpnltud	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vpcmpnleud	k5, zmm6, zmm5	 # AVX512F
	vpcmpnleud	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpnleud	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpnleud	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpnleud	k5, zmm6, dword bcst [eax]	 # AVX512F
	vpcmpnleud	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpnleud	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpnleud	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpnleud	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpnleud	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vpcmpnleud	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vpcmpnleud	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vpcmpnleud	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vpcmpuq	k5, zmm6, zmm5, 0xab	 # AVX512F
	vpcmpuq	k5{k7}, zmm6, zmm5, 0xab	 # AVX512F
	vpcmpuq	k5, zmm6, zmm5, 123	 # AVX512F
	vpcmpuq	k5, zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpcmpuq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpcmpuq	k5, zmm6, qword bcst [eax], 123	 # AVX512F
	vpcmpuq	k5, zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpcmpuq	k5, zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpcmpuq	k5, zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpcmpuq	k5, zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpcmpuq	k5, zmm6, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vpcmpuq	k5, zmm6, qword bcst [edx+1024], 123	 # AVX512F
	vpcmpuq	k5, zmm6, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vpcmpuq	k5, zmm6, qword bcst [edx-1032], 123	 # AVX512F

	vpcmpequq	k5, zmm6, zmm5	 # AVX512F
	vpcmpequq	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpequq	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpequq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpequq	k5, zmm6, qword bcst [eax]	 # AVX512F
	vpcmpequq	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpequq	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpequq	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpequq	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpequq	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vpcmpequq	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vpcmpequq	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vpcmpequq	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vpcmpltuq	k5, zmm6, zmm5	 # AVX512F
	vpcmpltuq	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpltuq	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpltuq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpltuq	k5, zmm6, qword bcst [eax]	 # AVX512F
	vpcmpltuq	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpltuq	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpltuq	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpltuq	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpltuq	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vpcmpltuq	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vpcmpltuq	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vpcmpltuq	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vpcmpleuq	k5, zmm6, zmm5	 # AVX512F
	vpcmpleuq	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpleuq	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpleuq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpleuq	k5, zmm6, qword bcst [eax]	 # AVX512F
	vpcmpleuq	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpleuq	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpleuq	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpleuq	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpleuq	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vpcmpleuq	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vpcmpleuq	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vpcmpleuq	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vpcmpnequq	k5, zmm6, zmm5	 # AVX512F
	vpcmpnequq	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpnequq	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpnequq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpnequq	k5, zmm6, qword bcst [eax]	 # AVX512F
	vpcmpnequq	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpnequq	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpnequq	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpnequq	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpnequq	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vpcmpnequq	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vpcmpnequq	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vpcmpnequq	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vpcmpnltuq	k5, zmm6, zmm5	 # AVX512F
	vpcmpnltuq	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpnltuq	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpnltuq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpnltuq	k5, zmm6, qword bcst [eax]	 # AVX512F
	vpcmpnltuq	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpnltuq	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpnltuq	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpnltuq	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpnltuq	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vpcmpnltuq	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vpcmpnltuq	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vpcmpnltuq	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vpcmpnleuq	k5, zmm6, zmm5	 # AVX512F
	vpcmpnleuq	k5{k7}, zmm6, zmm5	 # AVX512F
	vpcmpnleuq	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpcmpnleuq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpcmpnleuq	k5, zmm6, qword bcst [eax]	 # AVX512F
	vpcmpnleuq	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpcmpnleuq	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpcmpnleuq	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpcmpnleuq	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpcmpnleuq	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vpcmpnleuq	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vpcmpnleuq	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vpcmpnleuq	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vpblendmq	zmm6, zmm5, zmm4	 # AVX512F
	vpblendmq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpblendmq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpblendmq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpblendmq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpblendmq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpblendmq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpblendmq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpblendmq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpblendmq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpblendmq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpblendmq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpblendmq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpblendmq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpcompressd	ZMMWORD PTR [ecx], zmm6	 # AVX512F
	vpcompressd	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpcompressd	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpcompressd	ZMMWORD PTR [edx+508], zmm6	 # AVX512F Disp8
	vpcompressd	ZMMWORD PTR [edx+512], zmm6	 # AVX512F
	vpcompressd	ZMMWORD PTR [edx-512], zmm6	 # AVX512F Disp8
	vpcompressd	ZMMWORD PTR [edx-516], zmm6	 # AVX512F

	vpcompressd	zmm6, zmm5	 # AVX512F
	vpcompressd	zmm6{k7}, zmm5	 # AVX512F
	vpcompressd	zmm6{k7}{z}, zmm5	 # AVX512F

	vpermd	zmm6, zmm5, zmm4	 # AVX512F
	vpermd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpermd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpermd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpermd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpermd	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpermd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpermd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpermd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpermd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpermd	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpermd	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpermd	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpermd	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpermilpd	zmm6, zmm5, 0xab	 # AVX512F
	vpermilpd	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vpermilpd	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vpermilpd	zmm6, zmm5, 123	 # AVX512F
	vpermilpd	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpermilpd	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpermilpd	zmm6, qword bcst [eax], 123	 # AVX512F
	vpermilpd	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpermilpd	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpermilpd	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpermilpd	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpermilpd	zmm6, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vpermilpd	zmm6, qword bcst [edx+1024], 123	 # AVX512F
	vpermilpd	zmm6, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vpermilpd	zmm6, qword bcst [edx-1032], 123	 # AVX512F

	vpermilpd	zmm6, zmm5, zmm4	 # AVX512F
	vpermilpd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpermilpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpermilpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpermilpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpermilpd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpermilpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpermilpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpermilpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpermilpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpermilpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpermilpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpermilpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpermilpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpermilps	zmm6, zmm5, 0xab	 # AVX512F
	vpermilps	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vpermilps	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vpermilps	zmm6, zmm5, 123	 # AVX512F
	vpermilps	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpermilps	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpermilps	zmm6, dword bcst [eax], 123	 # AVX512F
	vpermilps	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpermilps	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpermilps	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpermilps	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpermilps	zmm6, dword bcst [edx+508], 123	 # AVX512F Disp8
	vpermilps	zmm6, dword bcst [edx+512], 123	 # AVX512F
	vpermilps	zmm6, dword bcst [edx-512], 123	 # AVX512F Disp8
	vpermilps	zmm6, dword bcst [edx-516], 123	 # AVX512F

	vpermilps	zmm6, zmm5, zmm4	 # AVX512F
	vpermilps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpermilps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpermilps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpermilps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpermilps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpermilps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpermilps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpermilps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpermilps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpermilps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpermilps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpermilps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpermilps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpermpd	zmm6, zmm5, 0xab	 # AVX512F
	vpermpd	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vpermpd	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vpermpd	zmm6, zmm5, 123	 # AVX512F
	vpermpd	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpermpd	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpermpd	zmm6, qword bcst [eax], 123	 # AVX512F
	vpermpd	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpermpd	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpermpd	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpermpd	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpermpd	zmm6, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vpermpd	zmm6, qword bcst [edx+1024], 123	 # AVX512F
	vpermpd	zmm6, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vpermpd	zmm6, qword bcst [edx-1032], 123	 # AVX512F

	vpermps	zmm6, zmm5, zmm4	 # AVX512F
	vpermps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpermps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpermps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpermps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpermps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpermps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpermps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpermps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpermps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpermps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpermps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpermps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpermps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpermq	zmm6, zmm5, 0xab	 # AVX512F
	vpermq	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vpermq	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vpermq	zmm6, zmm5, 123	 # AVX512F
	vpermq	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpermq	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpermq	zmm6, qword bcst [eax], 123	 # AVX512F
	vpermq	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpermq	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpermq	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpermq	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpermq	zmm6, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vpermq	zmm6, qword bcst [edx+1024], 123	 # AVX512F
	vpermq	zmm6, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vpermq	zmm6, qword bcst [edx-1032], 123	 # AVX512F

	vpexpandd	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpexpandd	zmm6{k7}, ZMMWORD PTR [ecx]	 # AVX512F
	vpexpandd	zmm6{k7}{z}, ZMMWORD PTR [ecx]	 # AVX512F
	vpexpandd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpexpandd	zmm6, ZMMWORD PTR [edx+508]	 # AVX512F Disp8
	vpexpandd	zmm6, ZMMWORD PTR [edx+512]	 # AVX512F
	vpexpandd	zmm6, ZMMWORD PTR [edx-512]	 # AVX512F Disp8
	vpexpandd	zmm6, ZMMWORD PTR [edx-516]	 # AVX512F

	vpexpandd	zmm6, zmm5	 # AVX512F
	vpexpandd	zmm6{k7}, zmm5	 # AVX512F
	vpexpandd	zmm6{k7}{z}, zmm5	 # AVX512F

	vpexpandq	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vpexpandq	zmm6{k7}, ZMMWORD PTR [ecx]	 # AVX512F
	vpexpandq	zmm6{k7}{z}, ZMMWORD PTR [ecx]	 # AVX512F
	vpexpandq	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpexpandq	zmm6, ZMMWORD PTR [edx+1016]	 # AVX512F Disp8
	vpexpandq	zmm6, ZMMWORD PTR [edx+1024]	 # AVX512F
	vpexpandq	zmm6, ZMMWORD PTR [edx-1024]	 # AVX512F Disp8
	vpexpandq	zmm6, ZMMWORD PTR [edx-1032]	 # AVX512F

	vpexpandq	zmm6, zmm5	 # AVX512F
	vpexpandq	zmm6{k7}, zmm5	 # AVX512F
	vpexpandq	zmm6{k7}{z}, zmm5	 # AVX512F

	vpgatherdd	zmm6{k1}, [ebp+zmm7*8-123]	 # AVX512F
	vpgatherdd	zmm6{k1}, dword ptr [ebp+zmm7*8-123]	 # AVX512F
	vpgatherdd	zmm6{k1}, [eax+zmm7+256]	 # AVX512F
	vpgatherdd	zmm6{k1}, [ecx+zmm7*4+1024]	 # AVX512F

	vpgatherdq	zmm6{k1}, [ebp+ymm7*8-123]	 # AVX512F
	vpgatherdq	zmm6{k1}, qword ptr [ebp+ymm7*8-123]	 # AVX512F
	vpgatherdq	zmm6{k1}, [eax+ymm7+256]	 # AVX512F
	vpgatherdq	zmm6{k1}, [ecx+ymm7*4+1024]	 # AVX512F

	vpgatherqd	ymm6{k1}, [ebp+zmm7*8-123]	 # AVX512F
	vpgatherqd	ymm6{k1}, dword ptr [ebp+zmm7*8-123]	 # AVX512F
	vpgatherqd	ymm6{k1}, [eax+zmm7+256]	 # AVX512F
	vpgatherqd	ymm6{k1}, [ecx+zmm7*4+1024]	 # AVX512F

	vpgatherqq	zmm6{k1}, [ebp+zmm7*8-123]	 # AVX512F
	vpgatherqq	zmm6{k1}, qword ptr [ebp+zmm7*8-123]	 # AVX512F
	vpgatherqq	zmm6{k1}, [eax+zmm7+256]	 # AVX512F
	vpgatherqq	zmm6{k1}, [ecx+zmm7*4+1024]	 # AVX512F

	vpmaxsd	zmm6, zmm5, zmm4	 # AVX512F
	vpmaxsd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpmaxsd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpmaxsd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpmaxsd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmaxsd	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpmaxsd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpmaxsd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpmaxsd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpmaxsd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpmaxsd	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpmaxsd	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpmaxsd	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpmaxsd	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpmaxsq	zmm6, zmm5, zmm4	 # AVX512F
	vpmaxsq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpmaxsq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpmaxsq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpmaxsq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmaxsq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpmaxsq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpmaxsq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpmaxsq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpmaxsq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpmaxsq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpmaxsq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpmaxsq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpmaxsq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpmaxud	zmm6, zmm5, zmm4	 # AVX512F
	vpmaxud	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpmaxud	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpmaxud	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpmaxud	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmaxud	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpmaxud	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpmaxud	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpmaxud	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpmaxud	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpmaxud	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpmaxud	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpmaxud	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpmaxud	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpmaxuq	zmm6, zmm5, zmm4	 # AVX512F
	vpmaxuq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpmaxuq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpmaxuq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpmaxuq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmaxuq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpmaxuq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpmaxuq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpmaxuq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpmaxuq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpmaxuq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpmaxuq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpmaxuq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpmaxuq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpminsd	zmm6, zmm5, zmm4	 # AVX512F
	vpminsd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpminsd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpminsd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpminsd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpminsd	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpminsd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpminsd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpminsd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpminsd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpminsd	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpminsd	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpminsd	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpminsd	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpminsq	zmm6, zmm5, zmm4	 # AVX512F
	vpminsq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpminsq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpminsq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpminsq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpminsq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpminsq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpminsq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpminsq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpminsq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpminsq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpminsq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpminsq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpminsq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpminud	zmm6, zmm5, zmm4	 # AVX512F
	vpminud	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpminud	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpminud	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpminud	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpminud	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpminud	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpminud	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpminud	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpminud	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpminud	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpminud	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpminud	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpminud	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpminuq	zmm6, zmm5, zmm4	 # AVX512F
	vpminuq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpminuq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpminuq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpminuq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpminuq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpminuq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpminuq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpminuq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpminuq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpminuq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpminuq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpminuq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpminuq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpmovsxbd	zmm6{k7}, xmm5	 # AVX512F
	vpmovsxbd	zmm6{k7}{z}, xmm5	 # AVX512F
	vpmovsxbd	zmm6{k7}, XMMWORD PTR [ecx]	 # AVX512F
	vpmovsxbd	zmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmovsxbd	zmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512F Disp8
	vpmovsxbd	zmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512F
	vpmovsxbd	zmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512F Disp8
	vpmovsxbd	zmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512F

	vpmovsxbq	zmm6{k7}, xmm5	 # AVX512F
	vpmovsxbq	zmm6{k7}{z}, xmm5	 # AVX512F
	vpmovsxbq	zmm6{k7}, QWORD PTR [ecx]	 # AVX512F
	vpmovsxbq	zmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmovsxbq	zmm6{k7}, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vpmovsxbq	zmm6{k7}, QWORD PTR [edx+1024]	 # AVX512F
	vpmovsxbq	zmm6{k7}, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vpmovsxbq	zmm6{k7}, QWORD PTR [edx-1032]	 # AVX512F

	vpmovsxdq	zmm6{k7}, ymm5	 # AVX512F
	vpmovsxdq	zmm6{k7}{z}, ymm5	 # AVX512F
	vpmovsxdq	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512F
	vpmovsxdq	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmovsxdq	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512F Disp8
	vpmovsxdq	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512F
	vpmovsxdq	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512F Disp8
	vpmovsxdq	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512F

	vpmovsxwd	zmm6{k7}, ymm5	 # AVX512F
	vpmovsxwd	zmm6{k7}{z}, ymm5	 # AVX512F
	vpmovsxwd	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512F
	vpmovsxwd	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmovsxwd	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512F Disp8
	vpmovsxwd	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512F
	vpmovsxwd	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512F Disp8
	vpmovsxwd	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512F

	vpmovsxwq	zmm6{k7}, xmm5	 # AVX512F
	vpmovsxwq	zmm6{k7}{z}, xmm5	 # AVX512F
	vpmovsxwq	zmm6{k7}, XMMWORD PTR [ecx]	 # AVX512F
	vpmovsxwq	zmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmovsxwq	zmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512F Disp8
	vpmovsxwq	zmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512F
	vpmovsxwq	zmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512F Disp8
	vpmovsxwq	zmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512F

	vpmovzxbd	zmm6{k7}, xmm5	 # AVX512F
	vpmovzxbd	zmm6{k7}{z}, xmm5	 # AVX512F
	vpmovzxbd	zmm6{k7}, XMMWORD PTR [ecx]	 # AVX512F
	vpmovzxbd	zmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmovzxbd	zmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512F Disp8
	vpmovzxbd	zmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512F
	vpmovzxbd	zmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512F Disp8
	vpmovzxbd	zmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512F

	vpmovzxbq	zmm6{k7}, xmm5	 # AVX512F
	vpmovzxbq	zmm6{k7}{z}, xmm5	 # AVX512F
	vpmovzxbq	zmm6{k7}, QWORD PTR [ecx]	 # AVX512F
	vpmovzxbq	zmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmovzxbq	zmm6{k7}, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vpmovzxbq	zmm6{k7}, QWORD PTR [edx+1024]	 # AVX512F
	vpmovzxbq	zmm6{k7}, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vpmovzxbq	zmm6{k7}, QWORD PTR [edx-1032]	 # AVX512F

	vpmovzxdq	zmm6{k7}, ymm5	 # AVX512F
	vpmovzxdq	zmm6{k7}{z}, ymm5	 # AVX512F
	vpmovzxdq	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512F
	vpmovzxdq	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmovzxdq	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512F Disp8
	vpmovzxdq	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512F
	vpmovzxdq	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512F Disp8
	vpmovzxdq	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512F

	vpmovzxwd	zmm6{k7}, ymm5	 # AVX512F
	vpmovzxwd	zmm6{k7}{z}, ymm5	 # AVX512F
	vpmovzxwd	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512F
	vpmovzxwd	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmovzxwd	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512F Disp8
	vpmovzxwd	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512F
	vpmovzxwd	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512F Disp8
	vpmovzxwd	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512F

	vpmovzxwq	zmm6{k7}, xmm5	 # AVX512F
	vpmovzxwq	zmm6{k7}{z}, xmm5	 # AVX512F
	vpmovzxwq	zmm6{k7}, XMMWORD PTR [ecx]	 # AVX512F
	vpmovzxwq	zmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmovzxwq	zmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512F Disp8
	vpmovzxwq	zmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512F
	vpmovzxwq	zmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512F Disp8
	vpmovzxwq	zmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512F

	vpmuldq	zmm6, zmm5, zmm4	 # AVX512F
	vpmuldq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpmuldq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpmuldq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpmuldq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmuldq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpmuldq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpmuldq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpmuldq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpmuldq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpmuldq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpmuldq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpmuldq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpmuldq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpmulld	zmm6, zmm5, zmm4	 # AVX512F
	vpmulld	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpmulld	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpmulld	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpmulld	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmulld	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpmulld	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpmulld	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpmulld	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpmulld	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpmulld	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpmulld	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpmulld	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpmulld	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpmuludq	zmm6, zmm5, zmm4	 # AVX512F
	vpmuludq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpmuludq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpmuludq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpmuludq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpmuludq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpmuludq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpmuludq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpmuludq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpmuludq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpmuludq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpmuludq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpmuludq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpmuludq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpord	zmm6, zmm5, zmm4	 # AVX512F
	vpord	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpord	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpord	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpord	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpord	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpord	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpord	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpord	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpord	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpord	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpord	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpord	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpord	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vporq	zmm6, zmm5, zmm4	 # AVX512F
	vporq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vporq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vporq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vporq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vporq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vporq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vporq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vporq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vporq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vporq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vporq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vporq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vporq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpscatterdd	[ebp+zmm7*8-123]{k1}, zmm6	 # AVX512F
	vpscatterdd	dword ptr [ebp+zmm7*8-123]{k1}, zmm6	 # AVX512F
	vpscatterdd	[eax+zmm7+256]{k1}, zmm6	 # AVX512F
	vpscatterdd	[ecx+zmm7*4+1024]{k1}, zmm6	 # AVX512F

	vpscatterdq	[ebp+ymm7*8-123]{k1}, zmm6	 # AVX512F
	vpscatterdq	qword ptr [ebp+ymm7*8-123]{k1}, zmm6	 # AVX512F
	vpscatterdq	[eax+ymm7+256]{k1}, zmm6	 # AVX512F
	vpscatterdq	[ecx+ymm7*4+1024]{k1}, zmm6	 # AVX512F

	vpscatterqd	[ebp+zmm7*8-123]{k1}, ymm6	 # AVX512F
	vpscatterqd	dword ptr [ebp+zmm7*8-123]{k1}, ymm6	 # AVX512F
	vpscatterqd	[eax+zmm7+256]{k1}, ymm6	 # AVX512F
	vpscatterqd	[ecx+zmm7*4+1024]{k1}, ymm6	 # AVX512F

	vpscatterqq	[ebp+zmm7*8-123]{k1}, zmm6	 # AVX512F
	vpscatterqq	qword ptr [ebp+zmm7*8-123]{k1}, zmm6	 # AVX512F
	vpscatterqq	[eax+zmm7+256]{k1}, zmm6	 # AVX512F
	vpscatterqq	[ecx+zmm7*4+1024]{k1}, zmm6	 # AVX512F

	vpshufd	zmm6, zmm5, 0xab	 # AVX512F
	vpshufd	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vpshufd	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vpshufd	zmm6, zmm5, 123	 # AVX512F
	vpshufd	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpshufd	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpshufd	zmm6, dword bcst [eax], 123	 # AVX512F
	vpshufd	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpshufd	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpshufd	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpshufd	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpshufd	zmm6, dword bcst [edx+508], 123	 # AVX512F Disp8
	vpshufd	zmm6, dword bcst [edx+512], 123	 # AVX512F
	vpshufd	zmm6, dword bcst [edx-512], 123	 # AVX512F Disp8
	vpshufd	zmm6, dword bcst [edx-516], 123	 # AVX512F

	vpslld	zmm6{k7}, zmm5, xmm4	 # AVX512F
	vpslld	zmm6{k7}{z}, zmm5, xmm4	 # AVX512F
	vpslld	zmm6{k7}, zmm5, XMMWORD PTR [ecx]	 # AVX512F
	vpslld	zmm6{k7}, zmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpslld	zmm6{k7}, zmm5, XMMWORD PTR [edx+2032]	 # AVX512F Disp8
	vpslld	zmm6{k7}, zmm5, XMMWORD PTR [edx+2048]	 # AVX512F
	vpslld	zmm6{k7}, zmm5, XMMWORD PTR [edx-2048]	 # AVX512F Disp8
	vpslld	zmm6{k7}, zmm5, XMMWORD PTR [edx-2064]	 # AVX512F

	vpsllq	zmm6{k7}, zmm5, xmm4	 # AVX512F
	vpsllq	zmm6{k7}{z}, zmm5, xmm4	 # AVX512F
	vpsllq	zmm6{k7}, zmm5, XMMWORD PTR [ecx]	 # AVX512F
	vpsllq	zmm6{k7}, zmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpsllq	zmm6{k7}, zmm5, XMMWORD PTR [edx+2032]	 # AVX512F Disp8
	vpsllq	zmm6{k7}, zmm5, XMMWORD PTR [edx+2048]	 # AVX512F
	vpsllq	zmm6{k7}, zmm5, XMMWORD PTR [edx-2048]	 # AVX512F Disp8
	vpsllq	zmm6{k7}, zmm5, XMMWORD PTR [edx-2064]	 # AVX512F

	vpsllvd	zmm6, zmm5, zmm4	 # AVX512F
	vpsllvd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpsllvd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpsllvd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpsllvd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpsllvd	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpsllvd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpsllvd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpsllvd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpsllvd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpsllvd	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpsllvd	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpsllvd	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpsllvd	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpsllvq	zmm6, zmm5, zmm4	 # AVX512F
	vpsllvq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpsllvq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpsllvq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpsllvq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpsllvq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpsllvq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpsllvq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpsllvq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpsllvq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpsllvq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpsllvq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpsllvq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpsllvq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpsrad	zmm6{k7}, zmm5, xmm4	 # AVX512F
	vpsrad	zmm6{k7}{z}, zmm5, xmm4	 # AVX512F
	vpsrad	zmm6{k7}, zmm5, XMMWORD PTR [ecx]	 # AVX512F
	vpsrad	zmm6{k7}, zmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpsrad	zmm6{k7}, zmm5, XMMWORD PTR [edx+2032]	 # AVX512F Disp8
	vpsrad	zmm6{k7}, zmm5, XMMWORD PTR [edx+2048]	 # AVX512F
	vpsrad	zmm6{k7}, zmm5, XMMWORD PTR [edx-2048]	 # AVX512F Disp8
	vpsrad	zmm6{k7}, zmm5, XMMWORD PTR [edx-2064]	 # AVX512F

	vpsraq	zmm6{k7}, zmm5, xmm4	 # AVX512F
	vpsraq	zmm6{k7}{z}, zmm5, xmm4	 # AVX512F
	vpsraq	zmm6{k7}, zmm5, XMMWORD PTR [ecx]	 # AVX512F
	vpsraq	zmm6{k7}, zmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpsraq	zmm6{k7}, zmm5, XMMWORD PTR [edx+2032]	 # AVX512F Disp8
	vpsraq	zmm6{k7}, zmm5, XMMWORD PTR [edx+2048]	 # AVX512F
	vpsraq	zmm6{k7}, zmm5, XMMWORD PTR [edx-2048]	 # AVX512F Disp8
	vpsraq	zmm6{k7}, zmm5, XMMWORD PTR [edx-2064]	 # AVX512F

	vpsravd	zmm6, zmm5, zmm4	 # AVX512F
	vpsravd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpsravd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpsravd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpsravd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpsravd	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpsravd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpsravd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpsravd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpsravd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpsravd	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpsravd	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpsravd	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpsravd	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpsravq	zmm6, zmm5, zmm4	 # AVX512F
	vpsravq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpsravq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpsravq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpsravq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpsravq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpsravq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpsravq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpsravq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpsravq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpsravq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpsravq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpsravq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpsravq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpsrld	zmm6{k7}, zmm5, xmm4	 # AVX512F
	vpsrld	zmm6{k7}{z}, zmm5, xmm4	 # AVX512F
	vpsrld	zmm6{k7}, zmm5, XMMWORD PTR [ecx]	 # AVX512F
	vpsrld	zmm6{k7}, zmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpsrld	zmm6{k7}, zmm5, XMMWORD PTR [edx+2032]	 # AVX512F Disp8
	vpsrld	zmm6{k7}, zmm5, XMMWORD PTR [edx+2048]	 # AVX512F
	vpsrld	zmm6{k7}, zmm5, XMMWORD PTR [edx-2048]	 # AVX512F Disp8
	vpsrld	zmm6{k7}, zmm5, XMMWORD PTR [edx-2064]	 # AVX512F

	vpsrlq	zmm6{k7}, zmm5, xmm4	 # AVX512F
	vpsrlq	zmm6{k7}{z}, zmm5, xmm4	 # AVX512F
	vpsrlq	zmm6{k7}, zmm5, XMMWORD PTR [ecx]	 # AVX512F
	vpsrlq	zmm6{k7}, zmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpsrlq	zmm6{k7}, zmm5, XMMWORD PTR [edx+2032]	 # AVX512F Disp8
	vpsrlq	zmm6{k7}, zmm5, XMMWORD PTR [edx+2048]	 # AVX512F
	vpsrlq	zmm6{k7}, zmm5, XMMWORD PTR [edx-2048]	 # AVX512F Disp8
	vpsrlq	zmm6{k7}, zmm5, XMMWORD PTR [edx-2064]	 # AVX512F

	vpsrlvd	zmm6, zmm5, zmm4	 # AVX512F
	vpsrlvd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpsrlvd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpsrlvd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpsrlvd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpsrlvd	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpsrlvd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpsrlvd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpsrlvd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpsrlvd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpsrlvd	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpsrlvd	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpsrlvd	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpsrlvd	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpsrlvq	zmm6, zmm5, zmm4	 # AVX512F
	vpsrlvq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpsrlvq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpsrlvq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpsrlvq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpsrlvq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpsrlvq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpsrlvq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpsrlvq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpsrlvq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpsrlvq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpsrlvq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpsrlvq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpsrlvq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpsrld	zmm6, zmm5, 0xab	 # AVX512F
	vpsrld	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vpsrld	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vpsrld	zmm6, zmm5, 123	 # AVX512F
	vpsrld	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpsrld	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpsrld	zmm6, dword bcst [eax], 123	 # AVX512F
	vpsrld	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpsrld	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpsrld	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpsrld	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpsrld	zmm6, dword bcst [edx+508], 123	 # AVX512F Disp8
	vpsrld	zmm6, dword bcst [edx+512], 123	 # AVX512F
	vpsrld	zmm6, dword bcst [edx-512], 123	 # AVX512F Disp8
	vpsrld	zmm6, dword bcst [edx-516], 123	 # AVX512F

	vpsrlq	zmm6, zmm5, 0xab	 # AVX512F
	vpsrlq	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vpsrlq	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vpsrlq	zmm6, zmm5, 123	 # AVX512F
	vpsrlq	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpsrlq	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpsrlq	zmm6, qword bcst [eax], 123	 # AVX512F
	vpsrlq	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpsrlq	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpsrlq	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpsrlq	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpsrlq	zmm6, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vpsrlq	zmm6, qword bcst [edx+1024], 123	 # AVX512F
	vpsrlq	zmm6, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vpsrlq	zmm6, qword bcst [edx-1032], 123	 # AVX512F

	vpsubd	zmm6, zmm5, zmm4	 # AVX512F
	vpsubd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpsubd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpsubd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpsubd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpsubd	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpsubd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpsubd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpsubd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpsubd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpsubd	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpsubd	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpsubd	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpsubd	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpsubq	zmm6, zmm5, zmm4	 # AVX512F
	vpsubq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpsubq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpsubq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpsubq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpsubq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpsubq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpsubq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpsubq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpsubq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpsubq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpsubq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpsubq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpsubq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vptestmd	k5, zmm6, zmm5	 # AVX512F
	vptestmd	k5{k7}, zmm6, zmm5	 # AVX512F
	vptestmd	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vptestmd	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vptestmd	k5, zmm6, dword bcst [eax]	 # AVX512F
	vptestmd	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vptestmd	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vptestmd	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vptestmd	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vptestmd	k5, zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vptestmd	k5, zmm6, dword bcst [edx+512]	 # AVX512F
	vptestmd	k5, zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vptestmd	k5, zmm6, dword bcst [edx-516]	 # AVX512F

	vptestmq	k5, zmm6, zmm5	 # AVX512F
	vptestmq	k5{k7}, zmm6, zmm5	 # AVX512F
	vptestmq	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vptestmq	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vptestmq	k5, zmm6, qword bcst [eax]	 # AVX512F
	vptestmq	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vptestmq	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vptestmq	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vptestmq	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vptestmq	k5, zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vptestmq	k5, zmm6, qword bcst [edx+1024]	 # AVX512F
	vptestmq	k5, zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vptestmq	k5, zmm6, qword bcst [edx-1032]	 # AVX512F

	vpunpckhdq	zmm6, zmm5, zmm4	 # AVX512F
	vpunpckhdq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpunpckhdq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpunpckhdq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpunpckhdq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpunpckhdq	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpunpckhdq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpunpckhdq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpunpckhdq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpunpckhdq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpunpckhdq	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpunpckhdq	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpunpckhdq	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpunpckhdq	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpunpckhqdq	zmm6, zmm5, zmm4	 # AVX512F
	vpunpckhqdq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpunpckhqdq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpunpckhqdq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpunpckhqdq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpunpckhqdq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpunpckhqdq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpunpckhqdq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpunpckhqdq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpunpckhqdq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpunpckhqdq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpunpckhqdq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpunpckhqdq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpunpckhqdq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpunpckldq	zmm6, zmm5, zmm4	 # AVX512F
	vpunpckldq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpunpckldq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpunpckldq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpunpckldq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpunpckldq	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpunpckldq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpunpckldq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpunpckldq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpunpckldq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpunpckldq	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpunpckldq	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpunpckldq	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpunpckldq	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpunpcklqdq	zmm6, zmm5, zmm4	 # AVX512F
	vpunpcklqdq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpunpcklqdq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpunpcklqdq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpunpcklqdq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpunpcklqdq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpunpcklqdq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpunpcklqdq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpunpcklqdq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpunpcklqdq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpunpcklqdq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpunpcklqdq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpunpcklqdq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpunpcklqdq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpxord	zmm6, zmm5, zmm4	 # AVX512F
	vpxord	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpxord	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpxord	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpxord	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpxord	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpxord	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpxord	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpxord	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpxord	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpxord	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpxord	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpxord	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpxord	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpxorq	zmm6, zmm5, zmm4	 # AVX512F
	vpxorq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpxorq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpxorq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpxorq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpxorq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpxorq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpxorq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpxorq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpxorq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpxorq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpxorq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpxorq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpxorq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vrcp14pd	zmm6, zmm5	 # AVX512F
	vrcp14pd	zmm6{k7}, zmm5	 # AVX512F
	vrcp14pd	zmm6{k7}{z}, zmm5	 # AVX512F
	vrcp14pd	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vrcp14pd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vrcp14pd	zmm6, qword bcst [eax]	 # AVX512F
	vrcp14pd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vrcp14pd	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vrcp14pd	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vrcp14pd	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vrcp14pd	zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vrcp14pd	zmm6, qword bcst [edx+1024]	 # AVX512F
	vrcp14pd	zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vrcp14pd	zmm6, qword bcst [edx-1032]	 # AVX512F

	vrcp14ps	zmm6, zmm5	 # AVX512F
	vrcp14ps	zmm6{k7}, zmm5	 # AVX512F
	vrcp14ps	zmm6{k7}{z}, zmm5	 # AVX512F
	vrcp14ps	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vrcp14ps	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vrcp14ps	zmm6, dword bcst [eax]	 # AVX512F
	vrcp14ps	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vrcp14ps	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vrcp14ps	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vrcp14ps	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vrcp14ps	zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vrcp14ps	zmm6, dword bcst [edx+512]	 # AVX512F
	vrcp14ps	zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vrcp14ps	zmm6, dword bcst [edx-516]	 # AVX512F

	vrcp14sd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vrcp14sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vrcp14sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vrcp14sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vrcp14sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vrcp14sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vrcp14sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vrcp14sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vrcp14ss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vrcp14ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vrcp14ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vrcp14ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vrcp14ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vrcp14ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vrcp14ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vrcp14ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vrsqrt14pd	zmm6, zmm5	 # AVX512F
	vrsqrt14pd	zmm6{k7}, zmm5	 # AVX512F
	vrsqrt14pd	zmm6{k7}{z}, zmm5	 # AVX512F
	vrsqrt14pd	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vrsqrt14pd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vrsqrt14pd	zmm6, qword bcst [eax]	 # AVX512F
	vrsqrt14pd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vrsqrt14pd	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vrsqrt14pd	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vrsqrt14pd	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vrsqrt14pd	zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vrsqrt14pd	zmm6, qword bcst [edx+1024]	 # AVX512F
	vrsqrt14pd	zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vrsqrt14pd	zmm6, qword bcst [edx-1032]	 # AVX512F

	vrsqrt14ps	zmm6, zmm5	 # AVX512F
	vrsqrt14ps	zmm6{k7}, zmm5	 # AVX512F
	vrsqrt14ps	zmm6{k7}{z}, zmm5	 # AVX512F
	vrsqrt14ps	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vrsqrt14ps	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vrsqrt14ps	zmm6, dword bcst [eax]	 # AVX512F
	vrsqrt14ps	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vrsqrt14ps	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vrsqrt14ps	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vrsqrt14ps	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vrsqrt14ps	zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vrsqrt14ps	zmm6, dword bcst [edx+512]	 # AVX512F
	vrsqrt14ps	zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vrsqrt14ps	zmm6, dword bcst [edx-516]	 # AVX512F

	vrsqrt14sd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vrsqrt14sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vrsqrt14sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vrsqrt14sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vrsqrt14sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vrsqrt14sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vrsqrt14sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vrsqrt14sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vrsqrt14ss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vrsqrt14ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vrsqrt14ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vrsqrt14ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vrsqrt14ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vrsqrt14ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vrsqrt14ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vrsqrt14ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vscatterdpd	[ebp+ymm7*8-123]{k1}, zmm6	 # AVX512F
	vscatterdpd	qword ptr [ebp+ymm7*8-123]{k1}, zmm6	 # AVX512F
	vscatterdpd	[eax+ymm7+256]{k1}, zmm6	 # AVX512F
	vscatterdpd	[ecx+ymm7*4+1024]{k1}, zmm6	 # AVX512F

	vscatterdps	[ebp+zmm7*8-123]{k1}, zmm6	 # AVX512F
	vscatterdps	dword ptr [ebp+zmm7*8-123]{k1}, zmm6	 # AVX512F
	vscatterdps	[eax+zmm7+256]{k1}, zmm6	 # AVX512F
	vscatterdps	[ecx+zmm7*4+1024]{k1}, zmm6	 # AVX512F

	vscatterqpd	[ebp+zmm7*8-123]{k1}, zmm6	 # AVX512F
	vscatterqpd	qword ptr [ebp+zmm7*8-123]{k1}, zmm6	 # AVX512F
	vscatterqpd	[eax+zmm7+256]{k1}, zmm6	 # AVX512F
	vscatterqpd	[ecx+zmm7*4+1024]{k1}, zmm6	 # AVX512F

	vscatterqps	[ebp+zmm7*8-123]{k1}, ymm6	 # AVX512F
	vscatterqps	dword ptr [ebp+zmm7*8-123]{k1}, ymm6	 # AVX512F
	vscatterqps	[eax+zmm7+256]{k1}, ymm6	 # AVX512F
	vscatterqps	[ecx+zmm7*4+1024]{k1}, ymm6	 # AVX512F

	vshufpd	zmm6, zmm5, zmm4, 0xab	 # AVX512F
	vshufpd	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512F
	vshufpd	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512F
	vshufpd	zmm6, zmm5, zmm4, 123	 # AVX512F
	vshufpd	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512F
	vshufpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vshufpd	zmm6, zmm5, qword bcst [eax], 123	 # AVX512F
	vshufpd	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vshufpd	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vshufpd	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vshufpd	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vshufpd	zmm6, zmm5, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vshufpd	zmm6, zmm5, qword bcst [edx+1024], 123	 # AVX512F
	vshufpd	zmm6, zmm5, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vshufpd	zmm6, zmm5, qword bcst [edx-1032], 123	 # AVX512F

	vshufps	zmm6, zmm5, zmm4, 0xab	 # AVX512F
	vshufps	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512F
	vshufps	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512F
	vshufps	zmm6, zmm5, zmm4, 123	 # AVX512F
	vshufps	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512F
	vshufps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vshufps	zmm6, zmm5, dword bcst [eax], 123	 # AVX512F
	vshufps	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vshufps	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vshufps	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vshufps	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vshufps	zmm6, zmm5, dword bcst [edx+508], 123	 # AVX512F Disp8
	vshufps	zmm6, zmm5, dword bcst [edx+512], 123	 # AVX512F
	vshufps	zmm6, zmm5, dword bcst [edx-512], 123	 # AVX512F Disp8
	vshufps	zmm6, zmm5, dword bcst [edx-516], 123	 # AVX512F

	vsqrtpd	zmm6, zmm5	 # AVX512F
	vsqrtpd	zmm6{k7}, zmm5	 # AVX512F
	vsqrtpd	zmm6{k7}{z}, zmm5	 # AVX512F
	vsqrtpd	zmm6, zmm5{rn-sae}	 # AVX512F
	vsqrtpd	zmm6, zmm5{ru-sae}	 # AVX512F
	vsqrtpd	zmm6, zmm5{rd-sae}	 # AVX512F
	vsqrtpd	zmm6, zmm5{rz-sae}	 # AVX512F
	vsqrtpd	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vsqrtpd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vsqrtpd	zmm6, qword bcst [eax]	 # AVX512F
	vsqrtpd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vsqrtpd	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vsqrtpd	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vsqrtpd	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vsqrtpd	zmm6, qword bcst [edx+1016]	 # AVX512F Disp8
	vsqrtpd	zmm6, qword bcst [edx+1024]	 # AVX512F
	vsqrtpd	zmm6, qword bcst [edx-1024]	 # AVX512F Disp8
	vsqrtpd	zmm6, qword bcst [edx-1032]	 # AVX512F

	vsqrtps	zmm6, zmm5	 # AVX512F
	vsqrtps	zmm6{k7}, zmm5	 # AVX512F
	vsqrtps	zmm6{k7}{z}, zmm5	 # AVX512F
	vsqrtps	zmm6, zmm5{rn-sae}	 # AVX512F
	vsqrtps	zmm6, zmm5{ru-sae}	 # AVX512F
	vsqrtps	zmm6, zmm5{rd-sae}	 # AVX512F
	vsqrtps	zmm6, zmm5{rz-sae}	 # AVX512F
	vsqrtps	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vsqrtps	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vsqrtps	zmm6, dword bcst [eax]	 # AVX512F
	vsqrtps	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vsqrtps	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vsqrtps	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vsqrtps	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vsqrtps	zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vsqrtps	zmm6, dword bcst [edx+512]	 # AVX512F
	vsqrtps	zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vsqrtps	zmm6, dword bcst [edx-516]	 # AVX512F

	vsqrtsd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vsqrtsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vsqrtsd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vsqrtsd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vsqrtsd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vsqrtsd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vsqrtsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vsqrtsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vsqrtsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vsqrtsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vsqrtsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vsqrtsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vsqrtss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vsqrtss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vsqrtss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vsqrtss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vsqrtss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vsqrtss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vsqrtss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vsqrtss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vsqrtss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vsqrtss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vsqrtss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vsqrtss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vsubpd	zmm6, zmm5, zmm4	 # AVX512F
	vsubpd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vsubpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vsubpd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vsubpd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vsubpd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vsubpd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vsubpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vsubpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vsubpd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vsubpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vsubpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vsubpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vsubpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vsubpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vsubpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vsubpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vsubpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vsubps	zmm6, zmm5, zmm4	 # AVX512F
	vsubps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vsubps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vsubps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vsubps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vsubps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vsubps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vsubps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vsubps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vsubps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vsubps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vsubps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vsubps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vsubps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vsubps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vsubps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vsubps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vsubps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vsubsd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vsubsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vsubsd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vsubsd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vsubsd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vsubsd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vsubsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vsubsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vsubsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vsubsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vsubsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vsubsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vsubss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vsubss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vsubss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vsubss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vsubss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vsubss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vsubss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vsubss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vsubss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vsubss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vsubss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vsubss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vucomisd	xmm6, xmm5{sae}	 # AVX512F

	vucomiss	xmm6, xmm5{sae}	 # AVX512F

	vunpckhpd	zmm6, zmm5, zmm4	 # AVX512F
	vunpckhpd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vunpckhpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vunpckhpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vunpckhpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vunpckhpd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vunpckhpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vunpckhpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vunpckhpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vunpckhpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vunpckhpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vunpckhpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vunpckhpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vunpckhpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vunpckhps	zmm6, zmm5, zmm4	 # AVX512F
	vunpckhps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vunpckhps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vunpckhps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vunpckhps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vunpckhps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vunpckhps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vunpckhps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vunpckhps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vunpckhps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vunpckhps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vunpckhps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vunpckhps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vunpckhps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vunpcklpd	zmm6, zmm5, zmm4	 # AVX512F
	vunpcklpd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vunpcklpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vunpcklpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vunpcklpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vunpcklpd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vunpcklpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vunpcklpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vunpcklpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vunpcklpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vunpcklpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vunpcklpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vunpcklpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vunpcklpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vunpcklps	zmm6, zmm5, zmm4	 # AVX512F
	vunpcklps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vunpcklps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vunpcklps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vunpcklps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vunpcklps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vunpcklps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vunpcklps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vunpcklps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vunpcklps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vunpcklps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vunpcklps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vunpcklps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vunpcklps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpternlogd	zmm6, zmm5, zmm4, 0xab	 # AVX512F
	vpternlogd	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512F
	vpternlogd	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512F
	vpternlogd	zmm6, zmm5, zmm4, 123	 # AVX512F
	vpternlogd	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpternlogd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpternlogd	zmm6, zmm5, dword bcst [eax], 123	 # AVX512F
	vpternlogd	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpternlogd	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpternlogd	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpternlogd	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpternlogd	zmm6, zmm5, dword bcst [edx+508], 123	 # AVX512F Disp8
	vpternlogd	zmm6, zmm5, dword bcst [edx+512], 123	 # AVX512F
	vpternlogd	zmm6, zmm5, dword bcst [edx-512], 123	 # AVX512F Disp8
	vpternlogd	zmm6, zmm5, dword bcst [edx-516], 123	 # AVX512F

	vpternlogq	zmm6, zmm5, zmm4, 0xab	 # AVX512F
	vpternlogq	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512F
	vpternlogq	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512F
	vpternlogq	zmm6, zmm5, zmm4, 123	 # AVX512F
	vpternlogq	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpternlogq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpternlogq	zmm6, zmm5, qword bcst [eax], 123	 # AVX512F
	vpternlogq	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpternlogq	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpternlogq	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpternlogq	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpternlogq	zmm6, zmm5, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vpternlogq	zmm6, zmm5, qword bcst [edx+1024], 123	 # AVX512F
	vpternlogq	zmm6, zmm5, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vpternlogq	zmm6, zmm5, qword bcst [edx-1032], 123	 # AVX512F

	vpmovqb	xmm6{k7}, zmm5	 # AVX512F
	vpmovqb	xmm6{k7}{z}, zmm5	 # AVX512F

	vpmovsqb	xmm6{k7}, zmm5	 # AVX512F
	vpmovsqb	xmm6{k7}{z}, zmm5	 # AVX512F

	vpmovusqb	xmm6{k7}, zmm5	 # AVX512F
	vpmovusqb	xmm6{k7}{z}, zmm5	 # AVX512F

	vpmovqw	xmm6{k7}, zmm5	 # AVX512F
	vpmovqw	xmm6{k7}{z}, zmm5	 # AVX512F

	vpmovsqw	xmm6{k7}, zmm5	 # AVX512F
	vpmovsqw	xmm6{k7}{z}, zmm5	 # AVX512F

	vpmovusqw	xmm6{k7}, zmm5	 # AVX512F
	vpmovusqw	xmm6{k7}{z}, zmm5	 # AVX512F

	vpmovqd	ymm6{k7}, zmm5	 # AVX512F
	vpmovqd	ymm6{k7}{z}, zmm5	 # AVX512F

	vpmovsqd	ymm6{k7}, zmm5	 # AVX512F
	vpmovsqd	ymm6{k7}{z}, zmm5	 # AVX512F

	vpmovusqd	ymm6{k7}, zmm5	 # AVX512F
	vpmovusqd	ymm6{k7}{z}, zmm5	 # AVX512F

	vpmovdb	xmm6{k7}, zmm5	 # AVX512F
	vpmovdb	xmm6{k7}{z}, zmm5	 # AVX512F

	vpmovsdb	xmm6{k7}, zmm5	 # AVX512F
	vpmovsdb	xmm6{k7}{z}, zmm5	 # AVX512F

	vpmovusdb	xmm6{k7}, zmm5	 # AVX512F
	vpmovusdb	xmm6{k7}{z}, zmm5	 # AVX512F

	vpmovdw	ymm6{k7}, zmm5	 # AVX512F
	vpmovdw	ymm6{k7}{z}, zmm5	 # AVX512F

	vpmovsdw	ymm6{k7}, zmm5	 # AVX512F
	vpmovsdw	ymm6{k7}{z}, zmm5	 # AVX512F

	vpmovusdw	ymm6{k7}, zmm5	 # AVX512F
	vpmovusdw	ymm6{k7}{z}, zmm5	 # AVX512F

	vshuff32x4	zmm6, zmm5, zmm4, 0xab	 # AVX512F
	vshuff32x4	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512F
	vshuff32x4	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512F
	vshuff32x4	zmm6, zmm5, zmm4, 123	 # AVX512F
	vshuff32x4	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512F
	vshuff32x4	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vshuff32x4	zmm6, zmm5, dword bcst [eax], 123	 # AVX512F
	vshuff32x4	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vshuff32x4	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vshuff32x4	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vshuff32x4	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vshuff32x4	zmm6, zmm5, dword bcst [edx+508], 123	 # AVX512F Disp8
	vshuff32x4	zmm6, zmm5, dword bcst [edx+512], 123	 # AVX512F
	vshuff32x4	zmm6, zmm5, dword bcst [edx-512], 123	 # AVX512F Disp8
	vshuff32x4	zmm6, zmm5, dword bcst [edx-516], 123	 # AVX512F

	vshuff64x2	zmm6, zmm5, zmm4, 0xab	 # AVX512F
	vshuff64x2	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512F
	vshuff64x2	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512F
	vshuff64x2	zmm6, zmm5, zmm4, 123	 # AVX512F
	vshuff64x2	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512F
	vshuff64x2	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vshuff64x2	zmm6, zmm5, qword bcst [eax], 123	 # AVX512F
	vshuff64x2	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vshuff64x2	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vshuff64x2	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vshuff64x2	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vshuff64x2	zmm6, zmm5, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vshuff64x2	zmm6, zmm5, qword bcst [edx+1024], 123	 # AVX512F
	vshuff64x2	zmm6, zmm5, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vshuff64x2	zmm6, zmm5, qword bcst [edx-1032], 123	 # AVX512F

	vshufi32x4	zmm6, zmm5, zmm4, 0xab	 # AVX512F
	vshufi32x4	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512F
	vshufi32x4	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512F
	vshufi32x4	zmm6, zmm5, zmm4, 123	 # AVX512F
	vshufi32x4	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512F
	vshufi32x4	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vshufi32x4	zmm6, zmm5, dword bcst [eax], 123	 # AVX512F
	vshufi32x4	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vshufi32x4	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vshufi32x4	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vshufi32x4	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vshufi32x4	zmm6, zmm5, dword bcst [edx+508], 123	 # AVX512F Disp8
	vshufi32x4	zmm6, zmm5, dword bcst [edx+512], 123	 # AVX512F
	vshufi32x4	zmm6, zmm5, dword bcst [edx-512], 123	 # AVX512F Disp8
	vshufi32x4	zmm6, zmm5, dword bcst [edx-516], 123	 # AVX512F

	vshufi64x2	zmm6, zmm5, zmm4, 0xab	 # AVX512F
	vshufi64x2	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512F
	vshufi64x2	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512F
	vshufi64x2	zmm6, zmm5, zmm4, 123	 # AVX512F
	vshufi64x2	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512F
	vshufi64x2	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vshufi64x2	zmm6, zmm5, qword bcst [eax], 123	 # AVX512F
	vshufi64x2	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vshufi64x2	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vshufi64x2	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vshufi64x2	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vshufi64x2	zmm6, zmm5, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vshufi64x2	zmm6, zmm5, qword bcst [edx+1024], 123	 # AVX512F
	vshufi64x2	zmm6, zmm5, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vshufi64x2	zmm6, zmm5, qword bcst [edx-1032], 123	 # AVX512F

	vpermq	zmm6, zmm5, zmm4	 # AVX512F
	vpermq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpermq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpermq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpermq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpermq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpermq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpermq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpermq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpermq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpermq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpermq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpermq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpermq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpermpd	zmm6, zmm5, zmm4	 # AVX512F
	vpermpd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpermpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpermpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpermpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpermpd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpermpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpermpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpermpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpermpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpermpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpermpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpermpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpermpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpermt2d	zmm6, zmm5, zmm4	 # AVX512F
	vpermt2d	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpermt2d	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpermt2d	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpermt2d	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpermt2d	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpermt2d	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpermt2d	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpermt2d	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpermt2d	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpermt2d	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpermt2d	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpermt2d	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpermt2d	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpermt2q	zmm6, zmm5, zmm4	 # AVX512F
	vpermt2q	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpermt2q	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpermt2q	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpermt2q	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpermt2q	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpermt2q	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpermt2q	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpermt2q	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpermt2q	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpermt2q	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpermt2q	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpermt2q	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpermt2q	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpermt2ps	zmm6, zmm5, zmm4	 # AVX512F
	vpermt2ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpermt2ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpermt2ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpermt2ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpermt2ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpermt2ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpermt2ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpermt2ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpermt2ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpermt2ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpermt2ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpermt2ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpermt2ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpermt2pd	zmm6, zmm5, zmm4	 # AVX512F
	vpermt2pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpermt2pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpermt2pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpermt2pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpermt2pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpermt2pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpermt2pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpermt2pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpermt2pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpermt2pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpermt2pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpermt2pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpermt2pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	valignq	zmm6, zmm5, zmm4, 0xab	 # AVX512F
	valignq	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512F
	valignq	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512F
	valignq	zmm6, zmm5, zmm4, 123	 # AVX512F
	valignq	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512F
	valignq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	valignq	zmm6, zmm5, qword bcst [eax], 123	 # AVX512F
	valignq	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	valignq	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	valignq	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	valignq	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	valignq	zmm6, zmm5, qword bcst [edx+1016], 123	 # AVX512F Disp8
	valignq	zmm6, zmm5, qword bcst [edx+1024], 123	 # AVX512F
	valignq	zmm6, zmm5, qword bcst [edx-1024], 123	 # AVX512F Disp8
	valignq	zmm6, zmm5, qword bcst [edx-1032], 123	 # AVX512F

	vcvtsd2usi	eax, xmm6	 # AVX512F
	vcvtsd2usi	eax, xmm6{rn-sae}	 # AVX512F
	vcvtsd2usi	eax, xmm6{ru-sae}	 # AVX512F
	vcvtsd2usi	eax, xmm6{rd-sae}	 # AVX512F
	vcvtsd2usi	eax, xmm6{rz-sae}	 # AVX512F
	vcvtsd2usi	eax, QWORD PTR [ecx]	 # AVX512F
	vcvtsd2usi	eax, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtsd2usi	eax, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcvtsd2usi	eax, QWORD PTR [edx+1024]	 # AVX512F
	vcvtsd2usi	eax, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcvtsd2usi	eax, QWORD PTR [edx-1032]	 # AVX512F
	vcvtsd2usi	ebp, xmm6	 # AVX512F
	vcvtsd2usi	ebp, xmm6{rn-sae}	 # AVX512F
	vcvtsd2usi	ebp, xmm6{ru-sae}	 # AVX512F
	vcvtsd2usi	ebp, xmm6{rd-sae}	 # AVX512F
	vcvtsd2usi	ebp, xmm6{rz-sae}	 # AVX512F
	vcvtsd2usi	ebp, QWORD PTR [ecx]	 # AVX512F
	vcvtsd2usi	ebp, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtsd2usi	ebp, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcvtsd2usi	ebp, QWORD PTR [edx+1024]	 # AVX512F
	vcvtsd2usi	ebp, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcvtsd2usi	ebp, QWORD PTR [edx-1032]	 # AVX512F

	vcvtss2usi	eax, xmm6	 # AVX512F
	vcvtss2usi	eax, xmm6{rn-sae}	 # AVX512F
	vcvtss2usi	eax, xmm6{ru-sae}	 # AVX512F
	vcvtss2usi	eax, xmm6{rd-sae}	 # AVX512F
	vcvtss2usi	eax, xmm6{rz-sae}	 # AVX512F
	vcvtss2usi	eax, DWORD PTR [ecx]	 # AVX512F
	vcvtss2usi	eax, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtss2usi	eax, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcvtss2usi	eax, DWORD PTR [edx+512]	 # AVX512F
	vcvtss2usi	eax, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcvtss2usi	eax, DWORD PTR [edx-516]	 # AVX512F
	vcvtss2usi	ebp, xmm6	 # AVX512F
	vcvtss2usi	ebp, xmm6{rn-sae}	 # AVX512F
	vcvtss2usi	ebp, xmm6{ru-sae}	 # AVX512F
	vcvtss2usi	ebp, xmm6{rd-sae}	 # AVX512F
	vcvtss2usi	ebp, xmm6{rz-sae}	 # AVX512F
	vcvtss2usi	ebp, DWORD PTR [ecx]	 # AVX512F
	vcvtss2usi	ebp, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtss2usi	ebp, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcvtss2usi	ebp, DWORD PTR [edx+512]	 # AVX512F
	vcvtss2usi	ebp, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcvtss2usi	ebp, DWORD PTR [edx-516]	 # AVX512F

	vcvtusi2sd	xmm6, xmm5, eax	 # AVX512F
	vcvtusi2sd	xmm6, xmm5, ebp	 # AVX512F
	vcvtusi2sd	xmm6, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcvtusi2sd	xmm6, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtusi2sd	xmm6, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcvtusi2sd	xmm6, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcvtusi2sd	xmm6, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcvtusi2sd	xmm6, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vcvtusi2ss	xmm6, xmm5, eax	 # AVX512F
	vcvtusi2ss	xmm6, xmm5, eax{rn-sae}	 # AVX512F
	vcvtusi2ss	xmm6, xmm5, eax{ru-sae}	 # AVX512F
	vcvtusi2ss	xmm6, xmm5, eax{rd-sae}	 # AVX512F
	vcvtusi2ss	xmm6, xmm5, eax{rz-sae}	 # AVX512F
	vcvtusi2ss	xmm6, xmm5, ebp	 # AVX512F
	vcvtusi2ss	xmm6, xmm5, ebp{rn-sae}	 # AVX512F
	vcvtusi2ss	xmm6, xmm5, ebp{ru-sae}	 # AVX512F
	vcvtusi2ss	xmm6, xmm5, ebp{rd-sae}	 # AVX512F
	vcvtusi2ss	xmm6, xmm5, ebp{rz-sae}	 # AVX512F
	vcvtusi2ss	xmm6, xmm5, DWORD PTR [ecx]	 # AVX512F
	vcvtusi2ss	xmm6, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvtusi2ss	xmm6, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcvtusi2ss	xmm6, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vcvtusi2ss	xmm6, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcvtusi2ss	xmm6, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vscalefpd	zmm6, zmm5, zmm4	 # AVX512F
	vscalefpd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vscalefpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vscalefpd	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vscalefpd	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vscalefpd	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vscalefpd	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vscalefpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vscalefpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vscalefpd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vscalefpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vscalefpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vscalefpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vscalefpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vscalefpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vscalefpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vscalefpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vscalefpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vscalefps	zmm6, zmm5, zmm4	 # AVX512F
	vscalefps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vscalefps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vscalefps	zmm6, zmm5, zmm4{rn-sae}	 # AVX512F
	vscalefps	zmm6, zmm5, zmm4{ru-sae}	 # AVX512F
	vscalefps	zmm6, zmm5, zmm4{rd-sae}	 # AVX512F
	vscalefps	zmm6, zmm5, zmm4{rz-sae}	 # AVX512F
	vscalefps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vscalefps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vscalefps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vscalefps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vscalefps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vscalefps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vscalefps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vscalefps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vscalefps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vscalefps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vscalefps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vscalefsd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vscalefsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vscalefsd	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vscalefsd	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vscalefsd	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vscalefsd	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vscalefsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512F
	vscalefsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vscalefsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vscalefsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512F
	vscalefsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vscalefsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512F

	vscalefss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vscalefss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vscalefss	xmm6{k7}, xmm5, xmm4{rn-sae}	 # AVX512F
	vscalefss	xmm6{k7}, xmm5, xmm4{ru-sae}	 # AVX512F
	vscalefss	xmm6{k7}, xmm5, xmm4{rd-sae}	 # AVX512F
	vscalefss	xmm6{k7}, xmm5, xmm4{rz-sae}	 # AVX512F
	vscalefss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512F
	vscalefss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vscalefss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512F Disp8
	vscalefss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512F
	vscalefss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512F Disp8
	vscalefss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512F

	vfixupimmps	zmm6, zmm5, zmm4, 0xab	 # AVX512F
	vfixupimmps	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512F
	vfixupimmps	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512F
	vfixupimmps	zmm6, zmm5, zmm4{sae}, 0xab	 # AVX512F
	vfixupimmps	zmm6, zmm5, zmm4, 123	 # AVX512F
	vfixupimmps	zmm6, zmm5, zmm4{sae}, 123	 # AVX512F
	vfixupimmps	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512F
	vfixupimmps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vfixupimmps	zmm6, zmm5, dword bcst [eax], 123	 # AVX512F
	vfixupimmps	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vfixupimmps	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vfixupimmps	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vfixupimmps	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vfixupimmps	zmm6, zmm5, dword bcst [edx+508], 123	 # AVX512F Disp8
	vfixupimmps	zmm6, zmm5, dword bcst [edx+512], 123	 # AVX512F
	vfixupimmps	zmm6, zmm5, dword bcst [edx-512], 123	 # AVX512F Disp8
	vfixupimmps	zmm6, zmm5, dword bcst [edx-516], 123	 # AVX512F

	vfixupimmpd	zmm6, zmm5, zmm4, 0xab	 # AVX512F
	vfixupimmpd	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512F
	vfixupimmpd	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512F
	vfixupimmpd	zmm6, zmm5, zmm4{sae}, 0xab	 # AVX512F
	vfixupimmpd	zmm6, zmm5, zmm4, 123	 # AVX512F
	vfixupimmpd	zmm6, zmm5, zmm4{sae}, 123	 # AVX512F
	vfixupimmpd	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512F
	vfixupimmpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vfixupimmpd	zmm6, zmm5, qword bcst [eax], 123	 # AVX512F
	vfixupimmpd	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vfixupimmpd	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vfixupimmpd	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vfixupimmpd	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vfixupimmpd	zmm6, zmm5, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vfixupimmpd	zmm6, zmm5, qword bcst [edx+1024], 123	 # AVX512F
	vfixupimmpd	zmm6, zmm5, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vfixupimmpd	zmm6, zmm5, qword bcst [edx-1032], 123	 # AVX512F

	vfixupimmss	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512F
	vfixupimmss	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512F
	vfixupimmss	xmm6{k7}, xmm5, xmm4{sae}, 0xab	 # AVX512F
	vfixupimmss	xmm6{k7}, xmm5, xmm4, 123	 # AVX512F
	vfixupimmss	xmm6{k7}, xmm5, xmm4{sae}, 123	 # AVX512F
	vfixupimmss	xmm6{k7}, xmm5, DWORD PTR [ecx], 123	 # AVX512F
	vfixupimmss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vfixupimmss	xmm6{k7}, xmm5, DWORD PTR [edx+508], 123	 # AVX512F Disp8
	vfixupimmss	xmm6{k7}, xmm5, DWORD PTR [edx+512], 123	 # AVX512F
	vfixupimmss	xmm6{k7}, xmm5, DWORD PTR [edx-512], 123	 # AVX512F Disp8
	vfixupimmss	xmm6{k7}, xmm5, DWORD PTR [edx-516], 123	 # AVX512F

	vfixupimmsd	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512F
	vfixupimmsd	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512F
	vfixupimmsd	xmm6{k7}, xmm5, xmm4{sae}, 0xab	 # AVX512F
	vfixupimmsd	xmm6{k7}, xmm5, xmm4, 123	 # AVX512F
	vfixupimmsd	xmm6{k7}, xmm5, xmm4{sae}, 123	 # AVX512F
	vfixupimmsd	xmm6{k7}, xmm5, QWORD PTR [ecx], 123	 # AVX512F
	vfixupimmsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vfixupimmsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016], 123	 # AVX512F Disp8
	vfixupimmsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024], 123	 # AVX512F
	vfixupimmsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024], 123	 # AVX512F Disp8
	vfixupimmsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032], 123	 # AVX512F

	vpslld	zmm6, zmm5, 0xab	 # AVX512F
	vpslld	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vpslld	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vpslld	zmm6, zmm5, 123	 # AVX512F
	vpslld	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpslld	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpslld	zmm6, dword bcst [eax], 123	 # AVX512F
	vpslld	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpslld	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpslld	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpslld	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpslld	zmm6, dword bcst [edx+508], 123	 # AVX512F Disp8
	vpslld	zmm6, dword bcst [edx+512], 123	 # AVX512F
	vpslld	zmm6, dword bcst [edx-512], 123	 # AVX512F Disp8
	vpslld	zmm6, dword bcst [edx-516], 123	 # AVX512F

	vpsllq	zmm6, zmm5, 0xab	 # AVX512F
	vpsllq	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vpsllq	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vpsllq	zmm6, zmm5, 123	 # AVX512F
	vpsllq	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpsllq	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpsllq	zmm6, qword bcst [eax], 123	 # AVX512F
	vpsllq	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpsllq	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpsllq	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpsllq	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpsllq	zmm6, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vpsllq	zmm6, qword bcst [edx+1024], 123	 # AVX512F
	vpsllq	zmm6, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vpsllq	zmm6, qword bcst [edx-1032], 123	 # AVX512F

	vpsrad	zmm6, zmm5, 0xab	 # AVX512F
	vpsrad	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vpsrad	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vpsrad	zmm6, zmm5, 123	 # AVX512F
	vpsrad	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpsrad	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpsrad	zmm6, dword bcst [eax], 123	 # AVX512F
	vpsrad	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpsrad	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpsrad	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpsrad	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpsrad	zmm6, dword bcst [edx+508], 123	 # AVX512F Disp8
	vpsrad	zmm6, dword bcst [edx+512], 123	 # AVX512F
	vpsrad	zmm6, dword bcst [edx-512], 123	 # AVX512F Disp8
	vpsrad	zmm6, dword bcst [edx-516], 123	 # AVX512F

	vpsraq	zmm6, zmm5, 0xab	 # AVX512F
	vpsraq	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vpsraq	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vpsraq	zmm6, zmm5, 123	 # AVX512F
	vpsraq	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vpsraq	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vpsraq	zmm6, qword bcst [eax], 123	 # AVX512F
	vpsraq	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vpsraq	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vpsraq	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vpsraq	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vpsraq	zmm6, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vpsraq	zmm6, qword bcst [edx+1024], 123	 # AVX512F
	vpsraq	zmm6, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vpsraq	zmm6, qword bcst [edx-1032], 123	 # AVX512F

	vprolvd	zmm6, zmm5, zmm4	 # AVX512F
	vprolvd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vprolvd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vprolvd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vprolvd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vprolvd	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vprolvd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vprolvd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vprolvd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vprolvd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vprolvd	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vprolvd	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vprolvd	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vprolvd	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vprold	zmm6, zmm5, 0xab	 # AVX512F
	vprold	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vprold	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vprold	zmm6, zmm5, 123	 # AVX512F
	vprold	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vprold	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vprold	zmm6, dword bcst [eax], 123	 # AVX512F
	vprold	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vprold	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vprold	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vprold	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vprold	zmm6, dword bcst [edx+508], 123	 # AVX512F Disp8
	vprold	zmm6, dword bcst [edx+512], 123	 # AVX512F
	vprold	zmm6, dword bcst [edx-512], 123	 # AVX512F Disp8
	vprold	zmm6, dword bcst [edx-516], 123	 # AVX512F

	vprolvq	zmm6, zmm5, zmm4	 # AVX512F
	vprolvq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vprolvq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vprolvq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vprolvq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vprolvq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vprolvq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vprolvq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vprolvq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vprolvq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vprolvq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vprolvq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vprolvq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vprolvq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vprolq	zmm6, zmm5, 0xab	 # AVX512F
	vprolq	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vprolq	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vprolq	zmm6, zmm5, 123	 # AVX512F
	vprolq	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vprolq	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vprolq	zmm6, qword bcst [eax], 123	 # AVX512F
	vprolq	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vprolq	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vprolq	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vprolq	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vprolq	zmm6, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vprolq	zmm6, qword bcst [edx+1024], 123	 # AVX512F
	vprolq	zmm6, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vprolq	zmm6, qword bcst [edx-1032], 123	 # AVX512F

	vprorvd	zmm6, zmm5, zmm4	 # AVX512F
	vprorvd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vprorvd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vprorvd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vprorvd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vprorvd	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vprorvd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vprorvd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vprorvd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vprorvd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vprorvd	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vprorvd	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vprorvd	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vprorvd	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vprord	zmm6, zmm5, 0xab	 # AVX512F
	vprord	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vprord	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vprord	zmm6, zmm5, 123	 # AVX512F
	vprord	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vprord	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vprord	zmm6, dword bcst [eax], 123	 # AVX512F
	vprord	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vprord	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vprord	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vprord	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vprord	zmm6, dword bcst [edx+508], 123	 # AVX512F Disp8
	vprord	zmm6, dword bcst [edx+512], 123	 # AVX512F
	vprord	zmm6, dword bcst [edx-512], 123	 # AVX512F Disp8
	vprord	zmm6, dword bcst [edx-516], 123	 # AVX512F

	vprorvq	zmm6, zmm5, zmm4	 # AVX512F
	vprorvq	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vprorvq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vprorvq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vprorvq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vprorvq	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vprorvq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vprorvq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vprorvq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vprorvq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vprorvq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vprorvq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vprorvq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vprorvq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vprorq	zmm6, zmm5, 0xab	 # AVX512F
	vprorq	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vprorq	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vprorq	zmm6, zmm5, 123	 # AVX512F
	vprorq	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vprorq	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vprorq	zmm6, qword bcst [eax], 123	 # AVX512F
	vprorq	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vprorq	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vprorq	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vprorq	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vprorq	zmm6, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vprorq	zmm6, qword bcst [edx+1024], 123	 # AVX512F
	vprorq	zmm6, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vprorq	zmm6, qword bcst [edx-1032], 123	 # AVX512F

	vrndscalepd	zmm6, zmm5, 0xab	 # AVX512F
	vrndscalepd	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vrndscalepd	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vrndscalepd	zmm6, zmm5{sae}, 0xab	 # AVX512F
	vrndscalepd	zmm6, zmm5, 123	 # AVX512F
	vrndscalepd	zmm6, zmm5{sae}, 123	 # AVX512F
	vrndscalepd	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vrndscalepd	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vrndscalepd	zmm6, qword bcst [eax], 123	 # AVX512F
	vrndscalepd	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vrndscalepd	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vrndscalepd	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vrndscalepd	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vrndscalepd	zmm6, qword bcst [edx+1016], 123	 # AVX512F Disp8
	vrndscalepd	zmm6, qword bcst [edx+1024], 123	 # AVX512F
	vrndscalepd	zmm6, qword bcst [edx-1024], 123	 # AVX512F Disp8
	vrndscalepd	zmm6, qword bcst [edx-1032], 123	 # AVX512F

	vrndscaleps	zmm6, zmm5, 0xab	 # AVX512F
	vrndscaleps	zmm6{k7}, zmm5, 0xab	 # AVX512F
	vrndscaleps	zmm6{k7}{z}, zmm5, 0xab	 # AVX512F
	vrndscaleps	zmm6, zmm5{sae}, 0xab	 # AVX512F
	vrndscaleps	zmm6, zmm5, 123	 # AVX512F
	vrndscaleps	zmm6, zmm5{sae}, 123	 # AVX512F
	vrndscaleps	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512F
	vrndscaleps	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vrndscaleps	zmm6, dword bcst [eax], 123	 # AVX512F
	vrndscaleps	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512F Disp8
	vrndscaleps	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512F
	vrndscaleps	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512F Disp8
	vrndscaleps	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512F
	vrndscaleps	zmm6, dword bcst [edx+508], 123	 # AVX512F Disp8
	vrndscaleps	zmm6, dword bcst [edx+512], 123	 # AVX512F
	vrndscaleps	zmm6, dword bcst [edx-512], 123	 # AVX512F Disp8
	vrndscaleps	zmm6, dword bcst [edx-516], 123	 # AVX512F

	vrndscalesd	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512F
	vrndscalesd	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512F
	vrndscalesd	xmm6{k7}, xmm5, xmm4{sae}, 0xab	 # AVX512F
	vrndscalesd	xmm6{k7}, xmm5, xmm4, 123	 # AVX512F
	vrndscalesd	xmm6{k7}, xmm5, xmm4{sae}, 123	 # AVX512F
	vrndscalesd	xmm6{k7}, xmm5, QWORD PTR [ecx], 123	 # AVX512F
	vrndscalesd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vrndscalesd	xmm6{k7}, xmm5, QWORD PTR [edx+1016], 123	 # AVX512F Disp8
	vrndscalesd	xmm6{k7}, xmm5, QWORD PTR [edx+1024], 123	 # AVX512F
	vrndscalesd	xmm6{k7}, xmm5, QWORD PTR [edx-1024], 123	 # AVX512F Disp8
	vrndscalesd	xmm6{k7}, xmm5, QWORD PTR [edx-1032], 123	 # AVX512F

	vrndscaless	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512F
	vrndscaless	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512F
	vrndscaless	xmm6{k7}, xmm5, xmm4{sae}, 0xab	 # AVX512F
	vrndscaless	xmm6{k7}, xmm5, xmm4, 123	 # AVX512F
	vrndscaless	xmm6{k7}, xmm5, xmm4{sae}, 123	 # AVX512F
	vrndscaless	xmm6{k7}, xmm5, DWORD PTR [ecx], 123	 # AVX512F
	vrndscaless	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456], 123	 # AVX512F
	vrndscaless	xmm6{k7}, xmm5, DWORD PTR [edx+508], 123	 # AVX512F Disp8
	vrndscaless	xmm6{k7}, xmm5, DWORD PTR [edx+512], 123	 # AVX512F
	vrndscaless	xmm6{k7}, xmm5, DWORD PTR [edx-512], 123	 # AVX512F Disp8
	vrndscaless	xmm6{k7}, xmm5, DWORD PTR [edx-516], 123	 # AVX512F

	vpcompressq	ZMMWORD PTR [ecx], zmm6	 # AVX512F
	vpcompressq	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpcompressq	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpcompressq	ZMMWORD PTR [edx+1016], zmm6	 # AVX512F Disp8
	vpcompressq	ZMMWORD PTR [edx+1024], zmm6	 # AVX512F
	vpcompressq	ZMMWORD PTR [edx-1024], zmm6	 # AVX512F Disp8
	vpcompressq	ZMMWORD PTR [edx-1032], zmm6	 # AVX512F

	vpcompressq	zmm6, zmm5	 # AVX512F
	vpcompressq	zmm6{k7}, zmm5	 # AVX512F
	vpcompressq	zmm6{k7}{z}, zmm5	 # AVX512F

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
	kmovw	k5, WORD PTR [ecx]	 # AVX512F
	kmovw	k5, WORD PTR [esp+esi*8-123456]	 # AVX512F

	kmovw	WORD PTR [ecx], k5	 # AVX512F
	kmovw	WORD PTR [esp+esi*8-123456], k5	 # AVX512F

	kmovw	k5, eax	 # AVX512F
	kmovw	k5, ebp	 # AVX512F

	kmovw	eax, k5	 # AVX512F
	kmovw	ebp, k5	 # AVX512F

	kunpckbw	k5, k6, k7	 # AVX512F

	vcvtps2ph	YMMWORD PTR [ecx], zmm6, 0xab	 # AVX512F
	vcvtps2ph	YMMWORD PTR [ecx]{k7}, zmm6, 0xab	 # AVX512F
	vcvtps2ph	YMMWORD PTR [ecx], zmm6, 123	 # AVX512F
	vcvtps2ph	YMMWORD PTR [esp+esi*8-123456], zmm6, 123	 # AVX512F
	vcvtps2ph	YMMWORD PTR [edx+4064], zmm6, 123	 # AVX512F Disp8
	vcvtps2ph	YMMWORD PTR [edx+4096], zmm6, 123	 # AVX512F
	vcvtps2ph	YMMWORD PTR [edx-4096], zmm6, 123	 # AVX512F Disp8
	vcvtps2ph	YMMWORD PTR [edx-4128], zmm6, 123	 # AVX512F

	vextractf32x4	XMMWORD PTR [ecx], zmm6, 0xab	 # AVX512F
	vextractf32x4	XMMWORD PTR [ecx]{k7}, zmm6, 0xab	 # AVX512F
	vextractf32x4	XMMWORD PTR [ecx], zmm6, 123	 # AVX512F
	vextractf32x4	XMMWORD PTR [esp+esi*8-123456], zmm6, 123	 # AVX512F
	vextractf32x4	XMMWORD PTR [edx+2032], zmm6, 123	 # AVX512F Disp8
	vextractf32x4	XMMWORD PTR [edx+2048], zmm6, 123	 # AVX512F
	vextractf32x4	XMMWORD PTR [edx-2048], zmm6, 123	 # AVX512F Disp8
	vextractf32x4	XMMWORD PTR [edx-2064], zmm6, 123	 # AVX512F

	vextractf64x4	YMMWORD PTR [ecx], zmm6, 0xab	 # AVX512F
	vextractf64x4	YMMWORD PTR [ecx]{k7}, zmm6, 0xab	 # AVX512F
	vextractf64x4	YMMWORD PTR [ecx], zmm6, 123	 # AVX512F
	vextractf64x4	YMMWORD PTR [esp+esi*8-123456], zmm6, 123	 # AVX512F
	vextractf64x4	YMMWORD PTR [edx+4064], zmm6, 123	 # AVX512F Disp8
	vextractf64x4	YMMWORD PTR [edx+4096], zmm6, 123	 # AVX512F
	vextractf64x4	YMMWORD PTR [edx-4096], zmm6, 123	 # AVX512F Disp8
	vextractf64x4	YMMWORD PTR [edx-4128], zmm6, 123	 # AVX512F

	vextracti32x4	XMMWORD PTR [ecx], zmm6, 0xab	 # AVX512F
	vextracti32x4	XMMWORD PTR [ecx]{k7}, zmm6, 0xab	 # AVX512F
	vextracti32x4	XMMWORD PTR [ecx], zmm6, 123	 # AVX512F
	vextracti32x4	XMMWORD PTR [esp+esi*8-123456], zmm6, 123	 # AVX512F
	vextracti32x4	XMMWORD PTR [edx+2032], zmm6, 123	 # AVX512F Disp8
	vextracti32x4	XMMWORD PTR [edx+2048], zmm6, 123	 # AVX512F
	vextracti32x4	XMMWORD PTR [edx-2048], zmm6, 123	 # AVX512F Disp8
	vextracti32x4	XMMWORD PTR [edx-2064], zmm6, 123	 # AVX512F

	vextracti64x4	YMMWORD PTR [ecx], zmm6, 0xab	 # AVX512F
	vextracti64x4	YMMWORD PTR [ecx]{k7}, zmm6, 0xab	 # AVX512F
	vextracti64x4	YMMWORD PTR [ecx], zmm6, 123	 # AVX512F
	vextracti64x4	YMMWORD PTR [esp+esi*8-123456], zmm6, 123	 # AVX512F
	vextracti64x4	YMMWORD PTR [edx+4064], zmm6, 123	 # AVX512F Disp8
	vextracti64x4	YMMWORD PTR [edx+4096], zmm6, 123	 # AVX512F
	vextracti64x4	YMMWORD PTR [edx-4096], zmm6, 123	 # AVX512F Disp8
	vextracti64x4	YMMWORD PTR [edx-4128], zmm6, 123	 # AVX512F

	vmovapd	ZMMWORD PTR [ecx], zmm6	 # AVX512F
	vmovapd	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vmovapd	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vmovapd	ZMMWORD PTR [edx+8128], zmm6	 # AVX512F Disp8
	vmovapd	ZMMWORD PTR [edx+8192], zmm6	 # AVX512F
	vmovapd	ZMMWORD PTR [edx-8192], zmm6	 # AVX512F Disp8
	vmovapd	ZMMWORD PTR [edx-8256], zmm6	 # AVX512F

	vmovaps	ZMMWORD PTR [ecx], zmm6	 # AVX512F
	vmovaps	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vmovaps	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vmovaps	ZMMWORD PTR [edx+8128], zmm6	 # AVX512F Disp8
	vmovaps	ZMMWORD PTR [edx+8192], zmm6	 # AVX512F
	vmovaps	ZMMWORD PTR [edx-8192], zmm6	 # AVX512F Disp8
	vmovaps	ZMMWORD PTR [edx-8256], zmm6	 # AVX512F

	vmovdqa32	ZMMWORD PTR [ecx], zmm6	 # AVX512F
	vmovdqa32	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vmovdqa32	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vmovdqa32	ZMMWORD PTR [edx+8128], zmm6	 # AVX512F Disp8
	vmovdqa32	ZMMWORD PTR [edx+8192], zmm6	 # AVX512F
	vmovdqa32	ZMMWORD PTR [edx-8192], zmm6	 # AVX512F Disp8
	vmovdqa32	ZMMWORD PTR [edx-8256], zmm6	 # AVX512F

	vmovdqa64	ZMMWORD PTR [ecx], zmm6	 # AVX512F
	vmovdqa64	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vmovdqa64	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vmovdqa64	ZMMWORD PTR [edx+8128], zmm6	 # AVX512F Disp8
	vmovdqa64	ZMMWORD PTR [edx+8192], zmm6	 # AVX512F
	vmovdqa64	ZMMWORD PTR [edx-8192], zmm6	 # AVX512F Disp8
	vmovdqa64	ZMMWORD PTR [edx-8256], zmm6	 # AVX512F

	vmovdqu32	ZMMWORD PTR [ecx], zmm6	 # AVX512F
	vmovdqu32	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vmovdqu32	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vmovdqu32	ZMMWORD PTR [edx+8128], zmm6	 # AVX512F Disp8
	vmovdqu32	ZMMWORD PTR [edx+8192], zmm6	 # AVX512F
	vmovdqu32	ZMMWORD PTR [edx-8192], zmm6	 # AVX512F Disp8
	vmovdqu32	ZMMWORD PTR [edx-8256], zmm6	 # AVX512F

	vmovdqu64	ZMMWORD PTR [ecx], zmm6	 # AVX512F
	vmovdqu64	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vmovdqu64	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vmovdqu64	ZMMWORD PTR [edx+8128], zmm6	 # AVX512F Disp8
	vmovdqu64	ZMMWORD PTR [edx+8192], zmm6	 # AVX512F
	vmovdqu64	ZMMWORD PTR [edx-8192], zmm6	 # AVX512F Disp8
	vmovdqu64	ZMMWORD PTR [edx-8256], zmm6	 # AVX512F

	vmovupd	ZMMWORD PTR [ecx], zmm6	 # AVX512F
	vmovupd	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vmovupd	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vmovupd	ZMMWORD PTR [edx+8128], zmm6	 # AVX512F Disp8
	vmovupd	ZMMWORD PTR [edx+8192], zmm6	 # AVX512F
	vmovupd	ZMMWORD PTR [edx-8192], zmm6	 # AVX512F Disp8
	vmovupd	ZMMWORD PTR [edx-8256], zmm6	 # AVX512F

	vmovups	ZMMWORD PTR [ecx], zmm6	 # AVX512F
	vmovups	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vmovups	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vmovups	ZMMWORD PTR [edx+8128], zmm6	 # AVX512F Disp8
	vmovups	ZMMWORD PTR [edx+8192], zmm6	 # AVX512F
	vmovups	ZMMWORD PTR [edx-8192], zmm6	 # AVX512F Disp8
	vmovups	ZMMWORD PTR [edx-8256], zmm6	 # AVX512F

	vpmovqb	QWORD PTR [ecx], zmm6	 # AVX512F
	vpmovqb	QWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpmovqb	QWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpmovqb	QWORD PTR [edx+1016], zmm6	 # AVX512F Disp8
	vpmovqb	QWORD PTR [edx+1024], zmm6	 # AVX512F
	vpmovqb	QWORD PTR [edx-1024], zmm6	 # AVX512F Disp8
	vpmovqb	QWORD PTR [edx-1032], zmm6	 # AVX512F

	vpmovsqb	QWORD PTR [ecx], zmm6	 # AVX512F
	vpmovsqb	QWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpmovsqb	QWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpmovsqb	QWORD PTR [edx+1016], zmm6	 # AVX512F Disp8
	vpmovsqb	QWORD PTR [edx+1024], zmm6	 # AVX512F
	vpmovsqb	QWORD PTR [edx-1024], zmm6	 # AVX512F Disp8
	vpmovsqb	QWORD PTR [edx-1032], zmm6	 # AVX512F

	vpmovusqb	QWORD PTR [ecx], zmm6	 # AVX512F
	vpmovusqb	QWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpmovusqb	QWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpmovusqb	QWORD PTR [edx+1016], zmm6	 # AVX512F Disp8
	vpmovusqb	QWORD PTR [edx+1024], zmm6	 # AVX512F
	vpmovusqb	QWORD PTR [edx-1024], zmm6	 # AVX512F Disp8
	vpmovusqb	QWORD PTR [edx-1032], zmm6	 # AVX512F

	vpmovqw	XMMWORD PTR [ecx], zmm6	 # AVX512F
	vpmovqw	XMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpmovqw	XMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpmovqw	XMMWORD PTR [edx+2032], zmm6	 # AVX512F Disp8
	vpmovqw	XMMWORD PTR [edx+2048], zmm6	 # AVX512F
	vpmovqw	XMMWORD PTR [edx-2048], zmm6	 # AVX512F Disp8
	vpmovqw	XMMWORD PTR [edx-2064], zmm6	 # AVX512F

	vpmovsqw	XMMWORD PTR [ecx], zmm6	 # AVX512F
	vpmovsqw	XMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpmovsqw	XMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpmovsqw	XMMWORD PTR [edx+2032], zmm6	 # AVX512F Disp8
	vpmovsqw	XMMWORD PTR [edx+2048], zmm6	 # AVX512F
	vpmovsqw	XMMWORD PTR [edx-2048], zmm6	 # AVX512F Disp8
	vpmovsqw	XMMWORD PTR [edx-2064], zmm6	 # AVX512F

	vpmovusqw	XMMWORD PTR [ecx], zmm6	 # AVX512F
	vpmovusqw	XMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpmovusqw	XMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpmovusqw	XMMWORD PTR [edx+2032], zmm6	 # AVX512F Disp8
	vpmovusqw	XMMWORD PTR [edx+2048], zmm6	 # AVX512F
	vpmovusqw	XMMWORD PTR [edx-2048], zmm6	 # AVX512F Disp8
	vpmovusqw	XMMWORD PTR [edx-2064], zmm6	 # AVX512F

	vpmovqd	YMMWORD PTR [ecx], zmm6	 # AVX512F
	vpmovqd	YMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpmovqd	YMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpmovqd	YMMWORD PTR [edx+4064], zmm6	 # AVX512F Disp8
	vpmovqd	YMMWORD PTR [edx+4096], zmm6	 # AVX512F
	vpmovqd	YMMWORD PTR [edx-4096], zmm6	 # AVX512F Disp8
	vpmovqd	YMMWORD PTR [edx-4128], zmm6	 # AVX512F

	vpmovsqd	YMMWORD PTR [ecx], zmm6	 # AVX512F
	vpmovsqd	YMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpmovsqd	YMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpmovsqd	YMMWORD PTR [edx+4064], zmm6	 # AVX512F Disp8
	vpmovsqd	YMMWORD PTR [edx+4096], zmm6	 # AVX512F
	vpmovsqd	YMMWORD PTR [edx-4096], zmm6	 # AVX512F Disp8
	vpmovsqd	YMMWORD PTR [edx-4128], zmm6	 # AVX512F

	vpmovusqd	YMMWORD PTR [ecx], zmm6	 # AVX512F
	vpmovusqd	YMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpmovusqd	YMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpmovusqd	YMMWORD PTR [edx+4064], zmm6	 # AVX512F Disp8
	vpmovusqd	YMMWORD PTR [edx+4096], zmm6	 # AVX512F
	vpmovusqd	YMMWORD PTR [edx-4096], zmm6	 # AVX512F Disp8
	vpmovusqd	YMMWORD PTR [edx-4128], zmm6	 # AVX512F

	vpmovdb	XMMWORD PTR [ecx], zmm6	 # AVX512F
	vpmovdb	XMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpmovdb	XMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpmovdb	XMMWORD PTR [edx+2032], zmm6	 # AVX512F Disp8
	vpmovdb	XMMWORD PTR [edx+2048], zmm6	 # AVX512F
	vpmovdb	XMMWORD PTR [edx-2048], zmm6	 # AVX512F Disp8
	vpmovdb	XMMWORD PTR [edx-2064], zmm6	 # AVX512F

	vpmovsdb	XMMWORD PTR [ecx], zmm6	 # AVX512F
	vpmovsdb	XMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpmovsdb	XMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpmovsdb	XMMWORD PTR [edx+2032], zmm6	 # AVX512F Disp8
	vpmovsdb	XMMWORD PTR [edx+2048], zmm6	 # AVX512F
	vpmovsdb	XMMWORD PTR [edx-2048], zmm6	 # AVX512F Disp8
	vpmovsdb	XMMWORD PTR [edx-2064], zmm6	 # AVX512F

	vpmovusdb	XMMWORD PTR [ecx], zmm6	 # AVX512F
	vpmovusdb	XMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpmovusdb	XMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpmovusdb	XMMWORD PTR [edx+2032], zmm6	 # AVX512F Disp8
	vpmovusdb	XMMWORD PTR [edx+2048], zmm6	 # AVX512F
	vpmovusdb	XMMWORD PTR [edx-2048], zmm6	 # AVX512F Disp8
	vpmovusdb	XMMWORD PTR [edx-2064], zmm6	 # AVX512F

	vpmovdw	YMMWORD PTR [ecx], zmm6	 # AVX512F
	vpmovdw	YMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpmovdw	YMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpmovdw	YMMWORD PTR [edx+4064], zmm6	 # AVX512F Disp8
	vpmovdw	YMMWORD PTR [edx+4096], zmm6	 # AVX512F
	vpmovdw	YMMWORD PTR [edx-4096], zmm6	 # AVX512F Disp8
	vpmovdw	YMMWORD PTR [edx-4128], zmm6	 # AVX512F

	vpmovsdw	YMMWORD PTR [ecx], zmm6	 # AVX512F
	vpmovsdw	YMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpmovsdw	YMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpmovsdw	YMMWORD PTR [edx+4064], zmm6	 # AVX512F Disp8
	vpmovsdw	YMMWORD PTR [edx+4096], zmm6	 # AVX512F
	vpmovsdw	YMMWORD PTR [edx-4096], zmm6	 # AVX512F Disp8
	vpmovsdw	YMMWORD PTR [edx-4128], zmm6	 # AVX512F

	vpmovusdw	YMMWORD PTR [ecx], zmm6	 # AVX512F
	vpmovusdw	YMMWORD PTR [ecx]{k7}, zmm6	 # AVX512F
	vpmovusdw	YMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512F
	vpmovusdw	YMMWORD PTR [edx+4064], zmm6	 # AVX512F Disp8
	vpmovusdw	YMMWORD PTR [edx+4096], zmm6	 # AVX512F
	vpmovusdw	YMMWORD PTR [edx-4096], zmm6	 # AVX512F Disp8
	vpmovusdw	YMMWORD PTR [edx-4128], zmm6	 # AVX512F

	vcvttpd2udq	ymm6{k7}, zmm5	 # AVX512F
	vcvttpd2udq	ymm6{k7}{z}, zmm5	 # AVX512F
	vcvttpd2udq	ymm6{k7}, zmm5{sae}	 # AVX512F
	vcvttpd2udq	ymm6{k7}, ZMMWORD PTR [ecx]	 # AVX512F
	vcvttpd2udq	ymm6{k7}, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvttpd2udq	ymm6{k7}, qword bcst [eax]	 # AVX512F
	vcvttpd2udq	ymm6{k7}, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcvttpd2udq	ymm6{k7}, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcvttpd2udq	ymm6{k7}, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcvttpd2udq	ymm6{k7}, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcvttpd2udq	ymm6{k7}, qword bcst [edx+1016]	 # AVX512F Disp8
	vcvttpd2udq	ymm6{k7}, qword bcst [edx+1024]	 # AVX512F
	vcvttpd2udq	ymm6{k7}, qword bcst [edx-1024]	 # AVX512F Disp8
	vcvttpd2udq	ymm6{k7}, qword bcst [edx-1032]	 # AVX512F

	vcvttps2udq	zmm6, zmm5	 # AVX512F
	vcvttps2udq	zmm6{k7}, zmm5	 # AVX512F
	vcvttps2udq	zmm6{k7}{z}, zmm5	 # AVX512F
	vcvttps2udq	zmm6, zmm5{sae}	 # AVX512F
	vcvttps2udq	zmm6, ZMMWORD PTR [ecx]	 # AVX512F
	vcvttps2udq	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvttps2udq	zmm6, dword bcst [eax]	 # AVX512F
	vcvttps2udq	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vcvttps2udq	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512F
	vcvttps2udq	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vcvttps2udq	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512F
	vcvttps2udq	zmm6, dword bcst [edx+508]	 # AVX512F Disp8
	vcvttps2udq	zmm6, dword bcst [edx+512]	 # AVX512F
	vcvttps2udq	zmm6, dword bcst [edx-512]	 # AVX512F Disp8
	vcvttps2udq	zmm6, dword bcst [edx-516]	 # AVX512F

	vcvttsd2usi	eax, xmm6	 # AVX512F
	vcvttsd2usi	eax, xmm6{sae}	 # AVX512F
	vcvttsd2usi	eax, QWORD PTR [ecx]	 # AVX512F
	vcvttsd2usi	eax, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvttsd2usi	eax, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcvttsd2usi	eax, QWORD PTR [edx+1024]	 # AVX512F
	vcvttsd2usi	eax, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcvttsd2usi	eax, QWORD PTR [edx-1032]	 # AVX512F
	vcvttsd2usi	ebp, xmm6	 # AVX512F
	vcvttsd2usi	ebp, xmm6{sae}	 # AVX512F
	vcvttsd2usi	ebp, QWORD PTR [ecx]	 # AVX512F
	vcvttsd2usi	ebp, QWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvttsd2usi	ebp, QWORD PTR [edx+1016]	 # AVX512F Disp8
	vcvttsd2usi	ebp, QWORD PTR [edx+1024]	 # AVX512F
	vcvttsd2usi	ebp, QWORD PTR [edx-1024]	 # AVX512F Disp8
	vcvttsd2usi	ebp, QWORD PTR [edx-1032]	 # AVX512F

	vcvttss2usi	eax, xmm6	 # AVX512F
	vcvttss2usi	eax, xmm6{sae}	 # AVX512F
	vcvttss2usi	eax, DWORD PTR [ecx]	 # AVX512F
	vcvttss2usi	eax, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvttss2usi	eax, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcvttss2usi	eax, DWORD PTR [edx+512]	 # AVX512F
	vcvttss2usi	eax, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcvttss2usi	eax, DWORD PTR [edx-516]	 # AVX512F
	vcvttss2usi	ebp, xmm6	 # AVX512F
	vcvttss2usi	ebp, xmm6{sae}	 # AVX512F
	vcvttss2usi	ebp, DWORD PTR [ecx]	 # AVX512F
	vcvttss2usi	ebp, DWORD PTR [esp+esi*8-123456]	 # AVX512F
	vcvttss2usi	ebp, DWORD PTR [edx+508]	 # AVX512F Disp8
	vcvttss2usi	ebp, DWORD PTR [edx+512]	 # AVX512F
	vcvttss2usi	ebp, DWORD PTR [edx-512]	 # AVX512F Disp8
	vcvttss2usi	ebp, DWORD PTR [edx-516]	 # AVX512F

	vpermi2d	zmm6, zmm5, zmm4	 # AVX512F
	vpermi2d	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpermi2d	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpermi2d	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpermi2d	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpermi2d	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpermi2d	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpermi2d	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpermi2d	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpermi2d	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpermi2d	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpermi2d	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpermi2d	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpermi2d	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpermi2q	zmm6, zmm5, zmm4	 # AVX512F
	vpermi2q	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpermi2q	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpermi2q	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpermi2q	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpermi2q	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpermi2q	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpermi2q	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpermi2q	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpermi2q	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpermi2q	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpermi2q	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpermi2q	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpermi2q	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vpermi2ps	zmm6, zmm5, zmm4	 # AVX512F
	vpermi2ps	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpermi2ps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpermi2ps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpermi2ps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpermi2ps	zmm6, zmm5, dword bcst [eax]	 # AVX512F
	vpermi2ps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpermi2ps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpermi2ps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpermi2ps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpermi2ps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vpermi2ps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512F
	vpermi2ps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vpermi2ps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512F

	vpermi2pd	zmm6, zmm5, zmm4	 # AVX512F
	vpermi2pd	zmm6{k7}, zmm5, zmm4	 # AVX512F
	vpermi2pd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F
	vpermi2pd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vpermi2pd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vpermi2pd	zmm6, zmm5, qword bcst [eax]	 # AVX512F
	vpermi2pd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vpermi2pd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vpermi2pd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vpermi2pd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vpermi2pd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vpermi2pd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512F
	vpermi2pd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vpermi2pd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512F

	vptestnmd	k5, zmm5, zmm4	 # AVX512F
	vptestnmd	k5{k7}, zmm5, zmm4	 # AVX512F
	vptestnmd	k5, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vptestnmd	k5, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vptestnmd	k5, zmm5, dword bcst [eax]	 # AVX512F
	vptestnmd	k5, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vptestnmd	k5, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vptestnmd	k5, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vptestnmd	k5, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vptestnmd	k5, zmm5, dword bcst [edx+508]	 # AVX512F Disp8
	vptestnmd	k5, zmm5, dword bcst [edx+512]	 # AVX512F
	vptestnmd	k5, zmm5, dword bcst [edx-512]	 # AVX512F Disp8
	vptestnmd	k5, zmm5, dword bcst [edx-516]	 # AVX512F

	vptestnmq	k5, zmm5, zmm4	 # AVX512F
	vptestnmq	k5{k7}, zmm5, zmm4	 # AVX512F
	vptestnmq	k5, zmm5, ZMMWORD PTR [ecx]	 # AVX512F
	vptestnmq	k5, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F
	vptestnmq	k5, zmm5, qword bcst [eax]	 # AVX512F
	vptestnmq	k5, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F Disp8
	vptestnmq	k5, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512F
	vptestnmq	k5, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512F Disp8
	vptestnmq	k5, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512F
	vptestnmq	k5, zmm5, qword bcst [edx+1016]	 # AVX512F Disp8
	vptestnmq	k5, zmm5, qword bcst [edx+1024]	 # AVX512F
	vptestnmq	k5, zmm5, qword bcst [edx-1024]	 # AVX512F Disp8
	vptestnmq	k5, zmm5, qword bcst [edx-1032]	 # AVX512F

	vaddps		zmm0, zmm0, [bx]
	vaddps		zmm0, zmm0, [bx+0x40]
	vaddps		zmm0, zmm0, [bx+0x1234]
