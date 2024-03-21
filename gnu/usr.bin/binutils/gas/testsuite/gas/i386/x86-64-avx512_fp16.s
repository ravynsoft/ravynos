# Check 64bit AVX512-FP16 instructions

	.allow_index_reg
	.text
_start:
	vaddph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vaddph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vaddph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vaddph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vaddph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vaddph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vaddph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vaddsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vaddsh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vaddsh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vaddsh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vaddsh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vaddsh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vaddsh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcmpph	$123, %zmm28, %zmm29, %k5	 #AVX512-FP16
	vcmpph	$123, {sae}, %zmm28, %zmm29, %k5	 #AVX512-FP16 HAS_SAE
	vcmpph	$123, {sae}, %zmm28, %zmm29, %k5{%k7}	 #AVX512-FP16 MASK_ENABLING HAS_SAE
	vcmpph	$123, 0x10000000(%rbp, %r14, 8), %zmm29, %k5{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcmpph	$123, (%r9){1to32}, %zmm29, %k5	 #AVX512-FP16 BROADCAST_EN
	vcmpph	$123, 8128(%rcx), %zmm29, %k5	 #AVX512-FP16 Disp8(7f)
	vcmpph	$123, -256(%rdx){1to32}, %zmm29, %k5{%k7}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING
	vcmpsh	$123, %xmm28, %xmm29, %k5	 #AVX512-FP16
	vcmpsh	$123, {sae}, %xmm28, %xmm29, %k5	 #AVX512-FP16 HAS_SAE
	vcmpsh	$123, {sae}, %xmm28, %xmm29, %k5{%k7}	 #AVX512-FP16 MASK_ENABLING HAS_SAE
	vcmpsh	$123, 0x10000000(%rbp, %r14, 8), %xmm29, %k5{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcmpsh	$123, (%r9), %xmm29, %k5	 #AVX512-FP16
	vcmpsh	$123, 254(%rcx), %xmm29, %k5	 #AVX512-FP16 Disp8(7f)
	vcmpsh	$123, -256(%rdx), %xmm29, %k5{%k7}	 #AVX512-FP16 Disp8(80) MASK_ENABLING
	vcomish	%xmm29, %xmm30	 #AVX512-FP16
	vcomish	{sae}, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE
	vcomish	0x10000000(%rbp, %r14, 8), %xmm30	 #AVX512-FP16
	vcomish	(%r9), %xmm30	 #AVX512-FP16
	vcomish	254(%rcx), %xmm30	 #AVX512-FP16 Disp8(7f)
	vcomish	-256(%rdx), %xmm30	 #AVX512-FP16 Disp8(80)
	vcvtdq2ph	%zmm29, %ymm30	 #AVX512-FP16
	vcvtdq2ph	{rn-sae}, %zmm29, %ymm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtdq2ph	{rn-sae}, %zmm29, %ymm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtdq2ph	0x10000000(%rbp, %r14, 8), %ymm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtdq2ph	(%r9){1to16}, %ymm30	 #AVX512-FP16 BROADCAST_EN
	vcvtdq2ph	8128(%rcx), %ymm30	 #AVX512-FP16 Disp8(7f)
	vcvtdq2ph	-512(%rdx){1to16}, %ymm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtpd2ph	%zmm29, %xmm30	 #AVX512-FP16
	vcvtpd2ph	{rn-sae}, %zmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtpd2ph	{rn-sae}, %zmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtpd2phz	0x10000000(%rbp, %r14, 8), %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtpd2ph	(%r9){1to8}, %xmm30	 #AVX512-FP16 BROADCAST_EN
	vcvtpd2phz	8128(%rcx), %xmm30	 #AVX512-FP16 Disp8(7f)
	vcvtpd2ph	-1024(%rdx){1to8}, %xmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2dq	%ymm29, %zmm30	 #AVX512-FP16
	vcvtph2dq	{rn-sae}, %ymm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2dq	{rn-sae}, %ymm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2dq	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2dq	(%r9){1to16}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvtph2dq	4064(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvtph2dq	-256(%rdx){1to16}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2pd	%xmm29, %zmm30	 #AVX512-FP16
	vcvtph2pd	{sae}, %xmm29, %zmm30	 #AVX512-FP16 HAS_SAE
	vcvtph2pd	{sae}, %xmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtph2pd	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2pd	(%r9){1to8}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvtph2pd	2032(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvtph2pd	-256(%rdx){1to8}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2psx	%ymm29, %zmm30	 #AVX512-FP16
	vcvtph2psx	{sae}, %ymm29, %zmm30	 #AVX512-FP16 HAS_SAE
	vcvtph2psx	{sae}, %ymm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtph2psx	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2psx	(%r9){1to16}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvtph2psx	4064(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvtph2psx	-256(%rdx){1to16}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2qq	%xmm29, %zmm30	 #AVX512-FP16
	vcvtph2qq	{rn-sae}, %xmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2qq	{rn-sae}, %xmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2qq	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2qq	(%r9){1to8}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvtph2qq	2032(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvtph2qq	-256(%rdx){1to8}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2udq	%ymm29, %zmm30	 #AVX512-FP16
	vcvtph2udq	{rn-sae}, %ymm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2udq	{rn-sae}, %ymm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2udq	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2udq	(%r9){1to16}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvtph2udq	4064(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvtph2udq	-256(%rdx){1to16}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uqq	%xmm29, %zmm30	 #AVX512-FP16
	vcvtph2uqq	{rn-sae}, %xmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2uqq	{rn-sae}, %xmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2uqq	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2uqq	(%r9){1to8}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvtph2uqq	2032(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvtph2uqq	-256(%rdx){1to8}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uw	%zmm29, %zmm30	 #AVX512-FP16
	vcvtph2uw	{rn-sae}, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2uw	{rn-sae}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2uw	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2uw	(%r9){1to32}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvtph2uw	8128(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvtph2uw	-256(%rdx){1to32}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2w	%zmm29, %zmm30	 #AVX512-FP16
	vcvtph2w	{rn-sae}, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2w	{rn-sae}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2w	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtph2w	(%r9){1to32}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvtph2w	8128(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvtph2w	-256(%rdx){1to32}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtps2phx	%zmm29, %ymm30	 #AVX512-FP16
	vcvtps2phx	{rn-sae}, %zmm29, %ymm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtps2phx	{rn-sae}, %zmm29, %ymm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtps2phx	0x10000000(%rbp, %r14, 8), %ymm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtps2phx	(%r9){1to16}, %ymm30	 #AVX512-FP16 BROADCAST_EN
	vcvtps2phx	8128(%rcx), %ymm30	 #AVX512-FP16 Disp8(7f)
	vcvtps2phx	-512(%rdx){1to16}, %ymm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtqq2ph	%zmm29, %xmm30	 #AVX512-FP16
	vcvtqq2ph	{rn-sae}, %zmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtqq2ph	{rn-sae}, %zmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtqq2phz	0x10000000(%rbp, %r14, 8), %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtqq2ph	(%r9){1to8}, %xmm30	 #AVX512-FP16 BROADCAST_EN
	vcvtqq2phz	8128(%rcx), %xmm30	 #AVX512-FP16 Disp8(7f)
	vcvtqq2ph	-1024(%rdx){1to8}, %xmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsd2sh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vcvtsd2sh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsd2sh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtsd2sh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtsd2sh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vcvtsd2sh	1016(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vcvtsd2sh	-1024(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsh2sd	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vcvtsh2sd	{sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE
	vcvtsh2sd	{sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtsh2sd	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtsh2sd	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vcvtsh2sd	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vcvtsh2sd	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsh2si	%xmm30, %edx	 #AVX512-FP16
	vcvtsh2si	{rn-sae}, %xmm30, %edx	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsh2si	%xmm30, %r12	 #AVX512-FP16
	vcvtsh2si	{rn-sae}, %xmm30, %r12	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsh2si	0x10000000(%rbp, %r14, 8), %edx	 #AVX512-FP16
	vcvtsh2si	(%r9), %edx	 #AVX512-FP16
	vcvtsh2si	254(%rcx), %edx	 #AVX512-FP16 Disp8(7f)
	vcvtsh2si	-256(%rdx), %edx	 #AVX512-FP16 Disp8(80)
	vcvtsh2si	0x10000000(%rbp, %r14, 8), %r12	 #AVX512-FP16
	vcvtsh2si	(%r9), %r12	 #AVX512-FP16
	vcvtsh2si	254(%rcx), %r12	 #AVX512-FP16 Disp8(7f)
	vcvtsh2si	-256(%rdx), %r12	 #AVX512-FP16 Disp8(80)
	vcvtsh2ss	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vcvtsh2ss	{sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE
	vcvtsh2ss	{sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtsh2ss	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtsh2ss	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vcvtsh2ss	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vcvtsh2ss	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsh2usi	%xmm30, %edx	 #AVX512-FP16
	vcvtsh2usi	{rn-sae}, %xmm30, %edx	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsh2usi	%xmm30, %r12	 #AVX512-FP16
	vcvtsh2usi	{rn-sae}, %xmm30, %r12	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsh2usi	0x10000000(%rbp, %r14, 8), %edx	 #AVX512-FP16
	vcvtsh2usi	(%r9), %edx	 #AVX512-FP16
	vcvtsh2usi	254(%rcx), %edx	 #AVX512-FP16 Disp8(7f)
	vcvtsh2usi	-256(%rdx), %edx	 #AVX512-FP16 Disp8(80)
	vcvtsh2usi	0x10000000(%rbp, %r14, 8), %r12	 #AVX512-FP16
	vcvtsh2usi	(%r9), %r12	 #AVX512-FP16
	vcvtsh2usi	254(%rcx), %r12	 #AVX512-FP16 Disp8(7f)
	vcvtsh2usi	-256(%rdx), %r12	 #AVX512-FP16 Disp8(80)
	vcvtsi2sh	%r12, %xmm29, %xmm30	 #AVX512-FP16
	vcvtsi2sh	%r12, {rn-sae}, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsi2sh	%edx, %xmm29, %xmm30	 #AVX512-FP16
	vcvtsi2sh	%edx, {rn-sae}, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsi2shl	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30	 #AVX512-FP16
	vcvtsi2shl	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vcvtsi2shl	508(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vcvtsi2shl	-512(%rdx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(80)
	vcvtsi2shq	1016(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vcvtsi2shq	-1024(%rdx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(80)
	vcvtss2sh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vcvtss2sh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtss2sh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtss2sh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtss2sh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vcvtss2sh	508(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vcvtss2sh	-512(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2dq	%ymm29, %zmm30	 #AVX512-FP16
	vcvttph2dq	{sae}, %ymm29, %zmm30	 #AVX512-FP16 HAS_SAE
	vcvttph2dq	{sae}, %ymm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2dq	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvttph2dq	(%r9){1to16}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvttph2dq	4064(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvttph2dq	-256(%rdx){1to16}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2qq	%xmm29, %zmm30	 #AVX512-FP16
	vcvttph2qq	{sae}, %xmm29, %zmm30	 #AVX512-FP16 HAS_SAE
	vcvttph2qq	{sae}, %xmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2qq	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvttph2qq	(%r9){1to8}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvttph2qq	2032(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvttph2qq	-256(%rdx){1to8}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2udq	%ymm29, %zmm30	 #AVX512-FP16
	vcvttph2udq	{sae}, %ymm29, %zmm30	 #AVX512-FP16 HAS_SAE
	vcvttph2udq	{sae}, %ymm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2udq	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvttph2udq	(%r9){1to16}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvttph2udq	4064(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvttph2udq	-256(%rdx){1to16}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uqq	%xmm29, %zmm30	 #AVX512-FP16
	vcvttph2uqq	{sae}, %xmm29, %zmm30	 #AVX512-FP16 HAS_SAE
	vcvttph2uqq	{sae}, %xmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2uqq	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvttph2uqq	(%r9){1to8}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvttph2uqq	2032(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvttph2uqq	-256(%rdx){1to8}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uw	%zmm29, %zmm30	 #AVX512-FP16
	vcvttph2uw	{sae}, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE
	vcvttph2uw	{sae}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2uw	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvttph2uw	(%r9){1to32}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvttph2uw	8128(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvttph2uw	-256(%rdx){1to32}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2w	%zmm29, %zmm30	 #AVX512-FP16
	vcvttph2w	{sae}, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE
	vcvttph2w	{sae}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2w	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvttph2w	(%r9){1to32}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvttph2w	8128(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvttph2w	-256(%rdx){1to32}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttsh2si	%xmm30, %edx	 #AVX512-FP16
	vcvttsh2si	{sae}, %xmm30, %edx	 #AVX512-FP16 HAS_SAE
	vcvttsh2si	%xmm30, %r12	 #AVX512-FP16
	vcvttsh2si	{sae}, %xmm30, %r12	 #AVX512-FP16 HAS_SAE
	vcvttsh2si	0x10000000(%rbp, %r14, 8), %edx	 #AVX512-FP16
	vcvttsh2si	(%r9), %edx	 #AVX512-FP16
	vcvttsh2si	254(%rcx), %edx	 #AVX512-FP16 Disp8(7f)
	vcvttsh2si	-256(%rdx), %edx	 #AVX512-FP16 Disp8(80)
	vcvttsh2si	0x10000000(%rbp, %r14, 8), %r12	 #AVX512-FP16
	vcvttsh2si	(%r9), %r12	 #AVX512-FP16
	vcvttsh2si	254(%rcx), %r12	 #AVX512-FP16 Disp8(7f)
	vcvttsh2si	-256(%rdx), %r12	 #AVX512-FP16 Disp8(80)
	vcvttsh2usi	%xmm30, %edx	 #AVX512-FP16
	vcvttsh2usi	{sae}, %xmm30, %edx	 #AVX512-FP16 HAS_SAE
	vcvttsh2usi	%xmm30, %r12	 #AVX512-FP16
	vcvttsh2usi	{sae}, %xmm30, %r12	 #AVX512-FP16 HAS_SAE
	vcvttsh2usi	0x10000000(%rbp, %r14, 8), %edx	 #AVX512-FP16
	vcvttsh2usi	(%r9), %edx	 #AVX512-FP16
	vcvttsh2usi	254(%rcx), %edx	 #AVX512-FP16 Disp8(7f)
	vcvttsh2usi	-256(%rdx), %edx	 #AVX512-FP16 Disp8(80)
	vcvttsh2usi	0x10000000(%rbp, %r14, 8), %r12	 #AVX512-FP16
	vcvttsh2usi	(%r9), %r12	 #AVX512-FP16
	vcvttsh2usi	254(%rcx), %r12	 #AVX512-FP16 Disp8(7f)
	vcvttsh2usi	-256(%rdx), %r12	 #AVX512-FP16 Disp8(80)
	vcvtudq2ph	%zmm29, %ymm30	 #AVX512-FP16
	vcvtudq2ph	{rn-sae}, %zmm29, %ymm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtudq2ph	{rn-sae}, %zmm29, %ymm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtudq2ph	0x10000000(%rbp, %r14, 8), %ymm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtudq2ph	(%r9){1to16}, %ymm30	 #AVX512-FP16 BROADCAST_EN
	vcvtudq2ph	8128(%rcx), %ymm30	 #AVX512-FP16 Disp8(7f)
	vcvtudq2ph	-512(%rdx){1to16}, %ymm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtuqq2ph	%zmm29, %xmm30	 #AVX512-FP16
	vcvtuqq2ph	{rn-sae}, %zmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtuqq2ph	{rn-sae}, %zmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtuqq2phz	0x10000000(%rbp, %r14, 8), %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtuqq2ph	(%r9){1to8}, %xmm30	 #AVX512-FP16 BROADCAST_EN
	vcvtuqq2phz	8128(%rcx), %xmm30	 #AVX512-FP16 Disp8(7f)
	vcvtuqq2ph	-1024(%rdx){1to8}, %xmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtusi2sh	%r12, %xmm29, %xmm30	 #AVX512-FP16
	vcvtusi2sh	%r12, {rn-sae}, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtusi2sh	%edx, %xmm29, %xmm30	 #AVX512-FP16
	vcvtusi2sh	%edx, {rn-sae}, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtusi2shl	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30	 #AVX512-FP16
	vcvtusi2shl	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vcvtusi2shl	508(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vcvtusi2shl	-512(%rdx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(80)
	vcvtusi2shq	1016(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vcvtusi2shq	-1024(%rdx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(80)
	vcvtuw2ph	%zmm29, %zmm30	 #AVX512-FP16
	vcvtuw2ph	{rn-sae}, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtuw2ph	{rn-sae}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtuw2ph	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtuw2ph	(%r9){1to32}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvtuw2ph	8128(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvtuw2ph	-256(%rdx){1to32}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtw2ph	%zmm29, %zmm30	 #AVX512-FP16
	vcvtw2ph	{rn-sae}, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtw2ph	{rn-sae}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtw2ph	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vcvtw2ph	(%r9){1to32}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vcvtw2ph	8128(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vcvtw2ph	-256(%rdx){1to32}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vdivph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vdivph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vdivph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vdivph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vdivph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vdivph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vdivph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vdivsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vdivsh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vdivsh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vdivsh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vdivsh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vdivsh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vdivsh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfcmaddcph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfcmaddcph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmaddcph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmaddcph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfcmaddcph	(%r9){1to16}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfcmaddcph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfcmaddcph	-512(%rdx){1to16}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmaddcsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfcmaddcsh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmaddcsh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmaddcsh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfcmaddcsh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfcmaddcsh	508(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfcmaddcsh	-512(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfcmulcph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfcmulcph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmulcph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmulcph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfcmulcph	(%r9){1to16}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfcmulcph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfcmulcph	-512(%rdx){1to16}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmulcsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfcmulcsh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmulcsh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmulcsh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfcmulcsh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfcmulcsh	508(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfcmulcsh	-512(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd132ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfmadd132ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd132ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd132ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmadd132ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfmadd132ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfmadd132ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd132sh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfmadd132sh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd132sh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd132sh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmadd132sh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfmadd132sh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfmadd132sh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd213ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfmadd213ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd213ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd213ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmadd213ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfmadd213ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfmadd213ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd213sh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfmadd213sh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd213sh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd213sh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmadd213sh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfmadd213sh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfmadd213sh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd231ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfmadd231ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd231ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd231ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmadd231ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfmadd231ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfmadd231ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd231sh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfmadd231sh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd231sh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd231sh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmadd231sh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfmadd231sh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfmadd231sh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddcph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfmaddcph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddcph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddcph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmaddcph	(%r9){1to16}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfmaddcph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfmaddcph	-512(%rdx){1to16}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddcsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfmaddcsh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddcsh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddcsh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmaddcsh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfmaddcsh	508(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfmaddcsh	-512(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub132ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfmaddsub132ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddsub132ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddsub132ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmaddsub132ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfmaddsub132ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfmaddsub132ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub213ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfmaddsub213ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddsub213ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddsub213ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmaddsub213ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfmaddsub213ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfmaddsub213ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub231ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfmaddsub231ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddsub231ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddsub231ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmaddsub231ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfmaddsub231ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfmaddsub231ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub132ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfmsub132ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub132ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub132ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsub132ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfmsub132ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfmsub132ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub132sh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfmsub132sh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub132sh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub132sh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsub132sh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfmsub132sh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfmsub132sh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub213ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfmsub213ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub213ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub213ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsub213ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfmsub213ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfmsub213ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub213sh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfmsub213sh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub213sh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub213sh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsub213sh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfmsub213sh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfmsub213sh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub231ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfmsub231ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub231ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub231ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsub231ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfmsub231ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfmsub231ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub231sh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfmsub231sh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub231sh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub231sh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsub231sh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfmsub231sh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfmsub231sh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd132ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfmsubadd132ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsubadd132ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsubadd132ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsubadd132ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfmsubadd132ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfmsubadd132ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd213ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfmsubadd213ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsubadd213ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsubadd213ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsubadd213ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfmsubadd213ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfmsubadd213ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd231ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfmsubadd231ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsubadd231ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsubadd231ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmsubadd231ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfmsubadd231ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfmsubadd231ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmulcph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfmulcph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmulcph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmulcph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmulcph	(%r9){1to16}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfmulcph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfmulcph	-512(%rdx){1to16}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmulcsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfmulcsh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmulcsh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmulcsh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfmulcsh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfmulcsh	508(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfmulcsh	-512(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd132ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfnmadd132ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd132ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd132ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmadd132ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfnmadd132ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfnmadd132ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd132sh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfnmadd132sh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd132sh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd132sh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmadd132sh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfnmadd132sh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfnmadd132sh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd213ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfnmadd213ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd213ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd213ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmadd213ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfnmadd213ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfnmadd213ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd213sh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfnmadd213sh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd213sh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd213sh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmadd213sh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfnmadd213sh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfnmadd213sh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd231ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfnmadd231ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd231ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd231ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmadd231ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfnmadd231ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfnmadd231ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd231sh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfnmadd231sh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd231sh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd231sh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmadd231sh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfnmadd231sh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfnmadd231sh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub132ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfnmsub132ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub132ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub132ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmsub132ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfnmsub132ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfnmsub132ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub132sh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfnmsub132sh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub132sh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub132sh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmsub132sh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfnmsub132sh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfnmsub132sh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub213ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfnmsub213ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub213ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub213ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmsub213ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfnmsub213ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfnmsub213ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub213sh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfnmsub213sh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub213sh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub213sh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmsub213sh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfnmsub213sh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfnmsub213sh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub231ph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vfnmsub231ph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub231ph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub231ph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmsub231ph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vfnmsub231ph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vfnmsub231ph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub231sh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vfnmsub231sh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub231sh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub231sh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfnmsub231sh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vfnmsub231sh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vfnmsub231sh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfpclassph	$123, %zmm30, %k5	 #AVX512-FP16
	vfpclassph	$123, %zmm30, %k5{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfpclassphz	$123, 0x10000000(%rbp, %r14, 8), %k5{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfpclassph	$123, (%r9){1to32}, %k5	 #AVX512-FP16 BROADCAST_EN
	vfpclassphz	$123, 8128(%rcx), %k5	 #AVX512-FP16 Disp8(7f)
	vfpclassph	$123, -256(%rdx){1to32}, %k5{%k7}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING
	vfpclasssh	$123, %xmm30, %k5	 #AVX512-FP16
	vfpclasssh	$123, %xmm30, %k5{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfpclasssh	$123, 0x10000000(%rbp, %r14, 8), %k5{%k7}	 #AVX512-FP16 MASK_ENABLING
	vfpclasssh	$123, (%r9), %k5	 #AVX512-FP16
	vfpclasssh	$123, 254(%rcx), %k5	 #AVX512-FP16 Disp8(7f)
	vfpclasssh	$123, -256(%rdx), %k5{%k7}	 #AVX512-FP16 Disp8(80) MASK_ENABLING
	vgetexpph	%zmm29, %zmm30	 #AVX512-FP16
	vgetexpph	{sae}, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE
	vgetexpph	{sae}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetexpph	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vgetexpph	(%r9){1to32}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vgetexpph	8128(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vgetexpph	-256(%rdx){1to32}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vgetexpsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vgetexpsh	{sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE
	vgetexpsh	{sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetexpsh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vgetexpsh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vgetexpsh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vgetexpsh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vgetmantph	$123, %zmm29, %zmm30	 #AVX512-FP16
	vgetmantph	$123, {sae}, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE
	vgetmantph	$123, {sae}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetmantph	$123, 0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vgetmantph	$123, (%r9){1to32}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vgetmantph	$123, 8128(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vgetmantph	$123, -256(%rdx){1to32}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vgetmantsh	$123, %xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vgetmantsh	$123, {sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE
	vgetmantsh	$123, {sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetmantsh	$123, 0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vgetmantsh	$123, (%r9), %xmm29, %xmm30	 #AVX512-FP16
	vgetmantsh	$123, 254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vgetmantsh	$123, -256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vmaxph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vmaxph	{sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE
	vmaxph	{sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vmaxph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vmaxph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vmaxph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vmaxph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmaxsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vmaxsh	{sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE
	vmaxsh	{sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vmaxsh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vmaxsh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vmaxsh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vmaxsh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vminph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vminph	{sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE
	vminph	{sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vminph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vminph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vminph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vminph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vminsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vminsh	{sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE
	vminsh	{sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vminsh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vminsh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vminsh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vminsh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vmovsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vmovsh	%xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vmovsh	0x10000000(%rbp, %r14, 8), %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vmovsh	(%r9), %xmm30	 #AVX512-FP16
	vmovsh	254(%rcx), %xmm30	 #AVX512-FP16 Disp8(7f)
	vmovsh	-256(%rdx), %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vmovsh	%xmm30, 0x10000000(%rbp, %r14, 8){%k7}	 #AVX512-FP16 MASK_ENABLING
	vmovsh	%xmm30, (%r9)	 #AVX512-FP16
	vmovsh	%xmm30, 254(%rcx)	 #AVX512-FP16 Disp8(7f)
	vmovsh	%xmm30, -256(%rdx){%k7}	 #AVX512-FP16 Disp8(80) MASK_ENABLING
	vmovw	0x10000000(%rbp, %r14, 8), %xmm30	 #AVX512-FP16
	vmovw	(%r9), %xmm30	 #AVX512-FP16
	vmovw	254(%rcx), %xmm30	 #AVX512-FP16 Disp8(7f)
	vmovw	-256(%rdx), %xmm30	 #AVX512-FP16 Disp8(80)
	vmovw	%xmm30, 0x10000000(%rbp, %r14, 8)	 #AVX512-FP16
	vmovw	%xmm30, (%r9)	 #AVX512-FP16
	vmovw	%xmm30, 254(%rcx)	 #AVX512-FP16 Disp8(7f)
	vmovw	%xmm30, -256(%rdx)	 #AVX512-FP16 Disp8(80)
	vmulph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vmulph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vmulph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vmulph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vmulph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vmulph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vmulph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmulsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vmulsh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vmulsh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vmulsh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vmulsh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vmulsh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vmulsh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vrcpph	%zmm29, %zmm30	 #AVX512-FP16
	vrcpph	%zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrcpph	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vrcpph	(%r9){1to32}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vrcpph	8128(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vrcpph	-256(%rdx){1to32}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrcpsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vrcpsh	%xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrcpsh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vrcpsh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vrcpsh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vrcpsh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vreduceph	$123, %zmm29, %zmm30	 #AVX512-FP16
	vreduceph	$123, {sae}, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE
	vreduceph	$123, {sae}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vreduceph	$123, 0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vreduceph	$123, (%r9){1to32}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vreduceph	$123, 8128(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vreduceph	$123, -256(%rdx){1to32}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vreducesh	$123, %xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vreducesh	$123, {sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE
	vreducesh	$123, {sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vreducesh	$123, 0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vreducesh	$123, (%r9), %xmm29, %xmm30	 #AVX512-FP16
	vreducesh	$123, 254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vreducesh	$123, -256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vrndscaleph	$123, %zmm29, %zmm30	 #AVX512-FP16
	vrndscaleph	$123, {sae}, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE
	vrndscaleph	$123, {sae}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vrndscaleph	$123, 0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vrndscaleph	$123, (%r9){1to32}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vrndscaleph	$123, 8128(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vrndscaleph	$123, -256(%rdx){1to32}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrndscalesh	$123, %xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vrndscalesh	$123, {sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE
	vrndscalesh	$123, {sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vrndscalesh	$123, 0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vrndscalesh	$123, (%r9), %xmm29, %xmm30	 #AVX512-FP16
	vrndscalesh	$123, 254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vrndscalesh	$123, -256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vrsqrtph	%zmm29, %zmm30	 #AVX512-FP16
	vrsqrtph	%zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrsqrtph	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vrsqrtph	(%r9){1to32}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vrsqrtph	8128(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vrsqrtph	-256(%rdx){1to32}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrsqrtsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vrsqrtsh	%xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrsqrtsh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vrsqrtsh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vrsqrtsh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vrsqrtsh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vscalefph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vscalefph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vscalefph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vscalefph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vscalefph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vscalefph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vscalefph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vscalefsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vscalefsh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vscalefsh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vscalefsh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vscalefsh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vscalefsh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vscalefsh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vsqrtph	%zmm29, %zmm30	 #AVX512-FP16
	vsqrtph	{rn-sae}, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsqrtph	{rn-sae}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsqrtph	0x10000000(%rbp, %r14, 8), %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vsqrtph	(%r9){1to32}, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vsqrtph	8128(%rcx), %zmm30	 #AVX512-FP16 Disp8(7f)
	vsqrtph	-256(%rdx){1to32}, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsqrtsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vsqrtsh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsqrtsh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsqrtsh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vsqrtsh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vsqrtsh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vsqrtsh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vsubph	%zmm28, %zmm29, %zmm30	 #AVX512-FP16
	vsubph	{rn-sae}, %zmm28, %zmm29, %zmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsubph	{rn-sae}, %zmm28, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsubph	0x10000000(%rbp, %r14, 8), %zmm29, %zmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vsubph	(%r9){1to32}, %zmm29, %zmm30	 #AVX512-FP16 BROADCAST_EN
	vsubph	8128(%rcx), %zmm29, %zmm30	 #AVX512-FP16 Disp8(7f)
	vsubph	-256(%rdx){1to32}, %zmm29, %zmm30{%k7}{z}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsubsh	%xmm28, %xmm29, %xmm30	 #AVX512-FP16
	vsubsh	{rn-sae}, %xmm28, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsubsh	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsubsh	0x10000000(%rbp, %r14, 8), %xmm29, %xmm30{%k7}	 #AVX512-FP16 MASK_ENABLING
	vsubsh	(%r9), %xmm29, %xmm30	 #AVX512-FP16
	vsubsh	254(%rcx), %xmm29, %xmm30	 #AVX512-FP16 Disp8(7f)
	vsubsh	-256(%rdx), %xmm29, %xmm30{%k7}{z}	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vucomish	%xmm29, %xmm30	 #AVX512-FP16
	vucomish	{sae}, %xmm29, %xmm30	 #AVX512-FP16 HAS_SAE
	vucomish	0x10000000(%rbp, %r14, 8), %xmm30	 #AVX512-FP16
	vucomish	(%r9), %xmm30	 #AVX512-FP16
	vucomish	254(%rcx), %xmm30	 #AVX512-FP16 Disp8(7f)
	vucomish	-256(%rdx), %xmm30	 #AVX512-FP16 Disp8(80)

.intel_syntax noprefix
	vaddph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vaddph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vaddph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vaddph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vaddph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vaddph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vaddph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vaddsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vaddsh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vaddsh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vaddsh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vaddsh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vaddsh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vaddsh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcmpph	k5, zmm29, zmm28, 123	 #AVX512-FP16
	vcmpph	k5, zmm29, zmm28{sae}, 123	 #AVX512-FP16 HAS_SAE
	vcmpph	k5{k7}, zmm29, zmm28{sae}, 123	 #AVX512-FP16 MASK_ENABLING HAS_SAE
	vcmpph	k5{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vcmpph	k5, zmm29, WORD BCST [r9], 123	 #AVX512-FP16 BROADCAST_EN
	vcmpph	k5, zmm29, ZMMWORD PTR [rcx+8128], 123	 #AVX512-FP16 Disp8(7f)
	vcmpph	k5{k7}, zmm29, WORD BCST [rdx-256], 123	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING
	vcmpsh	k5, xmm29, xmm28, 123	 #AVX512-FP16
	vcmpsh	k5, xmm29, xmm28{sae}, 123	 #AVX512-FP16 HAS_SAE
	vcmpsh	k5{k7}, xmm29, xmm28{sae}, 123	 #AVX512-FP16 MASK_ENABLING HAS_SAE
	vcmpsh	k5{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vcmpsh	k5, xmm29, WORD PTR [r9], 123	 #AVX512-FP16
	vcmpsh	k5, xmm29, WORD PTR [rcx+254], 123	 #AVX512-FP16 Disp8(7f)
	vcmpsh	k5{k7}, xmm29, WORD PTR [rdx-256], 123	 #AVX512-FP16 Disp8(80) MASK_ENABLING
	vcomish	xmm30, xmm29	 #AVX512-FP16
	vcomish	xmm30, xmm29{sae}	 #AVX512-FP16 HAS_SAE
	vcomish	xmm30, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16
	vcomish	xmm30, WORD PTR [r9]	 #AVX512-FP16
	vcomish	xmm30, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vcomish	xmm30, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80)
	vcvtdq2ph	ymm30, zmm29	 #AVX512-FP16
	vcvtdq2ph	ymm30, zmm29{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtdq2ph	ymm30{k7}{z}, zmm29{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtdq2ph	ymm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtdq2ph	ymm30, DWORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvtdq2ph	ymm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtdq2ph	ymm30{k7}{z}, DWORD BCST [rdx-512]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtpd2ph	xmm30, zmm29	 #AVX512-FP16
	vcvtpd2ph	xmm30, zmm29{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtpd2ph	xmm30{k7}{z}, zmm29{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtpd2ph	xmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtpd2ph	xmm30, QWORD BCST [r9]{1to8}	 #AVX512-FP16 BROADCAST_EN
	vcvtpd2ph	xmm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtpd2ph	xmm30{k7}{z}, QWORD BCST [rdx-1024]{1to8}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2dq	zmm30, ymm29	 #AVX512-FP16
	vcvtph2dq	zmm30, ymm29{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2dq	zmm30{k7}{z}, ymm29{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2dq	zmm30{k7}, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2dq	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2dq	zmm30, YMMWORD PTR [rcx+4064]	 #AVX512-FP16 Disp8(7f)
	vcvtph2dq	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2pd	zmm30, xmm29	 #AVX512-FP16
	vcvtph2pd	zmm30, xmm29{sae}	 #AVX512-FP16 HAS_SAE
	vcvtph2pd	zmm30{k7}{z}, xmm29{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtph2pd	zmm30{k7}, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2pd	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2pd	zmm30, XMMWORD PTR [rcx+2032]	 #AVX512-FP16 Disp8(7f)
	vcvtph2pd	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2psx	zmm30, ymm29	 #AVX512-FP16
	vcvtph2psx	zmm30, ymm29{sae}	 #AVX512-FP16 HAS_SAE
	vcvtph2psx	zmm30{k7}{z}, ymm29{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtph2psx	zmm30{k7}, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2psx	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2psx	zmm30, YMMWORD PTR [rcx+4064]	 #AVX512-FP16 Disp8(7f)
	vcvtph2psx	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2qq	zmm30, xmm29	 #AVX512-FP16
	vcvtph2qq	zmm30, xmm29{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2qq	zmm30{k7}{z}, xmm29{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2qq	zmm30{k7}, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2qq	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2qq	zmm30, XMMWORD PTR [rcx+2032]	 #AVX512-FP16 Disp8(7f)
	vcvtph2qq	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2udq	zmm30, ymm29	 #AVX512-FP16
	vcvtph2udq	zmm30, ymm29{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2udq	zmm30{k7}{z}, ymm29{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2udq	zmm30{k7}, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2udq	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2udq	zmm30, YMMWORD PTR [rcx+4064]	 #AVX512-FP16 Disp8(7f)
	vcvtph2udq	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uqq	zmm30, xmm29	 #AVX512-FP16
	vcvtph2uqq	zmm30, xmm29{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2uqq	zmm30{k7}{z}, xmm29{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2uqq	zmm30{k7}, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2uqq	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2uqq	zmm30, XMMWORD PTR [rcx+2032]	 #AVX512-FP16 Disp8(7f)
	vcvtph2uqq	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2uw	zmm30, zmm29	 #AVX512-FP16
	vcvtph2uw	zmm30, zmm29{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2uw	zmm30{k7}{z}, zmm29{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2uw	zmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2uw	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2uw	zmm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtph2uw	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtph2w	zmm30, zmm29	 #AVX512-FP16
	vcvtph2w	zmm30, zmm29{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtph2w	zmm30{k7}{z}, zmm29{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtph2w	zmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtph2w	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvtph2w	zmm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtph2w	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtps2phx	ymm30, zmm29	 #AVX512-FP16
	vcvtps2phx	ymm30, zmm29{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtps2phx	ymm30{k7}{z}, zmm29{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtps2phx	ymm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtps2phx	ymm30, DWORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvtps2phx	ymm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtps2phx	ymm30{k7}{z}, DWORD BCST [rdx-512]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtqq2ph	xmm30, zmm29	 #AVX512-FP16
	vcvtqq2ph	xmm30, zmm29{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtqq2ph	xmm30{k7}{z}, zmm29{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtqq2ph	xmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtqq2ph	xmm30, QWORD BCST [r9]{1to8}	 #AVX512-FP16 BROADCAST_EN
	vcvtqq2ph	xmm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtqq2ph	xmm30{k7}{z}, QWORD BCST [rdx-1024]{1to8}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsd2sh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vcvtsd2sh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsd2sh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtsd2sh	xmm30{k7}, xmm29, QWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtsd2sh	xmm30, xmm29, QWORD PTR [r9]	 #AVX512-FP16
	vcvtsd2sh	xmm30, xmm29, QWORD PTR [rcx+1016]	 #AVX512-FP16 Disp8(7f)
	vcvtsd2sh	xmm30{k7}{z}, xmm29, QWORD PTR [rdx-1024]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsh2sd	xmm30, xmm29, xmm28	 #AVX512-FP16
	vcvtsh2sd	xmm30, xmm29, xmm28{sae}	 #AVX512-FP16 HAS_SAE
	vcvtsh2sd	xmm30{k7}{z}, xmm29, xmm28{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtsh2sd	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtsh2sd	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vcvtsh2sd	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vcvtsh2sd	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsh2si	edx, xmm30	 #AVX512-FP16
	vcvtsh2si	edx, xmm30{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsh2si	r12, xmm30	 #AVX512-FP16
	vcvtsh2si	r12, xmm30{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsh2si	edx, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16
	vcvtsh2si	edx, WORD PTR [r9]	 #AVX512-FP16
	vcvtsh2si	edx, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vcvtsh2si	edx, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80)
	vcvtsh2si	r12, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16
	vcvtsh2si	r12, WORD PTR [r9]	 #AVX512-FP16
	vcvtsh2si	r12, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vcvtsh2si	r12, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80)
	vcvtsh2ss	xmm30, xmm29, xmm28	 #AVX512-FP16
	vcvtsh2ss	xmm30, xmm29, xmm28{sae}	 #AVX512-FP16 HAS_SAE
	vcvtsh2ss	xmm30{k7}{z}, xmm29, xmm28{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvtsh2ss	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtsh2ss	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vcvtsh2ss	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vcvtsh2ss	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvtsh2usi	edx, xmm30	 #AVX512-FP16
	vcvtsh2usi	edx, xmm30{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsh2usi	r12, xmm30	 #AVX512-FP16
	vcvtsh2usi	r12, xmm30{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsh2usi	edx, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16
	vcvtsh2usi	edx, WORD PTR [r9]	 #AVX512-FP16
	vcvtsh2usi	edx, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vcvtsh2usi	edx, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80)
	vcvtsh2usi	r12, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16
	vcvtsh2usi	r12, WORD PTR [r9]	 #AVX512-FP16
	vcvtsh2usi	r12, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vcvtsh2usi	r12, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80)
	vcvtsi2sh	xmm30, xmm29, r12	 #AVX512-FP16
	vcvtsi2sh	xmm30, xmm29, r12{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsi2sh	xmm30, xmm29, edx	 #AVX512-FP16
	vcvtsi2sh	xmm30, xmm29, edx{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtsi2sh	xmm30, xmm29, DWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16
	vcvtsi2sh	xmm30, xmm29, DWORD PTR [r9]	 #AVX512-FP16
	vcvtsi2sh	xmm30, xmm29, DWORD PTR [rcx+508]	 #AVX512-FP16 Disp8(7f)
	vcvtsi2sh	xmm30, xmm29, DWORD PTR [rdx-512]	 #AVX512-FP16 Disp8(80)
	vcvtsi2sh	xmm30, xmm29, QWORD PTR [rcx+1016]	 #AVX512-FP16 Disp8(7f)
	vcvtsi2sh	xmm30, xmm29, QWORD PTR [rdx-1024]	 #AVX512-FP16 Disp8(80)
	vcvtss2sh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vcvtss2sh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtss2sh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtss2sh	xmm30{k7}, xmm29, DWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtss2sh	xmm30, xmm29, DWORD PTR [r9]	 #AVX512-FP16
	vcvtss2sh	xmm30, xmm29, DWORD PTR [rcx+508]	 #AVX512-FP16 Disp8(7f)
	vcvtss2sh	xmm30{k7}{z}, xmm29, DWORD PTR [rdx-512]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2dq	zmm30, ymm29	 #AVX512-FP16
	vcvttph2dq	zmm30, ymm29{sae}	 #AVX512-FP16 HAS_SAE
	vcvttph2dq	zmm30{k7}{z}, ymm29{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2dq	zmm30{k7}, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvttph2dq	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvttph2dq	zmm30, YMMWORD PTR [rcx+4064]	 #AVX512-FP16 Disp8(7f)
	vcvttph2dq	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2qq	zmm30, xmm29	 #AVX512-FP16
	vcvttph2qq	zmm30, xmm29{sae}	 #AVX512-FP16 HAS_SAE
	vcvttph2qq	zmm30{k7}{z}, xmm29{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2qq	zmm30{k7}, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvttph2qq	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvttph2qq	zmm30, XMMWORD PTR [rcx+2032]	 #AVX512-FP16 Disp8(7f)
	vcvttph2qq	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2udq	zmm30, ymm29	 #AVX512-FP16
	vcvttph2udq	zmm30, ymm29{sae}	 #AVX512-FP16 HAS_SAE
	vcvttph2udq	zmm30{k7}{z}, ymm29{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2udq	zmm30{k7}, YMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvttph2udq	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvttph2udq	zmm30, YMMWORD PTR [rcx+4064]	 #AVX512-FP16 Disp8(7f)
	vcvttph2udq	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uqq	zmm30, xmm29	 #AVX512-FP16
	vcvttph2uqq	zmm30, xmm29{sae}	 #AVX512-FP16 HAS_SAE
	vcvttph2uqq	zmm30{k7}{z}, xmm29{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2uqq	zmm30{k7}, XMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvttph2uqq	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvttph2uqq	zmm30, XMMWORD PTR [rcx+2032]	 #AVX512-FP16 Disp8(7f)
	vcvttph2uqq	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2uw	zmm30, zmm29	 #AVX512-FP16
	vcvttph2uw	zmm30, zmm29{sae}	 #AVX512-FP16 HAS_SAE
	vcvttph2uw	zmm30{k7}{z}, zmm29{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2uw	zmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvttph2uw	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvttph2uw	zmm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvttph2uw	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttph2w	zmm30, zmm29	 #AVX512-FP16
	vcvttph2w	zmm30, zmm29{sae}	 #AVX512-FP16 HAS_SAE
	vcvttph2w	zmm30{k7}{z}, zmm29{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vcvttph2w	zmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvttph2w	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvttph2w	zmm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvttph2w	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvttsh2si	edx, xmm30	 #AVX512-FP16
	vcvttsh2si	edx, xmm30{sae}	 #AVX512-FP16 HAS_SAE
	vcvttsh2si	r12, xmm30	 #AVX512-FP16
	vcvttsh2si	r12, xmm30{sae}	 #AVX512-FP16 HAS_SAE
	vcvttsh2si	edx, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16
	vcvttsh2si	edx, WORD PTR [r9]	 #AVX512-FP16
	vcvttsh2si	edx, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vcvttsh2si	edx, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80)
	vcvttsh2si	r12, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16
	vcvttsh2si	r12, WORD PTR [r9]	 #AVX512-FP16
	vcvttsh2si	r12, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vcvttsh2si	r12, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80)
	vcvttsh2usi	edx, xmm30	 #AVX512-FP16
	vcvttsh2usi	edx, xmm30{sae}	 #AVX512-FP16 HAS_SAE
	vcvttsh2usi	r12, xmm30	 #AVX512-FP16
	vcvttsh2usi	r12, xmm30{sae}	 #AVX512-FP16 HAS_SAE
	vcvttsh2usi	edx, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16
	vcvttsh2usi	edx, WORD PTR [r9]	 #AVX512-FP16
	vcvttsh2usi	edx, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vcvttsh2usi	edx, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80)
	vcvttsh2usi	r12, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16
	vcvttsh2usi	r12, WORD PTR [r9]	 #AVX512-FP16
	vcvttsh2usi	r12, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vcvttsh2usi	r12, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80)
	vcvtudq2ph	ymm30, zmm29	 #AVX512-FP16
	vcvtudq2ph	ymm30, zmm29{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtudq2ph	ymm30{k7}{z}, zmm29{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtudq2ph	ymm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtudq2ph	ymm30, DWORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvtudq2ph	ymm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtudq2ph	ymm30{k7}{z}, DWORD BCST [rdx-512]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtuqq2ph	xmm30, zmm29	 #AVX512-FP16
	vcvtuqq2ph	xmm30, zmm29{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtuqq2ph	xmm30{k7}{z}, zmm29{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtuqq2ph	xmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtuqq2ph	xmm30, QWORD BCST [r9]{1to8}	 #AVX512-FP16 BROADCAST_EN
	vcvtuqq2ph	xmm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtuqq2ph	xmm30{k7}{z}, QWORD BCST [rdx-1024]{1to8}	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtusi2sh	xmm30, xmm29, r12	 #AVX512-FP16
	vcvtusi2sh	xmm30, xmm29, r12{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtusi2sh	xmm30, xmm29, edx	 #AVX512-FP16
	vcvtusi2sh	xmm30, xmm29, edx{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtusi2sh	xmm30, xmm29, DWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16
	vcvtusi2sh	xmm30, xmm29, DWORD PTR [r9]	 #AVX512-FP16
	vcvtusi2sh	xmm30, xmm29, DWORD PTR [rcx+508]	 #AVX512-FP16 Disp8(7f)
	vcvtusi2sh	xmm30, xmm29, DWORD PTR [rdx-512]	 #AVX512-FP16 Disp8(80)
	vcvtusi2sh	xmm30, xmm29, QWORD PTR [rcx+1016]	 #AVX512-FP16 Disp8(7f)
	vcvtusi2sh	xmm30, xmm29, QWORD PTR [rdx-1024]	 #AVX512-FP16 Disp8(80)
	vcvtuw2ph	zmm30, zmm29	 #AVX512-FP16
	vcvtuw2ph	zmm30, zmm29{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtuw2ph	zmm30{k7}{z}, zmm29{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtuw2ph	zmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtuw2ph	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvtuw2ph	zmm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtuw2ph	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vcvtw2ph	zmm30, zmm29	 #AVX512-FP16
	vcvtw2ph	zmm30, zmm29{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vcvtw2ph	zmm30{k7}{z}, zmm29{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vcvtw2ph	zmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vcvtw2ph	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vcvtw2ph	zmm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vcvtw2ph	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vdivph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vdivph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vdivph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vdivph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vdivph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vdivph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vdivph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vdivsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vdivsh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vdivsh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vdivsh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vdivsh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vdivsh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vdivsh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfcmaddcph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfcmaddcph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmaddcph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmaddcph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfcmaddcph	zmm30, zmm29, DWORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfcmaddcph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfcmaddcph	zmm30{k7}{z}, zmm29, DWORD BCST [rdx-512]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmaddcsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfcmaddcsh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmaddcsh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmaddcsh	xmm30{k7}, xmm29, DWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfcmaddcsh	xmm30, xmm29, DWORD PTR [r9]	 #AVX512-FP16
	vfcmaddcsh	xmm30, xmm29, DWORD PTR [rcx+508]	 #AVX512-FP16 Disp8(7f)
	vfcmaddcsh	xmm30{k7}{z}, xmm29, DWORD PTR [rdx-512]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfcmulcph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfcmulcph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmulcph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmulcph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfcmulcph	zmm30, zmm29, DWORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfcmulcph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfcmulcph	zmm30{k7}{z}, zmm29, DWORD BCST [rdx-512]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfcmulcsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfcmulcsh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfcmulcsh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfcmulcsh	xmm30{k7}, xmm29, DWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfcmulcsh	xmm30, xmm29, DWORD PTR [r9]	 #AVX512-FP16
	vfcmulcsh	xmm30, xmm29, DWORD PTR [rcx+508]	 #AVX512-FP16 Disp8(7f)
	vfcmulcsh	xmm30{k7}{z}, xmm29, DWORD PTR [rdx-512]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd132ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfmadd132ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd132ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd132ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmadd132ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfmadd132ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmadd132ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd132sh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfmadd132sh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd132sh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd132sh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmadd132sh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vfmadd132sh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vfmadd132sh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd213ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfmadd213ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd213ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd213ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmadd213ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfmadd213ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmadd213ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd213sh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfmadd213sh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd213sh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd213sh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmadd213sh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vfmadd213sh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vfmadd213sh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd231ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfmadd231ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd231ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd231ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmadd231ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfmadd231ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmadd231ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmadd231sh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfmadd231sh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmadd231sh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmadd231sh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmadd231sh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vfmadd231sh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vfmadd231sh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddcph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfmaddcph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddcph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddcph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmaddcph	zmm30, zmm29, DWORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfmaddcph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmaddcph	zmm30{k7}{z}, zmm29, DWORD BCST [rdx-512]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddcsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfmaddcsh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddcsh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddcsh	xmm30{k7}, xmm29, DWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmaddcsh	xmm30, xmm29, DWORD PTR [r9]	 #AVX512-FP16
	vfmaddcsh	xmm30, xmm29, DWORD PTR [rcx+508]	 #AVX512-FP16 Disp8(7f)
	vfmaddcsh	xmm30{k7}{z}, xmm29, DWORD PTR [rdx-512]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub132ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfmaddsub132ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddsub132ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddsub132ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmaddsub132ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfmaddsub132ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmaddsub132ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub213ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfmaddsub213ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddsub213ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddsub213ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmaddsub213ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfmaddsub213ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmaddsub213ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmaddsub231ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfmaddsub231ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmaddsub231ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmaddsub231ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmaddsub231ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfmaddsub231ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmaddsub231ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub132ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfmsub132ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub132ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub132ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsub132ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfmsub132ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmsub132ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub132sh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfmsub132sh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub132sh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub132sh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsub132sh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vfmsub132sh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vfmsub132sh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub213ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfmsub213ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub213ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub213ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsub213ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfmsub213ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmsub213ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub213sh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfmsub213sh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub213sh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub213sh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsub213sh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vfmsub213sh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vfmsub213sh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub231ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfmsub231ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub231ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub231ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsub231ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfmsub231ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmsub231ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsub231sh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfmsub231sh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsub231sh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsub231sh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsub231sh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vfmsub231sh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vfmsub231sh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd132ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfmsubadd132ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsubadd132ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsubadd132ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsubadd132ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfmsubadd132ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmsubadd132ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd213ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfmsubadd213ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsubadd213ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsubadd213ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsubadd213ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfmsubadd213ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmsubadd213ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmsubadd231ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfmsubadd231ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmsubadd231ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmsubadd231ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmsubadd231ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfmsubadd231ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmsubadd231ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmulcph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfmulcph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmulcph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmulcph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmulcph	zmm30, zmm29, DWORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfmulcph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfmulcph	zmm30{k7}{z}, zmm29, DWORD BCST [rdx-512]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfmulcsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfmulcsh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfmulcsh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfmulcsh	xmm30{k7}, xmm29, DWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfmulcsh	xmm30, xmm29, DWORD PTR [r9]	 #AVX512-FP16
	vfmulcsh	xmm30, xmm29, DWORD PTR [rcx+508]	 #AVX512-FP16 Disp8(7f)
	vfmulcsh	xmm30{k7}{z}, xmm29, DWORD PTR [rdx-512]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd132ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfnmadd132ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd132ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd132ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmadd132ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfnmadd132ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfnmadd132ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd132sh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfnmadd132sh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd132sh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd132sh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmadd132sh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vfnmadd132sh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vfnmadd132sh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd213ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfnmadd213ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd213ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd213ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmadd213ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfnmadd213ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfnmadd213ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd213sh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfnmadd213sh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd213sh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd213sh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmadd213sh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vfnmadd213sh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vfnmadd213sh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd231ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfnmadd231ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd231ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd231ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmadd231ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfnmadd231ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfnmadd231ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmadd231sh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfnmadd231sh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmadd231sh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmadd231sh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmadd231sh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vfnmadd231sh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vfnmadd231sh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub132ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfnmsub132ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub132ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub132ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmsub132ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfnmsub132ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfnmsub132ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub132sh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfnmsub132sh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub132sh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub132sh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmsub132sh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vfnmsub132sh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vfnmsub132sh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub213ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfnmsub213ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub213ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub213ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmsub213ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfnmsub213ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfnmsub213ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub213sh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfnmsub213sh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub213sh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub213sh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmsub213sh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vfnmsub213sh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vfnmsub213sh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub231ph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vfnmsub231ph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub231ph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub231ph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmsub231ph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vfnmsub231ph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vfnmsub231ph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vfnmsub231sh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vfnmsub231sh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vfnmsub231sh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vfnmsub231sh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vfnmsub231sh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vfnmsub231sh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vfnmsub231sh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vfpclassph	k5, zmm30, 123	 #AVX512-FP16
	vfpclassph	k5{k7}, zmm30, 123	 #AVX512-FP16 MASK_ENABLING
	vfpclassph	k5{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vfpclassph	k5, WORD BCST [r9]{1to32}, 123	 #AVX512-FP16 BROADCAST_EN
	vfpclassph	k5, ZMMWORD PTR [rcx+8128], 123	 #AVX512-FP16 Disp8(7f)
	vfpclassph	k5{k7}, WORD BCST [rdx-256]{1to32}, 123	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING
	vfpclasssh	k5, xmm30, 123	 #AVX512-FP16
	vfpclasssh	k5{k7}, xmm30, 123	 #AVX512-FP16 MASK_ENABLING
	vfpclasssh	k5{k7}, WORD PTR [rbp+r14*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vfpclasssh	k5, WORD PTR [r9], 123	 #AVX512-FP16
	vfpclasssh	k5, WORD PTR [rcx+254], 123	 #AVX512-FP16 Disp8(7f)
	vfpclasssh	k5{k7}, WORD PTR [rdx-256], 123	 #AVX512-FP16 Disp8(80) MASK_ENABLING
	vgetexpph	zmm30, zmm29	 #AVX512-FP16
	vgetexpph	zmm30, zmm29{sae}	 #AVX512-FP16 HAS_SAE
	vgetexpph	zmm30{k7}{z}, zmm29{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetexpph	zmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vgetexpph	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vgetexpph	zmm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vgetexpph	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vgetexpsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vgetexpsh	xmm30, xmm29, xmm28{sae}	 #AVX512-FP16 HAS_SAE
	vgetexpsh	xmm30{k7}{z}, xmm29, xmm28{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetexpsh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vgetexpsh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vgetexpsh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vgetexpsh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vgetmantph	zmm30, zmm29, 123	 #AVX512-FP16
	vgetmantph	zmm30, zmm29{sae}, 123	 #AVX512-FP16 HAS_SAE
	vgetmantph	zmm30{k7}{z}, zmm29{sae}, 123	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetmantph	zmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vgetmantph	zmm30, WORD BCST [r9], 123	 #AVX512-FP16 BROADCAST_EN
	vgetmantph	zmm30, ZMMWORD PTR [rcx+8128], 123	 #AVX512-FP16 Disp8(7f)
	vgetmantph	zmm30{k7}{z}, WORD BCST [rdx-256], 123	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vgetmantsh	xmm30, xmm29, xmm28, 123	 #AVX512-FP16
	vgetmantsh	xmm30, xmm29, xmm28{sae}, 123	 #AVX512-FP16 HAS_SAE
	vgetmantsh	xmm30{k7}{z}, xmm29, xmm28{sae}, 123	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vgetmantsh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vgetmantsh	xmm30, xmm29, WORD PTR [r9], 123	 #AVX512-FP16
	vgetmantsh	xmm30, xmm29, WORD PTR [rcx+254], 123	 #AVX512-FP16 Disp8(7f)
	vgetmantsh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256], 123	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vmaxph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vmaxph	zmm30, zmm29, zmm28{sae}	 #AVX512-FP16 HAS_SAE
	vmaxph	zmm30{k7}{z}, zmm29, zmm28{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vmaxph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vmaxph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vmaxph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vmaxph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmaxsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vmaxsh	xmm30, xmm29, xmm28{sae}	 #AVX512-FP16 HAS_SAE
	vmaxsh	xmm30{k7}{z}, xmm29, xmm28{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vmaxsh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vmaxsh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vmaxsh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vmaxsh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vminph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vminph	zmm30, zmm29, zmm28{sae}	 #AVX512-FP16 HAS_SAE
	vminph	zmm30{k7}{z}, zmm29, zmm28{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vminph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vminph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vminph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vminph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vminsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vminsh	xmm30, xmm29, xmm28{sae}	 #AVX512-FP16 HAS_SAE
	vminsh	xmm30{k7}{z}, xmm29, xmm28{sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vminsh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vminsh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vminsh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vminsh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vmovsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vmovsh	xmm30{k7}{z}, xmm29, xmm28	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vmovsh	xmm30{k7}, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vmovsh	xmm30, WORD PTR [r9]	 #AVX512-FP16
	vmovsh	xmm30, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vmovsh	xmm30{k7}{z}, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vmovsh	WORD PTR [rbp+r14*8+0x10000000]{k7}, xmm30	 #AVX512-FP16 MASK_ENABLING
	vmovsh	WORD PTR [r9], xmm30	 #AVX512-FP16
	vmovsh	WORD PTR [rcx+254], xmm30	 #AVX512-FP16 Disp8(7f)
	vmovsh	WORD PTR [rdx-256]{k7}, xmm30	 #AVX512-FP16 Disp8(80) MASK_ENABLING
	vmovw	xmm30, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16
	vmovw	xmm30, WORD PTR [r9]	 #AVX512-FP16
	vmovw	xmm30, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vmovw	xmm30, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80)
	vmovw	WORD PTR [rbp+r14*8+0x10000000], xmm30	 #AVX512-FP16
	vmovw	WORD PTR [r9], xmm30	 #AVX512-FP16
	vmovw	WORD PTR [rcx+254], xmm30	 #AVX512-FP16 Disp8(7f)
	vmovw	WORD PTR [rdx-256], xmm30	 #AVX512-FP16 Disp8(80)
	vmulph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vmulph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vmulph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vmulph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vmulph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vmulph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vmulph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vmulsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vmulsh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vmulsh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vmulsh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vmulsh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vmulsh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vmulsh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vrcpph	zmm30, zmm29	 #AVX512-FP16
	vrcpph	zmm30{k7}{z}, zmm29	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrcpph	zmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vrcpph	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vrcpph	zmm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vrcpph	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrcpsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vrcpsh	xmm30{k7}{z}, xmm29, xmm28	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrcpsh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vrcpsh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vrcpsh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vrcpsh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vreduceph	zmm30, zmm29, 123	 #AVX512-FP16
	vreduceph	zmm30, zmm29{sae}, 123	 #AVX512-FP16 HAS_SAE
	vreduceph	zmm30{k7}{z}, zmm29{sae}, 123	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vreduceph	zmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vreduceph	zmm30, WORD BCST [r9], 123	 #AVX512-FP16 BROADCAST_EN
	vreduceph	zmm30, ZMMWORD PTR [rcx+8128], 123	 #AVX512-FP16 Disp8(7f)
	vreduceph	zmm30{k7}{z}, WORD BCST [rdx-256], 123	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vreducesh	xmm30, xmm29, xmm28, 123	 #AVX512-FP16
	vreducesh	xmm30, xmm29, xmm28{sae}, 123	 #AVX512-FP16 HAS_SAE
	vreducesh	xmm30{k7}{z}, xmm29, xmm28{sae}, 123	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vreducesh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vreducesh	xmm30, xmm29, WORD PTR [r9], 123	 #AVX512-FP16
	vreducesh	xmm30, xmm29, WORD PTR [rcx+254], 123	 #AVX512-FP16 Disp8(7f)
	vreducesh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256], 123	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vrndscaleph	zmm30, zmm29, 123	 #AVX512-FP16
	vrndscaleph	zmm30, zmm29{sae}, 123	 #AVX512-FP16 HAS_SAE
	vrndscaleph	zmm30{k7}{z}, zmm29{sae}, 123	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vrndscaleph	zmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vrndscaleph	zmm30, WORD BCST [r9], 123	 #AVX512-FP16 BROADCAST_EN
	vrndscaleph	zmm30, ZMMWORD PTR [rcx+8128], 123	 #AVX512-FP16 Disp8(7f)
	vrndscaleph	zmm30{k7}{z}, WORD BCST [rdx-256], 123	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrndscalesh	xmm30, xmm29, xmm28, 123	 #AVX512-FP16
	vrndscalesh	xmm30, xmm29, xmm28{sae}, 123	 #AVX512-FP16 HAS_SAE
	vrndscalesh	xmm30{k7}{z}, xmm29, xmm28{sae}, 123	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE
	vrndscalesh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000], 123	 #AVX512-FP16 MASK_ENABLING
	vrndscalesh	xmm30, xmm29, WORD PTR [r9], 123	 #AVX512-FP16
	vrndscalesh	xmm30, xmm29, WORD PTR [rcx+254], 123	 #AVX512-FP16 Disp8(7f)
	vrndscalesh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256], 123	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vrsqrtph	zmm30, zmm29	 #AVX512-FP16
	vrsqrtph	zmm30{k7}{z}, zmm29	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrsqrtph	zmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vrsqrtph	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vrsqrtph	zmm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vrsqrtph	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vrsqrtsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vrsqrtsh	xmm30{k7}{z}, xmm29, xmm28	 #AVX512-FP16 MASK_ENABLING ZEROCTL
	vrsqrtsh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vrsqrtsh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vrsqrtsh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vrsqrtsh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vscalefph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vscalefph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vscalefph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vscalefph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vscalefph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vscalefph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vscalefph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vscalefsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vscalefsh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vscalefsh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vscalefsh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vscalefsh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vscalefsh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vscalefsh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vsqrtph	zmm30, zmm29	 #AVX512-FP16
	vsqrtph	zmm30, zmm29{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsqrtph	zmm30{k7}{z}, zmm29{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsqrtph	zmm30{k7}, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vsqrtph	zmm30, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vsqrtph	zmm30, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vsqrtph	zmm30{k7}{z}, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsqrtsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vsqrtsh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsqrtsh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsqrtsh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vsqrtsh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vsqrtsh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vsqrtsh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vsubph	zmm30, zmm29, zmm28	 #AVX512-FP16
	vsubph	zmm30, zmm29, zmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsubph	zmm30{k7}{z}, zmm29, zmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsubph	zmm30{k7}, zmm29, ZMMWORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vsubph	zmm30, zmm29, WORD BCST [r9]	 #AVX512-FP16 BROADCAST_EN
	vsubph	zmm30, zmm29, ZMMWORD PTR [rcx+8128]	 #AVX512-FP16 Disp8(7f)
	vsubph	zmm30{k7}{z}, zmm29, WORD BCST [rdx-256]	 #AVX512-FP16 BROADCAST_EN Disp8(80) MASK_ENABLING ZEROCTL
	vsubsh	xmm30, xmm29, xmm28	 #AVX512-FP16
	vsubsh	xmm30, xmm29, xmm28{rn-sae}	 #AVX512-FP16 HAS_SAE RC_CTRL
	vsubsh	xmm30{k7}{z}, xmm29, xmm28{rn-sae}	 #AVX512-FP16 MASK_ENABLING ZEROCTL HAS_SAE RC_CTRL
	vsubsh	xmm30{k7}, xmm29, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16 MASK_ENABLING
	vsubsh	xmm30, xmm29, WORD PTR [r9]	 #AVX512-FP16
	vsubsh	xmm30, xmm29, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vsubsh	xmm30{k7}{z}, xmm29, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80) MASK_ENABLING ZEROCTL
	vucomish	xmm30, xmm29	 #AVX512-FP16
	vucomish	xmm30, xmm29{sae}	 #AVX512-FP16 HAS_SAE
	vucomish	xmm30, WORD PTR [rbp+r14*8+0x10000000]	 #AVX512-FP16
	vucomish	xmm30, WORD PTR [r9]	 #AVX512-FP16
	vucomish	xmm30, WORD PTR [rcx+254]	 #AVX512-FP16 Disp8(7f)
	vucomish	xmm30, WORD PTR [rdx-256]	 #AVX512-FP16 Disp8(80)
