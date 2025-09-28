# Check 32bit AVX512{DQ,VL} instructions

	.allow_index_reg
	.text
_start:
	vbroadcastf64x2	(%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcastf64x2	(%ecx), %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vbroadcastf64x2	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcastf64x2	2032(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vbroadcastf64x2	2048(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcastf64x2	-2048(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vbroadcastf64x2	-2064(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcasti64x2	(%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcasti64x2	(%ecx), %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vbroadcasti64x2	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcasti64x2	2032(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vbroadcasti64x2	2048(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcasti64x2	-2048(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vbroadcasti64x2	-2064(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcastf32x2	%xmm7, %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcastf32x2	%xmm7, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vbroadcastf32x2	(%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcastf32x2	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcastf32x2	1016(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vbroadcastf32x2	1024(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcastf32x2	-1024(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vbroadcastf32x2	-1032(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	%xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	%xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtpd2qq	(%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	(%eax){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	2032(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	-2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	-2064(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	%ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	%ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtpd2qq	(%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	(%eax){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	4064(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	-4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	-4128(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	-1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	-1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	%xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	%xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtpd2uqq	(%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	(%eax){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	2032(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	-2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	-2064(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	%ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	%ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtpd2uqq	(%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	(%eax){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	4064(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	-4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	-4128(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	-1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	-1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	%xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	%xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtps2qq	(%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	(%eax){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	1016(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	1024(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	-1024(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	-1032(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	508(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	512(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	-512(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	-516(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	%xmm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	%xmm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtps2qq	(%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	(%eax){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	2032(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	2048(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	-2048(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	-2064(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	508(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	512(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	-512(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	-516(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	%xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	%xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtps2uqq	(%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	(%eax){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	1016(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	1024(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	-1024(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	-1032(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	508(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	512(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	-512(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	-516(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	%xmm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	%xmm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtps2uqq	(%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	(%eax){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	2032(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	2048(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	-2048(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	-2064(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	508(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	512(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	-512(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	-516(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	%xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	%xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtqq2pd	(%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	(%eax){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	2032(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	-2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	-2064(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	%ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	%ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtqq2pd	(%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	(%eax){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	4064(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	-4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	-4128(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	-1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	-1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2ps	%xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2ps	%xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtqq2psx	(%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2psx	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2ps	(%eax){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2psx	2032(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2psx	2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2psx	-2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2psx	-2064(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2psx	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2psx	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2psx	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2psx	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2ps	%ymm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2ps	%ymm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtqq2psy	(%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2psy	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2ps	(%eax){1to4}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2psy	4064(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2psy	4096(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2psy	-4096(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2psy	-4128(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2psy	1016(%edx){1to4}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2psy	1024(%edx){1to4}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtqq2psy	-1024(%edx){1to4}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtqq2psy	-1032(%edx){1to4}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	%xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	%xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtuqq2pd	(%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	(%eax){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	2032(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	-2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	-2064(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	%ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	%ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtuqq2pd	(%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	(%eax){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	4064(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	-4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	-4128(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	-1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	-1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2ps	%xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2ps	%xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtuqq2psx	(%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2psx	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2ps	(%eax){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2psx	2032(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psx	2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2psx	-2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psx	-2064(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2psx	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psx	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2psx	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psx	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2ps	%ymm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2ps	%ymm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvtuqq2psy	(%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2psy	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2ps	(%eax){1to4}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2psy	4064(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psy	4096(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2psy	-4096(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psy	-4128(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2psy	1016(%edx){1to4}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psy	1024(%edx){1to4}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2psy	-1024(%edx){1to4}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psy	-1032(%edx){1to4}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vextractf64x2	$0xab, %ymm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vextractf64x2	$0xab, %ymm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vextractf64x2	$123, %ymm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vextracti64x2	$0xab, %ymm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vextracti64x2	$0xab, %ymm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vextracti64x2	$123, %ymm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vfpclasspd	$0xab, %xmm6, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspd	$123, %xmm6, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspdx	$123, (%ecx), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspdx	$123, -123456(%esp,%esi,8), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspd	$123, (%eax){1to2}, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspdx	$123, 2032(%edx), %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspdx	$123, 2048(%edx), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspdx	$123, -2048(%edx), %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspdx	$123, -2064(%edx), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspdx	$123, 1016(%edx){1to2}, %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspdx	$123, 1024(%edx){1to2}, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspdx	$123, -1024(%edx){1to2}, %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspdx	$123, -1032(%edx){1to2}, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspd	$0xab, %ymm6, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspd	$123, %ymm6, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspdy	$123, (%ecx), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspdy	$123, -123456(%esp,%esi,8), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspd	$123, (%eax){1to4}, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspdy	$123, 4064(%edx), %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspdy	$123, 4096(%edx), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspdy	$123, -4096(%edx), %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspdy	$123, -4128(%edx), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspdy	$123, 1016(%edx){1to4}, %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspdy	$123, 1024(%edx){1to4}, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspdy	$123, -1024(%edx){1to4}, %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspdy	$123, -1032(%edx){1to4}, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclassps	$0xab, %xmm6, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclassps	$123, %xmm6, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspsx	$123, (%ecx), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspsx	$123, -123456(%esp,%esi,8), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclassps	$123, (%eax){1to4}, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspsx	$123, 2032(%edx), %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspsx	$123, 2048(%edx), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspsx	$123, -2048(%edx), %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspsx	$123, -2064(%edx), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspsx	$123, 508(%edx){1to4}, %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspsx	$123, 512(%edx){1to4}, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspsx	$123, -512(%edx){1to4}, %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspsx	$123, -516(%edx){1to4}, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclassps	$0xab, %ymm6, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclassps	$123, %ymm6, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspsy	$123, (%ecx), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspsy	$123, -123456(%esp,%esi,8), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclassps	$123, (%eax){1to8}, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspsy	$123, 4064(%edx), %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspsy	$123, 4096(%edx), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspsy	$123, -4096(%edx), %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspsy	$123, -4128(%edx), %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspsy	$123, 508(%edx){1to8}, %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspsy	$123, 512(%edx){1to8}, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspsy	$123, -512(%edx){1to8}, %k5{%k7}	 # AVX512{DQ,VL} Disp8
	vfpclasspsy	$123, -516(%edx){1to8}, %k5{%k7}	 # AVX512{DQ,VL}
	vinsertf64x2	$0xab, %xmm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vinsertf64x2	$0xab, %xmm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vinsertf64x2	$123, %xmm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vinsertf64x2	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vinsertf64x2	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vinsertf64x2	$123, 2032(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vinsertf64x2	$123, 2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vinsertf64x2	$123, -2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vinsertf64x2	$123, -2064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vinserti64x2	$0xab, %xmm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vinserti64x2	$0xab, %xmm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vinserti64x2	$123, %xmm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vinserti64x2	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vinserti64x2	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vinserti64x2	$123, 2032(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vinserti64x2	$123, 2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vinserti64x2	$123, -2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vinserti64x2	$123, -2064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcasti32x2	%xmm7, %xmm6{%k7}	 # AVX512{DQ,VL}
	vbroadcasti32x2	%xmm7, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vbroadcasti32x2	(%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vbroadcasti32x2	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vbroadcasti32x2	1016(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	1024(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vbroadcasti32x2	-1024(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	-1032(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vbroadcasti32x2	%xmm7, %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcasti32x2	%xmm7, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vbroadcasti32x2	(%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcasti32x2	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcasti32x2	1016(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	1024(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vbroadcasti32x2	-1024(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	-1032(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vpmullq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vpmullq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vpmullq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vpmullq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vpmullq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vpmullq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vpmullq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vpmullq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vpmullq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vpmullq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vpmullq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vrangepd	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$123, (%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, 2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$123, -2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, -2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$123, 1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, 1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$123, -1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, -1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vrangepd	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$123, (%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$123, 1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, 1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangepd	$123, -1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, -1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vrangeps	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$123, (%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, 2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$123, -2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, -2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$123, 508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, 512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$123, -512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, -516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vrangeps	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$123, (%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$123, 508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, 512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vrangeps	$123, -512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, -516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandpd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vandpd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandpd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandpd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandpd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandpd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandpd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandpd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandpd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandpd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandpd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandpd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vandpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vandps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vandps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vandnpd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnpd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnpd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnpd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnpd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vandnpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vandnps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vandnps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vandnps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vandnps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vandnps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorpd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vorpd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorpd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorpd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorpd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorpd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorpd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorpd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorpd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorpd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorpd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorpd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vorpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vorps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vorps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vorps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vorps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vorps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vxorpd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorpd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorpd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorpd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorpd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vxorpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vxorps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vxorps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vxorps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vxorps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vxorps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vreducepd	$123, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$123, (%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$123, (%eax){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$123, 1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, 1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$123, -1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, -1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vreducepd	$123, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$123, (%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$123, (%eax){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$123, 1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, 1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vreducepd	$123, -1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, -1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vreduceps	$123, %xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$123, (%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$123, (%eax){1to4}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$123, 508(%edx){1to4}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, 512(%edx){1to4}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$123, -512(%edx){1to4}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, -516(%edx){1to4}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vreduceps	$123, %ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$123, (%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$123, (%eax){1to8}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$123, 508(%edx){1to8}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, 512(%edx){1to8}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vreduceps	$123, -512(%edx){1to8}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, -516(%edx){1to8}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vextractf64x2	$0xab, %ymm5, (%ecx){%k7}	 # AVX512{DQ,VL}
	vextractf64x2	$123, %ymm5, (%ecx){%k7}	 # AVX512{DQ,VL}
	vextractf64x2	$123, %ymm5, -123456(%esp,%esi,8){%k7}	 # AVX512{DQ,VL}
	vextractf64x2	$123, %ymm5, 2032(%edx){%k7}	 # AVX512{DQ,VL} Disp8
	vextractf64x2	$123, %ymm5, 2048(%edx){%k7}	 # AVX512{DQ,VL}
	vextractf64x2	$123, %ymm5, -2048(%edx){%k7}	 # AVX512{DQ,VL} Disp8
	vextractf64x2	$123, %ymm5, -2064(%edx){%k7}	 # AVX512{DQ,VL}
	vextracti64x2	$0xab, %ymm5, (%ecx){%k7}	 # AVX512{DQ,VL}
	vextracti64x2	$123, %ymm5, (%ecx){%k7}	 # AVX512{DQ,VL}
	vextracti64x2	$123, %ymm5, -123456(%esp,%esi,8){%k7}	 # AVX512{DQ,VL}
	vextracti64x2	$123, %ymm5, 2032(%edx){%k7}	 # AVX512{DQ,VL} Disp8
	vextracti64x2	$123, %ymm5, 2048(%edx){%k7}	 # AVX512{DQ,VL}
	vextracti64x2	$123, %ymm5, -2048(%edx){%k7}	 # AVX512{DQ,VL} Disp8
	vextracti64x2	$123, %ymm5, -2064(%edx){%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	%xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	%xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvttpd2qq	(%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	(%eax){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	2032(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	-2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	-2064(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	%ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	%ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvttpd2qq	(%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	(%eax){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	4064(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	-4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	-4128(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	-1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	-1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	%xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	%xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvttpd2uqq	(%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	(%eax){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	2032(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	-2048(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	-2064(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	%ymm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	%ymm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvttpd2uqq	(%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	(%eax){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	4064(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	-4096(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	-4128(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	-1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	-1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	%xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	%xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvttps2qq	(%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	(%eax){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	1016(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	1024(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	-1024(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	-1032(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	508(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	512(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	-512(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	-516(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	%xmm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	%xmm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvttps2qq	(%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	(%eax){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	2032(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	2048(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	-2048(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	-2064(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	508(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	512(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	-512(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	-516(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	%xmm5, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	%xmm5, %xmm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvttps2uqq	(%ecx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	(%eax){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	1016(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	1024(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	-1024(%edx), %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	-1032(%edx), %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	508(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	512(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	-512(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	-516(%edx){1to2}, %xmm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	%xmm5, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	%xmm5, %ymm6{%k7}{z}	 # AVX512{DQ,VL}
	vcvttps2uqq	(%ecx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	(%eax){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	2032(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	2048(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	-2048(%edx), %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	-2064(%edx), %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	508(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	512(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	-512(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	-516(%edx){1to4}, %ymm6{%k7}	 # AVX512{DQ,VL}
	vpmovd2m	%xmm6, %k5	 # AVX512{DQ,VL}
	vpmovd2m	%ymm6, %k5	 # AVX512{DQ,VL}
	vpmovq2m	%xmm6, %k5	 # AVX512{DQ,VL}
	vpmovq2m	%ymm6, %k5	 # AVX512{DQ,VL}
	vpmovm2d	%k5, %xmm6	 # AVX512{DQ,VL}
	vpmovm2d	%k5, %ymm6	 # AVX512{DQ,VL}
	vpmovm2q	%k5, %xmm6	 # AVX512{DQ,VL}
	vpmovm2q	%k5, %ymm6	 # AVX512{DQ,VL}

	.intel_syntax noprefix
	vbroadcastf64x2	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vbroadcastf64x2	ymm6{k7}{z}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vbroadcastf64x2	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vbroadcastf64x2	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vbroadcastf64x2	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vbroadcastf64x2	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vbroadcastf64x2	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vbroadcasti64x2	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vbroadcasti64x2	ymm6{k7}{z}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vbroadcasti64x2	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vbroadcasti64x2	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vbroadcasti64x2	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vbroadcasti64x2	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vbroadcasti64x2	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vbroadcastf32x2	ymm6{k7}, xmm7	 # AVX512{DQ,VL}
	vbroadcastf32x2	ymm6{k7}{z}, xmm7	 # AVX512{DQ,VL}
	vbroadcastf32x2	ymm6{k7}, QWORD PTR [ecx]	 # AVX512{DQ,VL}
	vbroadcastf32x2	ymm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vbroadcastf32x2	ymm6{k7}, QWORD PTR [edx+1016]	 # AVX512{DQ,VL} Disp8
	vbroadcastf32x2	ymm6{k7}, QWORD PTR [edx+1024]	 # AVX512{DQ,VL}
	vbroadcastf32x2	ymm6{k7}, QWORD PTR [edx-1024]	 # AVX512{DQ,VL} Disp8
	vbroadcastf32x2	ymm6{k7}, QWORD PTR [edx-1032]	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm6{k7}, [eax]{1to2}	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm6{k7}, [edx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	xmm6{k7}, [edx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm6{k7}, [edx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	xmm6{k7}, [edx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm6{k7}, ymm5	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm6{k7}{z}, ymm5	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm6{k7}, [eax]{1to4}	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm6{k7}, [edx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	ymm6{k7}, [edx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm6{k7}, [edx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	ymm6{k7}, [edx-1032]{1to4}	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm6{k7}, [eax]{1to2}	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm6{k7}, [edx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	xmm6{k7}, [edx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm6{k7}, [edx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	xmm6{k7}, [edx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm6{k7}, ymm5	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm6{k7}{z}, ymm5	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm6{k7}, [eax]{1to4}	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm6{k7}, [edx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	ymm6{k7}, [edx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm6{k7}, [edx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	ymm6{k7}, [edx-1032]{1to4}	 # AVX512{DQ,VL}
	vcvtps2qq	xmm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvtps2qq	xmm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvtps2qq	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtps2qq	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtps2qq	xmm6{k7}, [eax]{1to2}	 # AVX512{DQ,VL}
	vcvtps2qq	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{DQ,VL}
	vcvtps2qq	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{DQ,VL}
	vcvtps2qq	xmm6{k7}, [edx+508]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	xmm6{k7}, [edx+512]{1to2}	 # AVX512{DQ,VL}
	vcvtps2qq	xmm6{k7}, [edx-512]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	xmm6{k7}, [edx-516]{1to2}	 # AVX512{DQ,VL}
	vcvtps2qq	xmm6{k7}, DWORD BCST [edx+508]	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	ymm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvtps2qq	ymm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvtps2qq	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtps2qq	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtps2qq	ymm6{k7}, [eax]{1to4}	 # AVX512{DQ,VL}
	vcvtps2qq	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vcvtps2qq	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vcvtps2qq	ymm6{k7}, [edx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	ymm6{k7}, [edx+512]{1to4}	 # AVX512{DQ,VL}
	vcvtps2qq	ymm6{k7}, [edx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	ymm6{k7}, [edx-516]{1to4}	 # AVX512{DQ,VL}
	vcvtps2qq	ymm6{k7}, DWORD BCST [edx+508]	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	xmm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm6{k7}, [eax]{1to2}	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm6{k7}, [edx+508]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	xmm6{k7}, [edx+512]{1to2}	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm6{k7}, [edx-512]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	xmm6{k7}, [edx-516]{1to2}	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm6{k7}, DWORD BCST [edx+508]	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	ymm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm6{k7}, [eax]{1to4}	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm6{k7}, [edx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	ymm6{k7}, [edx+512]{1to4}	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm6{k7}, [edx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	ymm6{k7}, [edx-516]{1to4}	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm6{k7}, DWORD BCST [edx+508]	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	xmm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm6{k7}, [eax]{1to2}	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm6{k7}, [edx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	xmm6{k7}, [edx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm6{k7}, [edx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	xmm6{k7}, [edx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm6{k7}, ymm5	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm6{k7}{z}, ymm5	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm6{k7}, [eax]{1to4}	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm6{k7}, [edx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	ymm6{k7}, [edx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm6{k7}, [edx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	ymm6{k7}, [edx-1032]{1to4}	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, [eax]{1to2}	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, QWORD BCST [edx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm6{k7}, QWORD BCST [edx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, QWORD BCST [edx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm6{k7}, QWORD BCST [edx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, ymm5	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}{z}, ymm5	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, [eax]{1to4}	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, QWORD BCST [edx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm6{k7}, QWORD BCST [edx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm6{k7}, QWORD BCST [edx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm6{k7}, QWORD BCST [edx-1032]{1to4}	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm6{k7}, [eax]{1to2}	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm6{k7}, [edx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	xmm6{k7}, [edx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm6{k7}, [edx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	xmm6{k7}, [edx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm6{k7}, ymm5	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm6{k7}{z}, ymm5	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm6{k7}, [eax]{1to4}	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm6{k7}, [edx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	ymm6{k7}, [edx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm6{k7}, [edx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	ymm6{k7}, [edx-1032]{1to4}	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, [eax]{1to2}	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, QWORD BCST [edx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm6{k7}, QWORD BCST [edx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, QWORD BCST [edx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm6{k7}, QWORD BCST [edx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, ymm5	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}{z}, ymm5	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, [eax]{1to4}	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, QWORD BCST [edx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm6{k7}, QWORD BCST [edx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm6{k7}, QWORD BCST [edx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm6{k7}, QWORD BCST [edx-1032]{1to4}	 # AVX512{DQ,VL}
	vextractf64x2	xmm6{k7}, ymm5, 0xab	 # AVX512{DQ,VL}
	vextractf64x2	xmm6{k7}{z}, ymm5, 0xab	 # AVX512{DQ,VL}
	vextractf64x2	xmm6{k7}, ymm5, 123	 # AVX512{DQ,VL}
	vextracti64x2	xmm6{k7}, ymm5, 0xab	 # AVX512{DQ,VL}
	vextracti64x2	xmm6{k7}{z}, ymm5, 0xab	 # AVX512{DQ,VL}
	vextracti64x2	xmm6{k7}, ymm5, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, xmm6, 0xab	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, xmm6, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, XMMWORD PTR [ecx], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, [eax]{1to2}, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, QWORD BCST [edx+1016]{1to2}, 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5{k7}, QWORD BCST [edx+1024]{1to2}, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, QWORD BCST [edx-1024]{1to2}, 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5{k7}, QWORD BCST [edx-1032]{1to2}, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, ymm6, 0xab	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, ymm6, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, YMMWORD PTR [ecx], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, [eax]{1to4}, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, QWORD BCST [edx+1016]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5{k7}, QWORD BCST [edx+1024]{1to4}, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, QWORD BCST [edx-1024]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5{k7}, QWORD BCST [edx-1032]{1to4}, 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, xmm6, 0xab	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, xmm6, 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, XMMWORD PTR [ecx], 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, [eax]{1to4}, 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, DWORD BCST [edx+508]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5{k7}, DWORD BCST [edx+512]{1to4}, 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, DWORD BCST [edx-512]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5{k7}, DWORD BCST [edx-516]{1to4}, 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, ymm6, 0xab	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, ymm6, 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, YMMWORD PTR [ecx], 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, [eax]{1to8}, 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, DWORD BCST [edx+508]{1to8}, 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5{k7}, DWORD BCST [edx+512]{1to8}, 123	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, DWORD BCST [edx-512]{1to8}, 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5{k7}, DWORD BCST [edx-516]{1to8}, 123	 # AVX512{DQ,VL}
	vinsertf64x2	ymm6{k7}, ymm5, xmm4, 0xab	 # AVX512{DQ,VL}
	vinsertf64x2	ymm6{k7}{z}, ymm5, xmm4, 0xab	 # AVX512{DQ,VL}
	vinsertf64x2	ymm6{k7}, ymm5, xmm4, 123	 # AVX512{DQ,VL}
	vinsertf64x2	ymm6{k7}, ymm5, XMMWORD PTR [ecx], 123	 # AVX512{DQ,VL}
	vinsertf64x2	ymm6{k7}, ymm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{DQ,VL}
	vinsertf64x2	ymm6{k7}, ymm5, XMMWORD PTR [edx+2032], 123	 # AVX512{DQ,VL} Disp8
	vinsertf64x2	ymm6{k7}, ymm5, XMMWORD PTR [edx+2048], 123	 # AVX512{DQ,VL}
	vinsertf64x2	ymm6{k7}, ymm5, XMMWORD PTR [edx-2048], 123	 # AVX512{DQ,VL} Disp8
	vinsertf64x2	ymm6{k7}, ymm5, XMMWORD PTR [edx-2064], 123	 # AVX512{DQ,VL}
	vinserti64x2	ymm6{k7}, ymm5, xmm4, 0xab	 # AVX512{DQ,VL}
	vinserti64x2	ymm6{k7}{z}, ymm5, xmm4, 0xab	 # AVX512{DQ,VL}
	vinserti64x2	ymm6{k7}, ymm5, xmm4, 123	 # AVX512{DQ,VL}
	vinserti64x2	ymm6{k7}, ymm5, XMMWORD PTR [ecx], 123	 # AVX512{DQ,VL}
	vinserti64x2	ymm6{k7}, ymm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{DQ,VL}
	vinserti64x2	ymm6{k7}, ymm5, XMMWORD PTR [edx+2032], 123	 # AVX512{DQ,VL} Disp8
	vinserti64x2	ymm6{k7}, ymm5, XMMWORD PTR [edx+2048], 123	 # AVX512{DQ,VL}
	vinserti64x2	ymm6{k7}, ymm5, XMMWORD PTR [edx-2048], 123	 # AVX512{DQ,VL} Disp8
	vinserti64x2	ymm6{k7}, ymm5, XMMWORD PTR [edx-2064], 123	 # AVX512{DQ,VL}
	vbroadcasti32x2	xmm6{k7}, xmm7	 # AVX512{DQ,VL}
	vbroadcasti32x2	xmm6{k7}{z}, xmm7	 # AVX512{DQ,VL}
	vbroadcasti32x2	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{DQ,VL}
	vbroadcasti32x2	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vbroadcasti32x2	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{DQ,VL}
	vbroadcasti32x2	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{DQ,VL}
	vbroadcasti32x2	ymm6{k7}, xmm7	 # AVX512{DQ,VL}
	vbroadcasti32x2	ymm6{k7}{z}, xmm7	 # AVX512{DQ,VL}
	vbroadcasti32x2	ymm6{k7}, QWORD PTR [ecx]	 # AVX512{DQ,VL}
	vbroadcasti32x2	ymm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vbroadcasti32x2	ymm6{k7}, QWORD PTR [edx+1016]	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	ymm6{k7}, QWORD PTR [edx+1024]	 # AVX512{DQ,VL}
	vbroadcasti32x2	ymm6{k7}, QWORD PTR [edx-1024]	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	ymm6{k7}, QWORD PTR [edx-1032]	 # AVX512{DQ,VL}
	vpmullq	xmm6{k7}, xmm5, xmm4	 # AVX512{DQ,VL}
	vpmullq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{DQ,VL}
	vpmullq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vpmullq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vpmullq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{DQ,VL}
	vpmullq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vpmullq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vpmullq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vpmullq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vpmullq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vpmullq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{DQ,VL}
	vpmullq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vpmullq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{DQ,VL}
	vpmullq	ymm6{k7}, ymm5, ymm4	 # AVX512{DQ,VL}
	vpmullq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{DQ,VL}
	vpmullq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vpmullq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vpmullq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{DQ,VL}
	vpmullq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vpmullq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vpmullq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vpmullq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vpmullq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vpmullq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{DQ,VL}
	vpmullq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vpmullq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{DQ,VL}
	vrangepd	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{DQ,VL}
	vrangepd	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{DQ,VL}
	vrangepd	xmm6{k7}, xmm5, xmm4, 123	 # AVX512{DQ,VL}
	vrangepd	xmm6{k7}, xmm5, XMMWORD PTR [ecx], 123	 # AVX512{DQ,VL}
	vrangepd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{DQ,VL}
	vrangepd	xmm6{k7}, xmm5, [eax]{1to2}, 123	 # AVX512{DQ,VL}
	vrangepd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{DQ,VL} Disp8
	vrangepd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048], 123	 # AVX512{DQ,VL}
	vrangepd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048], 123	 # AVX512{DQ,VL} Disp8
	vrangepd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064], 123	 # AVX512{DQ,VL}
	vrangepd	xmm6{k7}, xmm5, [edx+1016]{1to2}, 123	 # AVX512{DQ,VL} Disp8
	vrangepd	xmm6{k7}, xmm5, [edx+1024]{1to2}, 123	 # AVX512{DQ,VL}
	vrangepd	xmm6{k7}, xmm5, [edx-1024]{1to2}, 123	 # AVX512{DQ,VL} Disp8
	vrangepd	xmm6{k7}, xmm5, [edx-1032]{1to2}, 123	 # AVX512{DQ,VL}
	vrangepd	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{DQ,VL}
	vrangepd	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{DQ,VL}
	vrangepd	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{DQ,VL}
	vrangepd	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{DQ,VL}
	vrangepd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{DQ,VL}
	vrangepd	ymm6{k7}, ymm5, [eax]{1to4}, 123	 # AVX512{DQ,VL}
	vrangepd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{DQ,VL} Disp8
	vrangepd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{DQ,VL}
	vrangepd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{DQ,VL} Disp8
	vrangepd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{DQ,VL}
	vrangepd	ymm6{k7}, ymm5, [edx+1016]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vrangepd	ymm6{k7}, ymm5, [edx+1024]{1to4}, 123	 # AVX512{DQ,VL}
	vrangepd	ymm6{k7}, ymm5, [edx-1024]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vrangepd	ymm6{k7}, ymm5, [edx-1032]{1to4}, 123	 # AVX512{DQ,VL}
	vrangeps	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{DQ,VL}
	vrangeps	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{DQ,VL}
	vrangeps	xmm6{k7}, xmm5, xmm4, 123	 # AVX512{DQ,VL}
	vrangeps	xmm6{k7}, xmm5, XMMWORD PTR [ecx], 123	 # AVX512{DQ,VL}
	vrangeps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{DQ,VL}
	vrangeps	xmm6{k7}, xmm5, [eax]{1to4}, 123	 # AVX512{DQ,VL}
	vrangeps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{DQ,VL} Disp8
	vrangeps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048], 123	 # AVX512{DQ,VL}
	vrangeps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048], 123	 # AVX512{DQ,VL} Disp8
	vrangeps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064], 123	 # AVX512{DQ,VL}
	vrangeps	xmm6{k7}, xmm5, [edx+508]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vrangeps	xmm6{k7}, xmm5, [edx+512]{1to4}, 123	 # AVX512{DQ,VL}
	vrangeps	xmm6{k7}, xmm5, [edx-512]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vrangeps	xmm6{k7}, xmm5, [edx-516]{1to4}, 123	 # AVX512{DQ,VL}
	vrangeps	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{DQ,VL}
	vrangeps	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{DQ,VL}
	vrangeps	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{DQ,VL}
	vrangeps	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{DQ,VL}
	vrangeps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{DQ,VL}
	vrangeps	ymm6{k7}, ymm5, [eax]{1to8}, 123	 # AVX512{DQ,VL}
	vrangeps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{DQ,VL} Disp8
	vrangeps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{DQ,VL}
	vrangeps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{DQ,VL} Disp8
	vrangeps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{DQ,VL}
	vrangeps	ymm6{k7}, ymm5, [edx+508]{1to8}, 123	 # AVX512{DQ,VL} Disp8
	vrangeps	ymm6{k7}, ymm5, [edx+512]{1to8}, 123	 # AVX512{DQ,VL}
	vrangeps	ymm6{k7}, ymm5, [edx-512]{1to8}, 123	 # AVX512{DQ,VL} Disp8
	vrangeps	ymm6{k7}, ymm5, [edx-516]{1to8}, 123	 # AVX512{DQ,VL}
	vandpd	xmm6{k7}, xmm5, xmm4	 # AVX512{DQ,VL}
	vandpd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{DQ,VL}
	vandpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vandpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vandpd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{DQ,VL}
	vandpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vandpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vandpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vandpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vandpd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vandpd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{DQ,VL}
	vandpd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vandpd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{DQ,VL}
	vandpd	ymm6{k7}, ymm5, ymm4	 # AVX512{DQ,VL}
	vandpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{DQ,VL}
	vandpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vandpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vandpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{DQ,VL}
	vandpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vandpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vandpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vandpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vandpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vandpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{DQ,VL}
	vandpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vandpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{DQ,VL}
	vandps	xmm6{k7}, xmm5, xmm4	 # AVX512{DQ,VL}
	vandps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{DQ,VL}
	vandps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vandps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vandps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{DQ,VL}
	vandps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vandps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vandps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vandps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vandps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vandps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{DQ,VL}
	vandps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vandps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{DQ,VL}
	vandps	ymm6{k7}, ymm5, ymm4	 # AVX512{DQ,VL}
	vandps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{DQ,VL}
	vandps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vandps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vandps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{DQ,VL}
	vandps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vandps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vandps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vandps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vandps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{DQ,VL} Disp8
	vandps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{DQ,VL}
	vandps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{DQ,VL} Disp8
	vandps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{DQ,VL}
	vandnpd	xmm6{k7}, xmm5, xmm4	 # AVX512{DQ,VL}
	vandnpd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{DQ,VL}
	vandnpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vandnpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vandnpd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{DQ,VL}
	vandnpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vandnpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vandnpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vandnpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vandnpd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vandnpd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{DQ,VL}
	vandnpd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vandnpd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{DQ,VL}
	vandnpd	ymm6{k7}, ymm5, ymm4	 # AVX512{DQ,VL}
	vandnpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{DQ,VL}
	vandnpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vandnpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vandnpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{DQ,VL}
	vandnpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vandnpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vandnpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vandnpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vandnpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vandnpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{DQ,VL}
	vandnpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vandnpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{DQ,VL}
	vandnps	xmm6{k7}, xmm5, xmm4	 # AVX512{DQ,VL}
	vandnps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{DQ,VL}
	vandnps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vandnps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vandnps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{DQ,VL}
	vandnps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vandnps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vandnps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vandnps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vandnps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vandnps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{DQ,VL}
	vandnps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vandnps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{DQ,VL}
	vandnps	ymm6{k7}, ymm5, ymm4	 # AVX512{DQ,VL}
	vandnps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{DQ,VL}
	vandnps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vandnps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vandnps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{DQ,VL}
	vandnps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vandnps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vandnps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vandnps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vandnps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{DQ,VL} Disp8
	vandnps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{DQ,VL}
	vandnps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{DQ,VL} Disp8
	vandnps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{DQ,VL}
	vorpd	xmm6{k7}, xmm5, xmm4	 # AVX512{DQ,VL}
	vorpd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{DQ,VL}
	vorpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vorpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vorpd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{DQ,VL}
	vorpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vorpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vorpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vorpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vorpd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vorpd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{DQ,VL}
	vorpd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vorpd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{DQ,VL}
	vorpd	ymm6{k7}, ymm5, ymm4	 # AVX512{DQ,VL}
	vorpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{DQ,VL}
	vorpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vorpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vorpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{DQ,VL}
	vorpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vorpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vorpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vorpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vorpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vorpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{DQ,VL}
	vorpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vorpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{DQ,VL}
	vorps	xmm6{k7}, xmm5, xmm4	 # AVX512{DQ,VL}
	vorps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{DQ,VL}
	vorps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vorps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vorps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{DQ,VL}
	vorps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vorps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vorps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vorps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vorps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vorps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{DQ,VL}
	vorps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vorps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{DQ,VL}
	vorps	ymm6{k7}, ymm5, ymm4	 # AVX512{DQ,VL}
	vorps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{DQ,VL}
	vorps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vorps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vorps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{DQ,VL}
	vorps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vorps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vorps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vorps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vorps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{DQ,VL} Disp8
	vorps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{DQ,VL}
	vorps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{DQ,VL} Disp8
	vorps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{DQ,VL}
	vxorpd	xmm6{k7}, xmm5, xmm4	 # AVX512{DQ,VL}
	vxorpd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{DQ,VL}
	vxorpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vxorpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vxorpd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{DQ,VL}
	vxorpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vxorpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vxorpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vxorpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vxorpd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vxorpd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{DQ,VL}
	vxorpd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vxorpd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{DQ,VL}
	vxorpd	ymm6{k7}, ymm5, ymm4	 # AVX512{DQ,VL}
	vxorpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{DQ,VL}
	vxorpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vxorpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vxorpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{DQ,VL}
	vxorpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vxorpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vxorpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vxorpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vxorpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vxorpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{DQ,VL}
	vxorpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vxorpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{DQ,VL}
	vxorps	xmm6{k7}, xmm5, xmm4	 # AVX512{DQ,VL}
	vxorps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{DQ,VL}
	vxorps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vxorps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vxorps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{DQ,VL}
	vxorps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vxorps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vxorps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vxorps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vxorps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vxorps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{DQ,VL}
	vxorps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vxorps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{DQ,VL}
	vxorps	ymm6{k7}, ymm5, ymm4	 # AVX512{DQ,VL}
	vxorps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{DQ,VL}
	vxorps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vxorps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vxorps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{DQ,VL}
	vxorps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vxorps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vxorps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vxorps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vxorps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{DQ,VL} Disp8
	vxorps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{DQ,VL}
	vxorps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{DQ,VL} Disp8
	vxorps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{DQ,VL}
	vreducepd	xmm6{k7}, xmm5, 0xab	 # AVX512{DQ,VL}
	vreducepd	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{DQ,VL}
	vreducepd	xmm6{k7}, xmm5, 123	 # AVX512{DQ,VL}
	vreducepd	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{DQ,VL}
	vreducepd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{DQ,VL}
	vreducepd	xmm6{k7}, [eax]{1to2}, 123	 # AVX512{DQ,VL}
	vreducepd	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{DQ,VL} Disp8
	vreducepd	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{DQ,VL}
	vreducepd	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{DQ,VL} Disp8
	vreducepd	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{DQ,VL}
	vreducepd	xmm6{k7}, [edx+1016]{1to2}, 123	 # AVX512{DQ,VL} Disp8
	vreducepd	xmm6{k7}, [edx+1024]{1to2}, 123	 # AVX512{DQ,VL}
	vreducepd	xmm6{k7}, [edx-1024]{1to2}, 123	 # AVX512{DQ,VL} Disp8
	vreducepd	xmm6{k7}, [edx-1032]{1to2}, 123	 # AVX512{DQ,VL}
	vreducepd	ymm6{k7}, ymm5, 0xab	 # AVX512{DQ,VL}
	vreducepd	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{DQ,VL}
	vreducepd	ymm6{k7}, ymm5, 123	 # AVX512{DQ,VL}
	vreducepd	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{DQ,VL}
	vreducepd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{DQ,VL}
	vreducepd	ymm6{k7}, [eax]{1to4}, 123	 # AVX512{DQ,VL}
	vreducepd	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{DQ,VL} Disp8
	vreducepd	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{DQ,VL}
	vreducepd	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{DQ,VL} Disp8
	vreducepd	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{DQ,VL}
	vreducepd	ymm6{k7}, [edx+1016]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vreducepd	ymm6{k7}, [edx+1024]{1to4}, 123	 # AVX512{DQ,VL}
	vreducepd	ymm6{k7}, [edx-1024]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vreducepd	ymm6{k7}, [edx-1032]{1to4}, 123	 # AVX512{DQ,VL}
	vreduceps	xmm6{k7}, xmm5, 0xab	 # AVX512{DQ,VL}
	vreduceps	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{DQ,VL}
	vreduceps	xmm6{k7}, xmm5, 123	 # AVX512{DQ,VL}
	vreduceps	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{DQ,VL}
	vreduceps	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{DQ,VL}
	vreduceps	xmm6{k7}, [eax]{1to4}, 123	 # AVX512{DQ,VL}
	vreduceps	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{DQ,VL} Disp8
	vreduceps	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{DQ,VL}
	vreduceps	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{DQ,VL} Disp8
	vreduceps	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{DQ,VL}
	vreduceps	xmm6{k7}, [edx+508]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vreduceps	xmm6{k7}, [edx+512]{1to4}, 123	 # AVX512{DQ,VL}
	vreduceps	xmm6{k7}, [edx-512]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vreduceps	xmm6{k7}, [edx-516]{1to4}, 123	 # AVX512{DQ,VL}
	vreduceps	ymm6{k7}, ymm5, 0xab	 # AVX512{DQ,VL}
	vreduceps	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{DQ,VL}
	vreduceps	ymm6{k7}, ymm5, 123	 # AVX512{DQ,VL}
	vreduceps	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{DQ,VL}
	vreduceps	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{DQ,VL}
	vreduceps	ymm6{k7}, [eax]{1to8}, 123	 # AVX512{DQ,VL}
	vreduceps	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{DQ,VL} Disp8
	vreduceps	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{DQ,VL}
	vreduceps	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{DQ,VL} Disp8
	vreduceps	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{DQ,VL}
	vreduceps	ymm6{k7}, [edx+508]{1to8}, 123	 # AVX512{DQ,VL} Disp8
	vreduceps	ymm6{k7}, [edx+512]{1to8}, 123	 # AVX512{DQ,VL}
	vreduceps	ymm6{k7}, [edx-512]{1to8}, 123	 # AVX512{DQ,VL} Disp8
	vreduceps	ymm6{k7}, [edx-516]{1to8}, 123	 # AVX512{DQ,VL}
	vextractf64x2	XMMWORD PTR [ecx]{k7}, ymm5, 0xab	 # AVX512{DQ,VL}
	vextractf64x2	XMMWORD PTR [ecx]{k7}, ymm5, 123	 # AVX512{DQ,VL}
	vextractf64x2	XMMWORD PTR [esp+esi*8-123456]{k7}, ymm5, 123	 # AVX512{DQ,VL}
	vextractf64x2	XMMWORD PTR [edx+2032]{k7}, ymm5, 123	 # AVX512{DQ,VL} Disp8
	vextractf64x2	XMMWORD PTR [edx+2048]{k7}, ymm5, 123	 # AVX512{DQ,VL}
	vextractf64x2	XMMWORD PTR [edx-2048]{k7}, ymm5, 123	 # AVX512{DQ,VL} Disp8
	vextractf64x2	XMMWORD PTR [edx-2064]{k7}, ymm5, 123	 # AVX512{DQ,VL}
	vextracti64x2	XMMWORD PTR [ecx]{k7}, ymm5, 0xab	 # AVX512{DQ,VL}
	vextracti64x2	XMMWORD PTR [ecx]{k7}, ymm5, 123	 # AVX512{DQ,VL}
	vextracti64x2	XMMWORD PTR [esp+esi*8-123456]{k7}, ymm5, 123	 # AVX512{DQ,VL}
	vextracti64x2	XMMWORD PTR [edx+2032]{k7}, ymm5, 123	 # AVX512{DQ,VL} Disp8
	vextracti64x2	XMMWORD PTR [edx+2048]{k7}, ymm5, 123	 # AVX512{DQ,VL}
	vextracti64x2	XMMWORD PTR [edx-2048]{k7}, ymm5, 123	 # AVX512{DQ,VL} Disp8
	vextracti64x2	XMMWORD PTR [edx-2064]{k7}, ymm5, 123	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm6{k7}, [eax]{1to2}	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm6{k7}, [edx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	xmm6{k7}, [edx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm6{k7}, [edx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	xmm6{k7}, [edx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm6{k7}, ymm5	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm6{k7}{z}, ymm5	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm6{k7}, [eax]{1to4}	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm6{k7}, [edx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	ymm6{k7}, [edx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm6{k7}, [edx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	ymm6{k7}, [edx-1032]{1to4}	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm6{k7}, [eax]{1to2}	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm6{k7}, [edx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	xmm6{k7}, [edx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm6{k7}, [edx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	xmm6{k7}, [edx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm6{k7}, ymm5	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm6{k7}{z}, ymm5	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm6{k7}, [eax]{1to4}	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm6{k7}, [edx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	ymm6{k7}, [edx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm6{k7}, [edx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	ymm6{k7}, [edx-1032]{1to4}	 # AVX512{DQ,VL}
	vcvttps2qq	xmm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvttps2qq	xmm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvttps2qq	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvttps2qq	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvttps2qq	xmm6{k7}, [eax]{1to2}	 # AVX512{DQ,VL}
	vcvttps2qq	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{DQ,VL}
	vcvttps2qq	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{DQ,VL}
	vcvttps2qq	xmm6{k7}, [edx+508]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	xmm6{k7}, [edx+512]{1to2}	 # AVX512{DQ,VL}
	vcvttps2qq	xmm6{k7}, [edx-512]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	xmm6{k7}, [edx-516]{1to2}	 # AVX512{DQ,VL}
	vcvttps2qq	xmm6{k7}, DWORD BCST [edx+508]	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	ymm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvttps2qq	ymm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvttps2qq	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvttps2qq	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvttps2qq	ymm6{k7}, [eax]{1to4}	 # AVX512{DQ,VL}
	vcvttps2qq	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vcvttps2qq	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vcvttps2qq	ymm6{k7}, [edx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	ymm6{k7}, [edx+512]{1to4}	 # AVX512{DQ,VL}
	vcvttps2qq	ymm6{k7}, [edx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	ymm6{k7}, [edx-516]{1to4}	 # AVX512{DQ,VL}
	vcvttps2qq	ymm6{k7}, DWORD BCST [edx+508]	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	xmm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm6{k7}, [eax]{1to2}	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm6{k7}, [edx+508]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	xmm6{k7}, [edx+512]{1to2}	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm6{k7}, [edx-512]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	xmm6{k7}, [edx-516]{1to2}	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm6{k7}, DWORD BCST [edx+508]	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	ymm6{k7}, xmm5	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm6{k7}{z}, xmm5	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm6{k7}, [eax]{1to4}	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm6{k7}, [edx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	ymm6{k7}, [edx+512]{1to4}	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm6{k7}, [edx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	ymm6{k7}, [edx-516]{1to4}	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm6{k7}, DWORD BCST [edx+508]	 # AVX512{DQ,VL} Disp8
	vpmovd2m	k5, xmm6	 # AVX512{DQ,VL}
	vpmovd2m	k5, ymm6	 # AVX512{DQ,VL}
	vpmovq2m	k5, xmm6	 # AVX512{DQ,VL}
	vpmovq2m	k5, ymm6	 # AVX512{DQ,VL}
	vpmovm2d	xmm6, k5	 # AVX512{DQ,VL}
	vpmovm2d	ymm6, k5	 # AVX512{DQ,VL}
	vpmovm2q	xmm6, k5	 # AVX512{DQ,VL}
	vpmovm2q	ymm6, k5	 # AVX512{DQ,VL}
