# Check 32bit AVX512-FP16 instructions

	.allow_index_reg
	.text
_start:
	vaddph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vaddph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vaddph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vaddph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vaddph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vaddph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vaddph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vaddsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vaddsh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vaddsh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vaddsh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vaddsh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vaddsh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vaddsh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcmpph	$123, %zmm4, %zmm5, %k5	 #AVX512-FP16
	vcmpph	$123, {sae}, %zmm4, %zmm5, %k5	 #AVX512-FP16 HAS_SAE
	vcmpph	$123, {sae}, %zmm4, %zmm5, %k5{%k7}	 #AVX512-FP16 MASK_ENABLING HAS_SAE
	vcmpph	$123, 0x10000000(%esp, %esi, 8), %zmm5, %k5{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcmpph	$123, (%ecx){1to32}, %zmm5, %k5	 #AVX512-FP16 BROADCAST_EN
	vcmpph	$123, 8128(%ecx), %zmm5, %k5	 #AVX512-FP16 Disp8(7f)
	vcmpph	$123, -256(%edx){1to32}, %zmm5, %k5{%k7}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING
	vcmpsh	$123, %xmm4, %xmm5, %k5	 #AVX512-FP16
	vcmpsh	$123, {sae}, %xmm4, %xmm5, %k5	 #AVX512-FP16 HAS_SAE
	vcmpsh	$123, {sae}, %xmm4, %xmm5, %k5{%k7}	 #AVX512-FP16 MASK_ENABLING HAS_SAE
	vcmpsh	$123, 0x10000000(%esp, %esi, 8), %xmm5, %k5{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcmpsh	$123, (%ecx), %xmm5, %k5	 #AVX512-FP16
	vcmpsh	$123, 254(%ecx), %xmm5, %k5	 #AVX512-FP16 Disp8(7f)
	vcmpsh	$123, -256(%edx), %xmm5, %k5{%k7}	 #AVX512-FP16 Disp8(80) MASK_ENABLING
	vcomish	%xmm5, %xmm6	 #AVX512-FP16
	vcomish	{sae}, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE
	vcomish	0x10000000(%esp, %esi, 8), %xmm6	 #AVX512-FP16
	vcomish	(%ecx), %xmm6	 #AVX512-FP16
	vcomish	254(%ecx), %xmm6	 #AVX512-FP16 Disp8(7f)
	vcomish	-256(%edx), %xmm6	 #AVX512-FP16 Disp8(80)
	vcvtdq2ph	%zmm5, %ymm6	 #AVX512-FP16
	vcvtdq2ph	{rn-sae}, %zmm5, %ymm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtdq2ph	{rn-sae}, %zmm5, %ymm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtdq2ph	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtdq2ph	(%ecx){1to16}, %ymm6	 #AVX512-FP16 BROADCAST_EN
	vcvtdq2ph	8128(%ecx), %ymm6	 #AVX512-FP16 Disp8(7f)
	vcvtdq2ph	-512(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtpd2ph	%zmm5, %xmm6	 #AVX512-FP16
	vcvtpd2ph	{rn-sae}, %zmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtpd2ph	{rn-sae}, %zmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtpd2phz	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtpd2ph	(%ecx){1to8}, %xmm6	 #AVX512-FP16 BROADCAST_EN
	vcvtpd2phz	8128(%ecx), %xmm6	 #AVX512-FP16 Disp8(7f)
	vcvtpd2ph	-1024(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2dq	%ymm5, %zmm6	 #AVX512-FP16
	vcvtph2dq	{rn-sae}, %ymm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2dq	{rn-sae}, %ymm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2dq	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2dq	(%ecx){1to16}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvtph2dq	4064(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvtph2dq	-256(%edx){1to16}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2pd	%xmm5, %zmm6	 #AVX512-FP16
	vcvtph2pd	{sae}, %xmm5, %zmm6	 #AVX512-FP16 HAS_SAE
	vcvtph2pd	{sae}, %xmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtph2pd	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2pd	(%ecx){1to8}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvtph2pd	2032(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvtph2pd	-256(%edx){1to8}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2psx	%ymm5, %zmm6	 #AVX512-FP16
	vcvtph2psx	{sae}, %ymm5, %zmm6	 #AVX512-FP16 HAS_SAE
	vcvtph2psx	{sae}, %ymm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtph2psx	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2psx	(%ecx){1to16}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvtph2psx	4064(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvtph2psx	-256(%edx){1to16}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2qq	%xmm5, %zmm6	 #AVX512-FP16
	vcvtph2qq	{rn-sae}, %xmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2qq	{rn-sae}, %xmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2qq	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2qq	(%ecx){1to8}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvtph2qq	2032(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvtph2qq	-256(%edx){1to8}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2udq	%ymm5, %zmm6	 #AVX512-FP16
	vcvtph2udq	{rn-sae}, %ymm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2udq	{rn-sae}, %ymm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2udq	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2udq	(%ecx){1to16}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvtph2udq	4064(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvtph2udq	-256(%edx){1to16}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uqq	%xmm5, %zmm6	 #AVX512-FP16
	vcvtph2uqq	{rn-sae}, %xmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2uqq	{rn-sae}, %xmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2uqq	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2uqq	(%ecx){1to8}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvtph2uqq	2032(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvtph2uqq	-256(%edx){1to8}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uw	%zmm5, %zmm6	 #AVX512-FP16
	vcvtph2uw	{rn-sae}, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2uw	{rn-sae}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2uw	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2uw	(%ecx){1to32}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvtph2uw	8128(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvtph2uw	-256(%edx){1to32}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2w	%zmm5, %zmm6	 #AVX512-FP16
	vcvtph2w	{rn-sae}, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2w	{rn-sae}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2w	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2w	(%ecx){1to32}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvtph2w	8128(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvtph2w	-256(%edx){1to32}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtps2phx	%zmm5, %ymm6	 #AVX512-FP16
	vcvtps2phx	{rn-sae}, %zmm5, %ymm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtps2phx	{rn-sae}, %zmm5, %ymm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtps2phx	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtps2phx	(%ecx){1to16}, %ymm6	 #AVX512-FP16 BROADCAST_EN
	vcvtps2phx	8128(%ecx), %ymm6	 #AVX512-FP16 Disp8(7f)
	vcvtps2phx	-512(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtqq2ph	%zmm5, %xmm6	 #AVX512-FP16
	vcvtqq2ph	{rn-sae}, %zmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtqq2ph	{rn-sae}, %zmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtqq2phz	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtqq2ph	(%ecx){1to8}, %xmm6	 #AVX512-FP16 BROADCAST_EN
	vcvtqq2phz	8128(%ecx), %xmm6	 #AVX512-FP16 Disp8(7f)
	vcvtqq2ph	-1024(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsd2sh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vcvtsd2sh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsd2sh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtsd2sh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtsd2sh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vcvtsd2sh	1016(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vcvtsd2sh	-1024(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsh2sd	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vcvtsh2sd	{sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE
	vcvtsh2sd	{sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtsh2sd	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtsh2sd	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vcvtsh2sd	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vcvtsh2sd	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsh2si	%xmm6, %edx	 #AVX512-FP16
	vcvtsh2si	{rn-sae}, %xmm6, %edx	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsh2si	0x10000000(%esp, %esi, 8), %edx	 #AVX512-FP16
	vcvtsh2si	(%ecx), %edx	 #AVX512-FP16
	vcvtsh2si	254(%ecx), %edx	 #AVX512-FP16 Disp8(7f)
	vcvtsh2si	-256(%edx), %edx	 #AVX512-FP16 Disp8(80)
	vcvtsh2ss	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vcvtsh2ss	{sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE
	vcvtsh2ss	{sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtsh2ss	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtsh2ss	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vcvtsh2ss	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vcvtsh2ss	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsh2usi	%xmm6, %edx	 #AVX512-FP16
	vcvtsh2usi	{rn-sae}, %xmm6, %edx	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsh2usi	0x10000000(%esp, %esi, 8), %edx	 #AVX512-FP16
	vcvtsh2usi	(%ecx), %edx	 #AVX512-FP16
	vcvtsh2usi	254(%ecx), %edx	 #AVX512-FP16 Disp8(7f)
	vcvtsh2usi	-256(%edx), %edx	 #AVX512-FP16 Disp8(80)
	vcvtsi2sh	%edx, %xmm5, %xmm6	 #AVX512-FP16
	vcvtsi2sh	%edx, {rn-sae}, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsi2shl	0x10000000(%esp, %esi, 8), %xmm5, %xmm6	 #AVX512-FP16
	vcvtsi2shl	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vcvtsi2shl	508(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vcvtsi2shl	-512(%edx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(80)
	vcvtss2sh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vcvtss2sh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtss2sh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtss2sh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtss2sh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vcvtss2sh	508(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vcvtss2sh	-512(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2dq	%ymm5, %zmm6	 #AVX512-FP16
	vcvttph2dq	{sae}, %ymm5, %zmm6	 #AVX512-FP16 HAS_SAE
	vcvttph2dq	{sae}, %ymm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2dq	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvttph2dq	(%ecx){1to16}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvttph2dq	4064(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvttph2dq	-256(%edx){1to16}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2qq	%xmm5, %zmm6	 #AVX512-FP16
	vcvttph2qq	{sae}, %xmm5, %zmm6	 #AVX512-FP16 HAS_SAE
	vcvttph2qq	{sae}, %xmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2qq	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvttph2qq	(%ecx){1to8}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvttph2qq	2032(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvttph2qq	-256(%edx){1to8}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2udq	%ymm5, %zmm6	 #AVX512-FP16
	vcvttph2udq	{sae}, %ymm5, %zmm6	 #AVX512-FP16 HAS_SAE
	vcvttph2udq	{sae}, %ymm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2udq	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvttph2udq	(%ecx){1to16}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvttph2udq	4064(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvttph2udq	-256(%edx){1to16}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uqq	%xmm5, %zmm6	 #AVX512-FP16
	vcvttph2uqq	{sae}, %xmm5, %zmm6	 #AVX512-FP16 HAS_SAE
	vcvttph2uqq	{sae}, %xmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2uqq	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvttph2uqq	(%ecx){1to8}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvttph2uqq	2032(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvttph2uqq	-256(%edx){1to8}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uw	%zmm5, %zmm6	 #AVX512-FP16
	vcvttph2uw	{sae}, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE
	vcvttph2uw	{sae}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2uw	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvttph2uw	(%ecx){1to32}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvttph2uw	8128(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvttph2uw	-256(%edx){1to32}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2w	%zmm5, %zmm6	 #AVX512-FP16
	vcvttph2w	{sae}, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE
	vcvttph2w	{sae}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2w	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvttph2w	(%ecx){1to32}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvttph2w	8128(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvttph2w	-256(%edx){1to32}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttsh2si	%xmm6, %edx	 #AVX512-FP16
	vcvttsh2si	{sae}, %xmm6, %edx	 #AVX512-FP16 HAS_SAE
	vcvttsh2si	0x10000000(%esp, %esi, 8), %edx	 #AVX512-FP16
	vcvttsh2si	(%ecx), %edx	 #AVX512-FP16
	vcvttsh2si	254(%ecx), %edx	 #AVX512-FP16 Disp8(7f)
	vcvttsh2si	-256(%edx), %edx	 #AVX512-FP16 Disp8(80)
	vcvttsh2usi	%xmm6, %edx	 #AVX512-FP16
	vcvttsh2usi	{sae}, %xmm6, %edx	 #AVX512-FP16 HAS_SAE
	vcvttsh2usi	0x10000000(%esp, %esi, 8), %edx	 #AVX512-FP16
	vcvttsh2usi	(%ecx), %edx	 #AVX512-FP16
	vcvttsh2usi	254(%ecx), %edx	 #AVX512-FP16 Disp8(7f)
	vcvttsh2usi	-256(%edx), %edx	 #AVX512-FP16 Disp8(80)
	vcvtudq2ph	%zmm5, %ymm6	 #AVX512-FP16
	vcvtudq2ph	{rn-sae}, %zmm5, %ymm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtudq2ph	{rn-sae}, %zmm5, %ymm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtudq2ph	0x10000000(%esp, %esi, 8), %ymm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtudq2ph	(%ecx){1to16}, %ymm6	 #AVX512-FP16 BROADCAST_EN
	vcvtudq2ph	8128(%ecx), %ymm6	 #AVX512-FP16 Disp8(7f)
	vcvtudq2ph	-512(%edx){1to16}, %ymm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtuqq2ph	%zmm5, %xmm6	 #AVX512-FP16
	vcvtuqq2ph	{rn-sae}, %zmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtuqq2ph	{rn-sae}, %zmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtuqq2phz	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtuqq2ph	(%ecx){1to8}, %xmm6	 #AVX512-FP16 BROADCAST_EN
	vcvtuqq2phz	8128(%ecx), %xmm6	 #AVX512-FP16 Disp8(7f)
	vcvtuqq2ph	-1024(%edx){1to8}, %xmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtusi2sh	%edx, %xmm5, %xmm6	 #AVX512-FP16
	vcvtusi2sh	%edx, {rn-sae}, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtusi2shl	0x10000000(%esp, %esi, 8), %xmm5, %xmm6	 #AVX512-FP16
	vcvtusi2shl	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vcvtusi2shl	508(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vcvtusi2shl	-512(%edx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(80)
	vcvtuw2ph	%zmm5, %zmm6	 #AVX512-FP16
	vcvtuw2ph	{rn-sae}, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtuw2ph	{rn-sae}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtuw2ph	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtuw2ph	(%ecx){1to32}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvtuw2ph	8128(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvtuw2ph	-256(%edx){1to32}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtw2ph	%zmm5, %zmm6	 #AVX512-FP16
	vcvtw2ph	{rn-sae}, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtw2ph	{rn-sae}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtw2ph	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtw2ph	(%ecx){1to32}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vcvtw2ph	8128(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vcvtw2ph	-256(%edx){1to32}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vdivph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vdivph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vdivph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vdivph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vdivph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vdivph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vdivph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vdivsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vdivsh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vdivsh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vdivsh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vdivsh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vdivsh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vdivsh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfcmaddcph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfcmaddcph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmaddcph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmaddcph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfcmaddcph	(%ecx){1to16}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfcmaddcph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfcmaddcph	-512(%edx){1to16}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmaddcsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfcmaddcsh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmaddcsh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmaddcsh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfcmaddcsh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfcmaddcsh	508(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfcmaddcsh	-512(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfcmulcph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfcmulcph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmulcph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmulcph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfcmulcph	(%ecx){1to16}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfcmulcph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfcmulcph	-512(%edx){1to16}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmulcsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfcmulcsh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmulcsh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmulcsh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfcmulcsh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfcmulcsh	508(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfcmulcsh	-512(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd132ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfmadd132ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd132ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd132ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmadd132ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfmadd132ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfmadd132ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd132sh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfmadd132sh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd132sh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd132sh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmadd132sh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfmadd132sh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfmadd132sh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd213ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfmadd213ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd213ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd213ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmadd213ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfmadd213ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfmadd213ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd213sh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfmadd213sh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd213sh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd213sh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmadd213sh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfmadd213sh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfmadd213sh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd231ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfmadd231ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd231ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd231ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmadd231ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfmadd231ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfmadd231ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd231sh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfmadd231sh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd231sh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd231sh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmadd231sh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfmadd231sh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfmadd231sh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddcph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfmaddcph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddcph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddcph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmaddcph	(%ecx){1to16}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfmaddcph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfmaddcph	-512(%edx){1to16}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddcsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfmaddcsh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddcsh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddcsh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmaddcsh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfmaddcsh	508(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfmaddcsh	-512(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub132ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfmaddsub132ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddsub132ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddsub132ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmaddsub132ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfmaddsub132ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfmaddsub132ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub213ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfmaddsub213ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddsub213ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddsub213ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmaddsub213ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfmaddsub213ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfmaddsub213ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub231ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfmaddsub231ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddsub231ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddsub231ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmaddsub231ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfmaddsub231ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfmaddsub231ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub132ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfmsub132ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub132ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub132ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsub132ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfmsub132ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfmsub132ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub132sh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfmsub132sh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub132sh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub132sh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsub132sh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfmsub132sh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfmsub132sh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub213ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfmsub213ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub213ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub213ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsub213ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfmsub213ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfmsub213ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub213sh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfmsub213sh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub213sh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub213sh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsub213sh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfmsub213sh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfmsub213sh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub231ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfmsub231ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub231ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub231ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsub231ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfmsub231ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfmsub231ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub231sh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfmsub231sh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub231sh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub231sh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsub231sh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfmsub231sh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfmsub231sh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd132ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfmsubadd132ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsubadd132ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsubadd132ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsubadd132ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfmsubadd132ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfmsubadd132ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd213ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfmsubadd213ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsubadd213ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsubadd213ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsubadd213ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfmsubadd213ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfmsubadd213ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd231ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfmsubadd231ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsubadd231ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsubadd231ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsubadd231ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfmsubadd231ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfmsubadd231ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmulcph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfmulcph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmulcph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmulcph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmulcph	(%ecx){1to16}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfmulcph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfmulcph	-512(%edx){1to16}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmulcsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfmulcsh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmulcsh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmulcsh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmulcsh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfmulcsh	508(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfmulcsh	-512(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd132ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfnmadd132ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd132ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd132ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmadd132ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfnmadd132ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfnmadd132ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd132sh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfnmadd132sh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd132sh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd132sh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmadd132sh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfnmadd132sh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfnmadd132sh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd213ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfnmadd213ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd213ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd213ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmadd213ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfnmadd213ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfnmadd213ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd213sh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfnmadd213sh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd213sh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd213sh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmadd213sh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfnmadd213sh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfnmadd213sh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd231ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfnmadd231ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd231ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd231ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmadd231ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfnmadd231ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfnmadd231ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd231sh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfnmadd231sh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd231sh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd231sh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmadd231sh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfnmadd231sh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfnmadd231sh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub132ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfnmsub132ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub132ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub132ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmsub132ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfnmsub132ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfnmsub132ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub132sh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfnmsub132sh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub132sh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub132sh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmsub132sh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfnmsub132sh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfnmsub132sh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub213ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfnmsub213ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub213ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub213ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmsub213ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfnmsub213ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfnmsub213ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub213sh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfnmsub213sh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub213sh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub213sh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmsub213sh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfnmsub213sh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfnmsub213sh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub231ph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vfnmsub231ph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub231ph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub231ph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmsub231ph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vfnmsub231ph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vfnmsub231ph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub231sh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vfnmsub231sh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub231sh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub231sh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmsub231sh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vfnmsub231sh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vfnmsub231sh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfpclassph	$123, %zmm6, %k5	 #AVX512-FP16
	vfpclassph	$123, %zmm6, %k5{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfpclassphz	$123, 0x10000000(%esp, %esi, 8), %k5{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfpclassph	$123, (%ecx){1to32}, %k5	 #AVX512-FP16 BROADCAST_EN
	vfpclassphz	$123, 8128(%ecx), %k5	 #AVX512-FP16 Disp8(7f)
	vfpclassph	$123, -256(%edx){1to32}, %k5{%k7}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING
	vfpclassphz	$123, 2(%ecx){1to32}, %k5	 #AVX512-FP16 BROADCAST_EN
	vfpclasssh	$123, %xmm6, %k5	 #AVX512-FP16
	vfpclasssh	$123, %xmm6, %k5{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfpclasssh	$123, 0x10000000(%esp, %esi, 8), %k5{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfpclasssh	$123, (%ecx), %k5	 #AVX512-FP16
	vfpclasssh	$123, 254(%ecx), %k5	 #AVX512-FP16 Disp8(7f)
	vfpclasssh	$123, -256(%edx), %k5{%k7}	 #AVX512-FP16 Disp8(80) MASK_ENABLING
	vgetexpph	%zmm5, %zmm6	 #AVX512-FP16
	vgetexpph	{sae}, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE
	vgetexpph	{sae}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetexpph	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vgetexpph	(%ecx){1to32}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vgetexpph	8128(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vgetexpph	-256(%edx){1to32}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vgetexpsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vgetexpsh	{sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE
	vgetexpsh	{sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetexpsh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vgetexpsh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vgetexpsh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vgetexpsh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vgetmantph	$123, %zmm5, %zmm6	 #AVX512-FP16
	vgetmantph	$123, {sae}, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE
	vgetmantph	$123, {sae}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetmantph	$123, 0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vgetmantph	$123, (%ecx){1to32}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vgetmantph	$123, 8128(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vgetmantph	$123, -256(%edx){1to32}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vgetmantsh	$123, %xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vgetmantsh	$123, {sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE
	vgetmantsh	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetmantsh	$123, 0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vgetmantsh	$123, (%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vgetmantsh	$123, 254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vgetmantsh	$123, -256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vmaxph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vmaxph	{sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE
	vmaxph	{sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vmaxph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vmaxph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vmaxph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vmaxph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmaxsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vmaxsh	{sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE
	vmaxsh	{sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vmaxsh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vmaxsh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vmaxsh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vmaxsh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vminph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vminph	{sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE
	vminph	{sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vminph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vminph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vminph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vminph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vminsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vminsh	{sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE
	vminsh	{sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vminsh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vminsh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vminsh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vminsh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vmovsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vmovsh	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vmovsh	0x10000000(%esp, %esi, 8), %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vmovsh	(%ecx), %xmm6	 #AVX512-FP16
	vmovsh	254(%ecx), %xmm6	 #AVX512-FP16 Disp8(7f)
	vmovsh	-256(%edx), %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vmovsh	%xmm6, 0x10000000(%esp, %esi, 8){%k7}	 #AVX512-FP16 MASK_ENABLING
	vmovsh	%xmm6, (%ecx)	 #AVX512-FP16
	vmovsh	%xmm6, 254(%ecx)	 #AVX512-FP16 Disp8(7f)
	vmovsh	%xmm6, -256(%edx){%k7}	 #AVX512-FP16 Disp8(80) MASK_ENABLING
	vmovw	%edx, %xmm6	 #AVX512-FP16
	vmovw	%xmm6, %edx	 #AVX512-FP16
	vmovw	0x10000000(%esp, %esi, 8), %xmm6	 #AVX512-FP16
	vmovw	(%ecx), %xmm6	 #AVX512-FP16
	vmovw	254(%ecx), %xmm6	 #AVX512-FP16 Disp8(7f)
	vmovw	-256(%edx), %xmm6	 #AVX512-FP16 Disp8(80)
	vmovw	%xmm6, 0x10000000(%esp, %esi, 8)	 #AVX512-FP16
	vmovw	%xmm6, (%ecx)	 #AVX512-FP16
	vmovw	%xmm6, 254(%ecx)	 #AVX512-FP16 Disp8(7f)
	vmovw	%xmm6, -256(%edx)	 #AVX512-FP16 Disp8(80)
	vmulph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vmulph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vmulph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vmulph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vmulph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vmulph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vmulph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmulsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vmulsh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vmulsh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vmulsh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vmulsh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vmulsh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vmulsh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vrcpph	%zmm5, %zmm6	 #AVX512-FP16
	vrcpph	%zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrcpph	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vrcpph	(%ecx){1to32}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vrcpph	8128(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vrcpph	-256(%edx){1to32}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrcpsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vrcpsh	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrcpsh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vrcpsh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vrcpsh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vrcpsh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vreduceph	$123, %zmm5, %zmm6	 #AVX512-FP16
	vreduceph	$123, {sae}, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE
	vreduceph	$123, {sae}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vreduceph	$123, 0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vreduceph	$123, (%ecx){1to32}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vreduceph	$123, 8128(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vreduceph	$123, -256(%edx){1to32}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vreducesh	$123, %xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vreducesh	$123, {sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE
	vreducesh	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vreducesh	$123, 0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vreducesh	$123, (%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vreducesh	$123, 254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vreducesh	$123, -256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vrndscaleph	$123, %zmm5, %zmm6	 #AVX512-FP16
	vrndscaleph	$123, {sae}, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE
	vrndscaleph	$123, {sae}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vrndscaleph	$123, 0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vrndscaleph	$123, (%ecx){1to32}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vrndscaleph	$123, 8128(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vrndscaleph	$123, -256(%edx){1to32}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrndscalesh	$123, %xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vrndscalesh	$123, {sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE
	vrndscalesh	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vrndscalesh	$123, 0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vrndscalesh	$123, (%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vrndscalesh	$123, 254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vrndscalesh	$123, -256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vrsqrtph	%zmm5, %zmm6	 #AVX512-FP16
	vrsqrtph	%zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrsqrtph	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vrsqrtph	(%ecx){1to32}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vrsqrtph	8128(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vrsqrtph	-256(%edx){1to32}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrsqrtsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vrsqrtsh	%xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrsqrtsh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vrsqrtsh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vrsqrtsh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vrsqrtsh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vscalefph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vscalefph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vscalefph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vscalefph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vscalefph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vscalefph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vscalefph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vscalefsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vscalefsh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vscalefsh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vscalefsh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vscalefsh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vscalefsh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vscalefsh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vsqrtph	%zmm5, %zmm6	 #AVX512-FP16
	vsqrtph	{rn-sae}, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsqrtph	{rn-sae}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsqrtph	0x10000000(%esp, %esi, 8), %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vsqrtph	(%ecx){1to32}, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vsqrtph	8128(%ecx), %zmm6	 #AVX512-FP16 Disp8(7f)
	vsqrtph	-256(%edx){1to32}, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsqrtsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vsqrtsh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsqrtsh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsqrtsh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vsqrtsh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vsqrtsh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vsqrtsh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vsubph	%zmm4, %zmm5, %zmm6	 #AVX512-FP16
	vsubph	{rn-sae}, %zmm4, %zmm5, %zmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsubph	{rn-sae}, %zmm4, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsubph	0x10000000(%esp, %esi, 8), %zmm5, %zmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vsubph	(%ecx){1to32}, %zmm5, %zmm6	 #AVX512-FP16 BROADCAST_EN
	vsubph	8128(%ecx), %zmm5, %zmm6	 #AVX512-FP16 Disp8(7f)
	vsubph	-256(%edx){1to32}, %zmm5, %zmm6{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsubsh	%xmm4, %xmm5, %xmm6	 #AVX512-FP16
	vsubsh	{rn-sae}, %xmm4, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsubsh	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsubsh	0x10000000(%esp, %esi, 8), %xmm5, %xmm6{%k7}	 #AVX512-FP16 MASK_ENABLING
	vsubsh	(%ecx), %xmm5, %xmm6	 #AVX512-FP16
	vsubsh	254(%ecx), %xmm5, %xmm6	 #AVX512-FP16 Disp8(7f)
	vsubsh	-256(%edx), %xmm5, %xmm6{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vucomish	%xmm5, %xmm6	 #AVX512-FP16
	vucomish	{sae}, %xmm5, %xmm6	 #AVX512-FP16 HAS_SAE
	vucomish	0x10000000(%esp, %esi, 8), %xmm6	 #AVX512-FP16
	vucomish	(%ecx), %xmm6	 #AVX512-FP16
	vucomish	254(%ecx), %xmm6	 #AVX512-FP16 Disp8(7f)
	vucomish	-256(%edx), %xmm6	 #AVX512-FP16 Disp8(80)

.intel_syntax noprefix
	vaddph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vaddph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vaddph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vaddph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vaddph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vaddph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vaddph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vaddsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vaddsh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vaddsh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vaddsh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vaddsh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vaddsh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vaddsh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcmpph	k5, zmm5, zmm4, 123	 #AVX512-FP16
	vcmpph	k5, zmm5, zmm4{sae}, 123	 #AVX512-FP16 HAS_SAE
	vcmpph	k5{k7}, zmm5, zmm4{sae}, 123	 #AVX512-FP16 MASK_ENABLING HAS_SAE
	vcmpph	k5{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vcmpph	k5, zmm5, WORD BCST [ecx], 123	 #AVX512-FP16 BROADCAST_EN
	vcmpph	k5, zmm5, ZMMWORD PTR [ecx+8128], 123	 #AVX512-FP16 Disp8(7f)
	vcmpph	k5{k7}, zmm5, WORD BCST [edx-256], 123	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING
	vcmpsh	k5, xmm5, xmm4, 123	 #AVX512-FP16
	vcmpsh	k5, xmm5, xmm4{sae}, 123	 #AVX512-FP16 HAS_SAE
	vcmpsh	k5{k7}, xmm5, xmm4{sae}, 123	 #AVX512-FP16 MASK_ENABLING HAS_SAE
	vcmpsh	k5{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vcmpsh	k5, xmm5, WORD PTR [ecx], 123	 #AVX512-FP16
	vcmpsh	k5, xmm5, WORD PTR [ecx+254], 123	 #AVX512-FP16 Disp8(7f)
	vcmpsh	k5{k7}, xmm5, WORD PTR [edx-256], 123	 #AVX512-FP16 Disp8(80) MASK_ENABLING
	vcomish	xmm6, xmm5	 #AVX512-FP16
	vcomish	xmm6, xmm5{sae}	 #AVX512-FP16 HAS_SAE
	vcomish	xmm6, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16
	vcomish	xmm6, WORD PTR [ecx]	 #AVX512-FP16
	vcomish	xmm6, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vcomish	xmm6, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80)
	vcvtdq2ph	ymm6, zmm5	 #AVX512-FP16
	vcvtdq2ph	ymm6, zmm5{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtdq2ph	ymm6{k7}{z}, zmm5{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtdq2ph	ymm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtdq2ph	ymm6, DWORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvtdq2ph	ymm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtdq2ph	ymm6{k7}{z}, DWORD BCST [edx-512]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtpd2ph	xmm6, zmm5	 #AVX512-FP16
	vcvtpd2ph	xmm6, zmm5{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtpd2ph	xmm6{k7}{z}, zmm5{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtpd2ph	xmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtpd2ph	xmm6, QWORD BCST [ecx]{1to8}	 #AVX512-FP16 BROADCAST_EN
	vcvtpd2ph	xmm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtpd2ph	xmm6{k7}{z}, QWORD BCST [edx-1024]{1to8}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2dq	zmm6, ymm5	 #AVX512-FP16
	vcvtph2dq	zmm6, ymm5{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2dq	zmm6{k7}{z}, ymm5{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2dq	zmm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2dq	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2dq	zmm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16 Disp8(7f)
	vcvtph2dq	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2pd	zmm6, xmm5	 #AVX512-FP16
	vcvtph2pd	zmm6, xmm5{sae}	 #AVX512-FP16 HAS_SAE
	vcvtph2pd	zmm6{k7}{z}, xmm5{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtph2pd	zmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2pd	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2pd	zmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16 Disp8(7f)
	vcvtph2pd	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2psx	zmm6, ymm5	 #AVX512-FP16
	vcvtph2psx	zmm6, ymm5{sae}	 #AVX512-FP16 HAS_SAE
	vcvtph2psx	zmm6{k7}{z}, ymm5{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtph2psx	zmm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2psx	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2psx	zmm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16 Disp8(7f)
	vcvtph2psx	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2qq	zmm6, xmm5	 #AVX512-FP16
	vcvtph2qq	zmm6, xmm5{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2qq	zmm6{k7}{z}, xmm5{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2qq	zmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2qq	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2qq	zmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16 Disp8(7f)
	vcvtph2qq	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2udq	zmm6, ymm5	 #AVX512-FP16
	vcvtph2udq	zmm6, ymm5{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2udq	zmm6{k7}{z}, ymm5{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2udq	zmm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2udq	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2udq	zmm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16 Disp8(7f)
	vcvtph2udq	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uqq	zmm6, xmm5	 #AVX512-FP16
	vcvtph2uqq	zmm6, xmm5{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2uqq	zmm6{k7}{z}, xmm5{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2uqq	zmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2uqq	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2uqq	zmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16 Disp8(7f)
	vcvtph2uqq	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uw	zmm6, zmm5	 #AVX512-FP16
	vcvtph2uw	zmm6, zmm5{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2uw	zmm6{k7}{z}, zmm5{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2uw	zmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2uw	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2uw	zmm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtph2uw	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2w	zmm6, zmm5	 #AVX512-FP16
	vcvtph2w	zmm6, zmm5{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2w	zmm6{k7}{z}, zmm5{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2w	zmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2w	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2w	zmm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtph2w	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtps2phx	ymm6, zmm5	 #AVX512-FP16
	vcvtps2phx	ymm6, zmm5{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtps2phx	ymm6{k7}{z}, zmm5{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtps2phx	ymm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtps2phx	ymm6, DWORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvtps2phx	ymm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtps2phx	ymm6{k7}{z}, DWORD BCST [edx-512]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtqq2ph	xmm6, zmm5	 #AVX512-FP16
	vcvtqq2ph	xmm6, zmm5{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtqq2ph	xmm6{k7}{z}, zmm5{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtqq2ph	xmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtqq2ph	xmm6, QWORD BCST [ecx]{1to8}	 #AVX512-FP16 BROADCAST_EN
	vcvtqq2ph	xmm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtqq2ph	xmm6{k7}{z}, QWORD BCST [edx-1024]{1to8}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsd2sh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vcvtsd2sh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsd2sh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtsd2sh	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtsd2sh	xmm6, xmm5, QWORD PTR [ecx]	 #AVX512-FP16
	vcvtsd2sh	xmm6, xmm5, QWORD PTR [ecx+1016]	 #AVX512-FP16 Disp8(7f)
	vcvtsd2sh	xmm6{k7}{z}, xmm5, QWORD PTR [edx-1024]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsh2sd	xmm6, xmm5, xmm4	 #AVX512-FP16
	vcvtsh2sd	xmm6, xmm5, xmm4{sae}	 #AVX512-FP16 HAS_SAE
	vcvtsh2sd	xmm6{k7}{z}, xmm5, xmm4{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtsh2sd	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtsh2sd	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vcvtsh2sd	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vcvtsh2sd	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsh2si	edx, xmm6	 #AVX512-FP16
	vcvtsh2si	edx, xmm6{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsh2si	edx, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16
	vcvtsh2si	edx, WORD PTR [ecx]	 #AVX512-FP16
	vcvtsh2si	edx, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vcvtsh2si	edx, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80)
	vcvtsh2ss	xmm6, xmm5, xmm4	 #AVX512-FP16
	vcvtsh2ss	xmm6, xmm5, xmm4{sae}	 #AVX512-FP16 HAS_SAE
	vcvtsh2ss	xmm6{k7}{z}, xmm5, xmm4{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtsh2ss	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtsh2ss	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vcvtsh2ss	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vcvtsh2ss	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsh2usi	edx, xmm6	 #AVX512-FP16
	vcvtsh2usi	edx, xmm6{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsh2usi	edx, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16
	vcvtsh2usi	edx, WORD PTR [ecx]	 #AVX512-FP16
	vcvtsh2usi	edx, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vcvtsh2usi	edx, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80)
	vcvtsi2sh	xmm6, xmm5, edx	 #AVX512-FP16
	vcvtsi2sh	xmm6, xmm5, edx{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsi2sh	xmm6, xmm5, DWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16
	vcvtsi2sh	xmm6, xmm5, DWORD PTR [ecx]	 #AVX512-FP16
	vcvtsi2sh	xmm6, xmm5, DWORD PTR [ecx+508]	 #AVX512-FP16 Disp8(7f)
	vcvtsi2sh	xmm6, xmm5, DWORD PTR [edx-512]	 #AVX512-FP16 Disp8(80)
	vcvtss2sh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vcvtss2sh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtss2sh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtss2sh	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtss2sh	xmm6, xmm5, DWORD PTR [ecx]	 #AVX512-FP16
	vcvtss2sh	xmm6, xmm5, DWORD PTR [ecx+508]	 #AVX512-FP16 Disp8(7f)
	vcvtss2sh	xmm6{k7}{z}, xmm5, DWORD PTR [edx-512]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2dq	zmm6, ymm5	 #AVX512-FP16
	vcvttph2dq	zmm6, ymm5{sae}	 #AVX512-FP16 HAS_SAE
	vcvttph2dq	zmm6{k7}{z}, ymm5{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2dq	zmm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvttph2dq	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvttph2dq	zmm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16 Disp8(7f)
	vcvttph2dq	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2qq	zmm6, xmm5	 #AVX512-FP16
	vcvttph2qq	zmm6, xmm5{sae}	 #AVX512-FP16 HAS_SAE
	vcvttph2qq	zmm6{k7}{z}, xmm5{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2qq	zmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvttph2qq	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvttph2qq	zmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16 Disp8(7f)
	vcvttph2qq	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2udq	zmm6, ymm5	 #AVX512-FP16
	vcvttph2udq	zmm6, ymm5{sae}	 #AVX512-FP16 HAS_SAE
	vcvttph2udq	zmm6{k7}{z}, ymm5{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2udq	zmm6{k7}, YMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvttph2udq	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvttph2udq	zmm6, YMMWORD PTR [ecx+4064]	 #AVX512-FP16 Disp8(7f)
	vcvttph2udq	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uqq	zmm6, xmm5	 #AVX512-FP16
	vcvttph2uqq	zmm6, xmm5{sae}	 #AVX512-FP16 HAS_SAE
	vcvttph2uqq	zmm6{k7}{z}, xmm5{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2uqq	zmm6{k7}, XMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvttph2uqq	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvttph2uqq	zmm6, XMMWORD PTR [ecx+2032]	 #AVX512-FP16 Disp8(7f)
	vcvttph2uqq	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uw	zmm6, zmm5	 #AVX512-FP16
	vcvttph2uw	zmm6, zmm5{sae}	 #AVX512-FP16 HAS_SAE
	vcvttph2uw	zmm6{k7}{z}, zmm5{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2uw	zmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvttph2uw	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvttph2uw	zmm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvttph2uw	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2w	zmm6, zmm5	 #AVX512-FP16
	vcvttph2w	zmm6, zmm5{sae}	 #AVX512-FP16 HAS_SAE
	vcvttph2w	zmm6{k7}{z}, zmm5{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2w	zmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvttph2w	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvttph2w	zmm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvttph2w	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttsh2si	edx, xmm6	 #AVX512-FP16
	vcvttsh2si	edx, xmm6{sae}	 #AVX512-FP16 HAS_SAE
	vcvttsh2si	edx, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16
	vcvttsh2si	edx, WORD PTR [ecx]	 #AVX512-FP16
	vcvttsh2si	edx, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vcvttsh2si	edx, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80)
	vcvttsh2usi	edx, xmm6	 #AVX512-FP16
	vcvttsh2usi	edx, xmm6{sae}	 #AVX512-FP16 HAS_SAE
	vcvttsh2usi	edx, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16
	vcvttsh2usi	edx, WORD PTR [ecx]	 #AVX512-FP16
	vcvttsh2usi	edx, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vcvttsh2usi	edx, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80)
	vcvtudq2ph	ymm6, zmm5	 #AVX512-FP16
	vcvtudq2ph	ymm6, zmm5{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtudq2ph	ymm6{k7}{z}, zmm5{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtudq2ph	ymm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtudq2ph	ymm6, DWORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvtudq2ph	ymm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtudq2ph	ymm6{k7}{z}, DWORD BCST [edx-512]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtuqq2ph	xmm6, zmm5	 #AVX512-FP16
	vcvtuqq2ph	xmm6, zmm5{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtuqq2ph	xmm6{k7}{z}, zmm5{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtuqq2ph	xmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtuqq2ph	xmm6, QWORD BCST [ecx]{1to8}	 #AVX512-FP16 BROADCAST_EN
	vcvtuqq2ph	xmm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtuqq2ph	xmm6{k7}{z}, QWORD BCST [edx-1024]{1to8}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtusi2sh	xmm6, xmm5, edx	 #AVX512-FP16
	vcvtusi2sh	xmm6, xmm5, edx{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtusi2sh	xmm6, xmm5, DWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16
	vcvtusi2sh	xmm6, xmm5, DWORD PTR [ecx]	 #AVX512-FP16
	vcvtusi2sh	xmm6, xmm5, DWORD PTR [ecx+508]	 #AVX512-FP16 Disp8(7f)
	vcvtusi2sh	xmm6, xmm5, DWORD PTR [edx-512]	 #AVX512-FP16 Disp8(80)
	vcvtuw2ph	zmm6, zmm5	 #AVX512-FP16
	vcvtuw2ph	zmm6, zmm5{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtuw2ph	zmm6{k7}{z}, zmm5{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtuw2ph	zmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtuw2ph	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvtuw2ph	zmm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtuw2ph	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtw2ph	zmm6, zmm5	 #AVX512-FP16
	vcvtw2ph	zmm6, zmm5{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtw2ph	zmm6{k7}{z}, zmm5{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtw2ph	zmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtw2ph	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vcvtw2ph	zmm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtw2ph	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vdivph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vdivph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vdivph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vdivph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vdivph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vdivph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vdivph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vdivsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vdivsh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vdivsh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vdivsh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vdivsh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vdivsh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vdivsh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfcmaddcph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfcmaddcph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmaddcph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmaddcph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfcmaddcph	zmm6, zmm5, DWORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfcmaddcph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfcmaddcph	zmm6{k7}{z}, zmm5, DWORD BCST [edx-512]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmaddcsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfcmaddcsh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmaddcsh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmaddcsh	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfcmaddcsh	xmm6, xmm5, DWORD PTR [ecx]	 #AVX512-FP16
	vfcmaddcsh	xmm6, xmm5, DWORD PTR [ecx+508]	 #AVX512-FP16 Disp8(7f)
	vfcmaddcsh	xmm6{k7}{z}, xmm5, DWORD PTR [edx-512]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfcmulcph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfcmulcph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmulcph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmulcph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfcmulcph	zmm6, zmm5, DWORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfcmulcph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfcmulcph	zmm6{k7}{z}, zmm5, DWORD BCST [edx-512]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmulcsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfcmulcsh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmulcsh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmulcsh	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfcmulcsh	xmm6, xmm5, DWORD PTR [ecx]	 #AVX512-FP16
	vfcmulcsh	xmm6, xmm5, DWORD PTR [ecx+508]	 #AVX512-FP16 Disp8(7f)
	vfcmulcsh	xmm6{k7}{z}, xmm5, DWORD PTR [edx-512]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd132ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfmadd132ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd132ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd132ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmadd132ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfmadd132ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmadd132ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd132sh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfmadd132sh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd132sh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd132sh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmadd132sh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vfmadd132sh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vfmadd132sh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd213ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfmadd213ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd213ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd213ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmadd213ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfmadd213ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmadd213ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd213sh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfmadd213sh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd213sh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd213sh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmadd213sh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vfmadd213sh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vfmadd213sh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd231ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfmadd231ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd231ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd231ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmadd231ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfmadd231ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmadd231ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd231sh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfmadd231sh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd231sh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd231sh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmadd231sh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vfmadd231sh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vfmadd231sh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddcph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfmaddcph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddcph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddcph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmaddcph	zmm6, zmm5, DWORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfmaddcph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmaddcph	zmm6{k7}{z}, zmm5, DWORD BCST [edx-512]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddcsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfmaddcsh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddcsh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddcsh	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmaddcsh	xmm6, xmm5, DWORD PTR [ecx]	 #AVX512-FP16
	vfmaddcsh	xmm6, xmm5, DWORD PTR [ecx+508]	 #AVX512-FP16 Disp8(7f)
	vfmaddcsh	xmm6{k7}{z}, xmm5, DWORD PTR [edx-512]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub132ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfmaddsub132ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddsub132ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddsub132ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmaddsub132ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfmaddsub132ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmaddsub132ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub213ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfmaddsub213ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddsub213ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddsub213ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmaddsub213ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfmaddsub213ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmaddsub213ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub231ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfmaddsub231ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddsub231ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddsub231ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmaddsub231ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfmaddsub231ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmaddsub231ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub132ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfmsub132ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub132ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub132ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsub132ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfmsub132ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmsub132ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub132sh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfmsub132sh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub132sh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub132sh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsub132sh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vfmsub132sh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vfmsub132sh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub213ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfmsub213ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub213ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub213ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsub213ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfmsub213ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmsub213ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub213sh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfmsub213sh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub213sh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub213sh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsub213sh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vfmsub213sh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vfmsub213sh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub231ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfmsub231ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub231ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub231ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsub231ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfmsub231ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmsub231ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub231sh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfmsub231sh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub231sh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub231sh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsub231sh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vfmsub231sh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vfmsub231sh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd132ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfmsubadd132ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsubadd132ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsubadd132ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsubadd132ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfmsubadd132ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmsubadd132ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd213ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfmsubadd213ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsubadd213ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsubadd213ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsubadd213ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfmsubadd213ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmsubadd213ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd231ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfmsubadd231ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsubadd231ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsubadd231ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsubadd231ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfmsubadd231ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmsubadd231ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmulcph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfmulcph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmulcph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmulcph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmulcph	zmm6, zmm5, DWORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfmulcph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmulcph	zmm6{k7}{z}, zmm5, DWORD BCST [edx-512]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmulcsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfmulcsh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmulcsh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmulcsh	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmulcsh	xmm6, xmm5, DWORD PTR [ecx]	 #AVX512-FP16
	vfmulcsh	xmm6, xmm5, DWORD PTR [ecx+508]	 #AVX512-FP16 Disp8(7f)
	vfmulcsh	xmm6{k7}{z}, xmm5, DWORD PTR [edx-512]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd132ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfnmadd132ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd132ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd132ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmadd132ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfnmadd132ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfnmadd132ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd132sh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfnmadd132sh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd132sh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd132sh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmadd132sh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vfnmadd132sh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vfnmadd132sh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd213ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfnmadd213ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd213ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd213ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmadd213ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfnmadd213ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfnmadd213ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd213sh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfnmadd213sh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd213sh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd213sh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmadd213sh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vfnmadd213sh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vfnmadd213sh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd231ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfnmadd231ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd231ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd231ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmadd231ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfnmadd231ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfnmadd231ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd231sh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfnmadd231sh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd231sh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd231sh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmadd231sh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vfnmadd231sh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vfnmadd231sh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub132ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfnmsub132ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub132ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub132ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmsub132ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfnmsub132ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfnmsub132ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub132sh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfnmsub132sh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub132sh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub132sh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmsub132sh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vfnmsub132sh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vfnmsub132sh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub213ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfnmsub213ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub213ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub213ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmsub213ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfnmsub213ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfnmsub213ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub213sh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfnmsub213sh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub213sh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub213sh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmsub213sh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vfnmsub213sh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vfnmsub213sh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub231ph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vfnmsub231ph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub231ph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub231ph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmsub231ph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vfnmsub231ph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vfnmsub231ph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub231sh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vfnmsub231sh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub231sh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub231sh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmsub231sh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vfnmsub231sh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vfnmsub231sh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfpclassph	k5, zmm6, 123	 #AVX512-FP16
	vfpclassph	k5{k7}, zmm6, 123	 #AVX512-FP16 MASK_ENABLING
	vfpclassph	k5{k7}, ZMMWORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vfpclassph	k5, WORD BCST [ecx]{1to32}, 123	 #AVX512-FP16 BROADCAST_EN
	vfpclassph	k5, ZMMWORD PTR [ecx+8128], 123	 #AVX512-FP16 Disp8(7f)
	vfpclassph	k5{k7}, WORD BCST [edx-256]{1to32}, 123	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING
	vfpclasssh	k5, xmm6, 123	 #AVX512-FP16
	vfpclasssh	k5{k7}, xmm6, 123	 #AVX512-FP16 MASK_ENABLING
	vfpclasssh	k5{k7}, WORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vfpclasssh	k5, WORD PTR [ecx], 123	 #AVX512-FP16
	vfpclasssh	k5, WORD PTR [ecx+254], 123	 #AVX512-FP16 Disp8(7f)
	vfpclasssh	k5{k7}, WORD PTR [edx-256], 123	 #AVX512-FP16 Disp8(80) MASK_ENABLING
	vgetexpph	zmm6, zmm5	 #AVX512-FP16
	vgetexpph	zmm6, zmm5{sae}	 #AVX512-FP16 HAS_SAE
	vgetexpph	zmm6{k7}{z}, zmm5{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetexpph	zmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vgetexpph	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vgetexpph	zmm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vgetexpph	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vgetexpsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vgetexpsh	xmm6, xmm5, xmm4{sae}	 #AVX512-FP16 HAS_SAE
	vgetexpsh	xmm6{k7}{z}, xmm5, xmm4{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetexpsh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vgetexpsh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vgetexpsh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vgetexpsh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vgetmantph	zmm6, zmm5, 123	 #AVX512-FP16
	vgetmantph	zmm6, zmm5{sae}, 123	 #AVX512-FP16 HAS_SAE
	vgetmantph	zmm6{k7}{z}, zmm5{sae}, 123	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetmantph	zmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vgetmantph	zmm6, WORD BCST [ecx], 123	 #AVX512-FP16 BROADCAST_EN
	vgetmantph	zmm6, ZMMWORD PTR [ecx+8128], 123	 #AVX512-FP16 Disp8(7f)
	vgetmantph	zmm6{k7}{z}, WORD BCST [edx-256], 123	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vgetmantsh	xmm6, xmm5, xmm4, 123	 #AVX512-FP16
	vgetmantsh	xmm6, xmm5, xmm4{sae}, 123	 #AVX512-FP16 HAS_SAE
	vgetmantsh	xmm6{k7}{z}, xmm5, xmm4{sae}, 123	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetmantsh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vgetmantsh	xmm6, xmm5, WORD PTR [ecx], 123	 #AVX512-FP16
	vgetmantsh	xmm6, xmm5, WORD PTR [ecx+254], 123	 #AVX512-FP16 Disp8(7f)
	vgetmantsh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256], 123	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vmaxph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vmaxph	zmm6, zmm5, zmm4{sae}	 #AVX512-FP16 HAS_SAE
	vmaxph	zmm6{k7}{z}, zmm5, zmm4{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vmaxph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vmaxph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vmaxph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vmaxph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmaxsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vmaxsh	xmm6, xmm5, xmm4{sae}	 #AVX512-FP16 HAS_SAE
	vmaxsh	xmm6{k7}{z}, xmm5, xmm4{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vmaxsh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vmaxsh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vmaxsh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vmaxsh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vminph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vminph	zmm6, zmm5, zmm4{sae}	 #AVX512-FP16 HAS_SAE
	vminph	zmm6{k7}{z}, zmm5, zmm4{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vminph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vminph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vminph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vminph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vminsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vminsh	xmm6, xmm5, xmm4{sae}	 #AVX512-FP16 HAS_SAE
	vminsh	xmm6{k7}{z}, xmm5, xmm4{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vminsh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vminsh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vminsh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vminsh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vmovsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vmovsh	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vmovsh	xmm6{k7}, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vmovsh	xmm6, WORD PTR [ecx]	 #AVX512-FP16
	vmovsh	xmm6, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vmovsh	xmm6{k7}{z}, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vmovsh	WORD PTR [esp+esi*8+0x10000000]{k7}, xmm6	 #AVX512-FP16 MASK_ENABLING
	vmovsh	WORD PTR [ecx], xmm6	 #AVX512-FP16
	vmovsh	WORD PTR [ecx+254], xmm6	 #AVX512-FP16 Disp8(7f)
	vmovsh	WORD PTR [edx-256]{k7}, xmm6	 #AVX512-FP16 Disp8(80) MASK_ENABLING
	vmovw	xmm6, edx	 #AVX512-FP16
	vmovw	edx, xmm6	 #AVX512-FP16
	vmovw	xmm6, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16
	vmovw	xmm6, WORD PTR [ecx]	 #AVX512-FP16
	vmovw	xmm6, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vmovw	xmm6, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80)
	vmovw	WORD PTR [esp+esi*8+0x10000000], xmm6	 #AVX512-FP16
	vmovw	WORD PTR [ecx], xmm6	 #AVX512-FP16
	vmovw	WORD PTR [ecx+254], xmm6	 #AVX512-FP16 Disp8(7f)
	vmovw	WORD PTR [edx-256], xmm6	 #AVX512-FP16 Disp8(80)
	vmulph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vmulph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vmulph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vmulph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vmulph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vmulph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vmulph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmulsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vmulsh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vmulsh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vmulsh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vmulsh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vmulsh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vmulsh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vrcpph	zmm6, zmm5	 #AVX512-FP16
	vrcpph	zmm6{k7}{z}, zmm5	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrcpph	zmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vrcpph	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vrcpph	zmm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vrcpph	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrcpsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vrcpsh	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrcpsh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vrcpsh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vrcpsh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vrcpsh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vreduceph	zmm6, zmm5, 123	 #AVX512-FP16
	vreduceph	zmm6, zmm5{sae}, 123	 #AVX512-FP16 HAS_SAE
	vreduceph	zmm6{k7}{z}, zmm5{sae}, 123	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vreduceph	zmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vreduceph	zmm6, WORD BCST [ecx], 123	 #AVX512-FP16 BROADCAST_EN
	vreduceph	zmm6, ZMMWORD PTR [ecx+8128], 123	 #AVX512-FP16 Disp8(7f)
	vreduceph	zmm6{k7}{z}, WORD BCST [edx-256], 123	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vreducesh	xmm6, xmm5, xmm4, 123	 #AVX512-FP16
	vreducesh	xmm6, xmm5, xmm4{sae}, 123	 #AVX512-FP16 HAS_SAE
	vreducesh	xmm6{k7}{z}, xmm5, xmm4{sae}, 123	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vreducesh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vreducesh	xmm6, xmm5, WORD PTR [ecx], 123	 #AVX512-FP16
	vreducesh	xmm6, xmm5, WORD PTR [ecx+254], 123	 #AVX512-FP16 Disp8(7f)
	vreducesh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256], 123	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vrndscaleph	zmm6, zmm5, 123	 #AVX512-FP16
	vrndscaleph	zmm6, zmm5{sae}, 123	 #AVX512-FP16 HAS_SAE
	vrndscaleph	zmm6{k7}{z}, zmm5{sae}, 123	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vrndscaleph	zmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vrndscaleph	zmm6, WORD BCST [ecx], 123	 #AVX512-FP16 BROADCAST_EN
	vrndscaleph	zmm6, ZMMWORD PTR [ecx+8128], 123	 #AVX512-FP16 Disp8(7f)
	vrndscaleph	zmm6{k7}{z}, WORD BCST [edx-256], 123	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrndscalesh	xmm6, xmm5, xmm4, 123	 #AVX512-FP16
	vrndscalesh	xmm6, xmm5, xmm4{sae}, 123	 #AVX512-FP16 HAS_SAE
	vrndscalesh	xmm6{k7}{z}, xmm5, xmm4{sae}, 123	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vrndscalesh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vrndscalesh	xmm6, xmm5, WORD PTR [ecx], 123	 #AVX512-FP16
	vrndscalesh	xmm6, xmm5, WORD PTR [ecx+254], 123	 #AVX512-FP16 Disp8(7f)
	vrndscalesh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256], 123	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vrsqrtph	zmm6, zmm5	 #AVX512-FP16
	vrsqrtph	zmm6{k7}{z}, zmm5	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrsqrtph	zmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vrsqrtph	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vrsqrtph	zmm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vrsqrtph	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrsqrtsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vrsqrtsh	xmm6{k7}{z}, xmm5, xmm4	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrsqrtsh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vrsqrtsh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vrsqrtsh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vrsqrtsh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vscalefph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vscalefph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vscalefph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vscalefph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vscalefph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vscalefph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vscalefph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vscalefsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vscalefsh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vscalefsh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vscalefsh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vscalefsh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vscalefsh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vscalefsh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vsqrtph	zmm6, zmm5	 #AVX512-FP16
	vsqrtph	zmm6, zmm5{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsqrtph	zmm6{k7}{z}, zmm5{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsqrtph	zmm6{k7}, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vsqrtph	zmm6, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vsqrtph	zmm6, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vsqrtph	zmm6{k7}{z}, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsqrtsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vsqrtsh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsqrtsh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsqrtsh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vsqrtsh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vsqrtsh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vsqrtsh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vsubph	zmm6, zmm5, zmm4	 #AVX512-FP16
	vsubph	zmm6, zmm5, zmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsubph	zmm6{k7}{z}, zmm5, zmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsubph	zmm6{k7}, zmm5, ZMMWORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vsubph	zmm6, zmm5, WORD BCST [ecx]	 #AVX512-FP16 BROADCAST_EN
	vsubph	zmm6, zmm5, ZMMWORD PTR [ecx+8128]	 #AVX512-FP16 Disp8(7f)
	vsubph	zmm6{k7}{z}, zmm5, WORD BCST [edx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsubsh	xmm6, xmm5, xmm4	 #AVX512-FP16
	vsubsh	xmm6, xmm5, xmm4{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsubsh	xmm6{k7}{z}, xmm5, xmm4{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsubsh	xmm6{k7}, xmm5, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vsubsh	xmm6, xmm5, WORD PTR [ecx]	 #AVX512-FP16
	vsubsh	xmm6, xmm5, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vsubsh	xmm6{k7}{z}, xmm5, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vucomish	xmm6, xmm5	 #AVX512-FP16
	vucomish	xmm6, xmm5{sae}	 #AVX512-FP16 HAS_SAE
	vucomish	xmm6, WORD PTR [esp+esi*8+0x10000000]	 #AVX512-FP16
	vucomish	xmm6, WORD PTR [ecx]	 #AVX512-FP16
	vucomish	xmm6, WORD PTR [ecx+254]	 #AVX512-FP16 Disp8(7f)
	vucomish	xmm6, WORD PTR [edx-256]	 #AVX512-FP16 Disp8(80)
