# Check 32bit AVX512DQ instructions

	.allow_index_reg
	.text
_start:
	vbroadcastf32x8	(%ecx), %zmm6	 # AVX512DQ
	vbroadcastf32x8	(%ecx), %zmm6{%k7}	 # AVX512DQ
	vbroadcastf32x8	(%ecx), %zmm6{%k7}{z}	 # AVX512DQ
	vbroadcastf32x8	-123456(%esp,%esi,8), %zmm6	 # AVX512DQ
	vbroadcastf32x8	4064(%edx), %zmm6	 # AVX512DQ Disp8
	vbroadcastf32x8	4096(%edx), %zmm6	 # AVX512DQ
	vbroadcastf32x8	-4096(%edx), %zmm6	 # AVX512DQ Disp8
	vbroadcastf32x8	-4128(%edx), %zmm6	 # AVX512DQ
	vbroadcastf64x2	(%ecx), %zmm6	 # AVX512DQ
	vbroadcastf64x2	(%ecx), %zmm6{%k7}	 # AVX512DQ
	vbroadcastf64x2	(%ecx), %zmm6{%k7}{z}	 # AVX512DQ
	vbroadcastf64x2	-123456(%esp,%esi,8), %zmm6	 # AVX512DQ
	vbroadcastf64x2	2032(%edx), %zmm6	 # AVX512DQ Disp8
	vbroadcastf64x2	2048(%edx), %zmm6	 # AVX512DQ
	vbroadcastf64x2	-2048(%edx), %zmm6	 # AVX512DQ Disp8
	vbroadcastf64x2	-2064(%edx), %zmm6	 # AVX512DQ
	vbroadcasti32x8	(%ecx), %zmm6	 # AVX512DQ
	vbroadcasti32x8	(%ecx), %zmm6{%k7}	 # AVX512DQ
	vbroadcasti32x8	(%ecx), %zmm6{%k7}{z}	 # AVX512DQ
	vbroadcasti32x8	-123456(%esp,%esi,8), %zmm6	 # AVX512DQ
	vbroadcasti32x8	4064(%edx), %zmm6	 # AVX512DQ Disp8
	vbroadcasti32x8	4096(%edx), %zmm6	 # AVX512DQ
	vbroadcasti32x8	-4096(%edx), %zmm6	 # AVX512DQ Disp8
	vbroadcasti32x8	-4128(%edx), %zmm6	 # AVX512DQ
	vbroadcasti64x2	(%ecx), %zmm6	 # AVX512DQ
	vbroadcasti64x2	(%ecx), %zmm6{%k7}	 # AVX512DQ
	vbroadcasti64x2	(%ecx), %zmm6{%k7}{z}	 # AVX512DQ
	vbroadcasti64x2	-123456(%esp,%esi,8), %zmm6	 # AVX512DQ
	vbroadcasti64x2	2032(%edx), %zmm6	 # AVX512DQ Disp8
	vbroadcasti64x2	2048(%edx), %zmm6	 # AVX512DQ
	vbroadcasti64x2	-2048(%edx), %zmm6	 # AVX512DQ Disp8
	vbroadcasti64x2	-2064(%edx), %zmm6	 # AVX512DQ
	vbroadcastf32x2	%xmm7, %zmm6	 # AVX512DQ
	vbroadcastf32x2	%xmm7, %zmm6{%k7}	 # AVX512DQ
	vbroadcastf32x2	%xmm7, %zmm6{%k7}{z}	 # AVX512DQ
	vbroadcastf32x2	(%ecx), %zmm6	 # AVX512DQ
	vbroadcastf32x2	-123456(%esp,%esi,8), %zmm6	 # AVX512DQ
	vbroadcastf32x2	1016(%edx), %zmm6	 # AVX512DQ Disp8
	vbroadcastf32x2	1024(%edx), %zmm6	 # AVX512DQ
	vbroadcastf32x2	-1024(%edx), %zmm6	 # AVX512DQ Disp8
	vbroadcastf32x2	-1032(%edx), %zmm6	 # AVX512DQ
	vcvtpd2qq	%zmm5, %zmm6	 # AVX512DQ
	vcvtpd2qq	%zmm5, %zmm6{%k7}	 # AVX512DQ
	vcvtpd2qq	%zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vcvtpd2qq	{rn-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtpd2qq	{ru-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtpd2qq	{rd-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtpd2qq	{rz-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtpd2qq	(%ecx), %zmm6	 # AVX512DQ
	vcvtpd2qq	-123456(%esp,%esi,8), %zmm6	 # AVX512DQ
	vcvtpd2qq	(%eax){1to8}, %zmm6	 # AVX512DQ
	vcvtpd2qq	8128(%edx), %zmm6	 # AVX512DQ Disp8
	vcvtpd2qq	8192(%edx), %zmm6	 # AVX512DQ
	vcvtpd2qq	-8192(%edx), %zmm6	 # AVX512DQ Disp8
	vcvtpd2qq	-8256(%edx), %zmm6	 # AVX512DQ
	vcvtpd2qq	1016(%edx){1to8}, %zmm6	 # AVX512DQ Disp8
	vcvtpd2qq	1024(%edx){1to8}, %zmm6	 # AVX512DQ
	vcvtpd2qq	-1024(%edx){1to8}, %zmm6	 # AVX512DQ Disp8
	vcvtpd2qq	-1032(%edx){1to8}, %zmm6	 # AVX512DQ
	vcvtpd2uqq	%zmm5, %zmm6	 # AVX512DQ
	vcvtpd2uqq	%zmm5, %zmm6{%k7}	 # AVX512DQ
	vcvtpd2uqq	%zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vcvtpd2uqq	{rn-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtpd2uqq	{ru-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtpd2uqq	{rd-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtpd2uqq	{rz-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtpd2uqq	(%ecx), %zmm6	 # AVX512DQ
	vcvtpd2uqq	-123456(%esp,%esi,8), %zmm6	 # AVX512DQ
	vcvtpd2uqq	(%eax){1to8}, %zmm6	 # AVX512DQ
	vcvtpd2uqq	8128(%edx), %zmm6	 # AVX512DQ Disp8
	vcvtpd2uqq	8192(%edx), %zmm6	 # AVX512DQ
	vcvtpd2uqq	-8192(%edx), %zmm6	 # AVX512DQ Disp8
	vcvtpd2uqq	-8256(%edx), %zmm6	 # AVX512DQ
	vcvtpd2uqq	1016(%edx){1to8}, %zmm6	 # AVX512DQ Disp8
	vcvtpd2uqq	1024(%edx){1to8}, %zmm6	 # AVX512DQ
	vcvtpd2uqq	-1024(%edx){1to8}, %zmm6	 # AVX512DQ Disp8
	vcvtpd2uqq	-1032(%edx){1to8}, %zmm6	 # AVX512DQ
	vcvtps2qq	%ymm5, %zmm6{%k7}	 # AVX512DQ
	vcvtps2qq	%ymm5, %zmm6{%k7}{z}	 # AVX512DQ
	vcvtps2qq	{rn-sae}, %ymm5, %zmm6{%k7}	 # AVX512DQ
	vcvtps2qq	{ru-sae}, %ymm5, %zmm6{%k7}	 # AVX512DQ
	vcvtps2qq	{rd-sae}, %ymm5, %zmm6{%k7}	 # AVX512DQ
	vcvtps2qq	{rz-sae}, %ymm5, %zmm6{%k7}	 # AVX512DQ
	vcvtps2qq	(%ecx), %zmm6{%k7}	 # AVX512DQ
	vcvtps2qq	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512DQ
	vcvtps2qq	(%eax){1to8}, %zmm6{%k7}	 # AVX512DQ
	vcvtps2qq	4064(%edx), %zmm6{%k7}	 # AVX512DQ Disp8
	vcvtps2qq	4096(%edx), %zmm6{%k7}	 # AVX512DQ
	vcvtps2qq	-4096(%edx), %zmm6{%k7}	 # AVX512DQ Disp8
	vcvtps2qq	-4128(%edx), %zmm6{%k7}	 # AVX512DQ
	vcvtps2qq	508(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ Disp8
	vcvtps2qq	512(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ
	vcvtps2qq	-512(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ Disp8
	vcvtps2qq	-516(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ
	vcvtps2uqq	%ymm5, %zmm6{%k7}	 # AVX512DQ
	vcvtps2uqq	%ymm5, %zmm6{%k7}{z}	 # AVX512DQ
	vcvtps2uqq	{rn-sae}, %ymm5, %zmm6{%k7}	 # AVX512DQ
	vcvtps2uqq	{ru-sae}, %ymm5, %zmm6{%k7}	 # AVX512DQ
	vcvtps2uqq	{rd-sae}, %ymm5, %zmm6{%k7}	 # AVX512DQ
	vcvtps2uqq	{rz-sae}, %ymm5, %zmm6{%k7}	 # AVX512DQ
	vcvtps2uqq	(%ecx), %zmm6{%k7}	 # AVX512DQ
	vcvtps2uqq	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512DQ
	vcvtps2uqq	(%eax){1to8}, %zmm6{%k7}	 # AVX512DQ
	vcvtps2uqq	4064(%edx), %zmm6{%k7}	 # AVX512DQ Disp8
	vcvtps2uqq	4096(%edx), %zmm6{%k7}	 # AVX512DQ
	vcvtps2uqq	-4096(%edx), %zmm6{%k7}	 # AVX512DQ Disp8
	vcvtps2uqq	-4128(%edx), %zmm6{%k7}	 # AVX512DQ
	vcvtps2uqq	508(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ Disp8
	vcvtps2uqq	512(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ
	vcvtps2uqq	-512(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ Disp8
	vcvtps2uqq	-516(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ
	vcvtqq2pd	%zmm5, %zmm6	 # AVX512DQ
	vcvtqq2pd	%zmm5, %zmm6{%k7}	 # AVX512DQ
	vcvtqq2pd	%zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vcvtqq2pd	{rn-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtqq2pd	{ru-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtqq2pd	{rd-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtqq2pd	{rz-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtqq2pd	(%ecx), %zmm6	 # AVX512DQ
	vcvtqq2pd	-123456(%esp,%esi,8), %zmm6	 # AVX512DQ
	vcvtqq2pd	(%eax){1to8}, %zmm6	 # AVX512DQ
	vcvtqq2pd	8128(%edx), %zmm6	 # AVX512DQ Disp8
	vcvtqq2pd	8192(%edx), %zmm6	 # AVX512DQ
	vcvtqq2pd	-8192(%edx), %zmm6	 # AVX512DQ Disp8
	vcvtqq2pd	-8256(%edx), %zmm6	 # AVX512DQ
	vcvtqq2pd	1016(%edx){1to8}, %zmm6	 # AVX512DQ Disp8
	vcvtqq2pd	1024(%edx){1to8}, %zmm6	 # AVX512DQ
	vcvtqq2pd	-1024(%edx){1to8}, %zmm6	 # AVX512DQ Disp8
	vcvtqq2pd	-1032(%edx){1to8}, %zmm6	 # AVX512DQ
	vcvtqq2ps	%zmm5, %ymm6{%k7}	 # AVX512DQ
	vcvtqq2ps	%zmm5, %ymm6{%k7}{z}	 # AVX512DQ
	vcvtqq2ps	{rn-sae}, %zmm5, %ymm6{%k7}	 # AVX512DQ
	vcvtqq2ps	{ru-sae}, %zmm5, %ymm6{%k7}	 # AVX512DQ
	vcvtqq2ps	{rd-sae}, %zmm5, %ymm6{%k7}	 # AVX512DQ
	vcvtqq2ps	{rz-sae}, %zmm5, %ymm6{%k7}	 # AVX512DQ
	vcvtqq2ps	(%ecx), %ymm6{%k7}	 # AVX512DQ
	vcvtqq2ps	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512DQ
	vcvtqq2ps	(%eax){1to8}, %ymm6{%k7}	 # AVX512DQ
	vcvtqq2ps	8128(%edx), %ymm6{%k7}	 # AVX512DQ Disp8
	vcvtqq2ps	8192(%edx), %ymm6{%k7}	 # AVX512DQ
	vcvtqq2ps	-8192(%edx), %ymm6{%k7}	 # AVX512DQ Disp8
	vcvtqq2ps	-8256(%edx), %ymm6{%k7}	 # AVX512DQ
	vcvtqq2ps	1016(%edx){1to8}, %ymm6{%k7}	 # AVX512DQ Disp8
	vcvtqq2ps	1024(%edx){1to8}, %ymm6{%k7}	 # AVX512DQ
	vcvtqq2ps	-1024(%edx){1to8}, %ymm6{%k7}	 # AVX512DQ Disp8
	vcvtqq2ps	-1032(%edx){1to8}, %ymm6{%k7}	 # AVX512DQ
	vcvtuqq2pd	%zmm5, %zmm6	 # AVX512DQ
	vcvtuqq2pd	%zmm5, %zmm6{%k7}	 # AVX512DQ
	vcvtuqq2pd	%zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vcvtuqq2pd	{rn-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtuqq2pd	{ru-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtuqq2pd	{rd-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtuqq2pd	{rz-sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvtuqq2pd	(%ecx), %zmm6	 # AVX512DQ
	vcvtuqq2pd	-123456(%esp,%esi,8), %zmm6	 # AVX512DQ
	vcvtuqq2pd	(%eax){1to8}, %zmm6	 # AVX512DQ
	vcvtuqq2pd	8128(%edx), %zmm6	 # AVX512DQ Disp8
	vcvtuqq2pd	8192(%edx), %zmm6	 # AVX512DQ
	vcvtuqq2pd	-8192(%edx), %zmm6	 # AVX512DQ Disp8
	vcvtuqq2pd	-8256(%edx), %zmm6	 # AVX512DQ
	vcvtuqq2pd	1016(%edx){1to8}, %zmm6	 # AVX512DQ Disp8
	vcvtuqq2pd	1024(%edx){1to8}, %zmm6	 # AVX512DQ
	vcvtuqq2pd	-1024(%edx){1to8}, %zmm6	 # AVX512DQ Disp8
	vcvtuqq2pd	-1032(%edx){1to8}, %zmm6	 # AVX512DQ
	vcvtuqq2ps	%zmm5, %ymm6{%k7}	 # AVX512DQ
	vcvtuqq2ps	%zmm5, %ymm6{%k7}{z}	 # AVX512DQ
	vcvtuqq2ps	{rn-sae}, %zmm5, %ymm6{%k7}	 # AVX512DQ
	vcvtuqq2ps	{ru-sae}, %zmm5, %ymm6{%k7}	 # AVX512DQ
	vcvtuqq2ps	{rd-sae}, %zmm5, %ymm6{%k7}	 # AVX512DQ
	vcvtuqq2ps	{rz-sae}, %zmm5, %ymm6{%k7}	 # AVX512DQ
	vcvtuqq2ps	(%ecx), %ymm6{%k7}	 # AVX512DQ
	vcvtuqq2ps	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512DQ
	vcvtuqq2ps	(%eax){1to8}, %ymm6{%k7}	 # AVX512DQ
	vcvtuqq2ps	8128(%edx), %ymm6{%k7}	 # AVX512DQ Disp8
	vcvtuqq2ps	8192(%edx), %ymm6{%k7}	 # AVX512DQ
	vcvtuqq2ps	-8192(%edx), %ymm6{%k7}	 # AVX512DQ Disp8
	vcvtuqq2ps	-8256(%edx), %ymm6{%k7}	 # AVX512DQ
	vcvtuqq2ps	1016(%edx){1to8}, %ymm6{%k7}	 # AVX512DQ Disp8
	vcvtuqq2ps	1024(%edx){1to8}, %ymm6{%k7}	 # AVX512DQ
	vcvtuqq2ps	-1024(%edx){1to8}, %ymm6{%k7}	 # AVX512DQ Disp8
	vcvtuqq2ps	-1032(%edx){1to8}, %ymm6{%k7}	 # AVX512DQ
	vextractf64x2	$0xab, %zmm5, %xmm6{%k7}	 # AVX512DQ
	vextractf64x2	$0xab, %zmm5, %xmm6{%k7}{z}	 # AVX512DQ
	vextractf64x2	$123, %zmm5, %xmm6{%k7}	 # AVX512DQ
	vextractf32x8	$0xab, %zmm5, %ymm6{%k7}	 # AVX512DQ
	vextractf32x8	$0xab, %zmm5, %ymm6{%k7}{z}	 # AVX512DQ
	vextractf32x8	$123, %zmm5, %ymm6{%k7}	 # AVX512DQ
	vextracti64x2	$0xab, %zmm5, %xmm6{%k7}	 # AVX512DQ
	vextracti64x2	$0xab, %zmm5, %xmm6{%k7}{z}	 # AVX512DQ
	vextracti64x2	$123, %zmm5, %xmm6{%k7}	 # AVX512DQ
	vextracti32x8	$0xab, %zmm5, %ymm6{%k7}	 # AVX512DQ
	vextracti32x8	$0xab, %zmm5, %ymm6{%k7}{z}	 # AVX512DQ
	vextracti32x8	$123, %zmm5, %ymm6{%k7}	 # AVX512DQ
	vfpclasspd	$0xab, %zmm6, %k5	 # AVX512DQ
	vfpclasspd	$0xab, %zmm6, %k5{%k7}	 # AVX512DQ
	vfpclasspd	$123, %zmm6, %k5	 # AVX512DQ
	vfpclasspdz	$123, (%ecx), %k5	 # AVX512DQ
	vfpclasspdz	$123, -123456(%esp,%esi,8), %k5	 # AVX512DQ
	vfpclasspd	$123, (%eax){1to8}, %k5	 # AVX512DQ
	vfpclasspdz	$123, 8128(%edx), %k5	 # AVX512DQ Disp8
	vfpclasspdz	$123, 8192(%edx), %k5	 # AVX512DQ
	vfpclasspdz	$123, -8192(%edx), %k5	 # AVX512DQ Disp8
	vfpclasspdz	$123, -8256(%edx), %k5	 # AVX512DQ
	vfpclasspdz	$123, 1016(%edx){1to8}, %k5	 # AVX512DQ Disp8
	vfpclasspdz	$123, 1024(%edx){1to8}, %k5	 # AVX512DQ
	vfpclasspdz	$123, -1024(%edx){1to8}, %k5	 # AVX512DQ Disp8
	vfpclasspdz	$123, -1032(%edx){1to8}, %k5	 # AVX512DQ
	vfpclassps	$0xab, %zmm6, %k5	 # AVX512DQ
	vfpclassps	$0xab, %zmm6, %k5{%k7}	 # AVX512DQ
	vfpclassps	$123, %zmm6, %k5	 # AVX512DQ
	vfpclasspsz	$123, (%ecx), %k5	 # AVX512DQ
	vfpclasspsz	$123, -123456(%esp,%esi,8), %k5	 # AVX512DQ
	vfpclassps	$123, (%eax){1to16}, %k5	 # AVX512DQ
	vfpclasspsz	$123, 8128(%edx), %k5	 # AVX512DQ Disp8
	vfpclasspsz	$123, 8192(%edx), %k5	 # AVX512DQ
	vfpclasspsz	$123, -8192(%edx), %k5	 # AVX512DQ Disp8
	vfpclasspsz	$123, -8256(%edx), %k5	 # AVX512DQ
	vfpclasspsz	$123, 508(%edx){1to16}, %k5	 # AVX512DQ Disp8
	vfpclasspsz	$123, 512(%edx){1to16}, %k5	 # AVX512DQ
	vfpclasspsz	$123, -512(%edx){1to16}, %k5	 # AVX512DQ Disp8
	vfpclasspsz	$123, -516(%edx){1to16}, %k5	 # AVX512DQ
	vfpclasssd	$0xab, %xmm6, %k5{%k7}	 # AVX512DQ
	vfpclasssd	$123, %xmm6, %k5{%k7}	 # AVX512DQ
	vfpclasssd	$123, (%ecx), %k5{%k7}	 # AVX512DQ
	vfpclasssd	$123, -123456(%esp,%esi,8), %k5{%k7}	 # AVX512DQ
	vfpclasssd	$123, 1016(%edx), %k5{%k7}	 # AVX512DQ Disp8
	vfpclasssd	$123, 1024(%edx), %k5{%k7}	 # AVX512DQ
	vfpclasssd	$123, -1024(%edx), %k5{%k7}	 # AVX512DQ Disp8
	vfpclasssd	$123, -1032(%edx), %k5{%k7}	 # AVX512DQ
	vfpclassss	$0xab, %xmm6, %k5{%k7}	 # AVX512DQ
	vfpclassss	$123, %xmm6, %k5{%k7}	 # AVX512DQ
	vfpclassss	$123, (%ecx), %k5{%k7}	 # AVX512DQ
	vfpclassss	$123, -123456(%esp,%esi,8), %k5{%k7}	 # AVX512DQ
	vfpclassss	$123, 508(%edx), %k5{%k7}	 # AVX512DQ Disp8
	vfpclassss	$123, 512(%edx), %k5{%k7}	 # AVX512DQ
	vfpclassss	$123, -512(%edx), %k5{%k7}	 # AVX512DQ Disp8
	vfpclassss	$123, -516(%edx), %k5{%k7}	 # AVX512DQ
	vinsertf64x2	$0xab, %xmm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinsertf64x2	$0xab, %xmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vinsertf64x2	$123, %xmm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinsertf64x2	$123, (%ecx), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinsertf64x2	$123, -123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinsertf64x2	$123, 2032(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ Disp8
	vinsertf64x2	$123, 2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinsertf64x2	$123, -2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ Disp8
	vinsertf64x2	$123, -2064(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinsertf32x8	$0xab, %ymm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinsertf32x8	$0xab, %ymm4, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vinsertf32x8	$123, %ymm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinsertf32x8	$123, (%ecx), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinsertf32x8	$123, -123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinsertf32x8	$123, 4064(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ Disp8
	vinsertf32x8	$123, 4096(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinsertf32x8	$123, -4096(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ Disp8
	vinsertf32x8	$123, -4128(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinserti64x2	$0xab, %xmm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinserti64x2	$0xab, %xmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vinserti64x2	$123, %xmm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinserti64x2	$123, (%ecx), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinserti64x2	$123, -123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinserti64x2	$123, 2032(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ Disp8
	vinserti64x2	$123, 2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinserti64x2	$123, -2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ Disp8
	vinserti64x2	$123, -2064(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinserti32x8	$0xab, %ymm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinserti32x8	$0xab, %ymm4, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vinserti32x8	$123, %ymm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinserti32x8	$123, (%ecx), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinserti32x8	$123, -123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinserti32x8	$123, 4064(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ Disp8
	vinserti32x8	$123, 4096(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vinserti32x8	$123, -4096(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ Disp8
	vinserti32x8	$123, -4128(%edx), %zmm5, %zmm6{%k7}	 # AVX512DQ
	vbroadcasti32x2	%xmm7, %zmm6	 # AVX512DQ
	vbroadcasti32x2	%xmm7, %zmm6{%k7}	 # AVX512DQ
	vbroadcasti32x2	%xmm7, %zmm6{%k7}{z}	 # AVX512DQ
	vbroadcasti32x2	(%ecx), %zmm6	 # AVX512DQ
	vbroadcasti32x2	-123456(%esp,%esi,8), %zmm6	 # AVX512DQ
	vbroadcasti32x2	1016(%edx), %zmm6	 # AVX512DQ Disp8
	vbroadcasti32x2	1024(%edx), %zmm6	 # AVX512DQ
	vbroadcasti32x2	-1024(%edx), %zmm6	 # AVX512DQ Disp8
	vbroadcasti32x2	-1032(%edx), %zmm6	 # AVX512DQ
	vpmullq	%zmm4, %zmm5, %zmm6	 # AVX512DQ
	vpmullq	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vpmullq	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vpmullq	(%ecx), %zmm5, %zmm6	 # AVX512DQ
	vpmullq	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512DQ
	vpmullq	(%eax){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vpmullq	8128(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vpmullq	8192(%edx), %zmm5, %zmm6	 # AVX512DQ
	vpmullq	-8192(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vpmullq	-8256(%edx), %zmm5, %zmm6	 # AVX512DQ
	vpmullq	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vpmullq	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vpmullq	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vpmullq	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vrangepd	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512DQ
	vrangepd	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vrangepd	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vrangepd	$0xab, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512DQ
	vrangepd	$123, %zmm4, %zmm5, %zmm6	 # AVX512DQ
	vrangepd	$123, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512DQ
	vrangepd	$123, (%ecx), %zmm5, %zmm6	 # AVX512DQ
	vrangepd	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512DQ
	vrangepd	$123, (%eax){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vrangepd	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vrangepd	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512DQ
	vrangepd	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vrangepd	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512DQ
	vrangepd	$123, 1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vrangepd	$123, 1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vrangepd	$123, -1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vrangepd	$123, -1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vrangeps	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512DQ
	vrangeps	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vrangeps	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vrangeps	$0xab, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512DQ
	vrangeps	$123, %zmm4, %zmm5, %zmm6	 # AVX512DQ
	vrangeps	$123, {sae}, %zmm4, %zmm5, %zmm6	 # AVX512DQ
	vrangeps	$123, (%ecx), %zmm5, %zmm6	 # AVX512DQ
	vrangeps	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512DQ
	vrangeps	$123, (%eax){1to16}, %zmm5, %zmm6	 # AVX512DQ
	vrangeps	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vrangeps	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512DQ
	vrangeps	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vrangeps	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512DQ
	vrangeps	$123, 508(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vrangeps	$123, 512(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ
	vrangeps	$123, -512(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vrangeps	$123, -516(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ
	vrangesd	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangesd	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512DQ
	vrangesd	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangesd	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangesd	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangesd	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangesd	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangesd	$123, 1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ Disp8
	vrangesd	$123, 1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangesd	$123, -1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ Disp8
	vrangesd	$123, -1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangess	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangess	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512DQ
	vrangess	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangess	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangess	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangess	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangess	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangess	$123, 508(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ Disp8
	vrangess	$123, 512(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ
	vrangess	$123, -512(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ Disp8
	vrangess	$123, -516(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ
	vandpd	%zmm4, %zmm5, %zmm6	 # AVX512DQ
	vandpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vandpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vandpd	(%ecx), %zmm5, %zmm6	 # AVX512DQ
	vandpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512DQ
	vandpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vandpd	8128(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vandpd	8192(%edx), %zmm5, %zmm6	 # AVX512DQ
	vandpd	-8192(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vandpd	-8256(%edx), %zmm5, %zmm6	 # AVX512DQ
	vandpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vandpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vandpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vandpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vandps	%zmm4, %zmm5, %zmm6	 # AVX512DQ
	vandps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vandps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vandps	(%ecx), %zmm5, %zmm6	 # AVX512DQ
	vandps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512DQ
	vandps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512DQ
	vandps	8128(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vandps	8192(%edx), %zmm5, %zmm6	 # AVX512DQ
	vandps	-8192(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vandps	-8256(%edx), %zmm5, %zmm6	 # AVX512DQ
	vandps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vandps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ
	vandps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vandps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ
	vandnpd	%zmm4, %zmm5, %zmm6	 # AVX512DQ
	vandnpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vandnpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vandnpd	(%ecx), %zmm5, %zmm6	 # AVX512DQ
	vandnpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512DQ
	vandnpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vandnpd	8128(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vandnpd	8192(%edx), %zmm5, %zmm6	 # AVX512DQ
	vandnpd	-8192(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vandnpd	-8256(%edx), %zmm5, %zmm6	 # AVX512DQ
	vandnpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vandnpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vandnpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vandnpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vandnps	%zmm4, %zmm5, %zmm6	 # AVX512DQ
	vandnps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vandnps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vandnps	(%ecx), %zmm5, %zmm6	 # AVX512DQ
	vandnps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512DQ
	vandnps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512DQ
	vandnps	8128(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vandnps	8192(%edx), %zmm5, %zmm6	 # AVX512DQ
	vandnps	-8192(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vandnps	-8256(%edx), %zmm5, %zmm6	 # AVX512DQ
	vandnps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vandnps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ
	vandnps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vandnps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ
	vorpd	%zmm4, %zmm5, %zmm6	 # AVX512DQ
	vorpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vorpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vorpd	(%ecx), %zmm5, %zmm6	 # AVX512DQ
	vorpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512DQ
	vorpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vorpd	8128(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vorpd	8192(%edx), %zmm5, %zmm6	 # AVX512DQ
	vorpd	-8192(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vorpd	-8256(%edx), %zmm5, %zmm6	 # AVX512DQ
	vorpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vorpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vorpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vorpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vorps	%zmm4, %zmm5, %zmm6	 # AVX512DQ
	vorps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vorps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vorps	(%ecx), %zmm5, %zmm6	 # AVX512DQ
	vorps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512DQ
	vorps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512DQ
	vorps	8128(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vorps	8192(%edx), %zmm5, %zmm6	 # AVX512DQ
	vorps	-8192(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vorps	-8256(%edx), %zmm5, %zmm6	 # AVX512DQ
	vorps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vorps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ
	vorps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vorps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ
	vxorpd	%zmm4, %zmm5, %zmm6	 # AVX512DQ
	vxorpd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vxorpd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vxorpd	(%ecx), %zmm5, %zmm6	 # AVX512DQ
	vxorpd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512DQ
	vxorpd	(%eax){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vxorpd	8128(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vxorpd	8192(%edx), %zmm5, %zmm6	 # AVX512DQ
	vxorpd	-8192(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vxorpd	-8256(%edx), %zmm5, %zmm6	 # AVX512DQ
	vxorpd	1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vxorpd	1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vxorpd	-1024(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vxorpd	-1032(%edx){1to8}, %zmm5, %zmm6	 # AVX512DQ
	vxorps	%zmm4, %zmm5, %zmm6	 # AVX512DQ
	vxorps	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vxorps	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vxorps	(%ecx), %zmm5, %zmm6	 # AVX512DQ
	vxorps	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512DQ
	vxorps	(%eax){1to16}, %zmm5, %zmm6	 # AVX512DQ
	vxorps	8128(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vxorps	8192(%edx), %zmm5, %zmm6	 # AVX512DQ
	vxorps	-8192(%edx), %zmm5, %zmm6	 # AVX512DQ Disp8
	vxorps	-8256(%edx), %zmm5, %zmm6	 # AVX512DQ
	vxorps	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vxorps	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ
	vxorps	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ Disp8
	vxorps	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512DQ
	vreducepd	$0xab, %zmm5, %zmm6	 # AVX512DQ
	vreducepd	$0xab, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vreducepd	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vreducepd	$0xab, {sae}, %zmm5, %zmm6	 # AVX512DQ
	vreducepd	$123, %zmm5, %zmm6	 # AVX512DQ
	vreducepd	$123, {sae}, %zmm5, %zmm6	 # AVX512DQ
	vreducepd	$123, (%ecx), %zmm6	 # AVX512DQ
	vreducepd	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512DQ
	vreducepd	$123, (%eax){1to8}, %zmm6	 # AVX512DQ
	vreducepd	$123, 8128(%edx), %zmm6	 # AVX512DQ Disp8
	vreducepd	$123, 8192(%edx), %zmm6	 # AVX512DQ
	vreducepd	$123, -8192(%edx), %zmm6	 # AVX512DQ Disp8
	vreducepd	$123, -8256(%edx), %zmm6	 # AVX512DQ
	vreducepd	$123, 1016(%edx){1to8}, %zmm6	 # AVX512DQ Disp8
	vreducepd	$123, 1024(%edx){1to8}, %zmm6	 # AVX512DQ
	vreducepd	$123, -1024(%edx){1to8}, %zmm6	 # AVX512DQ Disp8
	vreducepd	$123, -1032(%edx){1to8}, %zmm6	 # AVX512DQ
	vreduceps	$0xab, %zmm5, %zmm6	 # AVX512DQ
	vreduceps	$0xab, %zmm5, %zmm6{%k7}	 # AVX512DQ
	vreduceps	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vreduceps	$0xab, {sae}, %zmm5, %zmm6	 # AVX512DQ
	vreduceps	$123, %zmm5, %zmm6	 # AVX512DQ
	vreduceps	$123, {sae}, %zmm5, %zmm6	 # AVX512DQ
	vreduceps	$123, (%ecx), %zmm6	 # AVX512DQ
	vreduceps	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512DQ
	vreduceps	$123, (%eax){1to16}, %zmm6	 # AVX512DQ
	vreduceps	$123, 8128(%edx), %zmm6	 # AVX512DQ Disp8
	vreduceps	$123, 8192(%edx), %zmm6	 # AVX512DQ
	vreduceps	$123, -8192(%edx), %zmm6	 # AVX512DQ Disp8
	vreduceps	$123, -8256(%edx), %zmm6	 # AVX512DQ
	vreduceps	$123, 508(%edx){1to16}, %zmm6	 # AVX512DQ Disp8
	vreduceps	$123, 512(%edx){1to16}, %zmm6	 # AVX512DQ
	vreduceps	$123, -512(%edx){1to16}, %zmm6	 # AVX512DQ Disp8
	vreduceps	$123, -516(%edx){1to16}, %zmm6	 # AVX512DQ
	vreducesd	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducesd	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512DQ
	vreducesd	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducesd	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducesd	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducesd	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducesd	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducesd	$123, 1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ Disp8
	vreducesd	$123, 1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducesd	$123, -1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ Disp8
	vreducesd	$123, -1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducess	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducess	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512DQ
	vreducess	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducess	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducess	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducess	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducess	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducess	$123, 508(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ Disp8
	vreducess	$123, 512(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ
	vreducess	$123, -512(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ Disp8
	vreducess	$123, -516(%edx), %xmm5, %xmm6{%k7}	 # AVX512DQ
	kandb	%k7, %k6, %k5	 # AVX512DQ
	kandnb	%k7, %k6, %k5	 # AVX512DQ
	korb	%k7, %k6, %k5	 # AVX512DQ
	kxnorb	%k7, %k6, %k5	 # AVX512DQ
	kxorb	%k7, %k6, %k5	 # AVX512DQ
	knotb	%k6, %k5	 # AVX512DQ
	kortestb	%k6, %k5	 # AVX512DQ
	ktestw	%k6, %k5	 # AVX512DQ
	ktestb	%k6, %k5	 # AVX512DQ
	kshiftrb	$0xab, %k6, %k5	 # AVX512DQ
	kshiftrb	$123, %k6, %k5	 # AVX512DQ
	kshiftlb	$0xab, %k6, %k5	 # AVX512DQ
	kshiftlb	$123, %k6, %k5	 # AVX512DQ
	kmovb	%k6, %k5	 # AVX512DQ
	kmovb	(%ecx), %k5	 # AVX512DQ
	kmovb	-123456(%esp,%esi,8), %k5	 # AVX512DQ
	kmovb	%k5, (%ecx)	 # AVX512DQ
	kmovb	%k5, -123456(%esp,%esi,8)	 # AVX512DQ
	kmovb	%eax, %k5	 # AVX512DQ
	kmovb	%ebp, %k5	 # AVX512DQ
	kmovb	%k5, %eax	 # AVX512DQ
	kmovb	%k5, %ebp	 # AVX512DQ
	kaddw	%k7, %k6, %k5	 # AVX512DQ
	kaddb	%k7, %k6, %k5	 # AVX512DQ
	vextractf64x2	$0xab, %zmm6, (%ecx)	 # AVX512DQ
	vextractf64x2	$0xab, %zmm6, (%ecx){%k7}	 # AVX512DQ
	vextractf64x2	$123, %zmm6, (%ecx)	 # AVX512DQ
	vextractf64x2	$123, %zmm6, -123456(%esp,%esi,8)	 # AVX512DQ
	vextractf64x2	$123, %zmm6, 2032(%edx)	 # AVX512DQ Disp8
	vextractf64x2	$123, %zmm6, 2048(%edx)	 # AVX512DQ
	vextractf64x2	$123, %zmm6, -2048(%edx)	 # AVX512DQ Disp8
	vextractf64x2	$123, %zmm6, -2064(%edx)	 # AVX512DQ
	vextractf32x8	$0xab, %zmm6, (%ecx)	 # AVX512DQ
	vextractf32x8	$0xab, %zmm6, (%ecx){%k7}	 # AVX512DQ
	vextractf32x8	$123, %zmm6, (%ecx)	 # AVX512DQ
	vextractf32x8	$123, %zmm6, -123456(%esp,%esi,8)	 # AVX512DQ
	vextractf32x8	$123, %zmm6, 4064(%edx)	 # AVX512DQ Disp8
	vextractf32x8	$123, %zmm6, 4096(%edx)	 # AVX512DQ
	vextractf32x8	$123, %zmm6, -4096(%edx)	 # AVX512DQ Disp8
	vextractf32x8	$123, %zmm6, -4128(%edx)	 # AVX512DQ
	vextracti64x2	$0xab, %zmm6, (%ecx)	 # AVX512DQ
	vextracti64x2	$0xab, %zmm6, (%ecx){%k7}	 # AVX512DQ
	vextracti64x2	$123, %zmm6, (%ecx)	 # AVX512DQ
	vextracti64x2	$123, %zmm6, -123456(%esp,%esi,8)	 # AVX512DQ
	vextracti64x2	$123, %zmm6, 2032(%edx)	 # AVX512DQ Disp8
	vextracti64x2	$123, %zmm6, 2048(%edx)	 # AVX512DQ
	vextracti64x2	$123, %zmm6, -2048(%edx)	 # AVX512DQ Disp8
	vextracti64x2	$123, %zmm6, -2064(%edx)	 # AVX512DQ
	vextracti32x8	$0xab, %zmm6, (%ecx)	 # AVX512DQ
	vextracti32x8	$0xab, %zmm6, (%ecx){%k7}	 # AVX512DQ
	vextracti32x8	$123, %zmm6, (%ecx)	 # AVX512DQ
	vextracti32x8	$123, %zmm6, -123456(%esp,%esi,8)	 # AVX512DQ
	vextracti32x8	$123, %zmm6, 4064(%edx)	 # AVX512DQ Disp8
	vextracti32x8	$123, %zmm6, 4096(%edx)	 # AVX512DQ
	vextracti32x8	$123, %zmm6, -4096(%edx)	 # AVX512DQ Disp8
	vextracti32x8	$123, %zmm6, -4128(%edx)	 # AVX512DQ
	vcvttpd2qq	%zmm5, %zmm6	 # AVX512DQ
	vcvttpd2qq	%zmm5, %zmm6{%k7}	 # AVX512DQ
	vcvttpd2qq	%zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vcvttpd2qq	{sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvttpd2qq	(%ecx), %zmm6	 # AVX512DQ
	vcvttpd2qq	-123456(%esp,%esi,8), %zmm6	 # AVX512DQ
	vcvttpd2qq	(%eax){1to8}, %zmm6	 # AVX512DQ
	vcvttpd2qq	8128(%edx), %zmm6	 # AVX512DQ Disp8
	vcvttpd2qq	8192(%edx), %zmm6	 # AVX512DQ
	vcvttpd2qq	-8192(%edx), %zmm6	 # AVX512DQ Disp8
	vcvttpd2qq	-8256(%edx), %zmm6	 # AVX512DQ
	vcvttpd2qq	1016(%edx){1to8}, %zmm6	 # AVX512DQ Disp8
	vcvttpd2qq	1024(%edx){1to8}, %zmm6	 # AVX512DQ
	vcvttpd2qq	-1024(%edx){1to8}, %zmm6	 # AVX512DQ Disp8
	vcvttpd2qq	-1032(%edx){1to8}, %zmm6	 # AVX512DQ
	vcvttpd2uqq	%zmm5, %zmm6	 # AVX512DQ
	vcvttpd2uqq	%zmm5, %zmm6{%k7}	 # AVX512DQ
	vcvttpd2uqq	%zmm5, %zmm6{%k7}{z}	 # AVX512DQ
	vcvttpd2uqq	{sae}, %zmm5, %zmm6	 # AVX512DQ
	vcvttpd2uqq	(%ecx), %zmm6	 # AVX512DQ
	vcvttpd2uqq	-123456(%esp,%esi,8), %zmm6	 # AVX512DQ
	vcvttpd2uqq	(%eax){1to8}, %zmm6	 # AVX512DQ
	vcvttpd2uqq	8128(%edx), %zmm6	 # AVX512DQ Disp8
	vcvttpd2uqq	8192(%edx), %zmm6	 # AVX512DQ
	vcvttpd2uqq	-8192(%edx), %zmm6	 # AVX512DQ Disp8
	vcvttpd2uqq	-8256(%edx), %zmm6	 # AVX512DQ
	vcvttpd2uqq	1016(%edx){1to8}, %zmm6	 # AVX512DQ Disp8
	vcvttpd2uqq	1024(%edx){1to8}, %zmm6	 # AVX512DQ
	vcvttpd2uqq	-1024(%edx){1to8}, %zmm6	 # AVX512DQ Disp8
	vcvttpd2uqq	-1032(%edx){1to8}, %zmm6	 # AVX512DQ
	vcvttps2qq	%ymm5, %zmm6{%k7}	 # AVX512DQ
	vcvttps2qq	%ymm5, %zmm6{%k7}{z}	 # AVX512DQ
	vcvttps2qq	{sae}, %ymm5, %zmm6{%k7}	 # AVX512DQ
	vcvttps2qq	(%ecx), %zmm6{%k7}	 # AVX512DQ
	vcvttps2qq	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512DQ
	vcvttps2qq	(%eax){1to8}, %zmm6{%k7}	 # AVX512DQ
	vcvttps2qq	4064(%edx), %zmm6{%k7}	 # AVX512DQ Disp8
	vcvttps2qq	4096(%edx), %zmm6{%k7}	 # AVX512DQ
	vcvttps2qq	-4096(%edx), %zmm6{%k7}	 # AVX512DQ Disp8
	vcvttps2qq	-4128(%edx), %zmm6{%k7}	 # AVX512DQ
	vcvttps2qq	508(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ Disp8
	vcvttps2qq	512(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ
	vcvttps2qq	-512(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ Disp8
	vcvttps2qq	-516(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ
	vcvttps2uqq	%ymm5, %zmm6{%k7}	 # AVX512DQ
	vcvttps2uqq	%ymm5, %zmm6{%k7}{z}	 # AVX512DQ
	vcvttps2uqq	{sae}, %ymm5, %zmm6{%k7}	 # AVX512DQ
	vcvttps2uqq	(%ecx), %zmm6{%k7}	 # AVX512DQ
	vcvttps2uqq	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512DQ
	vcvttps2uqq	(%eax){1to8}, %zmm6{%k7}	 # AVX512DQ
	vcvttps2uqq	4064(%edx), %zmm6{%k7}	 # AVX512DQ Disp8
	vcvttps2uqq	4096(%edx), %zmm6{%k7}	 # AVX512DQ
	vcvttps2uqq	-4096(%edx), %zmm6{%k7}	 # AVX512DQ Disp8
	vcvttps2uqq	-4128(%edx), %zmm6{%k7}	 # AVX512DQ
	vcvttps2uqq	508(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ Disp8
	vcvttps2uqq	512(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ
	vcvttps2uqq	-512(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ Disp8
	vcvttps2uqq	-516(%edx){1to8}, %zmm6{%k7}	 # AVX512DQ
	vpmovd2m	%zmm6, %k5	 # AVX512DQ
	vpmovq2m	%zmm6, %k5	 # AVX512DQ
	vpmovm2d	%k5, %zmm6	 # AVX512DQ
	vpmovm2q	%k5, %zmm6	 # AVX512DQ

	.intel_syntax noprefix
	vbroadcastf32x8	zmm6, YMMWORD PTR [ecx]	 # AVX512DQ
	vbroadcastf32x8	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512DQ
	vbroadcastf32x8	zmm6{k7}{z}, YMMWORD PTR [ecx]	 # AVX512DQ
	vbroadcastf32x8	zmm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vbroadcastf32x8	zmm6, YMMWORD PTR [edx+4064]	 # AVX512DQ Disp8
	vbroadcastf32x8	zmm6, YMMWORD PTR [edx+4096]	 # AVX512DQ
	vbroadcastf32x8	zmm6, YMMWORD PTR [edx-4096]	 # AVX512DQ Disp8
	vbroadcastf32x8	zmm6, YMMWORD PTR [edx-4128]	 # AVX512DQ
	vbroadcastf64x2	zmm6, XMMWORD PTR [ecx]	 # AVX512DQ
	vbroadcastf64x2	zmm6{k7}, XMMWORD PTR [ecx]	 # AVX512DQ
	vbroadcastf64x2	zmm6{k7}{z}, XMMWORD PTR [ecx]	 # AVX512DQ
	vbroadcastf64x2	zmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vbroadcastf64x2	zmm6, XMMWORD PTR [edx+2032]	 # AVX512DQ Disp8
	vbroadcastf64x2	zmm6, XMMWORD PTR [edx+2048]	 # AVX512DQ
	vbroadcastf64x2	zmm6, XMMWORD PTR [edx-2048]	 # AVX512DQ Disp8
	vbroadcastf64x2	zmm6, XMMWORD PTR [edx-2064]	 # AVX512DQ
	vbroadcasti32x8	zmm6, YMMWORD PTR [ecx]	 # AVX512DQ
	vbroadcasti32x8	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512DQ
	vbroadcasti32x8	zmm6{k7}{z}, YMMWORD PTR [ecx]	 # AVX512DQ
	vbroadcasti32x8	zmm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vbroadcasti32x8	zmm6, YMMWORD PTR [edx+4064]	 # AVX512DQ Disp8
	vbroadcasti32x8	zmm6, YMMWORD PTR [edx+4096]	 # AVX512DQ
	vbroadcasti32x8	zmm6, YMMWORD PTR [edx-4096]	 # AVX512DQ Disp8
	vbroadcasti32x8	zmm6, YMMWORD PTR [edx-4128]	 # AVX512DQ
	vbroadcasti64x2	zmm6, XMMWORD PTR [ecx]	 # AVX512DQ
	vbroadcasti64x2	zmm6{k7}, XMMWORD PTR [ecx]	 # AVX512DQ
	vbroadcasti64x2	zmm6{k7}{z}, XMMWORD PTR [ecx]	 # AVX512DQ
	vbroadcasti64x2	zmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vbroadcasti64x2	zmm6, XMMWORD PTR [edx+2032]	 # AVX512DQ Disp8
	vbroadcasti64x2	zmm6, XMMWORD PTR [edx+2048]	 # AVX512DQ
	vbroadcasti64x2	zmm6, XMMWORD PTR [edx-2048]	 # AVX512DQ Disp8
	vbroadcasti64x2	zmm6, XMMWORD PTR [edx-2064]	 # AVX512DQ
	vbroadcastf32x2	zmm6, xmm7	 # AVX512DQ
	vbroadcastf32x2	zmm6{k7}, xmm7	 # AVX512DQ
	vbroadcastf32x2	zmm6{k7}{z}, xmm7	 # AVX512DQ
	vbroadcastf32x2	zmm6, QWORD PTR [ecx]	 # AVX512DQ
	vbroadcastf32x2	zmm6, QWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vbroadcastf32x2	zmm6, QWORD PTR [edx+1016]	 # AVX512DQ Disp8
	vbroadcastf32x2	zmm6, QWORD PTR [edx+1024]	 # AVX512DQ
	vbroadcastf32x2	zmm6, QWORD PTR [edx-1024]	 # AVX512DQ Disp8
	vbroadcastf32x2	zmm6, QWORD PTR [edx-1032]	 # AVX512DQ
	vcvtpd2qq	zmm6, zmm5	 # AVX512DQ
	vcvtpd2qq	zmm6{k7}, zmm5	 # AVX512DQ
	vcvtpd2qq	zmm6{k7}{z}, zmm5	 # AVX512DQ
	vcvtpd2qq	zmm6, zmm5{rn-sae}	 # AVX512DQ
	vcvtpd2qq	zmm6, zmm5{ru-sae}	 # AVX512DQ
	vcvtpd2qq	zmm6, zmm5{rd-sae}	 # AVX512DQ
	vcvtpd2qq	zmm6, zmm5{rz-sae}	 # AVX512DQ
	vcvtpd2qq	zmm6, ZMMWORD PTR [ecx]	 # AVX512DQ
	vcvtpd2qq	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vcvtpd2qq	zmm6, qword bcst [eax]	 # AVX512DQ
	vcvtpd2qq	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vcvtpd2qq	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vcvtpd2qq	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vcvtpd2qq	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vcvtpd2qq	zmm6, qword bcst [edx+1016]	 # AVX512DQ Disp8
	vcvtpd2qq	zmm6, qword bcst [edx+1024]	 # AVX512DQ
	vcvtpd2qq	zmm6, qword bcst [edx-1024]	 # AVX512DQ Disp8
	vcvtpd2qq	zmm6, qword bcst [edx-1032]	 # AVX512DQ
	vcvtpd2uqq	zmm6, zmm5	 # AVX512DQ
	vcvtpd2uqq	zmm6{k7}, zmm5	 # AVX512DQ
	vcvtpd2uqq	zmm6{k7}{z}, zmm5	 # AVX512DQ
	vcvtpd2uqq	zmm6, zmm5{rn-sae}	 # AVX512DQ
	vcvtpd2uqq	zmm6, zmm5{ru-sae}	 # AVX512DQ
	vcvtpd2uqq	zmm6, zmm5{rd-sae}	 # AVX512DQ
	vcvtpd2uqq	zmm6, zmm5{rz-sae}	 # AVX512DQ
	vcvtpd2uqq	zmm6, ZMMWORD PTR [ecx]	 # AVX512DQ
	vcvtpd2uqq	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vcvtpd2uqq	zmm6, qword bcst [eax]	 # AVX512DQ
	vcvtpd2uqq	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vcvtpd2uqq	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vcvtpd2uqq	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vcvtpd2uqq	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vcvtpd2uqq	zmm6, qword bcst [edx+1016]	 # AVX512DQ Disp8
	vcvtpd2uqq	zmm6, qword bcst [edx+1024]	 # AVX512DQ
	vcvtpd2uqq	zmm6, qword bcst [edx-1024]	 # AVX512DQ Disp8
	vcvtpd2uqq	zmm6, qword bcst [edx-1032]	 # AVX512DQ
	vcvtps2qq	zmm6{k7}, ymm5	 # AVX512DQ
	vcvtps2qq	zmm6{k7}{z}, ymm5	 # AVX512DQ
	vcvtps2qq	zmm6{k7}, ymm5{rn-sae}	 # AVX512DQ
	vcvtps2qq	zmm6{k7}, ymm5{ru-sae}	 # AVX512DQ
	vcvtps2qq	zmm6{k7}, ymm5{rd-sae}	 # AVX512DQ
	vcvtps2qq	zmm6{k7}, ymm5{rz-sae}	 # AVX512DQ
	vcvtps2qq	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512DQ
	vcvtps2qq	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vcvtps2qq	zmm6{k7}, dword bcst [eax]	 # AVX512DQ
	vcvtps2qq	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512DQ Disp8
	vcvtps2qq	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512DQ
	vcvtps2qq	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512DQ Disp8
	vcvtps2qq	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512DQ
	vcvtps2qq	zmm6{k7}, dword bcst [edx+508]	 # AVX512DQ Disp8
	vcvtps2qq	zmm6{k7}, dword bcst [edx+512]	 # AVX512DQ
	vcvtps2qq	zmm6{k7}, dword bcst [edx-512]	 # AVX512DQ Disp8
	vcvtps2qq	zmm6{k7}, dword bcst [edx-516]	 # AVX512DQ
	vcvtps2qq	zmm6{k7}, DWORD BCST [edx+508]	 # AVX512DQ Disp8
	vcvtps2uqq	zmm6{k7}, ymm5	 # AVX512DQ
	vcvtps2uqq	zmm6{k7}{z}, ymm5	 # AVX512DQ
	vcvtps2uqq	zmm6{k7}, ymm5{rn-sae}	 # AVX512DQ
	vcvtps2uqq	zmm6{k7}, ymm5{ru-sae}	 # AVX512DQ
	vcvtps2uqq	zmm6{k7}, ymm5{rd-sae}	 # AVX512DQ
	vcvtps2uqq	zmm6{k7}, ymm5{rz-sae}	 # AVX512DQ
	vcvtps2uqq	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512DQ
	vcvtps2uqq	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vcvtps2uqq	zmm6{k7}, dword bcst [eax]	 # AVX512DQ
	vcvtps2uqq	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512DQ Disp8
	vcvtps2uqq	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512DQ
	vcvtps2uqq	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512DQ Disp8
	vcvtps2uqq	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512DQ
	vcvtps2uqq	zmm6{k7}, dword bcst [edx+508]	 # AVX512DQ Disp8
	vcvtps2uqq	zmm6{k7}, dword bcst [edx+512]	 # AVX512DQ
	vcvtps2uqq	zmm6{k7}, dword bcst [edx-512]	 # AVX512DQ Disp8
	vcvtps2uqq	zmm6{k7}, dword bcst [edx-516]	 # AVX512DQ
	vcvtps2uqq	zmm6{k7}, DWORD BCST [edx+508]	 # AVX512DQ Disp8
	vcvtqq2pd	zmm6, zmm5	 # AVX512DQ
	vcvtqq2pd	zmm6{k7}, zmm5	 # AVX512DQ
	vcvtqq2pd	zmm6{k7}{z}, zmm5	 # AVX512DQ
	vcvtqq2pd	zmm6, zmm5{rn-sae}	 # AVX512DQ
	vcvtqq2pd	zmm6, zmm5{ru-sae}	 # AVX512DQ
	vcvtqq2pd	zmm6, zmm5{rd-sae}	 # AVX512DQ
	vcvtqq2pd	zmm6, zmm5{rz-sae}	 # AVX512DQ
	vcvtqq2pd	zmm6, ZMMWORD PTR [ecx]	 # AVX512DQ
	vcvtqq2pd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vcvtqq2pd	zmm6, qword bcst [eax]	 # AVX512DQ
	vcvtqq2pd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vcvtqq2pd	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vcvtqq2pd	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vcvtqq2pd	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vcvtqq2pd	zmm6, qword bcst [edx+1016]	 # AVX512DQ Disp8
	vcvtqq2pd	zmm6, qword bcst [edx+1024]	 # AVX512DQ
	vcvtqq2pd	zmm6, qword bcst [edx-1024]	 # AVX512DQ Disp8
	vcvtqq2pd	zmm6, qword bcst [edx-1032]	 # AVX512DQ
	vcvtqq2ps	ymm6{k7}, zmm5	 # AVX512DQ
	vcvtqq2ps	ymm6{k7}{z}, zmm5	 # AVX512DQ
	vcvtqq2ps	ymm6{k7}, zmm5{rn-sae}	 # AVX512DQ
	vcvtqq2ps	ymm6{k7}, zmm5{ru-sae}	 # AVX512DQ
	vcvtqq2ps	ymm6{k7}, zmm5{rd-sae}	 # AVX512DQ
	vcvtqq2ps	ymm6{k7}, zmm5{rz-sae}	 # AVX512DQ
	vcvtqq2ps	ymm6{k7}, ZMMWORD PTR [ecx]	 # AVX512DQ
	vcvtqq2ps	ymm6{k7}, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vcvtqq2ps	ymm6{k7}, qword bcst [eax]	 # AVX512DQ
	vcvtqq2ps	ymm6{k7}, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vcvtqq2ps	ymm6{k7}, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vcvtqq2ps	ymm6{k7}, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vcvtqq2ps	ymm6{k7}, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vcvtqq2ps	ymm6{k7}, qword bcst [edx+1016]	 # AVX512DQ Disp8
	vcvtqq2ps	ymm6{k7}, qword bcst [edx+1024]	 # AVX512DQ
	vcvtqq2ps	ymm6{k7}, qword bcst [edx-1024]	 # AVX512DQ Disp8
	vcvtqq2ps	ymm6{k7}, qword bcst [edx-1032]	 # AVX512DQ
	vcvtuqq2pd	zmm6, zmm5	 # AVX512DQ
	vcvtuqq2pd	zmm6{k7}, zmm5	 # AVX512DQ
	vcvtuqq2pd	zmm6{k7}{z}, zmm5	 # AVX512DQ
	vcvtuqq2pd	zmm6, zmm5{rn-sae}	 # AVX512DQ
	vcvtuqq2pd	zmm6, zmm5{ru-sae}	 # AVX512DQ
	vcvtuqq2pd	zmm6, zmm5{rd-sae}	 # AVX512DQ
	vcvtuqq2pd	zmm6, zmm5{rz-sae}	 # AVX512DQ
	vcvtuqq2pd	zmm6, ZMMWORD PTR [ecx]	 # AVX512DQ
	vcvtuqq2pd	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vcvtuqq2pd	zmm6, qword bcst [eax]	 # AVX512DQ
	vcvtuqq2pd	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vcvtuqq2pd	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vcvtuqq2pd	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vcvtuqq2pd	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vcvtuqq2pd	zmm6, qword bcst [edx+1016]	 # AVX512DQ Disp8
	vcvtuqq2pd	zmm6, qword bcst [edx+1024]	 # AVX512DQ
	vcvtuqq2pd	zmm6, qword bcst [edx-1024]	 # AVX512DQ Disp8
	vcvtuqq2pd	zmm6, qword bcst [edx-1032]	 # AVX512DQ
	vcvtuqq2ps	ymm6{k7}, zmm5	 # AVX512DQ
	vcvtuqq2ps	ymm6{k7}{z}, zmm5	 # AVX512DQ
	vcvtuqq2ps	ymm6{k7}, zmm5{rn-sae}	 # AVX512DQ
	vcvtuqq2ps	ymm6{k7}, zmm5{ru-sae}	 # AVX512DQ
	vcvtuqq2ps	ymm6{k7}, zmm5{rd-sae}	 # AVX512DQ
	vcvtuqq2ps	ymm6{k7}, zmm5{rz-sae}	 # AVX512DQ
	vcvtuqq2ps	ymm6{k7}, ZMMWORD PTR [ecx]	 # AVX512DQ
	vcvtuqq2ps	ymm6{k7}, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vcvtuqq2ps	ymm6{k7}, qword bcst [eax]	 # AVX512DQ
	vcvtuqq2ps	ymm6{k7}, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vcvtuqq2ps	ymm6{k7}, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vcvtuqq2ps	ymm6{k7}, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vcvtuqq2ps	ymm6{k7}, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vcvtuqq2ps	ymm6{k7}, qword bcst [edx+1016]	 # AVX512DQ Disp8
	vcvtuqq2ps	ymm6{k7}, qword bcst [edx+1024]	 # AVX512DQ
	vcvtuqq2ps	ymm6{k7}, qword bcst [edx-1024]	 # AVX512DQ Disp8
	vcvtuqq2ps	ymm6{k7}, qword bcst [edx-1032]	 # AVX512DQ
	vextractf64x2	xmm6{k7}, zmm5, 0xab	 # AVX512DQ
	vextractf64x2	xmm6{k7}{z}, zmm5, 0xab	 # AVX512DQ
	vextractf64x2	xmm6{k7}, zmm5, 123	 # AVX512DQ
	vextractf32x8	ymm6{k7}, zmm5, 0xab	 # AVX512DQ
	vextractf32x8	ymm6{k7}{z}, zmm5, 0xab	 # AVX512DQ
	vextractf32x8	ymm6{k7}, zmm5, 123	 # AVX512DQ
	vextracti64x2	xmm6{k7}, zmm5, 0xab	 # AVX512DQ
	vextracti64x2	xmm6{k7}{z}, zmm5, 0xab	 # AVX512DQ
	vextracti64x2	xmm6{k7}, zmm5, 123	 # AVX512DQ
	vextracti32x8	ymm6{k7}, zmm5, 0xab	 # AVX512DQ
	vextracti32x8	ymm6{k7}{z}, zmm5, 0xab	 # AVX512DQ
	vextracti32x8	ymm6{k7}, zmm5, 123	 # AVX512DQ
	vfpclasspd	k5, zmm6, 0xab	 # AVX512DQ
	vfpclasspd	k5{k7}, zmm6, 0xab	 # AVX512DQ
	vfpclasspd	k5, zmm6, 123	 # AVX512DQ
	vfpclasspd	k5, ZMMWORD PTR [ecx], 123	 # AVX512DQ
	vfpclasspd	k5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vfpclasspd	k5, [eax]{1to8}, 123	 # AVX512DQ
	vfpclasspd	k5, ZMMWORD PTR [edx+8128], 123	 # AVX512DQ Disp8
	vfpclasspd	k5, ZMMWORD PTR [edx+8192], 123	 # AVX512DQ
	vfpclasspd	k5, ZMMWORD PTR [edx-8192], 123	 # AVX512DQ Disp8
	vfpclasspd	k5, ZMMWORD PTR [edx-8256], 123	 # AVX512DQ
	vfpclasspd	k5, QWORD BCST [edx+1016]{1to8}, 123	 # AVX512DQ Disp8
	vfpclasspd	k5, QWORD BCST [edx+1024]{1to8}, 123	 # AVX512DQ
	vfpclasspd	k5, QWORD BCST [edx-1024]{1to8}, 123	 # AVX512DQ Disp8
	vfpclasspd	k5, QWORD BCST [edx-1032]{1to8}, 123	 # AVX512DQ
	vfpclassps	k5, zmm6, 0xab	 # AVX512DQ
	vfpclassps	k5{k7}, zmm6, 0xab	 # AVX512DQ
	vfpclassps	k5, zmm6, 123	 # AVX512DQ
	vfpclassps	k5, ZMMWORD PTR [ecx], 123	 # AVX512DQ
	vfpclassps	k5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vfpclassps	k5, [eax]{1to16}, 123	 # AVX512DQ
	vfpclassps	k5, ZMMWORD PTR [edx+8128], 123	 # AVX512DQ Disp8
	vfpclassps	k5, ZMMWORD PTR [edx+8192], 123	 # AVX512DQ
	vfpclassps	k5, ZMMWORD PTR [edx-8192], 123	 # AVX512DQ Disp8
	vfpclassps	k5, ZMMWORD PTR [edx-8256], 123	 # AVX512DQ
	vfpclassps	k5, DWORD BCST [edx+508]{1to16}, 123	 # AVX512DQ Disp8
	vfpclassps	k5, DWORD BCST [edx+512]{1to16}, 123	 # AVX512DQ
	vfpclassps	k5, DWORD BCST [edx-512]{1to16}, 123	 # AVX512DQ Disp8
	vfpclassps	k5, DWORD BCST [edx-516]{1to16}, 123	 # AVX512DQ
	vfpclasssd	k5{k7}, xmm6, 0xab	 # AVX512DQ
	vfpclasssd	k5{k7}, xmm6, 123	 # AVX512DQ
	vfpclasssd	k5{k7}, QWORD PTR [ecx], 123	 # AVX512DQ
	vfpclasssd	k5{k7}, QWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vfpclasssd	k5{k7}, QWORD PTR [edx+1016], 123	 # AVX512DQ Disp8
	vfpclasssd	k5{k7}, QWORD PTR [edx+1024], 123	 # AVX512DQ
	vfpclasssd	k5{k7}, QWORD PTR [edx-1024], 123	 # AVX512DQ Disp8
	vfpclasssd	k5{k7}, QWORD PTR [edx-1032], 123	 # AVX512DQ
	vfpclassss	k5{k7}, xmm6, 0xab	 # AVX512DQ
	vfpclassss	k5{k7}, xmm6, 123	 # AVX512DQ
	vfpclassss	k5{k7}, DWORD PTR [ecx], 123	 # AVX512DQ
	vfpclassss	k5{k7}, DWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vfpclassss	k5{k7}, DWORD PTR [edx+508], 123	 # AVX512DQ Disp8
	vfpclassss	k5{k7}, DWORD PTR [edx+512], 123	 # AVX512DQ
	vfpclassss	k5{k7}, DWORD PTR [edx-512], 123	 # AVX512DQ Disp8
	vfpclassss	k5{k7}, DWORD PTR [edx-516], 123	 # AVX512DQ
	vinsertf64x2	zmm6{k7}, zmm5, xmm4, 0xab	 # AVX512DQ
	vinsertf64x2	zmm6{k7}{z}, zmm5, xmm4, 0xab	 # AVX512DQ
	vinsertf64x2	zmm6{k7}, zmm5, xmm4, 123	 # AVX512DQ
	vinsertf64x2	zmm6{k7}, zmm5, XMMWORD PTR [ecx], 123	 # AVX512DQ
	vinsertf64x2	zmm6{k7}, zmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vinsertf64x2	zmm6{k7}, zmm5, XMMWORD PTR [edx+2032], 123	 # AVX512DQ Disp8
	vinsertf64x2	zmm6{k7}, zmm5, XMMWORD PTR [edx+2048], 123	 # AVX512DQ
	vinsertf64x2	zmm6{k7}, zmm5, XMMWORD PTR [edx-2048], 123	 # AVX512DQ Disp8
	vinsertf64x2	zmm6{k7}, zmm5, XMMWORD PTR [edx-2064], 123	 # AVX512DQ
	vinsertf32x8	zmm6{k7}, zmm5, ymm4, 0xab	 # AVX512DQ
	vinsertf32x8	zmm6{k7}{z}, zmm5, ymm4, 0xab	 # AVX512DQ
	vinsertf32x8	zmm6{k7}, zmm5, ymm4, 123	 # AVX512DQ
	vinsertf32x8	zmm6{k7}, zmm5, YMMWORD PTR [ecx], 123	 # AVX512DQ
	vinsertf32x8	zmm6{k7}, zmm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vinsertf32x8	zmm6{k7}, zmm5, YMMWORD PTR [edx+4064], 123	 # AVX512DQ Disp8
	vinsertf32x8	zmm6{k7}, zmm5, YMMWORD PTR [edx+4096], 123	 # AVX512DQ
	vinsertf32x8	zmm6{k7}, zmm5, YMMWORD PTR [edx-4096], 123	 # AVX512DQ Disp8
	vinsertf32x8	zmm6{k7}, zmm5, YMMWORD PTR [edx-4128], 123	 # AVX512DQ
	vinserti64x2	zmm6{k7}, zmm5, xmm4, 0xab	 # AVX512DQ
	vinserti64x2	zmm6{k7}{z}, zmm5, xmm4, 0xab	 # AVX512DQ
	vinserti64x2	zmm6{k7}, zmm5, xmm4, 123	 # AVX512DQ
	vinserti64x2	zmm6{k7}, zmm5, XMMWORD PTR [ecx], 123	 # AVX512DQ
	vinserti64x2	zmm6{k7}, zmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vinserti64x2	zmm6{k7}, zmm5, XMMWORD PTR [edx+2032], 123	 # AVX512DQ Disp8
	vinserti64x2	zmm6{k7}, zmm5, XMMWORD PTR [edx+2048], 123	 # AVX512DQ
	vinserti64x2	zmm6{k7}, zmm5, XMMWORD PTR [edx-2048], 123	 # AVX512DQ Disp8
	vinserti64x2	zmm6{k7}, zmm5, XMMWORD PTR [edx-2064], 123	 # AVX512DQ
	vinserti32x8	zmm6{k7}, zmm5, ymm4, 0xab	 # AVX512DQ
	vinserti32x8	zmm6{k7}{z}, zmm5, ymm4, 0xab	 # AVX512DQ
	vinserti32x8	zmm6{k7}, zmm5, ymm4, 123	 # AVX512DQ
	vinserti32x8	zmm6{k7}, zmm5, YMMWORD PTR [ecx], 123	 # AVX512DQ
	vinserti32x8	zmm6{k7}, zmm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vinserti32x8	zmm6{k7}, zmm5, YMMWORD PTR [edx+4064], 123	 # AVX512DQ Disp8
	vinserti32x8	zmm6{k7}, zmm5, YMMWORD PTR [edx+4096], 123	 # AVX512DQ
	vinserti32x8	zmm6{k7}, zmm5, YMMWORD PTR [edx-4096], 123	 # AVX512DQ Disp8
	vinserti32x8	zmm6{k7}, zmm5, YMMWORD PTR [edx-4128], 123	 # AVX512DQ
	vbroadcasti32x2	zmm6, xmm7	 # AVX512DQ
	vbroadcasti32x2	zmm6{k7}, xmm7	 # AVX512DQ
	vbroadcasti32x2	zmm6{k7}{z}, xmm7	 # AVX512DQ
	vbroadcasti32x2	zmm6, QWORD PTR [ecx]	 # AVX512DQ
	vbroadcasti32x2	zmm6, QWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vbroadcasti32x2	zmm6, QWORD PTR [edx+1016]	 # AVX512DQ Disp8
	vbroadcasti32x2	zmm6, QWORD PTR [edx+1024]	 # AVX512DQ
	vbroadcasti32x2	zmm6, QWORD PTR [edx-1024]	 # AVX512DQ Disp8
	vbroadcasti32x2	zmm6, QWORD PTR [edx-1032]	 # AVX512DQ
	vpmullq	zmm6, zmm5, zmm4	 # AVX512DQ
	vpmullq	zmm6{k7}, zmm5, zmm4	 # AVX512DQ
	vpmullq	zmm6{k7}{z}, zmm5, zmm4	 # AVX512DQ
	vpmullq	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512DQ
	vpmullq	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vpmullq	zmm6, zmm5, qword bcst [eax]	 # AVX512DQ
	vpmullq	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vpmullq	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vpmullq	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vpmullq	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vpmullq	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512DQ Disp8
	vpmullq	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512DQ
	vpmullq	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512DQ Disp8
	vpmullq	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512DQ
	vrangepd	zmm6, zmm5, zmm4, 0xab	 # AVX512DQ
	vrangepd	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512DQ
	vrangepd	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512DQ
	vrangepd	zmm6, zmm5, zmm4{sae}, 0xab	 # AVX512DQ
	vrangepd	zmm6, zmm5, zmm4, 123	 # AVX512DQ
	vrangepd	zmm6, zmm5, zmm4{sae}, 123	 # AVX512DQ
	vrangepd	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512DQ
	vrangepd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vrangepd	zmm6, zmm5, qword bcst [eax], 123	 # AVX512DQ
	vrangepd	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512DQ Disp8
	vrangepd	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512DQ
	vrangepd	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512DQ Disp8
	vrangepd	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512DQ
	vrangepd	zmm6, zmm5, qword bcst [edx+1016], 123	 # AVX512DQ Disp8
	vrangepd	zmm6, zmm5, qword bcst [edx+1024], 123	 # AVX512DQ
	vrangepd	zmm6, zmm5, qword bcst [edx-1024], 123	 # AVX512DQ Disp8
	vrangepd	zmm6, zmm5, qword bcst [edx-1032], 123	 # AVX512DQ
	vrangeps	zmm6, zmm5, zmm4, 0xab	 # AVX512DQ
	vrangeps	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512DQ
	vrangeps	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512DQ
	vrangeps	zmm6, zmm5, zmm4{sae}, 0xab	 # AVX512DQ
	vrangeps	zmm6, zmm5, zmm4, 123	 # AVX512DQ
	vrangeps	zmm6, zmm5, zmm4{sae}, 123	 # AVX512DQ
	vrangeps	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512DQ
	vrangeps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vrangeps	zmm6, zmm5, dword bcst [eax], 123	 # AVX512DQ
	vrangeps	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512DQ Disp8
	vrangeps	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512DQ
	vrangeps	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512DQ Disp8
	vrangeps	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512DQ
	vrangeps	zmm6, zmm5, dword bcst [edx+508], 123	 # AVX512DQ Disp8
	vrangeps	zmm6, zmm5, dword bcst [edx+512], 123	 # AVX512DQ
	vrangeps	zmm6, zmm5, dword bcst [edx-512], 123	 # AVX512DQ Disp8
	vrangeps	zmm6, zmm5, dword bcst [edx-516], 123	 # AVX512DQ
	vrangesd	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512DQ
	vrangesd	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512DQ
	vrangesd	xmm6{k7}, xmm5, xmm4{sae}, 0xab	 # AVX512DQ
	vrangesd	xmm6{k7}, xmm5, xmm4, 123	 # AVX512DQ
	vrangesd	xmm6{k7}, xmm5, xmm4{sae}, 123	 # AVX512DQ
	vrangesd	xmm6{k7}, xmm5, QWORD PTR [ecx], 123	 # AVX512DQ
	vrangesd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vrangesd	xmm6{k7}, xmm5, QWORD PTR [edx+1016], 123	 # AVX512DQ Disp8
	vrangesd	xmm6{k7}, xmm5, QWORD PTR [edx+1024], 123	 # AVX512DQ
	vrangesd	xmm6{k7}, xmm5, QWORD PTR [edx-1024], 123	 # AVX512DQ Disp8
	vrangesd	xmm6{k7}, xmm5, QWORD PTR [edx-1032], 123	 # AVX512DQ
	vrangess	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512DQ
	vrangess	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512DQ
	vrangess	xmm6{k7}, xmm5, xmm4{sae}, 0xab	 # AVX512DQ
	vrangess	xmm6{k7}, xmm5, xmm4, 123	 # AVX512DQ
	vrangess	xmm6{k7}, xmm5, xmm4{sae}, 123	 # AVX512DQ
	vrangess	xmm6{k7}, xmm5, DWORD PTR [ecx], 123	 # AVX512DQ
	vrangess	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vrangess	xmm6{k7}, xmm5, DWORD PTR [edx+508], 123	 # AVX512DQ Disp8
	vrangess	xmm6{k7}, xmm5, DWORD PTR [edx+512], 123	 # AVX512DQ
	vrangess	xmm6{k7}, xmm5, DWORD PTR [edx-512], 123	 # AVX512DQ Disp8
	vrangess	xmm6{k7}, xmm5, DWORD PTR [edx-516], 123	 # AVX512DQ
	vandpd	zmm6, zmm5, zmm4	 # AVX512DQ
	vandpd	zmm6{k7}, zmm5, zmm4	 # AVX512DQ
	vandpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512DQ
	vandpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512DQ
	vandpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vandpd	zmm6, zmm5, qword bcst [eax]	 # AVX512DQ
	vandpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vandpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vandpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vandpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vandpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512DQ Disp8
	vandpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512DQ
	vandpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512DQ Disp8
	vandpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512DQ
	vandps	zmm6, zmm5, zmm4	 # AVX512DQ
	vandps	zmm6{k7}, zmm5, zmm4	 # AVX512DQ
	vandps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512DQ
	vandps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512DQ
	vandps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vandps	zmm6, zmm5, dword bcst [eax]	 # AVX512DQ
	vandps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vandps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vandps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vandps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vandps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512DQ Disp8
	vandps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512DQ
	vandps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512DQ Disp8
	vandps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512DQ
	vandnpd	zmm6, zmm5, zmm4	 # AVX512DQ
	vandnpd	zmm6{k7}, zmm5, zmm4	 # AVX512DQ
	vandnpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512DQ
	vandnpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512DQ
	vandnpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vandnpd	zmm6, zmm5, qword bcst [eax]	 # AVX512DQ
	vandnpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vandnpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vandnpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vandnpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vandnpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512DQ Disp8
	vandnpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512DQ
	vandnpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512DQ Disp8
	vandnpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512DQ
	vandnps	zmm6, zmm5, zmm4	 # AVX512DQ
	vandnps	zmm6{k7}, zmm5, zmm4	 # AVX512DQ
	vandnps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512DQ
	vandnps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512DQ
	vandnps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vandnps	zmm6, zmm5, dword bcst [eax]	 # AVX512DQ
	vandnps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vandnps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vandnps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vandnps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vandnps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512DQ Disp8
	vandnps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512DQ
	vandnps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512DQ Disp8
	vandnps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512DQ
	vorpd	zmm6, zmm5, zmm4	 # AVX512DQ
	vorpd	zmm6{k7}, zmm5, zmm4	 # AVX512DQ
	vorpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512DQ
	vorpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512DQ
	vorpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vorpd	zmm6, zmm5, qword bcst [eax]	 # AVX512DQ
	vorpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vorpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vorpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vorpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vorpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512DQ Disp8
	vorpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512DQ
	vorpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512DQ Disp8
	vorpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512DQ
	vorps	zmm6, zmm5, zmm4	 # AVX512DQ
	vorps	zmm6{k7}, zmm5, zmm4	 # AVX512DQ
	vorps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512DQ
	vorps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512DQ
	vorps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vorps	zmm6, zmm5, dword bcst [eax]	 # AVX512DQ
	vorps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vorps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vorps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vorps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vorps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512DQ Disp8
	vorps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512DQ
	vorps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512DQ Disp8
	vorps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512DQ
	vxorpd	zmm6, zmm5, zmm4	 # AVX512DQ
	vxorpd	zmm6{k7}, zmm5, zmm4	 # AVX512DQ
	vxorpd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512DQ
	vxorpd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512DQ
	vxorpd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vxorpd	zmm6, zmm5, qword bcst [eax]	 # AVX512DQ
	vxorpd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vxorpd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vxorpd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vxorpd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vxorpd	zmm6, zmm5, qword bcst [edx+1016]	 # AVX512DQ Disp8
	vxorpd	zmm6, zmm5, qword bcst [edx+1024]	 # AVX512DQ
	vxorpd	zmm6, zmm5, qword bcst [edx-1024]	 # AVX512DQ Disp8
	vxorpd	zmm6, zmm5, qword bcst [edx-1032]	 # AVX512DQ
	vxorps	zmm6, zmm5, zmm4	 # AVX512DQ
	vxorps	zmm6{k7}, zmm5, zmm4	 # AVX512DQ
	vxorps	zmm6{k7}{z}, zmm5, zmm4	 # AVX512DQ
	vxorps	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512DQ
	vxorps	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vxorps	zmm6, zmm5, dword bcst [eax]	 # AVX512DQ
	vxorps	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vxorps	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vxorps	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vxorps	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vxorps	zmm6, zmm5, dword bcst [edx+508]	 # AVX512DQ Disp8
	vxorps	zmm6, zmm5, dword bcst [edx+512]	 # AVX512DQ
	vxorps	zmm6, zmm5, dword bcst [edx-512]	 # AVX512DQ Disp8
	vxorps	zmm6, zmm5, dword bcst [edx-516]	 # AVX512DQ
	vreducepd	zmm6, zmm5, 0xab	 # AVX512DQ
	vreducepd	zmm6{k7}, zmm5, 0xab	 # AVX512DQ
	vreducepd	zmm6{k7}{z}, zmm5, 0xab	 # AVX512DQ
	vreducepd	zmm6, zmm5{sae}, 0xab	 # AVX512DQ
	vreducepd	zmm6, zmm5, 123	 # AVX512DQ
	vreducepd	zmm6, zmm5{sae}, 123	 # AVX512DQ
	vreducepd	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512DQ
	vreducepd	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vreducepd	zmm6, qword bcst [eax], 123	 # AVX512DQ
	vreducepd	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512DQ Disp8
	vreducepd	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512DQ
	vreducepd	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512DQ Disp8
	vreducepd	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512DQ
	vreducepd	zmm6, qword bcst [edx+1016], 123	 # AVX512DQ Disp8
	vreducepd	zmm6, qword bcst [edx+1024], 123	 # AVX512DQ
	vreducepd	zmm6, qword bcst [edx-1024], 123	 # AVX512DQ Disp8
	vreducepd	zmm6, qword bcst [edx-1032], 123	 # AVX512DQ
	vreduceps	zmm6, zmm5, 0xab	 # AVX512DQ
	vreduceps	zmm6{k7}, zmm5, 0xab	 # AVX512DQ
	vreduceps	zmm6{k7}{z}, zmm5, 0xab	 # AVX512DQ
	vreduceps	zmm6, zmm5{sae}, 0xab	 # AVX512DQ
	vreduceps	zmm6, zmm5, 123	 # AVX512DQ
	vreduceps	zmm6, zmm5{sae}, 123	 # AVX512DQ
	vreduceps	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512DQ
	vreduceps	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vreduceps	zmm6, dword bcst [eax], 123	 # AVX512DQ
	vreduceps	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512DQ Disp8
	vreduceps	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512DQ
	vreduceps	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512DQ Disp8
	vreduceps	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512DQ
	vreduceps	zmm6, dword bcst [edx+508], 123	 # AVX512DQ Disp8
	vreduceps	zmm6, dword bcst [edx+512], 123	 # AVX512DQ
	vreduceps	zmm6, dword bcst [edx-512], 123	 # AVX512DQ Disp8
	vreduceps	zmm6, dword bcst [edx-516], 123	 # AVX512DQ
	vreducesd	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512DQ
	vreducesd	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512DQ
	vreducesd	xmm6{k7}, xmm5, xmm4{sae}, 0xab	 # AVX512DQ
	vreducesd	xmm6{k7}, xmm5, xmm4, 123	 # AVX512DQ
	vreducesd	xmm6{k7}, xmm5, xmm4{sae}, 123	 # AVX512DQ
	vreducesd	xmm6{k7}, xmm5, QWORD PTR [ecx], 123	 # AVX512DQ
	vreducesd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vreducesd	xmm6{k7}, xmm5, QWORD PTR [edx+1016], 123	 # AVX512DQ Disp8
	vreducesd	xmm6{k7}, xmm5, QWORD PTR [edx+1024], 123	 # AVX512DQ
	vreducesd	xmm6{k7}, xmm5, QWORD PTR [edx-1024], 123	 # AVX512DQ Disp8
	vreducesd	xmm6{k7}, xmm5, QWORD PTR [edx-1032], 123	 # AVX512DQ
	vreducess	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512DQ
	vreducess	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512DQ
	vreducess	xmm6{k7}, xmm5, xmm4{sae}, 0xab	 # AVX512DQ
	vreducess	xmm6{k7}, xmm5, xmm4, 123	 # AVX512DQ
	vreducess	xmm6{k7}, xmm5, xmm4{sae}, 123	 # AVX512DQ
	vreducess	xmm6{k7}, xmm5, DWORD PTR [ecx], 123	 # AVX512DQ
	vreducess	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456], 123	 # AVX512DQ
	vreducess	xmm6{k7}, xmm5, DWORD PTR [edx+508], 123	 # AVX512DQ Disp8
	vreducess	xmm6{k7}, xmm5, DWORD PTR [edx+512], 123	 # AVX512DQ
	vreducess	xmm6{k7}, xmm5, DWORD PTR [edx-512], 123	 # AVX512DQ Disp8
	vreducess	xmm6{k7}, xmm5, DWORD PTR [edx-516], 123	 # AVX512DQ
	kandb	k5, k6, k7	 # AVX512DQ
	kandnb	k5, k6, k7	 # AVX512DQ
	korb	k5, k6, k7	 # AVX512DQ
	kxnorb	k5, k6, k7	 # AVX512DQ
	kxorb	k5, k6, k7	 # AVX512DQ
	knotb	k5, k6	 # AVX512DQ
	kortestb	k5, k6	 # AVX512DQ
	ktestw	k5, k6	 # AVX512DQ
	ktestb	k5, k6	 # AVX512DQ
	kshiftrb	k5, k6, 0xab	 # AVX512DQ
	kshiftrb	k5, k6, 123	 # AVX512DQ
	kshiftlb	k5, k6, 0xab	 # AVX512DQ
	kshiftlb	k5, k6, 123	 # AVX512DQ
	kmovb	k5, k6	 # AVX512DQ
	kmovb	k5, BYTE PTR [ecx]	 # AVX512DQ
	kmovb	k5, BYTE PTR [esp+esi*8-123456]	 # AVX512DQ
	kmovb	BYTE PTR [ecx], k5	 # AVX512DQ
	kmovb	BYTE PTR [esp+esi*8-123456], k5	 # AVX512DQ
	kmovb	k5, eax	 # AVX512DQ
	kmovb	k5, ebp	 # AVX512DQ
	kmovb	eax, k5	 # AVX512DQ
	kmovb	ebp, k5	 # AVX512DQ
	kaddw	k5, k6, k7	 # AVX512DQ
	kaddb	k5, k6, k7	 # AVX512DQ
	vextractf64x2	XMMWORD PTR [ecx], zmm6, 0xab	 # AVX512DQ
	vextractf64x2	XMMWORD PTR [ecx]{k7}, zmm6, 0xab	 # AVX512DQ
	vextractf64x2	XMMWORD PTR [ecx], zmm6, 123	 # AVX512DQ
	vextractf64x2	XMMWORD PTR [esp+esi*8-123456], zmm6, 123	 # AVX512DQ
	vextractf64x2	XMMWORD PTR [edx+2032], zmm6, 123	 # AVX512DQ Disp8
	vextractf64x2	XMMWORD PTR [edx+2048], zmm6, 123	 # AVX512DQ
	vextractf64x2	XMMWORD PTR [edx-2048], zmm6, 123	 # AVX512DQ Disp8
	vextractf64x2	XMMWORD PTR [edx-2064], zmm6, 123	 # AVX512DQ
	vextractf32x8	YMMWORD PTR [ecx], zmm6, 0xab	 # AVX512DQ
	vextractf32x8	YMMWORD PTR [ecx]{k7}, zmm6, 0xab	 # AVX512DQ
	vextractf32x8	YMMWORD PTR [ecx], zmm6, 123	 # AVX512DQ
	vextractf32x8	YMMWORD PTR [esp+esi*8-123456], zmm6, 123	 # AVX512DQ
	vextractf32x8	YMMWORD PTR [edx+4064], zmm6, 123	 # AVX512DQ Disp8
	vextractf32x8	YMMWORD PTR [edx+4096], zmm6, 123	 # AVX512DQ
	vextractf32x8	YMMWORD PTR [edx-4096], zmm6, 123	 # AVX512DQ Disp8
	vextractf32x8	YMMWORD PTR [edx-4128], zmm6, 123	 # AVX512DQ
	vextracti64x2	XMMWORD PTR [ecx], zmm6, 0xab	 # AVX512DQ
	vextracti64x2	XMMWORD PTR [ecx]{k7}, zmm6, 0xab	 # AVX512DQ
	vextracti64x2	XMMWORD PTR [ecx], zmm6, 123	 # AVX512DQ
	vextracti64x2	XMMWORD PTR [esp+esi*8-123456], zmm6, 123	 # AVX512DQ
	vextracti64x2	XMMWORD PTR [edx+2032], zmm6, 123	 # AVX512DQ Disp8
	vextracti64x2	XMMWORD PTR [edx+2048], zmm6, 123	 # AVX512DQ
	vextracti64x2	XMMWORD PTR [edx-2048], zmm6, 123	 # AVX512DQ Disp8
	vextracti64x2	XMMWORD PTR [edx-2064], zmm6, 123	 # AVX512DQ
	vextracti32x8	YMMWORD PTR [ecx], zmm6, 0xab	 # AVX512DQ
	vextracti32x8	YMMWORD PTR [ecx]{k7}, zmm6, 0xab	 # AVX512DQ
	vextracti32x8	YMMWORD PTR [ecx], zmm6, 123	 # AVX512DQ
	vextracti32x8	YMMWORD PTR [esp+esi*8-123456], zmm6, 123	 # AVX512DQ
	vextracti32x8	YMMWORD PTR [edx+4064], zmm6, 123	 # AVX512DQ Disp8
	vextracti32x8	YMMWORD PTR [edx+4096], zmm6, 123	 # AVX512DQ
	vextracti32x8	YMMWORD PTR [edx-4096], zmm6, 123	 # AVX512DQ Disp8
	vextracti32x8	YMMWORD PTR [edx-4128], zmm6, 123	 # AVX512DQ
	vcvttpd2qq	zmm6, zmm5	 # AVX512DQ
	vcvttpd2qq	zmm6{k7}, zmm5	 # AVX512DQ
	vcvttpd2qq	zmm6{k7}{z}, zmm5	 # AVX512DQ
	vcvttpd2qq	zmm6, zmm5{sae}	 # AVX512DQ
	vcvttpd2qq	zmm6, ZMMWORD PTR [ecx]	 # AVX512DQ
	vcvttpd2qq	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vcvttpd2qq	zmm6, qword bcst [eax]	 # AVX512DQ
	vcvttpd2qq	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vcvttpd2qq	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vcvttpd2qq	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vcvttpd2qq	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vcvttpd2qq	zmm6, qword bcst [edx+1016]	 # AVX512DQ Disp8
	vcvttpd2qq	zmm6, qword bcst [edx+1024]	 # AVX512DQ
	vcvttpd2qq	zmm6, qword bcst [edx-1024]	 # AVX512DQ Disp8
	vcvttpd2qq	zmm6, qword bcst [edx-1032]	 # AVX512DQ
	vcvttpd2uqq	zmm6, zmm5	 # AVX512DQ
	vcvttpd2uqq	zmm6{k7}, zmm5	 # AVX512DQ
	vcvttpd2uqq	zmm6{k7}{z}, zmm5	 # AVX512DQ
	vcvttpd2uqq	zmm6, zmm5{sae}	 # AVX512DQ
	vcvttpd2uqq	zmm6, ZMMWORD PTR [ecx]	 # AVX512DQ
	vcvttpd2uqq	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vcvttpd2uqq	zmm6, qword bcst [eax]	 # AVX512DQ
	vcvttpd2uqq	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512DQ Disp8
	vcvttpd2uqq	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512DQ
	vcvttpd2uqq	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512DQ Disp8
	vcvttpd2uqq	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512DQ
	vcvttpd2uqq	zmm6, qword bcst [edx+1016]	 # AVX512DQ Disp8
	vcvttpd2uqq	zmm6, qword bcst [edx+1024]	 # AVX512DQ
	vcvttpd2uqq	zmm6, qword bcst [edx-1024]	 # AVX512DQ Disp8
	vcvttpd2uqq	zmm6, qword bcst [edx-1032]	 # AVX512DQ
	vcvttps2qq	zmm6{k7}, ymm5	 # AVX512DQ
	vcvttps2qq	zmm6{k7}{z}, ymm5	 # AVX512DQ
	vcvttps2qq	zmm6{k7}, ymm5{sae}	 # AVX512DQ
	vcvttps2qq	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512DQ
	vcvttps2qq	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vcvttps2qq	zmm6{k7}, dword bcst [eax]	 # AVX512DQ
	vcvttps2qq	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512DQ Disp8
	vcvttps2qq	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512DQ
	vcvttps2qq	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512DQ Disp8
	vcvttps2qq	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512DQ
	vcvttps2qq	zmm6{k7}, dword bcst [edx+508]	 # AVX512DQ Disp8
	vcvttps2qq	zmm6{k7}, dword bcst [edx+512]	 # AVX512DQ
	vcvttps2qq	zmm6{k7}, dword bcst [edx-512]	 # AVX512DQ Disp8
	vcvttps2qq	zmm6{k7}, dword bcst [edx-516]	 # AVX512DQ
	vcvttps2qq	zmm6{k7}, DWORD BCST [edx+508]	 # AVX512DQ Disp8
	vcvttps2uqq	zmm6{k7}, ymm5	 # AVX512DQ
	vcvttps2uqq	zmm6{k7}{z}, ymm5	 # AVX512DQ
	vcvttps2uqq	zmm6{k7}, ymm5{sae}	 # AVX512DQ
	vcvttps2uqq	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512DQ
	vcvttps2uqq	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512DQ
	vcvttps2uqq	zmm6{k7}, dword bcst [eax]	 # AVX512DQ
	vcvttps2uqq	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512DQ Disp8
	vcvttps2uqq	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512DQ
	vcvttps2uqq	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512DQ Disp8
	vcvttps2uqq	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512DQ
	vcvttps2uqq	zmm6{k7}, dword bcst [edx+508]	 # AVX512DQ Disp8
	vcvttps2uqq	zmm6{k7}, dword bcst [edx+512]	 # AVX512DQ
	vcvttps2uqq	zmm6{k7}, dword bcst [edx-512]	 # AVX512DQ Disp8
	vcvttps2uqq	zmm6{k7}, dword bcst [edx-516]	 # AVX512DQ
	vcvttps2uqq	zmm6{k7}, DWORD BCST [edx+508]	 # AVX512DQ Disp8
	vpmovd2m	k5, zmm6	 # AVX512DQ
	vpmovq2m	k5, zmm6	 # AVX512DQ
	vpmovm2d	zmm6, k5	 # AVX512DQ
	vpmovm2q	zmm6, k5	 # AVX512DQ
