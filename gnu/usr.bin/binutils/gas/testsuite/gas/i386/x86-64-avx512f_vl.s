# Check 64bit AVX512{F,VL} instructions

	.allow_index_reg
	.text
_start:
	vaddpd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vaddpd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vaddpd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vaddpd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vaddpd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vaddpd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vaddpd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vaddpd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vaddpd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vaddpd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vaddpd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vaddpd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vaddpd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vaddpd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vaddpd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vaddpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vaddpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vaddpd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vaddpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vaddpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vaddpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vaddpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vaddpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vaddpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vaddpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vaddpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vaddpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vaddpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vaddps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vaddps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vaddps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vaddps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vaddps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vaddps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vaddps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vaddps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vaddps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vaddps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vaddps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vaddps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vaddps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vaddps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vaddps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vaddps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vaddps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vaddps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vaddps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vaddps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vaddps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vaddps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vaddps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vaddps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vaddps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vaddps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vaddps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vaddps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	valignd	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	valignd	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	valignd	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	valignd	$123, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	valignd	$123, (%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	valignd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	valignd	$123, (%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	valignd	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	valignd	$123, 2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	valignd	$123, -2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	valignd	$123, -2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	valignd	$123, 508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	valignd	$123, 512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	valignd	$123, -512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	valignd	$123, -516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	valignd	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	valignd	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	valignd	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	valignd	$123, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	valignd	$123, (%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	valignd	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	valignd	$123, (%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	valignd	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	valignd	$123, 4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	valignd	$123, -4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	valignd	$123, -4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	valignd	$123, 508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	valignd	$123, 512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	valignd	$123, -512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	valignd	$123, -516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmpd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmpd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vblendmpd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vblendmpd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmpd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmpd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmpd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vblendmpd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmpd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vblendmpd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmpd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vblendmpd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmpd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vblendmpd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmpd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vblendmpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vblendmpd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vblendmpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vblendmpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vblendmpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vblendmpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vblendmps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vblendmps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vblendmps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vblendmps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vblendmps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vblendmps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vblendmps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vblendmps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vblendmps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vblendmps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vblendmps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vblendmps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vblendmps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vblendmps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vbroadcastf32x4	(%rcx), %ymm30	 # AVX512{F,VL}
	vbroadcastf32x4	(%rcx), %ymm30{%k7}	 # AVX512{F,VL}
	vbroadcastf32x4	(%rcx), %ymm30{%k7}{z}	 # AVX512{F,VL}
	vbroadcastf32x4	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vbroadcastf32x4	2032(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vbroadcastf32x4	2048(%rdx), %ymm30	 # AVX512{F,VL}
	vbroadcastf32x4	-2048(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vbroadcastf32x4	-2064(%rdx), %ymm30	 # AVX512{F,VL}
	vbroadcasti32x4	(%rcx), %ymm30	 # AVX512{F,VL}
	vbroadcasti32x4	(%rcx), %ymm30{%k7}	 # AVX512{F,VL}
	vbroadcasti32x4	(%rcx), %ymm30{%k7}{z}	 # AVX512{F,VL}
	vbroadcasti32x4	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vbroadcasti32x4	2032(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vbroadcasti32x4	2048(%rdx), %ymm30	 # AVX512{F,VL}
	vbroadcasti32x4	-2048(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vbroadcasti32x4	-2064(%rdx), %ymm30	 # AVX512{F,VL}
	vbroadcastsd	(%rcx), %ymm30	 # AVX512{F,VL}
	vbroadcastsd	(%rcx), %ymm30{%k7}	 # AVX512{F,VL}
	vbroadcastsd	(%rcx), %ymm30{%k7}{z}	 # AVX512{F,VL}
	vbroadcastsd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vbroadcastsd	1016(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vbroadcastsd	1024(%rdx), %ymm30	 # AVX512{F,VL}
	vbroadcastsd	-1024(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vbroadcastsd	-1032(%rdx), %ymm30	 # AVX512{F,VL}
	vbroadcastsd	%xmm29, %ymm30	 # AVX512{F,VL}
	vbroadcastsd	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vbroadcastsd	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vbroadcastss	(%rcx), %xmm30	 # AVX512{F,VL}
	vbroadcastss	(%rcx), %xmm30{%k7}	 # AVX512{F,VL}
	vbroadcastss	(%rcx), %xmm30{%k7}{z}	 # AVX512{F,VL}
	vbroadcastss	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vbroadcastss	508(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vbroadcastss	512(%rdx), %xmm30	 # AVX512{F,VL}
	vbroadcastss	-512(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vbroadcastss	-516(%rdx), %xmm30	 # AVX512{F,VL}
	vbroadcastss	(%rcx), %ymm30	 # AVX512{F,VL}
	vbroadcastss	(%rcx), %ymm30{%k7}	 # AVX512{F,VL}
	vbroadcastss	(%rcx), %ymm30{%k7}{z}	 # AVX512{F,VL}
	vbroadcastss	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vbroadcastss	508(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vbroadcastss	512(%rdx), %ymm30	 # AVX512{F,VL}
	vbroadcastss	-512(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vbroadcastss	-516(%rdx), %ymm30	 # AVX512{F,VL}
	vbroadcastss	%xmm29, %xmm30	 # AVX512{F,VL}
	vbroadcastss	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vbroadcastss	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vbroadcastss	%xmm29, %ymm30	 # AVX512{F,VL}
	vbroadcastss	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vbroadcastss	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vcmppd	$0xab, %xmm28, %xmm29, %k5	 # AVX512{F,VL}
	vcmppd	$0xab, %xmm28, %xmm29, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, %xmm28, %xmm29, %k5	 # AVX512{F,VL}
	vcmppd	$123, (%rcx), %xmm29, %k5	 # AVX512{F,VL}
	vcmppd	$123, 0x123(%rax,%r14,8), %xmm29, %k5	 # AVX512{F,VL}
	vcmppd	$123, (%rcx){1to2}, %xmm29, %k5	 # AVX512{F,VL}
	vcmppd	$123, 2032(%rdx), %xmm29, %k5	 # AVX512{F,VL} Disp8
	vcmppd	$123, 2048(%rdx), %xmm29, %k5	 # AVX512{F,VL}
	vcmppd	$123, -2048(%rdx), %xmm29, %k5	 # AVX512{F,VL} Disp8
	vcmppd	$123, -2064(%rdx), %xmm29, %k5	 # AVX512{F,VL}
	vcmppd	$123, 1016(%rdx){1to2}, %xmm29, %k5	 # AVX512{F,VL} Disp8
	vcmppd	$123, 1024(%rdx){1to2}, %xmm29, %k5	 # AVX512{F,VL}
	vcmppd	$123, -1024(%rdx){1to2}, %xmm29, %k5	 # AVX512{F,VL} Disp8
	vcmppd	$123, -1032(%rdx){1to2}, %xmm29, %k5	 # AVX512{F,VL}
	vcmppd	$0xab, %ymm28, %ymm29, %k5	 # AVX512{F,VL}
	vcmppd	$0xab, %ymm28, %ymm29, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, %ymm28, %ymm29, %k5	 # AVX512{F,VL}
	vcmppd	$123, (%rcx), %ymm29, %k5	 # AVX512{F,VL}
	vcmppd	$123, 0x123(%rax,%r14,8), %ymm29, %k5	 # AVX512{F,VL}
	vcmppd	$123, (%rcx){1to4}, %ymm29, %k5	 # AVX512{F,VL}
	vcmppd	$123, 4064(%rdx), %ymm29, %k5	 # AVX512{F,VL} Disp8
	vcmppd	$123, 4096(%rdx), %ymm29, %k5	 # AVX512{F,VL}
	vcmppd	$123, -4096(%rdx), %ymm29, %k5	 # AVX512{F,VL} Disp8
	vcmppd	$123, -4128(%rdx), %ymm29, %k5	 # AVX512{F,VL}
	vcmppd	$123, 1016(%rdx){1to4}, %ymm29, %k5	 # AVX512{F,VL} Disp8
	vcmppd	$123, 1024(%rdx){1to4}, %ymm29, %k5	 # AVX512{F,VL}
	vcmppd	$123, -1024(%rdx){1to4}, %ymm29, %k5	 # AVX512{F,VL} Disp8
	vcmppd	$123, -1032(%rdx){1to4}, %ymm29, %k5	 # AVX512{F,VL}
	vcmpps	$0xab, %xmm28, %xmm29, %k5	 # AVX512{F,VL}
	vcmpps	$0xab, %xmm28, %xmm29, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, %xmm28, %xmm29, %k5	 # AVX512{F,VL}
	vcmpps	$123, (%rcx), %xmm29, %k5	 # AVX512{F,VL}
	vcmpps	$123, 0x123(%rax,%r14,8), %xmm29, %k5	 # AVX512{F,VL}
	vcmpps	$123, (%rcx){1to4}, %xmm29, %k5	 # AVX512{F,VL}
	vcmpps	$123, 2032(%rdx), %xmm29, %k5	 # AVX512{F,VL} Disp8
	vcmpps	$123, 2048(%rdx), %xmm29, %k5	 # AVX512{F,VL}
	vcmpps	$123, -2048(%rdx), %xmm29, %k5	 # AVX512{F,VL} Disp8
	vcmpps	$123, -2064(%rdx), %xmm29, %k5	 # AVX512{F,VL}
	vcmpps	$123, 508(%rdx){1to4}, %xmm29, %k5	 # AVX512{F,VL} Disp8
	vcmpps	$123, 512(%rdx){1to4}, %xmm29, %k5	 # AVX512{F,VL}
	vcmpps	$123, -512(%rdx){1to4}, %xmm29, %k5	 # AVX512{F,VL} Disp8
	vcmpps	$123, -516(%rdx){1to4}, %xmm29, %k5	 # AVX512{F,VL}
	vcmpps	$0xab, %ymm28, %ymm29, %k5	 # AVX512{F,VL}
	vcmpps	$0xab, %ymm28, %ymm29, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, %ymm28, %ymm29, %k5	 # AVX512{F,VL}
	vcmpps	$123, (%rcx), %ymm29, %k5	 # AVX512{F,VL}
	vcmpps	$123, 0x123(%rax,%r14,8), %ymm29, %k5	 # AVX512{F,VL}
	vcmpps	$123, (%rcx){1to8}, %ymm29, %k5	 # AVX512{F,VL}
	vcmpps	$123, 4064(%rdx), %ymm29, %k5	 # AVX512{F,VL} Disp8
	vcmpps	$123, 4096(%rdx), %ymm29, %k5	 # AVX512{F,VL}
	vcmpps	$123, -4096(%rdx), %ymm29, %k5	 # AVX512{F,VL} Disp8
	vcmpps	$123, -4128(%rdx), %ymm29, %k5	 # AVX512{F,VL}
	vcmpps	$123, 508(%rdx){1to8}, %ymm29, %k5	 # AVX512{F,VL} Disp8
	vcmpps	$123, 512(%rdx){1to8}, %ymm29, %k5	 # AVX512{F,VL}
	vcmpps	$123, -512(%rdx){1to8}, %ymm29, %k5	 # AVX512{F,VL} Disp8
	vcmpps	$123, -516(%rdx){1to8}, %ymm29, %k5	 # AVX512{F,VL}
	vcompresspd	%xmm30, (%rcx)	 # AVX512{F,VL}
	vcompresspd	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vcompresspd	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vcompresspd	%xmm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vcompresspd	%xmm30, 1024(%rdx)	 # AVX512{F,VL}
	vcompresspd	%xmm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vcompresspd	%xmm30, -1032(%rdx)	 # AVX512{F,VL}
	vcompresspd	%ymm30, (%rcx)	 # AVX512{F,VL}
	vcompresspd	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vcompresspd	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vcompresspd	%ymm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vcompresspd	%ymm30, 1024(%rdx)	 # AVX512{F,VL}
	vcompresspd	%ymm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vcompresspd	%ymm30, -1032(%rdx)	 # AVX512{F,VL}
	vcompresspd	%xmm29, %xmm30	 # AVX512{F,VL}
	vcompresspd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcompresspd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcompresspd	%ymm29, %ymm30	 # AVX512{F,VL}
	vcompresspd	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vcompresspd	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vcompressps	%xmm30, (%rcx)	 # AVX512{F,VL}
	vcompressps	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vcompressps	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vcompressps	%xmm30, 508(%rdx)	 # AVX512{F,VL} Disp8
	vcompressps	%xmm30, 512(%rdx)	 # AVX512{F,VL}
	vcompressps	%xmm30, -512(%rdx)	 # AVX512{F,VL} Disp8
	vcompressps	%xmm30, -516(%rdx)	 # AVX512{F,VL}
	vcompressps	%ymm30, (%rcx)	 # AVX512{F,VL}
	vcompressps	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vcompressps	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vcompressps	%ymm30, 508(%rdx)	 # AVX512{F,VL} Disp8
	vcompressps	%ymm30, 512(%rdx)	 # AVX512{F,VL}
	vcompressps	%ymm30, -512(%rdx)	 # AVX512{F,VL} Disp8
	vcompressps	%ymm30, -516(%rdx)	 # AVX512{F,VL}
	vcompressps	%xmm29, %xmm30	 # AVX512{F,VL}
	vcompressps	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcompressps	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcompressps	%ymm29, %ymm30	 # AVX512{F,VL}
	vcompressps	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vcompressps	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vcvtdq2pd	%xmm29, %xmm30	 # AVX512{F,VL}
	vcvtdq2pd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtdq2pd	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvtdq2pd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvtdq2pd	(%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtdq2pd	1016(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtdq2pd	1024(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtdq2pd	-1024(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtdq2pd	-1032(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtdq2pd	508(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtdq2pd	512(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtdq2pd	-512(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtdq2pd	-516(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtdq2pd	%xmm29, %ymm30	 # AVX512{F,VL}
	vcvtdq2pd	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vcvtdq2pd	(%rcx), %ymm30	 # AVX512{F,VL}
	vcvtdq2pd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vcvtdq2pd	(%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vcvtdq2pd	2032(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtdq2pd	2048(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtdq2pd	-2048(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtdq2pd	-2064(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtdq2pd	508(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vcvtdq2pd	512(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vcvtdq2pd	-512(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vcvtdq2pd	-516(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vcvtdq2ps	%xmm29, %xmm30	 # AVX512{F,VL}
	vcvtdq2ps	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtdq2ps	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvtdq2ps	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvtdq2ps	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtdq2ps	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtdq2ps	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtdq2ps	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtdq2ps	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtdq2ps	508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtdq2ps	512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtdq2ps	-512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtdq2ps	-516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtdq2ps	%ymm29, %ymm30	 # AVX512{F,VL}
	vcvtdq2ps	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vcvtdq2ps	(%rcx), %ymm30	 # AVX512{F,VL}
	vcvtdq2ps	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vcvtdq2ps	(%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvtdq2ps	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtdq2ps	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtdq2ps	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtdq2ps	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtdq2ps	508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vcvtdq2ps	512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvtdq2ps	-512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vcvtdq2ps	-516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvtpd2dq	%xmm29, %xmm30	 # AVX512{F,VL}
	vcvtpd2dq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtpd2dq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtpd2dqx	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvtpd2dqx	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvtpd2dq	(%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtpd2dqx	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2dqx	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtpd2dqx	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2dqx	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtpd2dqx	1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2dqx	1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtpd2dqx	-1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2dqx	-1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtpd2dq	%ymm29, %xmm30	 # AVX512{F,VL}
	vcvtpd2dq	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtpd2dq	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtpd2dqy	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvtpd2dqy	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvtpd2dq	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtpd2dqy	4064(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2dqy	4096(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtpd2dqy	-4096(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2dqy	-4128(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtpd2dqy	1016(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2dqy	1024(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtpd2dqy	-1024(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2dqy	-1032(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtpd2ps	%xmm29, %xmm30	 # AVX512{F,VL}
	vcvtpd2ps	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtpd2ps	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtpd2psx	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvtpd2psx	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvtpd2ps	(%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtpd2psx	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2psx	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtpd2psx	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2psx	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtpd2psx	1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2psx	1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtpd2psx	-1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2psx	-1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtpd2ps	%ymm29, %xmm30	 # AVX512{F,VL}
	vcvtpd2ps	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtpd2ps	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtpd2psy	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvtpd2psy	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvtpd2ps	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtpd2psy	4064(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2psy	4096(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtpd2psy	-4096(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2psy	-4128(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtpd2psy	1016(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2psy	1024(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtpd2psy	-1024(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2psy	-1032(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtpd2udq	%xmm29, %xmm30	 # AVX512{F,VL}
	vcvtpd2udq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtpd2udq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtpd2udqx	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvtpd2udqx	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvtpd2udq	(%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtpd2udqx	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2udqx	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtpd2udqx	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2udqx	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtpd2udqx	1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2udqx	1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtpd2udqx	-1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2udqx	-1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtpd2udq	%ymm29, %xmm30	 # AVX512{F,VL}
	vcvtpd2udq	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtpd2udq	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtpd2udqy	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvtpd2udqy	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvtpd2udq	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtpd2udqy	4064(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2udqy	4096(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtpd2udqy	-4096(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2udqy	-4128(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtpd2udqy	1016(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2udqy	1024(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtpd2udqy	-1024(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtpd2udqy	-1032(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtph2ps	%xmm29, %xmm30	 # AVX512{F,VL}
	vcvtph2ps	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtph2ps	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtph2ps	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvtph2ps	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvtph2ps	1016(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtph2ps	1024(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtph2ps	-1024(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtph2ps	-1032(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtph2ps	%xmm29, %ymm30	 # AVX512{F,VL}
	vcvtph2ps	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vcvtph2ps	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vcvtph2ps	(%rcx), %ymm30	 # AVX512{F,VL}
	vcvtph2ps	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vcvtph2ps	2032(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtph2ps	2048(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtph2ps	-2048(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtph2ps	-2064(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtps2dq	%xmm29, %xmm30	 # AVX512{F,VL}
	vcvtps2dq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtps2dq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtps2dq	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvtps2dq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvtps2dq	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtps2dq	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtps2dq	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtps2dq	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtps2dq	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtps2dq	508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtps2dq	512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtps2dq	-512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtps2dq	-516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtps2dq	%ymm29, %ymm30	 # AVX512{F,VL}
	vcvtps2dq	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vcvtps2dq	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vcvtps2dq	(%rcx), %ymm30	 # AVX512{F,VL}
	vcvtps2dq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vcvtps2dq	(%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvtps2dq	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtps2dq	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtps2dq	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtps2dq	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtps2dq	508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vcvtps2dq	512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvtps2dq	-512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vcvtps2dq	-516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvtps2pd	%xmm29, %xmm30	 # AVX512{F,VL}
	vcvtps2pd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtps2pd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtps2pd	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvtps2pd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvtps2pd	(%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtps2pd	1016(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtps2pd	1024(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtps2pd	-1024(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtps2pd	-1032(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtps2pd	508(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtps2pd	512(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtps2pd	-512(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtps2pd	-516(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtps2pd	%xmm29, %ymm30	 # AVX512{F,VL}
	vcvtps2pd	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vcvtps2pd	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vcvtps2pd	(%rcx), %ymm30	 # AVX512{F,VL}
	vcvtps2pd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vcvtps2pd	(%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vcvtps2pd	2032(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtps2pd	2048(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtps2pd	-2048(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtps2pd	-2064(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtps2pd	508(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vcvtps2pd	512(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vcvtps2pd	-512(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vcvtps2pd	-516(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtps2ph	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %ymm29, %xmm30	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtps2ph	$123, %ymm29, %xmm30	 # AVX512{F,VL}
	vcvtps2udq	%xmm29, %xmm30	 # AVX512{F,VL}
	vcvtps2udq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtps2udq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtps2udq	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvtps2udq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvtps2udq	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtps2udq	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtps2udq	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtps2udq	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtps2udq	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtps2udq	508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtps2udq	512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtps2udq	-512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtps2udq	-516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtps2udq	%ymm29, %ymm30	 # AVX512{F,VL}
	vcvtps2udq	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vcvtps2udq	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vcvtps2udq	(%rcx), %ymm30	 # AVX512{F,VL}
	vcvtps2udq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vcvtps2udq	(%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvtps2udq	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtps2udq	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtps2udq	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtps2udq	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtps2udq	508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vcvtps2udq	512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvtps2udq	-512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vcvtps2udq	-516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvttpd2dq	%xmm29, %xmm30	 # AVX512{F,VL}
	vcvttpd2dq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvttpd2dq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvttpd2dqx	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvttpd2dqx	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvttpd2dq	(%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvttpd2dqx	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2dqx	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vcvttpd2dqx	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2dqx	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vcvttpd2dqx	1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2dqx	1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvttpd2dqx	-1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2dqx	-1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvttpd2dq	%ymm29, %xmm30	 # AVX512{F,VL}
	vcvttpd2dq	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvttpd2dq	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvttpd2dqy	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvttpd2dqy	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvttpd2dq	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvttpd2dqy	4064(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2dqy	4096(%rdx), %xmm30	 # AVX512{F,VL}
	vcvttpd2dqy	-4096(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2dqy	-4128(%rdx), %xmm30	 # AVX512{F,VL}
	vcvttpd2dqy	1016(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2dqy	1024(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvttpd2dqy	-1024(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2dqy	-1032(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvttps2dq	%xmm29, %xmm30	 # AVX512{F,VL}
	vcvttps2dq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvttps2dq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvttps2dq	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvttps2dq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvttps2dq	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvttps2dq	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvttps2dq	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vcvttps2dq	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvttps2dq	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vcvttps2dq	508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvttps2dq	512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvttps2dq	-512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvttps2dq	-516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvttps2dq	%ymm29, %ymm30	 # AVX512{F,VL}
	vcvttps2dq	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vcvttps2dq	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vcvttps2dq	(%rcx), %ymm30	 # AVX512{F,VL}
	vcvttps2dq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vcvttps2dq	(%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvttps2dq	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvttps2dq	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vcvttps2dq	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvttps2dq	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vcvttps2dq	508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vcvttps2dq	512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvttps2dq	-512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vcvttps2dq	-516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvtudq2pd	%xmm29, %xmm30	 # AVX512{F,VL}
	vcvtudq2pd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtudq2pd	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvtudq2pd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvtudq2pd	(%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtudq2pd	1016(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtudq2pd	1024(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtudq2pd	-1024(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtudq2pd	-1032(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtudq2pd	508(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtudq2pd	512(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtudq2pd	-512(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtudq2pd	-516(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvtudq2pd	%xmm29, %ymm30	 # AVX512{F,VL}
	vcvtudq2pd	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vcvtudq2pd	(%rcx), %ymm30	 # AVX512{F,VL}
	vcvtudq2pd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vcvtudq2pd	(%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vcvtudq2pd	2032(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtudq2pd	2048(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtudq2pd	-2048(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtudq2pd	-2064(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtudq2pd	508(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vcvtudq2pd	512(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vcvtudq2pd	-512(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vcvtudq2pd	-516(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vcvtudq2ps	%xmm29, %xmm30	 # AVX512{F,VL}
	vcvtudq2ps	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvtudq2ps	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvtudq2ps	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvtudq2ps	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtudq2ps	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtudq2ps	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtudq2ps	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvtudq2ps	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vcvtudq2ps	508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtudq2ps	512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtudq2ps	-512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvtudq2ps	-516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvtudq2ps	%ymm29, %ymm30	 # AVX512{F,VL}
	vcvtudq2ps	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vcvtudq2ps	(%rcx), %ymm30	 # AVX512{F,VL}
	vcvtudq2ps	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vcvtudq2ps	(%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvtudq2ps	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtudq2ps	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtudq2ps	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvtudq2ps	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vcvtudq2ps	508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vcvtudq2ps	512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvtudq2ps	-512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vcvtudq2ps	-516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vdivpd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vdivpd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vdivpd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vdivpd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vdivpd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vdivpd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vdivpd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vdivpd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vdivpd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vdivpd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vdivpd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vdivpd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vdivpd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vdivpd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vdivpd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vdivpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vdivpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vdivpd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vdivpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vdivpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vdivpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vdivpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vdivpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vdivpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vdivpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vdivpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vdivpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vdivpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vdivps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vdivps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vdivps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vdivps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vdivps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vdivps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vdivps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vdivps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vdivps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vdivps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vdivps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vdivps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vdivps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vdivps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vdivps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vdivps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vdivps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vdivps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vdivps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vdivps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vdivps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vdivps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vdivps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vdivps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vdivps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vdivps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vdivps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vdivps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vexpandpd	(%rcx), %xmm30	 # AVX512{F,VL}
	vexpandpd	(%rcx), %xmm30{%k7}	 # AVX512{F,VL}
	vexpandpd	(%rcx), %xmm30{%k7}{z}	 # AVX512{F,VL}
	vexpandpd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vexpandpd	1016(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vexpandpd	1024(%rdx), %xmm30	 # AVX512{F,VL}
	vexpandpd	-1024(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vexpandpd	-1032(%rdx), %xmm30	 # AVX512{F,VL}
	vexpandpd	(%rcx), %ymm30	 # AVX512{F,VL}
	vexpandpd	(%rcx), %ymm30{%k7}	 # AVX512{F,VL}
	vexpandpd	(%rcx), %ymm30{%k7}{z}	 # AVX512{F,VL}
	vexpandpd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vexpandpd	1016(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vexpandpd	1024(%rdx), %ymm30	 # AVX512{F,VL}
	vexpandpd	-1024(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vexpandpd	-1032(%rdx), %ymm30	 # AVX512{F,VL}
	vexpandpd	%xmm29, %xmm30	 # AVX512{F,VL}
	vexpandpd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vexpandpd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vexpandpd	%ymm29, %ymm30	 # AVX512{F,VL}
	vexpandpd	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vexpandpd	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vexpandps	(%rcx), %xmm30	 # AVX512{F,VL}
	vexpandps	(%rcx), %xmm30{%k7}	 # AVX512{F,VL}
	vexpandps	(%rcx), %xmm30{%k7}{z}	 # AVX512{F,VL}
	vexpandps	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vexpandps	508(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vexpandps	512(%rdx), %xmm30	 # AVX512{F,VL}
	vexpandps	-512(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vexpandps	-516(%rdx), %xmm30	 # AVX512{F,VL}
	vexpandps	(%rcx), %ymm30	 # AVX512{F,VL}
	vexpandps	(%rcx), %ymm30{%k7}	 # AVX512{F,VL}
	vexpandps	(%rcx), %ymm30{%k7}{z}	 # AVX512{F,VL}
	vexpandps	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vexpandps	508(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vexpandps	512(%rdx), %ymm30	 # AVX512{F,VL}
	vexpandps	-512(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vexpandps	-516(%rdx), %ymm30	 # AVX512{F,VL}
	vexpandps	%xmm29, %xmm30	 # AVX512{F,VL}
	vexpandps	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vexpandps	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vexpandps	%ymm29, %ymm30	 # AVX512{F,VL}
	vexpandps	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vexpandps	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vextractf32x4	$0xab, %ymm29, %xmm30	 # AVX512{F,VL}
	vextractf32x4	$0xab, %ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vextractf32x4	$0xab, %ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vextractf32x4	$123, %ymm29, %xmm30	 # AVX512{F,VL}
	vextracti32x4	$0xab, %ymm29, %xmm30	 # AVX512{F,VL}
	vextracti32x4	$0xab, %ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vextracti32x4	$0xab, %ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vextracti32x4	$123, %ymm29, %xmm30	 # AVX512{F,VL}
	vfmadd132pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmadd132pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmadd132pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd132pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd132pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd132pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd132pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd132pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmadd132pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmadd132pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd132pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd132pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd132pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd132pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd132pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd132pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd132pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd132pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd132pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd132pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd132ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmadd132ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmadd132ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd132ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd132ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd132ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd132ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd132ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd132ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmadd132ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmadd132ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd132ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd132ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd132ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd132ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd132ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd132ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd132ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd132ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd132ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd132ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmadd213pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmadd213pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd213pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd213pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd213pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd213pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmadd213pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmadd213pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd213pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd213pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd213pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd213pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmadd213ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmadd213ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd213ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd213ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd213ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd213ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd213ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmadd213ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmadd213ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd213ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd213ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd213ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd213ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd213ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmadd231pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmadd231pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd231pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd231pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd231pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd231pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmadd231pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmadd231pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd231pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd231pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd231pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd231pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmadd231ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmadd231ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd231ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd231ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd231ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmadd231ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmadd231ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmadd231ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmadd231ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd231ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd231ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd231ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmadd231ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmadd231ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub132pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub132pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub132ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub132ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub132ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub132ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub213pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub213pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub213ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub213ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub213ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub213ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub231pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub231pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub231ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmaddsub231ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub231ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmaddsub231ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmsub132pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmsub132pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub132pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub132pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub132pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub132pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmsub132pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmsub132pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub132pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub132pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub132pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub132pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmsub132ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmsub132ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub132ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub132ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub132ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub132ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub132ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmsub132ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmsub132ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub132ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub132ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub132ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub132ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub132ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmsub213pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmsub213pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub213pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub213pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub213pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub213pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmsub213pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmsub213pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub213pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub213pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub213pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub213pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmsub213ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmsub213ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub213ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub213ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub213ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub213ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub213ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmsub213ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmsub213ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub213ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub213ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub213ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub213ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub213ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmsub231pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmsub231pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub231pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub231pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub231pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub231pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmsub231pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmsub231pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub231pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub231pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub231pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub231pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmsub231ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmsub231ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub231ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub231ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub231ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsub231ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsub231ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmsub231ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmsub231ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub231ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub231ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub231ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsub231ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsub231ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd132pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd132pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd132ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd132ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd132ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd132ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd213pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd213pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd213ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd213ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd213ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd213ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd231pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd231pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd231ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfmsubadd231ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd231ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfmsubadd231ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfnmadd132pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd132pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd132pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd132pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd132pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfnmadd132pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd132pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd132pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd132pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd132pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfnmadd132ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd132ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd132ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd132ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd132ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd132ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfnmadd132ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd132ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd132ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd132ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd132ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd132ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfnmadd213pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd213pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd213pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd213pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd213pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfnmadd213pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd213pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd213pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd213pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd213pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfnmadd213ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd213ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd213ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd213ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd213ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd213ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfnmadd213ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd213ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd213ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd213ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd213ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd213ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfnmadd231pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd231pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd231pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd231pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd231pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfnmadd231pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd231pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd231pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd231pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd231pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfnmadd231ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd231ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd231ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd231ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmadd231ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmadd231ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfnmadd231ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd231ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd231ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd231ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmadd231ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmadd231ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfnmsub132pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub132pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub132pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub132pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub132pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfnmsub132pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub132pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub132pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub132pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub132pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfnmsub132ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub132ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub132ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub132ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub132ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub132ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfnmsub132ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub132ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub132ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub132ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub132ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub132ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfnmsub213pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub213pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub213pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub213pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub213pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfnmsub213pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub213pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub213pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub213pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub213pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfnmsub213ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub213ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub213ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub213ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub213ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub213ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfnmsub213ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub213ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub213ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub213ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub213ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub213ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfnmsub231pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub231pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub231pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub231pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub231pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfnmsub231pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub231pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub231pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub231pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub231pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfnmsub231ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub231ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub231ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub231ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfnmsub231ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfnmsub231ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfnmsub231ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub231ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub231ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub231ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfnmsub231ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfnmsub231ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vgatherdpd	123(%r14,%xmm31,8), %xmm30{%k1}	 # AVX512{F,VL}
	vgatherdpd	256(%r9,%xmm31), %xmm30{%k1}	 # AVX512{F,VL}
	vgatherdpd	1024(%rcx,%xmm31,4), %xmm30{%k1}	 # AVX512{F,VL}
	vgatherdpd	123(%r14,%xmm31,8), %ymm30{%k1}	 # AVX512{F,VL}
	vgatherdpd	256(%r9,%xmm31), %ymm30{%k1}	 # AVX512{F,VL}
	vgatherdpd	1024(%rcx,%xmm31,4), %ymm30{%k1}	 # AVX512{F,VL}
	vgatherdps	123(%r14,%xmm31,8), %xmm30{%k1}	 # AVX512{F,VL}
	vgatherdps	256(%r9,%xmm31), %xmm30{%k1}	 # AVX512{F,VL}
	vgatherdps	1024(%rcx,%xmm31,4), %xmm30{%k1}	 # AVX512{F,VL}
	vgatherdps	123(%r14,%ymm31,8), %ymm30{%k1}	 # AVX512{F,VL}
	vgatherdps	256(%r9,%ymm31), %ymm30{%k1}	 # AVX512{F,VL}
	vgatherdps	1024(%rcx,%ymm31,4), %ymm30{%k1}	 # AVX512{F,VL}
	vgatherqpd	123(%r14,%xmm31,8), %xmm30{%k1}	 # AVX512{F,VL}
	vgatherqpd	256(%r9,%xmm31), %xmm30{%k1}	 # AVX512{F,VL}
	vgatherqpd	1024(%rcx,%xmm31,4), %xmm30{%k1}	 # AVX512{F,VL}
	vgatherqpd	123(%r14,%ymm31,8), %ymm30{%k1}	 # AVX512{F,VL}
	vgatherqpd	256(%r9,%ymm31), %ymm30{%k1}	 # AVX512{F,VL}
	vgatherqpd	1024(%rcx,%ymm31,4), %ymm30{%k1}	 # AVX512{F,VL}
	vgatherqps	123(%r14,%xmm31,8), %xmm30{%k1}	 # AVX512{F,VL}
	vgatherqps	256(%r9,%xmm31), %xmm30{%k1}	 # AVX512{F,VL}
	vgatherqps	1024(%rcx,%xmm31,4), %xmm30{%k1}	 # AVX512{F,VL}
	vgatherqps	123(%r14,%ymm31,8), %xmm30{%k1}	 # AVX512{F,VL}
	vgatherqps	256(%r9,%ymm31), %xmm30{%k1}	 # AVX512{F,VL}
	vgatherqps	1024(%rcx,%ymm31,4), %xmm30{%k1}	 # AVX512{F,VL}
	vgetexppd	%xmm29, %xmm30	 # AVX512{F,VL}
	vgetexppd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vgetexppd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vgetexppd	(%rcx), %xmm30	 # AVX512{F,VL}
	vgetexppd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vgetexppd	(%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vgetexppd	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vgetexppd	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vgetexppd	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vgetexppd	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vgetexppd	1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vgetexppd	1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vgetexppd	-1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vgetexppd	-1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vgetexppd	%ymm29, %ymm30	 # AVX512{F,VL}
	vgetexppd	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vgetexppd	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vgetexppd	(%rcx), %ymm30	 # AVX512{F,VL}
	vgetexppd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vgetexppd	(%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vgetexppd	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vgetexppd	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vgetexppd	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vgetexppd	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vgetexppd	1016(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vgetexppd	1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vgetexppd	-1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vgetexppd	-1032(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vgetexpps	%xmm29, %xmm30	 # AVX512{F,VL}
	vgetexpps	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vgetexpps	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vgetexpps	(%rcx), %xmm30	 # AVX512{F,VL}
	vgetexpps	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vgetexpps	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vgetexpps	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vgetexpps	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vgetexpps	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vgetexpps	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vgetexpps	508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vgetexpps	512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vgetexpps	-512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vgetexpps	-516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vgetexpps	%ymm29, %ymm30	 # AVX512{F,VL}
	vgetexpps	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vgetexpps	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vgetexpps	(%rcx), %ymm30	 # AVX512{F,VL}
	vgetexpps	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vgetexpps	(%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vgetexpps	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vgetexpps	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vgetexpps	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vgetexpps	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vgetexpps	508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vgetexpps	512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vgetexpps	-512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vgetexpps	-516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vgetmantpd	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vgetmantpd	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vgetmantpd	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vgetmantpd	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vgetmantpd	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vgetmantpd	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vgetmantpd	$123, (%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vgetmantpd	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vgetmantpd	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vgetmantpd	$123, 1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, 1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vgetmantpd	$123, -1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, -1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vgetmantpd	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vgetmantpd	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vgetmantpd	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vgetmantpd	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vgetmantpd	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vgetmantpd	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vgetmantpd	$123, (%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vgetmantpd	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vgetmantpd	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vgetmantpd	$123, 1016(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, 1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vgetmantpd	$123, -1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, -1032(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vgetmantps	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vgetmantps	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vgetmantps	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vgetmantps	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vgetmantps	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vgetmantps	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vgetmantps	$123, (%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vgetmantps	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vgetmantps	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vgetmantps	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vgetmantps	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vgetmantps	$123, 508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vgetmantps	$123, 512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vgetmantps	$123, -512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vgetmantps	$123, -516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vgetmantps	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vgetmantps	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vgetmantps	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vgetmantps	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vgetmantps	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vgetmantps	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vgetmantps	$123, (%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vgetmantps	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vgetmantps	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vgetmantps	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vgetmantps	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vgetmantps	$123, 508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vgetmantps	$123, 512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vgetmantps	$123, -512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vgetmantps	$123, -516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vinsertf32x4	$0xab, %xmm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vinsertf32x4	$0xab, %xmm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vinsertf32x4	$0xab, %xmm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vinsertf32x4	$123, %xmm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vinsertf32x4	$123, (%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vinsertf32x4	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vinsertf32x4	$123, 2032(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vinsertf32x4	$123, 2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vinsertf32x4	$123, -2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vinsertf32x4	$123, -2064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vinserti32x4	$0xab, %xmm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vinserti32x4	$0xab, %xmm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vinserti32x4	$0xab, %xmm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vinserti32x4	$123, %xmm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vinserti32x4	$123, (%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vinserti32x4	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vinserti32x4	$123, 2032(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vinserti32x4	$123, 2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vinserti32x4	$123, -2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vinserti32x4	$123, -2064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxpd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxpd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vmaxpd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vmaxpd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxpd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxpd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxpd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmaxpd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxpd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmaxpd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxpd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmaxpd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxpd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmaxpd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxpd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vmaxpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vmaxpd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmaxpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmaxpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmaxpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmaxpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vmaxps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vmaxps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmaxps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmaxps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmaxps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmaxps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vmaxps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vmaxps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vmaxps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmaxps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmaxps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmaxps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vmaxps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmaxps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vminpd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vminpd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vminpd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vminpd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vminpd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vminpd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vminpd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vminpd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vminpd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vminpd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vminpd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vminpd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vminpd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vminpd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vminpd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vminpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vminpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vminpd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vminpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vminpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vminpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vminpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vminpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vminpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vminpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vminpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vminpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vminpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vminps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vminps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vminps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vminps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vminps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vminps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vminps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vminps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vminps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vminps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vminps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vminps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vminps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vminps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vminps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vminps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vminps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vminps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vminps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vminps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vminps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vminps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vminps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vminps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vminps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vminps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vminps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vminps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vmovapd	%xmm29, %xmm30	 # AVX512{F,VL}
	vmovapd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vmovapd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vmovapd	(%rcx), %xmm30	 # AVX512{F,VL}
	vmovapd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vmovapd	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovapd	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vmovapd	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovapd	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vmovapd	%ymm29, %ymm30	 # AVX512{F,VL}
	vmovapd	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vmovapd	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vmovapd	(%rcx), %ymm30	 # AVX512{F,VL}
	vmovapd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vmovapd	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovapd	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vmovapd	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovapd	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vmovaps	%xmm29, %xmm30	 # AVX512{F,VL}
	vmovaps	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vmovaps	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vmovaps	(%rcx), %xmm30	 # AVX512{F,VL}
	vmovaps	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vmovaps	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovaps	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vmovaps	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovaps	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vmovaps	%ymm29, %ymm30	 # AVX512{F,VL}
	vmovaps	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vmovaps	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vmovaps	(%rcx), %ymm30	 # AVX512{F,VL}
	vmovaps	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vmovaps	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovaps	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vmovaps	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovaps	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vmovddup	%xmm29, %xmm30	 # AVX512{F,VL}
	vmovddup	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vmovddup	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vmovddup	(%rcx), %xmm30	 # AVX512{F,VL}
	vmovddup	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vmovddup	1016(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovddup	1024(%rdx), %xmm30	 # AVX512{F,VL}
	vmovddup	-1024(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovddup	-1032(%rdx), %xmm30	 # AVX512{F,VL}
	vmovddup	%ymm29, %ymm30	 # AVX512{F,VL}
	vmovddup	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vmovddup	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vmovddup	(%rcx), %ymm30	 # AVX512{F,VL}
	vmovddup	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vmovddup	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovddup	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vmovddup	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovddup	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vmovdqa32	%xmm29, %xmm30	 # AVX512{F,VL}
	vmovdqa32	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vmovdqa32	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vmovdqa32	(%rcx), %xmm30	 # AVX512{F,VL}
	vmovdqa32	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vmovdqa32	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovdqa32	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vmovdqa32	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovdqa32	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vmovdqa32	%ymm29, %ymm30	 # AVX512{F,VL}
	vmovdqa32	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vmovdqa32	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vmovdqa32	(%rcx), %ymm30	 # AVX512{F,VL}
	vmovdqa32	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vmovdqa32	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovdqa32	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vmovdqa32	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovdqa32	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vmovdqa64	%xmm29, %xmm30	 # AVX512{F,VL}
	vmovdqa64	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vmovdqa64	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vmovdqa64	(%rcx), %xmm30	 # AVX512{F,VL}
	vmovdqa64	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vmovdqa64	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovdqa64	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vmovdqa64	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovdqa64	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vmovdqa64	%ymm29, %ymm30	 # AVX512{F,VL}
	vmovdqa64	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vmovdqa64	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vmovdqa64	(%rcx), %ymm30	 # AVX512{F,VL}
	vmovdqa64	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vmovdqa64	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovdqa64	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vmovdqa64	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovdqa64	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vmovdqu32	%xmm29, %xmm30	 # AVX512{F,VL}
	vmovdqu32	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vmovdqu32	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vmovdqu32	(%rcx), %xmm30	 # AVX512{F,VL}
	vmovdqu32	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vmovdqu32	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovdqu32	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vmovdqu32	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovdqu32	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vmovdqu32	%ymm29, %ymm30	 # AVX512{F,VL}
	vmovdqu32	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vmovdqu32	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vmovdqu32	(%rcx), %ymm30	 # AVX512{F,VL}
	vmovdqu32	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vmovdqu32	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovdqu32	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vmovdqu32	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovdqu32	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vmovdqu64	%xmm29, %xmm30	 # AVX512{F,VL}
	vmovdqu64	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vmovdqu64	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vmovdqu64	(%rcx), %xmm30	 # AVX512{F,VL}
	vmovdqu64	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vmovdqu64	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovdqu64	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vmovdqu64	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovdqu64	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vmovdqu64	%ymm29, %ymm30	 # AVX512{F,VL}
	vmovdqu64	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vmovdqu64	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vmovdqu64	(%rcx), %ymm30	 # AVX512{F,VL}
	vmovdqu64	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vmovdqu64	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovdqu64	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vmovdqu64	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovdqu64	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vmovntdq	%xmm30, (%rcx)	 # AVX512{F,VL}
	vmovntdq	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovntdq	%xmm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vmovntdq	%xmm30, 2048(%rdx)	 # AVX512{F,VL}
	vmovntdq	%xmm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vmovntdq	%xmm30, -2064(%rdx)	 # AVX512{F,VL}
	vmovntdq	%ymm30, (%rcx)	 # AVX512{F,VL}
	vmovntdq	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovntdq	%ymm30, 4064(%rdx)	 # AVX512{F,VL} Disp8
	vmovntdq	%ymm30, 4096(%rdx)	 # AVX512{F,VL}
	vmovntdq	%ymm30, -4096(%rdx)	 # AVX512{F,VL} Disp8
	vmovntdq	%ymm30, -4128(%rdx)	 # AVX512{F,VL}
	vmovntdqa	(%rcx), %xmm30	 # AVX512{F,VL}
	vmovntdqa	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vmovntdqa	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovntdqa	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vmovntdqa	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovntdqa	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vmovntdqa	(%rcx), %ymm30	 # AVX512{F,VL}
	vmovntdqa	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vmovntdqa	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovntdqa	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vmovntdqa	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovntdqa	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vmovntpd	%xmm30, (%rcx)	 # AVX512{F,VL}
	vmovntpd	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovntpd	%xmm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vmovntpd	%xmm30, 2048(%rdx)	 # AVX512{F,VL}
	vmovntpd	%xmm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vmovntpd	%xmm30, -2064(%rdx)	 # AVX512{F,VL}
	vmovntpd	%ymm30, (%rcx)	 # AVX512{F,VL}
	vmovntpd	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovntpd	%ymm30, 4064(%rdx)	 # AVX512{F,VL} Disp8
	vmovntpd	%ymm30, 4096(%rdx)	 # AVX512{F,VL}
	vmovntpd	%ymm30, -4096(%rdx)	 # AVX512{F,VL} Disp8
	vmovntpd	%ymm30, -4128(%rdx)	 # AVX512{F,VL}
	vmovntps	%xmm30, (%rcx)	 # AVX512{F,VL}
	vmovntps	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovntps	%xmm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vmovntps	%xmm30, 2048(%rdx)	 # AVX512{F,VL}
	vmovntps	%xmm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vmovntps	%xmm30, -2064(%rdx)	 # AVX512{F,VL}
	vmovntps	%ymm30, (%rcx)	 # AVX512{F,VL}
	vmovntps	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovntps	%ymm30, 4064(%rdx)	 # AVX512{F,VL} Disp8
	vmovntps	%ymm30, 4096(%rdx)	 # AVX512{F,VL}
	vmovntps	%ymm30, -4096(%rdx)	 # AVX512{F,VL} Disp8
	vmovntps	%ymm30, -4128(%rdx)	 # AVX512{F,VL}
	vmovshdup	%xmm29, %xmm30	 # AVX512{F,VL}
	vmovshdup	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vmovshdup	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vmovshdup	(%rcx), %xmm30	 # AVX512{F,VL}
	vmovshdup	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vmovshdup	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovshdup	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vmovshdup	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovshdup	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vmovshdup	%ymm29, %ymm30	 # AVX512{F,VL}
	vmovshdup	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vmovshdup	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vmovshdup	(%rcx), %ymm30	 # AVX512{F,VL}
	vmovshdup	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vmovshdup	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovshdup	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vmovshdup	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovshdup	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vmovsldup	%xmm29, %xmm30	 # AVX512{F,VL}
	vmovsldup	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vmovsldup	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vmovsldup	(%rcx), %xmm30	 # AVX512{F,VL}
	vmovsldup	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vmovsldup	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovsldup	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vmovsldup	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovsldup	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vmovsldup	%ymm29, %ymm30	 # AVX512{F,VL}
	vmovsldup	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vmovsldup	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vmovsldup	(%rcx), %ymm30	 # AVX512{F,VL}
	vmovsldup	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vmovsldup	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovsldup	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vmovsldup	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovsldup	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vmovupd	%xmm29, %xmm30	 # AVX512{F,VL}
	vmovupd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vmovupd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vmovupd	(%rcx), %xmm30	 # AVX512{F,VL}
	vmovupd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vmovupd	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovupd	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vmovupd	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovupd	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vmovupd	%ymm29, %ymm30	 # AVX512{F,VL}
	vmovupd	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vmovupd	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vmovupd	(%rcx), %ymm30	 # AVX512{F,VL}
	vmovupd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vmovupd	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovupd	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vmovupd	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovupd	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vmovups	%xmm29, %xmm30	 # AVX512{F,VL}
	vmovups	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vmovups	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vmovups	(%rcx), %xmm30	 # AVX512{F,VL}
	vmovups	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vmovups	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovups	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vmovups	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vmovups	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vmovups	%ymm29, %ymm30	 # AVX512{F,VL}
	vmovups	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vmovups	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vmovups	(%rcx), %ymm30	 # AVX512{F,VL}
	vmovups	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vmovups	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovups	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vmovups	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vmovups	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vmulpd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vmulpd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vmulpd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vmulpd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vmulpd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vmulpd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vmulpd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmulpd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vmulpd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmulpd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vmulpd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmulpd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vmulpd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmulpd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vmulpd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vmulpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vmulpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vmulpd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vmulpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vmulpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vmulpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmulpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vmulpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmulpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vmulpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmulpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vmulpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmulpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vmulps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vmulps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vmulps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vmulps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vmulps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vmulps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vmulps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmulps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vmulps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmulps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vmulps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmulps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vmulps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vmulps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vmulps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vmulps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vmulps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vmulps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vmulps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vmulps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vmulps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmulps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vmulps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmulps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vmulps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmulps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vmulps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vmulps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpabsd	%xmm29, %xmm30	 # AVX512{F,VL}
	vpabsd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpabsd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpabsd	(%rcx), %xmm30	 # AVX512{F,VL}
	vpabsd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpabsd	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vpabsd	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpabsd	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vpabsd	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpabsd	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vpabsd	508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vpabsd	512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vpabsd	-512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vpabsd	-516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vpabsd	%ymm29, %ymm30	 # AVX512{F,VL}
	vpabsd	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpabsd	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpabsd	(%rcx), %ymm30	 # AVX512{F,VL}
	vpabsd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpabsd	(%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vpabsd	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpabsd	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vpabsd	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpabsd	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vpabsd	508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vpabsd	512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vpabsd	-512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vpabsd	-516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vpabsq	%xmm29, %xmm30	 # AVX512{F,VL}
	vpabsq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpabsq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpabsq	(%rcx), %xmm30	 # AVX512{F,VL}
	vpabsq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpabsq	(%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vpabsq	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpabsq	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vpabsq	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpabsq	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vpabsq	1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vpabsq	1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vpabsq	-1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vpabsq	-1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vpabsq	%ymm29, %ymm30	 # AVX512{F,VL}
	vpabsq	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpabsq	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpabsq	(%rcx), %ymm30	 # AVX512{F,VL}
	vpabsq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpabsq	(%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vpabsq	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpabsq	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vpabsq	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpabsq	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vpabsq	1016(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vpabsq	1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vpabsq	-1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vpabsq	-1032(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vpaddd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpaddd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpaddd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddd	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpaddd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpaddd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddd	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpaddd	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddd	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpaddd	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpaddd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpaddd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpaddd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpaddd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpaddd	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpaddd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpaddd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpaddd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpaddd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpaddd	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpaddd	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpaddd	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpaddd	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpaddq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpaddq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpaddq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpaddq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpaddq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpaddq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpaddq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpaddq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpaddq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpaddq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpaddq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpaddq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpaddq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpaddq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpaddq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpaddq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpaddq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpaddq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpaddq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpaddq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpaddq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpandd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpandd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandd	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandd	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandd	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandd	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandd	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpandd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpandd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandd	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandd	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandd	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandd	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandd	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpandnd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpandnd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnd	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandnd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandnd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnd	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandnd	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnd	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandnd	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpandnd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpandnd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnd	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandnd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandnd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnd	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandnd	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnd	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandnd	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpandnq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpandnq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandnq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandnq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandnq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandnq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandnq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpandnq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpandnq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandnq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandnq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandnq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandnq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandnq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpandq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpandq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpandq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpandq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpandq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpandq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpandq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpandq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpandq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpandq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpblendmd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpblendmd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpblendmd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmd	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpblendmd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpblendmd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmd	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpblendmd	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmd	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpblendmd	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpblendmd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpblendmd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpblendmd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpblendmd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpblendmd	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpblendmd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpblendmd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpblendmd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpblendmd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpblendmd	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpblendmd	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpblendmd	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpblendmd	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpbroadcastd	(%rcx), %xmm30	 # AVX512{F,VL}
	vpbroadcastd	(%rcx), %xmm30{%k7}	 # AVX512{F,VL}
	vpbroadcastd	(%rcx), %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpbroadcastd	508(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpbroadcastd	512(%rdx), %xmm30	 # AVX512{F,VL}
	vpbroadcastd	-512(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpbroadcastd	-516(%rdx), %xmm30	 # AVX512{F,VL}
	vpbroadcastd	(%rcx), %ymm30	 # AVX512{F,VL}
	vpbroadcastd	(%rcx), %ymm30{%k7}	 # AVX512{F,VL}
	vpbroadcastd	(%rcx), %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpbroadcastd	508(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpbroadcastd	512(%rdx), %ymm30	 # AVX512{F,VL}
	vpbroadcastd	-512(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpbroadcastd	-516(%rdx), %ymm30	 # AVX512{F,VL}
	vpbroadcastd	%xmm29, %xmm30	 # AVX512{F,VL}
	vpbroadcastd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpbroadcastd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastd	%xmm29, %ymm30	 # AVX512{F,VL}
	vpbroadcastd	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpbroadcastd	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastd	%eax, %xmm30	 # AVX512{F,VL}
	vpbroadcastd	%eax, %xmm30{%k7}	 # AVX512{F,VL}
	vpbroadcastd	%eax, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastd	%ebp, %xmm30	 # AVX512{F,VL}
	vpbroadcastd	%r13d, %xmm30	 # AVX512{F,VL}
	vpbroadcastd	%eax, %ymm30	 # AVX512{F,VL}
	vpbroadcastd	%eax, %ymm30{%k7}	 # AVX512{F,VL}
	vpbroadcastd	%eax, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastd	%ebp, %ymm30	 # AVX512{F,VL}
	vpbroadcastd	%r13d, %ymm30	 # AVX512{F,VL}
	vpbroadcastq	(%rcx), %xmm30	 # AVX512{F,VL}
	vpbroadcastq	(%rcx), %xmm30{%k7}	 # AVX512{F,VL}
	vpbroadcastq	(%rcx), %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpbroadcastq	1016(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpbroadcastq	1024(%rdx), %xmm30	 # AVX512{F,VL}
	vpbroadcastq	-1024(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpbroadcastq	-1032(%rdx), %xmm30	 # AVX512{F,VL}
	vpbroadcastq	(%rcx), %ymm30	 # AVX512{F,VL}
	vpbroadcastq	(%rcx), %ymm30{%k7}	 # AVX512{F,VL}
	vpbroadcastq	(%rcx), %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpbroadcastq	1016(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpbroadcastq	1024(%rdx), %ymm30	 # AVX512{F,VL}
	vpbroadcastq	-1024(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpbroadcastq	-1032(%rdx), %ymm30	 # AVX512{F,VL}
	vpbroadcastq	%xmm29, %xmm30	 # AVX512{F,VL}
	vpbroadcastq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpbroadcastq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastq	%xmm29, %ymm30	 # AVX512{F,VL}
	vpbroadcastq	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpbroadcastq	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastq	%rax, %xmm30	 # AVX512{F,VL}
	vpbroadcastq	%rax, %xmm30{%k7}	 # AVX512{F,VL}
	vpbroadcastq	%rax, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastq	%r8, %xmm30	 # AVX512{F,VL}
	vpbroadcastq	%rax, %ymm30	 # AVX512{F,VL}
	vpbroadcastq	%rax, %ymm30{%k7}	 # AVX512{F,VL}
	vpbroadcastq	%rax, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastq	%r8, %ymm30	 # AVX512{F,VL}
	vpcmpd	$0xab, %xmm29, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpd	$0xab, %xmm29, %xmm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, %xmm29, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpd	$123, (%rcx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpd	$123, 0x123(%rax,%r14,8), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpd	$123, (%rcx){1to4}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpd	$123, 2032(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpd	$123, 2048(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpd	$123, -2048(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpd	$123, -2064(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpd	$123, 508(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpd	$123, 512(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpd	$123, -512(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpd	$123, -516(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpd	$0xab, %ymm29, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpd	$0xab, %ymm29, %ymm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, %ymm29, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpd	$123, (%rcx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpd	$123, 0x123(%rax,%r14,8), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpd	$123, (%rcx){1to8}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpd	$123, 4064(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpd	$123, 4096(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpd	$123, -4096(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpd	$123, -4128(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpd	$123, 508(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpd	$123, 512(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpd	$123, -512(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpd	$123, -516(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	%xmm29, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	%xmm29, %xmm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	(%rcx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	0x123(%rax,%r14,8), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	(%rcx){1to4}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	2032(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqd	2048(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	-2048(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqd	-2064(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	508(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqd	512(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	-512(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqd	-516(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	%ymm29, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	%ymm29, %ymm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	(%rcx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	0x123(%rax,%r14,8), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	(%rcx){1to8}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	4064(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqd	4096(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	-4096(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqd	-4128(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	508(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqd	512(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqd	-512(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqd	-516(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	%xmm29, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	%xmm29, %xmm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	(%rcx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	0x123(%rax,%r14,8), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	(%rcx){1to2}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	2032(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqq	2048(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	-2048(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqq	-2064(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	1016(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqq	1024(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	-1024(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqq	-1032(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	%ymm29, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	%ymm29, %ymm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	(%rcx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	0x123(%rax,%r14,8), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	(%rcx){1to4}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	4064(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqq	4096(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	-4096(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqq	-4128(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	1016(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqq	1024(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpeqq	-1024(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpeqq	-1032(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	%xmm29, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	%xmm29, %xmm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	(%rcx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	0x123(%rax,%r14,8), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	(%rcx){1to4}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	2032(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtd	2048(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	-2048(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtd	-2064(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	508(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtd	512(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	-512(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtd	-516(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	%ymm29, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	%ymm29, %ymm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	(%rcx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	0x123(%rax,%r14,8), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	(%rcx){1to8}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	4064(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtd	4096(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	-4096(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtd	-4128(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	508(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtd	512(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtd	-512(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtd	-516(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	%xmm29, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	%xmm29, %xmm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	(%rcx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	0x123(%rax,%r14,8), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	(%rcx){1to2}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	2032(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtq	2048(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	-2048(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtq	-2064(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	1016(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtq	1024(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	-1024(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtq	-1032(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	%ymm29, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	%ymm29, %ymm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	(%rcx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	0x123(%rax,%r14,8), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	(%rcx){1to4}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	4064(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtq	4096(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	-4096(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtq	-4128(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	1016(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtq	1024(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpgtq	-1024(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpgtq	-1032(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpq	$0xab, %xmm29, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpq	$0xab, %xmm29, %xmm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, %xmm29, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpq	$123, (%rcx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpq	$123, 0x123(%rax,%r14,8), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpq	$123, (%rcx){1to2}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpq	$123, 2032(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpq	$123, 2048(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpq	$123, -2048(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpq	$123, -2064(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpq	$123, 1016(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpq	$123, 1024(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpq	$123, -1024(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpq	$123, -1032(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpq	$0xab, %ymm29, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpq	$0xab, %ymm29, %ymm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, %ymm29, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpq	$123, (%rcx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpq	$123, 0x123(%rax,%r14,8), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpq	$123, (%rcx){1to4}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpq	$123, 4064(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpq	$123, 4096(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpq	$123, -4096(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpq	$123, -4128(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpq	$123, 1016(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpq	$123, 1024(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpq	$123, -1024(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpq	$123, -1032(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpud	$0xab, %xmm29, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpud	$0xab, %xmm29, %xmm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, %xmm29, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpud	$123, (%rcx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpud	$123, 0x123(%rax,%r14,8), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpud	$123, (%rcx){1to4}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpud	$123, 2032(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpud	$123, 2048(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpud	$123, -2048(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpud	$123, -2064(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpud	$123, 508(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpud	$123, 512(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpud	$123, -512(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpud	$123, -516(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpud	$0xab, %ymm29, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpud	$0xab, %ymm29, %ymm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, %ymm29, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpud	$123, (%rcx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpud	$123, 0x123(%rax,%r14,8), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpud	$123, (%rcx){1to8}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpud	$123, 4064(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpud	$123, 4096(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpud	$123, -4096(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpud	$123, -4128(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpud	$123, 508(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpud	$123, 512(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpud	$123, -512(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpud	$123, -516(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$0xab, %xmm29, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$0xab, %xmm29, %xmm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, %xmm29, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$123, (%rcx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$123, 0x123(%rax,%r14,8), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$123, (%rcx){1to2}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$123, 2032(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, 2048(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$123, -2048(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, -2064(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$123, 1016(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, 1024(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$123, -1024(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, -1032(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$0xab, %ymm29, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$0xab, %ymm29, %ymm30, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, %ymm29, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$123, (%rcx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$123, 0x123(%rax,%r14,8), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$123, (%rcx){1to4}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$123, 4064(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, 4096(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$123, -4096(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, -4128(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$123, 1016(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, 1024(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL}
	vpcmpuq	$123, -1024(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, -1032(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL}
	vpblendmq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpblendmq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpblendmq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpblendmq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpblendmq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpblendmq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpblendmq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpblendmq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpblendmq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpblendmq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpblendmq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpblendmq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpblendmq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpblendmq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpblendmq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpblendmq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpblendmq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpblendmq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpblendmq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpblendmq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpblendmq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpcompressd	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpcompressd	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpcompressd	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpcompressd	%xmm30, 508(%rdx)	 # AVX512{F,VL} Disp8
	vpcompressd	%xmm30, 512(%rdx)	 # AVX512{F,VL}
	vpcompressd	%xmm30, -512(%rdx)	 # AVX512{F,VL} Disp8
	vpcompressd	%xmm30, -516(%rdx)	 # AVX512{F,VL}
	vpcompressd	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpcompressd	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpcompressd	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpcompressd	%ymm30, 508(%rdx)	 # AVX512{F,VL} Disp8
	vpcompressd	%ymm30, 512(%rdx)	 # AVX512{F,VL}
	vpcompressd	%ymm30, -512(%rdx)	 # AVX512{F,VL} Disp8
	vpcompressd	%ymm30, -516(%rdx)	 # AVX512{F,VL}
	vpcompressd	%xmm29, %xmm30	 # AVX512{F,VL}
	vpcompressd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpcompressd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpcompressd	%ymm29, %ymm30	 # AVX512{F,VL}
	vpcompressd	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpcompressd	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermd	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermd	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermd	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermd	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermd	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilpd	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilpd	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpermilpd	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpermilpd	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilpd	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vpermilpd	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpermilpd	$123, (%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vpermilpd	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpermilpd	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vpermilpd	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpermilpd	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vpermilpd	$123, 1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vpermilpd	$123, 1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vpermilpd	$123, -1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vpermilpd	$123, -1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vpermilpd	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilpd	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermilpd	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermilpd	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilpd	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vpermilpd	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpermilpd	$123, (%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vpermilpd	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpermilpd	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vpermilpd	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpermilpd	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vpermilpd	$123, 1016(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vpermilpd	$123, 1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vpermilpd	$123, -1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vpermilpd	$123, -1032(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vpermilpd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilpd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpermilpd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpermilpd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilpd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilpd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilpd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermilpd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilpd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermilpd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilpd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermilpd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilpd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermilpd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilpd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermilpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermilpd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermilpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermilpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermilpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermilpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilps	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilps	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpermilps	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpermilps	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilps	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vpermilps	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpermilps	$123, (%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vpermilps	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpermilps	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vpermilps	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpermilps	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vpermilps	$123, 508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vpermilps	$123, 512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vpermilps	$123, -512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vpermilps	$123, -516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vpermilps	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilps	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermilps	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermilps	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilps	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vpermilps	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpermilps	$123, (%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vpermilps	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpermilps	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vpermilps	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpermilps	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vpermilps	$123, 508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vpermilps	$123, 512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vpermilps	$123, -512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vpermilps	$123, -516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vpermilps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpermilps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpermilps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermilps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermilps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermilps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermilps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermilps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermilps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermilps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermilps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermilps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermilps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermilps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermilps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermpd	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermpd	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermpd	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermpd	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermpd	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vpermpd	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpermpd	$123, (%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vpermpd	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpermpd	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vpermpd	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpermpd	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vpermpd	$123, 1016(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vpermpd	$123, 1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vpermpd	$123, -1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vpermpd	$123, -1032(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vpermps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermq	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermq	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermq	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermq	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermq	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vpermq	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpermq	$123, (%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vpermq	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpermq	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vpermq	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpermq	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vpermq	$123, 1016(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vpermq	$123, 1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vpermq	$123, -1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vpermq	$123, -1032(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vpexpandd	(%rcx), %xmm30	 # AVX512{F,VL}
	vpexpandd	(%rcx), %xmm30{%k7}	 # AVX512{F,VL}
	vpexpandd	(%rcx), %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpexpandd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpexpandd	508(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpexpandd	512(%rdx), %xmm30	 # AVX512{F,VL}
	vpexpandd	-512(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpexpandd	-516(%rdx), %xmm30	 # AVX512{F,VL}
	vpexpandd	(%rcx), %ymm30	 # AVX512{F,VL}
	vpexpandd	(%rcx), %ymm30{%k7}	 # AVX512{F,VL}
	vpexpandd	(%rcx), %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpexpandd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpexpandd	508(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpexpandd	512(%rdx), %ymm30	 # AVX512{F,VL}
	vpexpandd	-512(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpexpandd	-516(%rdx), %ymm30	 # AVX512{F,VL}
	vpexpandd	%xmm29, %xmm30	 # AVX512{F,VL}
	vpexpandd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpexpandd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpexpandd	%ymm29, %ymm30	 # AVX512{F,VL}
	vpexpandd	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpexpandd	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpexpandq	(%rcx), %xmm30	 # AVX512{F,VL}
	vpexpandq	(%rcx), %xmm30{%k7}	 # AVX512{F,VL}
	vpexpandq	(%rcx), %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpexpandq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpexpandq	1016(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpexpandq	1024(%rdx), %xmm30	 # AVX512{F,VL}
	vpexpandq	-1024(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpexpandq	-1032(%rdx), %xmm30	 # AVX512{F,VL}
	vpexpandq	(%rcx), %ymm30	 # AVX512{F,VL}
	vpexpandq	(%rcx), %ymm30{%k7}	 # AVX512{F,VL}
	vpexpandq	(%rcx), %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpexpandq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpexpandq	1016(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpexpandq	1024(%rdx), %ymm30	 # AVX512{F,VL}
	vpexpandq	-1024(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpexpandq	-1032(%rdx), %ymm30	 # AVX512{F,VL}
	vpexpandq	%xmm29, %xmm30	 # AVX512{F,VL}
	vpexpandq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpexpandq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpexpandq	%ymm29, %ymm30	 # AVX512{F,VL}
	vpexpandq	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpexpandq	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpgatherdd	123(%r14,%xmm31,8), %xmm30{%k1}	 # AVX512{F,VL}
	vpgatherdd	256(%r9,%xmm31), %xmm30{%k1}	 # AVX512{F,VL}
	vpgatherdd	1024(%rcx,%xmm31,4), %xmm30{%k1}	 # AVX512{F,VL}
	vpgatherdd	123(%r14,%ymm31,8), %ymm30{%k1}	 # AVX512{F,VL}
	vpgatherdd	256(%r9,%ymm31), %ymm30{%k1}	 # AVX512{F,VL}
	vpgatherdd	1024(%rcx,%ymm31,4), %ymm30{%k1}	 # AVX512{F,VL}
	vpgatherdq	123(%r14,%xmm31,8), %xmm30{%k1}	 # AVX512{F,VL}
	vpgatherdq	256(%r9,%xmm31), %xmm30{%k1}	 # AVX512{F,VL}
	vpgatherdq	1024(%rcx,%xmm31,4), %xmm30{%k1}	 # AVX512{F,VL}
	vpgatherdq	123(%r14,%xmm31,8), %ymm30{%k1}	 # AVX512{F,VL}
	vpgatherdq	256(%r9,%xmm31), %ymm30{%k1}	 # AVX512{F,VL}
	vpgatherdq	1024(%rcx,%xmm31,4), %ymm30{%k1}	 # AVX512{F,VL}
	vpgatherqd	123(%r14,%xmm31,8), %xmm30{%k1}	 # AVX512{F,VL}
	vpgatherqd	256(%r9,%xmm31), %xmm30{%k1}	 # AVX512{F,VL}
	vpgatherqd	1024(%rcx,%xmm31,4), %xmm30{%k1}	 # AVX512{F,VL}
	vpgatherqd	123(%r14,%ymm31,8), %xmm30{%k1}	 # AVX512{F,VL}
	vpgatherqd	256(%r9,%ymm31), %xmm30{%k1}	 # AVX512{F,VL}
	vpgatherqd	1024(%rcx,%ymm31,4), %xmm30{%k1}	 # AVX512{F,VL}
	vpgatherqq	123(%r14,%xmm31,8), %xmm30{%k1}	 # AVX512{F,VL}
	vpgatherqq	256(%r9,%xmm31), %xmm30{%k1}	 # AVX512{F,VL}
	vpgatherqq	1024(%rcx,%xmm31,4), %xmm30{%k1}	 # AVX512{F,VL}
	vpgatherqq	123(%r14,%ymm31,8), %ymm30{%k1}	 # AVX512{F,VL}
	vpgatherqq	256(%r9,%ymm31), %ymm30{%k1}	 # AVX512{F,VL}
	vpgatherqq	1024(%rcx,%ymm31,4), %ymm30{%k1}	 # AVX512{F,VL}
	vpmaxsd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmaxsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmaxsd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsd	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxsd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxsd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsd	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxsd	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsd	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxsd	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxsd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmaxsd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmaxsd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxsd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxsd	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxsd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxsd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxsd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxsd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxsd	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxsd	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxsd	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxsd	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxsq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmaxsq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmaxsq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxsq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxsq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxsq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxsq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxsq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxsq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmaxsq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmaxsq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxsq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxsq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxsq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxsq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxsq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxsq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxsq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxsq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxsq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxsq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxud	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxud	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmaxud	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmaxud	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxud	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxud	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxud	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxud	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxud	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxud	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxud	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxud	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxud	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxud	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxud	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxud	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmaxud	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmaxud	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxud	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxud	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxud	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxud	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxud	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxud	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxud	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxud	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxud	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxud	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxuq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxuq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmaxuq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmaxuq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxuq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxuq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxuq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxuq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxuq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxuq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxuq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxuq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxuq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmaxuq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmaxuq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxuq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmaxuq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmaxuq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxuq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxuq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxuq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxuq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxuq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxuq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxuq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxuq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmaxuq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmaxuq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpminsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpminsd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsd	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminsd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminsd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsd	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminsd	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsd	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminsd	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpminsd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpminsd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsd	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminsd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminsd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsd	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminsd	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsd	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminsd	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpminsq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpminsq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminsq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminsq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminsq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminsq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminsq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpminsq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpminsq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminsq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminsq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminsq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminsq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminsq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminud	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminud	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpminud	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpminud	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminud	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminud	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminud	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminud	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminud	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminud	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminud	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminud	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminud	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminud	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminud	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminud	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpminud	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpminud	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminud	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminud	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminud	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminud	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminud	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminud	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminud	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminud	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminud	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminud	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminuq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminuq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpminuq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpminuq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminuq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminuq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminuq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminuq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminuq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminuq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpminuq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminuq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminuq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpminuq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpminuq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminuq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpminuq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpminuq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminuq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminuq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminuq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminuq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminuq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminuq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpminuq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminuq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpminuq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpminuq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmovsxbd	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovsxbd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsxbd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbd	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovsxbd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovsxbd	508(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxbd	512(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxbd	-512(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxbd	-516(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxbd	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovsxbd	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovsxbd	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbd	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovsxbd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovsxbd	1016(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxbd	1024(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxbd	-1024(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxbd	-1032(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxbq	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovsxbq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsxbq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbq	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovsxbq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovsxbq	254(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxbq	256(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxbq	-256(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxbq	-258(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxbq	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovsxbq	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovsxbq	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbq	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovsxbq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovsxbq	508(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxbq	512(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxbq	-512(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxbq	-516(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxdq	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovsxdq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsxdq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxdq	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovsxdq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovsxdq	1016(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxdq	1024(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxdq	-1024(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxdq	-1032(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxdq	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovsxdq	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovsxdq	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxdq	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovsxdq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovsxdq	2032(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxdq	2048(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxdq	-2048(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxdq	-2064(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxwd	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovsxwd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsxwd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwd	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovsxwd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovsxwd	1016(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxwd	1024(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxwd	-1024(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxwd	-1032(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxwd	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovsxwd	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovsxwd	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwd	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovsxwd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovsxwd	2032(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxwd	2048(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxwd	-2048(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxwd	-2064(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxwq	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovsxwq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsxwq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwq	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovsxwq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovsxwq	508(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxwq	512(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxwq	-512(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovsxwq	-516(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovsxwq	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovsxwq	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovsxwq	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwq	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovsxwq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovsxwq	1016(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxwq	1024(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovsxwq	-1024(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovsxwq	-1032(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxbd	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovzxbd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovzxbd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbd	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovzxbd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovzxbd	508(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxbd	512(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxbd	-512(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxbd	-516(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxbd	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovzxbd	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovzxbd	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbd	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovzxbd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovzxbd	1016(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxbd	1024(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxbd	-1024(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxbd	-1032(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxbq	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovzxbq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovzxbq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbq	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovzxbq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovzxbq	254(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxbq	256(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxbq	-256(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxbq	-258(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxbq	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovzxbq	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovzxbq	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbq	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovzxbq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovzxbq	508(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxbq	512(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxbq	-512(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxbq	-516(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxdq	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovzxdq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovzxdq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxdq	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovzxdq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovzxdq	1016(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxdq	1024(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxdq	-1024(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxdq	-1032(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxdq	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovzxdq	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovzxdq	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxdq	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovzxdq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovzxdq	2032(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxdq	2048(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxdq	-2048(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxdq	-2064(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxwd	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovzxwd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovzxwd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwd	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovzxwd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovzxwd	1016(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxwd	1024(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxwd	-1024(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxwd	-1032(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxwd	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovzxwd	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovzxwd	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwd	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovzxwd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovzxwd	2032(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxwd	2048(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxwd	-2048(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxwd	-2064(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxwq	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovzxwq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovzxwq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwq	(%rcx), %xmm30	 # AVX512{F,VL}
	vpmovzxwq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpmovzxwq	508(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxwq	512(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxwq	-512(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpmovzxwq	-516(%rdx), %xmm30	 # AVX512{F,VL}
	vpmovzxwq	%xmm29, %ymm30	 # AVX512{F,VL}
	vpmovzxwq	%xmm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmovzxwq	%xmm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwq	(%rcx), %ymm30	 # AVX512{F,VL}
	vpmovzxwq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpmovzxwq	1016(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxwq	1024(%rdx), %ymm30	 # AVX512{F,VL}
	vpmovzxwq	-1024(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpmovzxwq	-1032(%rdx), %ymm30	 # AVX512{F,VL}
	vpmuldq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuldq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmuldq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmuldq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuldq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuldq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuldq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmuldq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuldq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmuldq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuldq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmuldq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuldq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmuldq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuldq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmuldq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmuldq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmuldq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmuldq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmuldq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmuldq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmuldq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmuldq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmuldq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmuldq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmuldq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmuldq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmuldq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmulld	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmulld	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmulld	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmulld	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmulld	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmulld	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmulld	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmulld	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmulld	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmulld	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmulld	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmulld	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmulld	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmulld	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmulld	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmulld	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmulld	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmulld	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmulld	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmulld	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmulld	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmulld	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmulld	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmulld	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmulld	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmulld	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmulld	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmulld	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmuludq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuludq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmuludq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmuludq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuludq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuludq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuludq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmuludq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuludq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmuludq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuludq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmuludq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuludq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpmuludq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpmuludq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmuludq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpmuludq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpmuludq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmuludq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmuludq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmuludq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmuludq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmuludq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmuludq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpmuludq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmuludq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmuludq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpmuludq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpord	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpord	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpord	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpord	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpord	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpord	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpord	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpord	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpord	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpord	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpord	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpord	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpord	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpord	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpord	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpord	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpord	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpord	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpord	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpord	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpord	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpord	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpord	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpord	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpord	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpord	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpord	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpord	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vporq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vporq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vporq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vporq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vporq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vporq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vporq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vporq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vporq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vporq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vporq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vporq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vporq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vporq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vporq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vporq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vporq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vporq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vporq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vporq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vporq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vporq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vporq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vporq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vporq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vporq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vporq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vporq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpscatterdd	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vpscatterdd	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vpscatterdd	%xmm30, 256(%r9,%xmm31){%k1}	 # AVX512{F,VL}
	vpscatterdd	%xmm30, 1024(%rcx,%xmm31,4){%k1}	 # AVX512{F,VL}
	vpscatterdd	%ymm30, 123(%r14,%ymm31,8){%k1}	 # AVX512{F,VL}
	vpscatterdd	%ymm30, 123(%r14,%ymm31,8){%k1}	 # AVX512{F,VL}
	vpscatterdd	%ymm30, 256(%r9,%ymm31){%k1}	 # AVX512{F,VL}
	vpscatterdd	%ymm30, 1024(%rcx,%ymm31,4){%k1}	 # AVX512{F,VL}
	vpscatterdq	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vpscatterdq	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vpscatterdq	%xmm30, 256(%r9,%xmm31){%k1}	 # AVX512{F,VL}
	vpscatterdq	%xmm30, 1024(%rcx,%xmm31,4){%k1}	 # AVX512{F,VL}
	vpscatterdq	%ymm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vpscatterdq	%ymm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vpscatterdq	%ymm30, 256(%r9,%xmm31){%k1}	 # AVX512{F,VL}
	vpscatterdq	%ymm30, 1024(%rcx,%xmm31,4){%k1}	 # AVX512{F,VL}
	vpscatterqd	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vpscatterqd	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vpscatterqd	%xmm30, 256(%r9,%xmm31){%k1}	 # AVX512{F,VL}
	vpscatterqd	%xmm30, 1024(%rcx,%xmm31,4){%k1}	 # AVX512{F,VL}
	vpscatterqd	%xmm30, 123(%r14,%ymm31,8){%k1}	 # AVX512{F,VL}
	vpscatterqd	%xmm30, 123(%r14,%ymm31,8){%k1}	 # AVX512{F,VL}
	vpscatterqd	%xmm30, 256(%r9,%ymm31){%k1}	 # AVX512{F,VL}
	vpscatterqd	%xmm30, 1024(%rcx,%ymm31,4){%k1}	 # AVX512{F,VL}
	vpscatterqq	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vpscatterqq	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vpscatterqq	%xmm30, 256(%r9,%xmm31){%k1}	 # AVX512{F,VL}
	vpscatterqq	%xmm30, 1024(%rcx,%xmm31,4){%k1}	 # AVX512{F,VL}
	vpscatterqq	%ymm30, 123(%r14,%ymm31,8){%k1}	 # AVX512{F,VL}
	vpscatterqq	%ymm30, 123(%r14,%ymm31,8){%k1}	 # AVX512{F,VL}
	vpscatterqq	%ymm30, 256(%r9,%ymm31){%k1}	 # AVX512{F,VL}
	vpscatterqq	%ymm30, 1024(%rcx,%ymm31,4){%k1}	 # AVX512{F,VL}
	vpshufd	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vpshufd	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpshufd	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpshufd	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vpshufd	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vpshufd	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpshufd	$123, (%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vpshufd	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpshufd	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vpshufd	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpshufd	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vpshufd	$123, 508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vpshufd	$123, 512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vpshufd	$123, -512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vpshufd	$123, -516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vpshufd	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vpshufd	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpshufd	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpshufd	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vpshufd	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vpshufd	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpshufd	$123, (%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vpshufd	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpshufd	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vpshufd	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpshufd	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vpshufd	$123, 508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vpshufd	$123, 512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vpshufd	$123, -512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vpshufd	$123, -516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vpslld	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpslld	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpslld	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpslld	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpslld	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpslld	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpslld	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpslld	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpslld	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpslld	%xmm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpslld	%xmm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpslld	%xmm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpslld	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpslld	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpslld	2032(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpslld	2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpslld	-2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpslld	-2064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsllq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsllq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsllq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsllq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllq	%xmm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllq	%xmm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsllq	%xmm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsllq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllq	2032(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsllq	2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllq	-2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsllq	-2064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsllvd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsllvd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvd	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsllvd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsllvd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvd	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsllvd	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvd	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsllvd	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsllvd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsllvd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvd	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsllvd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsllvd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvd	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsllvd	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvd	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsllvd	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsllvq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsllvq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsllvq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsllvq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsllvq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsllvq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllvq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsllvq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsllvq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsllvq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsllvq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsllvq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllvq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsllvq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrad	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrad	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsrad	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsrad	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrad	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrad	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsrad	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrad	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsrad	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrad	%xmm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrad	%xmm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsrad	%xmm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsrad	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrad	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrad	2032(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsrad	2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrad	-2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsrad	-2064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsraq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsraq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsraq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsraq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsraq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsraq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsraq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsraq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsraq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsraq	%xmm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsraq	%xmm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsraq	%xmm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsraq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsraq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsraq	2032(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsraq	2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsraq	-2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsraq	-2064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsravd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsravd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravd	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsravd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsravd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravd	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsravd	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravd	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsravd	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsravd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsravd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravd	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsravd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsravd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravd	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsravd	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravd	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsravd	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsravq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsravq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsravq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsravq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsravq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsravq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsravq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsravq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsravq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsravq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsravq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsravq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsravq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsravq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrld	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrld	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsrld	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsrld	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrld	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrld	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsrld	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrld	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsrld	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrld	%xmm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrld	%xmm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsrld	%xmm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsrld	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrld	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrld	2032(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsrld	2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrld	-2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsrld	-2064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsrlq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsrlq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsrlq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsrlq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlq	%xmm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlq	%xmm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsrlq	%xmm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsrlq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlq	2032(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsrlq	2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlq	-2048(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsrlq	-2064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsrlvd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsrlvd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvd	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsrlvd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsrlvd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvd	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsrlvd	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvd	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsrlvd	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsrlvd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsrlvd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvd	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsrlvd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsrlvd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvd	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsrlvd	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvd	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsrlvd	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsrlvq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsrlvq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsrlvq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsrlvq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsrlvq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsrlvq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlvq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsrlvq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsrlvq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsrlvq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsrlvq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsrlvq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlvq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsrlvq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrld	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrld	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsrld	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsrld	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrld	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vpsrld	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpsrld	$123, (%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vpsrld	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpsrld	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vpsrld	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpsrld	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vpsrld	$123, 508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vpsrld	$123, 512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vpsrld	$123, -512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vpsrld	$123, -516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vpsrld	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrld	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsrld	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsrld	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrld	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vpsrld	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpsrld	$123, (%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vpsrld	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpsrld	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vpsrld	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpsrld	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vpsrld	$123, 508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vpsrld	$123, 512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vpsrld	$123, -512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vpsrld	$123, -516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vpsrlq	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlq	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsrlq	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsrlq	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrlq	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vpsrlq	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpsrlq	$123, (%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vpsrlq	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpsrlq	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vpsrlq	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpsrlq	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vpsrlq	$123, 1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vpsrlq	$123, 1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vpsrlq	$123, -1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vpsrlq	$123, -1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vpsrlq	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlq	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsrlq	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsrlq	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrlq	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vpsrlq	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpsrlq	$123, (%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vpsrlq	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpsrlq	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vpsrlq	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpsrlq	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vpsrlq	$123, 1016(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vpsrlq	$123, 1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vpsrlq	$123, -1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vpsrlq	$123, -1032(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vpsubd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsubd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsubd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubd	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsubd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsubd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubd	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsubd	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubd	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsubd	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsubd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsubd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsubd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsubd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsubd	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsubd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsubd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsubd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsubd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsubd	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsubd	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsubd	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsubd	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsubq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsubq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsubq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsubq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsubq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsubq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpsubq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsubq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsubq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsubq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsubq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsubq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsubq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsubq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsubq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsubq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsubq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpsubq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsubq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsubq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpsubq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vptestmd	%xmm29, %xmm30, %k5	 # AVX512{F,VL}
	vptestmd	%xmm29, %xmm30, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	(%rcx), %xmm30, %k5	 # AVX512{F,VL}
	vptestmd	0x123(%rax,%r14,8), %xmm30, %k5	 # AVX512{F,VL}
	vptestmd	(%rcx){1to4}, %xmm30, %k5	 # AVX512{F,VL}
	vptestmd	2032(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vptestmd	2048(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vptestmd	-2048(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vptestmd	-2064(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vptestmd	508(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vptestmd	512(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL}
	vptestmd	-512(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vptestmd	-516(%rdx){1to4}, %xmm30, %k5	 # AVX512{F,VL}
	vptestmd	%ymm29, %ymm30, %k5	 # AVX512{F,VL}
	vptestmd	%ymm29, %ymm30, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	(%rcx), %ymm30, %k5	 # AVX512{F,VL}
	vptestmd	0x123(%rax,%r14,8), %ymm30, %k5	 # AVX512{F,VL}
	vptestmd	(%rcx){1to8}, %ymm30, %k5	 # AVX512{F,VL}
	vptestmd	4064(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vptestmd	4096(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vptestmd	-4096(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vptestmd	-4128(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vptestmd	508(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vptestmd	512(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL}
	vptestmd	-512(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vptestmd	-516(%rdx){1to8}, %ymm30, %k5	 # AVX512{F,VL}
	vptestmq	%xmm29, %xmm30, %k5	 # AVX512{F,VL}
	vptestmq	%xmm29, %xmm30, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	(%rcx), %xmm30, %k5	 # AVX512{F,VL}
	vptestmq	0x123(%rax,%r14,8), %xmm30, %k5	 # AVX512{F,VL}
	vptestmq	(%rcx){1to2}, %xmm30, %k5	 # AVX512{F,VL}
	vptestmq	2032(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vptestmq	2048(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vptestmq	-2048(%rdx), %xmm30, %k5	 # AVX512{F,VL} Disp8
	vptestmq	-2064(%rdx), %xmm30, %k5	 # AVX512{F,VL}
	vptestmq	1016(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vptestmq	1024(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL}
	vptestmq	-1024(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL} Disp8
	vptestmq	-1032(%rdx){1to2}, %xmm30, %k5	 # AVX512{F,VL}
	vptestmq	%ymm29, %ymm30, %k5	 # AVX512{F,VL}
	vptestmq	%ymm29, %ymm30, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	(%rcx), %ymm30, %k5	 # AVX512{F,VL}
	vptestmq	0x123(%rax,%r14,8), %ymm30, %k5	 # AVX512{F,VL}
	vptestmq	(%rcx){1to4}, %ymm30, %k5	 # AVX512{F,VL}
	vptestmq	4064(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vptestmq	4096(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vptestmq	-4096(%rdx), %ymm30, %k5	 # AVX512{F,VL} Disp8
	vptestmq	-4128(%rdx), %ymm30, %k5	 # AVX512{F,VL}
	vptestmq	1016(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vptestmq	1024(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL}
	vptestmq	-1024(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL} Disp8
	vptestmq	-1032(%rdx){1to4}, %ymm30, %k5	 # AVX512{F,VL}
	vpunpckhdq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhdq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpunpckhdq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpunpckhdq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhdq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhdq	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhdq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpckhdq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhdq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpckhdq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhdq	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpckhdq	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhdq	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpckhdq	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhdq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckhdq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpunpckhdq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpunpckhdq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckhdq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckhdq	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckhdq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpckhdq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckhdq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpckhdq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckhdq	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpckhdq	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckhdq	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpckhdq	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckhqdq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhqdq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpunpckhqdq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhqdq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhqdq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhqdq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpckhqdq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhqdq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpckhqdq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhqdq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpckhqdq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhqdq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpckhqdq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckhqdq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckhqdq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpunpckhqdq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckhqdq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckhqdq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckhqdq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpckhqdq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckhqdq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpckhqdq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckhqdq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpckhqdq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckhqdq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpckhqdq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckldq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckldq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpunpckldq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpunpckldq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckldq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckldq	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckldq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpckldq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckldq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpckldq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckldq	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpckldq	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckldq	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpckldq	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpckldq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckldq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpunpckldq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpunpckldq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckldq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckldq	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckldq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpckldq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckldq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpckldq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckldq	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpckldq	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpckldq	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpckldq	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpcklqdq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpcklqdq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpunpcklqdq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpcklqdq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpcklqdq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpcklqdq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpcklqdq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpcklqdq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpcklqdq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpcklqdq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpcklqdq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpcklqdq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpunpcklqdq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpunpcklqdq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpcklqdq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpunpcklqdq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpcklqdq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpcklqdq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpcklqdq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpcklqdq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpcklqdq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpcklqdq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpcklqdq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpcklqdq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpunpcklqdq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpunpcklqdq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpxord	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpxord	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpxord	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpxord	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpxord	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpxord	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpxord	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpxord	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpxord	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpxord	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpxord	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpxord	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpxord	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpxord	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpxord	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpxord	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpxord	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpxord	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpxord	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpxord	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpxord	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpxord	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpxord	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpxord	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpxord	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpxord	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpxord	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpxord	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpxorq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpxorq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpxorq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpxorq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpxorq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpxorq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpxorq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpxorq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpxorq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpxorq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpxorq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpxorq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpxorq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpxorq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpxorq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpxorq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpxorq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpxorq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpxorq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpxorq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpxorq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpxorq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpxorq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpxorq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpxorq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpxorq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpxorq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpxorq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vrcp14pd	%xmm29, %xmm30	 # AVX512{F,VL}
	vrcp14pd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vrcp14pd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vrcp14pd	(%rcx), %xmm30	 # AVX512{F,VL}
	vrcp14pd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vrcp14pd	(%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vrcp14pd	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vrcp14pd	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vrcp14pd	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vrcp14pd	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vrcp14pd	1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vrcp14pd	1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vrcp14pd	-1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vrcp14pd	-1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vrcp14pd	%ymm29, %ymm30	 # AVX512{F,VL}
	vrcp14pd	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vrcp14pd	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vrcp14pd	(%rcx), %ymm30	 # AVX512{F,VL}
	vrcp14pd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vrcp14pd	(%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vrcp14pd	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vrcp14pd	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vrcp14pd	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vrcp14pd	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vrcp14pd	1016(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vrcp14pd	1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vrcp14pd	-1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vrcp14pd	-1032(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vrcp14ps	%xmm29, %xmm30	 # AVX512{F,VL}
	vrcp14ps	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vrcp14ps	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vrcp14ps	(%rcx), %xmm30	 # AVX512{F,VL}
	vrcp14ps	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vrcp14ps	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vrcp14ps	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vrcp14ps	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vrcp14ps	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vrcp14ps	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vrcp14ps	508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vrcp14ps	512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vrcp14ps	-512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vrcp14ps	-516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vrcp14ps	%ymm29, %ymm30	 # AVX512{F,VL}
	vrcp14ps	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vrcp14ps	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vrcp14ps	(%rcx), %ymm30	 # AVX512{F,VL}
	vrcp14ps	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vrcp14ps	(%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vrcp14ps	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vrcp14ps	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vrcp14ps	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vrcp14ps	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vrcp14ps	508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vrcp14ps	512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vrcp14ps	-512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vrcp14ps	-516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vrsqrt14pd	%xmm29, %xmm30	 # AVX512{F,VL}
	vrsqrt14pd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vrsqrt14pd	(%rcx), %xmm30	 # AVX512{F,VL}
	vrsqrt14pd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vrsqrt14pd	(%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vrsqrt14pd	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vrsqrt14pd	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vrsqrt14pd	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vrsqrt14pd	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vrsqrt14pd	1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vrsqrt14pd	1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vrsqrt14pd	-1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vrsqrt14pd	-1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vrsqrt14pd	%ymm29, %ymm30	 # AVX512{F,VL}
	vrsqrt14pd	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vrsqrt14pd	(%rcx), %ymm30	 # AVX512{F,VL}
	vrsqrt14pd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vrsqrt14pd	(%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vrsqrt14pd	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vrsqrt14pd	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vrsqrt14pd	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vrsqrt14pd	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vrsqrt14pd	1016(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vrsqrt14pd	1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vrsqrt14pd	-1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vrsqrt14pd	-1032(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vrsqrt14ps	%xmm29, %xmm30	 # AVX512{F,VL}
	vrsqrt14ps	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vrsqrt14ps	(%rcx), %xmm30	 # AVX512{F,VL}
	vrsqrt14ps	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vrsqrt14ps	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vrsqrt14ps	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vrsqrt14ps	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vrsqrt14ps	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vrsqrt14ps	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vrsqrt14ps	508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vrsqrt14ps	512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vrsqrt14ps	-512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vrsqrt14ps	-516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vrsqrt14ps	%ymm29, %ymm30	 # AVX512{F,VL}
	vrsqrt14ps	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vrsqrt14ps	(%rcx), %ymm30	 # AVX512{F,VL}
	vrsqrt14ps	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vrsqrt14ps	(%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vrsqrt14ps	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vrsqrt14ps	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vrsqrt14ps	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vrsqrt14ps	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vrsqrt14ps	508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vrsqrt14ps	512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vrsqrt14ps	-512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vrsqrt14ps	-516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vscatterdpd	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vscatterdpd	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vscatterdpd	%xmm30, 256(%r9,%xmm31){%k1}	 # AVX512{F,VL}
	vscatterdpd	%xmm30, 1024(%rcx,%xmm31,4){%k1}	 # AVX512{F,VL}
	vscatterdpd	%ymm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vscatterdpd	%ymm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vscatterdpd	%ymm30, 256(%r9,%xmm31){%k1}	 # AVX512{F,VL}
	vscatterdpd	%ymm30, 1024(%rcx,%xmm31,4){%k1}	 # AVX512{F,VL}
	vscatterdps	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vscatterdps	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vscatterdps	%xmm30, 256(%r9,%xmm31){%k1}	 # AVX512{F,VL}
	vscatterdps	%xmm30, 1024(%rcx,%xmm31,4){%k1}	 # AVX512{F,VL}
	vscatterdps	%ymm30, 123(%r14,%ymm31,8){%k1}	 # AVX512{F,VL}
	vscatterdps	%ymm30, 123(%r14,%ymm31,8){%k1}	 # AVX512{F,VL}
	vscatterdps	%ymm30, 256(%r9,%ymm31){%k1}	 # AVX512{F,VL}
	vscatterdps	%ymm30, 1024(%rcx,%ymm31,4){%k1}	 # AVX512{F,VL}
	vscatterqpd	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vscatterqpd	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vscatterqpd	%xmm30, 256(%r9,%xmm31){%k1}	 # AVX512{F,VL}
	vscatterqpd	%xmm30, 1024(%rcx,%xmm31,4){%k1}	 # AVX512{F,VL}
	vscatterqpd	%ymm30, 123(%r14,%ymm31,8){%k1}	 # AVX512{F,VL}
	vscatterqpd	%ymm30, 123(%r14,%ymm31,8){%k1}	 # AVX512{F,VL}
	vscatterqpd	%ymm30, 256(%r9,%ymm31){%k1}	 # AVX512{F,VL}
	vscatterqpd	%ymm30, 1024(%rcx,%ymm31,4){%k1}	 # AVX512{F,VL}
	vscatterqps	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vscatterqps	%xmm30, 123(%r14,%xmm31,8){%k1}	 # AVX512{F,VL}
	vscatterqps	%xmm30, 256(%r9,%xmm31){%k1}	 # AVX512{F,VL}
	vscatterqps	%xmm30, 1024(%rcx,%xmm31,4){%k1}	 # AVX512{F,VL}
	vscatterqps	%xmm30, 123(%r14,%ymm31,8){%k1}	 # AVX512{F,VL}
	vscatterqps	%xmm30, 123(%r14,%ymm31,8){%k1}	 # AVX512{F,VL}
	vscatterqps	%xmm30, 256(%r9,%ymm31){%k1}	 # AVX512{F,VL}
	vscatterqps	%xmm30, 1024(%rcx,%ymm31,4){%k1}	 # AVX512{F,VL}
	vshufpd	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vshufpd	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vshufpd	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vshufpd	$123, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vshufpd	$123, (%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vshufpd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vshufpd	$123, (%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vshufpd	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vshufpd	$123, 2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vshufpd	$123, -2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vshufpd	$123, -2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vshufpd	$123, 1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vshufpd	$123, 1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vshufpd	$123, -1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vshufpd	$123, -1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vshufpd	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufpd	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vshufpd	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vshufpd	$123, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufpd	$123, (%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufpd	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufpd	$123, (%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufpd	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufpd	$123, 4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufpd	$123, -4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufpd	$123, -4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufpd	$123, 1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufpd	$123, 1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufpd	$123, -1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufpd	$123, -1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufps	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vshufps	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vshufps	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vshufps	$123, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vshufps	$123, (%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vshufps	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vshufps	$123, (%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vshufps	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vshufps	$123, 2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vshufps	$123, -2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vshufps	$123, -2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vshufps	$123, 508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vshufps	$123, 512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vshufps	$123, -512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vshufps	$123, -516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vshufps	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufps	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vshufps	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vshufps	$123, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufps	$123, (%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufps	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufps	$123, (%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufps	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufps	$123, 4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufps	$123, -4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufps	$123, -4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufps	$123, 508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufps	$123, 512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufps	$123, -512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufps	$123, -516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vsqrtpd	%xmm29, %xmm30	 # AVX512{F,VL}
	vsqrtpd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vsqrtpd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vsqrtpd	(%rcx), %xmm30	 # AVX512{F,VL}
	vsqrtpd	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vsqrtpd	(%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vsqrtpd	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vsqrtpd	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vsqrtpd	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vsqrtpd	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vsqrtpd	1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vsqrtpd	1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vsqrtpd	-1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vsqrtpd	-1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vsqrtpd	%ymm29, %ymm30	 # AVX512{F,VL}
	vsqrtpd	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vsqrtpd	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vsqrtpd	(%rcx), %ymm30	 # AVX512{F,VL}
	vsqrtpd	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vsqrtpd	(%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vsqrtpd	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vsqrtpd	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vsqrtpd	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vsqrtpd	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vsqrtpd	1016(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vsqrtpd	1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vsqrtpd	-1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vsqrtpd	-1032(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vsqrtps	%xmm29, %xmm30	 # AVX512{F,VL}
	vsqrtps	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vsqrtps	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vsqrtps	(%rcx), %xmm30	 # AVX512{F,VL}
	vsqrtps	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vsqrtps	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vsqrtps	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vsqrtps	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vsqrtps	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vsqrtps	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vsqrtps	508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vsqrtps	512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vsqrtps	-512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vsqrtps	-516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vsqrtps	%ymm29, %ymm30	 # AVX512{F,VL}
	vsqrtps	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vsqrtps	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vsqrtps	(%rcx), %ymm30	 # AVX512{F,VL}
	vsqrtps	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vsqrtps	(%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vsqrtps	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vsqrtps	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vsqrtps	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vsqrtps	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vsqrtps	508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vsqrtps	512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vsqrtps	-512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vsqrtps	-516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vsubpd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vsubpd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vsubpd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vsubpd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vsubpd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vsubpd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vsubpd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vsubpd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vsubpd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vsubpd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vsubpd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vsubpd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vsubpd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vsubpd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vsubpd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vsubpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vsubpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vsubpd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vsubpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vsubpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vsubpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vsubpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vsubpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vsubpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vsubpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vsubpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vsubpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vsubpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vsubps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vsubps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vsubps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vsubps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vsubps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vsubps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vsubps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vsubps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vsubps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vsubps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vsubps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vsubps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vsubps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vsubps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vsubps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vsubps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vsubps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vsubps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vsubps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vsubps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vsubps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vsubps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vsubps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vsubps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vsubps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vsubps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vsubps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vsubps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhpd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhpd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vunpckhpd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vunpckhpd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhpd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhpd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhpd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpckhpd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhpd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpckhpd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhpd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpckhpd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhpd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpckhpd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhpd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vunpckhpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vunpckhpd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpckhpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpckhpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpckhpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpckhpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vunpckhps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vunpckhps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpckhps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpckhps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpckhps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpckhps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpckhps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vunpckhps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vunpckhps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpckhps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpckhps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpckhps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpckhps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpckhps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklpd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklpd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vunpcklpd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vunpcklpd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklpd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklpd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklpd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpcklpd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklpd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpcklpd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklpd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpcklpd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklpd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpcklpd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklpd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vunpcklpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vunpcklpd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpcklpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpcklpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpcklpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpcklpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vunpcklps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vunpcklps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpcklps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpcklps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpcklps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vunpcklps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vunpcklps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vunpcklps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vunpcklps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpcklps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpcklps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpcklps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vunpcklps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vunpcklps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogd	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogd	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpternlogd	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpternlogd	$123, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogd	$123, (%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogd	$123, (%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogd	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpternlogd	$123, 2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogd	$123, -2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpternlogd	$123, -2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogd	$123, 508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpternlogd	$123, 512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogd	$123, -512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpternlogd	$123, -516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogd	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogd	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpternlogd	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpternlogd	$123, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogd	$123, (%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogd	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogd	$123, (%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogd	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpternlogd	$123, 4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogd	$123, -4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpternlogd	$123, -4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogd	$123, 508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpternlogd	$123, 512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogd	$123, -512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpternlogd	$123, -516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogq	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogq	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpternlogq	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpternlogq	$123, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogq	$123, (%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogq	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogq	$123, (%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogq	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpternlogq	$123, 2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogq	$123, -2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpternlogq	$123, -2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogq	$123, 1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpternlogq	$123, 1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogq	$123, -1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpternlogq	$123, -1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpternlogq	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogq	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpternlogq	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpternlogq	$123, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogq	$123, (%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogq	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogq	$123, (%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogq	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpternlogq	$123, 4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogq	$123, -4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpternlogq	$123, -4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogq	$123, 1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpternlogq	$123, 1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpternlogq	$123, -1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpternlogq	$123, -1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpmovqb	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovqb	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovqb	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovqb	%ymm29, %xmm30	 # AVX512{F,VL}
	vpmovqb	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovqb	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsqb	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovsqb	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsqb	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsqb	%ymm29, %xmm30	 # AVX512{F,VL}
	vpmovsqb	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsqb	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovusqb	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovusqb	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovusqb	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovusqb	%ymm29, %xmm30	 # AVX512{F,VL}
	vpmovusqb	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovusqb	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovqw	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovqw	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovqw	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovqw	%ymm29, %xmm30	 # AVX512{F,VL}
	vpmovqw	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovqw	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsqw	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovsqw	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsqw	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsqw	%ymm29, %xmm30	 # AVX512{F,VL}
	vpmovsqw	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsqw	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovusqw	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovusqw	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovusqw	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovusqw	%ymm29, %xmm30	 # AVX512{F,VL}
	vpmovusqw	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovusqw	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovqd	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovqd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovqd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovqd	%ymm29, %xmm30	 # AVX512{F,VL}
	vpmovqd	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovqd	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsqd	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovsqd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsqd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsqd	%ymm29, %xmm30	 # AVX512{F,VL}
	vpmovsqd	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsqd	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovusqd	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovusqd	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovusqd	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovusqd	%ymm29, %xmm30	 # AVX512{F,VL}
	vpmovusqd	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovusqd	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovdb	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovdb	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovdb	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovdb	%ymm29, %xmm30	 # AVX512{F,VL}
	vpmovdb	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovdb	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsdb	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovsdb	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsdb	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsdb	%ymm29, %xmm30	 # AVX512{F,VL}
	vpmovsdb	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsdb	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovusdb	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovusdb	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovusdb	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovusdb	%ymm29, %xmm30	 # AVX512{F,VL}
	vpmovusdb	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovusdb	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovdw	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovdw	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovdw	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovdw	%ymm29, %xmm30	 # AVX512{F,VL}
	vpmovdw	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovdw	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsdw	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovsdw	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsdw	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovsdw	%ymm29, %xmm30	 # AVX512{F,VL}
	vpmovsdw	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovsdw	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovusdw	%xmm29, %xmm30	 # AVX512{F,VL}
	vpmovusdw	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovusdw	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpmovusdw	%ymm29, %xmm30	 # AVX512{F,VL}
	vpmovusdw	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpmovusdw	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vshuff32x4	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff32x4	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vshuff32x4	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vshuff32x4	$123, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff32x4	$123, (%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff32x4	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff32x4	$123, (%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff32x4	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshuff32x4	$123, 4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff32x4	$123, -4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshuff32x4	$123, -4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff32x4	$123, 508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshuff32x4	$123, 512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff32x4	$123, -512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshuff32x4	$123, -516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff64x2	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff64x2	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vshuff64x2	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vshuff64x2	$123, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff64x2	$123, (%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff64x2	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff64x2	$123, (%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff64x2	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshuff64x2	$123, 4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff64x2	$123, -4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshuff64x2	$123, -4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff64x2	$123, 1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshuff64x2	$123, 1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshuff64x2	$123, -1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshuff64x2	$123, -1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi32x4	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi32x4	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vshufi32x4	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vshufi32x4	$123, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi32x4	$123, (%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi32x4	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi32x4	$123, (%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi32x4	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufi32x4	$123, 4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi32x4	$123, -4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufi32x4	$123, -4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi32x4	$123, 508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufi32x4	$123, 512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi32x4	$123, -512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufi32x4	$123, -516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi64x2	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi64x2	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vshufi64x2	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vshufi64x2	$123, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi64x2	$123, (%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi64x2	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi64x2	$123, (%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi64x2	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufi64x2	$123, 4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi64x2	$123, -4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufi64x2	$123, -4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi64x2	$123, 1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufi64x2	$123, 1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vshufi64x2	$123, -1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vshufi64x2	$123, -1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermpd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermpd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2d	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2d	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpermt2d	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpermt2d	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2d	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2d	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2d	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2d	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2d	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2d	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2d	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2d	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2d	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2d	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2d	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2d	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermt2d	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermt2d	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2d	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2d	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2d	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2d	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2d	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2d	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2d	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2d	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2d	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2d	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2q	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2q	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpermt2q	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpermt2q	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2q	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2q	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2q	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2q	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2q	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2q	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2q	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2q	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2q	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2q	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2q	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2q	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermt2q	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermt2q	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2q	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2q	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2q	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2q	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2q	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2q	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2q	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2q	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2q	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2q	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpermt2ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpermt2ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermt2ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermt2ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpermt2pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpermt2pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermt2pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermt2pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermt2pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermt2pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermt2pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermt2pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	valignq	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	valignq	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	valignq	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	valignq	$123, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	valignq	$123, (%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	valignq	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	valignq	$123, (%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	valignq	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	valignq	$123, 2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	valignq	$123, -2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	valignq	$123, -2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	valignq	$123, 1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	valignq	$123, 1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	valignq	$123, -1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	valignq	$123, -1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	valignq	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	valignq	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	valignq	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	valignq	$123, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	valignq	$123, (%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	valignq	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	valignq	$123, (%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	valignq	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	valignq	$123, 4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	valignq	$123, -4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	valignq	$123, -4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	valignq	$123, 1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	valignq	$123, 1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	valignq	$123, -1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	valignq	$123, -1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefpd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefpd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vscalefpd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vscalefpd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefpd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefpd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefpd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vscalefpd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefpd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vscalefpd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefpd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vscalefpd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefpd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vscalefpd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefpd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefpd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vscalefpd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vscalefpd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefpd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefpd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefpd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vscalefpd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefpd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vscalefpd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefpd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vscalefpd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefpd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vscalefpd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vscalefps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vscalefps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vscalefps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vscalefps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vscalefps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vscalefps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vscalefps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vscalefps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vscalefps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vscalefps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vscalefps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vscalefps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vscalefps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vscalefps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmpd	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmpd	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfixupimmpd	$123, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmpd	$123, (%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmpd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmpd	$123, (%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmpd	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, 2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmpd	$123, -2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, -2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmpd	$123, 1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, 1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmpd	$123, -1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, -1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmpd	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmpd	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfixupimmpd	$123, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmpd	$123, (%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmpd	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmpd	$123, (%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmpd	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, 4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmpd	$123, -4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, -4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmpd	$123, 1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, 1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmpd	$123, -1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, -1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmps	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmps	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vfixupimmps	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vfixupimmps	$123, %xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmps	$123, (%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmps	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmps	$123, (%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmps	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, 2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmps	$123, -2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, -2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmps	$123, 508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, 512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmps	$123, -512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, -516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vfixupimmps	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmps	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vfixupimmps	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vfixupimmps	$123, %ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmps	$123, (%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmps	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmps	$123, (%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmps	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, 4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmps	$123, -4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, -4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmps	$123, 508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, 512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vfixupimmps	$123, -512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, -516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpslld	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vpslld	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpslld	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpslld	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vpslld	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vpslld	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpslld	$123, (%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vpslld	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpslld	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vpslld	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpslld	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vpslld	$123, 508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vpslld	$123, 512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vpslld	$123, -512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vpslld	$123, -516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vpslld	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vpslld	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpslld	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpslld	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vpslld	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vpslld	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpslld	$123, (%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vpslld	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpslld	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vpslld	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpslld	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vpslld	$123, 508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vpslld	$123, 512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vpslld	$123, -512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vpslld	$123, -516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vpsllq	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllq	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsllq	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsllq	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsllq	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vpsllq	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpsllq	$123, (%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vpsllq	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpsllq	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vpsllq	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpsllq	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vpsllq	$123, 1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vpsllq	$123, 1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vpsllq	$123, -1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vpsllq	$123, -1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vpsllq	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllq	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsllq	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsllq	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsllq	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vpsllq	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpsllq	$123, (%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vpsllq	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpsllq	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vpsllq	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpsllq	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vpsllq	$123, 1016(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vpsllq	$123, 1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vpsllq	$123, -1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vpsllq	$123, -1032(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vpsrad	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrad	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsrad	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsrad	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsrad	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vpsrad	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpsrad	$123, (%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vpsrad	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpsrad	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vpsrad	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpsrad	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vpsrad	$123, 508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vpsrad	$123, 512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vpsrad	$123, -512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vpsrad	$123, -516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vpsrad	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrad	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsrad	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsrad	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsrad	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vpsrad	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpsrad	$123, (%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vpsrad	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpsrad	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vpsrad	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpsrad	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vpsrad	$123, 508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vpsrad	$123, 512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vpsrad	$123, -512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vpsrad	$123, -516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vpsraq	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsraq	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpsraq	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpsraq	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vpsraq	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vpsraq	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vpsraq	$123, (%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vpsraq	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpsraq	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vpsraq	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vpsraq	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vpsraq	$123, 1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vpsraq	$123, 1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vpsraq	$123, -1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vpsraq	$123, -1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vpsraq	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsraq	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpsraq	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpsraq	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vpsraq	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vpsraq	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vpsraq	$123, (%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vpsraq	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpsraq	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vpsraq	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vpsraq	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vpsraq	$123, 1016(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vpsraq	$123, 1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vpsraq	$123, -1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vpsraq	$123, -1032(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vprolvd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vprolvd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vprolvd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvd	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprolvd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprolvd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvd	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprolvd	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvd	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprolvd	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vprolvd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vprolvd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vprolvd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vprolvd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vprolvd	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vprolvd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprolvd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vprolvd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprolvd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vprolvd	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprolvd	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vprolvd	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprolvd	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vprold	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vprold	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vprold	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vprold	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vprold	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vprold	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vprold	$123, (%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vprold	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vprold	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vprold	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vprold	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vprold	$123, 508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vprold	$123, 512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vprold	$123, -512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vprold	$123, -516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vprold	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vprold	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vprold	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vprold	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vprold	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vprold	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vprold	$123, (%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vprold	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vprold	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vprold	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vprold	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vprold	$123, 508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vprold	$123, 512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vprold	$123, -512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vprold	$123, -516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vprolvq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vprolvq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vprolvq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprolvq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprolvq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprolvq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprolvq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vprolvq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vprolvq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vprolvq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vprolvq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vprolvq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vprolvq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vprolvq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprolvq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vprolvq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprolvq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vprolvq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprolvq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vprolvq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprolvq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vprolq	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vprolq	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vprolq	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vprolq	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vprolq	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vprolq	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vprolq	$123, (%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vprolq	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vprolq	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vprolq	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vprolq	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vprolq	$123, 1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vprolq	$123, 1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vprolq	$123, -1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vprolq	$123, -1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vprolq	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vprolq	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vprolq	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vprolq	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vprolq	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vprolq	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vprolq	$123, (%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vprolq	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vprolq	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vprolq	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vprolq	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vprolq	$123, 1016(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vprolq	$123, 1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vprolq	$123, -1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vprolq	$123, -1032(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vprorvd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vprorvd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vprorvd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvd	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprorvd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprorvd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvd	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprorvd	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvd	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprorvd	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vprorvd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vprorvd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vprorvd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vprorvd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vprorvd	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vprorvd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprorvd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vprorvd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprorvd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vprorvd	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprorvd	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vprorvd	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprorvd	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vprord	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vprord	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vprord	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vprord	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vprord	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vprord	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vprord	$123, (%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vprord	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vprord	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vprord	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vprord	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vprord	$123, 508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vprord	$123, 512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vprord	$123, -512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vprord	$123, -516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vprord	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vprord	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vprord	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vprord	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vprord	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vprord	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vprord	$123, (%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vprord	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vprord	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vprord	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vprord	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vprord	$123, 508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vprord	$123, 512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vprord	$123, -512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vprord	$123, -516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vprorvq	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vprorvq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vprorvq	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvq	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprorvq	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprorvq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprorvq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vprorvq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vprorvq	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vprorvq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vprorvq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vprorvq	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vprorvq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vprorvq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vprorvq	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprorvq	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vprorvq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprorvq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vprorvq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprorvq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vprorvq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vprorvq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vprorq	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vprorq	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vprorq	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vprorq	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vprorq	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vprorq	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vprorq	$123, (%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vprorq	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vprorq	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vprorq	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vprorq	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vprorq	$123, 1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vprorq	$123, 1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vprorq	$123, -1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vprorq	$123, -1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vprorq	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vprorq	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vprorq	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vprorq	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vprorq	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vprorq	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vprorq	$123, (%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vprorq	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vprorq	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vprorq	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vprorq	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vprorq	$123, 1016(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vprorq	$123, 1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vprorq	$123, -1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vprorq	$123, -1032(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vrndscalepd	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vrndscalepd	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vrndscalepd	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vrndscalepd	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vrndscalepd	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vrndscalepd	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vrndscalepd	$123, (%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vrndscalepd	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vrndscalepd	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vrndscalepd	$123, 1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, 1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vrndscalepd	$123, -1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, -1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vrndscalepd	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vrndscalepd	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vrndscalepd	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vrndscalepd	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vrndscalepd	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vrndscalepd	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vrndscalepd	$123, (%rcx){1to4}, %ymm30	 # AVX512{F,VL}
	vrndscalepd	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vrndscalepd	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vrndscalepd	$123, 1016(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, 1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vrndscalepd	$123, -1024(%rdx){1to4}, %ymm30	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, -1032(%rdx){1to4}, %ymm30	 # AVX512{F,VL}
	vrndscaleps	$0xab, %xmm29, %xmm30	 # AVX512{F,VL}
	vrndscaleps	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vrndscaleps	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vrndscaleps	$123, %xmm29, %xmm30	 # AVX512{F,VL}
	vrndscaleps	$123, (%rcx), %xmm30	 # AVX512{F,VL}
	vrndscaleps	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vrndscaleps	$123, (%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vrndscaleps	$123, 2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, 2048(%rdx), %xmm30	 # AVX512{F,VL}
	vrndscaleps	$123, -2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, -2064(%rdx), %xmm30	 # AVX512{F,VL}
	vrndscaleps	$123, 508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, 512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vrndscaleps	$123, -512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, -516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vrndscaleps	$0xab, %ymm29, %ymm30	 # AVX512{F,VL}
	vrndscaleps	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vrndscaleps	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vrndscaleps	$123, %ymm29, %ymm30	 # AVX512{F,VL}
	vrndscaleps	$123, (%rcx), %ymm30	 # AVX512{F,VL}
	vrndscaleps	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vrndscaleps	$123, (%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vrndscaleps	$123, 4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, 4096(%rdx), %ymm30	 # AVX512{F,VL}
	vrndscaleps	$123, -4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, -4128(%rdx), %ymm30	 # AVX512{F,VL}
	vrndscaleps	$123, 508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, 512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vrndscaleps	$123, -512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, -516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vpcompressq	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpcompressq	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpcompressq	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpcompressq	%xmm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vpcompressq	%xmm30, 1024(%rdx)	 # AVX512{F,VL}
	vpcompressq	%xmm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vpcompressq	%xmm30, -1032(%rdx)	 # AVX512{F,VL}
	vpcompressq	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpcompressq	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpcompressq	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpcompressq	%ymm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vpcompressq	%ymm30, 1024(%rdx)	 # AVX512{F,VL}
	vpcompressq	%ymm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vpcompressq	%ymm30, -1032(%rdx)	 # AVX512{F,VL}
	vpcompressq	%xmm29, %xmm30	 # AVX512{F,VL}
	vpcompressq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpcompressq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpcompressq	%ymm29, %ymm30	 # AVX512{F,VL}
	vpcompressq	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpcompressq	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %xmm30, (%rcx)	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vcvtps2ph	$123, %xmm30, (%rcx)	 # AVX512{F,VL}
	vcvtps2ph	$123, %xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vcvtps2ph	$123, %xmm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vcvtps2ph	$123, %xmm30, 1024(%rdx)	 # AVX512{F,VL}
	vcvtps2ph	$123, %xmm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vcvtps2ph	$123, %xmm30, -1032(%rdx)	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %ymm30, (%rcx)	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vcvtps2ph	$123, %ymm30, (%rcx)	 # AVX512{F,VL}
	vcvtps2ph	$123, %ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vcvtps2ph	$123, %ymm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vcvtps2ph	$123, %ymm30, 2048(%rdx)	 # AVX512{F,VL}
	vcvtps2ph	$123, %ymm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vcvtps2ph	$123, %ymm30, -2064(%rdx)	 # AVX512{F,VL}
	vextractf32x4	$0xab, %ymm29, (%rcx)	 # AVX512{F,VL}
	vextractf32x4	$0xab, %ymm29, (%rcx){%k7}	 # AVX512{F,VL}
	vextractf32x4	$123, %ymm29, (%rcx)	 # AVX512{F,VL}
	vextractf32x4	$123, %ymm29, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vextractf32x4	$123, %ymm29, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vextractf32x4	$123, %ymm29, 2048(%rdx)	 # AVX512{F,VL}
	vextractf32x4	$123, %ymm29, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vextractf32x4	$123, %ymm29, -2064(%rdx)	 # AVX512{F,VL}
	vextracti32x4	$0xab, %ymm29, (%rcx)	 # AVX512{F,VL}
	vextracti32x4	$0xab, %ymm29, (%rcx){%k7}	 # AVX512{F,VL}
	vextracti32x4	$123, %ymm29, (%rcx)	 # AVX512{F,VL}
	vextracti32x4	$123, %ymm29, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vextracti32x4	$123, %ymm29, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vextracti32x4	$123, %ymm29, 2048(%rdx)	 # AVX512{F,VL}
	vextracti32x4	$123, %ymm29, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vextracti32x4	$123, %ymm29, -2064(%rdx)	 # AVX512{F,VL}
	vmovapd	%xmm30, (%rcx)	 # AVX512{F,VL}
	vmovapd	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovapd	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovapd	%xmm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vmovapd	%xmm30, 2048(%rdx)	 # AVX512{F,VL}
	vmovapd	%xmm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vmovapd	%xmm30, -2064(%rdx)	 # AVX512{F,VL}
	vmovapd	%ymm30, (%rcx)	 # AVX512{F,VL}
	vmovapd	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovapd	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovapd	%ymm30, 4064(%rdx)	 # AVX512{F,VL} Disp8
	vmovapd	%ymm30, 4096(%rdx)	 # AVX512{F,VL}
	vmovapd	%ymm30, -4096(%rdx)	 # AVX512{F,VL} Disp8
	vmovapd	%ymm30, -4128(%rdx)	 # AVX512{F,VL}
	vmovaps	%xmm30, (%rcx)	 # AVX512{F,VL}
	vmovaps	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovaps	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovaps	%xmm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vmovaps	%xmm30, 2048(%rdx)	 # AVX512{F,VL}
	vmovaps	%xmm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vmovaps	%xmm30, -2064(%rdx)	 # AVX512{F,VL}
	vmovaps	%ymm30, (%rcx)	 # AVX512{F,VL}
	vmovaps	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovaps	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovaps	%ymm30, 4064(%rdx)	 # AVX512{F,VL} Disp8
	vmovaps	%ymm30, 4096(%rdx)	 # AVX512{F,VL}
	vmovaps	%ymm30, -4096(%rdx)	 # AVX512{F,VL} Disp8
	vmovaps	%ymm30, -4128(%rdx)	 # AVX512{F,VL}
	vmovdqa32	%xmm30, (%rcx)	 # AVX512{F,VL}
	vmovdqa32	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovdqa32	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovdqa32	%xmm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqa32	%xmm30, 2048(%rdx)	 # AVX512{F,VL}
	vmovdqa32	%xmm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqa32	%xmm30, -2064(%rdx)	 # AVX512{F,VL}
	vmovdqa32	%ymm30, (%rcx)	 # AVX512{F,VL}
	vmovdqa32	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovdqa32	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovdqa32	%ymm30, 4064(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqa32	%ymm30, 4096(%rdx)	 # AVX512{F,VL}
	vmovdqa32	%ymm30, -4096(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqa32	%ymm30, -4128(%rdx)	 # AVX512{F,VL}
	vmovdqa64	%xmm30, (%rcx)	 # AVX512{F,VL}
	vmovdqa64	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovdqa64	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovdqa64	%xmm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqa64	%xmm30, 2048(%rdx)	 # AVX512{F,VL}
	vmovdqa64	%xmm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqa64	%xmm30, -2064(%rdx)	 # AVX512{F,VL}
	vmovdqa64	%ymm30, (%rcx)	 # AVX512{F,VL}
	vmovdqa64	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovdqa64	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovdqa64	%ymm30, 4064(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqa64	%ymm30, 4096(%rdx)	 # AVX512{F,VL}
	vmovdqa64	%ymm30, -4096(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqa64	%ymm30, -4128(%rdx)	 # AVX512{F,VL}
	vmovdqu32	%xmm30, (%rcx)	 # AVX512{F,VL}
	vmovdqu32	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovdqu32	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovdqu32	%xmm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqu32	%xmm30, 2048(%rdx)	 # AVX512{F,VL}
	vmovdqu32	%xmm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqu32	%xmm30, -2064(%rdx)	 # AVX512{F,VL}
	vmovdqu32	%ymm30, (%rcx)	 # AVX512{F,VL}
	vmovdqu32	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovdqu32	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovdqu32	%ymm30, 4064(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqu32	%ymm30, 4096(%rdx)	 # AVX512{F,VL}
	vmovdqu32	%ymm30, -4096(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqu32	%ymm30, -4128(%rdx)	 # AVX512{F,VL}
	vmovdqu64	%xmm30, (%rcx)	 # AVX512{F,VL}
	vmovdqu64	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovdqu64	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovdqu64	%xmm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqu64	%xmm30, 2048(%rdx)	 # AVX512{F,VL}
	vmovdqu64	%xmm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqu64	%xmm30, -2064(%rdx)	 # AVX512{F,VL}
	vmovdqu64	%ymm30, (%rcx)	 # AVX512{F,VL}
	vmovdqu64	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovdqu64	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovdqu64	%ymm30, 4064(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqu64	%ymm30, 4096(%rdx)	 # AVX512{F,VL}
	vmovdqu64	%ymm30, -4096(%rdx)	 # AVX512{F,VL} Disp8
	vmovdqu64	%ymm30, -4128(%rdx)	 # AVX512{F,VL}
	vmovupd	%xmm30, (%rcx)	 # AVX512{F,VL}
	vmovupd	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovupd	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovupd	%xmm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vmovupd	%xmm30, 2048(%rdx)	 # AVX512{F,VL}
	vmovupd	%xmm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vmovupd	%xmm30, -2064(%rdx)	 # AVX512{F,VL}
	vmovupd	%ymm30, (%rcx)	 # AVX512{F,VL}
	vmovupd	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovupd	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovupd	%ymm30, 4064(%rdx)	 # AVX512{F,VL} Disp8
	vmovupd	%ymm30, 4096(%rdx)	 # AVX512{F,VL}
	vmovupd	%ymm30, -4096(%rdx)	 # AVX512{F,VL} Disp8
	vmovupd	%ymm30, -4128(%rdx)	 # AVX512{F,VL}
	vmovups	%xmm30, (%rcx)	 # AVX512{F,VL}
	vmovups	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovups	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovups	%xmm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vmovups	%xmm30, 2048(%rdx)	 # AVX512{F,VL}
	vmovups	%xmm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vmovups	%xmm30, -2064(%rdx)	 # AVX512{F,VL}
	vmovups	%ymm30, (%rcx)	 # AVX512{F,VL}
	vmovups	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vmovups	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vmovups	%ymm30, 4064(%rdx)	 # AVX512{F,VL} Disp8
	vmovups	%ymm30, 4096(%rdx)	 # AVX512{F,VL}
	vmovups	%ymm30, -4096(%rdx)	 # AVX512{F,VL} Disp8
	vmovups	%ymm30, -4128(%rdx)	 # AVX512{F,VL}
	vpmovqb	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpmovqb	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovqb	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovqb	%xmm30, 254(%rdx)	 # AVX512{F,VL} Disp8
	vpmovqb	%xmm30, 256(%rdx)	 # AVX512{F,VL}
	vpmovqb	%xmm30, -256(%rdx)	 # AVX512{F,VL} Disp8
	vpmovqb	%xmm30, -258(%rdx)	 # AVX512{F,VL}
	vpmovqb	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpmovqb	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovqb	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovqb	%ymm30, 508(%rdx)	 # AVX512{F,VL} Disp8
	vpmovqb	%ymm30, 512(%rdx)	 # AVX512{F,VL}
	vpmovqb	%ymm30, -512(%rdx)	 # AVX512{F,VL} Disp8
	vpmovqb	%ymm30, -516(%rdx)	 # AVX512{F,VL}
	vpmovsqb	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpmovsqb	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovsqb	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovsqb	%xmm30, 254(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsqb	%xmm30, 256(%rdx)	 # AVX512{F,VL}
	vpmovsqb	%xmm30, -256(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsqb	%xmm30, -258(%rdx)	 # AVX512{F,VL}
	vpmovsqb	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpmovsqb	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovsqb	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovsqb	%ymm30, 508(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsqb	%ymm30, 512(%rdx)	 # AVX512{F,VL}
	vpmovsqb	%ymm30, -512(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsqb	%ymm30, -516(%rdx)	 # AVX512{F,VL}
	vpmovusqb	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpmovusqb	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovusqb	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovusqb	%xmm30, 254(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusqb	%xmm30, 256(%rdx)	 # AVX512{F,VL}
	vpmovusqb	%xmm30, -256(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusqb	%xmm30, -258(%rdx)	 # AVX512{F,VL}
	vpmovusqb	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpmovusqb	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovusqb	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovusqb	%ymm30, 508(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusqb	%ymm30, 512(%rdx)	 # AVX512{F,VL}
	vpmovusqb	%ymm30, -512(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusqb	%ymm30, -516(%rdx)	 # AVX512{F,VL}
	vpmovqw	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpmovqw	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovqw	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovqw	%xmm30, 508(%rdx)	 # AVX512{F,VL} Disp8
	vpmovqw	%xmm30, 512(%rdx)	 # AVX512{F,VL}
	vpmovqw	%xmm30, -512(%rdx)	 # AVX512{F,VL} Disp8
	vpmovqw	%xmm30, -516(%rdx)	 # AVX512{F,VL}
	vpmovqw	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpmovqw	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovqw	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovqw	%ymm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vpmovqw	%ymm30, 1024(%rdx)	 # AVX512{F,VL}
	vpmovqw	%ymm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vpmovqw	%ymm30, -1032(%rdx)	 # AVX512{F,VL}
	vpmovsqw	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpmovsqw	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovsqw	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovsqw	%xmm30, 508(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsqw	%xmm30, 512(%rdx)	 # AVX512{F,VL}
	vpmovsqw	%xmm30, -512(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsqw	%xmm30, -516(%rdx)	 # AVX512{F,VL}
	vpmovsqw	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpmovsqw	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovsqw	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovsqw	%ymm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsqw	%ymm30, 1024(%rdx)	 # AVX512{F,VL}
	vpmovsqw	%ymm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsqw	%ymm30, -1032(%rdx)	 # AVX512{F,VL}
	vpmovusqw	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpmovusqw	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovusqw	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovusqw	%xmm30, 508(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusqw	%xmm30, 512(%rdx)	 # AVX512{F,VL}
	vpmovusqw	%xmm30, -512(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusqw	%xmm30, -516(%rdx)	 # AVX512{F,VL}
	vpmovusqw	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpmovusqw	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovusqw	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovusqw	%ymm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusqw	%ymm30, 1024(%rdx)	 # AVX512{F,VL}
	vpmovusqw	%ymm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusqw	%ymm30, -1032(%rdx)	 # AVX512{F,VL}
	vpmovqd	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpmovqd	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovqd	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovqd	%xmm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vpmovqd	%xmm30, 1024(%rdx)	 # AVX512{F,VL}
	vpmovqd	%xmm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vpmovqd	%xmm30, -1032(%rdx)	 # AVX512{F,VL}
	vpmovqd	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpmovqd	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovqd	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovqd	%ymm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vpmovqd	%ymm30, 2048(%rdx)	 # AVX512{F,VL}
	vpmovqd	%ymm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vpmovqd	%ymm30, -2064(%rdx)	 # AVX512{F,VL}
	vpmovsqd	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpmovsqd	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovsqd	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovsqd	%xmm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsqd	%xmm30, 1024(%rdx)	 # AVX512{F,VL}
	vpmovsqd	%xmm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsqd	%xmm30, -1032(%rdx)	 # AVX512{F,VL}
	vpmovsqd	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpmovsqd	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovsqd	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovsqd	%ymm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsqd	%ymm30, 2048(%rdx)	 # AVX512{F,VL}
	vpmovsqd	%ymm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsqd	%ymm30, -2064(%rdx)	 # AVX512{F,VL}
	vpmovusqd	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpmovusqd	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovusqd	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovusqd	%xmm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusqd	%xmm30, 1024(%rdx)	 # AVX512{F,VL}
	vpmovusqd	%xmm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusqd	%xmm30, -1032(%rdx)	 # AVX512{F,VL}
	vpmovusqd	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpmovusqd	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovusqd	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovusqd	%ymm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusqd	%ymm30, 2048(%rdx)	 # AVX512{F,VL}
	vpmovusqd	%ymm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusqd	%ymm30, -2064(%rdx)	 # AVX512{F,VL}
	vpmovdb	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpmovdb	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovdb	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovdb	%xmm30, 508(%rdx)	 # AVX512{F,VL} Disp8
	vpmovdb	%xmm30, 512(%rdx)	 # AVX512{F,VL}
	vpmovdb	%xmm30, -512(%rdx)	 # AVX512{F,VL} Disp8
	vpmovdb	%xmm30, -516(%rdx)	 # AVX512{F,VL}
	vpmovdb	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpmovdb	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovdb	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovdb	%ymm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vpmovdb	%ymm30, 1024(%rdx)	 # AVX512{F,VL}
	vpmovdb	%ymm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vpmovdb	%ymm30, -1032(%rdx)	 # AVX512{F,VL}
	vpmovsdb	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpmovsdb	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovsdb	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovsdb	%xmm30, 508(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsdb	%xmm30, 512(%rdx)	 # AVX512{F,VL}
	vpmovsdb	%xmm30, -512(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsdb	%xmm30, -516(%rdx)	 # AVX512{F,VL}
	vpmovsdb	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpmovsdb	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovsdb	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovsdb	%ymm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsdb	%ymm30, 1024(%rdx)	 # AVX512{F,VL}
	vpmovsdb	%ymm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsdb	%ymm30, -1032(%rdx)	 # AVX512{F,VL}
	vpmovusdb	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpmovusdb	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovusdb	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovusdb	%xmm30, 508(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusdb	%xmm30, 512(%rdx)	 # AVX512{F,VL}
	vpmovusdb	%xmm30, -512(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusdb	%xmm30, -516(%rdx)	 # AVX512{F,VL}
	vpmovusdb	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpmovusdb	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovusdb	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovusdb	%ymm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusdb	%ymm30, 1024(%rdx)	 # AVX512{F,VL}
	vpmovusdb	%ymm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusdb	%ymm30, -1032(%rdx)	 # AVX512{F,VL}
	vpmovdw	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpmovdw	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovdw	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovdw	%xmm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vpmovdw	%xmm30, 1024(%rdx)	 # AVX512{F,VL}
	vpmovdw	%xmm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vpmovdw	%xmm30, -1032(%rdx)	 # AVX512{F,VL}
	vpmovdw	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpmovdw	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovdw	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovdw	%ymm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vpmovdw	%ymm30, 2048(%rdx)	 # AVX512{F,VL}
	vpmovdw	%ymm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vpmovdw	%ymm30, -2064(%rdx)	 # AVX512{F,VL}
	vpmovsdw	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpmovsdw	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovsdw	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovsdw	%xmm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsdw	%xmm30, 1024(%rdx)	 # AVX512{F,VL}
	vpmovsdw	%xmm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsdw	%xmm30, -1032(%rdx)	 # AVX512{F,VL}
	vpmovsdw	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpmovsdw	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovsdw	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovsdw	%ymm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsdw	%ymm30, 2048(%rdx)	 # AVX512{F,VL}
	vpmovsdw	%ymm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vpmovsdw	%ymm30, -2064(%rdx)	 # AVX512{F,VL}
	vpmovusdw	%xmm30, (%rcx)	 # AVX512{F,VL}
	vpmovusdw	%xmm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovusdw	%xmm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovusdw	%xmm30, 1016(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusdw	%xmm30, 1024(%rdx)	 # AVX512{F,VL}
	vpmovusdw	%xmm30, -1024(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusdw	%xmm30, -1032(%rdx)	 # AVX512{F,VL}
	vpmovusdw	%ymm30, (%rcx)	 # AVX512{F,VL}
	vpmovusdw	%ymm30, (%rcx){%k7}	 # AVX512{F,VL}
	vpmovusdw	%ymm30, 0x123(%rax,%r14,8)	 # AVX512{F,VL}
	vpmovusdw	%ymm30, 2032(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusdw	%ymm30, 2048(%rdx)	 # AVX512{F,VL}
	vpmovusdw	%ymm30, -2048(%rdx)	 # AVX512{F,VL} Disp8
	vpmovusdw	%ymm30, -2064(%rdx)	 # AVX512{F,VL}
	vcvttpd2udq	%xmm29, %xmm30	 # AVX512{F,VL}
	vcvttpd2udq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvttpd2udq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvttpd2udqx	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvttpd2udqx	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvttpd2udqx	(%rcx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvttpd2udqx	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2udqx	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vcvttpd2udqx	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2udqx	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vcvttpd2udqx	1016(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2udqx	1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvttpd2udqx	-1024(%rdx){1to2}, %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2udqx	-1032(%rdx){1to2}, %xmm30	 # AVX512{F,VL}
	vcvttpd2udq	%ymm29, %xmm30	 # AVX512{F,VL}
	vcvttpd2udq	%ymm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvttpd2udq	%ymm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvttpd2udqy	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvttpd2udqy	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvttpd2udqy	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvttpd2udqy	4064(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2udqy	4096(%rdx), %xmm30	 # AVX512{F,VL}
	vcvttpd2udqy	-4096(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2udqy	-4128(%rdx), %xmm30	 # AVX512{F,VL}
	vcvttpd2udqy	1016(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2udqy	1024(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvttpd2udqy	-1024(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvttpd2udqy	-1032(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvttps2udq	%xmm29, %xmm30	 # AVX512{F,VL}
	vcvttps2udq	%xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vcvttps2udq	%xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vcvttps2udq	(%rcx), %xmm30	 # AVX512{F,VL}
	vcvttps2udq	0x123(%rax,%r14,8), %xmm30	 # AVX512{F,VL}
	vcvttps2udq	(%rcx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvttps2udq	2032(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvttps2udq	2048(%rdx), %xmm30	 # AVX512{F,VL}
	vcvttps2udq	-2048(%rdx), %xmm30	 # AVX512{F,VL} Disp8
	vcvttps2udq	-2064(%rdx), %xmm30	 # AVX512{F,VL}
	vcvttps2udq	508(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvttps2udq	512(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvttps2udq	-512(%rdx){1to4}, %xmm30	 # AVX512{F,VL} Disp8
	vcvttps2udq	-516(%rdx){1to4}, %xmm30	 # AVX512{F,VL}
	vcvttps2udq	%ymm29, %ymm30	 # AVX512{F,VL}
	vcvttps2udq	%ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vcvttps2udq	%ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vcvttps2udq	(%rcx), %ymm30	 # AVX512{F,VL}
	vcvttps2udq	0x123(%rax,%r14,8), %ymm30	 # AVX512{F,VL}
	vcvttps2udq	(%rcx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvttps2udq	4064(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvttps2udq	4096(%rdx), %ymm30	 # AVX512{F,VL}
	vcvttps2udq	-4096(%rdx), %ymm30	 # AVX512{F,VL} Disp8
	vcvttps2udq	-4128(%rdx), %ymm30	 # AVX512{F,VL}
	vcvttps2udq	508(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vcvttps2udq	512(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vcvttps2udq	-512(%rdx){1to8}, %ymm30	 # AVX512{F,VL} Disp8
	vcvttps2udq	-516(%rdx){1to8}, %ymm30	 # AVX512{F,VL}
	vpermi2d	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2d	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpermi2d	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpermi2d	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2d	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2d	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2d	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2d	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2d	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2d	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2d	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2d	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2d	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2d	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2d	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2d	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermi2d	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermi2d	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2d	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2d	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2d	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2d	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2d	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2d	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2d	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2d	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2d	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2d	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2q	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2q	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpermi2q	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpermi2q	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2q	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2q	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2q	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2q	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2q	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2q	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2q	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2q	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2q	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2q	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2q	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2q	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermi2q	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermi2q	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2q	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2q	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2q	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2q	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2q	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2q	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2q	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2q	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2q	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2q	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2ps	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2ps	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpermi2ps	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpermi2ps	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2ps	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2ps	(%rcx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2ps	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2ps	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2ps	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2ps	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2ps	508(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2ps	512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2ps	-512(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2ps	-516(%rdx){1to4}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2ps	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2ps	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermi2ps	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermi2ps	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2ps	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2ps	(%rcx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2ps	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2ps	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2ps	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2ps	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2ps	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2ps	512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2ps	-512(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2ps	-516(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2pd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2pd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{F,VL}
	vpermi2pd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{F,VL}
	vpermi2pd	(%rcx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2pd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2pd	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2pd	2032(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2pd	2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2pd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2pd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2pd	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2pd	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2pd	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL} Disp8
	vpermi2pd	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{F,VL}
	vpermi2pd	%ymm28, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2pd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{F,VL}
	vpermi2pd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{F,VL}
	vpermi2pd	(%rcx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2pd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2pd	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2pd	4064(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2pd	4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2pd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2pd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2pd	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2pd	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vpermi2pd	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL} Disp8
	vpermi2pd	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{F,VL}
	vptestnmd	%xmm28, %xmm29, %k5	 # AVX512{F,VL}
	vptestnmd	%xmm28, %xmm29, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	(%rcx), %xmm29, %k5	 # AVX512{F,VL}
	vptestnmd	0x123(%rax,%r14,8), %xmm29, %k5	 # AVX512{F,VL}
	vptestnmd	(%rcx){1to4}, %xmm29, %k5	 # AVX512{F,VL}
	vptestnmd	2032(%rdx), %xmm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmd	2048(%rdx), %xmm29, %k5	 # AVX512{F,VL}
	vptestnmd	-2048(%rdx), %xmm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmd	-2064(%rdx), %xmm29, %k5	 # AVX512{F,VL}
	vptestnmd	508(%rdx){1to4}, %xmm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmd	512(%rdx){1to4}, %xmm29, %k5	 # AVX512{F,VL}
	vptestnmd	-512(%rdx){1to4}, %xmm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmd	-516(%rdx){1to4}, %xmm29, %k5	 # AVX512{F,VL}
	vptestnmd	%ymm28, %ymm29, %k5	 # AVX512{F,VL}
	vptestnmd	%ymm28, %ymm29, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	(%rcx), %ymm29, %k5	 # AVX512{F,VL}
	vptestnmd	0x123(%rax,%r14,8), %ymm29, %k5	 # AVX512{F,VL}
	vptestnmd	(%rcx){1to8}, %ymm29, %k5	 # AVX512{F,VL}
	vptestnmd	4064(%rdx), %ymm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmd	4096(%rdx), %ymm29, %k5	 # AVX512{F,VL}
	vptestnmd	-4096(%rdx), %ymm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmd	-4128(%rdx), %ymm29, %k5	 # AVX512{F,VL}
	vptestnmd	508(%rdx){1to8}, %ymm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmd	512(%rdx){1to8}, %ymm29, %k5	 # AVX512{F,VL}
	vptestnmd	-512(%rdx){1to8}, %ymm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmd	-516(%rdx){1to8}, %ymm29, %k5	 # AVX512{F,VL}
	vptestnmq	%xmm28, %xmm29, %k5	 # AVX512{F,VL}
	vptestnmq	%xmm28, %xmm29, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	(%rcx), %xmm29, %k5	 # AVX512{F,VL}
	vptestnmq	0x123(%rax,%r14,8), %xmm29, %k5	 # AVX512{F,VL}
	vptestnmq	(%rcx){1to2}, %xmm29, %k5	 # AVX512{F,VL}
	vptestnmq	2032(%rdx), %xmm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmq	2048(%rdx), %xmm29, %k5	 # AVX512{F,VL}
	vptestnmq	-2048(%rdx), %xmm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmq	-2064(%rdx), %xmm29, %k5	 # AVX512{F,VL}
	vptestnmq	1016(%rdx){1to2}, %xmm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmq	1024(%rdx){1to2}, %xmm29, %k5	 # AVX512{F,VL}
	vptestnmq	-1024(%rdx){1to2}, %xmm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmq	-1032(%rdx){1to2}, %xmm29, %k5	 # AVX512{F,VL}
	vptestnmq	%ymm28, %ymm29, %k5	 # AVX512{F,VL}
	vptestnmq	%ymm28, %ymm29, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	(%rcx), %ymm29, %k5	 # AVX512{F,VL}
	vptestnmq	0x123(%rax,%r14,8), %ymm29, %k5	 # AVX512{F,VL}
	vptestnmq	(%rcx){1to4}, %ymm29, %k5	 # AVX512{F,VL}
	vptestnmq	4064(%rdx), %ymm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmq	4096(%rdx), %ymm29, %k5	 # AVX512{F,VL}
	vptestnmq	-4096(%rdx), %ymm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmq	-4128(%rdx), %ymm29, %k5	 # AVX512{F,VL}
	vptestnmq	1016(%rdx){1to4}, %ymm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmq	1024(%rdx){1to4}, %ymm29, %k5	 # AVX512{F,VL}
	vptestnmq	-1024(%rdx){1to4}, %ymm29, %k5	 # AVX512{F,VL} Disp8
	vptestnmq	-1032(%rdx){1to4}, %ymm29, %k5	 # AVX512{F,VL}

	.intel_syntax noprefix
	vaddpd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vaddpd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vaddpd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vaddpd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vaddpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vaddpd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vaddpd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vaddpd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vaddpd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vaddpd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vaddpd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vaddpd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vaddpd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vaddpd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vaddpd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vaddpd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vaddpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vaddpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vaddpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vaddpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vaddpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vaddpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vaddpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vaddpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vaddpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vaddpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vaddpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vaddpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vaddps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vaddps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vaddps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vaddps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vaddps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vaddps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vaddps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vaddps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vaddps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vaddps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vaddps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vaddps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vaddps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vaddps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vaddps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vaddps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vaddps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vaddps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vaddps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vaddps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vaddps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vaddps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vaddps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vaddps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vaddps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vaddps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vaddps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vaddps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	valignd	xmm30, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	valignd	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	valignd	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	valignd	xmm30, xmm29, xmm28, 123	 # AVX512{F,VL}
	valignd	xmm30, xmm29, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	valignd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	valignd	xmm30, xmm29, [rcx]{1to4}, 123	 # AVX512{F,VL}
	valignd	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	valignd	xmm30, xmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	valignd	xmm30, xmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	valignd	xmm30, xmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	valignd	xmm30, xmm29, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	valignd	xmm30, xmm29, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	valignd	xmm30, xmm29, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	valignd	xmm30, xmm29, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	valignd	ymm30, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	valignd	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	valignd	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	valignd	ymm30, ymm29, ymm28, 123	 # AVX512{F,VL}
	valignd	ymm30, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	valignd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	valignd	ymm30, ymm29, [rcx]{1to8}, 123	 # AVX512{F,VL}
	valignd	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	valignd	ymm30, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	valignd	ymm30, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	valignd	ymm30, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	valignd	ymm30, ymm29, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	valignd	ymm30, ymm29, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	valignd	ymm30, ymm29, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	valignd	ymm30, ymm29, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vblendmpd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vblendmpd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vblendmpd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vblendmpd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vblendmpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vblendmpd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vblendmpd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vblendmpd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vblendmpd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vblendmpd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vblendmpd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vblendmpd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vblendmpd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vblendmpd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vblendmpd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vblendmpd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vblendmpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vblendmpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vblendmpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vblendmpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vblendmpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vblendmpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vblendmpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vblendmpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vblendmpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vblendmpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vblendmpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vblendmpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vblendmps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vblendmps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vblendmps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vblendmps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vblendmps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vblendmps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vblendmps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vblendmps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vblendmps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vblendmps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vblendmps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vblendmps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vblendmps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vblendmps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vblendmps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vblendmps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vblendmps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vblendmps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vblendmps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vblendmps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vblendmps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vblendmps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vblendmps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vblendmps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vblendmps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vblendmps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vblendmps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vblendmps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vbroadcastf32x4	ymm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vbroadcastf32x4	ymm30{k7}, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vbroadcastf32x4	ymm30{k7}{z}, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vbroadcastf32x4	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vbroadcastf32x4	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vbroadcastf32x4	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vbroadcastf32x4	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vbroadcastf32x4	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vbroadcasti32x4	ymm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vbroadcasti32x4	ymm30{k7}, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vbroadcasti32x4	ymm30{k7}{z}, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vbroadcasti32x4	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vbroadcasti32x4	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vbroadcasti32x4	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vbroadcasti32x4	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vbroadcasti32x4	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vbroadcastsd	ymm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vbroadcastsd	ymm30{k7}, QWORD PTR [rcx]	 # AVX512{F,VL}
	vbroadcastsd	ymm30{k7}{z}, QWORD PTR [rcx]	 # AVX512{F,VL}
	vbroadcastsd	ymm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vbroadcastsd	ymm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vbroadcastsd	ymm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vbroadcastsd	ymm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vbroadcastsd	ymm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vbroadcastsd	ymm30, xmm29	 # AVX512{F,VL}
	vbroadcastsd	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vbroadcastsd	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vbroadcastss	xmm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vbroadcastss	xmm30{k7}, DWORD PTR [rcx]	 # AVX512{F,VL}
	vbroadcastss	xmm30{k7}{z}, DWORD PTR [rcx]	 # AVX512{F,VL}
	vbroadcastss	xmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vbroadcastss	xmm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vbroadcastss	xmm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vbroadcastss	xmm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vbroadcastss	xmm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vbroadcastss	ymm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vbroadcastss	ymm30{k7}, DWORD PTR [rcx]	 # AVX512{F,VL}
	vbroadcastss	ymm30{k7}{z}, DWORD PTR [rcx]	 # AVX512{F,VL}
	vbroadcastss	ymm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vbroadcastss	ymm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vbroadcastss	ymm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vbroadcastss	ymm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vbroadcastss	ymm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vbroadcastss	xmm30, xmm29	 # AVX512{F,VL}
	vbroadcastss	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vbroadcastss	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vbroadcastss	ymm30, xmm29	 # AVX512{F,VL}
	vbroadcastss	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vbroadcastss	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcmppd	k5, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vcmppd	k5{k7}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vcmppd	k5, xmm29, xmm28, 123	 # AVX512{F,VL}
	vcmppd	k5, xmm29, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vcmppd	k5, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vcmppd	k5, xmm29, [rcx]{1to2}, 123	 # AVX512{F,VL}
	vcmppd	k5, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vcmppd	k5, xmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vcmppd	k5, xmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vcmppd	k5, xmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vcmppd	k5, xmm29, [rdx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vcmppd	k5, xmm29, [rdx+1024]{1to2}, 123	 # AVX512{F,VL}
	vcmppd	k5, xmm29, [rdx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vcmppd	k5, xmm29, [rdx-1032]{1to2}, 123	 # AVX512{F,VL}
	vcmppd	k5, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vcmppd	k5{k7}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vcmppd	k5, ymm29, ymm28, 123	 # AVX512{F,VL}
	vcmppd	k5, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vcmppd	k5, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vcmppd	k5, ymm29, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vcmppd	k5, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vcmppd	k5, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vcmppd	k5, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vcmppd	k5, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vcmppd	k5, ymm29, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vcmppd	k5, ymm29, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vcmppd	k5, ymm29, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vcmppd	k5, ymm29, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vcmpps	k5, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vcmpps	k5{k7}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vcmpps	k5, xmm29, xmm28, 123	 # AVX512{F,VL}
	vcmpps	k5, xmm29, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vcmpps	k5, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vcmpps	k5, xmm29, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vcmpps	k5, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vcmpps	k5, xmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vcmpps	k5, xmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vcmpps	k5, xmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vcmpps	k5, xmm29, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vcmpps	k5, xmm29, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	vcmpps	k5, xmm29, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vcmpps	k5, xmm29, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	vcmpps	k5, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vcmpps	k5{k7}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vcmpps	k5, ymm29, ymm28, 123	 # AVX512{F,VL}
	vcmpps	k5, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vcmpps	k5, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vcmpps	k5, ymm29, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vcmpps	k5, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vcmpps	k5, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vcmpps	k5, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vcmpps	k5, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vcmpps	k5, ymm29, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vcmpps	k5, ymm29, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vcmpps	k5, ymm29, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vcmpps	k5, ymm29, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vcompresspd	XMMWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vcompresspd	XMMWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vcompresspd	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vcompresspd	XMMWORD PTR [rdx+1016], xmm30	 # AVX512{F,VL} Disp8
	vcompresspd	XMMWORD PTR [rdx+1024], xmm30	 # AVX512{F,VL}
	vcompresspd	XMMWORD PTR [rdx-1024], xmm30	 # AVX512{F,VL} Disp8
	vcompresspd	XMMWORD PTR [rdx-1032], xmm30	 # AVX512{F,VL}
	vcompresspd	YMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vcompresspd	YMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vcompresspd	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vcompresspd	YMMWORD PTR [rdx+1016], ymm30	 # AVX512{F,VL} Disp8
	vcompresspd	YMMWORD PTR [rdx+1024], ymm30	 # AVX512{F,VL}
	vcompresspd	YMMWORD PTR [rdx-1024], ymm30	 # AVX512{F,VL} Disp8
	vcompresspd	YMMWORD PTR [rdx-1032], ymm30	 # AVX512{F,VL}
	vcompresspd	xmm30, xmm29	 # AVX512{F,VL}
	vcompresspd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcompresspd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcompresspd	ymm30, ymm29	 # AVX512{F,VL}
	vcompresspd	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vcompresspd	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vcompressps	XMMWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vcompressps	XMMWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vcompressps	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vcompressps	XMMWORD PTR [rdx+508], xmm30	 # AVX512{F,VL} Disp8
	vcompressps	XMMWORD PTR [rdx+512], xmm30	 # AVX512{F,VL}
	vcompressps	XMMWORD PTR [rdx-512], xmm30	 # AVX512{F,VL} Disp8
	vcompressps	XMMWORD PTR [rdx-516], xmm30	 # AVX512{F,VL}
	vcompressps	YMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vcompressps	YMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vcompressps	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vcompressps	YMMWORD PTR [rdx+508], ymm30	 # AVX512{F,VL} Disp8
	vcompressps	YMMWORD PTR [rdx+512], ymm30	 # AVX512{F,VL}
	vcompressps	YMMWORD PTR [rdx-512], ymm30	 # AVX512{F,VL} Disp8
	vcompressps	YMMWORD PTR [rdx-516], ymm30	 # AVX512{F,VL}
	vcompressps	xmm30, xmm29	 # AVX512{F,VL}
	vcompressps	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcompressps	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcompressps	ymm30, ymm29	 # AVX512{F,VL}
	vcompressps	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vcompressps	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vcvtdq2pd	xmm30, xmm29	 # AVX512{F,VL}
	vcvtdq2pd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcvtdq2pd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvtdq2pd	xmm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtdq2pd	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtdq2pd	xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vcvtdq2pd	xmm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vcvtdq2pd	xmm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vcvtdq2pd	xmm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vcvtdq2pd	xmm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vcvtdq2pd	xmm30, [rdx+508]{1to2}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	xmm30, [rdx+512]{1to2}	 # AVX512{F,VL}
	vcvtdq2pd	xmm30, [rdx-512]{1to2}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	xmm30, [rdx-516]{1to2}	 # AVX512{F,VL}
	vcvtdq2pd	ymm30, xmm29	 # AVX512{F,VL}
	vcvtdq2pd	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vcvtdq2pd	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvtdq2pd	ymm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtdq2pd	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtdq2pd	ymm30, [rcx]{1to4}	 # AVX512{F,VL}
	vcvtdq2pd	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vcvtdq2pd	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vcvtdq2pd	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vcvtdq2pd	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vcvtdq2pd	ymm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	ymm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vcvtdq2pd	ymm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	ymm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vcvtdq2ps	xmm30, xmm29	 # AVX512{F,VL}
	vcvtdq2ps	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcvtdq2ps	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvtdq2ps	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtdq2ps	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtdq2ps	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vcvtdq2ps	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vcvtdq2ps	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vcvtdq2ps	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vcvtdq2ps	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vcvtdq2ps	xmm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	xmm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vcvtdq2ps	xmm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	xmm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vcvtdq2ps	ymm30, ymm29	 # AVX512{F,VL}
	vcvtdq2ps	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vcvtdq2ps	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vcvtdq2ps	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtdq2ps	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtdq2ps	ymm30, [rcx]{1to8}	 # AVX512{F,VL}
	vcvtdq2ps	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vcvtdq2ps	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vcvtdq2ps	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vcvtdq2ps	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vcvtdq2ps	ymm30, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	ymm30, [rdx+512]{1to8}	 # AVX512{F,VL}
	vcvtdq2ps	ymm30, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	ymm30, [rdx-516]{1to8}	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, xmm29	 # AVX512{F,VL}
	vcvtpd2dq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcvtpd2dq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, QWORD BCST [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm30, QWORD BCST [rdx+1024]{1to2}	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, QWORD BCST [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm30, QWORD BCST [rdx-1032]{1to2}	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, ymm29	 # AVX512{F,VL}
	vcvtpd2dq	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vcvtpd2dq	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, QWORD BCST [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm30, QWORD BCST [rdx+1024]{1to4}	 # AVX512{F,VL}
	vcvtpd2dq	xmm30, QWORD BCST [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm30, QWORD BCST [rdx-1032]{1to4}	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, xmm29	 # AVX512{F,VL}
	vcvtpd2ps	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcvtpd2ps	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, QWORD BCST [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm30, QWORD BCST [rdx+1024]{1to2}	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, QWORD BCST [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm30, QWORD BCST [rdx-1032]{1to2}	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, ymm29	 # AVX512{F,VL}
	vcvtpd2ps	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vcvtpd2ps	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, QWORD BCST [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm30, QWORD BCST [rdx+1024]{1to4}	 # AVX512{F,VL}
	vcvtpd2ps	xmm30, QWORD BCST [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm30, QWORD BCST [rdx-1032]{1to4}	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, xmm29	 # AVX512{F,VL}
	vcvtpd2udq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcvtpd2udq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, QWORD BCST [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm30, QWORD BCST [rdx+1024]{1to2}	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, QWORD BCST [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm30, QWORD BCST [rdx-1032]{1to2}	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, ymm29	 # AVX512{F,VL}
	vcvtpd2udq	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vcvtpd2udq	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, QWORD BCST [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm30, QWORD BCST [rdx+1024]{1to4}	 # AVX512{F,VL}
	vcvtpd2udq	xmm30, QWORD BCST [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm30, QWORD BCST [rdx-1032]{1to4}	 # AVX512{F,VL}
	vcvtph2ps	xmm30, xmm29	 # AVX512{F,VL}
	vcvtph2ps	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcvtph2ps	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvtph2ps	xmm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtph2ps	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtph2ps	xmm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vcvtph2ps	xmm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vcvtph2ps	xmm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vcvtph2ps	xmm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vcvtph2ps	ymm30, xmm29	 # AVX512{F,VL}
	vcvtph2ps	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vcvtph2ps	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvtph2ps	ymm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtph2ps	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtph2ps	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vcvtph2ps	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vcvtph2ps	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vcvtph2ps	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vcvtps2dq	xmm30, xmm29	 # AVX512{F,VL}
	vcvtps2dq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcvtps2dq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvtps2dq	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtps2dq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtps2dq	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vcvtps2dq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vcvtps2dq	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vcvtps2dq	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vcvtps2dq	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vcvtps2dq	xmm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvtps2dq	xmm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vcvtps2dq	xmm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvtps2dq	xmm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vcvtps2dq	ymm30, ymm29	 # AVX512{F,VL}
	vcvtps2dq	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vcvtps2dq	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vcvtps2dq	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtps2dq	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtps2dq	ymm30, [rcx]{1to8}	 # AVX512{F,VL}
	vcvtps2dq	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vcvtps2dq	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vcvtps2dq	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vcvtps2dq	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vcvtps2dq	ymm30, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vcvtps2dq	ymm30, [rdx+512]{1to8}	 # AVX512{F,VL}
	vcvtps2dq	ymm30, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vcvtps2dq	ymm30, [rdx-516]{1to8}	 # AVX512{F,VL}
	vcvtps2pd	xmm30, xmm29	 # AVX512{F,VL}
	vcvtps2pd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcvtps2pd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvtps2pd	xmm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtps2pd	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtps2pd	xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vcvtps2pd	xmm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vcvtps2pd	xmm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vcvtps2pd	xmm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vcvtps2pd	xmm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vcvtps2pd	xmm30, [rdx+508]{1to2}	 # AVX512{F,VL} Disp8
	vcvtps2pd	xmm30, [rdx+512]{1to2}	 # AVX512{F,VL}
	vcvtps2pd	xmm30, [rdx-512]{1to2}	 # AVX512{F,VL} Disp8
	vcvtps2pd	xmm30, [rdx-516]{1to2}	 # AVX512{F,VL}
	vcvtps2pd	ymm30, xmm29	 # AVX512{F,VL}
	vcvtps2pd	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vcvtps2pd	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvtps2pd	ymm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtps2pd	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtps2pd	ymm30, [rcx]{1to4}	 # AVX512{F,VL}
	vcvtps2pd	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vcvtps2pd	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vcvtps2pd	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vcvtps2pd	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vcvtps2pd	ymm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvtps2pd	ymm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vcvtps2pd	ymm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvtps2pd	ymm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vcvtps2ph	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vcvtps2ph	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vcvtps2ph	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vcvtps2ph	xmm30, xmm29, 123	 # AVX512{F,VL}
	vcvtps2ph	xmm30, ymm29, 0xab	 # AVX512{F,VL}
	vcvtps2ph	xmm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vcvtps2ph	xmm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vcvtps2ph	xmm30, ymm29, 123	 # AVX512{F,VL}
	vcvtps2udq	xmm30, xmm29	 # AVX512{F,VL}
	vcvtps2udq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcvtps2udq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvtps2udq	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtps2udq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtps2udq	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vcvtps2udq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vcvtps2udq	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vcvtps2udq	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vcvtps2udq	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vcvtps2udq	xmm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvtps2udq	xmm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vcvtps2udq	xmm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvtps2udq	xmm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vcvtps2udq	ymm30, ymm29	 # AVX512{F,VL}
	vcvtps2udq	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vcvtps2udq	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vcvtps2udq	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtps2udq	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtps2udq	ymm30, [rcx]{1to8}	 # AVX512{F,VL}
	vcvtps2udq	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vcvtps2udq	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vcvtps2udq	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vcvtps2udq	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vcvtps2udq	ymm30, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vcvtps2udq	ymm30, [rdx+512]{1to8}	 # AVX512{F,VL}
	vcvtps2udq	ymm30, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vcvtps2udq	ymm30, [rdx-516]{1to8}	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, xmm29	 # AVX512{F,VL}
	vcvttpd2dq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcvttpd2dq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, QWORD BCST [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm30, QWORD BCST [rdx+1024]{1to2}	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, QWORD BCST [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm30, QWORD BCST [rdx-1032]{1to2}	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, ymm29	 # AVX512{F,VL}
	vcvttpd2dq	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vcvttpd2dq	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, QWORD BCST [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm30, QWORD BCST [rdx+1024]{1to4}	 # AVX512{F,VL}
	vcvttpd2dq	xmm30, QWORD BCST [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm30, QWORD BCST [rdx-1032]{1to4}	 # AVX512{F,VL}
	vcvttps2dq	xmm30, xmm29	 # AVX512{F,VL}
	vcvttps2dq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcvttps2dq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvttps2dq	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvttps2dq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvttps2dq	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vcvttps2dq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vcvttps2dq	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vcvttps2dq	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vcvttps2dq	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vcvttps2dq	xmm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvttps2dq	xmm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vcvttps2dq	xmm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvttps2dq	xmm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vcvttps2dq	ymm30, ymm29	 # AVX512{F,VL}
	vcvttps2dq	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vcvttps2dq	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vcvttps2dq	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvttps2dq	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvttps2dq	ymm30, [rcx]{1to8}	 # AVX512{F,VL}
	vcvttps2dq	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vcvttps2dq	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vcvttps2dq	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vcvttps2dq	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vcvttps2dq	ymm30, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vcvttps2dq	ymm30, [rdx+512]{1to8}	 # AVX512{F,VL}
	vcvttps2dq	ymm30, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vcvttps2dq	ymm30, [rdx-516]{1to8}	 # AVX512{F,VL}
	vcvtudq2pd	xmm30, xmm29	 # AVX512{F,VL}
	vcvtudq2pd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcvtudq2pd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvtudq2pd	xmm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtudq2pd	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtudq2pd	xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vcvtudq2pd	xmm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vcvtudq2pd	xmm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vcvtudq2pd	xmm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vcvtudq2pd	xmm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vcvtudq2pd	xmm30, [rdx+508]{1to2}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	xmm30, [rdx+512]{1to2}	 # AVX512{F,VL}
	vcvtudq2pd	xmm30, [rdx-512]{1to2}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	xmm30, [rdx-516]{1to2}	 # AVX512{F,VL}
	vcvtudq2pd	ymm30, xmm29	 # AVX512{F,VL}
	vcvtudq2pd	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vcvtudq2pd	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvtudq2pd	ymm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtudq2pd	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtudq2pd	ymm30, [rcx]{1to4}	 # AVX512{F,VL}
	vcvtudq2pd	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vcvtudq2pd	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vcvtudq2pd	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vcvtudq2pd	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vcvtudq2pd	ymm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	ymm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vcvtudq2pd	ymm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	ymm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vcvtudq2ps	xmm30, xmm29	 # AVX512{F,VL}
	vcvtudq2ps	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcvtudq2ps	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvtudq2ps	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtudq2ps	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtudq2ps	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vcvtudq2ps	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vcvtudq2ps	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vcvtudq2ps	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vcvtudq2ps	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vcvtudq2ps	xmm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	xmm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vcvtudq2ps	xmm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	xmm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vcvtudq2ps	ymm30, ymm29	 # AVX512{F,VL}
	vcvtudq2ps	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vcvtudq2ps	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vcvtudq2ps	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvtudq2ps	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvtudq2ps	ymm30, [rcx]{1to8}	 # AVX512{F,VL}
	vcvtudq2ps	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vcvtudq2ps	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vcvtudq2ps	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vcvtudq2ps	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vcvtudq2ps	ymm30, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	ymm30, [rdx+512]{1to8}	 # AVX512{F,VL}
	vcvtudq2ps	ymm30, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	ymm30, [rdx-516]{1to8}	 # AVX512{F,VL}
	vdivpd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vdivpd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vdivpd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vdivpd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vdivpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vdivpd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vdivpd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vdivpd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vdivpd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vdivpd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vdivpd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vdivpd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vdivpd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vdivpd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vdivpd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vdivpd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vdivpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vdivpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vdivpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vdivpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vdivpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vdivpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vdivpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vdivpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vdivpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vdivpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vdivpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vdivpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vdivps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vdivps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vdivps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vdivps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vdivps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vdivps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vdivps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vdivps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vdivps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vdivps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vdivps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vdivps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vdivps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vdivps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vdivps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vdivps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vdivps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vdivps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vdivps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vdivps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vdivps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vdivps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vdivps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vdivps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vdivps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vdivps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vdivps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vdivps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vexpandpd	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vexpandpd	xmm30{k7}, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vexpandpd	xmm30{k7}{z}, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vexpandpd	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vexpandpd	xmm30, XMMWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vexpandpd	xmm30, XMMWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vexpandpd	xmm30, XMMWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vexpandpd	xmm30, XMMWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vexpandpd	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vexpandpd	ymm30{k7}, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vexpandpd	ymm30{k7}{z}, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vexpandpd	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vexpandpd	ymm30, YMMWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vexpandpd	ymm30, YMMWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vexpandpd	ymm30, YMMWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vexpandpd	ymm30, YMMWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vexpandpd	xmm30, xmm29	 # AVX512{F,VL}
	vexpandpd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vexpandpd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vexpandpd	ymm30, ymm29	 # AVX512{F,VL}
	vexpandpd	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vexpandpd	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vexpandps	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vexpandps	xmm30{k7}, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vexpandps	xmm30{k7}{z}, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vexpandps	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vexpandps	xmm30, XMMWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vexpandps	xmm30, XMMWORD PTR [rdx+512]	 # AVX512{F,VL}
	vexpandps	xmm30, XMMWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vexpandps	xmm30, XMMWORD PTR [rdx-516]	 # AVX512{F,VL}
	vexpandps	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vexpandps	ymm30{k7}, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vexpandps	ymm30{k7}{z}, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vexpandps	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vexpandps	ymm30, YMMWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vexpandps	ymm30, YMMWORD PTR [rdx+512]	 # AVX512{F,VL}
	vexpandps	ymm30, YMMWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vexpandps	ymm30, YMMWORD PTR [rdx-516]	 # AVX512{F,VL}
	vexpandps	xmm30, xmm29	 # AVX512{F,VL}
	vexpandps	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vexpandps	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vexpandps	ymm30, ymm29	 # AVX512{F,VL}
	vexpandps	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vexpandps	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vextractf32x4	xmm30, ymm29, 0xab	 # AVX512{F,VL}
	vextractf32x4	xmm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vextractf32x4	xmm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vextractf32x4	xmm30, ymm29, 123	 # AVX512{F,VL}
	vextracti32x4	xmm30, ymm29, 0xab	 # AVX512{F,VL}
	vextracti32x4	xmm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vextracti32x4	xmm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vextracti32x4	xmm30, ymm29, 123	 # AVX512{F,VL}
	vfmadd132pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd132pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd132pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd132pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmadd132pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmadd132pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfmadd132pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmadd132pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmadd132pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmadd132pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmadd132pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmadd132pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfmadd132pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmadd132pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfmadd132pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd132pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd132pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd132pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmadd132pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmadd132pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmadd132pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmadd132pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmadd132pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmadd132pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmadd132pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd132pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfmadd132pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd132pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfmadd132ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd132ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd132ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd132ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmadd132ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmadd132ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmadd132ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmadd132ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmadd132ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmadd132ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmadd132ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd132ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfmadd132ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd132ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfmadd132ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd132ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd132ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd132ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmadd132ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmadd132ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfmadd132ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmadd132ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmadd132ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmadd132ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmadd132ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmadd132ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfmadd132ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmadd132ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfmadd213pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd213pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd213pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd213pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmadd213pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmadd213pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfmadd213pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmadd213pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmadd213pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmadd213pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmadd213pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmadd213pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfmadd213pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmadd213pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfmadd213pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd213pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd213pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd213pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmadd213pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmadd213pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmadd213pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmadd213pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmadd213pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmadd213pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmadd213pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd213pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfmadd213pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd213pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfmadd213ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd213ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd213ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd213ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmadd213ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmadd213ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmadd213ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmadd213ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmadd213ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmadd213ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmadd213ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd213ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfmadd213ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd213ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfmadd213ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd213ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd213ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd213ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmadd213ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmadd213ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfmadd213ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmadd213ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmadd213ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmadd213ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmadd213ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmadd213ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfmadd213ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmadd213ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfmadd231pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd231pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd231pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd231pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmadd231pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmadd231pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfmadd231pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmadd231pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmadd231pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmadd231pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmadd231pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmadd231pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfmadd231pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmadd231pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfmadd231pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd231pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd231pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd231pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmadd231pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmadd231pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmadd231pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmadd231pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmadd231pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmadd231pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmadd231pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd231pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfmadd231pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd231pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfmadd231ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd231ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd231ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmadd231ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmadd231ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmadd231ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmadd231ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmadd231ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmadd231ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmadd231ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmadd231ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd231ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfmadd231ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd231ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfmadd231ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd231ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd231ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmadd231ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmadd231ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmadd231ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfmadd231ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmadd231ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmadd231ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmadd231ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmadd231ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmadd231ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfmadd231ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmadd231ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfmaddsub132pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub132pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub132pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub132pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmaddsub132pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmaddsub132pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfmaddsub132pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmaddsub132pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmaddsub132pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfmaddsub132pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfmaddsub132pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub132pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub132pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub132pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmaddsub132pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmaddsub132pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmaddsub132pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmaddsub132pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmaddsub132pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfmaddsub132pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfmaddsub132ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub132ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub132ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub132ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmaddsub132ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmaddsub132ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmaddsub132ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmaddsub132ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmaddsub132ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfmaddsub132ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfmaddsub132ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub132ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub132ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub132ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmaddsub132ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmaddsub132ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfmaddsub132ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmaddsub132ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmaddsub132ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfmaddsub132ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfmaddsub213pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub213pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub213pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub213pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmaddsub213pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmaddsub213pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfmaddsub213pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmaddsub213pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmaddsub213pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfmaddsub213pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfmaddsub213pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub213pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub213pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub213pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmaddsub213pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmaddsub213pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmaddsub213pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmaddsub213pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmaddsub213pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfmaddsub213pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfmaddsub213ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub213ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub213ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub213ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmaddsub213ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmaddsub213ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmaddsub213ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmaddsub213ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmaddsub213ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfmaddsub213ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfmaddsub213ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub213ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub213ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub213ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmaddsub213ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmaddsub213ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfmaddsub213ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmaddsub213ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmaddsub213ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfmaddsub213ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfmaddsub231pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub231pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub231pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub231pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmaddsub231pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmaddsub231pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfmaddsub231pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmaddsub231pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmaddsub231pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfmaddsub231pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfmaddsub231pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub231pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub231pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub231pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmaddsub231pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmaddsub231pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmaddsub231pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmaddsub231pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmaddsub231pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfmaddsub231pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfmaddsub231ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub231ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub231ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmaddsub231ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmaddsub231ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmaddsub231ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmaddsub231ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmaddsub231ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmaddsub231ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfmaddsub231ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfmaddsub231ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub231ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub231ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmaddsub231ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmaddsub231ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmaddsub231ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfmaddsub231ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmaddsub231ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmaddsub231ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfmaddsub231ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfmsub132pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub132pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub132pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub132pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsub132pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsub132pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfmsub132pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmsub132pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmsub132pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmsub132pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmsub132pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmsub132pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfmsub132pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmsub132pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfmsub132pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub132pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub132pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub132pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsub132pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsub132pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmsub132pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmsub132pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmsub132pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmsub132pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmsub132pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub132pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfmsub132pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub132pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfmsub132ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub132ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub132ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub132ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsub132ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsub132ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmsub132ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmsub132ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmsub132ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmsub132ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmsub132ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub132ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfmsub132ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub132ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfmsub132ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub132ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub132ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub132ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsub132ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsub132ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfmsub132ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmsub132ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmsub132ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmsub132ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmsub132ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmsub132ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfmsub132ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmsub132ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfmsub213pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub213pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub213pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub213pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsub213pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsub213pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfmsub213pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmsub213pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmsub213pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmsub213pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmsub213pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmsub213pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfmsub213pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmsub213pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfmsub213pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub213pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub213pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub213pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsub213pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsub213pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmsub213pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmsub213pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmsub213pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmsub213pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmsub213pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub213pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfmsub213pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub213pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfmsub213ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub213ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub213ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub213ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsub213ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsub213ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmsub213ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmsub213ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmsub213ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmsub213ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmsub213ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub213ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfmsub213ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub213ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfmsub213ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub213ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub213ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub213ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsub213ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsub213ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfmsub213ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmsub213ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmsub213ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmsub213ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmsub213ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmsub213ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfmsub213ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmsub213ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfmsub231pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub231pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub231pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub231pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsub231pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsub231pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfmsub231pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmsub231pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmsub231pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmsub231pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmsub231pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmsub231pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfmsub231pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmsub231pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfmsub231pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub231pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub231pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub231pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsub231pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsub231pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmsub231pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmsub231pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmsub231pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmsub231pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmsub231pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub231pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfmsub231pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub231pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfmsub231ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub231ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub231ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsub231ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsub231ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsub231ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmsub231ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmsub231ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmsub231ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmsub231ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmsub231ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub231ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfmsub231ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub231ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfmsub231ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub231ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub231ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsub231ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsub231ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsub231ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfmsub231ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmsub231ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmsub231ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmsub231ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmsub231ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmsub231ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfmsub231ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmsub231ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfmsubadd132pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd132pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd132pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd132pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsubadd132pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsubadd132pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfmsubadd132pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmsubadd132pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmsubadd132pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfmsubadd132pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfmsubadd132pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd132pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd132pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd132pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsubadd132pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsubadd132pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmsubadd132pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmsubadd132pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmsubadd132pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfmsubadd132pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfmsubadd132ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd132ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd132ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd132ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsubadd132ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsubadd132ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmsubadd132ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmsubadd132ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmsubadd132ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfmsubadd132ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfmsubadd132ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd132ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd132ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd132ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsubadd132ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsubadd132ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfmsubadd132ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmsubadd132ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmsubadd132ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfmsubadd132ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfmsubadd213pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd213pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd213pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd213pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsubadd213pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsubadd213pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfmsubadd213pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmsubadd213pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmsubadd213pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfmsubadd213pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfmsubadd213pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd213pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd213pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd213pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsubadd213pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsubadd213pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmsubadd213pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmsubadd213pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmsubadd213pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfmsubadd213pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfmsubadd213ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd213ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd213ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd213ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsubadd213ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsubadd213ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmsubadd213ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmsubadd213ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmsubadd213ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfmsubadd213ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfmsubadd213ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd213ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd213ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd213ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsubadd213ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsubadd213ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfmsubadd213ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmsubadd213ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmsubadd213ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfmsubadd213ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfmsubadd231pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd231pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd231pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd231pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsubadd231pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsubadd231pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfmsubadd231pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmsubadd231pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmsubadd231pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfmsubadd231pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfmsubadd231pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd231pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd231pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd231pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsubadd231pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsubadd231pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmsubadd231pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmsubadd231pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmsubadd231pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfmsubadd231pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfmsubadd231ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd231ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd231ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfmsubadd231ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsubadd231ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsubadd231ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfmsubadd231ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfmsubadd231ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfmsubadd231ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfmsubadd231ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfmsubadd231ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd231ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd231ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfmsubadd231ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfmsubadd231ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfmsubadd231ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfmsubadd231ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfmsubadd231ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfmsubadd231ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfmsubadd231ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfnmadd132pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd132pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd132pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd132pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmadd132pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmadd132pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfnmadd132pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfnmadd132pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfnmadd132pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfnmadd132pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfnmadd132pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfnmadd132pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfnmadd132pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd132pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd132pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd132pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmadd132pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmadd132pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfnmadd132pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfnmadd132pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfnmadd132pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfnmadd132pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfnmadd132pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfnmadd132pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfnmadd132ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd132ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd132ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd132ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmadd132ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmadd132ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfnmadd132ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfnmadd132ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfnmadd132ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfnmadd132ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfnmadd132ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfnmadd132ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfnmadd132ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd132ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd132ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd132ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmadd132ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmadd132ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfnmadd132ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfnmadd132ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfnmadd132ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfnmadd132ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfnmadd132ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfnmadd132ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfnmadd213pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd213pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd213pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd213pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmadd213pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmadd213pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfnmadd213pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfnmadd213pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfnmadd213pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfnmadd213pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfnmadd213pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfnmadd213pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfnmadd213pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd213pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd213pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd213pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmadd213pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmadd213pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfnmadd213pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfnmadd213pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfnmadd213pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfnmadd213pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfnmadd213pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfnmadd213pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfnmadd213ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd213ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd213ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd213ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmadd213ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmadd213ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfnmadd213ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfnmadd213ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfnmadd213ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfnmadd213ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfnmadd213ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfnmadd213ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfnmadd213ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd213ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd213ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd213ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmadd213ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmadd213ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfnmadd213ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfnmadd213ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfnmadd213ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfnmadd213ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfnmadd213ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfnmadd213ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfnmadd231pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd231pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd231pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd231pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmadd231pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmadd231pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfnmadd231pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfnmadd231pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfnmadd231pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfnmadd231pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfnmadd231pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfnmadd231pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfnmadd231pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd231pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd231pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd231pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmadd231pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmadd231pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfnmadd231pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfnmadd231pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfnmadd231pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfnmadd231pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfnmadd231pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfnmadd231pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfnmadd231ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd231ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd231ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmadd231ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmadd231ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmadd231ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfnmadd231ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfnmadd231ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfnmadd231ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfnmadd231ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfnmadd231ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfnmadd231ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfnmadd231ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd231ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd231ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmadd231ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmadd231ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmadd231ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfnmadd231ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfnmadd231ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfnmadd231ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfnmadd231ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfnmadd231ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfnmadd231ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfnmsub132pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub132pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub132pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub132pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmsub132pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmsub132pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfnmsub132pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfnmsub132pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfnmsub132pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfnmsub132pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfnmsub132pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfnmsub132pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfnmsub132pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub132pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub132pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub132pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmsub132pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmsub132pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfnmsub132pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfnmsub132pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfnmsub132pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfnmsub132pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfnmsub132pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfnmsub132pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfnmsub132ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub132ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub132ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub132ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmsub132ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmsub132ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfnmsub132ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfnmsub132ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfnmsub132ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfnmsub132ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfnmsub132ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfnmsub132ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfnmsub132ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub132ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub132ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub132ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmsub132ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmsub132ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfnmsub132ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfnmsub132ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfnmsub132ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfnmsub132ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfnmsub132ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfnmsub132ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfnmsub213pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub213pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub213pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub213pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmsub213pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmsub213pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfnmsub213pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfnmsub213pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfnmsub213pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfnmsub213pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfnmsub213pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfnmsub213pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfnmsub213pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub213pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub213pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub213pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmsub213pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmsub213pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfnmsub213pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfnmsub213pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfnmsub213pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfnmsub213pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfnmsub213pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfnmsub213pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfnmsub213ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub213ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub213ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub213ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmsub213ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmsub213ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfnmsub213ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfnmsub213ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfnmsub213ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfnmsub213ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfnmsub213ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfnmsub213ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfnmsub213ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub213ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub213ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub213ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmsub213ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmsub213ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfnmsub213ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfnmsub213ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfnmsub213ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfnmsub213ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfnmsub213ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfnmsub213ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfnmsub231pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub231pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub231pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub231pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmsub231pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmsub231pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vfnmsub231pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfnmsub231pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfnmsub231pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfnmsub231pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfnmsub231pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vfnmsub231pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vfnmsub231pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub231pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub231pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub231pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmsub231pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmsub231pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfnmsub231pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfnmsub231pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfnmsub231pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfnmsub231pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfnmsub231pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vfnmsub231pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vfnmsub231ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub231ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub231ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vfnmsub231ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmsub231ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmsub231ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vfnmsub231ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vfnmsub231ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vfnmsub231ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vfnmsub231ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vfnmsub231ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vfnmsub231ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vfnmsub231ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub231ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub231ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vfnmsub231ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vfnmsub231ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vfnmsub231ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vfnmsub231ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vfnmsub231ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vfnmsub231ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vfnmsub231ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vfnmsub231ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vfnmsub231ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vgatherdpd	xmm30{k1}, [r14+xmm31*8-123]	 # AVX512{F,VL}
	vgatherdpd	xmm30{k1}, [r9+xmm31+256]	 # AVX512{F,VL}
	vgatherdpd	xmm30{k1}, [rcx+xmm31*4+1024]	 # AVX512{F,VL}
	vgatherdpd	ymm30{k1}, [r14+xmm31*8-123]	 # AVX512{F,VL}
	vgatherdpd	ymm30{k1}, [r9+xmm31+256]	 # AVX512{F,VL}
	vgatherdpd	ymm30{k1}, [rcx+xmm31*4+1024]	 # AVX512{F,VL}
	vgatherdps	xmm30{k1}, [r14+xmm31*8-123]	 # AVX512{F,VL}
	vgatherdps	xmm30{k1}, [r9+xmm31+256]	 # AVX512{F,VL}
	vgatherdps	xmm30{k1}, [rcx+xmm31*4+1024]	 # AVX512{F,VL}
	vgatherdps	ymm30{k1}, [r14+ymm31*8-123]	 # AVX512{F,VL}
	vgatherdps	ymm30{k1}, [r9+ymm31+256]	 # AVX512{F,VL}
	vgatherdps	ymm30{k1}, [rcx+ymm31*4+1024]	 # AVX512{F,VL}
	vgatherqpd	xmm30{k1}, [r14+xmm31*8-123]	 # AVX512{F,VL}
	vgatherqpd	xmm30{k1}, [r9+xmm31+256]	 # AVX512{F,VL}
	vgatherqpd	xmm30{k1}, [rcx+xmm31*4+1024]	 # AVX512{F,VL}
	vgatherqpd	ymm30{k1}, [r14+ymm31*8-123]	 # AVX512{F,VL}
	vgatherqpd	ymm30{k1}, [r9+ymm31+256]	 # AVX512{F,VL}
	vgatherqpd	ymm30{k1}, [rcx+ymm31*4+1024]	 # AVX512{F,VL}
	vgatherqps	xmm30{k1}, [r14+xmm31*8-123]	 # AVX512{F,VL}
	vgatherqps	xmm30{k1}, [r9+xmm31+256]	 # AVX512{F,VL}
	vgatherqps	xmm30{k1}, [rcx+xmm31*4+1024]	 # AVX512{F,VL}
	vgatherqps	xmm30{k1}, [r14+ymm31*8-123]	 # AVX512{F,VL}
	vgatherqps	xmm30{k1}, [r9+ymm31+256]	 # AVX512{F,VL}
	vgatherqps	xmm30{k1}, [rcx+ymm31*4+1024]	 # AVX512{F,VL}
	vgetexppd	xmm30, xmm29	 # AVX512{F,VL}
	vgetexppd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vgetexppd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vgetexppd	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vgetexppd	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vgetexppd	xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vgetexppd	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vgetexppd	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vgetexppd	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vgetexppd	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vgetexppd	xmm30, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vgetexppd	xmm30, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vgetexppd	xmm30, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vgetexppd	xmm30, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vgetexppd	ymm30, ymm29	 # AVX512{F,VL}
	vgetexppd	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vgetexppd	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vgetexppd	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vgetexppd	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vgetexppd	ymm30, [rcx]{1to4}	 # AVX512{F,VL}
	vgetexppd	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vgetexppd	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vgetexppd	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vgetexppd	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vgetexppd	ymm30, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vgetexppd	ymm30, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vgetexppd	ymm30, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vgetexppd	ymm30, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vgetexpps	xmm30, xmm29	 # AVX512{F,VL}
	vgetexpps	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vgetexpps	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vgetexpps	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vgetexpps	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vgetexpps	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vgetexpps	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vgetexpps	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vgetexpps	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vgetexpps	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vgetexpps	xmm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vgetexpps	xmm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vgetexpps	xmm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vgetexpps	xmm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vgetexpps	ymm30, ymm29	 # AVX512{F,VL}
	vgetexpps	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vgetexpps	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vgetexpps	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vgetexpps	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vgetexpps	ymm30, [rcx]{1to8}	 # AVX512{F,VL}
	vgetexpps	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vgetexpps	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vgetexpps	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vgetexpps	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vgetexpps	ymm30, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vgetexpps	ymm30, [rdx+512]{1to8}	 # AVX512{F,VL}
	vgetexpps	ymm30, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vgetexpps	ymm30, [rdx-516]{1to8}	 # AVX512{F,VL}
	vgetmantpd	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vgetmantpd	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vgetmantpd	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vgetmantpd	xmm30, xmm29, 123	 # AVX512{F,VL}
	vgetmantpd	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vgetmantpd	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vgetmantpd	xmm30, [rcx]{1to2}, 123	 # AVX512{F,VL}
	vgetmantpd	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vgetmantpd	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vgetmantpd	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vgetmantpd	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vgetmantpd	xmm30, [rdx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vgetmantpd	xmm30, [rdx+1024]{1to2}, 123	 # AVX512{F,VL}
	vgetmantpd	xmm30, [rdx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vgetmantpd	xmm30, [rdx-1032]{1to2}, 123	 # AVX512{F,VL}
	vgetmantpd	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vgetmantpd	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vgetmantpd	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vgetmantpd	ymm30, ymm29, 123	 # AVX512{F,VL}
	vgetmantpd	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vgetmantpd	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vgetmantpd	ymm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vgetmantpd	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vgetmantpd	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vgetmantpd	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vgetmantpd	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vgetmantpd	ymm30, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vgetmantpd	ymm30, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vgetmantpd	ymm30, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vgetmantpd	ymm30, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vgetmantps	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vgetmantps	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vgetmantps	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vgetmantps	xmm30, xmm29, 123	 # AVX512{F,VL}
	vgetmantps	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vgetmantps	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vgetmantps	xmm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vgetmantps	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vgetmantps	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vgetmantps	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vgetmantps	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vgetmantps	xmm30, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vgetmantps	xmm30, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	vgetmantps	xmm30, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vgetmantps	xmm30, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	vgetmantps	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vgetmantps	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vgetmantps	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vgetmantps	ymm30, ymm29, 123	 # AVX512{F,VL}
	vgetmantps	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vgetmantps	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vgetmantps	ymm30, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vgetmantps	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vgetmantps	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vgetmantps	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vgetmantps	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vgetmantps	ymm30, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vgetmantps	ymm30, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vgetmantps	ymm30, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vgetmantps	ymm30, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vinsertf32x4	ymm30, ymm29, xmm28, 0xab	 # AVX512{F,VL}
	vinsertf32x4	ymm30{k7}, ymm29, xmm28, 0xab	 # AVX512{F,VL}
	vinsertf32x4	ymm30{k7}{z}, ymm29, xmm28, 0xab	 # AVX512{F,VL}
	vinsertf32x4	ymm30, ymm29, xmm28, 123	 # AVX512{F,VL}
	vinsertf32x4	ymm30, ymm29, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vinsertf32x4	ymm30, ymm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vinsertf32x4	ymm30, ymm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vinsertf32x4	ymm30, ymm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vinsertf32x4	ymm30, ymm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vinsertf32x4	ymm30, ymm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vinserti32x4	ymm30, ymm29, xmm28, 0xab	 # AVX512{F,VL}
	vinserti32x4	ymm30{k7}, ymm29, xmm28, 0xab	 # AVX512{F,VL}
	vinserti32x4	ymm30{k7}{z}, ymm29, xmm28, 0xab	 # AVX512{F,VL}
	vinserti32x4	ymm30, ymm29, xmm28, 123	 # AVX512{F,VL}
	vinserti32x4	ymm30, ymm29, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vinserti32x4	ymm30, ymm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vinserti32x4	ymm30, ymm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vinserti32x4	ymm30, ymm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vinserti32x4	ymm30, ymm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vinserti32x4	ymm30, ymm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vmaxpd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vmaxpd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vmaxpd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vmaxpd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmaxpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmaxpd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vmaxpd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vmaxpd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vmaxpd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vmaxpd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vmaxpd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vmaxpd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vmaxpd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vmaxpd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vmaxpd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vmaxpd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vmaxpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vmaxpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmaxpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmaxpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vmaxpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmaxpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmaxpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmaxpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmaxpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vmaxpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vmaxpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vmaxpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vmaxps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vmaxps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vmaxps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vmaxps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmaxps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmaxps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vmaxps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vmaxps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vmaxps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vmaxps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vmaxps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vmaxps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vmaxps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vmaxps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vmaxps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vmaxps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vmaxps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vmaxps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmaxps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmaxps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vmaxps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmaxps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmaxps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmaxps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmaxps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vmaxps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vmaxps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vmaxps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vminpd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vminpd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vminpd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vminpd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vminpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vminpd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vminpd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vminpd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vminpd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vminpd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vminpd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vminpd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vminpd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vminpd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vminpd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vminpd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vminpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vminpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vminpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vminpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vminpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vminpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vminpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vminpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vminpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vminpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vminpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vminpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vminps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vminps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vminps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vminps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vminps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vminps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vminps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vminps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vminps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vminps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vminps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vminps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vminps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vminps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vminps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vminps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vminps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vminps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vminps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vminps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vminps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vminps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vminps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vminps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vminps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vminps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vminps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vminps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vmovapd	xmm30, xmm29	 # AVX512{F,VL}
	vmovapd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vmovapd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vmovapd	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovapd	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovapd	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vmovapd	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vmovapd	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vmovapd	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vmovapd	ymm30, ymm29	 # AVX512{F,VL}
	vmovapd	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vmovapd	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vmovapd	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovapd	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovapd	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmovapd	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmovapd	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmovapd	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmovaps	xmm30, xmm29	 # AVX512{F,VL}
	vmovaps	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vmovaps	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vmovaps	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovaps	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovaps	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vmovaps	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vmovaps	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vmovaps	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vmovaps	ymm30, ymm29	 # AVX512{F,VL}
	vmovaps	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vmovaps	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vmovaps	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovaps	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovaps	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmovaps	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmovaps	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmovaps	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmovddup	xmm30, xmm29	 # AVX512{F,VL}
	vmovddup	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vmovddup	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vmovddup	xmm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vmovddup	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovddup	xmm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vmovddup	xmm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vmovddup	xmm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vmovddup	xmm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vmovddup	ymm30, ymm29	 # AVX512{F,VL}
	vmovddup	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vmovddup	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vmovddup	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovddup	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovddup	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmovddup	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmovddup	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmovddup	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmovdqa32	xmm30, xmm29	 # AVX512{F,VL}
	vmovdqa32	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vmovdqa32	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vmovdqa32	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovdqa32	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovdqa32	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vmovdqa32	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vmovdqa32	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vmovdqa32	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vmovdqa32	ymm30, ymm29	 # AVX512{F,VL}
	vmovdqa32	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vmovdqa32	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vmovdqa32	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovdqa32	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovdqa32	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmovdqa32	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmovdqa32	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmovdqa32	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmovdqa64	xmm30, xmm29	 # AVX512{F,VL}
	vmovdqa64	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vmovdqa64	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vmovdqa64	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovdqa64	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovdqa64	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vmovdqa64	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vmovdqa64	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vmovdqa64	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vmovdqa64	ymm30, ymm29	 # AVX512{F,VL}
	vmovdqa64	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vmovdqa64	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vmovdqa64	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovdqa64	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovdqa64	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmovdqa64	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmovdqa64	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmovdqa64	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmovdqu32	xmm30, xmm29	 # AVX512{F,VL}
	vmovdqu32	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vmovdqu32	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vmovdqu32	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovdqu32	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovdqu32	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vmovdqu32	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vmovdqu32	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vmovdqu32	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vmovdqu32	ymm30, ymm29	 # AVX512{F,VL}
	vmovdqu32	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vmovdqu32	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vmovdqu32	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovdqu32	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovdqu32	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmovdqu32	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmovdqu32	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmovdqu32	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmovdqu64	xmm30, xmm29	 # AVX512{F,VL}
	vmovdqu64	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vmovdqu64	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vmovdqu64	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovdqu64	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovdqu64	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vmovdqu64	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vmovdqu64	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vmovdqu64	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vmovdqu64	ymm30, ymm29	 # AVX512{F,VL}
	vmovdqu64	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vmovdqu64	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vmovdqu64	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovdqu64	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovdqu64	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmovdqu64	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmovdqu64	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmovdqu64	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmovntdq	XMMWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vmovntdq	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vmovntdq	XMMWORD PTR [rdx+2032], xmm30	 # AVX512{F,VL} Disp8
	vmovntdq	XMMWORD PTR [rdx+2048], xmm30	 # AVX512{F,VL}
	vmovntdq	XMMWORD PTR [rdx-2048], xmm30	 # AVX512{F,VL} Disp8
	vmovntdq	XMMWORD PTR [rdx-2064], xmm30	 # AVX512{F,VL}
	vmovntdq	YMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vmovntdq	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vmovntdq	YMMWORD PTR [rdx+4064], ymm30	 # AVX512{F,VL} Disp8
	vmovntdq	YMMWORD PTR [rdx+4096], ymm30	 # AVX512{F,VL}
	vmovntdq	YMMWORD PTR [rdx-4096], ymm30	 # AVX512{F,VL} Disp8
	vmovntdq	YMMWORD PTR [rdx-4128], ymm30	 # AVX512{F,VL}
	vmovntdqa	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovntdqa	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovntdqa	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vmovntdqa	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vmovntdqa	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vmovntdqa	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vmovntdqa	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovntdqa	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovntdqa	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmovntdqa	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmovntdqa	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmovntdqa	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmovntpd	XMMWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vmovntpd	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vmovntpd	XMMWORD PTR [rdx+2032], xmm30	 # AVX512{F,VL} Disp8
	vmovntpd	XMMWORD PTR [rdx+2048], xmm30	 # AVX512{F,VL}
	vmovntpd	XMMWORD PTR [rdx-2048], xmm30	 # AVX512{F,VL} Disp8
	vmovntpd	XMMWORD PTR [rdx-2064], xmm30	 # AVX512{F,VL}
	vmovntpd	YMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vmovntpd	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vmovntpd	YMMWORD PTR [rdx+4064], ymm30	 # AVX512{F,VL} Disp8
	vmovntpd	YMMWORD PTR [rdx+4096], ymm30	 # AVX512{F,VL}
	vmovntpd	YMMWORD PTR [rdx-4096], ymm30	 # AVX512{F,VL} Disp8
	vmovntpd	YMMWORD PTR [rdx-4128], ymm30	 # AVX512{F,VL}
	vmovntps	XMMWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vmovntps	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vmovntps	XMMWORD PTR [rdx+2032], xmm30	 # AVX512{F,VL} Disp8
	vmovntps	XMMWORD PTR [rdx+2048], xmm30	 # AVX512{F,VL}
	vmovntps	XMMWORD PTR [rdx-2048], xmm30	 # AVX512{F,VL} Disp8
	vmovntps	XMMWORD PTR [rdx-2064], xmm30	 # AVX512{F,VL}
	vmovntps	YMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vmovntps	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vmovntps	YMMWORD PTR [rdx+4064], ymm30	 # AVX512{F,VL} Disp8
	vmovntps	YMMWORD PTR [rdx+4096], ymm30	 # AVX512{F,VL}
	vmovntps	YMMWORD PTR [rdx-4096], ymm30	 # AVX512{F,VL} Disp8
	vmovntps	YMMWORD PTR [rdx-4128], ymm30	 # AVX512{F,VL}
	vmovshdup	xmm30, xmm29	 # AVX512{F,VL}
	vmovshdup	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vmovshdup	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vmovshdup	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovshdup	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovshdup	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vmovshdup	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vmovshdup	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vmovshdup	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vmovshdup	ymm30, ymm29	 # AVX512{F,VL}
	vmovshdup	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vmovshdup	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vmovshdup	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovshdup	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovshdup	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmovshdup	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmovshdup	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmovshdup	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmovsldup	xmm30, xmm29	 # AVX512{F,VL}
	vmovsldup	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vmovsldup	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vmovsldup	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovsldup	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovsldup	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vmovsldup	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vmovsldup	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vmovsldup	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vmovsldup	ymm30, ymm29	 # AVX512{F,VL}
	vmovsldup	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vmovsldup	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vmovsldup	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovsldup	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovsldup	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmovsldup	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmovsldup	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmovsldup	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmovupd	xmm30, xmm29	 # AVX512{F,VL}
	vmovupd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vmovupd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vmovupd	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovupd	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovupd	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vmovupd	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vmovupd	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vmovupd	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vmovupd	ymm30, ymm29	 # AVX512{F,VL}
	vmovupd	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vmovupd	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vmovupd	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovupd	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovupd	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmovupd	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmovupd	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmovupd	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmovups	xmm30, xmm29	 # AVX512{F,VL}
	vmovups	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vmovups	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vmovups	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovups	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovups	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vmovups	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vmovups	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vmovups	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vmovups	ymm30, ymm29	 # AVX512{F,VL}
	vmovups	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vmovups	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vmovups	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmovups	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmovups	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmovups	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmovups	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmovups	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmulpd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vmulpd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vmulpd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vmulpd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmulpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmulpd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vmulpd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vmulpd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vmulpd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vmulpd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vmulpd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vmulpd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vmulpd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vmulpd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vmulpd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vmulpd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vmulpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vmulpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmulpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmulpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vmulpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmulpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmulpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmulpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmulpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vmulpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vmulpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vmulpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vmulps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vmulps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vmulps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vmulps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmulps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmulps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vmulps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vmulps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vmulps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vmulps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vmulps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vmulps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vmulps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vmulps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vmulps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vmulps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vmulps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vmulps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vmulps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vmulps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vmulps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vmulps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vmulps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vmulps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vmulps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vmulps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vmulps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vmulps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpabsd	xmm30, xmm29	 # AVX512{F,VL}
	vpabsd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpabsd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpabsd	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpabsd	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpabsd	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vpabsd	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpabsd	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpabsd	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpabsd	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpabsd	xmm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpabsd	xmm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpabsd	xmm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpabsd	xmm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpabsd	ymm30, ymm29	 # AVX512{F,VL}
	vpabsd	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vpabsd	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpabsd	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpabsd	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpabsd	ymm30, [rcx]{1to8}	 # AVX512{F,VL}
	vpabsd	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpabsd	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpabsd	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpabsd	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpabsd	ymm30, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpabsd	ymm30, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpabsd	ymm30, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpabsd	ymm30, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpabsq	xmm30, xmm29	 # AVX512{F,VL}
	vpabsq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpabsq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpabsq	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpabsq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpabsq	xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vpabsq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpabsq	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpabsq	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpabsq	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpabsq	xmm30, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpabsq	xmm30, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpabsq	xmm30, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpabsq	xmm30, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpabsq	ymm30, ymm29	 # AVX512{F,VL}
	vpabsq	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vpabsq	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpabsq	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpabsq	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpabsq	ymm30, [rcx]{1to4}	 # AVX512{F,VL}
	vpabsq	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpabsq	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpabsq	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpabsq	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpabsq	ymm30, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpabsq	ymm30, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpabsq	ymm30, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpabsq	ymm30, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpaddd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpaddd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpaddd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpaddd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpaddd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpaddd	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpaddd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpaddd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpaddd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpaddd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpaddd	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpaddd	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpaddd	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpaddd	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpaddd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpaddd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpaddd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpaddd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpaddd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpaddd	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpaddd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpaddd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpaddd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpaddd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpaddd	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpaddd	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpaddd	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpaddd	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpaddq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpaddq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpaddq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpaddq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpaddq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpaddq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpaddq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpaddq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpaddq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpaddq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpaddq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpaddq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpaddq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpaddq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpaddq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpaddq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpaddq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpaddq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpaddq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpaddq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpaddq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpaddq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpaddq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpaddq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpaddq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpaddq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpaddq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpaddq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpandd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpandd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpandd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpandd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpandd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpandd	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpandd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpandd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpandd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpandd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpandd	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpandd	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpandd	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpandd	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpandd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpandd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpandd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpandd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpandd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpandd	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpandd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpandd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpandd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpandd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpandd	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpandd	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpandd	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpandd	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpandnd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpandnd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpandnd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpandnd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpandnd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpandnd	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpandnd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpandnd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpandnd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpandnd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpandnd	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpandnd	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpandnd	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpandnd	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpandnd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpandnd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpandnd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpandnd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpandnd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpandnd	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpandnd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpandnd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpandnd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpandnd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpandnd	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpandnd	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpandnd	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpandnd	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpandnq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpandnq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpandnq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpandnq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpandnq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpandnq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpandnq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpandnq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpandnq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpandnq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpandnq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpandnq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpandnq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpandnq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpandnq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpandnq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpandnq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpandnq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpandnq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpandnq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpandnq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpandnq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpandnq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpandnq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpandnq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpandnq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpandnq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpandnq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpandq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpandq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpandq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpandq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpandq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpandq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpandq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpandq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpandq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpandq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpandq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpandq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpandq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpandq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpandq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpandq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpandq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpandq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpandq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpandq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpandq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpandq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpandq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpandq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpandq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpandq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpandq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpandq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpblendmd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpblendmd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpblendmd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpblendmd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpblendmd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpblendmd	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpblendmd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpblendmd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpblendmd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpblendmd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpblendmd	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpblendmd	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpblendmd	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpblendmd	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpblendmd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpblendmd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpblendmd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpblendmd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpblendmd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpblendmd	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpblendmd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpblendmd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpblendmd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpblendmd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpblendmd	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpblendmd	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpblendmd	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpblendmd	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpbroadcastd	xmm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpbroadcastd	xmm30{k7}, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpbroadcastd	xmm30{k7}{z}, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpbroadcastd	xmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpbroadcastd	xmm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpbroadcastd	xmm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpbroadcastd	xmm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpbroadcastd	xmm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpbroadcastd	ymm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpbroadcastd	ymm30{k7}, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpbroadcastd	ymm30{k7}{z}, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpbroadcastd	ymm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpbroadcastd	ymm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpbroadcastd	ymm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpbroadcastd	ymm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpbroadcastd	ymm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpbroadcastd	xmm30, xmm29	 # AVX512{F,VL}
	vpbroadcastd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpbroadcastd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpbroadcastd	ymm30, xmm29	 # AVX512{F,VL}
	vpbroadcastd	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpbroadcastd	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpbroadcastd	xmm30, eax	 # AVX512{F,VL}
	vpbroadcastd	xmm30{k7}, eax	 # AVX512{F,VL}
	vpbroadcastd	xmm30{k7}{z}, eax	 # AVX512{F,VL}
	vpbroadcastd	xmm30, ebp	 # AVX512{F,VL}
	vpbroadcastd	xmm30, r13d	 # AVX512{F,VL}
	vpbroadcastd	ymm30, eax	 # AVX512{F,VL}
	vpbroadcastd	ymm30{k7}, eax	 # AVX512{F,VL}
	vpbroadcastd	ymm30{k7}{z}, eax	 # AVX512{F,VL}
	vpbroadcastd	ymm30, ebp	 # AVX512{F,VL}
	vpbroadcastd	ymm30, r13d	 # AVX512{F,VL}
	vpbroadcastq	xmm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpbroadcastq	xmm30{k7}, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpbroadcastq	xmm30{k7}{z}, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpbroadcastq	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpbroadcastq	xmm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpbroadcastq	xmm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpbroadcastq	xmm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpbroadcastq	xmm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpbroadcastq	ymm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpbroadcastq	ymm30{k7}, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpbroadcastq	ymm30{k7}{z}, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpbroadcastq	ymm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpbroadcastq	ymm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpbroadcastq	ymm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpbroadcastq	ymm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpbroadcastq	ymm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpbroadcastq	xmm30, xmm29	 # AVX512{F,VL}
	vpbroadcastq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpbroadcastq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpbroadcastq	ymm30, xmm29	 # AVX512{F,VL}
	vpbroadcastq	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpbroadcastq	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpbroadcastq	xmm30, rax	 # AVX512{F,VL}
	vpbroadcastq	xmm30{k7}, rax	 # AVX512{F,VL}
	vpbroadcastq	xmm30{k7}{z}, rax	 # AVX512{F,VL}
	vpbroadcastq	xmm30, r8	 # AVX512{F,VL}
	vpbroadcastq	ymm30, rax	 # AVX512{F,VL}
	vpbroadcastq	ymm30{k7}, rax	 # AVX512{F,VL}
	vpbroadcastq	ymm30{k7}{z}, rax	 # AVX512{F,VL}
	vpbroadcastq	ymm30, r8	 # AVX512{F,VL}
	vpcmpd	k5, xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpcmpd	k5{k7}, xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpcmpd	k5, xmm30, xmm29, 123	 # AVX512{F,VL}
	vpcmpd	k5, xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpcmpd	k5, xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpcmpd	k5, xmm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpcmpd	k5, xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5, xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vpcmpd	k5, xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5, xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vpcmpd	k5, xmm30, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5, xmm30, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	vpcmpd	k5, xmm30, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5, xmm30, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	vpcmpd	k5, ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpcmpd	k5{k7}, ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpcmpd	k5, ymm30, ymm29, 123	 # AVX512{F,VL}
	vpcmpd	k5, ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpcmpd	k5, ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpcmpd	k5, ymm30, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vpcmpd	k5, ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5, ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpcmpd	k5, ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5, ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpcmpd	k5, ymm30, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5, ymm30, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vpcmpd	k5, ymm30, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5, ymm30, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vpcmpeqd	k5, xmm30, xmm29	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, xmm30, xmm29	 # AVX512{F,VL}
	vpcmpeqd	k5, xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpcmpeqd	k5, xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpcmpeqd	k5, xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vpcmpeqd	k5, xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5, xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpcmpeqd	k5, xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5, xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpcmpeqd	k5, xmm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5, xmm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpcmpeqd	k5, xmm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5, xmm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpcmpeqd	k5, ymm30, ymm29	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, ymm30, ymm29	 # AVX512{F,VL}
	vpcmpeqd	k5, ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpcmpeqd	k5, ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpcmpeqd	k5, ymm30, [rcx]{1to8}	 # AVX512{F,VL}
	vpcmpeqd	k5, ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5, ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpcmpeqd	k5, ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5, ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpcmpeqd	k5, ymm30, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5, ymm30, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpcmpeqd	k5, ymm30, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5, ymm30, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpcmpeqq	k5, xmm30, xmm29	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, xmm30, xmm29	 # AVX512{F,VL}
	vpcmpeqq	k5, xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpcmpeqq	k5, xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpcmpeqq	k5, xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vpcmpeqq	k5, xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5, xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpcmpeqq	k5, xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5, xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpcmpeqq	k5, xmm30, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5, xmm30, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpcmpeqq	k5, xmm30, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5, xmm30, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpcmpeqq	k5, ymm30, ymm29	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, ymm30, ymm29	 # AVX512{F,VL}
	vpcmpeqq	k5, ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpcmpeqq	k5, ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpcmpeqq	k5, ymm30, [rcx]{1to4}	 # AVX512{F,VL}
	vpcmpeqq	k5, ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5, ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpcmpeqq	k5, ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5, ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpcmpeqq	k5, ymm30, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5, ymm30, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpcmpeqq	k5, ymm30, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5, ymm30, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpcmpgtd	k5, xmm30, xmm29	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, xmm30, xmm29	 # AVX512{F,VL}
	vpcmpgtd	k5, xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpcmpgtd	k5, xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpcmpgtd	k5, xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vpcmpgtd	k5, xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5, xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpcmpgtd	k5, xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5, xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpcmpgtd	k5, xmm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5, xmm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpcmpgtd	k5, xmm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5, xmm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpcmpgtd	k5, ymm30, ymm29	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, ymm30, ymm29	 # AVX512{F,VL}
	vpcmpgtd	k5, ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpcmpgtd	k5, ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpcmpgtd	k5, ymm30, [rcx]{1to8}	 # AVX512{F,VL}
	vpcmpgtd	k5, ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5, ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpcmpgtd	k5, ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5, ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpcmpgtd	k5, ymm30, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5, ymm30, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpcmpgtd	k5, ymm30, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5, ymm30, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpcmpgtq	k5, xmm30, xmm29	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, xmm30, xmm29	 # AVX512{F,VL}
	vpcmpgtq	k5, xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpcmpgtq	k5, xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpcmpgtq	k5, xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vpcmpgtq	k5, xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5, xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpcmpgtq	k5, xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5, xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpcmpgtq	k5, xmm30, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5, xmm30, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpcmpgtq	k5, xmm30, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5, xmm30, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpcmpgtq	k5, ymm30, ymm29	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, ymm30, ymm29	 # AVX512{F,VL}
	vpcmpgtq	k5, ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpcmpgtq	k5, ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpcmpgtq	k5, ymm30, [rcx]{1to4}	 # AVX512{F,VL}
	vpcmpgtq	k5, ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5, ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpcmpgtq	k5, ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5, ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpcmpgtq	k5, ymm30, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5, ymm30, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpcmpgtq	k5, ymm30, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5, ymm30, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpcmpq	k5, xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpcmpq	k5{k7}, xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpcmpq	k5, xmm30, xmm29, 123	 # AVX512{F,VL}
	vpcmpq	k5, xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpcmpq	k5, xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpcmpq	k5, xmm30, [rcx]{1to2}, 123	 # AVX512{F,VL}
	vpcmpq	k5, xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5, xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vpcmpq	k5, xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5, xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vpcmpq	k5, xmm30, [rdx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5, xmm30, [rdx+1024]{1to2}, 123	 # AVX512{F,VL}
	vpcmpq	k5, xmm30, [rdx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5, xmm30, [rdx-1032]{1to2}, 123	 # AVX512{F,VL}
	vpcmpq	k5, ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpcmpq	k5{k7}, ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpcmpq	k5, ymm30, ymm29, 123	 # AVX512{F,VL}
	vpcmpq	k5, ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpcmpq	k5, ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpcmpq	k5, ymm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpcmpq	k5, ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5, ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpcmpq	k5, ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5, ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpcmpq	k5, ymm30, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5, ymm30, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpcmpq	k5, ymm30, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5, ymm30, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpcmpud	k5, xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpcmpud	k5{k7}, xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpcmpud	k5, xmm30, xmm29, 123	 # AVX512{F,VL}
	vpcmpud	k5, xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpcmpud	k5, xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpcmpud	k5, xmm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpcmpud	k5, xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5, xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vpcmpud	k5, xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5, xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vpcmpud	k5, xmm30, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5, xmm30, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	vpcmpud	k5, xmm30, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5, xmm30, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	vpcmpud	k5, ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpcmpud	k5{k7}, ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpcmpud	k5, ymm30, ymm29, 123	 # AVX512{F,VL}
	vpcmpud	k5, ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpcmpud	k5, ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpcmpud	k5, ymm30, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vpcmpud	k5, ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5, ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpcmpud	k5, ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5, ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpcmpud	k5, ymm30, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5, ymm30, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vpcmpud	k5, ymm30, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5, ymm30, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vpcmpuq	k5, xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpcmpuq	k5, xmm30, xmm29, 123	 # AVX512{F,VL}
	vpcmpuq	k5, xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpcmpuq	k5, xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpcmpuq	k5, xmm30, [rcx]{1to2}, 123	 # AVX512{F,VL}
	vpcmpuq	k5, xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5, xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vpcmpuq	k5, xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5, xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vpcmpuq	k5, xmm30, [rdx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5, xmm30, [rdx+1024]{1to2}, 123	 # AVX512{F,VL}
	vpcmpuq	k5, xmm30, [rdx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5, xmm30, [rdx-1032]{1to2}, 123	 # AVX512{F,VL}
	vpcmpuq	k5, ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpcmpuq	k5, ymm30, ymm29, 123	 # AVX512{F,VL}
	vpcmpuq	k5, ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpcmpuq	k5, ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpcmpuq	k5, ymm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpcmpuq	k5, ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5, ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpcmpuq	k5, ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5, ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpcmpuq	k5, ymm30, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5, ymm30, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpcmpuq	k5, ymm30, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5, ymm30, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpblendmq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpblendmq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpblendmq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpblendmq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpblendmq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpblendmq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpblendmq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpblendmq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpblendmq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpblendmq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpblendmq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpblendmq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpblendmq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpblendmq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpblendmq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpblendmq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpblendmq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpblendmq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpblendmq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpblendmq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpblendmq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpblendmq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpblendmq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpblendmq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpblendmq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpblendmq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpblendmq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpblendmq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpcompressd	XMMWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpcompressd	XMMWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpcompressd	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpcompressd	XMMWORD PTR [rdx+508], xmm30	 # AVX512{F,VL} Disp8
	vpcompressd	XMMWORD PTR [rdx+512], xmm30	 # AVX512{F,VL}
	vpcompressd	XMMWORD PTR [rdx-512], xmm30	 # AVX512{F,VL} Disp8
	vpcompressd	XMMWORD PTR [rdx-516], xmm30	 # AVX512{F,VL}
	vpcompressd	YMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpcompressd	YMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpcompressd	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpcompressd	YMMWORD PTR [rdx+508], ymm30	 # AVX512{F,VL} Disp8
	vpcompressd	YMMWORD PTR [rdx+512], ymm30	 # AVX512{F,VL}
	vpcompressd	YMMWORD PTR [rdx-512], ymm30	 # AVX512{F,VL} Disp8
	vpcompressd	YMMWORD PTR [rdx-516], ymm30	 # AVX512{F,VL}
	vpcompressd	xmm30, xmm29	 # AVX512{F,VL}
	vpcompressd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpcompressd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpcompressd	ymm30, ymm29	 # AVX512{F,VL}
	vpcompressd	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vpcompressd	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpermd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpermd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpermd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpermd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermd	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpermd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpermd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpermd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpermd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpermd	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpermd	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpermd	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpermd	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpermilpd	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpermilpd	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vpermilpd	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vpermilpd	xmm30, xmm29, 123	 # AVX512{F,VL}
	vpermilpd	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpermilpd	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpermilpd	xmm30, [rcx]{1to2}, 123	 # AVX512{F,VL}
	vpermilpd	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vpermilpd	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vpermilpd	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vpermilpd	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vpermilpd	xmm30, [rdx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpermilpd	xmm30, [rdx+1024]{1to2}, 123	 # AVX512{F,VL}
	vpermilpd	xmm30, [rdx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpermilpd	xmm30, [rdx-1032]{1to2}, 123	 # AVX512{F,VL}
	vpermilpd	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpermilpd	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vpermilpd	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vpermilpd	ymm30, ymm29, 123	 # AVX512{F,VL}
	vpermilpd	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpermilpd	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpermilpd	ymm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpermilpd	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpermilpd	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpermilpd	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpermilpd	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpermilpd	ymm30, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermilpd	ymm30, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpermilpd	ymm30, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermilpd	ymm30, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpermilpd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpermilpd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpermilpd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpermilpd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermilpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermilpd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpermilpd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpermilpd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpermilpd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpermilpd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpermilpd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpermilpd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpermilpd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpermilpd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpermilpd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpermilpd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpermilpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpermilpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermilpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermilpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpermilpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpermilpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpermilpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpermilpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpermilpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpermilpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpermilpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpermilpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpermilps	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpermilps	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vpermilps	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vpermilps	xmm30, xmm29, 123	 # AVX512{F,VL}
	vpermilps	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpermilps	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpermilps	xmm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpermilps	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vpermilps	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vpermilps	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vpermilps	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vpermilps	xmm30, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermilps	xmm30, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	vpermilps	xmm30, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermilps	xmm30, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	vpermilps	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpermilps	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vpermilps	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vpermilps	ymm30, ymm29, 123	 # AVX512{F,VL}
	vpermilps	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpermilps	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpermilps	ymm30, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vpermilps	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpermilps	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpermilps	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpermilps	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpermilps	ymm30, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpermilps	ymm30, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vpermilps	ymm30, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpermilps	ymm30, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vpermilps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpermilps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpermilps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpermilps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermilps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermilps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpermilps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpermilps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpermilps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpermilps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpermilps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpermilps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpermilps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpermilps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpermilps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpermilps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpermilps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpermilps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermilps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermilps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpermilps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpermilps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpermilps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpermilps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpermilps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpermilps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpermilps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpermilps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpermpd	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpermpd	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vpermpd	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vpermpd	ymm30, ymm29, 123	 # AVX512{F,VL}
	vpermpd	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpermpd	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpermpd	ymm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpermpd	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpermpd	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpermpd	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpermpd	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpermpd	ymm30, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermpd	ymm30, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpermpd	ymm30, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermpd	ymm30, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpermps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpermps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpermps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpermps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpermps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpermps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpermps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpermps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpermps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpermps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpermps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpermps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpermq	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpermq	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vpermq	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vpermq	ymm30, ymm29, 123	 # AVX512{F,VL}
	vpermq	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpermq	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpermq	ymm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpermq	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpermq	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpermq	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpermq	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpermq	ymm30, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermq	ymm30, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpermq	ymm30, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermq	ymm30, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpexpandd	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpexpandd	xmm30{k7}, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpexpandd	xmm30{k7}{z}, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpexpandd	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpexpandd	xmm30, XMMWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpexpandd	xmm30, XMMWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpexpandd	xmm30, XMMWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpexpandd	xmm30, XMMWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpexpandd	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpexpandd	ymm30{k7}, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpexpandd	ymm30{k7}{z}, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpexpandd	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpexpandd	ymm30, YMMWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpexpandd	ymm30, YMMWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpexpandd	ymm30, YMMWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpexpandd	ymm30, YMMWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpexpandd	xmm30, xmm29	 # AVX512{F,VL}
	vpexpandd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpexpandd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpexpandd	ymm30, ymm29	 # AVX512{F,VL}
	vpexpandd	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vpexpandd	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpexpandq	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpexpandq	xmm30{k7}, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpexpandq	xmm30{k7}{z}, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpexpandq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpexpandq	xmm30, XMMWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpexpandq	xmm30, XMMWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpexpandq	xmm30, XMMWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpexpandq	xmm30, XMMWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpexpandq	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpexpandq	ymm30{k7}, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpexpandq	ymm30{k7}{z}, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpexpandq	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpexpandq	ymm30, YMMWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpexpandq	ymm30, YMMWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpexpandq	ymm30, YMMWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpexpandq	ymm30, YMMWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpexpandq	xmm30, xmm29	 # AVX512{F,VL}
	vpexpandq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpexpandq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpexpandq	ymm30, ymm29	 # AVX512{F,VL}
	vpexpandq	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vpexpandq	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpgatherdd	xmm30{k1}, [r14+xmm31*8-123]	 # AVX512{F,VL}
	vpgatherdd	xmm30{k1}, [r9+xmm31+256]	 # AVX512{F,VL}
	vpgatherdd	xmm30{k1}, [rcx+xmm31*4+1024]	 # AVX512{F,VL}
	vpgatherdd	ymm30{k1}, [r14+ymm31*8-123]	 # AVX512{F,VL}
	vpgatherdd	ymm30{k1}, [r9+ymm31+256]	 # AVX512{F,VL}
	vpgatherdd	ymm30{k1}, [rcx+ymm31*4+1024]	 # AVX512{F,VL}
	vpgatherdq	xmm30{k1}, [r14+xmm31*8-123]	 # AVX512{F,VL}
	vpgatherdq	xmm30{k1}, [r9+xmm31+256]	 # AVX512{F,VL}
	vpgatherdq	xmm30{k1}, [rcx+xmm31*4+1024]	 # AVX512{F,VL}
	vpgatherdq	ymm30{k1}, [r14+xmm31*8-123]	 # AVX512{F,VL}
	vpgatherdq	ymm30{k1}, [r9+xmm31+256]	 # AVX512{F,VL}
	vpgatherdq	ymm30{k1}, [rcx+xmm31*4+1024]	 # AVX512{F,VL}
	vpgatherqd	xmm30{k1}, [r14+xmm31*8-123]	 # AVX512{F,VL}
	vpgatherqd	xmm30{k1}, [r9+xmm31+256]	 # AVX512{F,VL}
	vpgatherqd	xmm30{k1}, [rcx+xmm31*4+1024]	 # AVX512{F,VL}
	vpgatherqd	xmm30{k1}, [r14+ymm31*8-123]	 # AVX512{F,VL}
	vpgatherqd	xmm30{k1}, [r9+ymm31+256]	 # AVX512{F,VL}
	vpgatherqd	xmm30{k1}, [rcx+ymm31*4+1024]	 # AVX512{F,VL}
	vpgatherqq	xmm30{k1}, [r14+xmm31*8-123]	 # AVX512{F,VL}
	vpgatherqq	xmm30{k1}, [r9+xmm31+256]	 # AVX512{F,VL}
	vpgatherqq	xmm30{k1}, [rcx+xmm31*4+1024]	 # AVX512{F,VL}
	vpgatherqq	ymm30{k1}, [r14+ymm31*8-123]	 # AVX512{F,VL}
	vpgatherqq	ymm30{k1}, [r9+ymm31+256]	 # AVX512{F,VL}
	vpgatherqq	ymm30{k1}, [rcx+ymm31*4+1024]	 # AVX512{F,VL}
	vpmaxsd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpmaxsd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpmaxsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpmaxsd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmaxsd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmaxsd	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpmaxsd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpmaxsd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpmaxsd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpmaxsd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpmaxsd	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxsd	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpmaxsd	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxsd	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpmaxsd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpmaxsd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpmaxsd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpmaxsd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmaxsd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmaxsd	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpmaxsd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpmaxsd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpmaxsd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpmaxsd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpmaxsd	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpmaxsd	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpmaxsd	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpmaxsd	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpmaxsq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpmaxsq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpmaxsq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpmaxsq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmaxsq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmaxsq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpmaxsq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpmaxsq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpmaxsq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpmaxsq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpmaxsq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpmaxsq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpmaxsq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpmaxsq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpmaxsq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpmaxsq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpmaxsq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpmaxsq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmaxsq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmaxsq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpmaxsq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpmaxsq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpmaxsq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpmaxsq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpmaxsq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxsq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpmaxsq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxsq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpmaxud	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpmaxud	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpmaxud	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpmaxud	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmaxud	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmaxud	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpmaxud	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpmaxud	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpmaxud	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpmaxud	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpmaxud	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxud	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpmaxud	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxud	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpmaxud	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpmaxud	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpmaxud	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpmaxud	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmaxud	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmaxud	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpmaxud	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpmaxud	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpmaxud	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpmaxud	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpmaxud	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpmaxud	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpmaxud	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpmaxud	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpmaxuq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpmaxuq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpmaxuq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpmaxuq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmaxuq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmaxuq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpmaxuq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpmaxuq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpmaxuq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpmaxuq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpmaxuq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpmaxuq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpmaxuq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpmaxuq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpmaxuq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpmaxuq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpmaxuq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpmaxuq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmaxuq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmaxuq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpmaxuq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpmaxuq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpmaxuq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpmaxuq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpmaxuq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxuq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpmaxuq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxuq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpminsd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpminsd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpminsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpminsd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpminsd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpminsd	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpminsd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpminsd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpminsd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpminsd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpminsd	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpminsd	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpminsd	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpminsd	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpminsd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpminsd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpminsd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpminsd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpminsd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpminsd	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpminsd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpminsd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpminsd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpminsd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpminsd	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpminsd	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpminsd	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpminsd	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpminsq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpminsq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpminsq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpminsq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpminsq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpminsq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpminsq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpminsq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpminsq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpminsq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpminsq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpminsq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpminsq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpminsq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpminsq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpminsq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpminsq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpminsq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpminsq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpminsq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpminsq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpminsq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpminsq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpminsq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpminsq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpminsq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpminsq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpminsq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpminud	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpminud	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpminud	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpminud	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpminud	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpminud	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpminud	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpminud	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpminud	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpminud	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpminud	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpminud	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpminud	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpminud	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpminud	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpminud	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpminud	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpminud	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpminud	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpminud	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpminud	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpminud	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpminud	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpminud	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpminud	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpminud	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpminud	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpminud	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpminuq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpminuq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpminuq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpminuq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpminuq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpminuq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpminuq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpminuq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpminuq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpminuq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpminuq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpminuq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpminuq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpminuq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpminuq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpminuq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpminuq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpminuq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpminuq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpminuq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpminuq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpminuq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpminuq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpminuq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpminuq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpminuq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpminuq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpminuq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpmovsxbd	xmm30, xmm29	 # AVX512{F,VL}
	vpmovsxbd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxbd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxbd	xmm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxbd	xmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxbd	xmm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpmovsxbd	xmm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpmovsxbd	xmm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpmovsxbd	xmm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpmovsxbd	ymm30, xmm29	 # AVX512{F,VL}
	vpmovsxbd	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxbd	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxbd	ymm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxbd	ymm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxbd	ymm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpmovsxbd	ymm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpmovsxbd	ymm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpmovsxbd	ymm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpmovsxbq	xmm30, xmm29	 # AVX512{F,VL}
	vpmovsxbq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxbq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxbq	xmm30, WORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxbq	xmm30, WORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxbq	xmm30, WORD PTR [rdx+254]	 # AVX512{F,VL} Disp8
	vpmovsxbq	xmm30, WORD PTR [rdx+256]	 # AVX512{F,VL}
	vpmovsxbq	xmm30, WORD PTR [rdx-256]	 # AVX512{F,VL} Disp8
	vpmovsxbq	xmm30, WORD PTR [rdx-258]	 # AVX512{F,VL}
	vpmovsxbq	ymm30, xmm29	 # AVX512{F,VL}
	vpmovsxbq	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxbq	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxbq	ymm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxbq	ymm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxbq	ymm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpmovsxbq	ymm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpmovsxbq	ymm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpmovsxbq	ymm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpmovsxdq	xmm30, xmm29	 # AVX512{F,VL}
	vpmovsxdq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxdq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxdq	xmm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxdq	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxdq	xmm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpmovsxdq	xmm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpmovsxdq	xmm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpmovsxdq	xmm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpmovsxdq	ymm30, xmm29	 # AVX512{F,VL}
	vpmovsxdq	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxdq	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxdq	ymm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxdq	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxdq	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpmovsxdq	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpmovsxdq	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpmovsxdq	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpmovsxwd	xmm30, xmm29	 # AVX512{F,VL}
	vpmovsxwd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxwd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxwd	xmm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxwd	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxwd	xmm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpmovsxwd	xmm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpmovsxwd	xmm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpmovsxwd	xmm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpmovsxwd	ymm30, xmm29	 # AVX512{F,VL}
	vpmovsxwd	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxwd	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxwd	ymm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxwd	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxwd	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpmovsxwd	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpmovsxwd	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpmovsxwd	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpmovsxwq	xmm30, xmm29	 # AVX512{F,VL}
	vpmovsxwq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxwq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxwq	xmm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxwq	xmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxwq	xmm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpmovsxwq	xmm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpmovsxwq	xmm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpmovsxwq	xmm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpmovsxwq	ymm30, xmm29	 # AVX512{F,VL}
	vpmovsxwq	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsxwq	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsxwq	ymm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovsxwq	ymm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovsxwq	ymm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpmovsxwq	ymm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpmovsxwq	ymm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpmovsxwq	ymm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpmovzxbd	xmm30, xmm29	 # AVX512{F,VL}
	vpmovzxbd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxbd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxbd	xmm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxbd	xmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxbd	xmm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpmovzxbd	xmm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpmovzxbd	xmm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpmovzxbd	xmm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpmovzxbd	ymm30, xmm29	 # AVX512{F,VL}
	vpmovzxbd	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxbd	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxbd	ymm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxbd	ymm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxbd	ymm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpmovzxbd	ymm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpmovzxbd	ymm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpmovzxbd	ymm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpmovzxbq	xmm30, xmm29	 # AVX512{F,VL}
	vpmovzxbq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxbq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxbq	xmm30, WORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxbq	xmm30, WORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxbq	xmm30, WORD PTR [rdx+254]	 # AVX512{F,VL} Disp8
	vpmovzxbq	xmm30, WORD PTR [rdx+256]	 # AVX512{F,VL}
	vpmovzxbq	xmm30, WORD PTR [rdx-256]	 # AVX512{F,VL} Disp8
	vpmovzxbq	xmm30, WORD PTR [rdx-258]	 # AVX512{F,VL}
	vpmovzxbq	ymm30, xmm29	 # AVX512{F,VL}
	vpmovzxbq	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxbq	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxbq	ymm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxbq	ymm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxbq	ymm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpmovzxbq	ymm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpmovzxbq	ymm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpmovzxbq	ymm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpmovzxdq	xmm30, xmm29	 # AVX512{F,VL}
	vpmovzxdq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxdq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxdq	xmm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxdq	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxdq	xmm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpmovzxdq	xmm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpmovzxdq	xmm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpmovzxdq	xmm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpmovzxdq	ymm30, xmm29	 # AVX512{F,VL}
	vpmovzxdq	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxdq	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxdq	ymm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxdq	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxdq	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpmovzxdq	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpmovzxdq	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpmovzxdq	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpmovzxwd	xmm30, xmm29	 # AVX512{F,VL}
	vpmovzxwd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxwd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxwd	xmm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxwd	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxwd	xmm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpmovzxwd	xmm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpmovzxwd	xmm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpmovzxwd	xmm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpmovzxwd	ymm30, xmm29	 # AVX512{F,VL}
	vpmovzxwd	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxwd	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxwd	ymm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxwd	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxwd	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpmovzxwd	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpmovzxwd	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpmovzxwd	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpmovzxwq	xmm30, xmm29	 # AVX512{F,VL}
	vpmovzxwq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxwq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxwq	xmm30, DWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxwq	xmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxwq	xmm30, DWORD PTR [rdx+508]	 # AVX512{F,VL} Disp8
	vpmovzxwq	xmm30, DWORD PTR [rdx+512]	 # AVX512{F,VL}
	vpmovzxwq	xmm30, DWORD PTR [rdx-512]	 # AVX512{F,VL} Disp8
	vpmovzxwq	xmm30, DWORD PTR [rdx-516]	 # AVX512{F,VL}
	vpmovzxwq	ymm30, xmm29	 # AVX512{F,VL}
	vpmovzxwq	ymm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovzxwq	ymm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovzxwq	ymm30, QWORD PTR [rcx]	 # AVX512{F,VL}
	vpmovzxwq	ymm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmovzxwq	ymm30, QWORD PTR [rdx+1016]	 # AVX512{F,VL} Disp8
	vpmovzxwq	ymm30, QWORD PTR [rdx+1024]	 # AVX512{F,VL}
	vpmovzxwq	ymm30, QWORD PTR [rdx-1024]	 # AVX512{F,VL} Disp8
	vpmovzxwq	ymm30, QWORD PTR [rdx-1032]	 # AVX512{F,VL}
	vpmuldq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpmuldq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpmuldq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpmuldq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmuldq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmuldq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpmuldq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpmuldq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpmuldq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpmuldq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpmuldq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpmuldq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpmuldq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpmuldq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpmuldq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpmuldq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpmuldq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpmuldq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmuldq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmuldq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpmuldq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpmuldq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpmuldq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpmuldq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpmuldq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpmuldq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpmuldq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpmuldq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpmulld	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpmulld	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpmulld	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpmulld	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmulld	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmulld	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpmulld	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpmulld	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpmulld	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpmulld	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpmulld	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpmulld	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpmulld	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpmulld	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpmulld	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpmulld	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpmulld	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpmulld	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmulld	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmulld	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpmulld	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpmulld	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpmulld	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpmulld	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpmulld	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpmulld	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpmulld	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpmulld	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpmuludq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpmuludq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpmuludq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpmuludq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmuludq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmuludq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpmuludq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpmuludq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpmuludq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpmuludq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpmuludq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpmuludq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpmuludq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpmuludq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpmuludq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpmuludq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpmuludq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpmuludq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpmuludq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpmuludq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpmuludq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpmuludq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpmuludq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpmuludq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpmuludq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpmuludq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpmuludq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpmuludq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpord	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpord	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpord	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpord	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpord	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpord	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpord	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpord	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpord	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpord	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpord	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpord	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpord	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpord	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpord	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpord	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpord	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpord	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpord	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpord	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpord	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpord	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpord	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpord	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpord	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpord	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpord	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpord	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vporq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vporq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vporq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vporq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vporq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vporq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vporq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vporq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vporq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vporq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vporq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vporq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vporq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vporq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vporq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vporq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vporq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vporq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vporq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vporq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vporq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vporq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vporq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vporq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vporq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vporq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vporq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vporq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpscatterdd	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterdd	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterdd	[r9+xmm31+256]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterdd	[rcx+xmm31*4+1024]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterdd	[r14+ymm31*8-123]{k1}, ymm30	 # AVX512{F,VL}
	vpscatterdd	[r14+ymm31*8-123]{k1}, ymm30	 # AVX512{F,VL}
	vpscatterdd	[r9+ymm31+256]{k1}, ymm30	 # AVX512{F,VL}
	vpscatterdd	[rcx+ymm31*4+1024]{k1}, ymm30	 # AVX512{F,VL}
	vpscatterdq	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterdq	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterdq	[r9+xmm31+256]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterdq	[rcx+xmm31*4+1024]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterdq	[r14+xmm31*8-123]{k1}, ymm30	 # AVX512{F,VL}
	vpscatterdq	[r14+xmm31*8-123]{k1}, ymm30	 # AVX512{F,VL}
	vpscatterdq	[r9+xmm31+256]{k1}, ymm30	 # AVX512{F,VL}
	vpscatterdq	[rcx+xmm31*4+1024]{k1}, ymm30	 # AVX512{F,VL}
	vpscatterqd	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterqd	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterqd	[r9+xmm31+256]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterqd	[rcx+xmm31*4+1024]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterqd	[r14+ymm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterqd	[r14+ymm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterqd	[r9+ymm31+256]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterqd	[rcx+ymm31*4+1024]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterqq	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterqq	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterqq	[r9+xmm31+256]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterqq	[rcx+xmm31*4+1024]{k1}, xmm30	 # AVX512{F,VL}
	vpscatterqq	[r14+ymm31*8-123]{k1}, ymm30	 # AVX512{F,VL}
	vpscatterqq	[r14+ymm31*8-123]{k1}, ymm30	 # AVX512{F,VL}
	vpscatterqq	[r9+ymm31+256]{k1}, ymm30	 # AVX512{F,VL}
	vpscatterqq	[rcx+ymm31*4+1024]{k1}, ymm30	 # AVX512{F,VL}
	vpshufd	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpshufd	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vpshufd	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vpshufd	xmm30, xmm29, 123	 # AVX512{F,VL}
	vpshufd	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpshufd	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpshufd	xmm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpshufd	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vpshufd	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vpshufd	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vpshufd	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vpshufd	xmm30, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpshufd	xmm30, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	vpshufd	xmm30, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpshufd	xmm30, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	vpshufd	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpshufd	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vpshufd	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vpshufd	ymm30, ymm29, 123	 # AVX512{F,VL}
	vpshufd	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpshufd	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpshufd	ymm30, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vpshufd	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpshufd	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpshufd	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpshufd	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpshufd	ymm30, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpshufd	ymm30, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vpshufd	ymm30, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpshufd	ymm30, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vpslld	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpslld	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpslld	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpslld	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpslld	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpslld	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpslld	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpslld	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpslld	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpslld	ymm30, ymm29, xmm28	 # AVX512{F,VL}
	vpslld	ymm30{k7}, ymm29, xmm28	 # AVX512{F,VL}
	vpslld	ymm30{k7}{z}, ymm29, xmm28	 # AVX512{F,VL}
	vpslld	ymm30, ymm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpslld	ymm30, ymm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpslld	ymm30, ymm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpslld	ymm30, ymm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpslld	ymm30, ymm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpslld	ymm30, ymm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsllq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpsllq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpsllq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpsllq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsllq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsllq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsllq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsllq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsllq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsllq	ymm30, ymm29, xmm28	 # AVX512{F,VL}
	vpsllq	ymm30{k7}, ymm29, xmm28	 # AVX512{F,VL}
	vpsllq	ymm30{k7}{z}, ymm29, xmm28	 # AVX512{F,VL}
	vpsllq	ymm30, ymm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsllq	ymm30, ymm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsllq	ymm30, ymm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsllq	ymm30, ymm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsllq	ymm30, ymm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsllq	ymm30, ymm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsllvd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpsllvd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpsllvd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpsllvd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsllvd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsllvd	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpsllvd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsllvd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsllvd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsllvd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsllvd	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpsllvd	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpsllvd	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpsllvd	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpsllvd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpsllvd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpsllvd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpsllvd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsllvd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsllvd	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpsllvd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpsllvd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpsllvd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpsllvd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpsllvd	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpsllvd	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpsllvd	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpsllvd	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpsllvq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpsllvq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpsllvq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpsllvq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsllvq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsllvq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpsllvq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsllvq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsllvq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsllvq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsllvq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpsllvq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpsllvq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpsllvq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpsllvq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpsllvq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpsllvq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpsllvq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsllvq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsllvq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpsllvq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpsllvq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpsllvq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpsllvq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpsllvq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpsllvq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpsllvq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpsllvq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpsrad	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpsrad	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpsrad	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpsrad	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsrad	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsrad	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsrad	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsrad	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsrad	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsrad	ymm30, ymm29, xmm28	 # AVX512{F,VL}
	vpsrad	ymm30{k7}, ymm29, xmm28	 # AVX512{F,VL}
	vpsrad	ymm30{k7}{z}, ymm29, xmm28	 # AVX512{F,VL}
	vpsrad	ymm30, ymm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsrad	ymm30, ymm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsrad	ymm30, ymm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsrad	ymm30, ymm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsrad	ymm30, ymm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsrad	ymm30, ymm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsraq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpsraq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpsraq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpsraq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsraq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsraq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsraq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsraq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsraq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsraq	ymm30, ymm29, xmm28	 # AVX512{F,VL}
	vpsraq	ymm30{k7}, ymm29, xmm28	 # AVX512{F,VL}
	vpsraq	ymm30{k7}{z}, ymm29, xmm28	 # AVX512{F,VL}
	vpsraq	ymm30, ymm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsraq	ymm30, ymm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsraq	ymm30, ymm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsraq	ymm30, ymm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsraq	ymm30, ymm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsraq	ymm30, ymm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsravd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpsravd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpsravd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpsravd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsravd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsravd	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpsravd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsravd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsravd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsravd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsravd	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpsravd	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpsravd	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpsravd	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpsravd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpsravd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpsravd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpsravd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsravd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsravd	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpsravd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpsravd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpsravd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpsravd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpsravd	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpsravd	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpsravd	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpsravd	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpsravq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpsravq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpsravq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpsravq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsravq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsravq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpsravq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsravq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsravq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsravq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsravq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpsravq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpsravq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpsravq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpsravq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpsravq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpsravq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpsravq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsravq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsravq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpsravq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpsravq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpsravq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpsravq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpsravq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpsravq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpsravq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpsravq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpsrld	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpsrld	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpsrld	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpsrld	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsrld	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsrld	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsrld	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsrld	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsrld	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsrld	ymm30, ymm29, xmm28	 # AVX512{F,VL}
	vpsrld	ymm30{k7}, ymm29, xmm28	 # AVX512{F,VL}
	vpsrld	ymm30{k7}{z}, ymm29, xmm28	 # AVX512{F,VL}
	vpsrld	ymm30, ymm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsrld	ymm30, ymm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsrld	ymm30, ymm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsrld	ymm30, ymm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsrld	ymm30, ymm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsrld	ymm30, ymm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsrlq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpsrlq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpsrlq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpsrlq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsrlq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsrlq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsrlq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsrlq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsrlq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsrlq	ymm30, ymm29, xmm28	 # AVX512{F,VL}
	vpsrlq	ymm30{k7}, ymm29, xmm28	 # AVX512{F,VL}
	vpsrlq	ymm30{k7}{z}, ymm29, xmm28	 # AVX512{F,VL}
	vpsrlq	ymm30, ymm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsrlq	ymm30, ymm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsrlq	ymm30, ymm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsrlq	ymm30, ymm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsrlq	ymm30, ymm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsrlq	ymm30, ymm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsrlvd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpsrlvd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpsrlvd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpsrlvd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsrlvd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsrlvd	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpsrlvd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsrlvd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsrlvd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsrlvd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsrlvd	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpsrlvd	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpsrlvd	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpsrlvd	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpsrlvd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpsrlvd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpsrlvd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpsrlvd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsrlvd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsrlvd	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpsrlvd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpsrlvd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpsrlvd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpsrlvd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpsrlvd	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpsrlvd	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpsrlvd	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpsrlvd	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpsrlvq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpsrlvq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpsrlvq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpsrlvq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsrlvq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsrlvq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpsrlvq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsrlvq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsrlvq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsrlvq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsrlvq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpsrlvq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpsrlvq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpsrlvq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpsrlvq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpsrlvq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpsrlvq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpsrlvq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsrlvq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsrlvq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpsrlvq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpsrlvq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpsrlvq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpsrlvq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpsrlvq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpsrlvq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpsrlvq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpsrlvq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpsrld	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpsrld	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vpsrld	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vpsrld	xmm30, xmm29, 123	 # AVX512{F,VL}
	vpsrld	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpsrld	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpsrld	xmm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpsrld	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vpsrld	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vpsrld	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vpsrld	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vpsrld	xmm30, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsrld	xmm30, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	vpsrld	xmm30, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsrld	xmm30, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	vpsrld	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpsrld	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vpsrld	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vpsrld	ymm30, ymm29, 123	 # AVX512{F,VL}
	vpsrld	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpsrld	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpsrld	ymm30, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vpsrld	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpsrld	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpsrld	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpsrld	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpsrld	ymm30, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpsrld	ymm30, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vpsrld	ymm30, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpsrld	ymm30, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vpsrlq	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpsrlq	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vpsrlq	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vpsrlq	xmm30, xmm29, 123	 # AVX512{F,VL}
	vpsrlq	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpsrlq	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpsrlq	xmm30, [rcx]{1to2}, 123	 # AVX512{F,VL}
	vpsrlq	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vpsrlq	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vpsrlq	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vpsrlq	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vpsrlq	xmm30, [rdx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpsrlq	xmm30, [rdx+1024]{1to2}, 123	 # AVX512{F,VL}
	vpsrlq	xmm30, [rdx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpsrlq	xmm30, [rdx-1032]{1to2}, 123	 # AVX512{F,VL}
	vpsrlq	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpsrlq	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vpsrlq	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vpsrlq	ymm30, ymm29, 123	 # AVX512{F,VL}
	vpsrlq	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpsrlq	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpsrlq	ymm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpsrlq	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpsrlq	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpsrlq	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpsrlq	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpsrlq	ymm30, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsrlq	ymm30, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpsrlq	ymm30, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsrlq	ymm30, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpsubd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpsubd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpsubd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpsubd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsubd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsubd	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpsubd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsubd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsubd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsubd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsubd	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpsubd	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpsubd	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpsubd	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpsubd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpsubd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpsubd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpsubd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsubd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsubd	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpsubd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpsubd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpsubd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpsubd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpsubd	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpsubd	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpsubd	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpsubd	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpsubq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpsubq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpsubq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpsubq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsubq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsubq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpsubq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpsubq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpsubq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpsubq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpsubq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpsubq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpsubq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpsubq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpsubq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpsubq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpsubq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpsubq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpsubq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpsubq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpsubq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpsubq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpsubq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpsubq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpsubq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpsubq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpsubq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpsubq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vptestmd	k5, xmm30, xmm29	 # AVX512{F,VL}
	vptestmd	k5{k7}, xmm30, xmm29	 # AVX512{F,VL}
	vptestmd	k5, xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vptestmd	k5, xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vptestmd	k5, xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vptestmd	k5, xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vptestmd	k5, xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vptestmd	k5, xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vptestmd	k5, xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vptestmd	k5, xmm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vptestmd	k5, xmm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vptestmd	k5, xmm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vptestmd	k5, xmm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vptestmd	k5, ymm30, ymm29	 # AVX512{F,VL}
	vptestmd	k5{k7}, ymm30, ymm29	 # AVX512{F,VL}
	vptestmd	k5, ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vptestmd	k5, ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vptestmd	k5, ymm30, [rcx]{1to8}	 # AVX512{F,VL}
	vptestmd	k5, ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vptestmd	k5, ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vptestmd	k5, ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vptestmd	k5, ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vptestmd	k5, ymm30, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vptestmd	k5, ymm30, [rdx+512]{1to8}	 # AVX512{F,VL}
	vptestmd	k5, ymm30, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vptestmd	k5, ymm30, [rdx-516]{1to8}	 # AVX512{F,VL}
	vptestmq	k5, xmm30, xmm29	 # AVX512{F,VL}
	vptestmq	k5{k7}, xmm30, xmm29	 # AVX512{F,VL}
	vptestmq	k5, xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vptestmq	k5, xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vptestmq	k5, xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vptestmq	k5, xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vptestmq	k5, xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vptestmq	k5, xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vptestmq	k5, xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vptestmq	k5, xmm30, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vptestmq	k5, xmm30, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vptestmq	k5, xmm30, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vptestmq	k5, xmm30, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vptestmq	k5, ymm30, ymm29	 # AVX512{F,VL}
	vptestmq	k5{k7}, ymm30, ymm29	 # AVX512{F,VL}
	vptestmq	k5, ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vptestmq	k5, ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vptestmq	k5, ymm30, [rcx]{1to4}	 # AVX512{F,VL}
	vptestmq	k5, ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vptestmq	k5, ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vptestmq	k5, ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vptestmq	k5, ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vptestmq	k5, ymm30, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vptestmq	k5, ymm30, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vptestmq	k5, ymm30, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vptestmq	k5, ymm30, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpunpckhdq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpunpckhdq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpunpckhdq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpunpckhdq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpunpckhdq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpunpckhdq	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpunpckhdq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpunpckhdq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpunpckhdq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpunpckhdq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpunpckhdq	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpunpckhdq	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpunpckhdq	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpunpckhdq	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpunpckhdq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpunpckhdq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpunpckhdq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpunpckhdq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpunpckhdq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpunpckhdq	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpunpckhdq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpunpckhdq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpunpckhdq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpunpckhdq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpunpckhdq	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpunpckhdq	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpunpckhdq	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpunpckhdq	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpunpckhqdq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpunpckhqdq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpunpckhqdq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpunpckhqdq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpunpckhqdq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpunpckhqdq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpunpckhqdq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpunpckhqdq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpunpckhqdq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpunpckhqdq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpunpckhqdq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpunpckhqdq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpunpckhqdq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpunpckhqdq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpunpckhqdq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpunpckhqdq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpunpckhqdq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpunpckhqdq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpunpckhqdq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpunpckhqdq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpunpckhqdq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpunpckhqdq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpunpckhqdq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpunpckhqdq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpunpckldq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpunpckldq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpunpckldq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpunpckldq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpunpckldq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpunpckldq	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpunpckldq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpunpckldq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpunpckldq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpunpckldq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpunpckldq	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpunpckldq	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpunpckldq	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpunpckldq	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpunpckldq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpunpckldq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpunpckldq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpunpckldq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpunpckldq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpunpckldq	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpunpckldq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpunpckldq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpunpckldq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpunpckldq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpunpckldq	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpunpckldq	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpunpckldq	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpunpckldq	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpunpcklqdq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpunpcklqdq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpunpcklqdq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpunpcklqdq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpunpcklqdq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpunpcklqdq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpunpcklqdq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpunpcklqdq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpunpcklqdq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpunpcklqdq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpunpcklqdq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpunpcklqdq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpunpcklqdq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpunpcklqdq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpunpcklqdq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpunpcklqdq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpunpcklqdq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpunpcklqdq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpunpcklqdq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpunpcklqdq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpunpcklqdq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpunpcklqdq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpunpcklqdq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpunpcklqdq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpxord	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpxord	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpxord	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpxord	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpxord	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpxord	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpxord	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpxord	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpxord	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpxord	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpxord	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpxord	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpxord	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpxord	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpxord	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpxord	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpxord	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpxord	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpxord	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpxord	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpxord	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpxord	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpxord	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpxord	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpxord	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpxord	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpxord	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpxord	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpxorq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpxorq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpxorq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpxorq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpxorq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpxorq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpxorq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpxorq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpxorq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpxorq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpxorq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpxorq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpxorq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpxorq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpxorq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpxorq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpxorq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpxorq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpxorq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpxorq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpxorq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpxorq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpxorq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpxorq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpxorq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpxorq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpxorq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpxorq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vrcp14pd	xmm30, xmm29	 # AVX512{F,VL}
	vrcp14pd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vrcp14pd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vrcp14pd	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vrcp14pd	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vrcp14pd	xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vrcp14pd	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vrcp14pd	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vrcp14pd	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vrcp14pd	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vrcp14pd	xmm30, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vrcp14pd	xmm30, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vrcp14pd	xmm30, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vrcp14pd	xmm30, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vrcp14pd	ymm30, ymm29	 # AVX512{F,VL}
	vrcp14pd	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vrcp14pd	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vrcp14pd	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vrcp14pd	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vrcp14pd	ymm30, [rcx]{1to4}	 # AVX512{F,VL}
	vrcp14pd	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vrcp14pd	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vrcp14pd	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vrcp14pd	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vrcp14pd	ymm30, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vrcp14pd	ymm30, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vrcp14pd	ymm30, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vrcp14pd	ymm30, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vrcp14ps	xmm30, xmm29	 # AVX512{F,VL}
	vrcp14ps	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vrcp14ps	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vrcp14ps	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vrcp14ps	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vrcp14ps	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vrcp14ps	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vrcp14ps	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vrcp14ps	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vrcp14ps	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vrcp14ps	xmm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vrcp14ps	xmm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vrcp14ps	xmm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vrcp14ps	xmm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vrcp14ps	ymm30, ymm29	 # AVX512{F,VL}
	vrcp14ps	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vrcp14ps	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vrcp14ps	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vrcp14ps	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vrcp14ps	ymm30, [rcx]{1to8}	 # AVX512{F,VL}
	vrcp14ps	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vrcp14ps	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vrcp14ps	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vrcp14ps	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vrcp14ps	ymm30, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vrcp14ps	ymm30, [rdx+512]{1to8}	 # AVX512{F,VL}
	vrcp14ps	ymm30, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vrcp14ps	ymm30, [rdx-516]{1to8}	 # AVX512{F,VL}
	vrsqrt14pd	xmm30, xmm29	 # AVX512{F,VL}
	vrsqrt14pd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vrsqrt14pd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vrsqrt14pd	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vrsqrt14pd	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vrsqrt14pd	xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vrsqrt14pd	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vrsqrt14pd	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vrsqrt14pd	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vrsqrt14pd	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vrsqrt14pd	xmm30, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	xmm30, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vrsqrt14pd	xmm30, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	xmm30, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vrsqrt14pd	ymm30, ymm29	 # AVX512{F,VL}
	vrsqrt14pd	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vrsqrt14pd	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vrsqrt14pd	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vrsqrt14pd	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vrsqrt14pd	ymm30, [rcx]{1to4}	 # AVX512{F,VL}
	vrsqrt14pd	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vrsqrt14pd	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vrsqrt14pd	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vrsqrt14pd	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vrsqrt14pd	ymm30, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	ymm30, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vrsqrt14pd	ymm30, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	ymm30, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vrsqrt14ps	xmm30, xmm29	 # AVX512{F,VL}
	vrsqrt14ps	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vrsqrt14ps	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vrsqrt14ps	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vrsqrt14ps	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vrsqrt14ps	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vrsqrt14ps	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vrsqrt14ps	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vrsqrt14ps	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vrsqrt14ps	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vrsqrt14ps	xmm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	xmm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vrsqrt14ps	xmm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	xmm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vrsqrt14ps	ymm30, ymm29	 # AVX512{F,VL}
	vrsqrt14ps	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vrsqrt14ps	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vrsqrt14ps	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vrsqrt14ps	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vrsqrt14ps	ymm30, [rcx]{1to8}	 # AVX512{F,VL}
	vrsqrt14ps	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vrsqrt14ps	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vrsqrt14ps	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vrsqrt14ps	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vrsqrt14ps	ymm30, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	ymm30, [rdx+512]{1to8}	 # AVX512{F,VL}
	vrsqrt14ps	ymm30, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	ymm30, [rdx-516]{1to8}	 # AVX512{F,VL}
	vscatterdpd	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vscatterdpd	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vscatterdpd	[r9+xmm31+256]{k1}, xmm30	 # AVX512{F,VL}
	vscatterdpd	[rcx+xmm31*4+1024]{k1}, xmm30	 # AVX512{F,VL}
	vscatterdpd	[r14+xmm31*8-123]{k1}, ymm30	 # AVX512{F,VL}
	vscatterdpd	[r14+xmm31*8-123]{k1}, ymm30	 # AVX512{F,VL}
	vscatterdpd	[r9+xmm31+256]{k1}, ymm30	 # AVX512{F,VL}
	vscatterdpd	[rcx+xmm31*4+1024]{k1}, ymm30	 # AVX512{F,VL}
	vscatterdps	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vscatterdps	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vscatterdps	[r9+xmm31+256]{k1}, xmm30	 # AVX512{F,VL}
	vscatterdps	[rcx+xmm31*4+1024]{k1}, xmm30	 # AVX512{F,VL}
	vscatterdps	[r14+ymm31*8-123]{k1}, ymm30	 # AVX512{F,VL}
	vscatterdps	[r14+ymm31*8-123]{k1}, ymm30	 # AVX512{F,VL}
	vscatterdps	[r9+ymm31+256]{k1}, ymm30	 # AVX512{F,VL}
	vscatterdps	[rcx+ymm31*4+1024]{k1}, ymm30	 # AVX512{F,VL}
	vscatterqpd	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vscatterqpd	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vscatterqpd	[r9+xmm31+256]{k1}, xmm30	 # AVX512{F,VL}
	vscatterqpd	[rcx+xmm31*4+1024]{k1}, xmm30	 # AVX512{F,VL}
	vscatterqpd	[r14+ymm31*8-123]{k1}, ymm30	 # AVX512{F,VL}
	vscatterqpd	[r14+ymm31*8-123]{k1}, ymm30	 # AVX512{F,VL}
	vscatterqpd	[r9+ymm31+256]{k1}, ymm30	 # AVX512{F,VL}
	vscatterqpd	[rcx+ymm31*4+1024]{k1}, ymm30	 # AVX512{F,VL}
	vscatterqps	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vscatterqps	[r14+xmm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vscatterqps	[r9+xmm31+256]{k1}, xmm30	 # AVX512{F,VL}
	vscatterqps	[rcx+xmm31*4+1024]{k1}, xmm30	 # AVX512{F,VL}
	vscatterqps	[r14+ymm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vscatterqps	[r14+ymm31*8-123]{k1}, xmm30	 # AVX512{F,VL}
	vscatterqps	[r9+ymm31+256]{k1}, xmm30	 # AVX512{F,VL}
	vscatterqps	[rcx+ymm31*4+1024]{k1}, xmm30	 # AVX512{F,VL}
	vshufpd	xmm30, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vshufpd	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vshufpd	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vshufpd	xmm30, xmm29, xmm28, 123	 # AVX512{F,VL}
	vshufpd	xmm30, xmm29, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vshufpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vshufpd	xmm30, xmm29, [rcx]{1to2}, 123	 # AVX512{F,VL}
	vshufpd	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vshufpd	xmm30, xmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vshufpd	xmm30, xmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vshufpd	xmm30, xmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vshufpd	xmm30, xmm29, [rdx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vshufpd	xmm30, xmm29, [rdx+1024]{1to2}, 123	 # AVX512{F,VL}
	vshufpd	xmm30, xmm29, [rdx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vshufpd	xmm30, xmm29, [rdx-1032]{1to2}, 123	 # AVX512{F,VL}
	vshufpd	ymm30, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshufpd	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshufpd	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshufpd	ymm30, ymm29, ymm28, 123	 # AVX512{F,VL}
	vshufpd	ymm30, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vshufpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vshufpd	ymm30, ymm29, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vshufpd	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vshufpd	ymm30, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vshufpd	ymm30, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vshufpd	ymm30, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vshufpd	ymm30, ymm29, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshufpd	ymm30, ymm29, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vshufpd	ymm30, ymm29, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshufpd	ymm30, ymm29, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vshufps	xmm30, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vshufps	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vshufps	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vshufps	xmm30, xmm29, xmm28, 123	 # AVX512{F,VL}
	vshufps	xmm30, xmm29, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vshufps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vshufps	xmm30, xmm29, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vshufps	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vshufps	xmm30, xmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vshufps	xmm30, xmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vshufps	xmm30, xmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vshufps	xmm30, xmm29, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshufps	xmm30, xmm29, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	vshufps	xmm30, xmm29, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshufps	xmm30, xmm29, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	vshufps	ymm30, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshufps	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshufps	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshufps	ymm30, ymm29, ymm28, 123	 # AVX512{F,VL}
	vshufps	ymm30, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vshufps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vshufps	ymm30, ymm29, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vshufps	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vshufps	ymm30, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vshufps	ymm30, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vshufps	ymm30, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vshufps	ymm30, ymm29, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vshufps	ymm30, ymm29, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vshufps	ymm30, ymm29, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vshufps	ymm30, ymm29, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vsqrtpd	xmm30, xmm29	 # AVX512{F,VL}
	vsqrtpd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vsqrtpd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vsqrtpd	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vsqrtpd	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vsqrtpd	xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vsqrtpd	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vsqrtpd	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vsqrtpd	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vsqrtpd	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vsqrtpd	xmm30, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vsqrtpd	xmm30, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vsqrtpd	xmm30, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vsqrtpd	xmm30, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vsqrtpd	ymm30, ymm29	 # AVX512{F,VL}
	vsqrtpd	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vsqrtpd	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vsqrtpd	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vsqrtpd	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vsqrtpd	ymm30, [rcx]{1to4}	 # AVX512{F,VL}
	vsqrtpd	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vsqrtpd	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vsqrtpd	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vsqrtpd	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vsqrtpd	ymm30, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vsqrtpd	ymm30, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vsqrtpd	ymm30, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vsqrtpd	ymm30, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vsqrtps	xmm30, xmm29	 # AVX512{F,VL}
	vsqrtps	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vsqrtps	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vsqrtps	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vsqrtps	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vsqrtps	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vsqrtps	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vsqrtps	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vsqrtps	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vsqrtps	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vsqrtps	xmm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vsqrtps	xmm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vsqrtps	xmm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vsqrtps	xmm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vsqrtps	ymm30, ymm29	 # AVX512{F,VL}
	vsqrtps	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vsqrtps	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vsqrtps	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vsqrtps	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vsqrtps	ymm30, [rcx]{1to8}	 # AVX512{F,VL}
	vsqrtps	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vsqrtps	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vsqrtps	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vsqrtps	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vsqrtps	ymm30, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vsqrtps	ymm30, [rdx+512]{1to8}	 # AVX512{F,VL}
	vsqrtps	ymm30, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vsqrtps	ymm30, [rdx-516]{1to8}	 # AVX512{F,VL}
	vsubpd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vsubpd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vsubpd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vsubpd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vsubpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vsubpd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vsubpd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vsubpd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vsubpd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vsubpd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vsubpd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vsubpd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vsubpd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vsubpd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vsubpd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vsubpd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vsubpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vsubpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vsubpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vsubpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vsubpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vsubpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vsubpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vsubpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vsubpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vsubpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vsubpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vsubpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vsubps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vsubps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vsubps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vsubps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vsubps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vsubps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vsubps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vsubps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vsubps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vsubps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vsubps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vsubps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vsubps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vsubps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vsubps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vsubps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vsubps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vsubps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vsubps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vsubps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vsubps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vsubps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vsubps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vsubps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vsubps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vsubps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vsubps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vsubps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vunpckhpd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vunpckhpd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vunpckhpd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vunpckhpd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vunpckhpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vunpckhpd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vunpckhpd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vunpckhpd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vunpckhpd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vunpckhpd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vunpckhpd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vunpckhpd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vunpckhpd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vunpckhpd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vunpckhpd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vunpckhpd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vunpckhpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vunpckhpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vunpckhpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vunpckhpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vunpckhpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vunpckhpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vunpckhpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vunpckhpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vunpckhpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vunpckhpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vunpckhpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vunpckhpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vunpckhps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vunpckhps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vunpckhps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vunpckhps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vunpckhps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vunpckhps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vunpckhps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vunpckhps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vunpckhps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vunpckhps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vunpckhps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vunpckhps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vunpckhps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vunpckhps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vunpckhps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vunpckhps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vunpckhps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vunpckhps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vunpckhps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vunpckhps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vunpckhps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vunpckhps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vunpckhps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vunpckhps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vunpckhps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vunpckhps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vunpckhps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vunpckhps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vunpcklpd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vunpcklpd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vunpcklpd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vunpcklpd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vunpcklpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vunpcklpd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vunpcklpd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vunpcklpd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vunpcklpd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vunpcklpd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vunpcklpd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vunpcklpd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vunpcklpd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vunpcklpd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vunpcklpd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vunpcklpd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vunpcklpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vunpcklpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vunpcklpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vunpcklpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vunpcklpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vunpcklpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vunpcklpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vunpcklpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vunpcklpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vunpcklpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vunpcklpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vunpcklpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vunpcklps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vunpcklps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vunpcklps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vunpcklps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vunpcklps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vunpcklps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vunpcklps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vunpcklps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vunpcklps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vunpcklps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vunpcklps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vunpcklps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vunpcklps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vunpcklps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vunpcklps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vunpcklps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vunpcklps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vunpcklps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vunpcklps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vunpcklps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vunpcklps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vunpcklps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vunpcklps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vunpcklps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vunpcklps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vunpcklps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vunpcklps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vunpcklps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpternlogd	xmm30, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vpternlogd	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vpternlogd	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vpternlogd	xmm30, xmm29, xmm28, 123	 # AVX512{F,VL}
	vpternlogd	xmm30, xmm29, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpternlogd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpternlogd	xmm30, xmm29, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpternlogd	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vpternlogd	xmm30, xmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vpternlogd	xmm30, xmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vpternlogd	xmm30, xmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vpternlogd	xmm30, xmm29, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpternlogd	xmm30, xmm29, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	vpternlogd	xmm30, xmm29, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpternlogd	xmm30, xmm29, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	vpternlogd	ymm30, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vpternlogd	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vpternlogd	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vpternlogd	ymm30, ymm29, ymm28, 123	 # AVX512{F,VL}
	vpternlogd	ymm30, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpternlogd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpternlogd	ymm30, ymm29, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vpternlogd	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpternlogd	ymm30, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpternlogd	ymm30, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpternlogd	ymm30, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpternlogd	ymm30, ymm29, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpternlogd	ymm30, ymm29, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vpternlogd	ymm30, ymm29, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpternlogd	ymm30, ymm29, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vpternlogq	xmm30, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vpternlogq	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vpternlogq	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vpternlogq	xmm30, xmm29, xmm28, 123	 # AVX512{F,VL}
	vpternlogq	xmm30, xmm29, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpternlogq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpternlogq	xmm30, xmm29, [rcx]{1to2}, 123	 # AVX512{F,VL}
	vpternlogq	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vpternlogq	xmm30, xmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vpternlogq	xmm30, xmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vpternlogq	xmm30, xmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vpternlogq	xmm30, xmm29, [rdx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpternlogq	xmm30, xmm29, [rdx+1024]{1to2}, 123	 # AVX512{F,VL}
	vpternlogq	xmm30, xmm29, [rdx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpternlogq	xmm30, xmm29, [rdx-1032]{1to2}, 123	 # AVX512{F,VL}
	vpternlogq	ymm30, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vpternlogq	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vpternlogq	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vpternlogq	ymm30, ymm29, ymm28, 123	 # AVX512{F,VL}
	vpternlogq	ymm30, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpternlogq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpternlogq	ymm30, ymm29, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpternlogq	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpternlogq	ymm30, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpternlogq	ymm30, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpternlogq	ymm30, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpternlogq	ymm30, ymm29, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpternlogq	ymm30, ymm29, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpternlogq	ymm30, ymm29, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpternlogq	ymm30, ymm29, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpmovqb	xmm30, xmm29	 # AVX512{F,VL}
	vpmovqb	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovqb	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovqb	xmm30, ymm29	 # AVX512{F,VL}
	vpmovqb	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vpmovqb	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpmovsqb	xmm30, xmm29	 # AVX512{F,VL}
	vpmovsqb	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsqb	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsqb	xmm30, ymm29	 # AVX512{F,VL}
	vpmovsqb	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vpmovsqb	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpmovusqb	xmm30, xmm29	 # AVX512{F,VL}
	vpmovusqb	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovusqb	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovusqb	xmm30, ymm29	 # AVX512{F,VL}
	vpmovusqb	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vpmovusqb	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpmovqw	xmm30, xmm29	 # AVX512{F,VL}
	vpmovqw	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovqw	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovqw	xmm30, ymm29	 # AVX512{F,VL}
	vpmovqw	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vpmovqw	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpmovsqw	xmm30, xmm29	 # AVX512{F,VL}
	vpmovsqw	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsqw	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsqw	xmm30, ymm29	 # AVX512{F,VL}
	vpmovsqw	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vpmovsqw	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpmovusqw	xmm30, xmm29	 # AVX512{F,VL}
	vpmovusqw	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovusqw	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovusqw	xmm30, ymm29	 # AVX512{F,VL}
	vpmovusqw	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vpmovusqw	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpmovqd	xmm30, xmm29	 # AVX512{F,VL}
	vpmovqd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovqd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovqd	xmm30, ymm29	 # AVX512{F,VL}
	vpmovqd	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vpmovqd	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpmovsqd	xmm30, xmm29	 # AVX512{F,VL}
	vpmovsqd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsqd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsqd	xmm30, ymm29	 # AVX512{F,VL}
	vpmovsqd	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vpmovsqd	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpmovusqd	xmm30, xmm29	 # AVX512{F,VL}
	vpmovusqd	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovusqd	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovusqd	xmm30, ymm29	 # AVX512{F,VL}
	vpmovusqd	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vpmovusqd	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpmovdb	xmm30, xmm29	 # AVX512{F,VL}
	vpmovdb	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovdb	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovdb	xmm30, ymm29	 # AVX512{F,VL}
	vpmovdb	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vpmovdb	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpmovsdb	xmm30, xmm29	 # AVX512{F,VL}
	vpmovsdb	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsdb	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsdb	xmm30, ymm29	 # AVX512{F,VL}
	vpmovsdb	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vpmovsdb	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpmovusdb	xmm30, xmm29	 # AVX512{F,VL}
	vpmovusdb	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovusdb	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovusdb	xmm30, ymm29	 # AVX512{F,VL}
	vpmovusdb	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vpmovusdb	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpmovdw	xmm30, xmm29	 # AVX512{F,VL}
	vpmovdw	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovdw	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovdw	xmm30, ymm29	 # AVX512{F,VL}
	vpmovdw	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vpmovdw	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpmovsdw	xmm30, xmm29	 # AVX512{F,VL}
	vpmovsdw	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovsdw	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovsdw	xmm30, ymm29	 # AVX512{F,VL}
	vpmovsdw	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vpmovsdw	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vpmovusdw	xmm30, xmm29	 # AVX512{F,VL}
	vpmovusdw	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpmovusdw	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpmovusdw	xmm30, ymm29	 # AVX512{F,VL}
	vpmovusdw	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vpmovusdw	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vshuff32x4	ymm30, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshuff32x4	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshuff32x4	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshuff32x4	ymm30, ymm29, ymm28, 123	 # AVX512{F,VL}
	vshuff32x4	ymm30, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vshuff32x4	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vshuff32x4	ymm30, ymm29, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vshuff32x4	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vshuff32x4	ymm30, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vshuff32x4	ymm30, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vshuff32x4	ymm30, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vshuff32x4	ymm30, ymm29, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vshuff32x4	ymm30, ymm29, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vshuff32x4	ymm30, ymm29, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vshuff32x4	ymm30, ymm29, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vshuff64x2	ymm30, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshuff64x2	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshuff64x2	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshuff64x2	ymm30, ymm29, ymm28, 123	 # AVX512{F,VL}
	vshuff64x2	ymm30, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vshuff64x2	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vshuff64x2	ymm30, ymm29, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vshuff64x2	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vshuff64x2	ymm30, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vshuff64x2	ymm30, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vshuff64x2	ymm30, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vshuff64x2	ymm30, ymm29, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshuff64x2	ymm30, ymm29, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vshuff64x2	ymm30, ymm29, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshuff64x2	ymm30, ymm29, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vshufi32x4	ymm30, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshufi32x4	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshufi32x4	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshufi32x4	ymm30, ymm29, ymm28, 123	 # AVX512{F,VL}
	vshufi32x4	ymm30, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vshufi32x4	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vshufi32x4	ymm30, ymm29, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vshufi32x4	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vshufi32x4	ymm30, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vshufi32x4	ymm30, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vshufi32x4	ymm30, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vshufi32x4	ymm30, ymm29, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vshufi32x4	ymm30, ymm29, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vshufi32x4	ymm30, ymm29, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vshufi32x4	ymm30, ymm29, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vshufi64x2	ymm30, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshufi64x2	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshufi64x2	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vshufi64x2	ymm30, ymm29, ymm28, 123	 # AVX512{F,VL}
	vshufi64x2	ymm30, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vshufi64x2	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vshufi64x2	ymm30, ymm29, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vshufi64x2	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vshufi64x2	ymm30, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vshufi64x2	ymm30, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vshufi64x2	ymm30, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vshufi64x2	ymm30, ymm29, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshufi64x2	ymm30, ymm29, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vshufi64x2	ymm30, ymm29, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshufi64x2	ymm30, ymm29, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpermq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpermq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpermq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpermq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpermq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpermq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpermq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpermq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpermq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpermq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpermq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpermq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpermpd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpermpd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpermpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpermpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpermpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpermpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpermpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpermpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpermpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpermpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpermpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpermpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpermt2d	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpermt2d	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpermt2d	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpermt2d	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermt2d	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermt2d	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpermt2d	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpermt2d	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpermt2d	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpermt2d	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpermt2d	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2d	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpermt2d	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2d	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpermt2d	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpermt2d	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpermt2d	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpermt2d	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermt2d	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermt2d	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpermt2d	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpermt2d	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpermt2d	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpermt2d	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpermt2d	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpermt2d	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpermt2d	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpermt2d	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpermt2q	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpermt2q	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpermt2q	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpermt2q	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermt2q	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermt2q	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpermt2q	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpermt2q	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpermt2q	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpermt2q	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpermt2q	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpermt2q	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpermt2q	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpermt2q	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpermt2q	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpermt2q	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpermt2q	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpermt2q	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermt2q	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermt2q	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpermt2q	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpermt2q	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpermt2q	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpermt2q	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpermt2q	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2q	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpermt2q	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2q	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpermt2ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpermt2ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpermt2ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpermt2ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermt2ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermt2ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpermt2ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpermt2ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpermt2ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpermt2ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpermt2ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpermt2ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpermt2ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpermt2ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpermt2ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpermt2ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermt2ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermt2ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpermt2ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpermt2ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpermt2ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpermt2ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpermt2ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpermt2ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpermt2ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpermt2ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpermt2pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpermt2pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpermt2pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpermt2pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermt2pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermt2pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpermt2pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpermt2pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpermt2pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpermt2pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpermt2pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpermt2pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpermt2pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpermt2pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpermt2pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpermt2pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpermt2pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpermt2pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermt2pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermt2pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpermt2pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpermt2pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpermt2pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpermt2pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpermt2pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpermt2pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	valignq	xmm30, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	valignq	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	valignq	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	valignq	xmm30, xmm29, xmm28, 123	 # AVX512{F,VL}
	valignq	xmm30, xmm29, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	valignq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	valignq	xmm30, xmm29, [rcx]{1to2}, 123	 # AVX512{F,VL}
	valignq	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	valignq	xmm30, xmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	valignq	xmm30, xmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	valignq	xmm30, xmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	valignq	xmm30, xmm29, [rdx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	valignq	xmm30, xmm29, [rdx+1024]{1to2}, 123	 # AVX512{F,VL}
	valignq	xmm30, xmm29, [rdx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	valignq	xmm30, xmm29, [rdx-1032]{1to2}, 123	 # AVX512{F,VL}
	valignq	ymm30, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	valignq	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	valignq	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	valignq	ymm30, ymm29, ymm28, 123	 # AVX512{F,VL}
	valignq	ymm30, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	valignq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	valignq	ymm30, ymm29, [rcx]{1to4}, 123	 # AVX512{F,VL}
	valignq	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	valignq	ymm30, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	valignq	ymm30, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	valignq	ymm30, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	valignq	ymm30, ymm29, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	valignq	ymm30, ymm29, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	valignq	ymm30, ymm29, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	valignq	ymm30, ymm29, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vscalefpd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vscalefpd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vscalefpd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vscalefpd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vscalefpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vscalefpd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vscalefpd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vscalefpd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vscalefpd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vscalefpd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vscalefpd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vscalefpd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vscalefpd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vscalefpd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vscalefpd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vscalefpd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vscalefpd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vscalefpd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vscalefpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vscalefpd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vscalefpd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vscalefpd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vscalefpd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vscalefpd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vscalefpd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vscalefpd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vscalefpd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vscalefpd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vscalefps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vscalefps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vscalefps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vscalefps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vscalefps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vscalefps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vscalefps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vscalefps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vscalefps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vscalefps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vscalefps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vscalefps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vscalefps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vscalefps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vscalefps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vscalefps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vscalefps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vscalefps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vscalefps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vscalefps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vscalefps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vscalefps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vscalefps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vscalefps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vscalefps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vscalefps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vscalefps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vscalefps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vfixupimmpd	xmm30, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vfixupimmpd	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vfixupimmpd	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vfixupimmpd	xmm30, xmm29, xmm28, 123	 # AVX512{F,VL}
	vfixupimmpd	xmm30, xmm29, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vfixupimmpd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vfixupimmpd	xmm30, xmm29, [rcx]{1to2}, 123	 # AVX512{F,VL}
	vfixupimmpd	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	xmm30, xmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vfixupimmpd	xmm30, xmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	xmm30, xmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vfixupimmpd	xmm30, xmm29, [rdx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	xmm30, xmm29, [rdx+1024]{1to2}, 123	 # AVX512{F,VL}
	vfixupimmpd	xmm30, xmm29, [rdx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	xmm30, xmm29, [rdx-1032]{1to2}, 123	 # AVX512{F,VL}
	vfixupimmpd	ymm30, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vfixupimmpd	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vfixupimmpd	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vfixupimmpd	ymm30, ymm29, ymm28, 123	 # AVX512{F,VL}
	vfixupimmpd	ymm30, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vfixupimmpd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vfixupimmpd	ymm30, ymm29, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vfixupimmpd	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	ymm30, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vfixupimmpd	ymm30, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	ymm30, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vfixupimmpd	ymm30, ymm29, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	ymm30, ymm29, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vfixupimmpd	ymm30, ymm29, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	ymm30, ymm29, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vfixupimmps	xmm30, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vfixupimmps	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vfixupimmps	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{F,VL}
	vfixupimmps	xmm30, xmm29, xmm28, 123	 # AVX512{F,VL}
	vfixupimmps	xmm30, xmm29, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vfixupimmps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vfixupimmps	xmm30, xmm29, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vfixupimmps	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vfixupimmps	xmm30, xmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vfixupimmps	xmm30, xmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vfixupimmps	xmm30, xmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vfixupimmps	xmm30, xmm29, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vfixupimmps	xmm30, xmm29, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	vfixupimmps	xmm30, xmm29, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vfixupimmps	xmm30, xmm29, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	vfixupimmps	ymm30, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vfixupimmps	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vfixupimmps	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{F,VL}
	vfixupimmps	ymm30, ymm29, ymm28, 123	 # AVX512{F,VL}
	vfixupimmps	ymm30, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vfixupimmps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vfixupimmps	ymm30, ymm29, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vfixupimmps	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vfixupimmps	ymm30, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vfixupimmps	ymm30, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vfixupimmps	ymm30, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vfixupimmps	ymm30, ymm29, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vfixupimmps	ymm30, ymm29, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vfixupimmps	ymm30, ymm29, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vfixupimmps	ymm30, ymm29, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vpslld	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpslld	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vpslld	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vpslld	xmm30, xmm29, 123	 # AVX512{F,VL}
	vpslld	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpslld	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpslld	xmm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpslld	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vpslld	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vpslld	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vpslld	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vpslld	xmm30, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpslld	xmm30, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	vpslld	xmm30, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpslld	xmm30, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	vpslld	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpslld	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vpslld	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vpslld	ymm30, ymm29, 123	 # AVX512{F,VL}
	vpslld	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpslld	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpslld	ymm30, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vpslld	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpslld	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpslld	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpslld	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpslld	ymm30, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpslld	ymm30, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vpslld	ymm30, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpslld	ymm30, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vpsllq	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpsllq	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vpsllq	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vpsllq	xmm30, xmm29, 123	 # AVX512{F,VL}
	vpsllq	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpsllq	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpsllq	xmm30, [rcx]{1to2}, 123	 # AVX512{F,VL}
	vpsllq	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vpsllq	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vpsllq	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vpsllq	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vpsllq	xmm30, [rdx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpsllq	xmm30, [rdx+1024]{1to2}, 123	 # AVX512{F,VL}
	vpsllq	xmm30, [rdx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpsllq	xmm30, [rdx-1032]{1to2}, 123	 # AVX512{F,VL}
	vpsllq	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpsllq	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vpsllq	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vpsllq	ymm30, ymm29, 123	 # AVX512{F,VL}
	vpsllq	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpsllq	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpsllq	ymm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpsllq	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpsllq	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpsllq	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpsllq	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpsllq	ymm30, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsllq	ymm30, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpsllq	ymm30, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsllq	ymm30, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpsrad	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpsrad	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vpsrad	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vpsrad	xmm30, xmm29, 123	 # AVX512{F,VL}
	vpsrad	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpsrad	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpsrad	xmm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpsrad	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vpsrad	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vpsrad	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vpsrad	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vpsrad	xmm30, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsrad	xmm30, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	vpsrad	xmm30, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsrad	xmm30, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	vpsrad	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpsrad	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vpsrad	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vpsrad	ymm30, ymm29, 123	 # AVX512{F,VL}
	vpsrad	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpsrad	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpsrad	ymm30, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vpsrad	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpsrad	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpsrad	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpsrad	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpsrad	ymm30, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpsrad	ymm30, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vpsrad	ymm30, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpsrad	ymm30, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vpsraq	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vpsraq	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vpsraq	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vpsraq	xmm30, xmm29, 123	 # AVX512{F,VL}
	vpsraq	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpsraq	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpsraq	xmm30, [rcx]{1to2}, 123	 # AVX512{F,VL}
	vpsraq	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vpsraq	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vpsraq	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vpsraq	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vpsraq	xmm30, [rdx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpsraq	xmm30, [rdx+1024]{1to2}, 123	 # AVX512{F,VL}
	vpsraq	xmm30, [rdx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpsraq	xmm30, [rdx-1032]{1to2}, 123	 # AVX512{F,VL}
	vpsraq	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vpsraq	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vpsraq	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vpsraq	ymm30, ymm29, 123	 # AVX512{F,VL}
	vpsraq	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vpsraq	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vpsraq	ymm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vpsraq	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vpsraq	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vpsraq	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vpsraq	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vpsraq	ymm30, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsraq	ymm30, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpsraq	ymm30, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsraq	ymm30, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vprolvd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vprolvd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vprolvd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vprolvd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vprolvd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vprolvd	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vprolvd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vprolvd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vprolvd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vprolvd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vprolvd	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vprolvd	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vprolvd	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vprolvd	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vprolvd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vprolvd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vprolvd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vprolvd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vprolvd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vprolvd	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vprolvd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vprolvd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vprolvd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vprolvd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vprolvd	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vprolvd	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vprolvd	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vprolvd	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vprold	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vprold	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vprold	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vprold	xmm30, xmm29, 123	 # AVX512{F,VL}
	vprold	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vprold	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vprold	xmm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vprold	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vprold	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vprold	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vprold	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vprold	xmm30, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprold	xmm30, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	vprold	xmm30, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprold	xmm30, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	vprold	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vprold	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vprold	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vprold	ymm30, ymm29, 123	 # AVX512{F,VL}
	vprold	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vprold	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vprold	ymm30, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vprold	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vprold	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vprold	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vprold	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vprold	ymm30, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vprold	ymm30, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vprold	ymm30, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vprold	ymm30, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vprolvq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vprolvq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vprolvq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vprolvq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vprolvq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vprolvq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vprolvq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vprolvq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vprolvq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vprolvq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vprolvq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vprolvq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vprolvq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vprolvq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vprolvq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vprolvq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vprolvq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vprolvq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vprolvq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vprolvq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vprolvq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vprolvq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vprolvq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vprolvq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vprolvq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vprolvq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vprolvq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vprolvq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vprolq	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vprolq	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vprolq	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vprolq	xmm30, xmm29, 123	 # AVX512{F,VL}
	vprolq	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vprolq	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vprolq	xmm30, [rcx]{1to2}, 123	 # AVX512{F,VL}
	vprolq	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vprolq	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vprolq	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vprolq	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vprolq	xmm30, [rdx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vprolq	xmm30, [rdx+1024]{1to2}, 123	 # AVX512{F,VL}
	vprolq	xmm30, [rdx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vprolq	xmm30, [rdx-1032]{1to2}, 123	 # AVX512{F,VL}
	vprolq	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vprolq	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vprolq	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vprolq	ymm30, ymm29, 123	 # AVX512{F,VL}
	vprolq	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vprolq	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vprolq	ymm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vprolq	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vprolq	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vprolq	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vprolq	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vprolq	ymm30, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprolq	ymm30, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vprolq	ymm30, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprolq	ymm30, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vprorvd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vprorvd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vprorvd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vprorvd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vprorvd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vprorvd	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vprorvd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vprorvd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vprorvd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vprorvd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vprorvd	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vprorvd	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vprorvd	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vprorvd	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vprorvd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vprorvd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vprorvd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vprorvd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vprorvd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vprorvd	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vprorvd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vprorvd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vprorvd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vprorvd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vprorvd	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vprorvd	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vprorvd	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vprorvd	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vprord	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vprord	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vprord	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vprord	xmm30, xmm29, 123	 # AVX512{F,VL}
	vprord	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vprord	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vprord	xmm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vprord	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vprord	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vprord	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vprord	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vprord	xmm30, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprord	xmm30, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	vprord	xmm30, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprord	xmm30, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	vprord	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vprord	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vprord	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vprord	ymm30, ymm29, 123	 # AVX512{F,VL}
	vprord	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vprord	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vprord	ymm30, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vprord	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vprord	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vprord	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vprord	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vprord	ymm30, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vprord	ymm30, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vprord	ymm30, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vprord	ymm30, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vprorvq	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vprorvq	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vprorvq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vprorvq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vprorvq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vprorvq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vprorvq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vprorvq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vprorvq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vprorvq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vprorvq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vprorvq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vprorvq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vprorvq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vprorvq	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vprorvq	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vprorvq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vprorvq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vprorvq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vprorvq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vprorvq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vprorvq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vprorvq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vprorvq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vprorvq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vprorvq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vprorvq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vprorvq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vprorq	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vprorq	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vprorq	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vprorq	xmm30, xmm29, 123	 # AVX512{F,VL}
	vprorq	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vprorq	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vprorq	xmm30, [rcx]{1to2}, 123	 # AVX512{F,VL}
	vprorq	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vprorq	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vprorq	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vprorq	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vprorq	xmm30, [rdx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vprorq	xmm30, [rdx+1024]{1to2}, 123	 # AVX512{F,VL}
	vprorq	xmm30, [rdx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vprorq	xmm30, [rdx-1032]{1to2}, 123	 # AVX512{F,VL}
	vprorq	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vprorq	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vprorq	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vprorq	ymm30, ymm29, 123	 # AVX512{F,VL}
	vprorq	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vprorq	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vprorq	ymm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vprorq	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vprorq	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vprorq	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vprorq	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vprorq	ymm30, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprorq	ymm30, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vprorq	ymm30, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprorq	ymm30, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vrndscalepd	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vrndscalepd	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vrndscalepd	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vrndscalepd	xmm30, xmm29, 123	 # AVX512{F,VL}
	vrndscalepd	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vrndscalepd	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vrndscalepd	xmm30, [rcx]{1to2}, 123	 # AVX512{F,VL}
	vrndscalepd	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vrndscalepd	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vrndscalepd	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vrndscalepd	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vrndscalepd	xmm30, [rdx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vrndscalepd	xmm30, [rdx+1024]{1to2}, 123	 # AVX512{F,VL}
	vrndscalepd	xmm30, [rdx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vrndscalepd	xmm30, [rdx-1032]{1to2}, 123	 # AVX512{F,VL}
	vrndscalepd	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vrndscalepd	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vrndscalepd	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vrndscalepd	ymm30, ymm29, 123	 # AVX512{F,VL}
	vrndscalepd	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vrndscalepd	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vrndscalepd	ymm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vrndscalepd	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vrndscalepd	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vrndscalepd	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vrndscalepd	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vrndscalepd	ymm30, [rdx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vrndscalepd	ymm30, [rdx+1024]{1to4}, 123	 # AVX512{F,VL}
	vrndscalepd	ymm30, [rdx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vrndscalepd	ymm30, [rdx-1032]{1to4}, 123	 # AVX512{F,VL}
	vrndscaleps	xmm30, xmm29, 0xab	 # AVX512{F,VL}
	vrndscaleps	xmm30{k7}, xmm29, 0xab	 # AVX512{F,VL}
	vrndscaleps	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{F,VL}
	vrndscaleps	xmm30, xmm29, 123	 # AVX512{F,VL}
	vrndscaleps	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vrndscaleps	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vrndscaleps	xmm30, [rcx]{1to4}, 123	 # AVX512{F,VL}
	vrndscaleps	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{F,VL} Disp8
	vrndscaleps	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{F,VL}
	vrndscaleps	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{F,VL} Disp8
	vrndscaleps	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{F,VL}
	vrndscaleps	xmm30, [rdx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vrndscaleps	xmm30, [rdx+512]{1to4}, 123	 # AVX512{F,VL}
	vrndscaleps	xmm30, [rdx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vrndscaleps	xmm30, [rdx-516]{1to4}, 123	 # AVX512{F,VL}
	vrndscaleps	ymm30, ymm29, 0xab	 # AVX512{F,VL}
	vrndscaleps	ymm30{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vrndscaleps	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{F,VL}
	vrndscaleps	ymm30, ymm29, 123	 # AVX512{F,VL}
	vrndscaleps	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{F,VL}
	vrndscaleps	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{F,VL}
	vrndscaleps	ymm30, [rcx]{1to8}, 123	 # AVX512{F,VL}
	vrndscaleps	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{F,VL} Disp8
	vrndscaleps	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{F,VL}
	vrndscaleps	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{F,VL} Disp8
	vrndscaleps	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{F,VL}
	vrndscaleps	ymm30, [rdx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vrndscaleps	ymm30, [rdx+512]{1to8}, 123	 # AVX512{F,VL}
	vrndscaleps	ymm30, [rdx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vrndscaleps	ymm30, [rdx-516]{1to8}, 123	 # AVX512{F,VL}
	vpcompressq	XMMWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpcompressq	XMMWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpcompressq	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpcompressq	XMMWORD PTR [rdx+1016], xmm30	 # AVX512{F,VL} Disp8
	vpcompressq	XMMWORD PTR [rdx+1024], xmm30	 # AVX512{F,VL}
	vpcompressq	XMMWORD PTR [rdx-1024], xmm30	 # AVX512{F,VL} Disp8
	vpcompressq	XMMWORD PTR [rdx-1032], xmm30	 # AVX512{F,VL}
	vpcompressq	YMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpcompressq	YMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpcompressq	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpcompressq	YMMWORD PTR [rdx+1016], ymm30	 # AVX512{F,VL} Disp8
	vpcompressq	YMMWORD PTR [rdx+1024], ymm30	 # AVX512{F,VL}
	vpcompressq	YMMWORD PTR [rdx-1024], ymm30	 # AVX512{F,VL} Disp8
	vpcompressq	YMMWORD PTR [rdx-1032], ymm30	 # AVX512{F,VL}
	vpcompressq	xmm30, xmm29	 # AVX512{F,VL}
	vpcompressq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vpcompressq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vpcompressq	ymm30, ymm29	 # AVX512{F,VL}
	vpcompressq	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vpcompressq	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vcvtps2ph	QWORD PTR [rcx], xmm30, 0xab	 # AVX512{F,VL}
	vcvtps2ph	QWORD PTR [rcx]{k7}, xmm30, 0xab	 # AVX512{F,VL}
	vcvtps2ph	QWORD PTR [rcx], xmm30, 123	 # AVX512{F,VL}
	vcvtps2ph	QWORD PTR [rax+r14*8+0x1234], xmm30, 123	 # AVX512{F,VL}
	vcvtps2ph	QWORD PTR [rdx+1016], xmm30, 123	 # AVX512{F,VL} Disp8
	vcvtps2ph	QWORD PTR [rdx+1024], xmm30, 123	 # AVX512{F,VL}
	vcvtps2ph	QWORD PTR [rdx-1024], xmm30, 123	 # AVX512{F,VL} Disp8
	vcvtps2ph	QWORD PTR [rdx-1032], xmm30, 123	 # AVX512{F,VL}
	vcvtps2ph	XMMWORD PTR [rcx], ymm30, 0xab	 # AVX512{F,VL}
	vcvtps2ph	XMMWORD PTR [rcx]{k7}, ymm30, 0xab	 # AVX512{F,VL}
	vcvtps2ph	XMMWORD PTR [rcx], ymm30, 123	 # AVX512{F,VL}
	vcvtps2ph	XMMWORD PTR [rax+r14*8+0x1234], ymm30, 123	 # AVX512{F,VL}
	vcvtps2ph	XMMWORD PTR [rdx+2032], ymm30, 123	 # AVX512{F,VL} Disp8
	vcvtps2ph	XMMWORD PTR [rdx+2048], ymm30, 123	 # AVX512{F,VL}
	vcvtps2ph	XMMWORD PTR [rdx-2048], ymm30, 123	 # AVX512{F,VL} Disp8
	vcvtps2ph	XMMWORD PTR [rdx-2064], ymm30, 123	 # AVX512{F,VL}
	vextractf32x4	XMMWORD PTR [rcx], ymm29, 0xab	 # AVX512{F,VL}
	vextractf32x4	XMMWORD PTR [rcx]{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vextractf32x4	XMMWORD PTR [rcx], ymm29, 123	 # AVX512{F,VL}
	vextractf32x4	XMMWORD PTR [rax+r14*8+0x1234], ymm29, 123	 # AVX512{F,VL}
	vextractf32x4	XMMWORD PTR [rdx+2032], ymm29, 123	 # AVX512{F,VL} Disp8
	vextractf32x4	XMMWORD PTR [rdx+2048], ymm29, 123	 # AVX512{F,VL}
	vextractf32x4	XMMWORD PTR [rdx-2048], ymm29, 123	 # AVX512{F,VL} Disp8
	vextractf32x4	XMMWORD PTR [rdx-2064], ymm29, 123	 # AVX512{F,VL}
	vextracti32x4	XMMWORD PTR [rcx], ymm29, 0xab	 # AVX512{F,VL}
	vextracti32x4	XMMWORD PTR [rcx]{k7}, ymm29, 0xab	 # AVX512{F,VL}
	vextracti32x4	XMMWORD PTR [rcx], ymm29, 123	 # AVX512{F,VL}
	vextracti32x4	XMMWORD PTR [rax+r14*8+0x1234], ymm29, 123	 # AVX512{F,VL}
	vextracti32x4	XMMWORD PTR [rdx+2032], ymm29, 123	 # AVX512{F,VL} Disp8
	vextracti32x4	XMMWORD PTR [rdx+2048], ymm29, 123	 # AVX512{F,VL}
	vextracti32x4	XMMWORD PTR [rdx-2048], ymm29, 123	 # AVX512{F,VL} Disp8
	vextracti32x4	XMMWORD PTR [rdx-2064], ymm29, 123	 # AVX512{F,VL}
	vmovapd	XMMWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vmovapd	XMMWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vmovapd	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vmovapd	XMMWORD PTR [rdx+2032], xmm30	 # AVX512{F,VL} Disp8
	vmovapd	XMMWORD PTR [rdx+2048], xmm30	 # AVX512{F,VL}
	vmovapd	XMMWORD PTR [rdx-2048], xmm30	 # AVX512{F,VL} Disp8
	vmovapd	XMMWORD PTR [rdx-2064], xmm30	 # AVX512{F,VL}
	vmovapd	YMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vmovapd	YMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vmovapd	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vmovapd	YMMWORD PTR [rdx+4064], ymm30	 # AVX512{F,VL} Disp8
	vmovapd	YMMWORD PTR [rdx+4096], ymm30	 # AVX512{F,VL}
	vmovapd	YMMWORD PTR [rdx-4096], ymm30	 # AVX512{F,VL} Disp8
	vmovapd	YMMWORD PTR [rdx-4128], ymm30	 # AVX512{F,VL}
	vmovaps	XMMWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vmovaps	XMMWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vmovaps	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vmovaps	XMMWORD PTR [rdx+2032], xmm30	 # AVX512{F,VL} Disp8
	vmovaps	XMMWORD PTR [rdx+2048], xmm30	 # AVX512{F,VL}
	vmovaps	XMMWORD PTR [rdx-2048], xmm30	 # AVX512{F,VL} Disp8
	vmovaps	XMMWORD PTR [rdx-2064], xmm30	 # AVX512{F,VL}
	vmovaps	YMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vmovaps	YMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vmovaps	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vmovaps	YMMWORD PTR [rdx+4064], ymm30	 # AVX512{F,VL} Disp8
	vmovaps	YMMWORD PTR [rdx+4096], ymm30	 # AVX512{F,VL}
	vmovaps	YMMWORD PTR [rdx-4096], ymm30	 # AVX512{F,VL} Disp8
	vmovaps	YMMWORD PTR [rdx-4128], ymm30	 # AVX512{F,VL}
	vmovdqa32	XMMWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vmovdqa32	XMMWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vmovdqa32	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vmovdqa32	XMMWORD PTR [rdx+2032], xmm30	 # AVX512{F,VL} Disp8
	vmovdqa32	XMMWORD PTR [rdx+2048], xmm30	 # AVX512{F,VL}
	vmovdqa32	XMMWORD PTR [rdx-2048], xmm30	 # AVX512{F,VL} Disp8
	vmovdqa32	XMMWORD PTR [rdx-2064], xmm30	 # AVX512{F,VL}
	vmovdqa32	YMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vmovdqa32	YMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vmovdqa32	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vmovdqa32	YMMWORD PTR [rdx+4064], ymm30	 # AVX512{F,VL} Disp8
	vmovdqa32	YMMWORD PTR [rdx+4096], ymm30	 # AVX512{F,VL}
	vmovdqa32	YMMWORD PTR [rdx-4096], ymm30	 # AVX512{F,VL} Disp8
	vmovdqa32	YMMWORD PTR [rdx-4128], ymm30	 # AVX512{F,VL}
	vmovdqa64	XMMWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vmovdqa64	XMMWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vmovdqa64	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vmovdqa64	XMMWORD PTR [rdx+2032], xmm30	 # AVX512{F,VL} Disp8
	vmovdqa64	XMMWORD PTR [rdx+2048], xmm30	 # AVX512{F,VL}
	vmovdqa64	XMMWORD PTR [rdx-2048], xmm30	 # AVX512{F,VL} Disp8
	vmovdqa64	XMMWORD PTR [rdx-2064], xmm30	 # AVX512{F,VL}
	vmovdqa64	YMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vmovdqa64	YMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vmovdqa64	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vmovdqa64	YMMWORD PTR [rdx+4064], ymm30	 # AVX512{F,VL} Disp8
	vmovdqa64	YMMWORD PTR [rdx+4096], ymm30	 # AVX512{F,VL}
	vmovdqa64	YMMWORD PTR [rdx-4096], ymm30	 # AVX512{F,VL} Disp8
	vmovdqa64	YMMWORD PTR [rdx-4128], ymm30	 # AVX512{F,VL}
	vmovdqu32	XMMWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vmovdqu32	XMMWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vmovdqu32	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vmovdqu32	XMMWORD PTR [rdx+2032], xmm30	 # AVX512{F,VL} Disp8
	vmovdqu32	XMMWORD PTR [rdx+2048], xmm30	 # AVX512{F,VL}
	vmovdqu32	XMMWORD PTR [rdx-2048], xmm30	 # AVX512{F,VL} Disp8
	vmovdqu32	XMMWORD PTR [rdx-2064], xmm30	 # AVX512{F,VL}
	vmovdqu32	YMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vmovdqu32	YMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vmovdqu32	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vmovdqu32	YMMWORD PTR [rdx+4064], ymm30	 # AVX512{F,VL} Disp8
	vmovdqu32	YMMWORD PTR [rdx+4096], ymm30	 # AVX512{F,VL}
	vmovdqu32	YMMWORD PTR [rdx-4096], ymm30	 # AVX512{F,VL} Disp8
	vmovdqu32	YMMWORD PTR [rdx-4128], ymm30	 # AVX512{F,VL}
	vmovdqu64	XMMWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vmovdqu64	XMMWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vmovdqu64	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vmovdqu64	XMMWORD PTR [rdx+2032], xmm30	 # AVX512{F,VL} Disp8
	vmovdqu64	XMMWORD PTR [rdx+2048], xmm30	 # AVX512{F,VL}
	vmovdqu64	XMMWORD PTR [rdx-2048], xmm30	 # AVX512{F,VL} Disp8
	vmovdqu64	XMMWORD PTR [rdx-2064], xmm30	 # AVX512{F,VL}
	vmovdqu64	YMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vmovdqu64	YMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vmovdqu64	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vmovdqu64	YMMWORD PTR [rdx+4064], ymm30	 # AVX512{F,VL} Disp8
	vmovdqu64	YMMWORD PTR [rdx+4096], ymm30	 # AVX512{F,VL}
	vmovdqu64	YMMWORD PTR [rdx-4096], ymm30	 # AVX512{F,VL} Disp8
	vmovdqu64	YMMWORD PTR [rdx-4128], ymm30	 # AVX512{F,VL}
	vmovupd	XMMWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vmovupd	XMMWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vmovupd	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vmovupd	XMMWORD PTR [rdx+2032], xmm30	 # AVX512{F,VL} Disp8
	vmovupd	XMMWORD PTR [rdx+2048], xmm30	 # AVX512{F,VL}
	vmovupd	XMMWORD PTR [rdx-2048], xmm30	 # AVX512{F,VL} Disp8
	vmovupd	XMMWORD PTR [rdx-2064], xmm30	 # AVX512{F,VL}
	vmovupd	YMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vmovupd	YMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vmovupd	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vmovupd	YMMWORD PTR [rdx+4064], ymm30	 # AVX512{F,VL} Disp8
	vmovupd	YMMWORD PTR [rdx+4096], ymm30	 # AVX512{F,VL}
	vmovupd	YMMWORD PTR [rdx-4096], ymm30	 # AVX512{F,VL} Disp8
	vmovupd	YMMWORD PTR [rdx-4128], ymm30	 # AVX512{F,VL}
	vmovups	XMMWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vmovups	XMMWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vmovups	XMMWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vmovups	XMMWORD PTR [rdx+2032], xmm30	 # AVX512{F,VL} Disp8
	vmovups	XMMWORD PTR [rdx+2048], xmm30	 # AVX512{F,VL}
	vmovups	XMMWORD PTR [rdx-2048], xmm30	 # AVX512{F,VL} Disp8
	vmovups	XMMWORD PTR [rdx-2064], xmm30	 # AVX512{F,VL}
	vmovups	YMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vmovups	YMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vmovups	YMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vmovups	YMMWORD PTR [rdx+4064], ymm30	 # AVX512{F,VL} Disp8
	vmovups	YMMWORD PTR [rdx+4096], ymm30	 # AVX512{F,VL}
	vmovups	YMMWORD PTR [rdx-4096], ymm30	 # AVX512{F,VL} Disp8
	vmovups	YMMWORD PTR [rdx-4128], ymm30	 # AVX512{F,VL}
	vpmovqb	WORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpmovqb	WORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpmovqb	WORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpmovqb	WORD PTR [rdx+254], xmm30	 # AVX512{F,VL} Disp8
	vpmovqb	WORD PTR [rdx+256], xmm30	 # AVX512{F,VL}
	vpmovqb	WORD PTR [rdx-256], xmm30	 # AVX512{F,VL} Disp8
	vpmovqb	WORD PTR [rdx-258], xmm30	 # AVX512{F,VL}
	vpmovqb	DWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpmovqb	DWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpmovqb	DWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpmovqb	DWORD PTR [rdx+508], ymm30	 # AVX512{F,VL} Disp8
	vpmovqb	DWORD PTR [rdx+512], ymm30	 # AVX512{F,VL}
	vpmovqb	DWORD PTR [rdx-512], ymm30	 # AVX512{F,VL} Disp8
	vpmovqb	DWORD PTR [rdx-516], ymm30	 # AVX512{F,VL}
	vpmovsqb	WORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpmovsqb	WORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpmovsqb	WORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpmovsqb	WORD PTR [rdx+254], xmm30	 # AVX512{F,VL} Disp8
	vpmovsqb	WORD PTR [rdx+256], xmm30	 # AVX512{F,VL}
	vpmovsqb	WORD PTR [rdx-256], xmm30	 # AVX512{F,VL} Disp8
	vpmovsqb	WORD PTR [rdx-258], xmm30	 # AVX512{F,VL}
	vpmovsqb	DWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpmovsqb	DWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpmovsqb	DWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpmovsqb	DWORD PTR [rdx+508], ymm30	 # AVX512{F,VL} Disp8
	vpmovsqb	DWORD PTR [rdx+512], ymm30	 # AVX512{F,VL}
	vpmovsqb	DWORD PTR [rdx-512], ymm30	 # AVX512{F,VL} Disp8
	vpmovsqb	DWORD PTR [rdx-516], ymm30	 # AVX512{F,VL}
	vpmovusqb	WORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpmovusqb	WORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpmovusqb	WORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpmovusqb	WORD PTR [rdx+254], xmm30	 # AVX512{F,VL} Disp8
	vpmovusqb	WORD PTR [rdx+256], xmm30	 # AVX512{F,VL}
	vpmovusqb	WORD PTR [rdx-256], xmm30	 # AVX512{F,VL} Disp8
	vpmovusqb	WORD PTR [rdx-258], xmm30	 # AVX512{F,VL}
	vpmovusqb	DWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpmovusqb	DWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpmovusqb	DWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpmovusqb	DWORD PTR [rdx+508], ymm30	 # AVX512{F,VL} Disp8
	vpmovusqb	DWORD PTR [rdx+512], ymm30	 # AVX512{F,VL}
	vpmovusqb	DWORD PTR [rdx-512], ymm30	 # AVX512{F,VL} Disp8
	vpmovusqb	DWORD PTR [rdx-516], ymm30	 # AVX512{F,VL}
	vpmovqw	DWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpmovqw	DWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpmovqw	DWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpmovqw	DWORD PTR [rdx+508], xmm30	 # AVX512{F,VL} Disp8
	vpmovqw	DWORD PTR [rdx+512], xmm30	 # AVX512{F,VL}
	vpmovqw	DWORD PTR [rdx-512], xmm30	 # AVX512{F,VL} Disp8
	vpmovqw	DWORD PTR [rdx-516], xmm30	 # AVX512{F,VL}
	vpmovqw	QWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpmovqw	QWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpmovqw	QWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpmovqw	QWORD PTR [rdx+1016], ymm30	 # AVX512{F,VL} Disp8
	vpmovqw	QWORD PTR [rdx+1024], ymm30	 # AVX512{F,VL}
	vpmovqw	QWORD PTR [rdx-1024], ymm30	 # AVX512{F,VL} Disp8
	vpmovqw	QWORD PTR [rdx-1032], ymm30	 # AVX512{F,VL}
	vpmovsqw	DWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpmovsqw	DWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpmovsqw	DWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpmovsqw	DWORD PTR [rdx+508], xmm30	 # AVX512{F,VL} Disp8
	vpmovsqw	DWORD PTR [rdx+512], xmm30	 # AVX512{F,VL}
	vpmovsqw	DWORD PTR [rdx-512], xmm30	 # AVX512{F,VL} Disp8
	vpmovsqw	DWORD PTR [rdx-516], xmm30	 # AVX512{F,VL}
	vpmovsqw	QWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpmovsqw	QWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpmovsqw	QWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpmovsqw	QWORD PTR [rdx+1016], ymm30	 # AVX512{F,VL} Disp8
	vpmovsqw	QWORD PTR [rdx+1024], ymm30	 # AVX512{F,VL}
	vpmovsqw	QWORD PTR [rdx-1024], ymm30	 # AVX512{F,VL} Disp8
	vpmovsqw	QWORD PTR [rdx-1032], ymm30	 # AVX512{F,VL}
	vpmovusqw	DWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpmovusqw	DWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpmovusqw	DWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpmovusqw	DWORD PTR [rdx+508], xmm30	 # AVX512{F,VL} Disp8
	vpmovusqw	DWORD PTR [rdx+512], xmm30	 # AVX512{F,VL}
	vpmovusqw	DWORD PTR [rdx-512], xmm30	 # AVX512{F,VL} Disp8
	vpmovusqw	DWORD PTR [rdx-516], xmm30	 # AVX512{F,VL}
	vpmovusqw	QWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpmovusqw	QWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpmovusqw	QWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpmovusqw	QWORD PTR [rdx+1016], ymm30	 # AVX512{F,VL} Disp8
	vpmovusqw	QWORD PTR [rdx+1024], ymm30	 # AVX512{F,VL}
	vpmovusqw	QWORD PTR [rdx-1024], ymm30	 # AVX512{F,VL} Disp8
	vpmovusqw	QWORD PTR [rdx-1032], ymm30	 # AVX512{F,VL}
	vpmovqd	QWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpmovqd	QWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpmovqd	QWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpmovqd	QWORD PTR [rdx+1016], xmm30	 # AVX512{F,VL} Disp8
	vpmovqd	QWORD PTR [rdx+1024], xmm30	 # AVX512{F,VL}
	vpmovqd	QWORD PTR [rdx-1024], xmm30	 # AVX512{F,VL} Disp8
	vpmovqd	QWORD PTR [rdx-1032], xmm30	 # AVX512{F,VL}
	vpmovqd	XMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpmovqd	XMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpmovqd	XMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpmovqd	XMMWORD PTR [rdx+2032], ymm30	 # AVX512{F,VL} Disp8
	vpmovqd	XMMWORD PTR [rdx+2048], ymm30	 # AVX512{F,VL}
	vpmovqd	XMMWORD PTR [rdx-2048], ymm30	 # AVX512{F,VL} Disp8
	vpmovqd	XMMWORD PTR [rdx-2064], ymm30	 # AVX512{F,VL}
	vpmovsqd	QWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpmovsqd	QWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpmovsqd	QWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpmovsqd	QWORD PTR [rdx+1016], xmm30	 # AVX512{F,VL} Disp8
	vpmovsqd	QWORD PTR [rdx+1024], xmm30	 # AVX512{F,VL}
	vpmovsqd	QWORD PTR [rdx-1024], xmm30	 # AVX512{F,VL} Disp8
	vpmovsqd	QWORD PTR [rdx-1032], xmm30	 # AVX512{F,VL}
	vpmovsqd	XMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpmovsqd	XMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpmovsqd	XMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpmovsqd	XMMWORD PTR [rdx+2032], ymm30	 # AVX512{F,VL} Disp8
	vpmovsqd	XMMWORD PTR [rdx+2048], ymm30	 # AVX512{F,VL}
	vpmovsqd	XMMWORD PTR [rdx-2048], ymm30	 # AVX512{F,VL} Disp8
	vpmovsqd	XMMWORD PTR [rdx-2064], ymm30	 # AVX512{F,VL}
	vpmovusqd	QWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpmovusqd	QWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpmovusqd	QWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpmovusqd	QWORD PTR [rdx+1016], xmm30	 # AVX512{F,VL} Disp8
	vpmovusqd	QWORD PTR [rdx+1024], xmm30	 # AVX512{F,VL}
	vpmovusqd	QWORD PTR [rdx-1024], xmm30	 # AVX512{F,VL} Disp8
	vpmovusqd	QWORD PTR [rdx-1032], xmm30	 # AVX512{F,VL}
	vpmovusqd	XMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpmovusqd	XMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpmovusqd	XMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpmovusqd	XMMWORD PTR [rdx+2032], ymm30	 # AVX512{F,VL} Disp8
	vpmovusqd	XMMWORD PTR [rdx+2048], ymm30	 # AVX512{F,VL}
	vpmovusqd	XMMWORD PTR [rdx-2048], ymm30	 # AVX512{F,VL} Disp8
	vpmovusqd	XMMWORD PTR [rdx-2064], ymm30	 # AVX512{F,VL}
	vpmovdb	DWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpmovdb	DWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpmovdb	DWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpmovdb	DWORD PTR [rdx+508], xmm30	 # AVX512{F,VL} Disp8
	vpmovdb	DWORD PTR [rdx+512], xmm30	 # AVX512{F,VL}
	vpmovdb	DWORD PTR [rdx-512], xmm30	 # AVX512{F,VL} Disp8
	vpmovdb	DWORD PTR [rdx-516], xmm30	 # AVX512{F,VL}
	vpmovdb	QWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpmovdb	QWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpmovdb	QWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpmovdb	QWORD PTR [rdx+1016], ymm30	 # AVX512{F,VL} Disp8
	vpmovdb	QWORD PTR [rdx+1024], ymm30	 # AVX512{F,VL}
	vpmovdb	QWORD PTR [rdx-1024], ymm30	 # AVX512{F,VL} Disp8
	vpmovdb	QWORD PTR [rdx-1032], ymm30	 # AVX512{F,VL}
	vpmovsdb	DWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpmovsdb	DWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpmovsdb	DWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpmovsdb	DWORD PTR [rdx+508], xmm30	 # AVX512{F,VL} Disp8
	vpmovsdb	DWORD PTR [rdx+512], xmm30	 # AVX512{F,VL}
	vpmovsdb	DWORD PTR [rdx-512], xmm30	 # AVX512{F,VL} Disp8
	vpmovsdb	DWORD PTR [rdx-516], xmm30	 # AVX512{F,VL}
	vpmovsdb	QWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpmovsdb	QWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpmovsdb	QWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpmovsdb	QWORD PTR [rdx+1016], ymm30	 # AVX512{F,VL} Disp8
	vpmovsdb	QWORD PTR [rdx+1024], ymm30	 # AVX512{F,VL}
	vpmovsdb	QWORD PTR [rdx-1024], ymm30	 # AVX512{F,VL} Disp8
	vpmovsdb	QWORD PTR [rdx-1032], ymm30	 # AVX512{F,VL}
	vpmovusdb	DWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpmovusdb	DWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpmovusdb	DWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpmovusdb	DWORD PTR [rdx+508], xmm30	 # AVX512{F,VL} Disp8
	vpmovusdb	DWORD PTR [rdx+512], xmm30	 # AVX512{F,VL}
	vpmovusdb	DWORD PTR [rdx-512], xmm30	 # AVX512{F,VL} Disp8
	vpmovusdb	DWORD PTR [rdx-516], xmm30	 # AVX512{F,VL}
	vpmovusdb	QWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpmovusdb	QWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpmovusdb	QWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpmovusdb	QWORD PTR [rdx+1016], ymm30	 # AVX512{F,VL} Disp8
	vpmovusdb	QWORD PTR [rdx+1024], ymm30	 # AVX512{F,VL}
	vpmovusdb	QWORD PTR [rdx-1024], ymm30	 # AVX512{F,VL} Disp8
	vpmovusdb	QWORD PTR [rdx-1032], ymm30	 # AVX512{F,VL}
	vpmovdw	QWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpmovdw	QWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpmovdw	QWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpmovdw	QWORD PTR [rdx+1016], xmm30	 # AVX512{F,VL} Disp8
	vpmovdw	QWORD PTR [rdx+1024], xmm30	 # AVX512{F,VL}
	vpmovdw	QWORD PTR [rdx-1024], xmm30	 # AVX512{F,VL} Disp8
	vpmovdw	QWORD PTR [rdx-1032], xmm30	 # AVX512{F,VL}
	vpmovdw	XMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpmovdw	XMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpmovdw	XMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpmovdw	XMMWORD PTR [rdx+2032], ymm30	 # AVX512{F,VL} Disp8
	vpmovdw	XMMWORD PTR [rdx+2048], ymm30	 # AVX512{F,VL}
	vpmovdw	XMMWORD PTR [rdx-2048], ymm30	 # AVX512{F,VL} Disp8
	vpmovdw	XMMWORD PTR [rdx-2064], ymm30	 # AVX512{F,VL}
	vpmovsdw	QWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpmovsdw	QWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpmovsdw	QWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpmovsdw	QWORD PTR [rdx+1016], xmm30	 # AVX512{F,VL} Disp8
	vpmovsdw	QWORD PTR [rdx+1024], xmm30	 # AVX512{F,VL}
	vpmovsdw	QWORD PTR [rdx-1024], xmm30	 # AVX512{F,VL} Disp8
	vpmovsdw	QWORD PTR [rdx-1032], xmm30	 # AVX512{F,VL}
	vpmovsdw	XMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpmovsdw	XMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpmovsdw	XMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpmovsdw	XMMWORD PTR [rdx+2032], ymm30	 # AVX512{F,VL} Disp8
	vpmovsdw	XMMWORD PTR [rdx+2048], ymm30	 # AVX512{F,VL}
	vpmovsdw	XMMWORD PTR [rdx-2048], ymm30	 # AVX512{F,VL} Disp8
	vpmovsdw	XMMWORD PTR [rdx-2064], ymm30	 # AVX512{F,VL}
	vpmovusdw	QWORD PTR [rcx], xmm30	 # AVX512{F,VL}
	vpmovusdw	QWORD PTR [rcx]{k7}, xmm30	 # AVX512{F,VL}
	vpmovusdw	QWORD PTR [rax+r14*8+0x1234], xmm30	 # AVX512{F,VL}
	vpmovusdw	QWORD PTR [rdx+1016], xmm30	 # AVX512{F,VL} Disp8
	vpmovusdw	QWORD PTR [rdx+1024], xmm30	 # AVX512{F,VL}
	vpmovusdw	QWORD PTR [rdx-1024], xmm30	 # AVX512{F,VL} Disp8
	vpmovusdw	QWORD PTR [rdx-1032], xmm30	 # AVX512{F,VL}
	vpmovusdw	XMMWORD PTR [rcx], ymm30	 # AVX512{F,VL}
	vpmovusdw	XMMWORD PTR [rcx]{k7}, ymm30	 # AVX512{F,VL}
	vpmovusdw	XMMWORD PTR [rax+r14*8+0x1234], ymm30	 # AVX512{F,VL}
	vpmovusdw	XMMWORD PTR [rdx+2032], ymm30	 # AVX512{F,VL} Disp8
	vpmovusdw	XMMWORD PTR [rdx+2048], ymm30	 # AVX512{F,VL}
	vpmovusdw	XMMWORD PTR [rdx-2048], ymm30	 # AVX512{F,VL} Disp8
	vpmovusdw	XMMWORD PTR [rdx-2064], ymm30	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, xmm29	 # AVX512{F,VL}
	vcvttpd2udq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcvttpd2udq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, [rcx]{1to2}	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, QWORD BCST [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm30, QWORD BCST [rdx+1024]{1to2}	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, QWORD BCST [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm30, QWORD BCST [rdx-1032]{1to2}	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, ymm29	 # AVX512{F,VL}
	vcvttpd2udq	xmm30{k7}, ymm29	 # AVX512{F,VL}
	vcvttpd2udq	xmm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, QWORD BCST [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm30, QWORD BCST [rdx+1024]{1to4}	 # AVX512{F,VL}
	vcvttpd2udq	xmm30, QWORD BCST [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm30, QWORD BCST [rdx-1032]{1to4}	 # AVX512{F,VL}
	vcvttps2udq	xmm30, xmm29	 # AVX512{F,VL}
	vcvttps2udq	xmm30{k7}, xmm29	 # AVX512{F,VL}
	vcvttps2udq	xmm30{k7}{z}, xmm29	 # AVX512{F,VL}
	vcvttps2udq	xmm30, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvttps2udq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvttps2udq	xmm30, [rcx]{1to4}	 # AVX512{F,VL}
	vcvttps2udq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vcvttps2udq	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vcvttps2udq	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vcvttps2udq	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vcvttps2udq	xmm30, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvttps2udq	xmm30, [rdx+512]{1to4}	 # AVX512{F,VL}
	vcvttps2udq	xmm30, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvttps2udq	xmm30, [rdx-516]{1to4}	 # AVX512{F,VL}
	vcvttps2udq	ymm30, ymm29	 # AVX512{F,VL}
	vcvttps2udq	ymm30{k7}, ymm29	 # AVX512{F,VL}
	vcvttps2udq	ymm30{k7}{z}, ymm29	 # AVX512{F,VL}
	vcvttps2udq	ymm30, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vcvttps2udq	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vcvttps2udq	ymm30, [rcx]{1to8}	 # AVX512{F,VL}
	vcvttps2udq	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vcvttps2udq	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vcvttps2udq	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vcvttps2udq	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vcvttps2udq	ymm30, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vcvttps2udq	ymm30, [rdx+512]{1to8}	 # AVX512{F,VL}
	vcvttps2udq	ymm30, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vcvttps2udq	ymm30, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpermi2d	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpermi2d	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpermi2d	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpermi2d	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermi2d	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermi2d	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpermi2d	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpermi2d	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpermi2d	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpermi2d	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpermi2d	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2d	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpermi2d	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2d	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpermi2d	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpermi2d	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpermi2d	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpermi2d	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermi2d	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermi2d	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpermi2d	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpermi2d	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpermi2d	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpermi2d	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpermi2d	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpermi2d	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpermi2d	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpermi2d	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpermi2q	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpermi2q	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpermi2q	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpermi2q	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermi2q	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermi2q	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpermi2q	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpermi2q	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpermi2q	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpermi2q	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpermi2q	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpermi2q	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpermi2q	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpermi2q	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpermi2q	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpermi2q	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpermi2q	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpermi2q	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermi2q	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermi2q	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpermi2q	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpermi2q	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpermi2q	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpermi2q	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpermi2q	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2q	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpermi2q	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2q	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vpermi2ps	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpermi2ps	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpermi2ps	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpermi2ps	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermi2ps	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermi2ps	xmm30, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpermi2ps	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpermi2ps	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpermi2ps	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpermi2ps	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpermi2ps	xmm30, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2ps	xmm30, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vpermi2ps	xmm30, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2ps	xmm30, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vpermi2ps	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpermi2ps	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpermi2ps	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpermi2ps	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermi2ps	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermi2ps	ymm30, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vpermi2ps	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpermi2ps	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpermi2ps	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpermi2ps	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpermi2ps	ymm30, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpermi2ps	ymm30, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vpermi2ps	ymm30, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpermi2ps	ymm30, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vpermi2pd	xmm30, xmm29, xmm28	 # AVX512{F,VL}
	vpermi2pd	xmm30{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vpermi2pd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{F,VL}
	vpermi2pd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermi2pd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermi2pd	xmm30, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vpermi2pd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vpermi2pd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vpermi2pd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vpermi2pd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vpermi2pd	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpermi2pd	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vpermi2pd	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpermi2pd	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vpermi2pd	ymm30, ymm29, ymm28	 # AVX512{F,VL}
	vpermi2pd	ymm30{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vpermi2pd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{F,VL}
	vpermi2pd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vpermi2pd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vpermi2pd	ymm30, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vpermi2pd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vpermi2pd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vpermi2pd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vpermi2pd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vpermi2pd	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2pd	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vpermi2pd	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2pd	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
	vptestnmd	k5, xmm29, xmm28	 # AVX512{F,VL}
	vptestnmd	k5{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vptestnmd	k5, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vptestnmd	k5, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vptestnmd	k5, xmm29, [rcx]{1to4}	 # AVX512{F,VL}
	vptestnmd	k5, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vptestnmd	k5, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vptestnmd	k5, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vptestnmd	k5, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vptestnmd	k5, xmm29, [rdx+508]{1to4}	 # AVX512{F,VL} Disp8
	vptestnmd	k5, xmm29, [rdx+512]{1to4}	 # AVX512{F,VL}
	vptestnmd	k5, xmm29, [rdx-512]{1to4}	 # AVX512{F,VL} Disp8
	vptestnmd	k5, xmm29, [rdx-516]{1to4}	 # AVX512{F,VL}
	vptestnmd	k5, ymm29, ymm28	 # AVX512{F,VL}
	vptestnmd	k5{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vptestnmd	k5, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vptestnmd	k5, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vptestnmd	k5, ymm29, [rcx]{1to8}	 # AVX512{F,VL}
	vptestnmd	k5, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vptestnmd	k5, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vptestnmd	k5, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vptestnmd	k5, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vptestnmd	k5, ymm29, [rdx+508]{1to8}	 # AVX512{F,VL} Disp8
	vptestnmd	k5, ymm29, [rdx+512]{1to8}	 # AVX512{F,VL}
	vptestnmd	k5, ymm29, [rdx-512]{1to8}	 # AVX512{F,VL} Disp8
	vptestnmd	k5, ymm29, [rdx-516]{1to8}	 # AVX512{F,VL}
	vptestnmq	k5, xmm29, xmm28	 # AVX512{F,VL}
	vptestnmq	k5{k7}, xmm29, xmm28	 # AVX512{F,VL}
	vptestnmq	k5, xmm29, XMMWORD PTR [rcx]	 # AVX512{F,VL}
	vptestnmq	k5, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vptestnmq	k5, xmm29, [rcx]{1to2}	 # AVX512{F,VL}
	vptestnmq	k5, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{F,VL} Disp8
	vptestnmq	k5, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{F,VL}
	vptestnmq	k5, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{F,VL} Disp8
	vptestnmq	k5, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{F,VL}
	vptestnmq	k5, xmm29, [rdx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vptestnmq	k5, xmm29, [rdx+1024]{1to2}	 # AVX512{F,VL}
	vptestnmq	k5, xmm29, [rdx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vptestnmq	k5, xmm29, [rdx-1032]{1to2}	 # AVX512{F,VL}
	vptestnmq	k5, ymm29, ymm28	 # AVX512{F,VL}
	vptestnmq	k5{k7}, ymm29, ymm28	 # AVX512{F,VL}
	vptestnmq	k5, ymm29, YMMWORD PTR [rcx]	 # AVX512{F,VL}
	vptestnmq	k5, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{F,VL}
	vptestnmq	k5, ymm29, [rcx]{1to4}	 # AVX512{F,VL}
	vptestnmq	k5, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{F,VL} Disp8
	vptestnmq	k5, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{F,VL}
	vptestnmq	k5, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{F,VL} Disp8
	vptestnmq	k5, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{F,VL}
	vptestnmq	k5, ymm29, [rdx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vptestnmq	k5, ymm29, [rdx+1024]{1to4}	 # AVX512{F,VL}
	vptestnmq	k5, ymm29, [rdx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vptestnmq	k5, ymm29, [rdx-1032]{1to4}	 # AVX512{F,VL}
