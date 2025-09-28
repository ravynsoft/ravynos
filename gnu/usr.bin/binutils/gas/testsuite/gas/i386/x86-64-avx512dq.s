# Check 64bit AVX512DQ instructions

	.allow_index_reg
	.text
_start:
	vbroadcastf32x8	(%rcx), %zmm30	 # AVX512DQ
	vbroadcastf32x8	(%rcx), %zmm30{%k7}	 # AVX512DQ
	vbroadcastf32x8	(%rcx), %zmm30{%k7}{z}	 # AVX512DQ
	vbroadcastf32x8	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vbroadcastf32x8	4064(%rdx), %zmm30	 # AVX512DQ Disp8
	vbroadcastf32x8	4096(%rdx), %zmm30	 # AVX512DQ
	vbroadcastf32x8	-4096(%rdx), %zmm30	 # AVX512DQ Disp8
	vbroadcastf32x8	-4128(%rdx), %zmm30	 # AVX512DQ
	vbroadcastf64x2	(%rcx), %zmm30	 # AVX512DQ
	vbroadcastf64x2	(%rcx), %zmm30{%k7}	 # AVX512DQ
	vbroadcastf64x2	(%rcx), %zmm30{%k7}{z}	 # AVX512DQ
	vbroadcastf64x2	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vbroadcastf64x2	2032(%rdx), %zmm30	 # AVX512DQ Disp8
	vbroadcastf64x2	2048(%rdx), %zmm30	 # AVX512DQ
	vbroadcastf64x2	-2048(%rdx), %zmm30	 # AVX512DQ Disp8
	vbroadcastf64x2	-2064(%rdx), %zmm30	 # AVX512DQ
	vbroadcasti32x8	(%rcx), %zmm30	 # AVX512DQ
	vbroadcasti32x8	(%rcx), %zmm30{%k7}	 # AVX512DQ
	vbroadcasti32x8	(%rcx), %zmm30{%k7}{z}	 # AVX512DQ
	vbroadcasti32x8	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vbroadcasti32x8	4064(%rdx), %zmm30	 # AVX512DQ Disp8
	vbroadcasti32x8	4096(%rdx), %zmm30	 # AVX512DQ
	vbroadcasti32x8	-4096(%rdx), %zmm30	 # AVX512DQ Disp8
	vbroadcasti32x8	-4128(%rdx), %zmm30	 # AVX512DQ
	vbroadcasti64x2	(%rcx), %zmm30	 # AVX512DQ
	vbroadcasti64x2	(%rcx), %zmm30{%k7}	 # AVX512DQ
	vbroadcasti64x2	(%rcx), %zmm30{%k7}{z}	 # AVX512DQ
	vbroadcasti64x2	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vbroadcasti64x2	2032(%rdx), %zmm30	 # AVX512DQ Disp8
	vbroadcasti64x2	2048(%rdx), %zmm30	 # AVX512DQ
	vbroadcasti64x2	-2048(%rdx), %zmm30	 # AVX512DQ Disp8
	vbroadcasti64x2	-2064(%rdx), %zmm30	 # AVX512DQ
	vbroadcastf32x2	%xmm31, %zmm30	 # AVX512DQ
	vbroadcastf32x2	%xmm31, %zmm30{%k7}	 # AVX512DQ
	vbroadcastf32x2	%xmm31, %zmm30{%k7}{z}	 # AVX512DQ
	vbroadcastf32x2	(%rcx), %zmm30	 # AVX512DQ
	vbroadcastf32x2	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vbroadcastf32x2	1016(%rdx), %zmm30	 # AVX512DQ Disp8
	vbroadcastf32x2	1024(%rdx), %zmm30	 # AVX512DQ
	vbroadcastf32x2	-1024(%rdx), %zmm30	 # AVX512DQ Disp8
	vbroadcastf32x2	-1032(%rdx), %zmm30	 # AVX512DQ
	vcvtpd2qq	%zmm29, %zmm30	 # AVX512DQ
	vcvtpd2qq	%zmm29, %zmm30{%k7}	 # AVX512DQ
	vcvtpd2qq	%zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vcvtpd2qq	{rn-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtpd2qq	{ru-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtpd2qq	{rd-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtpd2qq	{rz-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtpd2qq	(%rcx), %zmm30	 # AVX512DQ
	vcvtpd2qq	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vcvtpd2qq	(%rcx){1to8}, %zmm30	 # AVX512DQ
	vcvtpd2qq	8128(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvtpd2qq	8192(%rdx), %zmm30	 # AVX512DQ
	vcvtpd2qq	-8192(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvtpd2qq	-8256(%rdx), %zmm30	 # AVX512DQ
	vcvtpd2qq	1016(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvtpd2qq	1024(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvtpd2qq	-1024(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvtpd2qq	-1032(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvtpd2uqq	%zmm29, %zmm30	 # AVX512DQ
	vcvtpd2uqq	%zmm29, %zmm30{%k7}	 # AVX512DQ
	vcvtpd2uqq	%zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vcvtpd2uqq	{rn-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtpd2uqq	{ru-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtpd2uqq	{rd-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtpd2uqq	{rz-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtpd2uqq	(%rcx), %zmm30	 # AVX512DQ
	vcvtpd2uqq	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vcvtpd2uqq	(%rcx){1to8}, %zmm30	 # AVX512DQ
	vcvtpd2uqq	8128(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvtpd2uqq	8192(%rdx), %zmm30	 # AVX512DQ
	vcvtpd2uqq	-8192(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvtpd2uqq	-8256(%rdx), %zmm30	 # AVX512DQ
	vcvtpd2uqq	1016(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvtpd2uqq	1024(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvtpd2uqq	-1024(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvtpd2uqq	-1032(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvtps2qq	%ymm29, %zmm30	 # AVX512DQ
	vcvtps2qq	%ymm29, %zmm30{%k7}	 # AVX512DQ
	vcvtps2qq	%ymm29, %zmm30{%k7}{z}	 # AVX512DQ
	vcvtps2qq	{rn-sae}, %ymm29, %zmm30	 # AVX512DQ
	vcvtps2qq	{ru-sae}, %ymm29, %zmm30	 # AVX512DQ
	vcvtps2qq	{rd-sae}, %ymm29, %zmm30	 # AVX512DQ
	vcvtps2qq	{rz-sae}, %ymm29, %zmm30	 # AVX512DQ
	vcvtps2qq	(%rcx), %zmm30	 # AVX512DQ
	vcvtps2qq	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vcvtps2qq	(%rcx){1to8}, %zmm30	 # AVX512DQ
	vcvtps2qq	4064(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvtps2qq	4096(%rdx), %zmm30	 # AVX512DQ
	vcvtps2qq	-4096(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvtps2qq	-4128(%rdx), %zmm30	 # AVX512DQ
	vcvtps2qq	508(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvtps2qq	512(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvtps2qq	-512(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvtps2qq	-516(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvtps2uqq	%ymm29, %zmm30	 # AVX512DQ
	vcvtps2uqq	%ymm29, %zmm30{%k7}	 # AVX512DQ
	vcvtps2uqq	%ymm29, %zmm30{%k7}{z}	 # AVX512DQ
	vcvtps2uqq	{rn-sae}, %ymm29, %zmm30	 # AVX512DQ
	vcvtps2uqq	{ru-sae}, %ymm29, %zmm30	 # AVX512DQ
	vcvtps2uqq	{rd-sae}, %ymm29, %zmm30	 # AVX512DQ
	vcvtps2uqq	{rz-sae}, %ymm29, %zmm30	 # AVX512DQ
	vcvtps2uqq	(%rcx), %zmm30	 # AVX512DQ
	vcvtps2uqq	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vcvtps2uqq	(%rcx){1to8}, %zmm30	 # AVX512DQ
	vcvtps2uqq	4064(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvtps2uqq	4096(%rdx), %zmm30	 # AVX512DQ
	vcvtps2uqq	-4096(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvtps2uqq	-4128(%rdx), %zmm30	 # AVX512DQ
	vcvtps2uqq	508(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvtps2uqq	512(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvtps2uqq	-512(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvtps2uqq	-516(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvtqq2pd	%zmm29, %zmm30	 # AVX512DQ
	vcvtqq2pd	%zmm29, %zmm30{%k7}	 # AVX512DQ
	vcvtqq2pd	%zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vcvtqq2pd	{rn-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtqq2pd	{ru-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtqq2pd	{rd-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtqq2pd	{rz-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtqq2pd	(%rcx), %zmm30	 # AVX512DQ
	vcvtqq2pd	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vcvtqq2pd	(%rcx){1to8}, %zmm30	 # AVX512DQ
	vcvtqq2pd	8128(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvtqq2pd	8192(%rdx), %zmm30	 # AVX512DQ
	vcvtqq2pd	-8192(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvtqq2pd	-8256(%rdx), %zmm30	 # AVX512DQ
	vcvtqq2pd	1016(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvtqq2pd	1024(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvtqq2pd	-1024(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvtqq2pd	-1032(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvtqq2ps	%zmm29, %ymm30	 # AVX512DQ
	vcvtqq2ps	%zmm29, %ymm30{%k7}	 # AVX512DQ
	vcvtqq2ps	%zmm29, %ymm30{%k7}{z}	 # AVX512DQ
	vcvtqq2ps	{rn-sae}, %zmm29, %ymm30	 # AVX512DQ
	vcvtqq2ps	{ru-sae}, %zmm29, %ymm30	 # AVX512DQ
	vcvtqq2ps	{rd-sae}, %zmm29, %ymm30	 # AVX512DQ
	vcvtqq2ps	{rz-sae}, %zmm29, %ymm30	 # AVX512DQ
	vcvtqq2ps	(%rcx), %ymm30	 # AVX512DQ
	vcvtqq2ps	0x123(%rax,%r14,8), %ymm30	 # AVX512DQ
	vcvtqq2ps	(%rcx){1to8}, %ymm30	 # AVX512DQ
	vcvtqq2ps	8128(%rdx), %ymm30	 # AVX512DQ Disp8
	vcvtqq2ps	8192(%rdx), %ymm30	 # AVX512DQ
	vcvtqq2ps	-8192(%rdx), %ymm30	 # AVX512DQ Disp8
	vcvtqq2ps	-8256(%rdx), %ymm30	 # AVX512DQ
	vcvtqq2ps	1016(%rdx){1to8}, %ymm30	 # AVX512DQ Disp8
	vcvtqq2ps	1024(%rdx){1to8}, %ymm30	 # AVX512DQ
	vcvtqq2ps	-1024(%rdx){1to8}, %ymm30	 # AVX512DQ Disp8
	vcvtqq2ps	-1032(%rdx){1to8}, %ymm30	 # AVX512DQ
	vcvtuqq2pd	%zmm29, %zmm30	 # AVX512DQ
	vcvtuqq2pd	%zmm29, %zmm30{%k7}	 # AVX512DQ
	vcvtuqq2pd	%zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vcvtuqq2pd	{rn-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtuqq2pd	{ru-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtuqq2pd	{rd-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtuqq2pd	{rz-sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvtuqq2pd	(%rcx), %zmm30	 # AVX512DQ
	vcvtuqq2pd	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vcvtuqq2pd	(%rcx){1to8}, %zmm30	 # AVX512DQ
	vcvtuqq2pd	8128(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvtuqq2pd	8192(%rdx), %zmm30	 # AVX512DQ
	vcvtuqq2pd	-8192(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvtuqq2pd	-8256(%rdx), %zmm30	 # AVX512DQ
	vcvtuqq2pd	1016(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvtuqq2pd	1024(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvtuqq2pd	-1024(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvtuqq2pd	-1032(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvtuqq2ps	%zmm29, %ymm30	 # AVX512DQ
	vcvtuqq2ps	%zmm29, %ymm30{%k7}	 # AVX512DQ
	vcvtuqq2ps	%zmm29, %ymm30{%k7}{z}	 # AVX512DQ
	vcvtuqq2ps	{rn-sae}, %zmm29, %ymm30	 # AVX512DQ
	vcvtuqq2ps	{ru-sae}, %zmm29, %ymm30	 # AVX512DQ
	vcvtuqq2ps	{rd-sae}, %zmm29, %ymm30	 # AVX512DQ
	vcvtuqq2ps	{rz-sae}, %zmm29, %ymm30	 # AVX512DQ
	vcvtuqq2ps	(%rcx), %ymm30	 # AVX512DQ
	vcvtuqq2ps	0x123(%rax,%r14,8), %ymm30	 # AVX512DQ
	vcvtuqq2ps	(%rcx){1to8}, %ymm30	 # AVX512DQ
	vcvtuqq2ps	8128(%rdx), %ymm30	 # AVX512DQ Disp8
	vcvtuqq2ps	8192(%rdx), %ymm30	 # AVX512DQ
	vcvtuqq2ps	-8192(%rdx), %ymm30	 # AVX512DQ Disp8
	vcvtuqq2ps	-8256(%rdx), %ymm30	 # AVX512DQ
	vcvtuqq2ps	1016(%rdx){1to8}, %ymm30	 # AVX512DQ Disp8
	vcvtuqq2ps	1024(%rdx){1to8}, %ymm30	 # AVX512DQ
	vcvtuqq2ps	-1024(%rdx){1to8}, %ymm30	 # AVX512DQ Disp8
	vcvtuqq2ps	-1032(%rdx){1to8}, %ymm30	 # AVX512DQ
	vextractf64x2	$0xab, %zmm29, %xmm30	 # AVX512DQ
	vextractf64x2	$0xab, %zmm29, %xmm30{%k7}	 # AVX512DQ
	vextractf64x2	$0xab, %zmm29, %xmm30{%k7}{z}	 # AVX512DQ
	vextractf64x2	$123, %zmm29, %xmm30	 # AVX512DQ
	vextractf32x8	$0xab, %zmm29, %ymm30	 # AVX512DQ
	vextractf32x8	$0xab, %zmm29, %ymm30{%k7}	 # AVX512DQ
	vextractf32x8	$0xab, %zmm29, %ymm30{%k7}{z}	 # AVX512DQ
	vextractf32x8	$123, %zmm29, %ymm30	 # AVX512DQ
	vextracti64x2	$0xab, %zmm29, %xmm30	 # AVX512DQ
	vextracti64x2	$0xab, %zmm29, %xmm30{%k7}	 # AVX512DQ
	vextracti64x2	$0xab, %zmm29, %xmm30{%k7}{z}	 # AVX512DQ
	vextracti64x2	$123, %zmm29, %xmm30	 # AVX512DQ
	vextracti32x8	$0xab, %zmm29, %ymm30	 # AVX512DQ
	vextracti32x8	$0xab, %zmm29, %ymm30{%k7}	 # AVX512DQ
	vextracti32x8	$0xab, %zmm29, %ymm30{%k7}{z}	 # AVX512DQ
	vextracti32x8	$123, %zmm29, %ymm30	 # AVX512DQ
	vfpclasspd	$0xab, %zmm30, %k5	 # AVX512DQ
	vfpclasspd	$0xab, %zmm30, %k5{%k7}	 # AVX512DQ
	vfpclasspd	$123, %zmm30, %k5	 # AVX512DQ
	vfpclasspdz	$123, (%rcx), %k5	 # AVX512DQ
	vfpclasspdz	$123, 0x123(%rax,%r14,8), %k5	 # AVX512DQ
	vfpclasspd	$123, (%rcx){1to8}, %k5	 # AVX512DQ
	vfpclasspdz	$123, 8128(%rdx), %k5	 # AVX512DQ Disp8
	vfpclasspdz	$123, 8192(%rdx), %k5	 # AVX512DQ
	vfpclasspdz	$123, -8192(%rdx), %k5	 # AVX512DQ Disp8
	vfpclasspdz	$123, -8256(%rdx), %k5	 # AVX512DQ
	vfpclasspdz	$123, 1016(%rdx){1to8}, %k5	 # AVX512DQ Disp8
	vfpclasspdz	$123, 1024(%rdx){1to8}, %k5	 # AVX512DQ
	vfpclasspdz	$123, -1024(%rdx){1to8}, %k5	 # AVX512DQ Disp8
	vfpclasspdz	$123, -1032(%rdx){1to8}, %k5	 # AVX512DQ
	vfpclassps	$0xab, %zmm30, %k5	 # AVX512DQ
	vfpclassps	$0xab, %zmm30, %k5{%k7}	 # AVX512DQ
	vfpclassps	$123, %zmm30, %k5	 # AVX512DQ
	vfpclasspsz	$123, (%rcx), %k5	 # AVX512DQ
	vfpclasspsz	$123, 0x123(%rax,%r14,8), %k5	 # AVX512DQ
	vfpclassps	$123, (%rcx){1to16}, %k5	 # AVX512DQ
	vfpclasspsz	$123, 8128(%rdx), %k5	 # AVX512DQ Disp8
	vfpclasspsz	$123, 8192(%rdx), %k5	 # AVX512DQ
	vfpclasspsz	$123, -8192(%rdx), %k5	 # AVX512DQ Disp8
	vfpclasspsz	$123, -8256(%rdx), %k5	 # AVX512DQ
	vfpclasspsz	$123, 508(%rdx){1to16}, %k5	 # AVX512DQ Disp8
	vfpclasspsz	$123, 512(%rdx){1to16}, %k5	 # AVX512DQ
	vfpclasspsz	$123, -512(%rdx){1to16}, %k5	 # AVX512DQ Disp8
	vfpclasspsz	$123, -516(%rdx){1to16}, %k5	 # AVX512DQ
	vfpclasssd	$0xab, %xmm30, %k5	 # AVX512DQ
	vfpclasssd	$0xab, %xmm30, %k5{%k7}	 # AVX512DQ
	vfpclasssd	$123, %xmm30, %k5	 # AVX512DQ
	vfpclasssd	$123, (%rcx), %k5	 # AVX512DQ
	vfpclasssd	$123, 0x123(%rax,%r14,8), %k5	 # AVX512DQ
	vfpclasssd	$123, 1016(%rdx), %k5	 # AVX512DQ Disp8
	vfpclasssd	$123, 1024(%rdx), %k5	 # AVX512DQ
	vfpclasssd	$123, -1024(%rdx), %k5	 # AVX512DQ Disp8
	vfpclasssd	$123, -1032(%rdx), %k5	 # AVX512DQ
	vfpclassss	$0xab, %xmm30, %k5	 # AVX512DQ
	vfpclassss	$0xab, %xmm30, %k5{%k7}	 # AVX512DQ
	vfpclassss	$123, %xmm30, %k5	 # AVX512DQ
	vfpclassss	$123, (%rcx), %k5	 # AVX512DQ
	vfpclassss	$123, 0x123(%rax,%r14,8), %k5	 # AVX512DQ
	vfpclassss	$123, 508(%rdx), %k5	 # AVX512DQ Disp8
	vfpclassss	$123, 512(%rdx), %k5	 # AVX512DQ
	vfpclassss	$123, -512(%rdx), %k5	 # AVX512DQ Disp8
	vfpclassss	$123, -516(%rdx), %k5	 # AVX512DQ
	vinsertf64x2	$0xab, %xmm28, %zmm29, %zmm30	 # AVX512DQ
	vinsertf64x2	$0xab, %xmm28, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vinsertf64x2	$0xab, %xmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vinsertf64x2	$123, %xmm28, %zmm29, %zmm30	 # AVX512DQ
	vinsertf64x2	$123, (%rcx), %zmm29, %zmm30	 # AVX512DQ
	vinsertf64x2	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512DQ
	vinsertf64x2	$123, 2032(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vinsertf64x2	$123, 2048(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vinsertf64x2	$123, -2048(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vinsertf64x2	$123, -2064(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vinsertf32x8	$0xab, %ymm28, %zmm29, %zmm30	 # AVX512DQ
	vinsertf32x8	$0xab, %ymm28, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vinsertf32x8	$0xab, %ymm28, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vinsertf32x8	$123, %ymm28, %zmm29, %zmm30	 # AVX512DQ
	vinsertf32x8	$123, (%rcx), %zmm29, %zmm30	 # AVX512DQ
	vinsertf32x8	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512DQ
	vinsertf32x8	$123, 4064(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vinsertf32x8	$123, 4096(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vinsertf32x8	$123, -4096(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vinsertf32x8	$123, -4128(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vinserti64x2	$0xab, %xmm28, %zmm29, %zmm30	 # AVX512DQ
	vinserti64x2	$0xab, %xmm28, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vinserti64x2	$0xab, %xmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vinserti64x2	$123, %xmm28, %zmm29, %zmm30	 # AVX512DQ
	vinserti64x2	$123, (%rcx), %zmm29, %zmm30	 # AVX512DQ
	vinserti64x2	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512DQ
	vinserti64x2	$123, 2032(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vinserti64x2	$123, 2048(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vinserti64x2	$123, -2048(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vinserti64x2	$123, -2064(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vinserti32x8	$0xab, %ymm28, %zmm29, %zmm30	 # AVX512DQ
	vinserti32x8	$0xab, %ymm28, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vinserti32x8	$0xab, %ymm28, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vinserti32x8	$123, %ymm28, %zmm29, %zmm30	 # AVX512DQ
	vinserti32x8	$123, (%rcx), %zmm29, %zmm30	 # AVX512DQ
	vinserti32x8	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512DQ
	vinserti32x8	$123, 4064(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vinserti32x8	$123, 4096(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vinserti32x8	$123, -4096(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vinserti32x8	$123, -4128(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vbroadcasti32x2	%xmm31, %zmm30	 # AVX512DQ
	vbroadcasti32x2	%xmm31, %zmm30{%k7}	 # AVX512DQ
	vbroadcasti32x2	%xmm31, %zmm30{%k7}{z}	 # AVX512DQ
	vbroadcasti32x2	(%rcx), %zmm30	 # AVX512DQ
	vbroadcasti32x2	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vbroadcasti32x2	1016(%rdx), %zmm30	 # AVX512DQ Disp8
	vbroadcasti32x2	1024(%rdx), %zmm30	 # AVX512DQ
	vbroadcasti32x2	-1024(%rdx), %zmm30	 # AVX512DQ Disp8
	vbroadcasti32x2	-1032(%rdx), %zmm30	 # AVX512DQ
	vpextrd	$0xab, %xmm29, %eax	 # AVX512DQ
	vpextrd	$123, %xmm29, %eax	 # AVX512DQ
	vpextrd	$123, %xmm29, %ebp	 # AVX512DQ
	vpextrd	$123, %xmm29, %r13d	 # AVX512DQ
	vpextrd	$123, %xmm29, (%rcx)	 # AVX512DQ
	vpextrd	$123, %xmm29, 0x123(%rax,%r14,8)	 # AVX512DQ
	vpextrd	$123, %xmm29, 508(%rdx)	 # AVX512DQ Disp8
	vpextrd	$123, %xmm29, 512(%rdx)	 # AVX512DQ
	vpextrd	$123, %xmm29, -512(%rdx)	 # AVX512DQ Disp8
	vpextrd	$123, %xmm29, -516(%rdx)	 # AVX512DQ
	vpextrq	$0xab, %xmm29, %rax	 # AVX512DQ
	vpextrq	$123, %xmm29, %rax	 # AVX512DQ
	vpextrq	$123, %xmm29, %r8	 # AVX512DQ
	vpextrq	$123, %xmm29, (%rcx)	 # AVX512DQ
	vpextrq	$123, %xmm29, 0x123(%rax,%r14,8)	 # AVX512DQ
	vpextrq	$123, %xmm29, 1016(%rdx)	 # AVX512DQ Disp8
	vpextrq	$123, %xmm29, 1024(%rdx)	 # AVX512DQ
	vpextrq	$123, %xmm29, -1024(%rdx)	 # AVX512DQ Disp8
	vpextrq	$123, %xmm29, -1032(%rdx)	 # AVX512DQ
	vpinsrd	$0xab, %eax, %xmm29, %xmm30	 # AVX512DQ
	vpinsrd	$123, %eax, %xmm29, %xmm30	 # AVX512DQ
	vpinsrd	$123, %ebp, %xmm29, %xmm30	 # AVX512DQ
	vpinsrd	$123, %r13d, %xmm29, %xmm30	 # AVX512DQ
	vpinsrd	$123, (%rcx), %xmm29, %xmm30	 # AVX512DQ
	vpinsrd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512DQ
	vpinsrd	$123, 508(%rdx), %xmm29, %xmm30	 # AVX512DQ Disp8
	vpinsrd	$123, 512(%rdx), %xmm29, %xmm30	 # AVX512DQ
	vpinsrd	$123, -512(%rdx), %xmm29, %xmm30	 # AVX512DQ Disp8
	vpinsrd	$123, -516(%rdx), %xmm29, %xmm30	 # AVX512DQ
	vpinsrq	$0xab, %rax, %xmm29, %xmm30	 # AVX512DQ
	vpinsrq	$123, %rax, %xmm29, %xmm30	 # AVX512DQ
	vpinsrq	$123, %r8, %xmm29, %xmm30	 # AVX512DQ
	vpinsrq	$123, (%rcx), %xmm29, %xmm30	 # AVX512DQ
	vpinsrq	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512DQ
	vpinsrq	$123, 1016(%rdx), %xmm29, %xmm30	 # AVX512DQ Disp8
	vpinsrq	$123, 1024(%rdx), %xmm29, %xmm30	 # AVX512DQ
	vpinsrq	$123, -1024(%rdx), %xmm29, %xmm30	 # AVX512DQ Disp8
	vpinsrq	$123, -1032(%rdx), %xmm29, %xmm30	 # AVX512DQ
	vpmullq	%zmm28, %zmm29, %zmm30	 # AVX512DQ
	vpmullq	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vpmullq	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vpmullq	(%rcx), %zmm29, %zmm30	 # AVX512DQ
	vpmullq	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512DQ
	vpmullq	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vpmullq	8128(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vpmullq	8192(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vpmullq	-8192(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vpmullq	-8256(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vpmullq	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vpmullq	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vpmullq	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vpmullq	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vrangepd	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512DQ
	vrangepd	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vrangepd	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vrangepd	$0xab, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512DQ
	vrangepd	$123, %zmm28, %zmm29, %zmm30	 # AVX512DQ
	vrangepd	$123, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512DQ
	vrangepd	$123, (%rcx), %zmm29, %zmm30	 # AVX512DQ
	vrangepd	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512DQ
	vrangepd	$123, (%rcx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vrangepd	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vrangepd	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vrangepd	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vrangepd	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vrangepd	$123, 1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vrangepd	$123, 1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vrangepd	$123, -1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vrangepd	$123, -1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vrangeps	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512DQ
	vrangeps	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vrangeps	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vrangeps	$0xab, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512DQ
	vrangeps	$123, %zmm28, %zmm29, %zmm30	 # AVX512DQ
	vrangeps	$123, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512DQ
	vrangeps	$123, (%rcx), %zmm29, %zmm30	 # AVX512DQ
	vrangeps	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512DQ
	vrangeps	$123, (%rcx){1to16}, %zmm29, %zmm30	 # AVX512DQ
	vrangeps	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vrangeps	$123, 8192(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vrangeps	$123, -8192(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vrangeps	$123, -8256(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vrangeps	$123, 508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vrangeps	$123, 512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ
	vrangeps	$123, -512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vrangeps	$123, -516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ
	vrangesd	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vrangesd	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512DQ
	vrangesd	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512DQ
	vrangesd	$0xab, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vrangesd	$123, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vrangesd	$123, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vrangesd	$123, (%rcx), %xmm29, %xmm30	 # AVX512DQ
	vrangesd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512DQ
	vrangesd	$123, 1016(%rdx), %xmm29, %xmm30	 # AVX512DQ Disp8
	vrangesd	$123, 1024(%rdx), %xmm29, %xmm30	 # AVX512DQ
	vrangesd	$123, -1024(%rdx), %xmm29, %xmm30	 # AVX512DQ Disp8
	vrangesd	$123, -1032(%rdx), %xmm29, %xmm30	 # AVX512DQ
	vrangess	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vrangess	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512DQ
	vrangess	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512DQ
	vrangess	$0xab, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vrangess	$123, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vrangess	$123, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vrangess	$123, (%rcx), %xmm29, %xmm30	 # AVX512DQ
	vrangess	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512DQ
	vrangess	$123, 508(%rdx), %xmm29, %xmm30	 # AVX512DQ Disp8
	vrangess	$123, 512(%rdx), %xmm29, %xmm30	 # AVX512DQ
	vrangess	$123, -512(%rdx), %xmm29, %xmm30	 # AVX512DQ Disp8
	vrangess	$123, -516(%rdx), %xmm29, %xmm30	 # AVX512DQ
	vandpd	%zmm28, %zmm29, %zmm30	 # AVX512DQ
	vandpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vandpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vandpd	(%rcx), %zmm29, %zmm30	 # AVX512DQ
	vandpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512DQ
	vandpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vandpd	8128(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vandpd	8192(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vandpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vandpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vandpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vandpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vandpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vandpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vandps	%zmm28, %zmm29, %zmm30	 # AVX512DQ
	vandps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vandps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vandps	(%rcx), %zmm29, %zmm30	 # AVX512DQ
	vandps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512DQ
	vandps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512DQ
	vandps	8128(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vandps	8192(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vandps	-8192(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vandps	-8256(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vandps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vandps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ
	vandps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vandps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ
	vandnpd	%zmm28, %zmm29, %zmm30	 # AVX512DQ
	vandnpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vandnpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vandnpd	(%rcx), %zmm29, %zmm30	 # AVX512DQ
	vandnpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512DQ
	vandnpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vandnpd	8128(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vandnpd	8192(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vandnpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vandnpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vandnpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vandnpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vandnpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vandnpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vandnps	%zmm28, %zmm29, %zmm30	 # AVX512DQ
	vandnps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vandnps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vandnps	(%rcx), %zmm29, %zmm30	 # AVX512DQ
	vandnps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512DQ
	vandnps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512DQ
	vandnps	8128(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vandnps	8192(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vandnps	-8192(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vandnps	-8256(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vandnps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vandnps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ
	vandnps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vandnps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ
	vorpd	%zmm28, %zmm29, %zmm30	 # AVX512DQ
	vorpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vorpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vorpd	(%rcx), %zmm29, %zmm30	 # AVX512DQ
	vorpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512DQ
	vorpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vorpd	8128(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vorpd	8192(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vorpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vorpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vorpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vorpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vorpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vorpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vorps	%zmm28, %zmm29, %zmm30	 # AVX512DQ
	vorps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vorps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vorps	(%rcx), %zmm29, %zmm30	 # AVX512DQ
	vorps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512DQ
	vorps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512DQ
	vorps	8128(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vorps	8192(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vorps	-8192(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vorps	-8256(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vorps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vorps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ
	vorps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vorps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ
	vxorpd	%zmm28, %zmm29, %zmm30	 # AVX512DQ
	vxorpd	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vxorpd	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vxorpd	(%rcx), %zmm29, %zmm30	 # AVX512DQ
	vxorpd	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512DQ
	vxorpd	(%rcx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vxorpd	8128(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vxorpd	8192(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vxorpd	-8192(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vxorpd	-8256(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vxorpd	1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vxorpd	1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vxorpd	-1024(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vxorpd	-1032(%rdx){1to8}, %zmm29, %zmm30	 # AVX512DQ
	vxorps	%zmm28, %zmm29, %zmm30	 # AVX512DQ
	vxorps	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vxorps	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vxorps	(%rcx), %zmm29, %zmm30	 # AVX512DQ
	vxorps	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512DQ
	vxorps	(%rcx){1to16}, %zmm29, %zmm30	 # AVX512DQ
	vxorps	8128(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vxorps	8192(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vxorps	-8192(%rdx), %zmm29, %zmm30	 # AVX512DQ Disp8
	vxorps	-8256(%rdx), %zmm29, %zmm30	 # AVX512DQ
	vxorps	508(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vxorps	512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ
	vxorps	-512(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ Disp8
	vxorps	-516(%rdx){1to16}, %zmm29, %zmm30	 # AVX512DQ
	vreducepd	$0xab, %zmm29, %zmm30	 # AVX512DQ
	vreducepd	$0xab, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vreducepd	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vreducepd	$0xab, {sae}, %zmm29, %zmm30	 # AVX512DQ
	vreducepd	$123, %zmm29, %zmm30	 # AVX512DQ
	vreducepd	$123, {sae}, %zmm29, %zmm30	 # AVX512DQ
	vreducepd	$123, (%rcx), %zmm30	 # AVX512DQ
	vreducepd	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vreducepd	$123, (%rcx){1to8}, %zmm30	 # AVX512DQ
	vreducepd	$123, 8128(%rdx), %zmm30	 # AVX512DQ Disp8
	vreducepd	$123, 8192(%rdx), %zmm30	 # AVX512DQ
	vreducepd	$123, -8192(%rdx), %zmm30	 # AVX512DQ Disp8
	vreducepd	$123, -8256(%rdx), %zmm30	 # AVX512DQ
	vreducepd	$123, 1016(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vreducepd	$123, 1024(%rdx){1to8}, %zmm30	 # AVX512DQ
	vreducepd	$123, -1024(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vreducepd	$123, -1032(%rdx){1to8}, %zmm30	 # AVX512DQ
	vreduceps	$0xab, %zmm29, %zmm30	 # AVX512DQ
	vreduceps	$0xab, %zmm29, %zmm30{%k7}	 # AVX512DQ
	vreduceps	$0xab, %zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vreduceps	$0xab, {sae}, %zmm29, %zmm30	 # AVX512DQ
	vreduceps	$123, %zmm29, %zmm30	 # AVX512DQ
	vreduceps	$123, {sae}, %zmm29, %zmm30	 # AVX512DQ
	vreduceps	$123, (%rcx), %zmm30	 # AVX512DQ
	vreduceps	$123, 0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vreduceps	$123, (%rcx){1to16}, %zmm30	 # AVX512DQ
	vreduceps	$123, 8128(%rdx), %zmm30	 # AVX512DQ Disp8
	vreduceps	$123, 8192(%rdx), %zmm30	 # AVX512DQ
	vreduceps	$123, -8192(%rdx), %zmm30	 # AVX512DQ Disp8
	vreduceps	$123, -8256(%rdx), %zmm30	 # AVX512DQ
	vreduceps	$123, 508(%rdx){1to16}, %zmm30	 # AVX512DQ Disp8
	vreduceps	$123, 512(%rdx){1to16}, %zmm30	 # AVX512DQ
	vreduceps	$123, -512(%rdx){1to16}, %zmm30	 # AVX512DQ Disp8
	vreduceps	$123, -516(%rdx){1to16}, %zmm30	 # AVX512DQ
	vreducesd	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vreducesd	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512DQ
	vreducesd	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512DQ
	vreducesd	$0xab, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vreducesd	$123, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vreducesd	$123, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vreducesd	$123, (%rcx), %xmm29, %xmm30	 # AVX512DQ
	vreducesd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512DQ
	vreducesd	$123, 1016(%rdx), %xmm29, %xmm30	 # AVX512DQ Disp8
	vreducesd	$123, 1024(%rdx), %xmm29, %xmm30	 # AVX512DQ
	vreducesd	$123, -1024(%rdx), %xmm29, %xmm30	 # AVX512DQ Disp8
	vreducesd	$123, -1032(%rdx), %xmm29, %xmm30	 # AVX512DQ
	vreducess	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vreducess	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512DQ
	vreducess	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512DQ
	vreducess	$0xab, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vreducess	$123, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vreducess	$123, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512DQ
	vreducess	$123, (%rcx), %xmm29, %xmm30	 # AVX512DQ
	vreducess	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512DQ
	vreducess	$123, 508(%rdx), %xmm29, %xmm30	 # AVX512DQ Disp8
	vreducess	$123, 512(%rdx), %xmm29, %xmm30	 # AVX512DQ
	vreducess	$123, -512(%rdx), %xmm29, %xmm30	 # AVX512DQ Disp8
	vreducess	$123, -516(%rdx), %xmm29, %xmm30	 # AVX512DQ
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
	kmovb	(%rcx), %k5	 # AVX512DQ
	kmovb	0x123(%rax,%r14,8), %k5	 # AVX512DQ
	kmovb	%k5, (%rcx)	 # AVX512DQ
	kmovb	%k5, 0x123(%rax,%r14,8)	 # AVX512DQ
	kmovb	%eax, %k5	 # AVX512DQ
	kmovb	%ebp, %k5	 # AVX512DQ
	kmovb	%r13d, %k5	 # AVX512DQ
	kmovb	%k5, %eax	 # AVX512DQ
	kmovb	%k5, %ebp	 # AVX512DQ
	kmovb	%k5, %r13d	 # AVX512DQ
	kaddw	%k7, %k6, %k5	 # AVX512DQ
	kaddb	%k7, %k6, %k5	 # AVX512DQ
	vextractf64x2	$0xab, %zmm30, (%rcx)	 # AVX512DQ
	vextractf64x2	$0xab, %zmm30, (%rcx){%k7}	 # AVX512DQ
	vextractf64x2	$123, %zmm30, (%rcx)	 # AVX512DQ
	vextractf64x2	$123, %zmm30, 0x123(%rax,%r14,8)	 # AVX512DQ
	vextractf64x2	$123, %zmm30, 2032(%rdx)	 # AVX512DQ Disp8
	vextractf64x2	$123, %zmm30, 2048(%rdx)	 # AVX512DQ
	vextractf64x2	$123, %zmm30, -2048(%rdx)	 # AVX512DQ Disp8
	vextractf64x2	$123, %zmm30, -2064(%rdx)	 # AVX512DQ
	vextractf32x8	$0xab, %zmm30, (%rcx)	 # AVX512DQ
	vextractf32x8	$0xab, %zmm30, (%rcx){%k7}	 # AVX512DQ
	vextractf32x8	$123, %zmm30, (%rcx)	 # AVX512DQ
	vextractf32x8	$123, %zmm30, 0x123(%rax,%r14,8)	 # AVX512DQ
	vextractf32x8	$123, %zmm30, 4064(%rdx)	 # AVX512DQ Disp8
	vextractf32x8	$123, %zmm30, 4096(%rdx)	 # AVX512DQ
	vextractf32x8	$123, %zmm30, -4096(%rdx)	 # AVX512DQ Disp8
	vextractf32x8	$123, %zmm30, -4128(%rdx)	 # AVX512DQ
	vextracti64x2	$0xab, %zmm30, (%rcx)	 # AVX512DQ
	vextracti64x2	$0xab, %zmm30, (%rcx){%k7}	 # AVX512DQ
	vextracti64x2	$123, %zmm30, (%rcx)	 # AVX512DQ
	vextracti64x2	$123, %zmm30, 0x123(%rax,%r14,8)	 # AVX512DQ
	vextracti64x2	$123, %zmm30, 2032(%rdx)	 # AVX512DQ Disp8
	vextracti64x2	$123, %zmm30, 2048(%rdx)	 # AVX512DQ
	vextracti64x2	$123, %zmm30, -2048(%rdx)	 # AVX512DQ Disp8
	vextracti64x2	$123, %zmm30, -2064(%rdx)	 # AVX512DQ
	vextracti32x8	$0xab, %zmm30, (%rcx)	 # AVX512DQ
	vextracti32x8	$0xab, %zmm30, (%rcx){%k7}	 # AVX512DQ
	vextracti32x8	$123, %zmm30, (%rcx)	 # AVX512DQ
	vextracti32x8	$123, %zmm30, 0x123(%rax,%r14,8)	 # AVX512DQ
	vextracti32x8	$123, %zmm30, 4064(%rdx)	 # AVX512DQ Disp8
	vextracti32x8	$123, %zmm30, 4096(%rdx)	 # AVX512DQ
	vextracti32x8	$123, %zmm30, -4096(%rdx)	 # AVX512DQ Disp8
	vextracti32x8	$123, %zmm30, -4128(%rdx)	 # AVX512DQ
	vcvttpd2qq	%zmm29, %zmm30	 # AVX512DQ
	vcvttpd2qq	%zmm29, %zmm30{%k7}	 # AVX512DQ
	vcvttpd2qq	%zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vcvttpd2qq	{sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvttpd2qq	(%rcx), %zmm30	 # AVX512DQ
	vcvttpd2qq	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vcvttpd2qq	(%rcx){1to8}, %zmm30	 # AVX512DQ
	vcvttpd2qq	8128(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvttpd2qq	8192(%rdx), %zmm30	 # AVX512DQ
	vcvttpd2qq	-8192(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvttpd2qq	-8256(%rdx), %zmm30	 # AVX512DQ
	vcvttpd2qq	1016(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvttpd2qq	1024(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvttpd2qq	-1024(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvttpd2qq	-1032(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvttpd2uqq	%zmm29, %zmm30	 # AVX512DQ
	vcvttpd2uqq	%zmm29, %zmm30{%k7}	 # AVX512DQ
	vcvttpd2uqq	%zmm29, %zmm30{%k7}{z}	 # AVX512DQ
	vcvttpd2uqq	{sae}, %zmm29, %zmm30	 # AVX512DQ
	vcvttpd2uqq	(%rcx), %zmm30	 # AVX512DQ
	vcvttpd2uqq	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vcvttpd2uqq	(%rcx){1to8}, %zmm30	 # AVX512DQ
	vcvttpd2uqq	8128(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvttpd2uqq	8192(%rdx), %zmm30	 # AVX512DQ
	vcvttpd2uqq	-8192(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvttpd2uqq	-8256(%rdx), %zmm30	 # AVX512DQ
	vcvttpd2uqq	1016(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvttpd2uqq	1024(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvttpd2uqq	-1024(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvttpd2uqq	-1032(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvttps2qq	%ymm29, %zmm30	 # AVX512DQ
	vcvttps2qq	%ymm29, %zmm30{%k7}	 # AVX512DQ
	vcvttps2qq	%ymm29, %zmm30{%k7}{z}	 # AVX512DQ
	vcvttps2qq	{sae}, %ymm29, %zmm30	 # AVX512DQ
	vcvttps2qq	(%rcx), %zmm30	 # AVX512DQ
	vcvttps2qq	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vcvttps2qq	(%rcx){1to8}, %zmm30	 # AVX512DQ
	vcvttps2qq	4064(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvttps2qq	4096(%rdx), %zmm30	 # AVX512DQ
	vcvttps2qq	-4096(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvttps2qq	-4128(%rdx), %zmm30	 # AVX512DQ
	vcvttps2qq	508(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvttps2qq	512(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvttps2qq	-512(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvttps2qq	-516(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvttps2uqq	%ymm29, %zmm30	 # AVX512DQ
	vcvttps2uqq	%ymm29, %zmm30{%k7}	 # AVX512DQ
	vcvttps2uqq	%ymm29, %zmm30{%k7}{z}	 # AVX512DQ
	vcvttps2uqq	{sae}, %ymm29, %zmm30	 # AVX512DQ
	vcvttps2uqq	(%rcx), %zmm30	 # AVX512DQ
	vcvttps2uqq	0x123(%rax,%r14,8), %zmm30	 # AVX512DQ
	vcvttps2uqq	(%rcx){1to8}, %zmm30	 # AVX512DQ
	vcvttps2uqq	4064(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvttps2uqq	4096(%rdx), %zmm30	 # AVX512DQ
	vcvttps2uqq	-4096(%rdx), %zmm30	 # AVX512DQ Disp8
	vcvttps2uqq	-4128(%rdx), %zmm30	 # AVX512DQ
	vcvttps2uqq	508(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvttps2uqq	512(%rdx){1to8}, %zmm30	 # AVX512DQ
	vcvttps2uqq	-512(%rdx){1to8}, %zmm30	 # AVX512DQ Disp8
	vcvttps2uqq	-516(%rdx){1to8}, %zmm30	 # AVX512DQ
	vpmovd2m	%zmm30, %k5	 # AVX512DQ
	vpmovq2m	%zmm30, %k5	 # AVX512DQ
	vpmovm2d	%k5, %zmm30	 # AVX512DQ
	vpmovm2q	%k5, %zmm30	 # AVX512DQ

	.intel_syntax noprefix
	vbroadcastf32x8	zmm30, YMMWORD PTR [rcx]	 # AVX512DQ
	vbroadcastf32x8	zmm30{k7}, YMMWORD PTR [rcx]	 # AVX512DQ
	vbroadcastf32x8	zmm30{k7}{z}, YMMWORD PTR [rcx]	 # AVX512DQ
	vbroadcastf32x8	zmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vbroadcastf32x8	zmm30, YMMWORD PTR [rdx+4064]	 # AVX512DQ Disp8
	vbroadcastf32x8	zmm30, YMMWORD PTR [rdx+4096]	 # AVX512DQ
	vbroadcastf32x8	zmm30, YMMWORD PTR [rdx-4096]	 # AVX512DQ Disp8
	vbroadcastf32x8	zmm30, YMMWORD PTR [rdx-4128]	 # AVX512DQ
	vbroadcastf64x2	zmm30, XMMWORD PTR [rcx]	 # AVX512DQ
	vbroadcastf64x2	zmm30{k7}, XMMWORD PTR [rcx]	 # AVX512DQ
	vbroadcastf64x2	zmm30{k7}{z}, XMMWORD PTR [rcx]	 # AVX512DQ
	vbroadcastf64x2	zmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vbroadcastf64x2	zmm30, XMMWORD PTR [rdx+2032]	 # AVX512DQ Disp8
	vbroadcastf64x2	zmm30, XMMWORD PTR [rdx+2048]	 # AVX512DQ
	vbroadcastf64x2	zmm30, XMMWORD PTR [rdx-2048]	 # AVX512DQ Disp8
	vbroadcastf64x2	zmm30, XMMWORD PTR [rdx-2064]	 # AVX512DQ
	vbroadcasti32x8	zmm30, YMMWORD PTR [rcx]	 # AVX512DQ
	vbroadcasti32x8	zmm30{k7}, YMMWORD PTR [rcx]	 # AVX512DQ
	vbroadcasti32x8	zmm30{k7}{z}, YMMWORD PTR [rcx]	 # AVX512DQ
	vbroadcasti32x8	zmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vbroadcasti32x8	zmm30, YMMWORD PTR [rdx+4064]	 # AVX512DQ Disp8
	vbroadcasti32x8	zmm30, YMMWORD PTR [rdx+4096]	 # AVX512DQ
	vbroadcasti32x8	zmm30, YMMWORD PTR [rdx-4096]	 # AVX512DQ Disp8
	vbroadcasti32x8	zmm30, YMMWORD PTR [rdx-4128]	 # AVX512DQ
	vbroadcasti64x2	zmm30, XMMWORD PTR [rcx]	 # AVX512DQ
	vbroadcasti64x2	zmm30{k7}, XMMWORD PTR [rcx]	 # AVX512DQ
	vbroadcasti64x2	zmm30{k7}{z}, XMMWORD PTR [rcx]	 # AVX512DQ
	vbroadcasti64x2	zmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vbroadcasti64x2	zmm30, XMMWORD PTR [rdx+2032]	 # AVX512DQ Disp8
	vbroadcasti64x2	zmm30, XMMWORD PTR [rdx+2048]	 # AVX512DQ
	vbroadcasti64x2	zmm30, XMMWORD PTR [rdx-2048]	 # AVX512DQ Disp8
	vbroadcasti64x2	zmm30, XMMWORD PTR [rdx-2064]	 # AVX512DQ
	vbroadcastf32x2	zmm30, xmm31	 # AVX512DQ
	vbroadcastf32x2	zmm30{k7}, xmm31	 # AVX512DQ
	vbroadcastf32x2	zmm30{k7}{z}, xmm31	 # AVX512DQ
	vbroadcastf32x2	zmm30, QWORD PTR [rcx]	 # AVX512DQ
	vbroadcastf32x2	zmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vbroadcastf32x2	zmm30, QWORD PTR [rdx+1016]	 # AVX512DQ Disp8
	vbroadcastf32x2	zmm30, QWORD PTR [rdx+1024]	 # AVX512DQ
	vbroadcastf32x2	zmm30, QWORD PTR [rdx-1024]	 # AVX512DQ Disp8
	vbroadcastf32x2	zmm30, QWORD PTR [rdx-1032]	 # AVX512DQ
	vcvtpd2qq	zmm30, zmm29	 # AVX512DQ
	vcvtpd2qq	zmm30{k7}, zmm29	 # AVX512DQ
	vcvtpd2qq	zmm30{k7}{z}, zmm29	 # AVX512DQ
	vcvtpd2qq	zmm30, zmm29{rn-sae}	 # AVX512DQ
	vcvtpd2qq	zmm30, zmm29{ru-sae}	 # AVX512DQ
	vcvtpd2qq	zmm30, zmm29{rd-sae}	 # AVX512DQ
	vcvtpd2qq	zmm30, zmm29{rz-sae}	 # AVX512DQ
	vcvtpd2qq	zmm30, ZMMWORD PTR [rcx]	 # AVX512DQ
	vcvtpd2qq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vcvtpd2qq	zmm30, qword bcst [rcx]	 # AVX512DQ
	vcvtpd2qq	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vcvtpd2qq	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vcvtpd2qq	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vcvtpd2qq	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vcvtpd2qq	zmm30, qword bcst [rdx+1016]	 # AVX512DQ Disp8
	vcvtpd2qq	zmm30, qword bcst [rdx+1024]	 # AVX512DQ
	vcvtpd2qq	zmm30, qword bcst [rdx-1024]	 # AVX512DQ Disp8
	vcvtpd2qq	zmm30, qword bcst [rdx-1032]	 # AVX512DQ
	vcvtpd2uqq	zmm30, zmm29	 # AVX512DQ
	vcvtpd2uqq	zmm30{k7}, zmm29	 # AVX512DQ
	vcvtpd2uqq	zmm30{k7}{z}, zmm29	 # AVX512DQ
	vcvtpd2uqq	zmm30, zmm29{rn-sae}	 # AVX512DQ
	vcvtpd2uqq	zmm30, zmm29{ru-sae}	 # AVX512DQ
	vcvtpd2uqq	zmm30, zmm29{rd-sae}	 # AVX512DQ
	vcvtpd2uqq	zmm30, zmm29{rz-sae}	 # AVX512DQ
	vcvtpd2uqq	zmm30, ZMMWORD PTR [rcx]	 # AVX512DQ
	vcvtpd2uqq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vcvtpd2uqq	zmm30, qword bcst [rcx]	 # AVX512DQ
	vcvtpd2uqq	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vcvtpd2uqq	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vcvtpd2uqq	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vcvtpd2uqq	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vcvtpd2uqq	zmm30, qword bcst [rdx+1016]	 # AVX512DQ Disp8
	vcvtpd2uqq	zmm30, qword bcst [rdx+1024]	 # AVX512DQ
	vcvtpd2uqq	zmm30, qword bcst [rdx-1024]	 # AVX512DQ Disp8
	vcvtpd2uqq	zmm30, qword bcst [rdx-1032]	 # AVX512DQ
	vcvtps2qq	zmm30, ymm29	 # AVX512DQ
	vcvtps2qq	zmm30{k7}, ymm29	 # AVX512DQ
	vcvtps2qq	zmm30{k7}{z}, ymm29	 # AVX512DQ
	vcvtps2qq	zmm30, ymm29{rn-sae}	 # AVX512DQ
	vcvtps2qq	zmm30, ymm29{ru-sae}	 # AVX512DQ
	vcvtps2qq	zmm30, ymm29{rd-sae}	 # AVX512DQ
	vcvtps2qq	zmm30, ymm29{rz-sae}	 # AVX512DQ
	vcvtps2qq	zmm30, YMMWORD PTR [rcx]	 # AVX512DQ
	vcvtps2qq	zmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vcvtps2qq	zmm30, dword bcst [rcx]	 # AVX512DQ
	vcvtps2qq	zmm30, YMMWORD PTR [rdx+4064]	 # AVX512DQ Disp8
	vcvtps2qq	zmm30, YMMWORD PTR [rdx+4096]	 # AVX512DQ
	vcvtps2qq	zmm30, YMMWORD PTR [rdx-4096]	 # AVX512DQ Disp8
	vcvtps2qq	zmm30, YMMWORD PTR [rdx-4128]	 # AVX512DQ
	vcvtps2qq	zmm30, dword bcst [rdx+508]	 # AVX512DQ Disp8
	vcvtps2qq	zmm30, dword bcst [rdx+512]	 # AVX512DQ
	vcvtps2qq	zmm30, dword bcst [rdx-512]	 # AVX512DQ Disp8
	vcvtps2qq	zmm30, dword bcst [rdx-516]	 # AVX512DQ
	vcvtps2qq	zmm30, DWORD BCST [rdx+508]	 # AVX512DQ Disp8
	vcvtps2uqq	zmm30, ymm29	 # AVX512DQ
	vcvtps2uqq	zmm30{k7}, ymm29	 # AVX512DQ
	vcvtps2uqq	zmm30{k7}{z}, ymm29	 # AVX512DQ
	vcvtps2uqq	zmm30, ymm29{rn-sae}	 # AVX512DQ
	vcvtps2uqq	zmm30, ymm29{ru-sae}	 # AVX512DQ
	vcvtps2uqq	zmm30, ymm29{rd-sae}	 # AVX512DQ
	vcvtps2uqq	zmm30, ymm29{rz-sae}	 # AVX512DQ
	vcvtps2uqq	zmm30, YMMWORD PTR [rcx]	 # AVX512DQ
	vcvtps2uqq	zmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vcvtps2uqq	zmm30, dword bcst [rcx]	 # AVX512DQ
	vcvtps2uqq	zmm30, YMMWORD PTR [rdx+4064]	 # AVX512DQ Disp8
	vcvtps2uqq	zmm30, YMMWORD PTR [rdx+4096]	 # AVX512DQ
	vcvtps2uqq	zmm30, YMMWORD PTR [rdx-4096]	 # AVX512DQ Disp8
	vcvtps2uqq	zmm30, YMMWORD PTR [rdx-4128]	 # AVX512DQ
	vcvtps2uqq	zmm30, dword bcst [rdx+508]	 # AVX512DQ Disp8
	vcvtps2uqq	zmm30, dword bcst [rdx+512]	 # AVX512DQ
	vcvtps2uqq	zmm30, dword bcst [rdx-512]	 # AVX512DQ Disp8
	vcvtps2uqq	zmm30, dword bcst [rdx-516]	 # AVX512DQ
	vcvtps2uqq	zmm30, DWORD BCST [rdx+508]	 # AVX512DQ Disp8
	vcvtqq2pd	zmm30, zmm29	 # AVX512DQ
	vcvtqq2pd	zmm30{k7}, zmm29	 # AVX512DQ
	vcvtqq2pd	zmm30{k7}{z}, zmm29	 # AVX512DQ
	vcvtqq2pd	zmm30, zmm29{rn-sae}	 # AVX512DQ
	vcvtqq2pd	zmm30, zmm29{ru-sae}	 # AVX512DQ
	vcvtqq2pd	zmm30, zmm29{rd-sae}	 # AVX512DQ
	vcvtqq2pd	zmm30, zmm29{rz-sae}	 # AVX512DQ
	vcvtqq2pd	zmm30, ZMMWORD PTR [rcx]	 # AVX512DQ
	vcvtqq2pd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vcvtqq2pd	zmm30, qword bcst [rcx]	 # AVX512DQ
	vcvtqq2pd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vcvtqq2pd	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vcvtqq2pd	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vcvtqq2pd	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vcvtqq2pd	zmm30, qword bcst [rdx+1016]	 # AVX512DQ Disp8
	vcvtqq2pd	zmm30, qword bcst [rdx+1024]	 # AVX512DQ
	vcvtqq2pd	zmm30, qword bcst [rdx-1024]	 # AVX512DQ Disp8
	vcvtqq2pd	zmm30, qword bcst [rdx-1032]	 # AVX512DQ
	vcvtqq2ps	ymm30, zmm29	 # AVX512DQ
	vcvtqq2ps	ymm30{k7}, zmm29	 # AVX512DQ
	vcvtqq2ps	ymm30{k7}{z}, zmm29	 # AVX512DQ
	vcvtqq2ps	ymm30, zmm29{rn-sae}	 # AVX512DQ
	vcvtqq2ps	ymm30, zmm29{ru-sae}	 # AVX512DQ
	vcvtqq2ps	ymm30, zmm29{rd-sae}	 # AVX512DQ
	vcvtqq2ps	ymm30, zmm29{rz-sae}	 # AVX512DQ
	vcvtqq2ps	ymm30, ZMMWORD PTR [rcx]	 # AVX512DQ
	vcvtqq2ps	ymm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vcvtqq2ps	ymm30, qword bcst [rcx]	 # AVX512DQ
	vcvtqq2ps	ymm30, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vcvtqq2ps	ymm30, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vcvtqq2ps	ymm30, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vcvtqq2ps	ymm30, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vcvtqq2ps	ymm30, qword bcst [rdx+1016]	 # AVX512DQ Disp8
	vcvtqq2ps	ymm30, qword bcst [rdx+1024]	 # AVX512DQ
	vcvtqq2ps	ymm30, qword bcst [rdx-1024]	 # AVX512DQ Disp8
	vcvtqq2ps	ymm30, qword bcst [rdx-1032]	 # AVX512DQ
	vcvtuqq2pd	zmm30, zmm29	 # AVX512DQ
	vcvtuqq2pd	zmm30{k7}, zmm29	 # AVX512DQ
	vcvtuqq2pd	zmm30{k7}{z}, zmm29	 # AVX512DQ
	vcvtuqq2pd	zmm30, zmm29{rn-sae}	 # AVX512DQ
	vcvtuqq2pd	zmm30, zmm29{ru-sae}	 # AVX512DQ
	vcvtuqq2pd	zmm30, zmm29{rd-sae}	 # AVX512DQ
	vcvtuqq2pd	zmm30, zmm29{rz-sae}	 # AVX512DQ
	vcvtuqq2pd	zmm30, ZMMWORD PTR [rcx]	 # AVX512DQ
	vcvtuqq2pd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vcvtuqq2pd	zmm30, qword bcst [rcx]	 # AVX512DQ
	vcvtuqq2pd	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vcvtuqq2pd	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vcvtuqq2pd	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vcvtuqq2pd	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vcvtuqq2pd	zmm30, qword bcst [rdx+1016]	 # AVX512DQ Disp8
	vcvtuqq2pd	zmm30, qword bcst [rdx+1024]	 # AVX512DQ
	vcvtuqq2pd	zmm30, qword bcst [rdx-1024]	 # AVX512DQ Disp8
	vcvtuqq2pd	zmm30, qword bcst [rdx-1032]	 # AVX512DQ
	vcvtuqq2ps	ymm30, zmm29	 # AVX512DQ
	vcvtuqq2ps	ymm30{k7}, zmm29	 # AVX512DQ
	vcvtuqq2ps	ymm30{k7}{z}, zmm29	 # AVX512DQ
	vcvtuqq2ps	ymm30, zmm29{rn-sae}	 # AVX512DQ
	vcvtuqq2ps	ymm30, zmm29{ru-sae}	 # AVX512DQ
	vcvtuqq2ps	ymm30, zmm29{rd-sae}	 # AVX512DQ
	vcvtuqq2ps	ymm30, zmm29{rz-sae}	 # AVX512DQ
	vcvtuqq2ps	ymm30, ZMMWORD PTR [rcx]	 # AVX512DQ
	vcvtuqq2ps	ymm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vcvtuqq2ps	ymm30, qword bcst [rcx]	 # AVX512DQ
	vcvtuqq2ps	ymm30, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vcvtuqq2ps	ymm30, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vcvtuqq2ps	ymm30, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vcvtuqq2ps	ymm30, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vcvtuqq2ps	ymm30, qword bcst [rdx+1016]	 # AVX512DQ Disp8
	vcvtuqq2ps	ymm30, qword bcst [rdx+1024]	 # AVX512DQ
	vcvtuqq2ps	ymm30, qword bcst [rdx-1024]	 # AVX512DQ Disp8
	vcvtuqq2ps	ymm30, qword bcst [rdx-1032]	 # AVX512DQ
	vextractf64x2	xmm30, zmm29, 0xab	 # AVX512DQ
	vextractf64x2	xmm30{k7}, zmm29, 0xab	 # AVX512DQ
	vextractf64x2	xmm30{k7}{z}, zmm29, 0xab	 # AVX512DQ
	vextractf64x2	xmm30, zmm29, 123	 # AVX512DQ
	vextractf32x8	ymm30, zmm29, 0xab	 # AVX512DQ
	vextractf32x8	ymm30{k7}, zmm29, 0xab	 # AVX512DQ
	vextractf32x8	ymm30{k7}{z}, zmm29, 0xab	 # AVX512DQ
	vextractf32x8	ymm30, zmm29, 123	 # AVX512DQ
	vextracti64x2	xmm30, zmm29, 0xab	 # AVX512DQ
	vextracti64x2	xmm30{k7}, zmm29, 0xab	 # AVX512DQ
	vextracti64x2	xmm30{k7}{z}, zmm29, 0xab	 # AVX512DQ
	vextracti64x2	xmm30, zmm29, 123	 # AVX512DQ
	vextracti32x8	ymm30, zmm29, 0xab	 # AVX512DQ
	vextracti32x8	ymm30{k7}, zmm29, 0xab	 # AVX512DQ
	vextracti32x8	ymm30{k7}{z}, zmm29, 0xab	 # AVX512DQ
	vextracti32x8	ymm30, zmm29, 123	 # AVX512DQ
	vfpclasspd	k5, zmm30, 0xab	 # AVX512DQ
	vfpclasspd	k5{k7}, zmm30, 0xab	 # AVX512DQ
	vfpclasspd	k5, zmm30, 123	 # AVX512DQ
	vfpclasspd	k5, ZMMWORD PTR [rcx], 123	 # AVX512DQ
	vfpclasspd	k5, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vfpclasspd	k5, [rcx]{1to8}, 123	 # AVX512DQ
	vfpclasspd	k5, ZMMWORD PTR [rdx+8128], 123	 # AVX512DQ Disp8
	vfpclasspd	k5, ZMMWORD PTR [rdx+8192], 123	 # AVX512DQ
	vfpclasspd	k5, ZMMWORD PTR [rdx-8192], 123	 # AVX512DQ Disp8
	vfpclasspd	k5, ZMMWORD PTR [rdx-8256], 123	 # AVX512DQ
	vfpclasspd	k5, QWORD BCST [rdx+1016]{1to8}, 123	 # AVX512DQ Disp8
	vfpclasspd	k5, QWORD BCST [rdx+1024]{1to8}, 123	 # AVX512DQ
	vfpclasspd	k5, QWORD BCST [rdx-1024]{1to8}, 123	 # AVX512DQ Disp8
	vfpclasspd	k5, QWORD BCST [rdx-1032]{1to8}, 123	 # AVX512DQ
	vfpclassps	k5, zmm30, 0xab	 # AVX512DQ
	vfpclassps	k5{k7}, zmm30, 0xab	 # AVX512DQ
	vfpclassps	k5, zmm30, 123	 # AVX512DQ
	vfpclassps	k5, ZMMWORD PTR [rcx], 123	 # AVX512DQ
	vfpclassps	k5, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vfpclassps	k5, [rcx]{1to16}, 123	 # AVX512DQ
	vfpclassps	k5, ZMMWORD PTR [rdx+8128], 123	 # AVX512DQ Disp8
	vfpclassps	k5, ZMMWORD PTR [rdx+8192], 123	 # AVX512DQ
	vfpclassps	k5, ZMMWORD PTR [rdx-8192], 123	 # AVX512DQ Disp8
	vfpclassps	k5, ZMMWORD PTR [rdx-8256], 123	 # AVX512DQ
	vfpclassps	k5, DWORD BCST [rdx+508]{1to16}, 123	 # AVX512DQ Disp8
	vfpclassps	k5, DWORD BCST [rdx+512]{1to16}, 123	 # AVX512DQ
	vfpclassps	k5, DWORD BCST [rdx-512]{1to16}, 123	 # AVX512DQ Disp8
	vfpclassps	k5, DWORD BCST [rdx-516]{1to16}, 123	 # AVX512DQ
	vfpclasssd	k5, xmm30, 0xab	 # AVX512DQ
	vfpclasssd	k5{k7}, xmm30, 0xab	 # AVX512DQ
	vfpclasssd	k5, xmm30, 123	 # AVX512DQ
	vfpclasssd	k5, QWORD PTR [rcx], 123	 # AVX512DQ
	vfpclasssd	k5, QWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vfpclasssd	k5, QWORD PTR [rdx+1016], 123	 # AVX512DQ Disp8
	vfpclasssd	k5, QWORD PTR [rdx+1024], 123	 # AVX512DQ
	vfpclasssd	k5, QWORD PTR [rdx-1024], 123	 # AVX512DQ Disp8
	vfpclasssd	k5, QWORD PTR [rdx-1032], 123	 # AVX512DQ
	vfpclassss	k5, xmm30, 0xab	 # AVX512DQ
	vfpclassss	k5{k7}, xmm30, 0xab	 # AVX512DQ
	vfpclassss	k5, xmm30, 123	 # AVX512DQ
	vfpclassss	k5, DWORD PTR [rcx], 123	 # AVX512DQ
	vfpclassss	k5, DWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vfpclassss	k5, DWORD PTR [rdx+508], 123	 # AVX512DQ Disp8
	vfpclassss	k5, DWORD PTR [rdx+512], 123	 # AVX512DQ
	vfpclassss	k5, DWORD PTR [rdx-512], 123	 # AVX512DQ Disp8
	vfpclassss	k5, DWORD PTR [rdx-516], 123	 # AVX512DQ
	vinsertf64x2	zmm30, zmm29, xmm28, 0xab	 # AVX512DQ
	vinsertf64x2	zmm30{k7}, zmm29, xmm28, 0xab	 # AVX512DQ
	vinsertf64x2	zmm30{k7}{z}, zmm29, xmm28, 0xab	 # AVX512DQ
	vinsertf64x2	zmm30, zmm29, xmm28, 123	 # AVX512DQ
	vinsertf64x2	zmm30, zmm29, XMMWORD PTR [rcx], 123	 # AVX512DQ
	vinsertf64x2	zmm30, zmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vinsertf64x2	zmm30, zmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512DQ Disp8
	vinsertf64x2	zmm30, zmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512DQ
	vinsertf64x2	zmm30, zmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512DQ Disp8
	vinsertf64x2	zmm30, zmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512DQ
	vinsertf32x8	zmm30, zmm29, ymm28, 0xab	 # AVX512DQ
	vinsertf32x8	zmm30{k7}, zmm29, ymm28, 0xab	 # AVX512DQ
	vinsertf32x8	zmm30{k7}{z}, zmm29, ymm28, 0xab	 # AVX512DQ
	vinsertf32x8	zmm30, zmm29, ymm28, 123	 # AVX512DQ
	vinsertf32x8	zmm30, zmm29, YMMWORD PTR [rcx], 123	 # AVX512DQ
	vinsertf32x8	zmm30, zmm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vinsertf32x8	zmm30, zmm29, YMMWORD PTR [rdx+4064], 123	 # AVX512DQ Disp8
	vinsertf32x8	zmm30, zmm29, YMMWORD PTR [rdx+4096], 123	 # AVX512DQ
	vinsertf32x8	zmm30, zmm29, YMMWORD PTR [rdx-4096], 123	 # AVX512DQ Disp8
	vinsertf32x8	zmm30, zmm29, YMMWORD PTR [rdx-4128], 123	 # AVX512DQ
	vinserti64x2	zmm30, zmm29, xmm28, 0xab	 # AVX512DQ
	vinserti64x2	zmm30{k7}, zmm29, xmm28, 0xab	 # AVX512DQ
	vinserti64x2	zmm30{k7}{z}, zmm29, xmm28, 0xab	 # AVX512DQ
	vinserti64x2	zmm30, zmm29, xmm28, 123	 # AVX512DQ
	vinserti64x2	zmm30, zmm29, XMMWORD PTR [rcx], 123	 # AVX512DQ
	vinserti64x2	zmm30, zmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vinserti64x2	zmm30, zmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512DQ Disp8
	vinserti64x2	zmm30, zmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512DQ
	vinserti64x2	zmm30, zmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512DQ Disp8
	vinserti64x2	zmm30, zmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512DQ
	vinserti32x8	zmm30, zmm29, ymm28, 0xab	 # AVX512DQ
	vinserti32x8	zmm30{k7}, zmm29, ymm28, 0xab	 # AVX512DQ
	vinserti32x8	zmm30{k7}{z}, zmm29, ymm28, 0xab	 # AVX512DQ
	vinserti32x8	zmm30, zmm29, ymm28, 123	 # AVX512DQ
	vinserti32x8	zmm30, zmm29, YMMWORD PTR [rcx], 123	 # AVX512DQ
	vinserti32x8	zmm30, zmm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vinserti32x8	zmm30, zmm29, YMMWORD PTR [rdx+4064], 123	 # AVX512DQ Disp8
	vinserti32x8	zmm30, zmm29, YMMWORD PTR [rdx+4096], 123	 # AVX512DQ
	vinserti32x8	zmm30, zmm29, YMMWORD PTR [rdx-4096], 123	 # AVX512DQ Disp8
	vinserti32x8	zmm30, zmm29, YMMWORD PTR [rdx-4128], 123	 # AVX512DQ
	vbroadcasti32x2	zmm30, xmm31	 # AVX512DQ
	vbroadcasti32x2	zmm30{k7}, xmm31	 # AVX512DQ
	vbroadcasti32x2	zmm30{k7}{z}, xmm31	 # AVX512DQ
	vbroadcasti32x2	zmm30, QWORD PTR [rcx]	 # AVX512DQ
	vbroadcasti32x2	zmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vbroadcasti32x2	zmm30, QWORD PTR [rdx+1016]	 # AVX512DQ Disp8
	vbroadcasti32x2	zmm30, QWORD PTR [rdx+1024]	 # AVX512DQ
	vbroadcasti32x2	zmm30, QWORD PTR [rdx-1024]	 # AVX512DQ Disp8
	vbroadcasti32x2	zmm30, QWORD PTR [rdx-1032]	 # AVX512DQ
	vpextrd	eax, xmm29, 0xab	 # AVX512DQ
	vpextrd	eax, xmm29, 123	 # AVX512DQ
	vpextrd	ebp, xmm29, 123	 # AVX512DQ
	vpextrd	r13d, xmm29, 123	 # AVX512DQ
	vpextrd	DWORD PTR [rcx], xmm29, 123	 # AVX512DQ
	vpextrd	DWORD PTR [rax+r14*8+0x1234], xmm29, 123	 # AVX512DQ
	vpextrd	DWORD PTR [rdx+508], xmm29, 123	 # AVX512DQ Disp8
	vpextrd	DWORD PTR [rdx+512], xmm29, 123	 # AVX512DQ
	vpextrd	DWORD PTR [rdx-512], xmm29, 123	 # AVX512DQ Disp8
	vpextrd	DWORD PTR [rdx-516], xmm29, 123	 # AVX512DQ
	vpextrq	rax, xmm29, 0xab	 # AVX512DQ
	vpextrq	rax, xmm29, 123	 # AVX512DQ
	vpextrq	r8, xmm29, 123	 # AVX512DQ
	vpextrq	QWORD PTR [rcx], xmm29, 123	 # AVX512DQ
	vpextrq	QWORD PTR [rax+r14*8+0x1234], xmm29, 123	 # AVX512DQ
	vpextrq	QWORD PTR [rdx+1016], xmm29, 123	 # AVX512DQ Disp8
	vpextrq	QWORD PTR [rdx+1024], xmm29, 123	 # AVX512DQ
	vpextrq	QWORD PTR [rdx-1024], xmm29, 123	 # AVX512DQ Disp8
	vpextrq	QWORD PTR [rdx-1032], xmm29, 123	 # AVX512DQ
	vpinsrd	xmm30, xmm29, eax, 0xab	 # AVX512DQ
	vpinsrd	xmm30, xmm29, eax, 123	 # AVX512DQ
	vpinsrd	xmm30, xmm29, ebp, 123	 # AVX512DQ
	vpinsrd	xmm30, xmm29, r13d, 123	 # AVX512DQ
	vpinsrd	xmm30, xmm29, DWORD PTR [rcx], 123	 # AVX512DQ
	vpinsrd	xmm30, xmm29, DWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vpinsrd	xmm30, xmm29, DWORD PTR [rdx+508], 123	 # AVX512DQ Disp8
	vpinsrd	xmm30, xmm29, DWORD PTR [rdx+512], 123	 # AVX512DQ
	vpinsrd	xmm30, xmm29, DWORD PTR [rdx-512], 123	 # AVX512DQ Disp8
	vpinsrd	xmm30, xmm29, DWORD PTR [rdx-516], 123	 # AVX512DQ
	vpinsrq	xmm30, xmm29, rax, 0xab	 # AVX512DQ
	vpinsrq	xmm30, xmm29, rax, 123	 # AVX512DQ
	vpinsrq	xmm30, xmm29, r8, 123	 # AVX512DQ
	vpinsrq	xmm30, xmm29, QWORD PTR [rcx], 123	 # AVX512DQ
	vpinsrq	xmm30, xmm29, QWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vpinsrq	xmm30, xmm29, QWORD PTR [rdx+1016], 123	 # AVX512DQ Disp8
	vpinsrq	xmm30, xmm29, QWORD PTR [rdx+1024], 123	 # AVX512DQ
	vpinsrq	xmm30, xmm29, QWORD PTR [rdx-1024], 123	 # AVX512DQ Disp8
	vpinsrq	xmm30, xmm29, QWORD PTR [rdx-1032], 123	 # AVX512DQ
	vpmullq	zmm30, zmm29, zmm28	 # AVX512DQ
	vpmullq	zmm30{k7}, zmm29, zmm28	 # AVX512DQ
	vpmullq	zmm30{k7}{z}, zmm29, zmm28	 # AVX512DQ
	vpmullq	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512DQ
	vpmullq	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vpmullq	zmm30, zmm29, qword bcst [rcx]	 # AVX512DQ
	vpmullq	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vpmullq	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vpmullq	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vpmullq	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vpmullq	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512DQ Disp8
	vpmullq	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512DQ
	vpmullq	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512DQ Disp8
	vpmullq	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512DQ
	vrangepd	zmm30, zmm29, zmm28, 0xab	 # AVX512DQ
	vrangepd	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512DQ
	vrangepd	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512DQ
	vrangepd	zmm30, zmm29, zmm28{sae}, 0xab	 # AVX512DQ
	vrangepd	zmm30, zmm29, zmm28, 123	 # AVX512DQ
	vrangepd	zmm30, zmm29, zmm28{sae}, 123	 # AVX512DQ
	vrangepd	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512DQ
	vrangepd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vrangepd	zmm30, zmm29, qword bcst [rcx], 123	 # AVX512DQ
	vrangepd	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512DQ Disp8
	vrangepd	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512DQ
	vrangepd	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512DQ Disp8
	vrangepd	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512DQ
	vrangepd	zmm30, zmm29, qword bcst [rdx+1016], 123	 # AVX512DQ Disp8
	vrangepd	zmm30, zmm29, qword bcst [rdx+1024], 123	 # AVX512DQ
	vrangepd	zmm30, zmm29, qword bcst [rdx-1024], 123	 # AVX512DQ Disp8
	vrangepd	zmm30, zmm29, qword bcst [rdx-1032], 123	 # AVX512DQ
	vrangeps	zmm30, zmm29, zmm28, 0xab	 # AVX512DQ
	vrangeps	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512DQ
	vrangeps	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512DQ
	vrangeps	zmm30, zmm29, zmm28{sae}, 0xab	 # AVX512DQ
	vrangeps	zmm30, zmm29, zmm28, 123	 # AVX512DQ
	vrangeps	zmm30, zmm29, zmm28{sae}, 123	 # AVX512DQ
	vrangeps	zmm30, zmm29, ZMMWORD PTR [rcx], 123	 # AVX512DQ
	vrangeps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vrangeps	zmm30, zmm29, dword bcst [rcx], 123	 # AVX512DQ
	vrangeps	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512DQ Disp8
	vrangeps	zmm30, zmm29, ZMMWORD PTR [rdx+8192], 123	 # AVX512DQ
	vrangeps	zmm30, zmm29, ZMMWORD PTR [rdx-8192], 123	 # AVX512DQ Disp8
	vrangeps	zmm30, zmm29, ZMMWORD PTR [rdx-8256], 123	 # AVX512DQ
	vrangeps	zmm30, zmm29, dword bcst [rdx+508], 123	 # AVX512DQ Disp8
	vrangeps	zmm30, zmm29, dword bcst [rdx+512], 123	 # AVX512DQ
	vrangeps	zmm30, zmm29, dword bcst [rdx-512], 123	 # AVX512DQ Disp8
	vrangeps	zmm30, zmm29, dword bcst [rdx-516], 123	 # AVX512DQ
	vrangesd	xmm30, xmm29, xmm28, 0xab	 # AVX512DQ
	vrangesd	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512DQ
	vrangesd	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512DQ
	vrangesd	xmm30, xmm29, xmm28{sae}, 0xab	 # AVX512DQ
	vrangesd	xmm30, xmm29, xmm28, 123	 # AVX512DQ
	vrangesd	xmm30, xmm29, xmm28{sae}, 123	 # AVX512DQ
	vrangesd	xmm30, xmm29, QWORD PTR [rcx], 123	 # AVX512DQ
	vrangesd	xmm30, xmm29, QWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vrangesd	xmm30, xmm29, QWORD PTR [rdx+1016], 123	 # AVX512DQ Disp8
	vrangesd	xmm30, xmm29, QWORD PTR [rdx+1024], 123	 # AVX512DQ
	vrangesd	xmm30, xmm29, QWORD PTR [rdx-1024], 123	 # AVX512DQ Disp8
	vrangesd	xmm30, xmm29, QWORD PTR [rdx-1032], 123	 # AVX512DQ
	vrangess	xmm30, xmm29, xmm28, 0xab	 # AVX512DQ
	vrangess	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512DQ
	vrangess	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512DQ
	vrangess	xmm30, xmm29, xmm28{sae}, 0xab	 # AVX512DQ
	vrangess	xmm30, xmm29, xmm28, 123	 # AVX512DQ
	vrangess	xmm30, xmm29, xmm28{sae}, 123	 # AVX512DQ
	vrangess	xmm30, xmm29, DWORD PTR [rcx], 123	 # AVX512DQ
	vrangess	xmm30, xmm29, DWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vrangess	xmm30, xmm29, DWORD PTR [rdx+508], 123	 # AVX512DQ Disp8
	vrangess	xmm30, xmm29, DWORD PTR [rdx+512], 123	 # AVX512DQ
	vrangess	xmm30, xmm29, DWORD PTR [rdx-512], 123	 # AVX512DQ Disp8
	vrangess	xmm30, xmm29, DWORD PTR [rdx-516], 123	 # AVX512DQ
	vandpd	zmm30, zmm29, zmm28	 # AVX512DQ
	vandpd	zmm30{k7}, zmm29, zmm28	 # AVX512DQ
	vandpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512DQ
	vandpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512DQ
	vandpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vandpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512DQ
	vandpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vandpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vandpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vandpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vandpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512DQ Disp8
	vandpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512DQ
	vandpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512DQ Disp8
	vandpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512DQ
	vandps	zmm30, zmm29, zmm28	 # AVX512DQ
	vandps	zmm30{k7}, zmm29, zmm28	 # AVX512DQ
	vandps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512DQ
	vandps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512DQ
	vandps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vandps	zmm30, zmm29, dword bcst [rcx]	 # AVX512DQ
	vandps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vandps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vandps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vandps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vandps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512DQ Disp8
	vandps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512DQ
	vandps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512DQ Disp8
	vandps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512DQ
	vandnpd	zmm30, zmm29, zmm28	 # AVX512DQ
	vandnpd	zmm30{k7}, zmm29, zmm28	 # AVX512DQ
	vandnpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512DQ
	vandnpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512DQ
	vandnpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vandnpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512DQ
	vandnpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vandnpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vandnpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vandnpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vandnpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512DQ Disp8
	vandnpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512DQ
	vandnpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512DQ Disp8
	vandnpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512DQ
	vandnps	zmm30, zmm29, zmm28	 # AVX512DQ
	vandnps	zmm30{k7}, zmm29, zmm28	 # AVX512DQ
	vandnps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512DQ
	vandnps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512DQ
	vandnps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vandnps	zmm30, zmm29, dword bcst [rcx]	 # AVX512DQ
	vandnps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vandnps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vandnps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vandnps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vandnps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512DQ Disp8
	vandnps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512DQ
	vandnps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512DQ Disp8
	vandnps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512DQ
	vorpd	zmm30, zmm29, zmm28	 # AVX512DQ
	vorpd	zmm30{k7}, zmm29, zmm28	 # AVX512DQ
	vorpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512DQ
	vorpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512DQ
	vorpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vorpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512DQ
	vorpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vorpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vorpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vorpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vorpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512DQ Disp8
	vorpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512DQ
	vorpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512DQ Disp8
	vorpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512DQ
	vorps	zmm30, zmm29, zmm28	 # AVX512DQ
	vorps	zmm30{k7}, zmm29, zmm28	 # AVX512DQ
	vorps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512DQ
	vorps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512DQ
	vorps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vorps	zmm30, zmm29, dword bcst [rcx]	 # AVX512DQ
	vorps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vorps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vorps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vorps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vorps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512DQ Disp8
	vorps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512DQ
	vorps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512DQ Disp8
	vorps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512DQ
	vxorpd	zmm30, zmm29, zmm28	 # AVX512DQ
	vxorpd	zmm30{k7}, zmm29, zmm28	 # AVX512DQ
	vxorpd	zmm30{k7}{z}, zmm29, zmm28	 # AVX512DQ
	vxorpd	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512DQ
	vxorpd	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vxorpd	zmm30, zmm29, qword bcst [rcx]	 # AVX512DQ
	vxorpd	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vxorpd	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vxorpd	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vxorpd	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vxorpd	zmm30, zmm29, qword bcst [rdx+1016]	 # AVX512DQ Disp8
	vxorpd	zmm30, zmm29, qword bcst [rdx+1024]	 # AVX512DQ
	vxorpd	zmm30, zmm29, qword bcst [rdx-1024]	 # AVX512DQ Disp8
	vxorpd	zmm30, zmm29, qword bcst [rdx-1032]	 # AVX512DQ
	vxorps	zmm30, zmm29, zmm28	 # AVX512DQ
	vxorps	zmm30{k7}, zmm29, zmm28	 # AVX512DQ
	vxorps	zmm30{k7}{z}, zmm29, zmm28	 # AVX512DQ
	vxorps	zmm30, zmm29, ZMMWORD PTR [rcx]	 # AVX512DQ
	vxorps	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vxorps	zmm30, zmm29, dword bcst [rcx]	 # AVX512DQ
	vxorps	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vxorps	zmm30, zmm29, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vxorps	zmm30, zmm29, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vxorps	zmm30, zmm29, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vxorps	zmm30, zmm29, dword bcst [rdx+508]	 # AVX512DQ Disp8
	vxorps	zmm30, zmm29, dword bcst [rdx+512]	 # AVX512DQ
	vxorps	zmm30, zmm29, dword bcst [rdx-512]	 # AVX512DQ Disp8
	vxorps	zmm30, zmm29, dword bcst [rdx-516]	 # AVX512DQ
	vreducepd	zmm30, zmm29, 0xab	 # AVX512DQ
	vreducepd	zmm30{k7}, zmm29, 0xab	 # AVX512DQ
	vreducepd	zmm30{k7}{z}, zmm29, 0xab	 # AVX512DQ
	vreducepd	zmm30, zmm29{sae}, 0xab	 # AVX512DQ
	vreducepd	zmm30, zmm29, 123	 # AVX512DQ
	vreducepd	zmm30, zmm29{sae}, 123	 # AVX512DQ
	vreducepd	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512DQ
	vreducepd	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vreducepd	zmm30, qword bcst [rcx], 123	 # AVX512DQ
	vreducepd	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512DQ Disp8
	vreducepd	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512DQ
	vreducepd	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512DQ Disp8
	vreducepd	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512DQ
	vreducepd	zmm30, qword bcst [rdx+1016], 123	 # AVX512DQ Disp8
	vreducepd	zmm30, qword bcst [rdx+1024], 123	 # AVX512DQ
	vreducepd	zmm30, qword bcst [rdx-1024], 123	 # AVX512DQ Disp8
	vreducepd	zmm30, qword bcst [rdx-1032], 123	 # AVX512DQ
	vreduceps	zmm30, zmm29, 0xab	 # AVX512DQ
	vreduceps	zmm30{k7}, zmm29, 0xab	 # AVX512DQ
	vreduceps	zmm30{k7}{z}, zmm29, 0xab	 # AVX512DQ
	vreduceps	zmm30, zmm29{sae}, 0xab	 # AVX512DQ
	vreduceps	zmm30, zmm29, 123	 # AVX512DQ
	vreduceps	zmm30, zmm29{sae}, 123	 # AVX512DQ
	vreduceps	zmm30, ZMMWORD PTR [rcx], 123	 # AVX512DQ
	vreduceps	zmm30, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vreduceps	zmm30, dword bcst [rcx], 123	 # AVX512DQ
	vreduceps	zmm30, ZMMWORD PTR [rdx+8128], 123	 # AVX512DQ Disp8
	vreduceps	zmm30, ZMMWORD PTR [rdx+8192], 123	 # AVX512DQ
	vreduceps	zmm30, ZMMWORD PTR [rdx-8192], 123	 # AVX512DQ Disp8
	vreduceps	zmm30, ZMMWORD PTR [rdx-8256], 123	 # AVX512DQ
	vreduceps	zmm30, dword bcst [rdx+508], 123	 # AVX512DQ Disp8
	vreduceps	zmm30, dword bcst [rdx+512], 123	 # AVX512DQ
	vreduceps	zmm30, dword bcst [rdx-512], 123	 # AVX512DQ Disp8
	vreduceps	zmm30, dword bcst [rdx-516], 123	 # AVX512DQ
	vreducesd	xmm30, xmm29, xmm28, 0xab	 # AVX512DQ
	vreducesd	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512DQ
	vreducesd	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512DQ
	vreducesd	xmm30, xmm29, xmm28{sae}, 0xab	 # AVX512DQ
	vreducesd	xmm30, xmm29, xmm28, 123	 # AVX512DQ
	vreducesd	xmm30, xmm29, xmm28{sae}, 123	 # AVX512DQ
	vreducesd	xmm30, xmm29, QWORD PTR [rcx], 123	 # AVX512DQ
	vreducesd	xmm30, xmm29, QWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vreducesd	xmm30, xmm29, QWORD PTR [rdx+1016], 123	 # AVX512DQ Disp8
	vreducesd	xmm30, xmm29, QWORD PTR [rdx+1024], 123	 # AVX512DQ
	vreducesd	xmm30, xmm29, QWORD PTR [rdx-1024], 123	 # AVX512DQ Disp8
	vreducesd	xmm30, xmm29, QWORD PTR [rdx-1032], 123	 # AVX512DQ
	vreducess	xmm30, xmm29, xmm28, 0xab	 # AVX512DQ
	vreducess	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512DQ
	vreducess	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512DQ
	vreducess	xmm30, xmm29, xmm28{sae}, 0xab	 # AVX512DQ
	vreducess	xmm30, xmm29, xmm28, 123	 # AVX512DQ
	vreducess	xmm30, xmm29, xmm28{sae}, 123	 # AVX512DQ
	vreducess	xmm30, xmm29, DWORD PTR [rcx], 123	 # AVX512DQ
	vreducess	xmm30, xmm29, DWORD PTR [rax+r14*8+0x1234], 123	 # AVX512DQ
	vreducess	xmm30, xmm29, DWORD PTR [rdx+508], 123	 # AVX512DQ Disp8
	vreducess	xmm30, xmm29, DWORD PTR [rdx+512], 123	 # AVX512DQ
	vreducess	xmm30, xmm29, DWORD PTR [rdx-512], 123	 # AVX512DQ Disp8
	vreducess	xmm30, xmm29, DWORD PTR [rdx-516], 123	 # AVX512DQ
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
	kmovb	k5, BYTE PTR [rcx]	 # AVX512DQ
	kmovb	k5, BYTE PTR [rax+r14*8+0x1234]	 # AVX512DQ
	kmovb	BYTE PTR [rcx], k5	 # AVX512DQ
	kmovb	BYTE PTR [rax+r14*8+0x1234], k5	 # AVX512DQ
	kmovb	k5, eax	 # AVX512DQ
	kmovb	k5, ebp	 # AVX512DQ
	kmovb	k5, r13d	 # AVX512DQ
	kmovb	eax, k5	 # AVX512DQ
	kmovb	ebp, k5	 # AVX512DQ
	kmovb	r13d, k5	 # AVX512DQ
	kaddw	k5, k6, k7	 # AVX512DQ
	kaddb	k5, k6, k7	 # AVX512DQ
	vextractf64x2	XMMWORD PTR [rcx], zmm30, 0xab	 # AVX512DQ
	vextractf64x2	XMMWORD PTR [rcx]{k7}, zmm30, 0xab	 # AVX512DQ
	vextractf64x2	XMMWORD PTR [rcx], zmm30, 123	 # AVX512DQ
	vextractf64x2	XMMWORD PTR [rax+r14*8+0x1234], zmm30, 123	 # AVX512DQ
	vextractf64x2	XMMWORD PTR [rdx+2032], zmm30, 123	 # AVX512DQ Disp8
	vextractf64x2	XMMWORD PTR [rdx+2048], zmm30, 123	 # AVX512DQ
	vextractf64x2	XMMWORD PTR [rdx-2048], zmm30, 123	 # AVX512DQ Disp8
	vextractf64x2	XMMWORD PTR [rdx-2064], zmm30, 123	 # AVX512DQ
	vextractf32x8	YMMWORD PTR [rcx], zmm30, 0xab	 # AVX512DQ
	vextractf32x8	YMMWORD PTR [rcx]{k7}, zmm30, 0xab	 # AVX512DQ
	vextractf32x8	YMMWORD PTR [rcx], zmm30, 123	 # AVX512DQ
	vextractf32x8	YMMWORD PTR [rax+r14*8+0x1234], zmm30, 123	 # AVX512DQ
	vextractf32x8	YMMWORD PTR [rdx+4064], zmm30, 123	 # AVX512DQ Disp8
	vextractf32x8	YMMWORD PTR [rdx+4096], zmm30, 123	 # AVX512DQ
	vextractf32x8	YMMWORD PTR [rdx-4096], zmm30, 123	 # AVX512DQ Disp8
	vextractf32x8	YMMWORD PTR [rdx-4128], zmm30, 123	 # AVX512DQ
	vextracti64x2	XMMWORD PTR [rcx], zmm30, 0xab	 # AVX512DQ
	vextracti64x2	XMMWORD PTR [rcx]{k7}, zmm30, 0xab	 # AVX512DQ
	vextracti64x2	XMMWORD PTR [rcx], zmm30, 123	 # AVX512DQ
	vextracti64x2	XMMWORD PTR [rax+r14*8+0x1234], zmm30, 123	 # AVX512DQ
	vextracti64x2	XMMWORD PTR [rdx+2032], zmm30, 123	 # AVX512DQ Disp8
	vextracti64x2	XMMWORD PTR [rdx+2048], zmm30, 123	 # AVX512DQ
	vextracti64x2	XMMWORD PTR [rdx-2048], zmm30, 123	 # AVX512DQ Disp8
	vextracti64x2	XMMWORD PTR [rdx-2064], zmm30, 123	 # AVX512DQ
	vextracti32x8	YMMWORD PTR [rcx], zmm30, 0xab	 # AVX512DQ
	vextracti32x8	YMMWORD PTR [rcx]{k7}, zmm30, 0xab	 # AVX512DQ
	vextracti32x8	YMMWORD PTR [rcx], zmm30, 123	 # AVX512DQ
	vextracti32x8	YMMWORD PTR [rax+r14*8+0x1234], zmm30, 123	 # AVX512DQ
	vextracti32x8	YMMWORD PTR [rdx+4064], zmm30, 123	 # AVX512DQ Disp8
	vextracti32x8	YMMWORD PTR [rdx+4096], zmm30, 123	 # AVX512DQ
	vextracti32x8	YMMWORD PTR [rdx-4096], zmm30, 123	 # AVX512DQ Disp8
	vextracti32x8	YMMWORD PTR [rdx-4128], zmm30, 123	 # AVX512DQ
	vcvttpd2qq	zmm30, zmm29	 # AVX512DQ
	vcvttpd2qq	zmm30{k7}, zmm29	 # AVX512DQ
	vcvttpd2qq	zmm30{k7}{z}, zmm29	 # AVX512DQ
	vcvttpd2qq	zmm30, zmm29{sae}	 # AVX512DQ
	vcvttpd2qq	zmm30, ZMMWORD PTR [rcx]	 # AVX512DQ
	vcvttpd2qq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vcvttpd2qq	zmm30, qword bcst [rcx]	 # AVX512DQ
	vcvttpd2qq	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vcvttpd2qq	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vcvttpd2qq	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vcvttpd2qq	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vcvttpd2qq	zmm30, qword bcst [rdx+1016]	 # AVX512DQ Disp8
	vcvttpd2qq	zmm30, qword bcst [rdx+1024]	 # AVX512DQ
	vcvttpd2qq	zmm30, qword bcst [rdx-1024]	 # AVX512DQ Disp8
	vcvttpd2qq	zmm30, qword bcst [rdx-1032]	 # AVX512DQ
	vcvttpd2uqq	zmm30, zmm29	 # AVX512DQ
	vcvttpd2uqq	zmm30{k7}, zmm29	 # AVX512DQ
	vcvttpd2uqq	zmm30{k7}{z}, zmm29	 # AVX512DQ
	vcvttpd2uqq	zmm30, zmm29{sae}	 # AVX512DQ
	vcvttpd2uqq	zmm30, ZMMWORD PTR [rcx]	 # AVX512DQ
	vcvttpd2uqq	zmm30, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vcvttpd2uqq	zmm30, qword bcst [rcx]	 # AVX512DQ
	vcvttpd2uqq	zmm30, ZMMWORD PTR [rdx+8128]	 # AVX512DQ Disp8
	vcvttpd2uqq	zmm30, ZMMWORD PTR [rdx+8192]	 # AVX512DQ
	vcvttpd2uqq	zmm30, ZMMWORD PTR [rdx-8192]	 # AVX512DQ Disp8
	vcvttpd2uqq	zmm30, ZMMWORD PTR [rdx-8256]	 # AVX512DQ
	vcvttpd2uqq	zmm30, qword bcst [rdx+1016]	 # AVX512DQ Disp8
	vcvttpd2uqq	zmm30, qword bcst [rdx+1024]	 # AVX512DQ
	vcvttpd2uqq	zmm30, qword bcst [rdx-1024]	 # AVX512DQ Disp8
	vcvttpd2uqq	zmm30, qword bcst [rdx-1032]	 # AVX512DQ
	vcvttps2qq	zmm30, ymm29	 # AVX512DQ
	vcvttps2qq	zmm30{k7}, ymm29	 # AVX512DQ
	vcvttps2qq	zmm30{k7}{z}, ymm29	 # AVX512DQ
	vcvttps2qq	zmm30, ymm29{sae}	 # AVX512DQ
	vcvttps2qq	zmm30, YMMWORD PTR [rcx]	 # AVX512DQ
	vcvttps2qq	zmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vcvttps2qq	zmm30, dword bcst [rcx]	 # AVX512DQ
	vcvttps2qq	zmm30, YMMWORD PTR [rdx+4064]	 # AVX512DQ Disp8
	vcvttps2qq	zmm30, YMMWORD PTR [rdx+4096]	 # AVX512DQ
	vcvttps2qq	zmm30, YMMWORD PTR [rdx-4096]	 # AVX512DQ Disp8
	vcvttps2qq	zmm30, YMMWORD PTR [rdx-4128]	 # AVX512DQ
	vcvttps2qq	zmm30, dword bcst [rdx+508]	 # AVX512DQ Disp8
	vcvttps2qq	zmm30, dword bcst [rdx+512]	 # AVX512DQ
	vcvttps2qq	zmm30, dword bcst [rdx-512]	 # AVX512DQ Disp8
	vcvttps2qq	zmm30, dword bcst [rdx-516]	 # AVX512DQ
	vcvttps2qq	zmm30, DWORD BCST [rdx+508]	 # AVX512DQ Disp8
	vcvttps2uqq	zmm30, ymm29	 # AVX512DQ
	vcvttps2uqq	zmm30{k7}, ymm29	 # AVX512DQ
	vcvttps2uqq	zmm30{k7}{z}, ymm29	 # AVX512DQ
	vcvttps2uqq	zmm30, ymm29{sae}	 # AVX512DQ
	vcvttps2uqq	zmm30, YMMWORD PTR [rcx]	 # AVX512DQ
	vcvttps2uqq	zmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512DQ
	vcvttps2uqq	zmm30, dword bcst [rcx]	 # AVX512DQ
	vcvttps2uqq	zmm30, YMMWORD PTR [rdx+4064]	 # AVX512DQ Disp8
	vcvttps2uqq	zmm30, YMMWORD PTR [rdx+4096]	 # AVX512DQ
	vcvttps2uqq	zmm30, YMMWORD PTR [rdx-4096]	 # AVX512DQ Disp8
	vcvttps2uqq	zmm30, YMMWORD PTR [rdx-4128]	 # AVX512DQ
	vcvttps2uqq	zmm30, dword bcst [rdx+508]	 # AVX512DQ Disp8
	vcvttps2uqq	zmm30, dword bcst [rdx+512]	 # AVX512DQ
	vcvttps2uqq	zmm30, dword bcst [rdx-512]	 # AVX512DQ Disp8
	vcvttps2uqq	zmm30, dword bcst [rdx-516]	 # AVX512DQ
	vcvttps2uqq	zmm30, DWORD BCST [rdx+508]	 # AVX512DQ Disp8
	vpmovd2m	k5, zmm30	 # AVX512DQ
	vpmovq2m	k5, zmm30	 # AVX512DQ
	vpmovm2d	zmm30, k5	 # AVX512DQ
	vpmovm2q	zmm30, k5	 # AVX512DQ
