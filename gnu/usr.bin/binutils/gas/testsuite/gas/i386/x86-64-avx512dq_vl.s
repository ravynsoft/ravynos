# Check 64bit AVX512{DQ,VL} instructions

	.allow_index_reg
	.text
_start:
	vbroadcastf64x2	(%rcx), %ymm30	 # AVX512{DQ,VL}
	vbroadcastf64x2	(%rcx), %ymm30{%k7}	 # AVX512{DQ,VL}
	vbroadcastf64x2	(%rcx), %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vbroadcastf64x2	0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vbroadcastf64x2	2032(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vbroadcastf64x2	2048(%rdx), %ymm30	 # AVX512{DQ,VL}
	vbroadcastf64x2	-2048(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vbroadcastf64x2	-2064(%rdx), %ymm30	 # AVX512{DQ,VL}
	vbroadcasti64x2	(%rcx), %ymm30	 # AVX512{DQ,VL}
	vbroadcasti64x2	(%rcx), %ymm30{%k7}	 # AVX512{DQ,VL}
	vbroadcasti64x2	(%rcx), %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vbroadcasti64x2	0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vbroadcasti64x2	2032(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vbroadcasti64x2	2048(%rdx), %ymm30	 # AVX512{DQ,VL}
	vbroadcasti64x2	-2048(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vbroadcasti64x2	-2064(%rdx), %ymm30	 # AVX512{DQ,VL}
	vbroadcastf32x2	%xmm31, %ymm30	 # AVX512{DQ,VL}
	vbroadcastf32x2	%xmm31, %ymm30{%k7}	 # AVX512{DQ,VL}
	vbroadcastf32x2	%xmm31, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vbroadcastf32x2	(%rcx), %ymm30	 # AVX512{DQ,VL}
	vbroadcastf32x2	0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vbroadcastf32x2	1016(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vbroadcastf32x2	1024(%rdx), %ymm30	 # AVX512{DQ,VL}
	vbroadcastf32x2	-1024(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vbroadcastf32x2	-1032(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvtpd2qq	%xmm29, %xmm30	 # AVX512{DQ,VL}
	vcvtpd2qq	%xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	%xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtpd2qq	(%rcx), %xmm30	 # AVX512{DQ,VL}
	vcvtpd2qq	0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vcvtpd2qq	(%rcx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtpd2qq	2032(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	2048(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtpd2qq	-2048(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	-2064(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtpd2qq	1016(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtpd2qq	-1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	-1032(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtpd2qq	%ymm29, %ymm30	 # AVX512{DQ,VL}
	vcvtpd2qq	%ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vcvtpd2qq	%ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtpd2qq	(%rcx), %ymm30	 # AVX512{DQ,VL}
	vcvtpd2qq	0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vcvtpd2qq	(%rcx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtpd2qq	4064(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	4096(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvtpd2qq	-4096(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	-4128(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvtpd2qq	1016(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	1024(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtpd2qq	-1024(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	-1032(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	%xmm29, %xmm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	%xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	%xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtpd2uqq	(%rcx), %xmm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	(%rcx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	2032(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	2048(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	-2048(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	-2064(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	1016(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	-1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	-1032(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	%ymm29, %ymm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	%ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vcvtpd2uqq	%ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtpd2uqq	(%rcx), %ymm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	(%rcx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	4064(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	4096(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	-4096(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	-4128(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	1016(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	1024(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtpd2uqq	-1024(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	-1032(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtps2qq	%xmm29, %xmm30	 # AVX512{DQ,VL}
	vcvtps2qq	%xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	%xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtps2qq	(%rcx), %xmm30	 # AVX512{DQ,VL}
	vcvtps2qq	0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vcvtps2qq	(%rcx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtps2qq	1016(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	1024(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtps2qq	-1024(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	-1032(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtps2qq	508(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	512(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtps2qq	-512(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	-516(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtps2qq	%xmm29, %ymm30	 # AVX512{DQ,VL}
	vcvtps2qq	%xmm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vcvtps2qq	%xmm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtps2qq	(%rcx), %ymm30	 # AVX512{DQ,VL}
	vcvtps2qq	0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vcvtps2qq	(%rcx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtps2qq	2032(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	2048(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvtps2qq	-2048(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	-2064(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvtps2qq	508(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	512(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtps2qq	-512(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	-516(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtps2uqq	%xmm29, %xmm30	 # AVX512{DQ,VL}
	vcvtps2uqq	%xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	%xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtps2uqq	(%rcx), %xmm30	 # AVX512{DQ,VL}
	vcvtps2uqq	0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vcvtps2uqq	(%rcx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtps2uqq	1016(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	1024(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtps2uqq	-1024(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	-1032(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtps2uqq	508(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	512(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtps2uqq	-512(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	-516(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtps2uqq	%xmm29, %ymm30	 # AVX512{DQ,VL}
	vcvtps2uqq	%xmm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vcvtps2uqq	%xmm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtps2uqq	(%rcx), %ymm30	 # AVX512{DQ,VL}
	vcvtps2uqq	0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vcvtps2uqq	(%rcx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtps2uqq	2032(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	2048(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvtps2uqq	-2048(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	-2064(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvtps2uqq	508(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	512(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtps2uqq	-512(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	-516(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtqq2pd	%xmm29, %xmm30	 # AVX512{DQ,VL}
	vcvtqq2pd	%xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	%xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtqq2pd	(%rcx), %xmm30	 # AVX512{DQ,VL}
	vcvtqq2pd	0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vcvtqq2pd	(%rcx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtqq2pd	2032(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	2048(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtqq2pd	-2048(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	-2064(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtqq2pd	1016(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtqq2pd	-1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	-1032(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtqq2pd	%ymm29, %ymm30	 # AVX512{DQ,VL}
	vcvtqq2pd	%ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vcvtqq2pd	%ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtqq2pd	(%rcx), %ymm30	 # AVX512{DQ,VL}
	vcvtqq2pd	0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vcvtqq2pd	(%rcx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtqq2pd	4064(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	4096(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvtqq2pd	-4096(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	-4128(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvtqq2pd	1016(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	1024(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtqq2pd	-1024(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	-1032(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtqq2ps	%xmm29, %xmm30	 # AVX512{DQ,VL}
	vcvtqq2ps	%xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vcvtqq2ps	%xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtqq2psx	(%rcx), %xmm30	 # AVX512{DQ,VL}
	vcvtqq2psx	0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vcvtqq2ps	(%rcx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtqq2psx	2032(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2psx	2048(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtqq2psx	-2048(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2psx	-2064(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtqq2psx	1016(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2psx	1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtqq2psx	-1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2psx	-1032(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtqq2ps	%ymm29, %xmm30	 # AVX512{DQ,VL}
	vcvtqq2ps	%ymm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vcvtqq2ps	%ymm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtqq2psy	(%rcx), %xmm30	 # AVX512{DQ,VL}
	vcvtqq2psy	0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vcvtqq2ps	(%rcx){1to4}, %xmm30	 # AVX512{DQ,VL}
	vcvtqq2psy	4064(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2psy	4096(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtqq2psy	-4096(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2psy	-4128(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtqq2psy	1016(%rdx){1to4}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2psy	1024(%rdx){1to4}, %xmm30	 # AVX512{DQ,VL}
	vcvtqq2psy	-1024(%rdx){1to4}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtqq2psy	-1032(%rdx){1to4}, %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	%xmm29, %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	%xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	%xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtuqq2pd	(%rcx), %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	(%rcx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	2032(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	2048(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	-2048(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	-2064(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	1016(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	-1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	-1032(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	%ymm29, %ymm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	%ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2pd	%ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtuqq2pd	(%rcx), %ymm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	(%rcx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	4064(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	4096(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	-4096(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	-4128(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	1016(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	1024(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtuqq2pd	-1024(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	-1032(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvtuqq2ps	%xmm29, %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2ps	%xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2ps	%xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtuqq2psx	(%rcx), %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2psx	0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2ps	(%rcx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2psx	2032(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psx	2048(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2psx	-2048(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psx	-2064(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2psx	1016(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psx	1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2psx	-1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psx	-1032(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2ps	%ymm29, %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2ps	%ymm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vcvtuqq2ps	%ymm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvtuqq2psy	(%rcx), %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2psy	0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2ps	(%rcx){1to4}, %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2psy	4064(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psy	4096(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2psy	-4096(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psy	-4128(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2psy	1016(%rdx){1to4}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psy	1024(%rdx){1to4}, %xmm30	 # AVX512{DQ,VL}
	vcvtuqq2psy	-1024(%rdx){1to4}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvtuqq2psy	-1032(%rdx){1to4}, %xmm30	 # AVX512{DQ,VL}
	vextractf64x2	$0xab, %ymm29, %xmm30	 # AVX512{DQ,VL}
	vextractf64x2	$0xab, %ymm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vextractf64x2	$0xab, %ymm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vextractf64x2	$123, %ymm29, %xmm30	 # AVX512{DQ,VL}
	vextracti64x2	$0xab, %ymm29, %xmm30	 # AVX512{DQ,VL}
	vextracti64x2	$0xab, %ymm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vextracti64x2	$0xab, %ymm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vextracti64x2	$123, %ymm29, %xmm30	 # AVX512{DQ,VL}
	vfpclasspd	$0xab, %xmm30, %k5	 # AVX512{DQ,VL}
	vfpclasspd	$0xab, %xmm30, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspd	$123, %xmm30, %k5	 # AVX512{DQ,VL}
	vfpclasspdx	$123, (%rcx), %k5	 # AVX512{DQ,VL}
	vfpclasspdx	$123, 0x123(%rax,%r14,8), %k5	 # AVX512{DQ,VL}
	vfpclasspd	$123, (%rcx){1to2}, %k5	 # AVX512{DQ,VL}
	vfpclasspdx	$123, 2032(%rdx), %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspdx	$123, 2048(%rdx), %k5	 # AVX512{DQ,VL}
	vfpclasspdx	$123, -2048(%rdx), %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspdx	$123, -2064(%rdx), %k5	 # AVX512{DQ,VL}
	vfpclasspdx	$123, 1016(%rdx){1to2}, %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspdx	$123, 1024(%rdx){1to2}, %k5	 # AVX512{DQ,VL}
	vfpclasspdx	$123, -1024(%rdx){1to2}, %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspdx	$123, -1032(%rdx){1to2}, %k5	 # AVX512{DQ,VL}
	vfpclasspd	$0xab, %ymm30, %k5	 # AVX512{DQ,VL}
	vfpclasspd	$0xab, %ymm30, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclasspd	$123, %ymm30, %k5	 # AVX512{DQ,VL}
	vfpclasspdy	$123, (%rcx), %k5	 # AVX512{DQ,VL}
	vfpclasspdy	$123, 0x123(%rax,%r14,8), %k5	 # AVX512{DQ,VL}
	vfpclasspd	$123, (%rcx){1to4}, %k5	 # AVX512{DQ,VL}
	vfpclasspdy	$123, 4064(%rdx), %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspdy	$123, 4096(%rdx), %k5	 # AVX512{DQ,VL}
	vfpclasspdy	$123, -4096(%rdx), %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspdy	$123, -4128(%rdx), %k5	 # AVX512{DQ,VL}
	vfpclasspdy	$123, 1016(%rdx){1to4}, %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspdy	$123, 1024(%rdx){1to4}, %k5	 # AVX512{DQ,VL}
	vfpclasspdy	$123, -1024(%rdx){1to4}, %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspdy	$123, -1032(%rdx){1to4}, %k5	 # AVX512{DQ,VL}
	vfpclassps	$0xab, %xmm30, %k5	 # AVX512{DQ,VL}
	vfpclassps	$0xab, %xmm30, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclassps	$123, %xmm30, %k5	 # AVX512{DQ,VL}
	vfpclasspsx	$123, (%rcx), %k5	 # AVX512{DQ,VL}
	vfpclasspsx	$123, 0x123(%rax,%r14,8), %k5	 # AVX512{DQ,VL}
	vfpclassps	$123, (%rcx){1to4}, %k5	 # AVX512{DQ,VL}
	vfpclasspsx	$123, 2032(%rdx), %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspsx	$123, 2048(%rdx), %k5	 # AVX512{DQ,VL}
	vfpclasspsx	$123, -2048(%rdx), %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspsx	$123, -2064(%rdx), %k5	 # AVX512{DQ,VL}
	vfpclasspsx	$123, 508(%rdx){1to4}, %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspsx	$123, 512(%rdx){1to4}, %k5	 # AVX512{DQ,VL}
	vfpclasspsx	$123, -512(%rdx){1to4}, %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspsx	$123, -516(%rdx){1to4}, %k5	 # AVX512{DQ,VL}
	vfpclassps	$0xab, %ymm30, %k5	 # AVX512{DQ,VL}
	vfpclassps	$0xab, %ymm30, %k5{%k7}	 # AVX512{DQ,VL}
	vfpclassps	$123, %ymm30, %k5	 # AVX512{DQ,VL}
	vfpclasspsy	$123, (%rcx), %k5	 # AVX512{DQ,VL}
	vfpclasspsy	$123, 0x123(%rax,%r14,8), %k5	 # AVX512{DQ,VL}
	vfpclassps	$123, (%rcx){1to8}, %k5	 # AVX512{DQ,VL}
	vfpclasspsy	$123, 4064(%rdx), %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspsy	$123, 4096(%rdx), %k5	 # AVX512{DQ,VL}
	vfpclasspsy	$123, -4096(%rdx), %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspsy	$123, -4128(%rdx), %k5	 # AVX512{DQ,VL}
	vfpclasspsy	$123, 508(%rdx){1to8}, %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspsy	$123, 512(%rdx){1to8}, %k5	 # AVX512{DQ,VL}
	vfpclasspsy	$123, -512(%rdx){1to8}, %k5	 # AVX512{DQ,VL} Disp8
	vfpclasspsy	$123, -516(%rdx){1to8}, %k5	 # AVX512{DQ,VL}
	vinsertf64x2	$0xab, %xmm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vinsertf64x2	$0xab, %xmm28, %ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vinsertf64x2	$0xab, %xmm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vinsertf64x2	$123, %xmm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vinsertf64x2	$123, (%rcx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vinsertf64x2	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vinsertf64x2	$123, 2032(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vinsertf64x2	$123, 2048(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vinsertf64x2	$123, -2048(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vinsertf64x2	$123, -2064(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vinserti64x2	$0xab, %xmm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vinserti64x2	$0xab, %xmm28, %ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vinserti64x2	$0xab, %xmm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vinserti64x2	$123, %xmm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vinserti64x2	$123, (%rcx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vinserti64x2	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vinserti64x2	$123, 2032(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vinserti64x2	$123, 2048(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vinserti64x2	$123, -2048(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vinserti64x2	$123, -2064(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vbroadcasti32x2	%xmm31, %xmm30	 # AVX512{DQ,VL}
	vbroadcasti32x2	%xmm31, %xmm30{%k7}	 # AVX512{DQ,VL}
	vbroadcasti32x2	%xmm31, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vbroadcasti32x2	(%rcx), %xmm30	 # AVX512{DQ,VL}
	vbroadcasti32x2	0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vbroadcasti32x2	1016(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	1024(%rdx), %xmm30	 # AVX512{DQ,VL}
	vbroadcasti32x2	-1024(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	-1032(%rdx), %xmm30	 # AVX512{DQ,VL}
	vbroadcasti32x2	%xmm31, %ymm30	 # AVX512{DQ,VL}
	vbroadcasti32x2	%xmm31, %ymm30{%k7}	 # AVX512{DQ,VL}
	vbroadcasti32x2	%xmm31, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vbroadcasti32x2	(%rcx), %ymm30	 # AVX512{DQ,VL}
	vbroadcasti32x2	0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vbroadcasti32x2	1016(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	1024(%rdx), %ymm30	 # AVX512{DQ,VL}
	vbroadcasti32x2	-1024(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	-1032(%rdx), %ymm30	 # AVX512{DQ,VL}
	vpmullq	%xmm28, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vpmullq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vpmullq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vpmullq	(%rcx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vpmullq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vpmullq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vpmullq	2032(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vpmullq	2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vpmullq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vpmullq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vpmullq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vpmullq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vpmullq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vpmullq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vpmullq	%ymm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vpmullq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vpmullq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vpmullq	(%rcx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vpmullq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vpmullq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vpmullq	4064(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vpmullq	4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vpmullq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vpmullq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vpmullq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vpmullq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vpmullq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vpmullq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangepd	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangepd	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vrangepd	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vrangepd	$123, %xmm28, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangepd	$123, (%rcx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangepd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangepd	$123, (%rcx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangepd	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, 2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangepd	$123, -2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, -2064(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangepd	$123, 1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, 1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangepd	$123, -1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, -1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangepd	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangepd	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vrangepd	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vrangepd	$123, %ymm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangepd	$123, (%rcx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangepd	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangepd	$123, (%rcx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangepd	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, 4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangepd	$123, -4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, -4128(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangepd	$123, 1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, 1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangepd	$123, -1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vrangepd	$123, -1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangeps	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangeps	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vrangeps	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vrangeps	$123, %xmm28, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangeps	$123, (%rcx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangeps	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangeps	$123, (%rcx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangeps	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, 2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangeps	$123, -2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, -2064(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangeps	$123, 508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, 512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangeps	$123, -512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, -516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vrangeps	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangeps	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vrangeps	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vrangeps	$123, %ymm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangeps	$123, (%rcx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangeps	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangeps	$123, (%rcx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangeps	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, 4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangeps	$123, -4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, -4128(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangeps	$123, 508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, 512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vrangeps	$123, -512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vrangeps	$123, -516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandpd	%xmm28, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandpd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vandpd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vandpd	(%rcx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandpd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandpd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandpd	2032(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandpd	2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandpd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandpd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandpd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandpd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandpd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandpd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandpd	%ymm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vandpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vandpd	(%rcx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandps	%xmm28, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vandps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vandps	(%rcx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandps	2032(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandps	2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandps	%ymm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vandps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vandps	(%rcx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandps	4064(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandps	4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnpd	%xmm28, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnpd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vandnpd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vandnpd	(%rcx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnpd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnpd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnpd	2032(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandnpd	2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnpd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandnpd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnpd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandnpd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnpd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandnpd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnpd	%ymm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vandnpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vandnpd	(%rcx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandnpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandnpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandnpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandnpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnps	%xmm28, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vandnps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vandnps	(%rcx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnps	2032(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandnps	2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandnps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandnps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vandnps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vandnps	%ymm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vandnps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vandnps	(%rcx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnps	4064(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandnps	4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandnps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandnps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vandnps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vandnps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorpd	%xmm28, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorpd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vorpd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vorpd	(%rcx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorpd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorpd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorpd	2032(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vorpd	2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorpd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vorpd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorpd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vorpd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorpd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vorpd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorpd	%ymm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vorpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vorpd	(%rcx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vorpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vorpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vorpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vorpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorps	%xmm28, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vorps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vorps	(%rcx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorps	2032(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vorps	2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vorps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vorps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vorps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vorps	%ymm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vorps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vorps	(%rcx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorps	4064(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vorps	4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vorps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vorps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vorps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vorps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorpd	%xmm28, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorpd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vxorpd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vxorpd	(%rcx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorpd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorpd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorpd	2032(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vxorpd	2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorpd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vxorpd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorpd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vxorpd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorpd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vxorpd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorpd	%ymm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vxorpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vxorpd	(%rcx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vxorpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vxorpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vxorpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vxorpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorps	%xmm28, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vxorps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vxorps	(%rcx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorps	2032(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vxorps	2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vxorps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vxorps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL} Disp8
	vxorps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vxorps	%ymm28, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vxorps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vxorps	(%rcx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorps	4064(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vxorps	4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vxorps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vxorps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vxorps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL} Disp8
	vxorps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vreducepd	$0xab, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vreducepd	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vreducepd	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vreducepd	$123, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vreducepd	$123, (%rcx), %xmm30	 # AVX512{DQ,VL}
	vreducepd	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vreducepd	$123, (%rcx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vreducepd	$123, 2032(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, 2048(%rdx), %xmm30	 # AVX512{DQ,VL}
	vreducepd	$123, -2048(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, -2064(%rdx), %xmm30	 # AVX512{DQ,VL}
	vreducepd	$123, 1016(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, 1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vreducepd	$123, -1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, -1032(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vreducepd	$0xab, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vreducepd	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vreducepd	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vreducepd	$123, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vreducepd	$123, (%rcx), %ymm30	 # AVX512{DQ,VL}
	vreducepd	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vreducepd	$123, (%rcx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vreducepd	$123, 4064(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, 4096(%rdx), %ymm30	 # AVX512{DQ,VL}
	vreducepd	$123, -4096(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, -4128(%rdx), %ymm30	 # AVX512{DQ,VL}
	vreducepd	$123, 1016(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, 1024(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vreducepd	$123, -1024(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vreducepd	$123, -1032(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vreduceps	$0xab, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vreduceps	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vreduceps	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vreduceps	$123, %xmm29, %xmm30	 # AVX512{DQ,VL}
	vreduceps	$123, (%rcx), %xmm30	 # AVX512{DQ,VL}
	vreduceps	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vreduceps	$123, (%rcx){1to4}, %xmm30	 # AVX512{DQ,VL}
	vreduceps	$123, 2032(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, 2048(%rdx), %xmm30	 # AVX512{DQ,VL}
	vreduceps	$123, -2048(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, -2064(%rdx), %xmm30	 # AVX512{DQ,VL}
	vreduceps	$123, 508(%rdx){1to4}, %xmm30	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, 512(%rdx){1to4}, %xmm30	 # AVX512{DQ,VL}
	vreduceps	$123, -512(%rdx){1to4}, %xmm30	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, -516(%rdx){1to4}, %xmm30	 # AVX512{DQ,VL}
	vreduceps	$0xab, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vreduceps	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vreduceps	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vreduceps	$123, %ymm29, %ymm30	 # AVX512{DQ,VL}
	vreduceps	$123, (%rcx), %ymm30	 # AVX512{DQ,VL}
	vreduceps	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vreduceps	$123, (%rcx){1to8}, %ymm30	 # AVX512{DQ,VL}
	vreduceps	$123, 4064(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, 4096(%rdx), %ymm30	 # AVX512{DQ,VL}
	vreduceps	$123, -4096(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, -4128(%rdx), %ymm30	 # AVX512{DQ,VL}
	vreduceps	$123, 508(%rdx){1to8}, %ymm30	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, 512(%rdx){1to8}, %ymm30	 # AVX512{DQ,VL}
	vreduceps	$123, -512(%rdx){1to8}, %ymm30	 # AVX512{DQ,VL} Disp8
	vreduceps	$123, -516(%rdx){1to8}, %ymm30	 # AVX512{DQ,VL}
	vextractf64x2	$0xab, %ymm29, (%rcx)	 # AVX512{DQ,VL}
	vextractf64x2	$0xab, %ymm29, (%rcx){%k7}	 # AVX512{DQ,VL}
	vextractf64x2	$123, %ymm29, (%rcx)	 # AVX512{DQ,VL}
	vextractf64x2	$123, %ymm29, 0x123(%rax,%r14,8)	 # AVX512{DQ,VL}
	vextractf64x2	$123, %ymm29, 2032(%rdx)	 # AVX512{DQ,VL} Disp8
	vextractf64x2	$123, %ymm29, 2048(%rdx)	 # AVX512{DQ,VL}
	vextractf64x2	$123, %ymm29, -2048(%rdx)	 # AVX512{DQ,VL} Disp8
	vextractf64x2	$123, %ymm29, -2064(%rdx)	 # AVX512{DQ,VL}
	vextracti64x2	$0xab, %ymm29, (%rcx)	 # AVX512{DQ,VL}
	vextracti64x2	$0xab, %ymm29, (%rcx){%k7}	 # AVX512{DQ,VL}
	vextracti64x2	$123, %ymm29, (%rcx)	 # AVX512{DQ,VL}
	vextracti64x2	$123, %ymm29, 0x123(%rax,%r14,8)	 # AVX512{DQ,VL}
	vextracti64x2	$123, %ymm29, 2032(%rdx)	 # AVX512{DQ,VL} Disp8
	vextracti64x2	$123, %ymm29, 2048(%rdx)	 # AVX512{DQ,VL}
	vextracti64x2	$123, %ymm29, -2048(%rdx)	 # AVX512{DQ,VL} Disp8
	vextracti64x2	$123, %ymm29, -2064(%rdx)	 # AVX512{DQ,VL}
	vcvttpd2qq	%xmm29, %xmm30	 # AVX512{DQ,VL}
	vcvttpd2qq	%xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	%xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvttpd2qq	(%rcx), %xmm30	 # AVX512{DQ,VL}
	vcvttpd2qq	0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vcvttpd2qq	(%rcx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvttpd2qq	2032(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	2048(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvttpd2qq	-2048(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	-2064(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvttpd2qq	1016(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvttpd2qq	-1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	-1032(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvttpd2qq	%ymm29, %ymm30	 # AVX512{DQ,VL}
	vcvttpd2qq	%ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vcvttpd2qq	%ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvttpd2qq	(%rcx), %ymm30	 # AVX512{DQ,VL}
	vcvttpd2qq	0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vcvttpd2qq	(%rcx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvttpd2qq	4064(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	4096(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvttpd2qq	-4096(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	-4128(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvttpd2qq	1016(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	1024(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvttpd2qq	-1024(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	-1032(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	%xmm29, %xmm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	%xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	%xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvttpd2uqq	(%rcx), %xmm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	(%rcx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	2032(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	2048(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	-2048(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	-2064(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	1016(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	-1024(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	-1032(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	%ymm29, %ymm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	%ymm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vcvttpd2uqq	%ymm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvttpd2uqq	(%rcx), %ymm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	(%rcx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	4064(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	4096(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	-4096(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	-4128(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	1016(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	1024(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvttpd2uqq	-1024(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	-1032(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvttps2qq	%xmm29, %xmm30	 # AVX512{DQ,VL}
	vcvttps2qq	%xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	%xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvttps2qq	(%rcx), %xmm30	 # AVX512{DQ,VL}
	vcvttps2qq	0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vcvttps2qq	(%rcx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvttps2qq	1016(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	1024(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvttps2qq	-1024(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	-1032(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvttps2qq	508(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	512(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvttps2qq	-512(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	-516(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvttps2qq	%xmm29, %ymm30	 # AVX512{DQ,VL}
	vcvttps2qq	%xmm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vcvttps2qq	%xmm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvttps2qq	(%rcx), %ymm30	 # AVX512{DQ,VL}
	vcvttps2qq	0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vcvttps2qq	(%rcx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvttps2qq	2032(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	2048(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvttps2qq	-2048(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	-2064(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvttps2qq	508(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	512(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvttps2qq	-512(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	-516(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvttps2uqq	%xmm29, %xmm30	 # AVX512{DQ,VL}
	vcvttps2uqq	%xmm29, %xmm30{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	%xmm29, %xmm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvttps2uqq	(%rcx), %xmm30	 # AVX512{DQ,VL}
	vcvttps2uqq	0x123(%rax,%r14,8), %xmm30	 # AVX512{DQ,VL}
	vcvttps2uqq	(%rcx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvttps2uqq	1016(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	1024(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvttps2uqq	-1024(%rdx), %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	-1032(%rdx), %xmm30	 # AVX512{DQ,VL}
	vcvttps2uqq	508(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	512(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvttps2uqq	-512(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	-516(%rdx){1to2}, %xmm30	 # AVX512{DQ,VL}
	vcvttps2uqq	%xmm29, %ymm30	 # AVX512{DQ,VL}
	vcvttps2uqq	%xmm29, %ymm30{%k7}	 # AVX512{DQ,VL}
	vcvttps2uqq	%xmm29, %ymm30{%k7}{z}	 # AVX512{DQ,VL}
	vcvttps2uqq	(%rcx), %ymm30	 # AVX512{DQ,VL}
	vcvttps2uqq	0x123(%rax,%r14,8), %ymm30	 # AVX512{DQ,VL}
	vcvttps2uqq	(%rcx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvttps2uqq	2032(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	2048(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvttps2uqq	-2048(%rdx), %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	-2064(%rdx), %ymm30	 # AVX512{DQ,VL}
	vcvttps2uqq	508(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	512(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vcvttps2uqq	-512(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	-516(%rdx){1to4}, %ymm30	 # AVX512{DQ,VL}
	vpmovd2m	%xmm30, %k5	 # AVX512{DQ,VL}
	vpmovd2m	%ymm30, %k5	 # AVX512{DQ,VL}
	vpmovq2m	%xmm30, %k5	 # AVX512{DQ,VL}
	vpmovq2m	%ymm30, %k5	 # AVX512{DQ,VL}
	vpmovm2d	%k5, %xmm30	 # AVX512{DQ,VL}
	vpmovm2d	%k5, %ymm30	 # AVX512{DQ,VL}
	vpmovm2q	%k5, %xmm30	 # AVX512{DQ,VL}
	vpmovm2q	%k5, %ymm30	 # AVX512{DQ,VL}

	.intel_syntax noprefix
	vbroadcastf64x2	ymm30, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vbroadcastf64x2	ymm30{k7}, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vbroadcastf64x2	ymm30{k7}{z}, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vbroadcastf64x2	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vbroadcastf64x2	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vbroadcastf64x2	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vbroadcastf64x2	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vbroadcastf64x2	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vbroadcasti64x2	ymm30, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vbroadcasti64x2	ymm30{k7}, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vbroadcasti64x2	ymm30{k7}{z}, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vbroadcasti64x2	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vbroadcasti64x2	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vbroadcasti64x2	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vbroadcasti64x2	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vbroadcasti64x2	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vbroadcastf32x2	ymm30, xmm31	 # AVX512{DQ,VL}
	vbroadcastf32x2	ymm30{k7}, xmm31	 # AVX512{DQ,VL}
	vbroadcastf32x2	ymm30{k7}{z}, xmm31	 # AVX512{DQ,VL}
	vbroadcastf32x2	ymm30, QWORD PTR [rcx]	 # AVX512{DQ,VL}
	vbroadcastf32x2	ymm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vbroadcastf32x2	ymm30, QWORD PTR [rdx+1016]	 # AVX512{DQ,VL} Disp8
	vbroadcastf32x2	ymm30, QWORD PTR [rdx+1024]	 # AVX512{DQ,VL}
	vbroadcastf32x2	ymm30, QWORD PTR [rdx-1024]	 # AVX512{DQ,VL} Disp8
	vbroadcastf32x2	ymm30, QWORD PTR [rdx-1032]	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm30, xmm29	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm30, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm30, [rcx]{1to2}	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm30, [rdx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	xmm30, [rdx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvtpd2qq	xmm30, [rdx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	xmm30, [rdx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm30, ymm29	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm30{k7}, ymm29	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm30{k7}{z}, ymm29	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm30, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm30, [rcx]{1to4}	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm30, [rdx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	ymm30, [rdx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvtpd2qq	ymm30, [rdx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtpd2qq	ymm30, [rdx-1032]{1to4}	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm30, xmm29	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm30, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm30, [rcx]{1to2}	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm30, [rdx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	xmm30, [rdx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvtpd2uqq	xmm30, [rdx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	xmm30, [rdx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm30, ymm29	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm30{k7}, ymm29	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm30{k7}{z}, ymm29	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm30, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm30, [rcx]{1to4}	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm30, [rdx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	ymm30, [rdx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvtpd2uqq	ymm30, [rdx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtpd2uqq	ymm30, [rdx-1032]{1to4}	 # AVX512{DQ,VL}
	vcvtps2qq	xmm30, xmm29	 # AVX512{DQ,VL}
	vcvtps2qq	xmm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvtps2qq	xmm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvtps2qq	xmm30, QWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtps2qq	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtps2qq	xmm30, [rcx]{1to2}	 # AVX512{DQ,VL}
	vcvtps2qq	xmm30, QWORD PTR [rdx+1016]	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	xmm30, QWORD PTR [rdx+1024]	 # AVX512{DQ,VL}
	vcvtps2qq	xmm30, QWORD PTR [rdx-1024]	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	xmm30, QWORD PTR [rdx-1032]	 # AVX512{DQ,VL}
	vcvtps2qq	xmm30, [rdx+508]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	xmm30, [rdx+512]{1to2}	 # AVX512{DQ,VL}
	vcvtps2qq	xmm30, [rdx-512]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	xmm30, [rdx-516]{1to2}	 # AVX512{DQ,VL}
	vcvtps2qq	xmm30, DWORD BCST [rdx+508]	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	ymm30, xmm29	 # AVX512{DQ,VL}
	vcvtps2qq	ymm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvtps2qq	ymm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvtps2qq	ymm30, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtps2qq	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtps2qq	ymm30, [rcx]{1to4}	 # AVX512{DQ,VL}
	vcvtps2qq	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vcvtps2qq	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vcvtps2qq	ymm30, [rdx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	ymm30, [rdx+512]{1to4}	 # AVX512{DQ,VL}
	vcvtps2qq	ymm30, [rdx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtps2qq	ymm30, [rdx-516]{1to4}	 # AVX512{DQ,VL}
	vcvtps2qq	ymm30, DWORD BCST [rdx+508]	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	xmm30, xmm29	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm30, QWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm30, [rcx]{1to2}	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm30, QWORD PTR [rdx+1016]	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	xmm30, QWORD PTR [rdx+1024]	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm30, QWORD PTR [rdx-1024]	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	xmm30, QWORD PTR [rdx-1032]	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm30, [rdx+508]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	xmm30, [rdx+512]{1to2}	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm30, [rdx-512]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	xmm30, [rdx-516]{1to2}	 # AVX512{DQ,VL}
	vcvtps2uqq	xmm30, DWORD BCST [rdx+508]	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	ymm30, xmm29	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm30, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm30, [rcx]{1to4}	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm30, [rdx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	ymm30, [rdx+512]{1to4}	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm30, [rdx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtps2uqq	ymm30, [rdx-516]{1to4}	 # AVX512{DQ,VL}
	vcvtps2uqq	ymm30, DWORD BCST [rdx+508]	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	xmm30, xmm29	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm30, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm30, [rcx]{1to2}	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm30, [rdx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	xmm30, [rdx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvtqq2pd	xmm30, [rdx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	xmm30, [rdx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm30, ymm29	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm30{k7}, ymm29	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm30{k7}{z}, ymm29	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm30, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm30, [rcx]{1to4}	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm30, [rdx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	ymm30, [rdx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvtqq2pd	ymm30, [rdx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtqq2pd	ymm30, [rdx-1032]{1to4}	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, xmm29	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, [rcx]{1to2}	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, QWORD BCST [rdx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm30, QWORD BCST [rdx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, QWORD BCST [rdx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm30, QWORD BCST [rdx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, ymm29	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30{k7}, ymm29	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30{k7}{z}, ymm29	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, [rcx]{1to4}	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm30, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm30, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, QWORD BCST [rdx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm30, QWORD BCST [rdx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvtqq2ps	xmm30, QWORD BCST [rdx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtqq2ps	xmm30, QWORD BCST [rdx-1032]{1to4}	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm30, xmm29	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm30, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm30, [rcx]{1to2}	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm30, [rdx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	xmm30, [rdx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvtuqq2pd	xmm30, [rdx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	xmm30, [rdx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm30, ymm29	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm30{k7}, ymm29	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm30{k7}{z}, ymm29	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm30, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm30, [rcx]{1to4}	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm30, [rdx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	ymm30, [rdx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvtuqq2pd	ymm30, [rdx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2pd	ymm30, [rdx-1032]{1to4}	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, xmm29	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, [rcx]{1to2}	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, QWORD BCST [rdx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm30, QWORD BCST [rdx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, QWORD BCST [rdx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm30, QWORD BCST [rdx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, ymm29	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30{k7}, ymm29	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30{k7}{z}, ymm29	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, [rcx]{1to4}	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm30, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm30, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, QWORD BCST [rdx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm30, QWORD BCST [rdx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvtuqq2ps	xmm30, QWORD BCST [rdx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvtuqq2ps	xmm30, QWORD BCST [rdx-1032]{1to4}	 # AVX512{DQ,VL}
	vextractf64x2	xmm30, ymm29, 0xab	 # AVX512{DQ,VL}
	vextractf64x2	xmm30{k7}, ymm29, 0xab	 # AVX512{DQ,VL}
	vextractf64x2	xmm30{k7}{z}, ymm29, 0xab	 # AVX512{DQ,VL}
	vextractf64x2	xmm30, ymm29, 123	 # AVX512{DQ,VL}
	vextracti64x2	xmm30, ymm29, 0xab	 # AVX512{DQ,VL}
	vextracti64x2	xmm30{k7}, ymm29, 0xab	 # AVX512{DQ,VL}
	vextracti64x2	xmm30{k7}{z}, ymm29, 0xab	 # AVX512{DQ,VL}
	vextracti64x2	xmm30, ymm29, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, xmm30, 0xab	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, xmm30, 0xab	 # AVX512{DQ,VL}
	vfpclasspd	k5, xmm30, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, XMMWORD PTR [rcx], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, [rcx]{1to2}, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, XMMWORD PTR [rdx+2032], 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5, XMMWORD PTR [rdx+2048], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, XMMWORD PTR [rdx-2048], 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5, XMMWORD PTR [rdx-2064], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, QWORD BCST [rdx+1016]{1to2}, 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5, QWORD BCST [rdx+1024]{1to2}, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, QWORD BCST [rdx-1024]{1to2}, 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5, QWORD BCST [rdx-1032]{1to2}, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, ymm30, 0xab	 # AVX512{DQ,VL}
	vfpclasspd	k5{k7}, ymm30, 0xab	 # AVX512{DQ,VL}
	vfpclasspd	k5, ymm30, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, YMMWORD PTR [rcx], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, [rcx]{1to4}, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, YMMWORD PTR [rdx+4064], 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5, YMMWORD PTR [rdx+4096], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, YMMWORD PTR [rdx-4096], 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5, YMMWORD PTR [rdx-4128], 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, QWORD BCST [rdx+1016]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5, QWORD BCST [rdx+1024]{1to4}, 123	 # AVX512{DQ,VL}
	vfpclasspd	k5, QWORD BCST [rdx-1024]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vfpclasspd	k5, QWORD BCST [rdx-1032]{1to4}, 123	 # AVX512{DQ,VL}
	vfpclassps	k5, xmm30, 0xab	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, xmm30, 0xab	 # AVX512{DQ,VL}
	vfpclassps	k5, xmm30, 123	 # AVX512{DQ,VL}
	vfpclassps	k5, XMMWORD PTR [rcx], 123	 # AVX512{DQ,VL}
	vfpclassps	k5, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{DQ,VL}
	vfpclassps	k5, [rcx]{1to4}, 123	 # AVX512{DQ,VL}
	vfpclassps	k5, XMMWORD PTR [rdx+2032], 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5, XMMWORD PTR [rdx+2048], 123	 # AVX512{DQ,VL}
	vfpclassps	k5, XMMWORD PTR [rdx-2048], 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5, XMMWORD PTR [rdx-2064], 123	 # AVX512{DQ,VL}
	vfpclassps	k5, DWORD BCST [rdx+508]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5, DWORD BCST [rdx+512]{1to4}, 123	 # AVX512{DQ,VL}
	vfpclassps	k5, DWORD BCST [rdx-512]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5, DWORD BCST [rdx-516]{1to4}, 123	 # AVX512{DQ,VL}
	vfpclassps	k5, ymm30, 0xab	 # AVX512{DQ,VL}
	vfpclassps	k5{k7}, ymm30, 0xab	 # AVX512{DQ,VL}
	vfpclassps	k5, ymm30, 123	 # AVX512{DQ,VL}
	vfpclassps	k5, YMMWORD PTR [rcx], 123	 # AVX512{DQ,VL}
	vfpclassps	k5, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{DQ,VL}
	vfpclassps	k5, [rcx]{1to8}, 123	 # AVX512{DQ,VL}
	vfpclassps	k5, YMMWORD PTR [rdx+4064], 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5, YMMWORD PTR [rdx+4096], 123	 # AVX512{DQ,VL}
	vfpclassps	k5, YMMWORD PTR [rdx-4096], 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5, YMMWORD PTR [rdx-4128], 123	 # AVX512{DQ,VL}
	vfpclassps	k5, DWORD BCST [rdx+508]{1to8}, 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5, DWORD BCST [rdx+512]{1to8}, 123	 # AVX512{DQ,VL}
	vfpclassps	k5, DWORD BCST [rdx-512]{1to8}, 123	 # AVX512{DQ,VL} Disp8
	vfpclassps	k5, DWORD BCST [rdx-516]{1to8}, 123	 # AVX512{DQ,VL}
	vinsertf64x2	ymm30, ymm29, xmm28, 0xab	 # AVX512{DQ,VL}
	vinsertf64x2	ymm30{k7}, ymm29, xmm28, 0xab	 # AVX512{DQ,VL}
	vinsertf64x2	ymm30{k7}{z}, ymm29, xmm28, 0xab	 # AVX512{DQ,VL}
	vinsertf64x2	ymm30, ymm29, xmm28, 123	 # AVX512{DQ,VL}
	vinsertf64x2	ymm30, ymm29, XMMWORD PTR [rcx], 123	 # AVX512{DQ,VL}
	vinsertf64x2	ymm30, ymm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{DQ,VL}
	vinsertf64x2	ymm30, ymm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{DQ,VL} Disp8
	vinsertf64x2	ymm30, ymm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{DQ,VL}
	vinsertf64x2	ymm30, ymm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{DQ,VL} Disp8
	vinsertf64x2	ymm30, ymm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{DQ,VL}
	vinserti64x2	ymm30, ymm29, xmm28, 0xab	 # AVX512{DQ,VL}
	vinserti64x2	ymm30{k7}, ymm29, xmm28, 0xab	 # AVX512{DQ,VL}
	vinserti64x2	ymm30{k7}{z}, ymm29, xmm28, 0xab	 # AVX512{DQ,VL}
	vinserti64x2	ymm30, ymm29, xmm28, 123	 # AVX512{DQ,VL}
	vinserti64x2	ymm30, ymm29, XMMWORD PTR [rcx], 123	 # AVX512{DQ,VL}
	vinserti64x2	ymm30, ymm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{DQ,VL}
	vinserti64x2	ymm30, ymm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{DQ,VL} Disp8
	vinserti64x2	ymm30, ymm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{DQ,VL}
	vinserti64x2	ymm30, ymm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{DQ,VL} Disp8
	vinserti64x2	ymm30, ymm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{DQ,VL}
	vbroadcasti32x2	xmm30, xmm31	 # AVX512{DQ,VL}
	vbroadcasti32x2	xmm30{k7}, xmm31	 # AVX512{DQ,VL}
	vbroadcasti32x2	xmm30{k7}{z}, xmm31	 # AVX512{DQ,VL}
	vbroadcasti32x2	xmm30, QWORD PTR [rcx]	 # AVX512{DQ,VL}
	vbroadcasti32x2	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vbroadcasti32x2	xmm30, QWORD PTR [rdx+1016]	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	xmm30, QWORD PTR [rdx+1024]	 # AVX512{DQ,VL}
	vbroadcasti32x2	xmm30, QWORD PTR [rdx-1024]	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	xmm30, QWORD PTR [rdx-1032]	 # AVX512{DQ,VL}
	vbroadcasti32x2	ymm30, xmm31	 # AVX512{DQ,VL}
	vbroadcasti32x2	ymm30{k7}, xmm31	 # AVX512{DQ,VL}
	vbroadcasti32x2	ymm30{k7}{z}, xmm31	 # AVX512{DQ,VL}
	vbroadcasti32x2	ymm30, QWORD PTR [rcx]	 # AVX512{DQ,VL}
	vbroadcasti32x2	ymm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vbroadcasti32x2	ymm30, QWORD PTR [rdx+1016]	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	ymm30, QWORD PTR [rdx+1024]	 # AVX512{DQ,VL}
	vbroadcasti32x2	ymm30, QWORD PTR [rdx-1024]	 # AVX512{DQ,VL} Disp8
	vbroadcasti32x2	ymm30, QWORD PTR [rdx-1032]	 # AVX512{DQ,VL}
	vpmullq	xmm30, xmm29, xmm28	 # AVX512{DQ,VL}
	vpmullq	xmm30{k7}, xmm29, xmm28	 # AVX512{DQ,VL}
	vpmullq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{DQ,VL}
	vpmullq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vpmullq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vpmullq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{DQ,VL}
	vpmullq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vpmullq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vpmullq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vpmullq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vpmullq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vpmullq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{DQ,VL}
	vpmullq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vpmullq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{DQ,VL}
	vpmullq	ymm30, ymm29, ymm28	 # AVX512{DQ,VL}
	vpmullq	ymm30{k7}, ymm29, ymm28	 # AVX512{DQ,VL}
	vpmullq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{DQ,VL}
	vpmullq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vpmullq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vpmullq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{DQ,VL}
	vpmullq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vpmullq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vpmullq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vpmullq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vpmullq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vpmullq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{DQ,VL}
	vpmullq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vpmullq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{DQ,VL}
	vrangepd	xmm30, xmm29, xmm28, 0xab	 # AVX512{DQ,VL}
	vrangepd	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{DQ,VL}
	vrangepd	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{DQ,VL}
	vrangepd	xmm30, xmm29, xmm28, 123	 # AVX512{DQ,VL}
	vrangepd	xmm30, xmm29, XMMWORD PTR [rcx], 123	 # AVX512{DQ,VL}
	vrangepd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{DQ,VL}
	vrangepd	xmm30, xmm29, [rcx]{1to2}, 123	 # AVX512{DQ,VL}
	vrangepd	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{DQ,VL} Disp8
	vrangepd	xmm30, xmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{DQ,VL}
	vrangepd	xmm30, xmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{DQ,VL} Disp8
	vrangepd	xmm30, xmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{DQ,VL}
	vrangepd	xmm30, xmm29, [rdx+1016]{1to2}, 123	 # AVX512{DQ,VL} Disp8
	vrangepd	xmm30, xmm29, [rdx+1024]{1to2}, 123	 # AVX512{DQ,VL}
	vrangepd	xmm30, xmm29, [rdx-1024]{1to2}, 123	 # AVX512{DQ,VL} Disp8
	vrangepd	xmm30, xmm29, [rdx-1032]{1to2}, 123	 # AVX512{DQ,VL}
	vrangepd	ymm30, ymm29, ymm28, 0xab	 # AVX512{DQ,VL}
	vrangepd	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{DQ,VL}
	vrangepd	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{DQ,VL}
	vrangepd	ymm30, ymm29, ymm28, 123	 # AVX512{DQ,VL}
	vrangepd	ymm30, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{DQ,VL}
	vrangepd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{DQ,VL}
	vrangepd	ymm30, ymm29, [rcx]{1to4}, 123	 # AVX512{DQ,VL}
	vrangepd	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{DQ,VL} Disp8
	vrangepd	ymm30, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{DQ,VL}
	vrangepd	ymm30, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{DQ,VL} Disp8
	vrangepd	ymm30, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{DQ,VL}
	vrangepd	ymm30, ymm29, [rdx+1016]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vrangepd	ymm30, ymm29, [rdx+1024]{1to4}, 123	 # AVX512{DQ,VL}
	vrangepd	ymm30, ymm29, [rdx-1024]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vrangepd	ymm30, ymm29, [rdx-1032]{1to4}, 123	 # AVX512{DQ,VL}
	vrangeps	xmm30, xmm29, xmm28, 0xab	 # AVX512{DQ,VL}
	vrangeps	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{DQ,VL}
	vrangeps	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{DQ,VL}
	vrangeps	xmm30, xmm29, xmm28, 123	 # AVX512{DQ,VL}
	vrangeps	xmm30, xmm29, XMMWORD PTR [rcx], 123	 # AVX512{DQ,VL}
	vrangeps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{DQ,VL}
	vrangeps	xmm30, xmm29, [rcx]{1to4}, 123	 # AVX512{DQ,VL}
	vrangeps	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{DQ,VL} Disp8
	vrangeps	xmm30, xmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{DQ,VL}
	vrangeps	xmm30, xmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{DQ,VL} Disp8
	vrangeps	xmm30, xmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{DQ,VL}
	vrangeps	xmm30, xmm29, [rdx+508]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vrangeps	xmm30, xmm29, [rdx+512]{1to4}, 123	 # AVX512{DQ,VL}
	vrangeps	xmm30, xmm29, [rdx-512]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vrangeps	xmm30, xmm29, [rdx-516]{1to4}, 123	 # AVX512{DQ,VL}
	vrangeps	ymm30, ymm29, ymm28, 0xab	 # AVX512{DQ,VL}
	vrangeps	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{DQ,VL}
	vrangeps	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{DQ,VL}
	vrangeps	ymm30, ymm29, ymm28, 123	 # AVX512{DQ,VL}
	vrangeps	ymm30, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{DQ,VL}
	vrangeps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{DQ,VL}
	vrangeps	ymm30, ymm29, [rcx]{1to8}, 123	 # AVX512{DQ,VL}
	vrangeps	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{DQ,VL} Disp8
	vrangeps	ymm30, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{DQ,VL}
	vrangeps	ymm30, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{DQ,VL} Disp8
	vrangeps	ymm30, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{DQ,VL}
	vrangeps	ymm30, ymm29, [rdx+508]{1to8}, 123	 # AVX512{DQ,VL} Disp8
	vrangeps	ymm30, ymm29, [rdx+512]{1to8}, 123	 # AVX512{DQ,VL}
	vrangeps	ymm30, ymm29, [rdx-512]{1to8}, 123	 # AVX512{DQ,VL} Disp8
	vrangeps	ymm30, ymm29, [rdx-516]{1to8}, 123	 # AVX512{DQ,VL}
	vandpd	xmm30, xmm29, xmm28	 # AVX512{DQ,VL}
	vandpd	xmm30{k7}, xmm29, xmm28	 # AVX512{DQ,VL}
	vandpd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{DQ,VL}
	vandpd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vandpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vandpd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{DQ,VL}
	vandpd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vandpd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vandpd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vandpd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vandpd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vandpd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{DQ,VL}
	vandpd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vandpd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{DQ,VL}
	vandpd	ymm30, ymm29, ymm28	 # AVX512{DQ,VL}
	vandpd	ymm30{k7}, ymm29, ymm28	 # AVX512{DQ,VL}
	vandpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{DQ,VL}
	vandpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vandpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vandpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{DQ,VL}
	vandpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vandpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vandpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vandpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vandpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vandpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{DQ,VL}
	vandpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vandpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{DQ,VL}
	vandps	xmm30, xmm29, xmm28	 # AVX512{DQ,VL}
	vandps	xmm30{k7}, xmm29, xmm28	 # AVX512{DQ,VL}
	vandps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{DQ,VL}
	vandps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vandps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vandps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{DQ,VL}
	vandps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vandps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vandps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vandps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vandps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vandps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{DQ,VL}
	vandps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vandps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{DQ,VL}
	vandps	ymm30, ymm29, ymm28	 # AVX512{DQ,VL}
	vandps	ymm30{k7}, ymm29, ymm28	 # AVX512{DQ,VL}
	vandps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{DQ,VL}
	vandps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vandps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vandps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{DQ,VL}
	vandps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vandps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vandps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vandps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vandps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{DQ,VL} Disp8
	vandps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{DQ,VL}
	vandps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{DQ,VL} Disp8
	vandps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{DQ,VL}
	vandnpd	xmm30, xmm29, xmm28	 # AVX512{DQ,VL}
	vandnpd	xmm30{k7}, xmm29, xmm28	 # AVX512{DQ,VL}
	vandnpd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{DQ,VL}
	vandnpd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vandnpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vandnpd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{DQ,VL}
	vandnpd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vandnpd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vandnpd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vandnpd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vandnpd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vandnpd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{DQ,VL}
	vandnpd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vandnpd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{DQ,VL}
	vandnpd	ymm30, ymm29, ymm28	 # AVX512{DQ,VL}
	vandnpd	ymm30{k7}, ymm29, ymm28	 # AVX512{DQ,VL}
	vandnpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{DQ,VL}
	vandnpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vandnpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vandnpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{DQ,VL}
	vandnpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vandnpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vandnpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vandnpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vandnpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vandnpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{DQ,VL}
	vandnpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vandnpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{DQ,VL}
	vandnps	xmm30, xmm29, xmm28	 # AVX512{DQ,VL}
	vandnps	xmm30{k7}, xmm29, xmm28	 # AVX512{DQ,VL}
	vandnps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{DQ,VL}
	vandnps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vandnps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vandnps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{DQ,VL}
	vandnps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vandnps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vandnps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vandnps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vandnps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vandnps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{DQ,VL}
	vandnps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vandnps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{DQ,VL}
	vandnps	ymm30, ymm29, ymm28	 # AVX512{DQ,VL}
	vandnps	ymm30{k7}, ymm29, ymm28	 # AVX512{DQ,VL}
	vandnps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{DQ,VL}
	vandnps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vandnps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vandnps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{DQ,VL}
	vandnps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vandnps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vandnps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vandnps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vandnps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{DQ,VL} Disp8
	vandnps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{DQ,VL}
	vandnps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{DQ,VL} Disp8
	vandnps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{DQ,VL}
	vorpd	xmm30, xmm29, xmm28	 # AVX512{DQ,VL}
	vorpd	xmm30{k7}, xmm29, xmm28	 # AVX512{DQ,VL}
	vorpd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{DQ,VL}
	vorpd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vorpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vorpd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{DQ,VL}
	vorpd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vorpd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vorpd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vorpd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vorpd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vorpd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{DQ,VL}
	vorpd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vorpd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{DQ,VL}
	vorpd	ymm30, ymm29, ymm28	 # AVX512{DQ,VL}
	vorpd	ymm30{k7}, ymm29, ymm28	 # AVX512{DQ,VL}
	vorpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{DQ,VL}
	vorpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vorpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vorpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{DQ,VL}
	vorpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vorpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vorpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vorpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vorpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vorpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{DQ,VL}
	vorpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vorpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{DQ,VL}
	vorps	xmm30, xmm29, xmm28	 # AVX512{DQ,VL}
	vorps	xmm30{k7}, xmm29, xmm28	 # AVX512{DQ,VL}
	vorps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{DQ,VL}
	vorps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vorps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vorps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{DQ,VL}
	vorps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vorps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vorps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vorps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vorps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vorps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{DQ,VL}
	vorps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vorps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{DQ,VL}
	vorps	ymm30, ymm29, ymm28	 # AVX512{DQ,VL}
	vorps	ymm30{k7}, ymm29, ymm28	 # AVX512{DQ,VL}
	vorps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{DQ,VL}
	vorps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vorps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vorps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{DQ,VL}
	vorps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vorps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vorps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vorps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vorps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{DQ,VL} Disp8
	vorps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{DQ,VL}
	vorps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{DQ,VL} Disp8
	vorps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{DQ,VL}
	vxorpd	xmm30, xmm29, xmm28	 # AVX512{DQ,VL}
	vxorpd	xmm30{k7}, xmm29, xmm28	 # AVX512{DQ,VL}
	vxorpd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{DQ,VL}
	vxorpd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vxorpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vxorpd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{DQ,VL}
	vxorpd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vxorpd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vxorpd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vxorpd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vxorpd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vxorpd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{DQ,VL}
	vxorpd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vxorpd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{DQ,VL}
	vxorpd	ymm30, ymm29, ymm28	 # AVX512{DQ,VL}
	vxorpd	ymm30{k7}, ymm29, ymm28	 # AVX512{DQ,VL}
	vxorpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{DQ,VL}
	vxorpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vxorpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vxorpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{DQ,VL}
	vxorpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vxorpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vxorpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vxorpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vxorpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vxorpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{DQ,VL}
	vxorpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vxorpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{DQ,VL}
	vxorps	xmm30, xmm29, xmm28	 # AVX512{DQ,VL}
	vxorps	xmm30{k7}, xmm29, xmm28	 # AVX512{DQ,VL}
	vxorps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{DQ,VL}
	vxorps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vxorps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vxorps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{DQ,VL}
	vxorps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vxorps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vxorps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vxorps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vxorps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vxorps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{DQ,VL}
	vxorps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vxorps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{DQ,VL}
	vxorps	ymm30, ymm29, ymm28	 # AVX512{DQ,VL}
	vxorps	ymm30{k7}, ymm29, ymm28	 # AVX512{DQ,VL}
	vxorps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{DQ,VL}
	vxorps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vxorps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vxorps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{DQ,VL}
	vxorps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vxorps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vxorps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vxorps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vxorps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{DQ,VL} Disp8
	vxorps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{DQ,VL}
	vxorps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{DQ,VL} Disp8
	vxorps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{DQ,VL}
	vreducepd	xmm30, xmm29, 0xab	 # AVX512{DQ,VL}
	vreducepd	xmm30{k7}, xmm29, 0xab	 # AVX512{DQ,VL}
	vreducepd	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{DQ,VL}
	vreducepd	xmm30, xmm29, 123	 # AVX512{DQ,VL}
	vreducepd	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{DQ,VL}
	vreducepd	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{DQ,VL}
	vreducepd	xmm30, [rcx]{1to2}, 123	 # AVX512{DQ,VL}
	vreducepd	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{DQ,VL} Disp8
	vreducepd	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{DQ,VL}
	vreducepd	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{DQ,VL} Disp8
	vreducepd	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{DQ,VL}
	vreducepd	xmm30, [rdx+1016]{1to2}, 123	 # AVX512{DQ,VL} Disp8
	vreducepd	xmm30, [rdx+1024]{1to2}, 123	 # AVX512{DQ,VL}
	vreducepd	xmm30, [rdx-1024]{1to2}, 123	 # AVX512{DQ,VL} Disp8
	vreducepd	xmm30, [rdx-1032]{1to2}, 123	 # AVX512{DQ,VL}
	vreducepd	ymm30, ymm29, 0xab	 # AVX512{DQ,VL}
	vreducepd	ymm30{k7}, ymm29, 0xab	 # AVX512{DQ,VL}
	vreducepd	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{DQ,VL}
	vreducepd	ymm30, ymm29, 123	 # AVX512{DQ,VL}
	vreducepd	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{DQ,VL}
	vreducepd	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{DQ,VL}
	vreducepd	ymm30, [rcx]{1to4}, 123	 # AVX512{DQ,VL}
	vreducepd	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{DQ,VL} Disp8
	vreducepd	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{DQ,VL}
	vreducepd	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{DQ,VL} Disp8
	vreducepd	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{DQ,VL}
	vreducepd	ymm30, [rdx+1016]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vreducepd	ymm30, [rdx+1024]{1to4}, 123	 # AVX512{DQ,VL}
	vreducepd	ymm30, [rdx-1024]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vreducepd	ymm30, [rdx-1032]{1to4}, 123	 # AVX512{DQ,VL}
	vreduceps	xmm30, xmm29, 0xab	 # AVX512{DQ,VL}
	vreduceps	xmm30{k7}, xmm29, 0xab	 # AVX512{DQ,VL}
	vreduceps	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{DQ,VL}
	vreduceps	xmm30, xmm29, 123	 # AVX512{DQ,VL}
	vreduceps	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{DQ,VL}
	vreduceps	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{DQ,VL}
	vreduceps	xmm30, [rcx]{1to4}, 123	 # AVX512{DQ,VL}
	vreduceps	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{DQ,VL} Disp8
	vreduceps	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{DQ,VL}
	vreduceps	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{DQ,VL} Disp8
	vreduceps	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{DQ,VL}
	vreduceps	xmm30, [rdx+508]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vreduceps	xmm30, [rdx+512]{1to4}, 123	 # AVX512{DQ,VL}
	vreduceps	xmm30, [rdx-512]{1to4}, 123	 # AVX512{DQ,VL} Disp8
	vreduceps	xmm30, [rdx-516]{1to4}, 123	 # AVX512{DQ,VL}
	vreduceps	ymm30, ymm29, 0xab	 # AVX512{DQ,VL}
	vreduceps	ymm30{k7}, ymm29, 0xab	 # AVX512{DQ,VL}
	vreduceps	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{DQ,VL}
	vreduceps	ymm30, ymm29, 123	 # AVX512{DQ,VL}
	vreduceps	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{DQ,VL}
	vreduceps	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{DQ,VL}
	vreduceps	ymm30, [rcx]{1to8}, 123	 # AVX512{DQ,VL}
	vreduceps	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{DQ,VL} Disp8
	vreduceps	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{DQ,VL}
	vreduceps	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{DQ,VL} Disp8
	vreduceps	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{DQ,VL}
	vreduceps	ymm30, [rdx+508]{1to8}, 123	 # AVX512{DQ,VL} Disp8
	vreduceps	ymm30, [rdx+512]{1to8}, 123	 # AVX512{DQ,VL}
	vreduceps	ymm30, [rdx-512]{1to8}, 123	 # AVX512{DQ,VL} Disp8
	vreduceps	ymm30, [rdx-516]{1to8}, 123	 # AVX512{DQ,VL}
	vextractf64x2	XMMWORD PTR [rcx], ymm29, 0xab	 # AVX512{DQ,VL}
	vextractf64x2	XMMWORD PTR [rcx]{k7}, ymm29, 0xab	 # AVX512{DQ,VL}
	vextractf64x2	XMMWORD PTR [rcx], ymm29, 123	 # AVX512{DQ,VL}
	vextractf64x2	XMMWORD PTR [rax+r14*8+0x1234], ymm29, 123	 # AVX512{DQ,VL}
	vextractf64x2	XMMWORD PTR [rdx+2032], ymm29, 123	 # AVX512{DQ,VL} Disp8
	vextractf64x2	XMMWORD PTR [rdx+2048], ymm29, 123	 # AVX512{DQ,VL}
	vextractf64x2	XMMWORD PTR [rdx-2048], ymm29, 123	 # AVX512{DQ,VL} Disp8
	vextractf64x2	XMMWORD PTR [rdx-2064], ymm29, 123	 # AVX512{DQ,VL}
	vextracti64x2	XMMWORD PTR [rcx], ymm29, 0xab	 # AVX512{DQ,VL}
	vextracti64x2	XMMWORD PTR [rcx]{k7}, ymm29, 0xab	 # AVX512{DQ,VL}
	vextracti64x2	XMMWORD PTR [rcx], ymm29, 123	 # AVX512{DQ,VL}
	vextracti64x2	XMMWORD PTR [rax+r14*8+0x1234], ymm29, 123	 # AVX512{DQ,VL}
	vextracti64x2	XMMWORD PTR [rdx+2032], ymm29, 123	 # AVX512{DQ,VL} Disp8
	vextracti64x2	XMMWORD PTR [rdx+2048], ymm29, 123	 # AVX512{DQ,VL}
	vextracti64x2	XMMWORD PTR [rdx-2048], ymm29, 123	 # AVX512{DQ,VL} Disp8
	vextracti64x2	XMMWORD PTR [rdx-2064], ymm29, 123	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm30, xmm29	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm30, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm30, [rcx]{1to2}	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm30, [rdx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	xmm30, [rdx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvttpd2qq	xmm30, [rdx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	xmm30, [rdx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm30, ymm29	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm30{k7}, ymm29	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm30{k7}{z}, ymm29	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm30, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm30, [rcx]{1to4}	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm30, [rdx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	ymm30, [rdx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvttpd2qq	ymm30, [rdx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttpd2qq	ymm30, [rdx-1032]{1to4}	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm30, xmm29	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm30, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm30, [rcx]{1to2}	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm30, [rdx+1016]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	xmm30, [rdx+1024]{1to2}	 # AVX512{DQ,VL}
	vcvttpd2uqq	xmm30, [rdx-1024]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	xmm30, [rdx-1032]{1to2}	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm30, ymm29	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm30{k7}, ymm29	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm30{k7}{z}, ymm29	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm30, YMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm30, [rcx]{1to4}	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm30, [rdx+1016]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	ymm30, [rdx+1024]{1to4}	 # AVX512{DQ,VL}
	vcvttpd2uqq	ymm30, [rdx-1024]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttpd2uqq	ymm30, [rdx-1032]{1to4}	 # AVX512{DQ,VL}
	vcvttps2qq	xmm30, xmm29	 # AVX512{DQ,VL}
	vcvttps2qq	xmm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvttps2qq	xmm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvttps2qq	xmm30, QWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvttps2qq	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvttps2qq	xmm30, [rcx]{1to2}	 # AVX512{DQ,VL}
	vcvttps2qq	xmm30, QWORD PTR [rdx+1016]	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	xmm30, QWORD PTR [rdx+1024]	 # AVX512{DQ,VL}
	vcvttps2qq	xmm30, QWORD PTR [rdx-1024]	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	xmm30, QWORD PTR [rdx-1032]	 # AVX512{DQ,VL}
	vcvttps2qq	xmm30, [rdx+508]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	xmm30, [rdx+512]{1to2}	 # AVX512{DQ,VL}
	vcvttps2qq	xmm30, [rdx-512]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	xmm30, [rdx-516]{1to2}	 # AVX512{DQ,VL}
	vcvttps2qq	xmm30, DWORD BCST [rdx+508]	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	ymm30, xmm29	 # AVX512{DQ,VL}
	vcvttps2qq	ymm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvttps2qq	ymm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvttps2qq	ymm30, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvttps2qq	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvttps2qq	ymm30, [rcx]{1to4}	 # AVX512{DQ,VL}
	vcvttps2qq	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vcvttps2qq	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vcvttps2qq	ymm30, [rdx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	ymm30, [rdx+512]{1to4}	 # AVX512{DQ,VL}
	vcvttps2qq	ymm30, [rdx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttps2qq	ymm30, [rdx-516]{1to4}	 # AVX512{DQ,VL}
	vcvttps2qq	ymm30, DWORD BCST [rdx+508]	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	xmm30, xmm29	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm30, QWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm30, [rcx]{1to2}	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm30, QWORD PTR [rdx+1016]	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	xmm30, QWORD PTR [rdx+1024]	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm30, QWORD PTR [rdx-1024]	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	xmm30, QWORD PTR [rdx-1032]	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm30, [rdx+508]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	xmm30, [rdx+512]{1to2}	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm30, [rdx-512]{1to2}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	xmm30, [rdx-516]{1to2}	 # AVX512{DQ,VL}
	vcvttps2uqq	xmm30, DWORD BCST [rdx+508]	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	ymm30, xmm29	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm30{k7}, xmm29	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm30{k7}{z}, xmm29	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm30, XMMWORD PTR [rcx]	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm30, [rcx]{1to4}	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm30, [rdx+508]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	ymm30, [rdx+512]{1to4}	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm30, [rdx-512]{1to4}	 # AVX512{DQ,VL} Disp8
	vcvttps2uqq	ymm30, [rdx-516]{1to4}	 # AVX512{DQ,VL}
	vcvttps2uqq	ymm30, DWORD BCST [rdx+508]	 # AVX512{DQ,VL} Disp8
	vpmovd2m	k5, xmm30	 # AVX512{DQ,VL}
	vpmovd2m	k5, ymm30	 # AVX512{DQ,VL}
	vpmovq2m	k5, xmm30	 # AVX512{DQ,VL}
	vpmovq2m	k5, ymm30	 # AVX512{DQ,VL}
	vpmovm2d	xmm30, k5	 # AVX512{DQ,VL}
	vpmovm2d	ymm30, k5	 # AVX512{DQ,VL}
	vpmovm2q	xmm30, k5	 # AVX512{DQ,VL}
	vpmovm2q	ymm30, k5	 # AVX512{DQ,VL}
