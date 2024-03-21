# Check 32bit AVX512-FP16,AVX512VL instructions

	.allow_index_reg
	.text
_start:
	vaddph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vaddph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vaddph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vaddph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vaddph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vaddph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vaddph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vaddph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vaddph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vaddph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vaddph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vaddph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcmpph	$123, %ymm4, %ymm5, %k5	 #AVX512-FP16,AVX512VL
	vcmpph	$123, %ymm4, %ymm5, %k5{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcmpph	$123, %xmm4, %xmm5, %k5	 #AVX512-FP16,AVX512VL
	vcmpph	$123, %xmm4, %xmm5, %k5{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcmpph	$123, 0x10000000(%esp, %esi, 8), %xmm5, %k5{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcmpph	$123, (%ecx){1to8}, %xmm5, %k5	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcmpph	$123, 2032(%ecx), %xmm5, %k5	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcmpph	$123, -256(%edx){1to8}, %xmm5, %k5{%k7}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING
	vcmpph	$123, 0x10000000(%esp, %esi, 8), %ymm5, %k5{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcmpph	$123, (%ecx){1to16}, %ymm5, %k5	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcmpph	$123, 4064(%ecx), %ymm5, %k5	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcmpph	$123, -256(%edx){1to16}, %ymm5, %k5{%k7}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING
	vcvtdq2ph	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtdq2ph	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtdq2ph	%ymm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtdq2ph	%ymm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtdq2phx	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtdq2ph	(%ecx){1to4}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtdq2phx	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtdq2ph	-512(%edx){1to4}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtdq2ph	(%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtdq2phy	4064(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtdq2ph	-512(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtpd2ph	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtpd2ph	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtpd2ph	%ymm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtpd2ph	%ymm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtpd2phx	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtpd2ph	(%ecx){1to2}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtpd2phx	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtpd2ph	-1024(%edx){1to2}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtpd2ph	(%ecx){1to4}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtpd2phy	4064(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtpd2ph	-1024(%edx){1to4}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2dq	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtph2dq	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2dq	%xmm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvtph2dq	%xmm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2dq	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2dq	(%ecx){1to4}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2dq	1016(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2dq	-256(%edx){1to4}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2dq	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2dq	(%ecx){1to8}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2dq	2032(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2dq	-256(%edx){1to8}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2pd	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtph2pd	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2pd	%xmm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvtph2pd	%xmm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2pd	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2pd	(%ecx){1to2}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2pd	508(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2pd	-256(%edx){1to2}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2pd	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2pd	(%ecx){1to4}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2pd	1016(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2pd	-256(%edx){1to4}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2psx	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtph2psx	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2psx	%xmm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvtph2psx	%xmm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2psx	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2psx	(%ecx){1to4}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2psx	1016(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2psx	-256(%edx){1to4}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2psx	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2psx	(%ecx){1to8}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2psx	2032(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2psx	-256(%edx){1to8}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2qq	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtph2qq	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2qq	%xmm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvtph2qq	%xmm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2qq	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2qq	(%ecx){1to2}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2qq	508(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2qq	-256(%edx){1to2}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2qq	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2qq	(%ecx){1to4}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2qq	1016(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2qq	-256(%edx){1to4}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2udq	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtph2udq	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2udq	%xmm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvtph2udq	%xmm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2udq	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2udq	(%ecx){1to4}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2udq	1016(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2udq	-256(%edx){1to4}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2udq	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2udq	(%ecx){1to8}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2udq	2032(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2udq	-256(%edx){1to8}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uqq	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtph2uqq	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2uqq	%xmm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvtph2uqq	%xmm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2uqq	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2uqq	(%ecx){1to2}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2uqq	508(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2uqq	-256(%edx){1to2}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uqq	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2uqq	(%ecx){1to4}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2uqq	1016(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2uqq	-256(%edx){1to4}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uw	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtph2uw	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2uw	%ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvtph2uw	%ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2uw	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2uw	(%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2uw	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2uw	-256(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uw	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2uw	(%ecx){1to16}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2uw	4064(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2uw	-256(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2w	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtph2w	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2w	%ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvtph2w	%ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2w	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2w	(%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2w	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2w	-256(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2w	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2w	(%ecx){1to16}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2w	4064(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2w	-256(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtps2phx	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtps2phx	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtps2phx	%ymm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtps2phx	%ymm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtps2phxx	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtps2phx	(%ecx){1to4}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtps2phxx	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtps2phx	-512(%edx){1to4}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtps2phx	(%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtps2phxy	4064(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtps2phx	-512(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtqq2ph	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtqq2ph	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtqq2ph	%ymm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtqq2ph	%ymm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtqq2phx	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtqq2ph	(%ecx){1to2}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtqq2phx	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtqq2ph	-1024(%edx){1to2}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtqq2ph	(%ecx){1to4}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtqq2phy	4064(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtqq2ph	-1024(%edx){1to4}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2dq	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvttph2dq	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2dq	%xmm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvttph2dq	%xmm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2dq	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2dq	(%ecx){1to4}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2dq	1016(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2dq	-256(%edx){1to4}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2dq	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2dq	(%ecx){1to8}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2dq	2032(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2dq	-256(%edx){1to8}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2qq	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvttph2qq	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2qq	%xmm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvttph2qq	%xmm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2qq	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2qq	(%ecx){1to2}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2qq	508(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2qq	-256(%edx){1to2}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2qq	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2qq	(%ecx){1to4}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2qq	1016(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2qq	-256(%edx){1to4}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2udq	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvttph2udq	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2udq	%xmm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvttph2udq	%xmm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2udq	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2udq	(%ecx){1to4}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2udq	1016(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2udq	-256(%edx){1to4}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2udq	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2udq	(%ecx){1to8}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2udq	2032(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2udq	-256(%edx){1to8}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uqq	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvttph2uqq	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2uqq	%xmm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvttph2uqq	%xmm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2uqq	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2uqq	(%ecx){1to2}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2uqq	508(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2uqq	-256(%edx){1to2}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uqq	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2uqq	(%ecx){1to4}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2uqq	1016(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2uqq	-256(%edx){1to4}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uw	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvttph2uw	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2uw	%ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvttph2uw	%ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2uw	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2uw	(%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2uw	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2uw	-256(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uw	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2uw	(%ecx){1to16}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2uw	4064(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2uw	-256(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2w	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvttph2w	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2w	%ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvttph2w	%ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2w	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2w	(%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2w	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2w	-256(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2w	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2w	(%ecx){1to16}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2w	4064(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2w	-256(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtudq2ph	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtudq2ph	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtudq2ph	%ymm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtudq2ph	%ymm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtudq2phx	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtudq2ph	(%ecx){1to4}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtudq2phx	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtudq2ph	-512(%edx){1to4}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtudq2ph	(%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtudq2phy	4064(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtudq2ph	-512(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtuqq2ph	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtuqq2ph	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtuqq2ph	%ymm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtuqq2ph	%ymm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtuqq2phx	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtuqq2ph	(%ecx){1to2}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtuqq2phx	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtuqq2ph	-1024(%edx){1to2}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtuqq2ph	(%ecx){1to4}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtuqq2phy	4064(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtuqq2ph	-1024(%edx){1to4}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtuw2ph	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtuw2ph	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtuw2ph	%ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvtuw2ph	%ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtuw2ph	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtuw2ph	(%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtuw2ph	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtuw2ph	-256(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtuw2ph	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtuw2ph	(%ecx){1to16}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtuw2ph	4064(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtuw2ph	-256(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtw2ph	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vcvtw2ph	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtw2ph	%ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vcvtw2ph	%ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtw2ph	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtw2ph	(%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtw2ph	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtw2ph	-256(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtw2ph	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtw2ph	(%ecx){1to16}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtw2ph	4064(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtw2ph	-256(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vdivph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vdivph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vdivph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vdivph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vdivph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vdivph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vdivph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vdivph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vdivph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vdivph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vdivph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vdivph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmaddcph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfcmaddcph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfcmaddcph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfcmaddcph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfcmaddcph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfcmaddcph	(%ecx){1to8}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfcmaddcph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfcmaddcph	-512(%edx){1to8}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmaddcph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfcmaddcph	(%ecx){1to4}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfcmaddcph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfcmaddcph	-512(%edx){1to4}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmulcph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfcmulcph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfcmulcph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfcmulcph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfcmulcph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfcmulcph	(%ecx){1to8}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfcmulcph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfcmulcph	-512(%edx){1to8}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmulcph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfcmulcph	(%ecx){1to4}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfcmulcph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfcmulcph	-512(%edx){1to4}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd132ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfmadd132ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmadd132ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfmadd132ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmadd132ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmadd132ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmadd132ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmadd132ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd132ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmadd132ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmadd132ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmadd132ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd213ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfmadd213ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmadd213ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfmadd213ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmadd213ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmadd213ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmadd213ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmadd213ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd213ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmadd213ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmadd213ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmadd213ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd231ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfmadd231ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmadd231ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfmadd231ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmadd231ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmadd231ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmadd231ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmadd231ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd231ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmadd231ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmadd231ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmadd231ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddcph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfmaddcph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddcph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfmaddcph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddcph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddcph	(%ecx){1to8}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddcph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddcph	-512(%edx){1to8}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddcph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddcph	(%ecx){1to4}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddcph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddcph	-512(%edx){1to4}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub132ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfmaddsub132ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddsub132ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfmaddsub132ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddsub132ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddsub132ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddsub132ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddsub132ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub132ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddsub132ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddsub132ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddsub132ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub213ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfmaddsub213ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddsub213ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfmaddsub213ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddsub213ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddsub213ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddsub213ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddsub213ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub213ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddsub213ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddsub213ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddsub213ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub231ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfmaddsub231ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddsub231ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfmaddsub231ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddsub231ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddsub231ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddsub231ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddsub231ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub231ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddsub231ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddsub231ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddsub231ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub132ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfmsub132ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsub132ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfmsub132ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsub132ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsub132ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsub132ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsub132ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub132ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsub132ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsub132ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsub132ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub213ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfmsub213ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsub213ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfmsub213ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsub213ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsub213ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsub213ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsub213ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub213ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsub213ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsub213ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsub213ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub231ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfmsub231ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsub231ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfmsub231ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsub231ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsub231ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsub231ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsub231ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub231ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsub231ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsub231ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsub231ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd132ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfmsubadd132ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsubadd132ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfmsubadd132ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsubadd132ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsubadd132ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsubadd132ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsubadd132ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd132ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsubadd132ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsubadd132ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsubadd132ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd213ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfmsubadd213ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsubadd213ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfmsubadd213ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsubadd213ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsubadd213ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsubadd213ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsubadd213ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd213ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsubadd213ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsubadd213ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsubadd213ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd231ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfmsubadd231ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsubadd231ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfmsubadd231ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsubadd231ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsubadd231ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsubadd231ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsubadd231ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd231ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsubadd231ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsubadd231ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsubadd231ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmulcph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfmulcph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmulcph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfmulcph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmulcph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmulcph	(%ecx){1to8}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmulcph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmulcph	-512(%edx){1to8}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmulcph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmulcph	(%ecx){1to4}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmulcph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmulcph	-512(%edx){1to4}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd132ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfnmadd132ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmadd132ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfnmadd132ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmadd132ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmadd132ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmadd132ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmadd132ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd132ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmadd132ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmadd132ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmadd132ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd213ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfnmadd213ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmadd213ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfnmadd213ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmadd213ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmadd213ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmadd213ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmadd213ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd213ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmadd213ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmadd213ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmadd213ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd231ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfnmadd231ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmadd231ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfnmadd231ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmadd231ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmadd231ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmadd231ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmadd231ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd231ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmadd231ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmadd231ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmadd231ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub132ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfnmsub132ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmsub132ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfnmsub132ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmsub132ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmsub132ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmsub132ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmsub132ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub132ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmsub132ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmsub132ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmsub132ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub213ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfnmsub213ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmsub213ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfnmsub213ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmsub213ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmsub213ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmsub213ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmsub213ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub213ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmsub213ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmsub213ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmsub213ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub231ph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vfnmsub231ph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmsub231ph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vfnmsub231ph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmsub231ph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmsub231ph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmsub231ph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmsub231ph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub231ph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmsub231ph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmsub231ph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmsub231ph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfpclassph	$123, %xmm6, %k5	 #AVX512-FP16,AVX512VL
	vfpclassph	$123, %xmm6, %k5{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfpclassph	$123, %ymm6, %k5	 #AVX512-FP16,AVX512VL
	vfpclassph	$123, %ymm6, %k5{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfpclassphx	$123, 0x10000000(%esp, %esi, 8), %k5{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfpclassph	$123, (%ecx){1to8}, %k5	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfpclassphx	$123, 2032(%ecx), %k5	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfpclassph	$123, -256(%edx){1to8}, %k5{%k7}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING
	vfpclassphx	$123, 2(%ecx){1to8}, %k5	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfpclassph	$123, (%ecx){1to16}, %k5	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfpclassphy	$123, 4064(%ecx), %k5	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfpclassph	$123, -256(%edx){1to16}, %k5{%k7}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING
	vfpclassphy	$123, 2(%ecx){1to16}, %k5	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vgetexpph	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vgetexpph	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vgetexpph	%ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vgetexpph	%ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vgetexpph	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vgetexpph	(%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vgetexpph	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vgetexpph	-256(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vgetexpph	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vgetexpph	(%ecx){1to16}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vgetexpph	4064(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vgetexpph	-256(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vgetmantph	$123, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vgetmantph	$123, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vgetmantph	$123, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vgetmantph	$123, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vgetmantph	$123, 0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vgetmantph	$123, (%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vgetmantph	$123, 2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vgetmantph	$123, -256(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vgetmantph	$123, 0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vgetmantph	$123, (%ecx){1to16}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vgetmantph	$123, 4064(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vgetmantph	$123, -256(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmaxph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vmaxph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vmaxph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vmaxph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vmaxph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vmaxph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vmaxph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vmaxph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmaxph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vmaxph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vmaxph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vmaxph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vminph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vminph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vminph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vminph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vminph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vminph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vminph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vminph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vminph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vminph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vminph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vminph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmulph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vmulph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vmulph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vmulph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vmulph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vmulph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vmulph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vmulph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmulph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vmulph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vmulph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vmulph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrcpph	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vrcpph	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vrcpph	%ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vrcpph	%ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vrcpph	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vrcpph	(%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vrcpph	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vrcpph	-256(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrcpph	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vrcpph	(%ecx){1to16}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vrcpph	4064(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vrcpph	-256(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vreduceph	$123, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vreduceph	$123, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vreduceph	$123, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vreduceph	$123, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vreduceph	$123, 0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vreduceph	$123, (%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vreduceph	$123, 2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vreduceph	$123, -256(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vreduceph	$123, 0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vreduceph	$123, (%ecx){1to16}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vreduceph	$123, 4064(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vreduceph	$123, -256(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrndscaleph	$123, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vrndscaleph	$123, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vrndscaleph	$123, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vrndscaleph	$123, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vrndscaleph	$123, 0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vrndscaleph	$123, (%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vrndscaleph	$123, 2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vrndscaleph	$123, -256(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrndscaleph	$123, 0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vrndscaleph	$123, (%ecx){1to16}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vrndscaleph	$123, 4064(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vrndscaleph	$123, -256(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrsqrtph	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vrsqrtph	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vrsqrtph	%ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vrsqrtph	%ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vrsqrtph	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vrsqrtph	(%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vrsqrtph	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vrsqrtph	-256(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrsqrtph	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vrsqrtph	(%ecx){1to16}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vrsqrtph	4064(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vrsqrtph	-256(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vscalefph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vscalefph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vscalefph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vscalefph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vscalefph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vscalefph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vscalefph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vscalefph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vscalefph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vscalefph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vscalefph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vscalefph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsqrtph	%xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vsqrtph	%xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vsqrtph	%ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vsqrtph	%ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vsqrtph	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vsqrtph	(%ecx){1to8}, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vsqrtph	2032(%ecx), %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vsqrtph	-256(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsqrtph	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vsqrtph	(%ecx){1to16}, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vsqrtph	4064(%ecx), %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vsqrtph	-256(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsubph	%ymm4, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL
	vsubph	%ymm4, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vsubph	%xmm4, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL
	vsubph	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vsubph	0x10000000(%esp, %esi, 8), %ymm5, %ymm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vsubph	(%ecx){1to16}, %ymm5, %ymm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vsubph	4064(%ecx), %ymm5, %ymm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vsubph	-256(%edx){1to16}, %ymm5, %ymm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsubph	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vsubph	(%ecx){1to8}, %xmm5, %xmm6	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vsubph	2032(%ecx), %xmm5, %xmm6	 #AVX512-FP16,AVX512VL Disp8(7f)
	vsubph	-256(%edx){1to8}, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL

.intel_syntax noprefix
	vaddph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vaddph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vaddph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vaddph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vaddph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vaddph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vaddph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vaddph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vaddph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vaddph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vaddph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vaddph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcmpph	k5, ymm5, ymm4, 123	 #AVX512-FP16,AVX512VL
	vcmpph	k5{k7}, ymm5, ymm4, 123	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcmpph	k5, xmm5, xmm4, 123	 #AVX512-FP16,AVX512VL
	vcmpph	k5{k7}, xmm5, xmm4, 123	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcmpph	k5{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcmpph	k5, xmm5, WORD BCST [ecx], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcmpph	k5, xmm5, XMMWORD PTR [ecx+2032], 123	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcmpph	k5{k7}, xmm5, WORD BCST [edx-256], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING
	vcmpph	k5{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcmpph	k5, ymm5, WORD BCST [ecx], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcmpph	k5, ymm5, YMMWORD PTR [ecx+4064], 123	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcmpph	k5{k7}, ymm5, WORD BCST [edx-256], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING
	vcvtdq2ph	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtdq2ph	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtdq2ph	xmm6, ymm5	 #AVX512-FP16,AVX512VL
	vcvtdq2ph	xmm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtdq2ph	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtdq2ph	xmm6, DWORD BCST [ecx]{1to4}	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtdq2ph	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtdq2ph	xmm6{k7}{z}, DWORD BCST [edx-512]{1to4}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtdq2ph	xmm6, DWORD BCST [ecx]{1to8}	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtdq2ph	xmm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtdq2ph	xmm6{k7}{z}, DWORD BCST [edx-512]{1to8}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtpd2ph	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtpd2ph	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtpd2ph	xmm6, ymm5	 #AVX512-FP16,AVX512VL
	vcvtpd2ph	xmm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtpd2ph	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtpd2ph	xmm6, QWORD BCST [ecx]{1to2}	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtpd2ph	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtpd2ph	xmm6{k7}{z}, QWORD BCST [edx-1024]{1to2}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtpd2ph	xmm6, QWORD BCST [ecx]{1to4}	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtpd2ph	xmm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtpd2ph	xmm6{k7}{z}, QWORD BCST [edx-1024]{1to4}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2dq	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtph2dq	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2dq	ymm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtph2dq	ymm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2dq	xmm6{k7}, QWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2dq	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2dq	xmm6, QWORD PTR [ecx+1016]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2dq	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2dq	ymm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2dq	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2dq	ymm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2dq	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2pd	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtph2pd	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2pd	ymm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtph2pd	ymm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2pd	xmm6{k7}, DWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2pd	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2pd	xmm6, DWORD PTR [ecx+508]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2pd	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2pd	ymm6{k7}, QWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2pd	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2pd	ymm6, QWORD PTR [ecx+1016]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2pd	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2psx	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtph2psx	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2psx	ymm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtph2psx	ymm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2psx	xmm6{k7}, QWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2psx	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2psx	xmm6, QWORD PTR [ecx+1016]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2psx	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2psx	ymm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2psx	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2psx	ymm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2psx	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2qq	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtph2qq	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2qq	ymm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtph2qq	ymm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2qq	xmm6{k7}, DWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2qq	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2qq	xmm6, DWORD PTR [ecx+508]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2qq	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2qq	ymm6{k7}, QWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2qq	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2qq	ymm6, QWORD PTR [ecx+1016]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2qq	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2udq	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtph2udq	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2udq	ymm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtph2udq	ymm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2udq	xmm6{k7}, QWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2udq	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2udq	xmm6, QWORD PTR [ecx+1016]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2udq	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2udq	ymm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2udq	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2udq	ymm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2udq	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uqq	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtph2uqq	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2uqq	ymm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtph2uqq	ymm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2uqq	xmm6{k7}, DWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2uqq	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2uqq	xmm6, DWORD PTR [ecx+508]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2uqq	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uqq	ymm6{k7}, QWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2uqq	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2uqq	ymm6, QWORD PTR [ecx+1016]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2uqq	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uw	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtph2uw	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2uw	ymm6, ymm5	 #AVX512-FP16,AVX512VL
	vcvtph2uw	ymm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2uw	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2uw	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2uw	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2uw	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uw	ymm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2uw	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2uw	ymm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2uw	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2w	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtph2w	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2w	ymm6, ymm5	 #AVX512-FP16,AVX512VL
	vcvtph2w	ymm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtph2w	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2w	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2w	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2w	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2w	ymm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtph2w	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtph2w	ymm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtph2w	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtps2phx	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtps2phx	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtps2phx	xmm6, ymm5	 #AVX512-FP16,AVX512VL
	vcvtps2phx	xmm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtps2phx	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtps2phx	xmm6, DWORD BCST [ecx]{1to4}	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtps2phx	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtps2phx	xmm6{k7}{z}, DWORD BCST [edx-512]{1to4}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtps2phx	xmm6, DWORD BCST [ecx]{1to8}	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtps2phx	xmm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtps2phx	xmm6{k7}{z}, DWORD BCST [edx-512]{1to8}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtqq2ph	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtqq2ph	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtqq2ph	xmm6, ymm5	 #AVX512-FP16,AVX512VL
	vcvtqq2ph	xmm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtqq2ph	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtqq2ph	xmm6, QWORD BCST [ecx]{1to2}	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtqq2ph	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtqq2ph	xmm6{k7}{z}, QWORD BCST [edx-1024]{1to2}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtqq2ph	xmm6, QWORD BCST [ecx]{1to4}	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtqq2ph	xmm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtqq2ph	xmm6{k7}{z}, QWORD BCST [edx-1024]{1to4}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2dq	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvttph2dq	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2dq	ymm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvttph2dq	ymm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2dq	xmm6{k7}, QWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2dq	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2dq	xmm6, QWORD PTR [ecx+1016]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2dq	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2dq	ymm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2dq	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2dq	ymm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2dq	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2qq	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvttph2qq	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2qq	ymm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvttph2qq	ymm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2qq	xmm6{k7}, DWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2qq	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2qq	xmm6, DWORD PTR [ecx+508]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2qq	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2qq	ymm6{k7}, QWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2qq	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2qq	ymm6, QWORD PTR [ecx+1016]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2qq	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2udq	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvttph2udq	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2udq	ymm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvttph2udq	ymm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2udq	xmm6{k7}, QWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2udq	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2udq	xmm6, QWORD PTR [ecx+1016]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2udq	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2udq	ymm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2udq	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2udq	ymm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2udq	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uqq	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvttph2uqq	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2uqq	ymm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvttph2uqq	ymm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2uqq	xmm6{k7}, DWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2uqq	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2uqq	xmm6, DWORD PTR [ecx+508]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2uqq	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uqq	ymm6{k7}, QWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2uqq	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2uqq	ymm6, QWORD PTR [ecx+1016]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2uqq	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uw	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvttph2uw	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2uw	ymm6, ymm5	 #AVX512-FP16,AVX512VL
	vcvttph2uw	ymm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2uw	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2uw	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2uw	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2uw	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uw	ymm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2uw	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2uw	ymm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2uw	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2w	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvttph2w	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2w	ymm6, ymm5	 #AVX512-FP16,AVX512VL
	vcvttph2w	ymm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvttph2w	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2w	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2w	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2w	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2w	ymm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvttph2w	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvttph2w	ymm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvttph2w	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtudq2ph	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtudq2ph	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtudq2ph	xmm6, ymm5	 #AVX512-FP16,AVX512VL
	vcvtudq2ph	xmm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtudq2ph	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtudq2ph	xmm6, DWORD BCST [ecx]{1to4}	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtudq2ph	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtudq2ph	xmm6{k7}{z}, DWORD BCST [edx-512]{1to4}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtudq2ph	xmm6, DWORD BCST [ecx]{1to8}	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtudq2ph	xmm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtudq2ph	xmm6{k7}{z}, DWORD BCST [edx-512]{1to8}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtuqq2ph	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtuqq2ph	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtuqq2ph	xmm6, ymm5	 #AVX512-FP16,AVX512VL
	vcvtuqq2ph	xmm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtuqq2ph	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtuqq2ph	xmm6, QWORD BCST [ecx]{1to2}	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtuqq2ph	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtuqq2ph	xmm6{k7}{z}, QWORD BCST [edx-1024]{1to2}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtuqq2ph	xmm6, QWORD BCST [ecx]{1to4}	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtuqq2ph	xmm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtuqq2ph	xmm6{k7}{z}, QWORD BCST [edx-1024]{1to4}	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtuw2ph	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtuw2ph	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtuw2ph	ymm6, ymm5	 #AVX512-FP16,AVX512VL
	vcvtuw2ph	ymm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtuw2ph	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtuw2ph	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtuw2ph	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtuw2ph	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtuw2ph	ymm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtuw2ph	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtuw2ph	ymm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtuw2ph	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtw2ph	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vcvtw2ph	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtw2ph	ymm6, ymm5	 #AVX512-FP16,AVX512VL
	vcvtw2ph	ymm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vcvtw2ph	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtw2ph	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtw2ph	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtw2ph	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtw2ph	ymm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vcvtw2ph	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vcvtw2ph	ymm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vcvtw2ph	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vdivph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vdivph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vdivph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vdivph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vdivph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vdivph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vdivph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vdivph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vdivph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vdivph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vdivph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vdivph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmaddcph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfcmaddcph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfcmaddcph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfcmaddcph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfcmaddcph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfcmaddcph	ymm6, ymm5, DWORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfcmaddcph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfcmaddcph	ymm6{k7}{z}, ymm5, DWORD BCST [edx-512]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmaddcph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfcmaddcph	xmm6, xmm5, DWORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfcmaddcph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfcmaddcph	xmm6{k7}{z}, xmm5, DWORD BCST [edx-512]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmulcph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfcmulcph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfcmulcph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfcmulcph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfcmulcph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfcmulcph	ymm6, ymm5, DWORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfcmulcph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfcmulcph	ymm6{k7}{z}, ymm5, DWORD BCST [edx-512]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmulcph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfcmulcph	xmm6, xmm5, DWORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfcmulcph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfcmulcph	xmm6{k7}{z}, xmm5, DWORD BCST [edx-512]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd132ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfmadd132ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmadd132ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfmadd132ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmadd132ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmadd132ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmadd132ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmadd132ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd132ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmadd132ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmadd132ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmadd132ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd213ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfmadd213ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmadd213ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfmadd213ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmadd213ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmadd213ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmadd213ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmadd213ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd213ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmadd213ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmadd213ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmadd213ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd231ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfmadd231ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmadd231ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfmadd231ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmadd231ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmadd231ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmadd231ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmadd231ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd231ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmadd231ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmadd231ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmadd231ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddcph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfmaddcph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddcph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfmaddcph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddcph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddcph	ymm6, ymm5, DWORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddcph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddcph	ymm6{k7}{z}, ymm5, DWORD BCST [edx-512]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddcph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddcph	xmm6, xmm5, DWORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddcph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddcph	xmm6{k7}{z}, xmm5, DWORD BCST [edx-512]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub132ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfmaddsub132ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddsub132ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfmaddsub132ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddsub132ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddsub132ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddsub132ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddsub132ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub132ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddsub132ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddsub132ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddsub132ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub213ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfmaddsub213ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddsub213ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfmaddsub213ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddsub213ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddsub213ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddsub213ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddsub213ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub213ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddsub213ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddsub213ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddsub213ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub231ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfmaddsub231ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddsub231ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfmaddsub231ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmaddsub231ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddsub231ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddsub231ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddsub231ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub231ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmaddsub231ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmaddsub231ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmaddsub231ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub132ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfmsub132ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsub132ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfmsub132ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsub132ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsub132ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsub132ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsub132ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub132ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsub132ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsub132ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsub132ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub213ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfmsub213ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsub213ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfmsub213ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsub213ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsub213ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsub213ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsub213ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub213ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsub213ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsub213ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsub213ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub231ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfmsub231ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsub231ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfmsub231ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsub231ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsub231ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsub231ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsub231ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub231ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsub231ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsub231ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsub231ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd132ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfmsubadd132ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsubadd132ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfmsubadd132ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsubadd132ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsubadd132ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsubadd132ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsubadd132ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd132ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsubadd132ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsubadd132ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsubadd132ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd213ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfmsubadd213ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsubadd213ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfmsubadd213ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsubadd213ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsubadd213ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsubadd213ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsubadd213ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd213ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsubadd213ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsubadd213ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsubadd213ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd231ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfmsubadd231ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsubadd231ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfmsubadd231ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmsubadd231ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsubadd231ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsubadd231ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsubadd231ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd231ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmsubadd231ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmsubadd231ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmsubadd231ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmulcph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfmulcph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmulcph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfmulcph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfmulcph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmulcph	ymm6, ymm5, DWORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmulcph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmulcph	ymm6{k7}{z}, ymm5, DWORD BCST [edx-512]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmulcph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfmulcph	xmm6, xmm5, DWORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfmulcph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfmulcph	xmm6{k7}{z}, xmm5, DWORD BCST [edx-512]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd132ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfnmadd132ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmadd132ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfnmadd132ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmadd132ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmadd132ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmadd132ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmadd132ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd132ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmadd132ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmadd132ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmadd132ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd213ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfnmadd213ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmadd213ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfnmadd213ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmadd213ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmadd213ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmadd213ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmadd213ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd213ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmadd213ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmadd213ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmadd213ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd231ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfnmadd231ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmadd231ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfnmadd231ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmadd231ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmadd231ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmadd231ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmadd231ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd231ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmadd231ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmadd231ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmadd231ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub132ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfnmsub132ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmsub132ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfnmsub132ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmsub132ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmsub132ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmsub132ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmsub132ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub132ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmsub132ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmsub132ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmsub132ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub213ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfnmsub213ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmsub213ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfnmsub213ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmsub213ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmsub213ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmsub213ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmsub213ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub213ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmsub213ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmsub213ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmsub213ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub231ph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vfnmsub231ph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmsub231ph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vfnmsub231ph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vfnmsub231ph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmsub231ph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmsub231ph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmsub231ph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub231ph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfnmsub231ph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfnmsub231ph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfnmsub231ph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfpclassph	k5, xmm6, 123	 #AVX512-FP16,AVX512VL
	vfpclassph	k5{k7}, xmm6, 123	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfpclassph	k5, ymm6, 123	 #AVX512-FP16,AVX512VL
	vfpclassph	k5{k7}, ymm6, 123	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfpclassph	k5{k7}, XMMWORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vfpclassph	k5, WORD BCST [ecx]{1to8}, 123	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfpclassph	k5, XMMWORD PTR [ecx+2032], 123	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfpclassph	k5{k7}, WORD BCST [edx-256]{1to8}, 123	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING
	vfpclassph	k5, WORD BCST [ecx]{1to16}, 123	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vfpclassph	k5, YMMWORD PTR [ecx+4064], 123	 #AVX512-FP16,AVX512VL Disp8(7f)
	vfpclassph	k5{k7}, WORD BCST [edx-256]{1to16}, 123	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING
	vgetexpph	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vgetexpph	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vgetexpph	ymm6, ymm5	 #AVX512-FP16,AVX512VL
	vgetexpph	ymm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vgetexpph	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vgetexpph	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vgetexpph	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vgetexpph	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vgetexpph	ymm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vgetexpph	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vgetexpph	ymm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vgetexpph	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vgetmantph	ymm6, ymm5, 123	 #AVX512-FP16,AVX512VL
	vgetmantph	ymm6{k7}{z}, ymm5, 123	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vgetmantph	xmm6, xmm5, 123	 #AVX512-FP16,AVX512VL
	vgetmantph	xmm6{k7}{z}, xmm5, 123	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vgetmantph	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vgetmantph	xmm6, WORD BCST [ecx], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vgetmantph	xmm6, XMMWORD PTR [ecx+2032], 123	 #AVX512-FP16,AVX512VL Disp8(7f)
	vgetmantph	xmm6{k7}{z}, WORD BCST [edx-256], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vgetmantph	ymm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vgetmantph	ymm6, WORD BCST [ecx], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vgetmantph	ymm6, YMMWORD PTR [ecx+4064], 123	 #AVX512-FP16,AVX512VL Disp8(7f)
	vgetmantph	ymm6{k7}{z}, WORD BCST [edx-256], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmaxph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vmaxph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vmaxph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vmaxph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vmaxph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vmaxph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vmaxph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vmaxph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmaxph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vmaxph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vmaxph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vmaxph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vminph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vminph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vminph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vminph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vminph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vminph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vminph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vminph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vminph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vminph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vminph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vminph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmulph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vmulph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vmulph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vmulph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vmulph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vmulph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vmulph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vmulph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmulph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vmulph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vmulph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vmulph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrcpph	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vrcpph	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vrcpph	ymm6, ymm5	 #AVX512-FP16,AVX512VL
	vrcpph	ymm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vrcpph	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vrcpph	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vrcpph	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vrcpph	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrcpph	ymm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vrcpph	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vrcpph	ymm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vrcpph	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vreduceph	ymm6, ymm5, 123	 #AVX512-FP16,AVX512VL
	vreduceph	ymm6{k7}{z}, ymm5, 123	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vreduceph	xmm6, xmm5, 123	 #AVX512-FP16,AVX512VL
	vreduceph	xmm6{k7}{z}, xmm5, 123	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vreduceph	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vreduceph	xmm6, WORD BCST [ecx], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vreduceph	xmm6, XMMWORD PTR [ecx+2032], 123	 #AVX512-FP16,AVX512VL Disp8(7f)
	vreduceph	xmm6{k7}{z}, WORD BCST [edx-256], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vreduceph	ymm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vreduceph	ymm6, WORD BCST [ecx], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vreduceph	ymm6, YMMWORD PTR [ecx+4064], 123	 #AVX512-FP16,AVX512VL Disp8(7f)
	vreduceph	ymm6{k7}{z}, WORD BCST [edx-256], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrndscaleph	ymm6, ymm5, 123	 #AVX512-FP16,AVX512VL
	vrndscaleph	ymm6{k7}{z}, ymm5, 123	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vrndscaleph	xmm6, xmm5, 123	 #AVX512-FP16,AVX512VL
	vrndscaleph	xmm6{k7}{z}, xmm5, 123	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vrndscaleph	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vrndscaleph	xmm6, WORD BCST [ecx], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vrndscaleph	xmm6, XMMWORD PTR [ecx+2032], 123	 #AVX512-FP16,AVX512VL Disp8(7f)
	vrndscaleph	xmm6{k7}{z}, WORD BCST [edx-256], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrndscaleph	ymm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vrndscaleph	ymm6, WORD BCST [ecx], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vrndscaleph	ymm6, YMMWORD PTR [ecx+4064], 123	 #AVX512-FP16,AVX512VL Disp8(7f)
	vrndscaleph	ymm6{k7}{z}, WORD BCST [edx-256], 123	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrsqrtph	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vrsqrtph	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vrsqrtph	ymm6, ymm5	 #AVX512-FP16,AVX512VL
	vrsqrtph	ymm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vrsqrtph	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vrsqrtph	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vrsqrtph	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vrsqrtph	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrsqrtph	ymm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vrsqrtph	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vrsqrtph	ymm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vrsqrtph	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vscalefph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vscalefph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vscalefph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vscalefph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vscalefph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vscalefph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vscalefph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vscalefph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vscalefph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vscalefph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vscalefph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vscalefph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsqrtph	xmm6, xmm5	 #AVX512-FP16,AVX512VL
	vsqrtph	xmm6{k7}{z}, xmm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vsqrtph	ymm6, ymm5	 #AVX512-FP16,AVX512VL
	vsqrtph	ymm6{k7}{z}, ymm5	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vsqrtph	xmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vsqrtph	xmm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vsqrtph	xmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vsqrtph	xmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsqrtph	ymm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vsqrtph	ymm6, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vsqrtph	ymm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vsqrtph	ymm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsubph	ymm6, ymm5, ymm4	 #AVX512-FP16,AVX512VL
	vsubph	ymm6{k7}{z}, ymm5, ymm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vsubph	xmm6, xmm5, xmm4	 #AVX512-FP16,AVX512VL
	vsubph	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16,AVX512VL MASK_ENABLING ZEROCTL
	vsubph	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vsubph	ymm6, ymm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vsubph	ymm6, ymm5, YMMWORD PTR [ecx+4064]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vsubph	ymm6{k7}{z}, ymm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsubph	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16,AVX512VL MASK_ENABLING
	vsubph	xmm6, xmm5, WORD BCST [ecx]	 #AVX512-FP16,AVX512VL BROADCAST_EN
	vsubph	xmm6, xmm5, XMMWORD PTR [ecx+2032]	 #AVX512-FP16,AVX512VL Disp8(7f)
	vsubph	xmm6{k7}{z}, xmm5, WORD BCST [edx-256]	 #AVX512-FP16,AVX512VL BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
