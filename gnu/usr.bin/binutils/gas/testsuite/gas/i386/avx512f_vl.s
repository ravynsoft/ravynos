# Check 32bit AVX512{F,VL} instructions

	.allow_index_reg
	.text
_start:
	vaddpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddpd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vaddpd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddpd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddpd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddpd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vaddpd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddpd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vaddpd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddpd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vaddpd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddpd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vaddpd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vaddpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vaddpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vaddpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vaddpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vaddpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vaddpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vaddpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vaddpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vaddpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vaddpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vaddpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vaddpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vaddps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vaddps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vaddps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vaddps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vaddps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vaddps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vaddps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vaddps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vaddps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vaddps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vaddps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vaddps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vaddps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vaddps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vaddps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vaddps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vaddps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vaddps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vaddps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignd	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignd	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	valignd	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignd	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignd	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignd	$123, (%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignd	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	valignd	$123, 2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignd	$123, -2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	valignd	$123, -2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignd	$123, 508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	valignd	$123, 512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignd	$123, -512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	valignd	$123, -516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignd	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignd	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	valignd	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignd	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignd	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignd	$123, (%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignd	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	valignd	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignd	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	valignd	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignd	$123, 508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	valignd	$123, 512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignd	$123, -512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	valignd	$123, -516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmpd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vblendmpd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmpd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmpd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmpd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmpd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmpd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmpd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmpd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmpd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmpd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmpd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vblendmpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vblendmps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vblendmps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vblendmps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vblendmps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vblendmps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcastf32x4	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcastf32x4	(%ecx), %ymm6{%k7}{z}	 # AVX512{F,VL}
	vbroadcastf32x4	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcastf32x4	2032(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vbroadcastf32x4	2048(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcastf32x4	-2048(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vbroadcastf32x4	-2064(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcasti32x4	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcasti32x4	(%ecx), %ymm6{%k7}{z}	 # AVX512{F,VL}
	vbroadcasti32x4	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcasti32x4	2032(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vbroadcasti32x4	2048(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcasti32x4	-2048(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vbroadcasti32x4	-2064(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcastsd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcastsd	(%ecx), %ymm6{%k7}{z}	 # AVX512{F,VL}
	vbroadcastsd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcastsd	1016(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vbroadcastsd	1024(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcastsd	-1024(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vbroadcastsd	-1032(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcastsd	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcastsd	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vbroadcastss	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vbroadcastss	(%ecx), %xmm6{%k7}{z}	 # AVX512{F,VL}
	vbroadcastss	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vbroadcastss	508(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vbroadcastss	512(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vbroadcastss	-512(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vbroadcastss	-516(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vbroadcastss	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcastss	(%ecx), %ymm6{%k7}{z}	 # AVX512{F,VL}
	vbroadcastss	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcastss	508(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vbroadcastss	512(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcastss	-512(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vbroadcastss	-516(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcastss	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vbroadcastss	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vbroadcastss	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vbroadcastss	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vcmppd	$0xab, %xmm4, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, %xmm4, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, (%ecx), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, -123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, (%eax){1to2}, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, 2032(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmppd	$123, 2048(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, -2048(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmppd	$123, -2064(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, 1016(%edx){1to2}, %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmppd	$123, 1024(%edx){1to2}, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, -1024(%edx){1to2}, %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmppd	$123, -1032(%edx){1to2}, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$0xab, %ymm4, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, %ymm4, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, (%ecx), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, -123456(%esp,%esi,8), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, (%eax){1to4}, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, 4064(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmppd	$123, 4096(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, -4096(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmppd	$123, -4128(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, 1016(%edx){1to4}, %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmppd	$123, 1024(%edx){1to4}, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmppd	$123, -1024(%edx){1to4}, %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmppd	$123, -1032(%edx){1to4}, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$0xab, %xmm4, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, %xmm4, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, (%ecx), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, -123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, (%eax){1to4}, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, 2032(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmpps	$123, 2048(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, -2048(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmpps	$123, -2064(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, 508(%edx){1to4}, %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmpps	$123, 512(%edx){1to4}, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, -512(%edx){1to4}, %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmpps	$123, -516(%edx){1to4}, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$0xab, %ymm4, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, %ymm4, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, (%ecx), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, -123456(%esp,%esi,8), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, (%eax){1to8}, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, 4064(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmpps	$123, 4096(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, -4096(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmpps	$123, -4128(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, 508(%edx){1to8}, %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmpps	$123, 512(%edx){1to8}, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcmpps	$123, -512(%edx){1to8}, %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vcmpps	$123, -516(%edx){1to8}, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vcompresspd	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vcompresspd	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vcompresspd	%xmm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vcompresspd	%xmm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vcompresspd	%xmm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vcompresspd	%xmm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vcompresspd	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vcompresspd	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vcompresspd	%ymm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vcompresspd	%ymm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vcompresspd	%ymm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vcompresspd	%ymm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vcompresspd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcompresspd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcompresspd	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vcompresspd	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vcompressps	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vcompressps	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vcompressps	%xmm6, 508(%edx){%k7}	 # AVX512{F,VL} Disp8
	vcompressps	%xmm6, 512(%edx){%k7}	 # AVX512{F,VL}
	vcompressps	%xmm6, -512(%edx){%k7}	 # AVX512{F,VL} Disp8
	vcompressps	%xmm6, -516(%edx){%k7}	 # AVX512{F,VL}
	vcompressps	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vcompressps	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vcompressps	%ymm6, 508(%edx){%k7}	 # AVX512{F,VL} Disp8
	vcompressps	%ymm6, 512(%edx){%k7}	 # AVX512{F,VL}
	vcompressps	%ymm6, -512(%edx){%k7}	 # AVX512{F,VL} Disp8
	vcompressps	%ymm6, -516(%edx){%k7}	 # AVX512{F,VL}
	vcompressps	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcompressps	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcompressps	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vcompressps	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vcvtdq2pd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtdq2pd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	(%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	1016(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	1024(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	-1024(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	-1032(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	508(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	512(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	-512(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	-516(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vcvtdq2pd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	(%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	2032(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	2048(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	-2048(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	-2064(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	508(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	512(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtdq2pd	-512(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	-516(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtdq2ps	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	-512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	-516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vcvtdq2ps	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	(%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtdq2ps	-512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	-516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtpd2dqx	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dqx	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dq	(%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dqx	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2dqx	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dqx	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2dqx	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dqx	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2dqx	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dqx	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2dqx	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dq	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dq	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtpd2dqy	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dqy	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dq	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dqy	4064(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2dqy	4096(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dqy	-4096(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2dqy	-4128(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dqy	1016(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2dqy	1024(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2dqy	-1024(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2dqy	-1032(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2ps	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2ps	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtpd2psx	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2psx	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2ps	(%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2psx	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2psx	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2psx	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2psx	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2psx	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2psx	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2psx	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2psx	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2ps	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2ps	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtpd2psy	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2psy	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2ps	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2psy	4064(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2psy	4096(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2psy	-4096(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2psy	-4128(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2psy	1016(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2psy	1024(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2psy	-1024(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2psy	-1032(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtpd2udqx	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udqx	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udq	(%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udqx	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2udqx	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udqx	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2udqx	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udqx	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2udqx	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udqx	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2udqx	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udq	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udq	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtpd2udqy	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udqy	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udq	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udqy	4064(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2udqy	4096(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udqy	-4096(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2udqy	-4128(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udqy	1016(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2udqy	1024(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtpd2udqy	-1024(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtpd2udqy	-1032(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtph2ps	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtph2ps	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtph2ps	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtph2ps	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtph2ps	1016(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtph2ps	1024(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtph2ps	-1024(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtph2ps	-1032(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtph2ps	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtph2ps	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vcvtph2ps	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtph2ps	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtph2ps	2032(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtph2ps	2048(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtph2ps	-2048(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtph2ps	-2064(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtps2dq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2dq	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2dq	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2dq	512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	-512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2dq	-516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vcvtps2dq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	(%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2dq	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2dq	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2dq	512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2dq	-512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2dq	-516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtps2pd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	(%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	1016(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2pd	1024(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	-1024(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2pd	-1032(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	508(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2pd	512(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	-512(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2pd	-516(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vcvtps2pd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	(%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	2032(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2pd	2048(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	-2048(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2pd	-2064(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	508(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2pd	512(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2pd	-512(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2pd	-516(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtps2ph	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtps2ph	$123, %ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtps2udq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2udq	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2udq	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2udq	512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	-512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2udq	-516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vcvtps2udq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	(%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2udq	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2udq	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2udq	512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtps2udq	-512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtps2udq	-516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvttpd2dqx	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dqx	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dq	(%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dqx	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2dqx	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dqx	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2dqx	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dqx	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2dqx	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dqx	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2dqx	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dq	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dq	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvttpd2dqy	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dqy	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dq	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dqy	4064(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2dqy	4096(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dqy	-4096(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2dqy	-4128(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dqy	1016(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2dqy	1024(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2dqy	-1024(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2dqy	-1032(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvttps2dq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2dq	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2dq	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2dq	512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	-512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2dq	-516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vcvttps2dq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	(%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2dq	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2dq	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2dq	512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvttps2dq	-512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2dq	-516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtudq2pd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	(%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	1016(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	1024(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	-1024(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	-1032(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	508(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	512(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	-512(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	-516(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vcvtudq2pd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	(%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	2032(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	2048(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	-2048(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	-2064(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	508(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	512(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2pd	-512(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	-516(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvtudq2ps	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	-512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	-516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vcvtudq2ps	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	(%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvtudq2ps	-512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	-516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vdivpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivpd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vdivpd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivpd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivpd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivpd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vdivpd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivpd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vdivpd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivpd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vdivpd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivpd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vdivpd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vdivpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vdivpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vdivpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vdivpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vdivpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vdivpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vdivpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vdivpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vdivpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vdivpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vdivpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vdivpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vdivps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vdivps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vdivps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vdivps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vdivps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vdivps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vdivps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vdivps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vdivps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vdivps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vdivps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vdivps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vdivps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vdivps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vdivps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vdivps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vdivps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vdivps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vdivps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vexpandpd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vexpandpd	(%ecx), %xmm6{%k7}{z}	 # AVX512{F,VL}
	vexpandpd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vexpandpd	1016(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vexpandpd	1024(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vexpandpd	-1024(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vexpandpd	-1032(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vexpandpd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vexpandpd	(%ecx), %ymm6{%k7}{z}	 # AVX512{F,VL}
	vexpandpd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vexpandpd	1016(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vexpandpd	1024(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vexpandpd	-1024(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vexpandpd	-1032(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vexpandpd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vexpandpd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vexpandpd	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vexpandpd	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vexpandps	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vexpandps	(%ecx), %xmm6{%k7}{z}	 # AVX512{F,VL}
	vexpandps	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vexpandps	508(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vexpandps	512(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vexpandps	-512(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vexpandps	-516(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vexpandps	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vexpandps	(%ecx), %ymm6{%k7}{z}	 # AVX512{F,VL}
	vexpandps	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vexpandps	508(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vexpandps	512(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vexpandps	-512(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vexpandps	-516(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vexpandps	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vexpandps	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vexpandps	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vexpandps	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vextractf32x4	$0xab, %ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vextractf32x4	$0xab, %ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vextractf32x4	$123, %ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vextracti32x4	$0xab, %ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vextracti32x4	$0xab, %ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vextracti32x4	$123, %ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmadd132pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmadd132pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd132pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmadd132ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmadd132ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd132ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd132ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmadd213pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmadd213pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmadd213ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmadd213ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd213ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd213ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmadd231pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmadd231pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmadd231ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmadd231ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmadd231ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmadd231ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub132pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub132pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub132ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub132ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub132ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub213pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub213pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub213ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub213ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub213ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub231pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub231pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub231ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmaddsub231ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmaddsub231ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmsub132pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmsub132pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmsub132ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmsub132ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub132ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub132ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmsub213pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmsub213pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmsub213ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmsub213ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub213ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub213ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmsub231pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmsub231pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmsub231ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmsub231ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsub231ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsub231ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd132pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd132pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd132ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd132ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd132ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd213pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd213pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd213ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd213ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd213ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd231pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd231pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd231ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfmsubadd231ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfmsubadd231ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfnmadd132pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfnmadd132pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfnmadd132ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfnmadd132ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd132ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfnmadd213pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfnmadd213pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfnmadd213ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfnmadd213ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd213ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfnmadd231pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfnmadd231pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfnmadd231ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfnmadd231ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmadd231ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfnmsub132pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfnmsub132pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfnmsub132ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfnmsub132ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub132ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfnmsub213pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfnmsub213pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfnmsub213ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfnmsub213ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub213ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfnmsub231pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfnmsub231pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfnmsub231ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfnmsub231ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfnmsub231ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vgatherdpd	123(%ebp,%xmm7,8), %xmm6{%k1}	 # AVX512{F,VL}
	vgatherdpd	256(%eax,%xmm7), %xmm6{%k1}	 # AVX512{F,VL}
	vgatherdpd	1024(%ecx,%xmm7,4), %xmm6{%k1}	 # AVX512{F,VL}
	vgatherdpd	123(%ebp,%xmm7,8), %ymm6{%k1}	 # AVX512{F,VL}
	vgatherdpd	256(%eax,%xmm7), %ymm6{%k1}	 # AVX512{F,VL}
	vgatherdpd	1024(%ecx,%xmm7,4), %ymm6{%k1}	 # AVX512{F,VL}
	vgatherdps	123(%ebp,%xmm7,8), %xmm6{%k1}	 # AVX512{F,VL}
	vgatherdps	256(%eax,%xmm7), %xmm6{%k1}	 # AVX512{F,VL}
	vgatherdps	1024(%ecx,%xmm7,4), %xmm6{%k1}	 # AVX512{F,VL}
	vgatherdps	123(%ebp,%ymm7,8), %ymm6{%k1}	 # AVX512{F,VL}
	vgatherdps	256(%eax,%ymm7), %ymm6{%k1}	 # AVX512{F,VL}
	vgatherdps	1024(%ecx,%ymm7,4), %ymm6{%k1}	 # AVX512{F,VL}
	vgatherqpd	123(%ebp,%xmm7,8), %xmm6{%k1}	 # AVX512{F,VL}
	vgatherqpd	256(%eax,%xmm7), %xmm6{%k1}	 # AVX512{F,VL}
	vgatherqpd	1024(%ecx,%xmm7,4), %xmm6{%k1}	 # AVX512{F,VL}
	vgatherqpd	123(%ebp,%ymm7,8), %ymm6{%k1}	 # AVX512{F,VL}
	vgatherqpd	256(%eax,%ymm7), %ymm6{%k1}	 # AVX512{F,VL}
	vgatherqpd	1024(%ecx,%ymm7,4), %ymm6{%k1}	 # AVX512{F,VL}
	vgatherqps	123(%ebp,%xmm7,8), %xmm6{%k1}	 # AVX512{F,VL}
	vgatherqps	256(%eax,%xmm7), %xmm6{%k1}	 # AVX512{F,VL}
	vgatherqps	1024(%ecx,%xmm7,4), %xmm6{%k1}	 # AVX512{F,VL}
	vgatherqps	123(%ebp,%ymm7,8), %xmm6{%k1}	 # AVX512{F,VL}
	vgatherqps	256(%eax,%ymm7), %xmm6{%k1}	 # AVX512{F,VL}
	vgatherqps	1024(%ecx,%ymm7,4), %xmm6{%k1}	 # AVX512{F,VL}
	vgetexppd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vgetexppd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vgetexppd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vgetexppd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vgetexppd	(%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vgetexppd	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexppd	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vgetexppd	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexppd	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vgetexppd	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexppd	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vgetexppd	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexppd	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vgetexppd	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vgetexppd	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vgetexppd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vgetexppd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vgetexppd	(%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vgetexppd	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexppd	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vgetexppd	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexppd	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vgetexppd	1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexppd	1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vgetexppd	-1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexppd	-1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vgetexpps	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vgetexpps	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vgetexpps	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vgetexpps	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vgetexpps	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vgetexpps	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexpps	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vgetexpps	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexpps	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vgetexpps	508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexpps	512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vgetexpps	-512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexpps	-516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vgetexpps	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vgetexpps	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vgetexpps	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vgetexpps	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vgetexpps	(%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vgetexpps	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexpps	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vgetexpps	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexpps	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vgetexpps	508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexpps	512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vgetexpps	-512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetexpps	-516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vgetmantpd	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$123, (%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$123, 1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, 1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$123, -1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, -1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vgetmantpd	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$123, (%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$123, 1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, 1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantpd	$123, -1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantpd	$123, -1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vgetmantps	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$123, (%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantps	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantps	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$123, 508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantps	$123, 512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$123, -512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantps	$123, -516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vgetmantps	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$123, (%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantps	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantps	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$123, 508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantps	$123, 512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vgetmantps	$123, -512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vgetmantps	$123, -516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vinsertf32x4	$0xab, %xmm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vinsertf32x4	$0xab, %xmm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vinsertf32x4	$123, %xmm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vinsertf32x4	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vinsertf32x4	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vinsertf32x4	$123, 2032(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vinsertf32x4	$123, 2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vinsertf32x4	$123, -2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vinsertf32x4	$123, -2064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vinserti32x4	$0xab, %xmm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vinserti32x4	$0xab, %xmm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vinserti32x4	$123, %xmm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vinserti32x4	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vinserti32x4	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vinserti32x4	$123, 2032(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vinserti32x4	$123, 2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vinserti32x4	$123, -2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vinserti32x4	$123, -2064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxpd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vmaxpd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxpd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxpd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxpd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxpd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxpd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxpd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxpd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxpd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxpd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxpd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vmaxpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vmaxps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmaxps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vmaxps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmaxps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmaxps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminpd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vminpd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminpd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminpd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminpd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vminpd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminpd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vminpd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminpd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vminpd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminpd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vminpd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vminpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vminpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vminpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vminpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vminpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vminps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vminps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vminps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vminps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vminps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vminps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vminps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vminps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vminps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vminps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vminps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vminps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmovapd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmovapd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vmovapd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovapd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vmovapd	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovapd	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovapd	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovapd	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovapd	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmovapd	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vmovapd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovapd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vmovapd	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovapd	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovapd	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovapd	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovaps	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmovaps	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vmovaps	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovaps	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vmovaps	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovaps	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovaps	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovaps	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovaps	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmovaps	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vmovaps	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovaps	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vmovaps	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovaps	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovaps	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovaps	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovddup	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmovddup	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vmovddup	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovddup	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vmovddup	1016(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovddup	1024(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovddup	-1024(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovddup	-1032(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovddup	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmovddup	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vmovddup	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovddup	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vmovddup	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovddup	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovddup	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovddup	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqa32	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqa32	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vmovdqa32	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqa32	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqa32	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqa32	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqa32	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqa32	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqa32	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqa32	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vmovdqa32	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqa32	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqa32	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqa32	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqa32	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqa32	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqa64	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqa64	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vmovdqa64	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqa64	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqa64	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqa64	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqa64	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqa64	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqa64	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqa64	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vmovdqa64	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqa64	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqa64	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqa64	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqa64	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqa64	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqu32	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqu32	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vmovdqu32	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqu32	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqu32	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqu32	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqu32	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqu32	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqu32	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqu32	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vmovdqu32	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqu32	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqu32	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqu32	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqu32	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqu32	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqu64	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqu64	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vmovdqu64	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqu64	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqu64	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqu64	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqu64	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqu64	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovdqu64	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqu64	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vmovdqu64	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqu64	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqu64	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqu64	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovdqu64	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovdqu64	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovshdup	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmovshdup	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vmovshdup	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovshdup	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vmovshdup	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovshdup	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovshdup	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovshdup	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovshdup	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmovshdup	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vmovshdup	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovshdup	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vmovshdup	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovshdup	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovshdup	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovshdup	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovsldup	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmovsldup	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vmovsldup	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovsldup	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vmovsldup	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovsldup	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovsldup	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovsldup	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovsldup	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmovsldup	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vmovsldup	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovsldup	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vmovsldup	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovsldup	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovsldup	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovsldup	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovupd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmovupd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vmovupd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovupd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vmovupd	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovupd	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovupd	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovupd	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovupd	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmovupd	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vmovupd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovupd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vmovupd	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovupd	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovupd	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovupd	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovups	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmovups	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vmovups	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovups	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vmovups	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovups	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovups	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmovups	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vmovups	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmovups	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vmovups	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovups	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vmovups	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovups	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmovups	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmovups	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vmulpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulpd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vmulpd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulpd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulpd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulpd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmulpd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulpd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmulpd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulpd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmulpd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulpd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmulpd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmulpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vmulpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmulpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmulpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmulpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmulpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmulpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmulpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmulpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmulpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmulpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmulpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmulps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vmulps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmulps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmulps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmulps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vmulps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vmulps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmulps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vmulps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmulps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmulps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmulps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmulps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmulps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmulps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmulps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmulps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vmulps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vmulps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpabsd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpabsd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpabsd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpabsd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpabsd	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpabsd	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsd	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpabsd	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsd	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpabsd	508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsd	512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpabsd	-512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsd	-516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpabsd	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpabsd	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpabsd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpabsd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpabsd	(%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpabsd	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsd	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpabsd	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsd	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpabsd	508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsd	512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpabsd	-512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsd	-516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpabsq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpabsq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpabsq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpabsq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpabsq	(%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vpabsq	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsq	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpabsq	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsq	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpabsq	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsq	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vpabsq	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsq	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vpabsq	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpabsq	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpabsq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpabsq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpabsq	(%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpabsq	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsq	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpabsq	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsq	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpabsq	1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsq	1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpabsq	-1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpabsq	-1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpaddd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddd	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddd	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddd	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddd	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddd	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpaddd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddd	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddd	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddd	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddd	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddd	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpaddq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpaddq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpaddq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpaddq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpaddq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpandd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandd	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandd	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandd	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandd	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandd	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpandd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandd	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandd	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandd	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandd	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandd	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpandnd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnd	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnd	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnd	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnd	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnd	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpandnd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnd	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnd	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnd	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnd	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnd	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpandnq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandnq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpandnq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandnq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandnq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpandq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpandq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpandq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpandq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpandq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpandq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpblendmd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpblendmd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmd	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmd	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmd	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmd	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmd	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpblendmd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpblendmd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpblendmd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpblendmd	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpblendmd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpblendmd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpblendmd	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmd	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpblendmd	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmd	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpbroadcastd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpbroadcastd	(%ecx), %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpbroadcastd	508(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpbroadcastd	512(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpbroadcastd	-512(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpbroadcastd	-516(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpbroadcastd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpbroadcastd	(%ecx), %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpbroadcastd	508(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpbroadcastd	512(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpbroadcastd	-512(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpbroadcastd	-516(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpbroadcastd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpbroadcastd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastd	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpbroadcastd	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastd	%eax, %xmm6{%k7}	 # AVX512{F,VL}
	vpbroadcastd	%eax, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastd	%ebp, %xmm6{%k7}	 # AVX512{F,VL}
	vpbroadcastd	%eax, %ymm6{%k7}	 # AVX512{F,VL}
	vpbroadcastd	%eax, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastd	%ebp, %ymm6{%k7}	 # AVX512{F,VL}
	vpbroadcastq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpbroadcastq	(%ecx), %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpbroadcastq	1016(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpbroadcastq	1024(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpbroadcastq	-1024(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpbroadcastq	-1032(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpbroadcastq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpbroadcastq	(%ecx), %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpbroadcastq	1016(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpbroadcastq	1024(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpbroadcastq	-1024(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpbroadcastq	-1032(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpbroadcastq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpbroadcastq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpbroadcastq	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpbroadcastq	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpcmpd	$0xab, %xmm5, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, %xmm5, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, (%ecx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, -123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, (%eax){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, 2032(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpd	$123, 2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, -2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpd	$123, -2064(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, 508(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpd	$123, 512(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, -512(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpd	$123, -516(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$0xab, %ymm5, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, %ymm5, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, (%ecx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, -123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, (%eax){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, 4064(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpd	$123, 4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, -4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpd	$123, -4128(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, 508(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpd	$123, 512(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpd	$123, -512(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpd	$123, -516(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	%xmm5, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	(%ecx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	-123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	(%eax){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	2032(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqd	2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	-2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqd	-2064(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	508(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqd	512(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	-512(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqd	-516(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	%ymm5, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	(%ecx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	-123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	(%eax){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	4064(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqd	4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	-4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqd	-4128(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	508(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqd	512(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqd	-512(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqd	-516(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	%xmm5, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	(%ecx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	-123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	(%eax){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	2032(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqq	2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	-2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqq	-2064(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	1016(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqq	1024(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	-1024(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqq	-1032(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	%ymm5, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	(%ecx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	-123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	(%eax){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	4064(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqq	4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	-4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqq	-4128(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	1016(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqq	1024(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpeqq	-1024(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpeqq	-1032(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	%xmm5, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	(%ecx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	-123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	(%eax){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	2032(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtd	2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	-2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtd	-2064(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	508(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtd	512(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	-512(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtd	-516(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	%ymm5, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	(%ecx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	-123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	(%eax){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	4064(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtd	4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	-4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtd	-4128(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	508(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtd	512(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtd	-512(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtd	-516(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	%xmm5, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	(%ecx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	-123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	(%eax){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	2032(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtq	2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	-2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtq	-2064(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	1016(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtq	1024(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	-1024(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtq	-1032(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	%ymm5, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	(%ecx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	-123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	(%eax){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	4064(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtq	4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	-4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtq	-4128(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	1016(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtq	1024(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpgtq	-1024(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpgtq	-1032(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$0xab, %xmm5, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, %xmm5, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, (%ecx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, -123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, (%eax){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, 2032(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpq	$123, 2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, -2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpq	$123, -2064(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, 1016(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpq	$123, 1024(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, -1024(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpq	$123, -1032(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$0xab, %ymm5, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, %ymm5, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, (%ecx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, -123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, (%eax){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, 4064(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpq	$123, 4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, -4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpq	$123, -4128(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, 1016(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpq	$123, 1024(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpq	$123, -1024(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpq	$123, -1032(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$0xab, %xmm5, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, %xmm5, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, (%ecx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, -123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, (%eax){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, 2032(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpud	$123, 2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, -2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpud	$123, -2064(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, 508(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpud	$123, 512(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, -512(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpud	$123, -516(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$0xab, %ymm5, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, %ymm5, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, (%ecx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, -123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, (%eax){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, 4064(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpud	$123, 4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, -4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpud	$123, -4128(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, 508(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpud	$123, 512(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpud	$123, -512(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpud	$123, -516(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$0xab, %xmm5, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, %xmm5, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, (%ecx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, -123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, (%eax){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, 2032(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, 2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, -2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, -2064(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, 1016(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, 1024(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, -1024(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, -1032(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$0xab, %ymm5, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, %ymm5, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, (%ecx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, -123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, (%eax){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, 4064(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, 4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, -4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, -4128(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, 1016(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, 1024(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpcmpuq	$123, -1024(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vpcmpuq	$123, -1032(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpblendmq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpblendmq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpblendmq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpblendmq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpblendmq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpblendmq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpblendmq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpblendmq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpblendmq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpblendmq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpblendmq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpblendmq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpcompressd	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpcompressd	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpcompressd	%xmm6, 508(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpcompressd	%xmm6, 512(%edx){%k7}	 # AVX512{F,VL}
	vpcompressd	%xmm6, -512(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpcompressd	%xmm6, -516(%edx){%k7}	 # AVX512{F,VL}
	vpcompressd	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpcompressd	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpcompressd	%ymm6, 508(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpcompressd	%ymm6, 512(%edx){%k7}	 # AVX512{F,VL}
	vpcompressd	%ymm6, -512(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpcompressd	%ymm6, -516(%edx){%k7}	 # AVX512{F,VL}
	vpcompressd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpcompressd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpcompressd	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpcompressd	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermd	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermd	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermd	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermd	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermd	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpermilpd	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$123, (%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$123, 1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	$123, 1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$123, -1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	$123, -1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermilpd	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$123, (%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$123, 1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	$123, 1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	$123, -1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	$123, -1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpermilpd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermilpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpermilps	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	$123, (%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	$123, 508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	$123, 512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	$123, -512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	$123, -516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermilps	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	$123, (%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	$123, 508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	$123, 512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	$123, -512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	$123, -516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpermilps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermilps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermilps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermilps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermilps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermpd	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	$123, (%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermpd	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermpd	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	$123, 1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermpd	$123, 1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	$123, -1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermpd	$123, -1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpermps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermq	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	$123, (%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermq	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermq	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	$123, 1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermq	$123, 1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	$123, -1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermq	$123, -1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpexpandd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpexpandd	(%ecx), %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpexpandd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpexpandd	508(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpexpandd	512(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpexpandd	-512(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpexpandd	-516(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpexpandd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpexpandd	(%ecx), %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpexpandd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpexpandd	508(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpexpandd	512(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpexpandd	-512(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpexpandd	-516(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpexpandd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpexpandd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpexpandd	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpexpandd	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpexpandq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpexpandq	(%ecx), %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpexpandq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpexpandq	1016(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpexpandq	1024(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpexpandq	-1024(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpexpandq	-1032(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpexpandq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpexpandq	(%ecx), %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpexpandq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpexpandq	1016(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpexpandq	1024(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpexpandq	-1024(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpexpandq	-1032(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpexpandq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpexpandq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpexpandq	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpexpandq	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpgatherdd	123(%ebp,%xmm7,8), %xmm6{%k1}	 # AVX512{F,VL}
	vpgatherdd	256(%eax,%xmm7), %xmm6{%k1}	 # AVX512{F,VL}
	vpgatherdd	1024(%ecx,%xmm7,4), %xmm6{%k1}	 # AVX512{F,VL}
	vpgatherdd	123(%ebp,%ymm7,8), %ymm6{%k1}	 # AVX512{F,VL}
	vpgatherdd	256(%eax,%ymm7), %ymm6{%k1}	 # AVX512{F,VL}
	vpgatherdd	1024(%ecx,%ymm7,4), %ymm6{%k1}	 # AVX512{F,VL}
	vpgatherdq	123(%ebp,%xmm7,8), %xmm6{%k1}	 # AVX512{F,VL}
	vpgatherdq	256(%eax,%xmm7), %xmm6{%k1}	 # AVX512{F,VL}
	vpgatherdq	1024(%ecx,%xmm7,4), %xmm6{%k1}	 # AVX512{F,VL}
	vpgatherdq	123(%ebp,%xmm7,8), %ymm6{%k1}	 # AVX512{F,VL}
	vpgatherdq	256(%eax,%xmm7), %ymm6{%k1}	 # AVX512{F,VL}
	vpgatherdq	1024(%ecx,%xmm7,4), %ymm6{%k1}	 # AVX512{F,VL}
	vpgatherqd	123(%ebp,%xmm7,8), %xmm6{%k1}	 # AVX512{F,VL}
	vpgatherqd	256(%eax,%xmm7), %xmm6{%k1}	 # AVX512{F,VL}
	vpgatherqd	1024(%ecx,%xmm7,4), %xmm6{%k1}	 # AVX512{F,VL}
	vpgatherqd	123(%ebp,%ymm7,8), %xmm6{%k1}	 # AVX512{F,VL}
	vpgatherqd	256(%eax,%ymm7), %xmm6{%k1}	 # AVX512{F,VL}
	vpgatherqd	1024(%ecx,%ymm7,4), %xmm6{%k1}	 # AVX512{F,VL}
	vpgatherqq	123(%ebp,%xmm7,8), %xmm6{%k1}	 # AVX512{F,VL}
	vpgatherqq	256(%eax,%xmm7), %xmm6{%k1}	 # AVX512{F,VL}
	vpgatherqq	1024(%ecx,%xmm7,4), %xmm6{%k1}	 # AVX512{F,VL}
	vpgatherqq	123(%ebp,%ymm7,8), %ymm6{%k1}	 # AVX512{F,VL}
	vpgatherqq	256(%eax,%ymm7), %ymm6{%k1}	 # AVX512{F,VL}
	vpgatherqq	1024(%ecx,%ymm7,4), %ymm6{%k1}	 # AVX512{F,VL}
	vpmaxsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmaxsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsd	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsd	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsd	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsd	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsd	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxsd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmaxsd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxsd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxsd	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxsd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxsd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxsd	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsd	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxsd	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsd	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmaxsq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmaxsq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxsq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxsq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxud	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxud	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmaxud	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxud	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxud	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxud	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxud	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxud	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxud	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxud	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxud	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxud	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxud	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxud	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxud	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmaxud	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxud	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxud	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxud	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxud	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxud	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxud	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxud	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxud	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxud	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxud	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmaxuq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxuq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxuq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxuq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxuq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmaxuq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxuq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxuq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxuq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmaxuq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmaxuq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpminsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsd	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsd	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsd	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsd	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsd	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpminsd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsd	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsd	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsd	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsd	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsd	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpminsq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminsq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpminsq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminsq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminsq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminud	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminud	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpminud	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminud	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminud	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminud	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminud	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminud	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminud	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminud	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminud	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminud	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminud	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminud	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminud	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpminud	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminud	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminud	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminud	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminud	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminud	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminud	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminud	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminud	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminud	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminud	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminuq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminuq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpminuq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminuq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminuq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminuq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminuq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminuq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminuq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminuq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminuq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminuq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpminuq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpminuq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminuq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpminuq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminuq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminuq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminuq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminuq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminuq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminuq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminuq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminuq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpminuq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpminuq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	508(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbd	512(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	-512(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbd	-516(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	1016(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbd	1024(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbd	-1024(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbd	-1032(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	254(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbq	256(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	-256(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbq	-258(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxbq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	508(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbq	512(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxbq	-512(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxbq	-516(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxdq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxdq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxdq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxdq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxdq	1016(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxdq	1024(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxdq	-1024(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxdq	-1032(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxdq	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxdq	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxdq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxdq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxdq	2032(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxdq	2048(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxdq	-2048(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxdq	-2064(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	1016(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwd	1024(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	-1024(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwd	-1032(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	2032(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwd	2048(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwd	-2048(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwd	-2064(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	508(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwq	512(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	-512(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwq	-516(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsxwq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	1016(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwq	1024(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovsxwq	-1024(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovsxwq	-1032(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	508(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbd	512(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	-512(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbd	-516(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	1016(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbd	1024(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbd	-1024(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbd	-1032(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	254(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbq	256(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	-256(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbq	-258(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxbq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	508(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbq	512(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxbq	-512(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxbq	-516(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxdq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxdq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxdq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxdq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxdq	1016(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxdq	1024(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxdq	-1024(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxdq	-1032(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxdq	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxdq	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxdq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxdq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxdq	2032(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxdq	2048(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxdq	-2048(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxdq	-2064(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	1016(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwd	1024(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	-1024(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwd	-1032(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	2032(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwd	2048(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwd	-2048(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwd	-2064(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	508(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwq	512(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	-512(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwq	-516(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	%xmm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	%xmm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmovzxwq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	1016(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwq	1024(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmovzxwq	-1024(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmovzxwq	-1032(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpmuldq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuldq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmuldq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuldq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuldq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuldq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuldq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuldq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuldq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuldq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuldq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuldq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuldq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuldq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmuldq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmuldq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmuldq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmuldq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmuldq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuldq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmuldq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuldq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmuldq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuldq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmuldq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuldq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmulld	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmulld	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmulld	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmulld	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmulld	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmulld	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmulld	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmulld	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmulld	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmulld	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmulld	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmulld	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmulld	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmulld	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmulld	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmulld	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmulld	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmulld	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmulld	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmulld	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmulld	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmulld	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmulld	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmulld	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmulld	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmulld	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmuludq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuludq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmuludq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuludq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuludq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuludq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuludq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuludq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuludq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuludq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuludq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuludq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuludq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmuludq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmuludq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpmuludq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmuludq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmuludq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmuludq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuludq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmuludq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuludq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmuludq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuludq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmuludq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpmuludq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpord	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpord	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpord	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpord	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpord	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpord	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpord	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpord	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpord	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpord	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpord	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpord	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpord	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpord	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpord	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpord	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpord	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpord	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpord	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpord	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpord	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpord	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpord	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpord	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpord	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpord	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vporq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vporq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vporq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vporq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vporq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vporq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vporq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vporq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vporq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vporq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vporq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vporq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vporq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vporq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vporq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vporq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vporq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vporq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vporq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vporq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vporq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vporq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vporq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vporq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vporq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vporq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpscatterdd	%xmm6, 123(%ebp,%xmm7,8){%k1}	 # AVX512{F,VL}
	vpscatterdd	%xmm6, 256(%eax,%xmm7){%k1}	 # AVX512{F,VL}
	vpscatterdd	%xmm6, 1024(%ecx,%xmm7,4){%k1}	 # AVX512{F,VL}
	vpscatterdd	%ymm6, 123(%ebp,%ymm7,8){%k1}	 # AVX512{F,VL}
	vpscatterdd	%ymm6, 256(%eax,%ymm7){%k1}	 # AVX512{F,VL}
	vpscatterdd	%ymm6, 1024(%ecx,%ymm7,4){%k1}	 # AVX512{F,VL}
	vpscatterdq	%xmm6, 123(%ebp,%xmm7,8){%k1}	 # AVX512{F,VL}
	vpscatterdq	%xmm6, 256(%eax,%xmm7){%k1}	 # AVX512{F,VL}
	vpscatterdq	%xmm6, 1024(%ecx,%xmm7,4){%k1}	 # AVX512{F,VL}
	vpscatterdq	%ymm6, 123(%ebp,%xmm7,8){%k1}	 # AVX512{F,VL}
	vpscatterdq	%ymm6, 256(%eax,%xmm7){%k1}	 # AVX512{F,VL}
	vpscatterdq	%ymm6, 1024(%ecx,%xmm7,4){%k1}	 # AVX512{F,VL}
	vpscatterqd	%xmm6, 123(%ebp,%xmm7,8){%k1}	 # AVX512{F,VL}
	vpscatterqd	%xmm6, 256(%eax,%xmm7){%k1}	 # AVX512{F,VL}
	vpscatterqd	%xmm6, 1024(%ecx,%xmm7,4){%k1}	 # AVX512{F,VL}
	vpscatterqd	%xmm6, 123(%ebp,%ymm7,8){%k1}	 # AVX512{F,VL}
	vpscatterqd	%xmm6, 256(%eax,%ymm7){%k1}	 # AVX512{F,VL}
	vpscatterqd	%xmm6, 1024(%ecx,%ymm7,4){%k1}	 # AVX512{F,VL}
	vpscatterqq	%xmm6, 123(%ebp,%xmm7,8){%k1}	 # AVX512{F,VL}
	vpscatterqq	%xmm6, 256(%eax,%xmm7){%k1}	 # AVX512{F,VL}
	vpscatterqq	%xmm6, 1024(%ecx,%xmm7,4){%k1}	 # AVX512{F,VL}
	vpscatterqq	%ymm6, 123(%ebp,%ymm7,8){%k1}	 # AVX512{F,VL}
	vpscatterqq	%ymm6, 256(%eax,%ymm7){%k1}	 # AVX512{F,VL}
	vpscatterqq	%ymm6, 1024(%ecx,%ymm7,4){%k1}	 # AVX512{F,VL}
	vpshufd	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpshufd	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpshufd	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpshufd	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpshufd	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpshufd	$123, (%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpshufd	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpshufd	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpshufd	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpshufd	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpshufd	$123, 508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpshufd	$123, 512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpshufd	$123, -512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpshufd	$123, -516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpshufd	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpshufd	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpshufd	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpshufd	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpshufd	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpshufd	$123, (%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpshufd	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpshufd	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpshufd	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpshufd	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpshufd	$123, 508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpshufd	$123, 512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpshufd	$123, -512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpshufd	$123, -516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpslld	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpslld	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpslld	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpslld	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpslld	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpslld	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpslld	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpslld	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpslld	%xmm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpslld	%xmm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpslld	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpslld	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpslld	2032(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpslld	2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpslld	-2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpslld	-2064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsllq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllq	%xmm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllq	%xmm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsllq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllq	2032(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllq	2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllq	-2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllq	-2064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsllvd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvd	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvd	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvd	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvd	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvd	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsllvd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvd	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvd	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvd	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvd	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvd	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsllvq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllvq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsllvq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllvq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllvq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrad	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrad	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsrad	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrad	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrad	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrad	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrad	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrad	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrad	%xmm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrad	%xmm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsrad	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrad	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrad	2032(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrad	2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrad	-2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrad	-2064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsraq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsraq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsraq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsraq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsraq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsraq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsraq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsraq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsraq	%xmm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsraq	%xmm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsraq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsraq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsraq	2032(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsraq	2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsraq	-2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsraq	-2064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsravd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravd	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravd	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravd	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravd	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravd	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsravd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravd	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravd	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravd	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravd	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravd	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsravq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsravq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsravq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsravq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsravq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrld	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrld	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsrld	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrld	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrld	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrld	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrld	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrld	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrld	%xmm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrld	%xmm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsrld	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrld	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrld	2032(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrld	2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrld	-2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrld	-2064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsrlq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlq	%xmm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlq	%xmm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsrlq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlq	2032(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlq	2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlq	-2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlq	-2064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsrlvd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvd	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvd	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsrlvd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvd	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvd	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvd	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsrlvq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsrlvq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlvq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlvq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrld	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrld	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsrld	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrld	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpsrld	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpsrld	$123, (%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrld	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrld	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpsrld	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrld	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpsrld	$123, 508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrld	$123, 512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrld	$123, -512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrld	$123, -516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrld	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrld	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsrld	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrld	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpsrld	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpsrld	$123, (%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrld	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrld	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpsrld	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrld	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpsrld	$123, 508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrld	$123, 512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrld	$123, -512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrld	$123, -516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsrlq	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$123, (%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlq	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlq	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$123, 1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlq	$123, 1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$123, -1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlq	$123, -1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsrlq	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$123, (%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlq	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlq	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$123, 1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlq	$123, 1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrlq	$123, -1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrlq	$123, -1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsubd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubd	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubd	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubd	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubd	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubd	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsubd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubd	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubd	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubd	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubd	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubd	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsubq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsubq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsubq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsubq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsubq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vptestmd	%xmm5, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	(%ecx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	-123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	(%eax){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	2032(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmd	2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	-2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmd	-2064(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	508(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmd	512(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	-512(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmd	-516(%edx){1to4}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	%ymm5, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	(%ecx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	-123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	(%eax){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	4064(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmd	4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	-4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmd	-4128(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	508(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmd	512(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmd	-512(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmd	-516(%edx){1to8}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	%xmm5, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	(%ecx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	-123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	(%eax){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	2032(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmq	2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	-2048(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmq	-2064(%edx), %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	1016(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmq	1024(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	-1024(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmq	-1032(%edx){1to2}, %xmm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	%ymm5, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	(%ecx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	-123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	(%eax){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	4064(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmq	4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	-4096(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmq	-4128(%edx), %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	1016(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmq	1024(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vptestmq	-1024(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestmq	-1032(%edx){1to4}, %ymm6, %k5{%k7}	 # AVX512{F,VL}
	vpunpckhdq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhdq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpunpckhdq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhdq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhdq	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhdq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhdq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhdq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhdq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhdq	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhdq	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhdq	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhdq	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhdq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckhdq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpunpckhdq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckhdq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckhdq	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckhdq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhdq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckhdq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhdq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckhdq	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhdq	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckhdq	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhdq	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpunpckhqdq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpunpckhqdq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckhqdq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpunpckldq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckldq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckldq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckldq	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckldq	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpunpckldq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckldq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckldq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckldq	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpckldq	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpckldq	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpunpcklqdq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpunpcklqdq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpunpcklqdq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxord	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxord	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpxord	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxord	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxord	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxord	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpxord	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxord	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpxord	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxord	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpxord	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxord	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpxord	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxord	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxord	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpxord	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxord	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxord	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxord	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpxord	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxord	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpxord	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxord	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpxord	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxord	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpxord	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxorq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxorq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpxorq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxorq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxorq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxorq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpxorq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxorq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpxorq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxorq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpxorq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxorq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpxorq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpxorq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxorq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpxorq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxorq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxorq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxorq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpxorq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxorq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpxorq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxorq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpxorq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpxorq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpxorq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vrcp14pd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	(%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14pd	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14pd	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14pd	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14pd	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vrcp14pd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	(%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14pd	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14pd	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14pd	1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14pd	-1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14pd	-1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vrcp14ps	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14ps	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14ps	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14ps	512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	-512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14ps	-516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vrcp14ps	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	(%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14ps	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14ps	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14ps	512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vrcp14ps	-512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrcp14ps	-516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vrsqrt14pd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	(%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vrsqrt14pd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	(%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14pd	-1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	-1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vrsqrt14ps	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	-512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	-516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vrsqrt14ps	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	(%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vrsqrt14ps	-512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	-516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vscatterdpd	%xmm6, 123(%ebp,%xmm7,8){%k1}	 # AVX512{F,VL}
	vscatterdpd	%xmm6, 256(%eax,%xmm7){%k1}	 # AVX512{F,VL}
	vscatterdpd	%xmm6, 1024(%ecx,%xmm7,4){%k1}	 # AVX512{F,VL}
	vscatterdpd	%ymm6, 123(%ebp,%xmm7,8){%k1}	 # AVX512{F,VL}
	vscatterdpd	%ymm6, 256(%eax,%xmm7){%k1}	 # AVX512{F,VL}
	vscatterdpd	%ymm6, 1024(%ecx,%xmm7,4){%k1}	 # AVX512{F,VL}
	vscatterdps	%xmm6, 123(%ebp,%xmm7,8){%k1}	 # AVX512{F,VL}
	vscatterdps	%xmm6, 256(%eax,%xmm7){%k1}	 # AVX512{F,VL}
	vscatterdps	%xmm6, 1024(%ecx,%xmm7,4){%k1}	 # AVX512{F,VL}
	vscatterdps	%ymm6, 123(%ebp,%ymm7,8){%k1}	 # AVX512{F,VL}
	vscatterdps	%ymm6, 256(%eax,%ymm7){%k1}	 # AVX512{F,VL}
	vscatterdps	%ymm6, 1024(%ecx,%ymm7,4){%k1}	 # AVX512{F,VL}
	vscatterqpd	%xmm6, 123(%ebp,%xmm7,8){%k1}	 # AVX512{F,VL}
	vscatterqpd	%xmm6, 256(%eax,%xmm7){%k1}	 # AVX512{F,VL}
	vscatterqpd	%xmm6, 1024(%ecx,%xmm7,4){%k1}	 # AVX512{F,VL}
	vscatterqpd	%ymm6, 123(%ebp,%ymm7,8){%k1}	 # AVX512{F,VL}
	vscatterqpd	%ymm6, 256(%eax,%ymm7){%k1}	 # AVX512{F,VL}
	vscatterqpd	%ymm6, 1024(%ecx,%ymm7,4){%k1}	 # AVX512{F,VL}
	vscatterqps	%xmm6, 123(%ebp,%xmm7,8){%k1}	 # AVX512{F,VL}
	vscatterqps	%xmm6, 256(%eax,%xmm7){%k1}	 # AVX512{F,VL}
	vscatterqps	%xmm6, 1024(%ecx,%xmm7,4){%k1}	 # AVX512{F,VL}
	vscatterqps	%xmm6, 123(%ebp,%ymm7,8){%k1}	 # AVX512{F,VL}
	vscatterqps	%xmm6, 256(%eax,%ymm7){%k1}	 # AVX512{F,VL}
	vscatterqps	%xmm6, 1024(%ecx,%ymm7,4){%k1}	 # AVX512{F,VL}
	vshufpd	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufpd	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vshufpd	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufpd	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufpd	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufpd	$123, (%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufpd	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vshufpd	$123, 2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufpd	$123, -2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vshufpd	$123, -2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufpd	$123, 1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vshufpd	$123, 1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufpd	$123, -1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vshufpd	$123, -1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufpd	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufpd	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vshufpd	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufpd	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufpd	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufpd	$123, (%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufpd	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufpd	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufpd	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufpd	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufpd	$123, 1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufpd	$123, 1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufpd	$123, -1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufpd	$123, -1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufps	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufps	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vshufps	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufps	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufps	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufps	$123, (%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufps	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vshufps	$123, 2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufps	$123, -2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vshufps	$123, -2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufps	$123, 508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vshufps	$123, 512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufps	$123, -512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vshufps	$123, -516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vshufps	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufps	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vshufps	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufps	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufps	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufps	$123, (%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufps	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufps	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufps	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufps	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufps	$123, 508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufps	$123, 512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufps	$123, -512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufps	$123, -516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vsqrtpd	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	(%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtpd	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtpd	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtpd	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtpd	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vsqrtpd	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	(%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtpd	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtpd	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtpd	1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtpd	-1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtpd	-1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtps	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtps	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vsqrtps	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtps	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtps	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtps	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtps	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtps	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtps	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtps	508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtps	512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtps	-512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtps	-516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vsqrtps	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtps	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vsqrtps	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtps	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtps	(%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtps	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtps	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtps	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtps	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtps	508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtps	512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vsqrtps	-512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsqrtps	-516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vsubpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubpd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vsubpd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubpd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubpd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubpd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsubpd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubpd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsubpd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubpd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsubpd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubpd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsubpd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsubpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vsubpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsubpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsubpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsubpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsubpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsubpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsubpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsubpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsubpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsubpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsubpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsubps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vsubps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsubps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsubps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsubps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vsubps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vsubps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsubps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vsubps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsubps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsubps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsubps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsubps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsubps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsubps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsubps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsubps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vsubps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vsubps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vunpckhpd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhpd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhpd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhpd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhpd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vunpckhpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vunpckhps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpckhps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vunpckhps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpckhps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpckhps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vunpcklpd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklpd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklpd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklpd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklpd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vunpcklpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vunpcklps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vunpcklps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vunpcklps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vunpcklps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vunpcklps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpternlogd	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$123, (%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogd	$123, 2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$123, -2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogd	$123, -2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$123, 508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogd	$123, 512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$123, -512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogd	$123, -516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpternlogd	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$123, (%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogd	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogd	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$123, 508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogd	$123, 512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogd	$123, -512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogd	$123, -516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpternlogq	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$123, (%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogq	$123, 2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$123, -2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogq	$123, -2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$123, 1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogq	$123, 1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$123, -1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogq	$123, -1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpternlogq	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$123, (%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogq	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogq	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$123, 1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogq	$123, 1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpternlogq	$123, -1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpternlogq	$123, -1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpmovqb	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovqb	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovqb	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovqb	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsqb	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsqb	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsqb	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsqb	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovusqb	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovusqb	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovusqb	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovusqb	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovqw	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovqw	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovqw	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovqw	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsqw	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsqw	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsqw	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsqw	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovusqw	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovusqw	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovusqw	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovusqw	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovqd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovqd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovqd	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovqd	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsqd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsqd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsqd	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsqd	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovusqd	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovusqd	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovusqd	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovusqd	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovdb	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovdb	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovdb	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovdb	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsdb	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsdb	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsdb	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsdb	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovusdb	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovusdb	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovusdb	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovusdb	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovdw	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovdw	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovdw	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovdw	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsdw	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsdw	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovsdw	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovsdw	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovusdw	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovusdw	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpmovusdw	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpmovusdw	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vshuff32x4	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff32x4	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vshuff32x4	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff32x4	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff32x4	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff32x4	$123, (%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff32x4	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshuff32x4	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff32x4	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshuff32x4	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff32x4	$123, 508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshuff32x4	$123, 512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff32x4	$123, -512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshuff32x4	$123, -516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff64x2	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff64x2	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vshuff64x2	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff64x2	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff64x2	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff64x2	$123, (%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff64x2	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshuff64x2	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff64x2	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshuff64x2	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff64x2	$123, 1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshuff64x2	$123, 1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshuff64x2	$123, -1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshuff64x2	$123, -1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi32x4	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi32x4	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vshufi32x4	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi32x4	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi32x4	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi32x4	$123, (%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi32x4	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufi32x4	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi32x4	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufi32x4	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi32x4	$123, 508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufi32x4	$123, 512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi32x4	$123, -512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufi32x4	$123, -516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi64x2	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi64x2	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vshufi64x2	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi64x2	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi64x2	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi64x2	$123, (%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi64x2	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufi64x2	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi64x2	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufi64x2	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi64x2	$123, 1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufi64x2	$123, 1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vshufi64x2	$123, -1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vshufi64x2	$123, -1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2d	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2d	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpermt2d	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2d	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2d	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2d	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2d	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2d	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2d	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2d	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2d	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2d	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2d	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2d	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2d	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermt2d	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2d	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2d	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2d	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2d	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2d	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2d	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2d	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2d	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2d	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2d	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2q	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2q	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpermt2q	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2q	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2q	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2q	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2q	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2q	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2q	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2q	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2q	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2q	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2q	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2q	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2q	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermt2q	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2q	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2q	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2q	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2q	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2q	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2q	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2q	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2q	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2q	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2q	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpermt2ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermt2ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpermt2pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermt2pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermt2pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermt2pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignq	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignq	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	valignq	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignq	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignq	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignq	$123, (%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignq	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	valignq	$123, 2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignq	$123, -2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	valignq	$123, -2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignq	$123, 1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	valignq	$123, 1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignq	$123, -1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	valignq	$123, -1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	valignq	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignq	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	valignq	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignq	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignq	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignq	$123, (%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignq	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	valignq	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignq	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	valignq	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignq	$123, 1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	valignq	$123, 1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	valignq	$123, -1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	valignq	$123, -1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefpd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vscalefpd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefpd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefpd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefpd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefpd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefpd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefpd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefpd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefpd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefpd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefpd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefpd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefpd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vscalefpd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefpd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefpd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefpd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefpd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefpd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefpd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefpd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefpd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefpd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefpd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vscalefps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vscalefps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vscalefps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vscalefps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vscalefps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfixupimmpd	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$123, (%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, 2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$123, -2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, -2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$123, 1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, 1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$123, -1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, -1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfixupimmpd	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$123, (%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$123, 1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, 1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmpd	$123, -1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmpd	$123, -1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vfixupimmps	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$123, (%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, 2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$123, -2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, -2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$123, 508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, 512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$123, -512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, -516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vfixupimmps	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$123, (%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$123, 508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, 512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vfixupimmps	$123, -512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vfixupimmps	$123, -516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpslld	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpslld	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpslld	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpslld	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpslld	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpslld	$123, (%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpslld	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpslld	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpslld	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpslld	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpslld	$123, 508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpslld	$123, 512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpslld	$123, -512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpslld	$123, -516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpslld	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpslld	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpslld	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpslld	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpslld	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpslld	$123, (%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpslld	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpslld	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpslld	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpslld	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpslld	$123, 508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpslld	$123, 512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpslld	$123, -512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpslld	$123, -516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllq	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllq	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsllq	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllq	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpsllq	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpsllq	$123, (%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllq	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllq	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpsllq	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllq	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpsllq	$123, 1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllq	$123, 1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllq	$123, -1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllq	$123, -1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vpsllq	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllq	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsllq	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllq	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpsllq	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpsllq	$123, (%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllq	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllq	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpsllq	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllq	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpsllq	$123, 1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllq	$123, 1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpsllq	$123, -1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsllq	$123, -1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrad	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrad	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsrad	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrad	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpsrad	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpsrad	$123, (%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrad	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrad	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpsrad	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrad	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpsrad	$123, 508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrad	$123, 512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrad	$123, -512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrad	$123, -516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vpsrad	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrad	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsrad	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrad	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpsrad	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpsrad	$123, (%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrad	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrad	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpsrad	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrad	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpsrad	$123, 508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrad	$123, 512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpsrad	$123, -512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsrad	$123, -516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpsraq	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsraq	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpsraq	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpsraq	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vpsraq	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vpsraq	$123, (%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vpsraq	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsraq	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpsraq	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsraq	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vpsraq	$123, 1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsraq	$123, 1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vpsraq	$123, -1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpsraq	$123, -1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vpsraq	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsraq	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpsraq	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpsraq	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vpsraq	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vpsraq	$123, (%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpsraq	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsraq	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpsraq	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsraq	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vpsraq	$123, 1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsraq	$123, 1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vpsraq	$123, -1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpsraq	$123, -1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vprolvd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvd	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvd	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvd	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvd	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvd	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vprolvd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvd	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvd	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvd	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvd	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvd	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprold	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprold	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vprold	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprold	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vprold	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vprold	$123, (%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vprold	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprold	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vprold	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprold	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vprold	$123, 508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprold	$123, 512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vprold	$123, -512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprold	$123, -516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vprold	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprold	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vprold	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprold	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vprold	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vprold	$123, (%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vprold	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprold	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vprold	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprold	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vprold	$123, 508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprold	$123, 512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vprold	$123, -512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprold	$123, -516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vprolvq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolvq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vprolvq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolvq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprolvq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolq	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolq	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vprolq	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprolq	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vprolq	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vprolq	$123, (%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vprolq	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprolq	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vprolq	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprolq	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vprolq	$123, 1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprolq	$123, 1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vprolq	$123, -1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprolq	$123, -1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vprolq	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolq	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vprolq	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprolq	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vprolq	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vprolq	$123, (%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vprolq	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprolq	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vprolq	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprolq	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vprolq	$123, 1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprolq	$123, 1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vprolq	$123, -1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprolq	$123, -1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vprorvd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvd	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvd	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvd	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvd	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvd	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vprorvd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvd	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvd	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvd	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvd	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvd	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprord	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprord	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vprord	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprord	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vprord	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vprord	$123, (%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vprord	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprord	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vprord	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprord	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vprord	$123, 508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprord	$123, 512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vprord	$123, -512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprord	$123, -516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vprord	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprord	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vprord	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprord	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vprord	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vprord	$123, (%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vprord	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprord	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vprord	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprord	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vprord	$123, 508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprord	$123, 512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vprord	$123, -512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprord	$123, -516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vprorvq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorvq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vprorvq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorvq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprorvq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorq	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorq	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vprorq	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vprorq	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vprorq	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vprorq	$123, (%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vprorq	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprorq	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vprorq	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprorq	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vprorq	$123, 1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprorq	$123, 1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vprorq	$123, -1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vprorq	$123, -1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vprorq	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorq	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vprorq	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vprorq	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vprorq	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vprorq	$123, (%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vprorq	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprorq	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vprorq	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprorq	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vprorq	$123, 1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprorq	$123, 1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vprorq	$123, -1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vprorq	$123, -1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vrndscalepd	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$123, (%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$123, 1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, 1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$123, -1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, -1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vrndscalepd	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$123, (%eax){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$123, 1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, 1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vrndscalepd	$123, -1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscalepd	$123, -1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vrndscaleps	$123, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$123, (%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$123, (%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$123, 508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, 512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$123, -512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, -516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vrndscaleps	$123, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$123, (%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$123, (%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$123, 508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, 512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vrndscaleps	$123, -512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vrndscaleps	$123, -516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpcompressq	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpcompressq	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpcompressq	%xmm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpcompressq	%xmm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vpcompressq	%xmm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpcompressq	%xmm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vpcompressq	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpcompressq	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpcompressq	%ymm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpcompressq	%ymm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vpcompressq	%ymm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpcompressq	%ymm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vpcompressq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpcompressq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpcompressq	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpcompressq	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vcvtps2ph	$123, %xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vcvtps2ph	$123, %xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vcvtps2ph	$123, %xmm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vcvtps2ph	$123, %xmm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vcvtps2ph	$123, %xmm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vcvtps2ph	$123, %xmm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vcvtps2ph	$0xab, %ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vcvtps2ph	$123, %ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vcvtps2ph	$123, %ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vcvtps2ph	$123, %ymm6, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vcvtps2ph	$123, %ymm6, 2048(%edx){%k7}	 # AVX512{F,VL}
	vcvtps2ph	$123, %ymm6, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vcvtps2ph	$123, %ymm6, -2064(%edx){%k7}	 # AVX512{F,VL}
	vextractf32x4	$0xab, %ymm5, (%ecx){%k7}	 # AVX512{F,VL}
	vextractf32x4	$123, %ymm5, (%ecx){%k7}	 # AVX512{F,VL}
	vextractf32x4	$123, %ymm5, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vextractf32x4	$123, %ymm5, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vextractf32x4	$123, %ymm5, 2048(%edx){%k7}	 # AVX512{F,VL}
	vextractf32x4	$123, %ymm5, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vextractf32x4	$123, %ymm5, -2064(%edx){%k7}	 # AVX512{F,VL}
	vextracti32x4	$0xab, %ymm5, (%ecx){%k7}	 # AVX512{F,VL}
	vextracti32x4	$123, %ymm5, (%ecx){%k7}	 # AVX512{F,VL}
	vextracti32x4	$123, %ymm5, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vextracti32x4	$123, %ymm5, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vextracti32x4	$123, %ymm5, 2048(%edx){%k7}	 # AVX512{F,VL}
	vextracti32x4	$123, %ymm5, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vextracti32x4	$123, %ymm5, -2064(%edx){%k7}	 # AVX512{F,VL}
	vmovapd	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovapd	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovapd	%xmm6, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovapd	%xmm6, 2048(%edx){%k7}	 # AVX512{F,VL}
	vmovapd	%xmm6, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovapd	%xmm6, -2064(%edx){%k7}	 # AVX512{F,VL}
	vmovapd	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovapd	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovapd	%ymm6, 4064(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovapd	%ymm6, 4096(%edx){%k7}	 # AVX512{F,VL}
	vmovapd	%ymm6, -4096(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovapd	%ymm6, -4128(%edx){%k7}	 # AVX512{F,VL}
	vmovaps	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovaps	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovaps	%xmm6, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovaps	%xmm6, 2048(%edx){%k7}	 # AVX512{F,VL}
	vmovaps	%xmm6, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovaps	%xmm6, -2064(%edx){%k7}	 # AVX512{F,VL}
	vmovaps	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovaps	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovaps	%ymm6, 4064(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovaps	%ymm6, 4096(%edx){%k7}	 # AVX512{F,VL}
	vmovaps	%ymm6, -4096(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovaps	%ymm6, -4128(%edx){%k7}	 # AVX512{F,VL}
	vmovdqa32	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovdqa32	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovdqa32	%xmm6, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqa32	%xmm6, 2048(%edx){%k7}	 # AVX512{F,VL}
	vmovdqa32	%xmm6, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqa32	%xmm6, -2064(%edx){%k7}	 # AVX512{F,VL}
	vmovdqa32	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovdqa32	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovdqa32	%ymm6, 4064(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqa32	%ymm6, 4096(%edx){%k7}	 # AVX512{F,VL}
	vmovdqa32	%ymm6, -4096(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqa32	%ymm6, -4128(%edx){%k7}	 # AVX512{F,VL}
	vmovdqa64	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovdqa64	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovdqa64	%xmm6, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqa64	%xmm6, 2048(%edx){%k7}	 # AVX512{F,VL}
	vmovdqa64	%xmm6, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqa64	%xmm6, -2064(%edx){%k7}	 # AVX512{F,VL}
	vmovdqa64	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovdqa64	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovdqa64	%ymm6, 4064(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqa64	%ymm6, 4096(%edx){%k7}	 # AVX512{F,VL}
	vmovdqa64	%ymm6, -4096(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqa64	%ymm6, -4128(%edx){%k7}	 # AVX512{F,VL}
	vmovdqu32	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovdqu32	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovdqu32	%xmm6, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqu32	%xmm6, 2048(%edx){%k7}	 # AVX512{F,VL}
	vmovdqu32	%xmm6, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqu32	%xmm6, -2064(%edx){%k7}	 # AVX512{F,VL}
	vmovdqu32	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovdqu32	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovdqu32	%ymm6, 4064(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqu32	%ymm6, 4096(%edx){%k7}	 # AVX512{F,VL}
	vmovdqu32	%ymm6, -4096(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqu32	%ymm6, -4128(%edx){%k7}	 # AVX512{F,VL}
	vmovdqu64	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovdqu64	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovdqu64	%xmm6, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqu64	%xmm6, 2048(%edx){%k7}	 # AVX512{F,VL}
	vmovdqu64	%xmm6, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqu64	%xmm6, -2064(%edx){%k7}	 # AVX512{F,VL}
	vmovdqu64	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovdqu64	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovdqu64	%ymm6, 4064(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqu64	%ymm6, 4096(%edx){%k7}	 # AVX512{F,VL}
	vmovdqu64	%ymm6, -4096(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovdqu64	%ymm6, -4128(%edx){%k7}	 # AVX512{F,VL}
	vmovupd	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovupd	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovupd	%xmm6, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovupd	%xmm6, 2048(%edx){%k7}	 # AVX512{F,VL}
	vmovupd	%xmm6, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovupd	%xmm6, -2064(%edx){%k7}	 # AVX512{F,VL}
	vmovupd	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovupd	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovupd	%ymm6, 4064(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovupd	%ymm6, 4096(%edx){%k7}	 # AVX512{F,VL}
	vmovupd	%ymm6, -4096(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovupd	%ymm6, -4128(%edx){%k7}	 # AVX512{F,VL}
	vmovups	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovups	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovups	%xmm6, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovups	%xmm6, 2048(%edx){%k7}	 # AVX512{F,VL}
	vmovups	%xmm6, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovups	%xmm6, -2064(%edx){%k7}	 # AVX512{F,VL}
	vmovups	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vmovups	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vmovups	%ymm6, 4064(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovups	%ymm6, 4096(%edx){%k7}	 # AVX512{F,VL}
	vmovups	%ymm6, -4096(%edx){%k7}	 # AVX512{F,VL} Disp8
	vmovups	%ymm6, -4128(%edx){%k7}	 # AVX512{F,VL}
	vpmovqb	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovqb	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovqb	%xmm6, 254(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovqb	%xmm6, 256(%edx){%k7}	 # AVX512{F,VL}
	vpmovqb	%xmm6, -256(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovqb	%xmm6, -258(%edx){%k7}	 # AVX512{F,VL}
	vpmovqb	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovqb	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovqb	%ymm6, 508(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovqb	%ymm6, 512(%edx){%k7}	 # AVX512{F,VL}
	vpmovqb	%ymm6, -512(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovqb	%ymm6, -516(%edx){%k7}	 # AVX512{F,VL}
	vpmovsqb	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovsqb	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovsqb	%xmm6, 254(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsqb	%xmm6, 256(%edx){%k7}	 # AVX512{F,VL}
	vpmovsqb	%xmm6, -256(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsqb	%xmm6, -258(%edx){%k7}	 # AVX512{F,VL}
	vpmovsqb	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovsqb	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovsqb	%ymm6, 508(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsqb	%ymm6, 512(%edx){%k7}	 # AVX512{F,VL}
	vpmovsqb	%ymm6, -512(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsqb	%ymm6, -516(%edx){%k7}	 # AVX512{F,VL}
	vpmovusqb	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovusqb	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovusqb	%xmm6, 254(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusqb	%xmm6, 256(%edx){%k7}	 # AVX512{F,VL}
	vpmovusqb	%xmm6, -256(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusqb	%xmm6, -258(%edx){%k7}	 # AVX512{F,VL}
	vpmovusqb	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovusqb	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovusqb	%ymm6, 508(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusqb	%ymm6, 512(%edx){%k7}	 # AVX512{F,VL}
	vpmovusqb	%ymm6, -512(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusqb	%ymm6, -516(%edx){%k7}	 # AVX512{F,VL}
	vpmovqw	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovqw	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovqw	%xmm6, 508(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovqw	%xmm6, 512(%edx){%k7}	 # AVX512{F,VL}
	vpmovqw	%xmm6, -512(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovqw	%xmm6, -516(%edx){%k7}	 # AVX512{F,VL}
	vpmovqw	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovqw	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovqw	%ymm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovqw	%ymm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vpmovqw	%ymm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovqw	%ymm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vpmovsqw	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovsqw	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovsqw	%xmm6, 508(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsqw	%xmm6, 512(%edx){%k7}	 # AVX512{F,VL}
	vpmovsqw	%xmm6, -512(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsqw	%xmm6, -516(%edx){%k7}	 # AVX512{F,VL}
	vpmovsqw	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovsqw	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovsqw	%ymm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsqw	%ymm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vpmovsqw	%ymm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsqw	%ymm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vpmovusqw	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovusqw	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovusqw	%xmm6, 508(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusqw	%xmm6, 512(%edx){%k7}	 # AVX512{F,VL}
	vpmovusqw	%xmm6, -512(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusqw	%xmm6, -516(%edx){%k7}	 # AVX512{F,VL}
	vpmovusqw	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovusqw	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovusqw	%ymm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusqw	%ymm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vpmovusqw	%ymm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusqw	%ymm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vpmovqd	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovqd	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovqd	%xmm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovqd	%xmm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vpmovqd	%xmm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovqd	%xmm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vpmovqd	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovqd	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovqd	%ymm6, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovqd	%ymm6, 2048(%edx){%k7}	 # AVX512{F,VL}
	vpmovqd	%ymm6, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovqd	%ymm6, -2064(%edx){%k7}	 # AVX512{F,VL}
	vpmovsqd	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovsqd	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovsqd	%xmm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsqd	%xmm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vpmovsqd	%xmm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsqd	%xmm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vpmovsqd	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovsqd	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovsqd	%ymm6, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsqd	%ymm6, 2048(%edx){%k7}	 # AVX512{F,VL}
	vpmovsqd	%ymm6, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsqd	%ymm6, -2064(%edx){%k7}	 # AVX512{F,VL}
	vpmovusqd	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovusqd	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovusqd	%xmm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusqd	%xmm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vpmovusqd	%xmm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusqd	%xmm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vpmovusqd	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovusqd	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovusqd	%ymm6, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusqd	%ymm6, 2048(%edx){%k7}	 # AVX512{F,VL}
	vpmovusqd	%ymm6, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusqd	%ymm6, -2064(%edx){%k7}	 # AVX512{F,VL}
	vpmovdb	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovdb	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovdb	%xmm6, 508(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovdb	%xmm6, 512(%edx){%k7}	 # AVX512{F,VL}
	vpmovdb	%xmm6, -512(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovdb	%xmm6, -516(%edx){%k7}	 # AVX512{F,VL}
	vpmovdb	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovdb	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovdb	%ymm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovdb	%ymm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vpmovdb	%ymm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovdb	%ymm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vpmovsdb	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovsdb	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovsdb	%xmm6, 508(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsdb	%xmm6, 512(%edx){%k7}	 # AVX512{F,VL}
	vpmovsdb	%xmm6, -512(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsdb	%xmm6, -516(%edx){%k7}	 # AVX512{F,VL}
	vpmovsdb	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovsdb	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovsdb	%ymm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsdb	%ymm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vpmovsdb	%ymm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsdb	%ymm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vpmovusdb	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovusdb	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovusdb	%xmm6, 508(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusdb	%xmm6, 512(%edx){%k7}	 # AVX512{F,VL}
	vpmovusdb	%xmm6, -512(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusdb	%xmm6, -516(%edx){%k7}	 # AVX512{F,VL}
	vpmovusdb	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovusdb	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovusdb	%ymm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusdb	%ymm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vpmovusdb	%ymm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusdb	%ymm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vpmovdw	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovdw	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovdw	%xmm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovdw	%xmm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vpmovdw	%xmm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovdw	%xmm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vpmovdw	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovdw	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovdw	%ymm6, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovdw	%ymm6, 2048(%edx){%k7}	 # AVX512{F,VL}
	vpmovdw	%ymm6, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovdw	%ymm6, -2064(%edx){%k7}	 # AVX512{F,VL}
	vpmovsdw	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovsdw	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovsdw	%xmm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsdw	%xmm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vpmovsdw	%xmm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsdw	%xmm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vpmovsdw	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovsdw	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovsdw	%ymm6, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsdw	%ymm6, 2048(%edx){%k7}	 # AVX512{F,VL}
	vpmovsdw	%ymm6, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovsdw	%ymm6, -2064(%edx){%k7}	 # AVX512{F,VL}
	vpmovusdw	%xmm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovusdw	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovusdw	%xmm6, 1016(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusdw	%xmm6, 1024(%edx){%k7}	 # AVX512{F,VL}
	vpmovusdw	%xmm6, -1024(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusdw	%xmm6, -1032(%edx){%k7}	 # AVX512{F,VL}
	vpmovusdw	%ymm6, (%ecx){%k7}	 # AVX512{F,VL}
	vpmovusdw	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{F,VL}
	vpmovusdw	%ymm6, 2032(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusdw	%ymm6, 2048(%edx){%k7}	 # AVX512{F,VL}
	vpmovusdw	%ymm6, -2048(%edx){%k7}	 # AVX512{F,VL} Disp8
	vpmovusdw	%ymm6, -2064(%edx){%k7}	 # AVX512{F,VL}
	vcvttpd2udq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2udq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvttpd2udqx	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2udqx	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2udq	(%eax){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2udqx	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2udqx	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2udqx	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2udqx	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2udqx	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2udqx	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2udqx	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2udqx	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2udq	%ymm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2udq	%ymm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvttpd2udqy	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2udqy	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2udq	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2udqy	4064(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2udqy	4096(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2udqy	-4096(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2udqy	-4128(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2udqy	1016(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2udqy	1024(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttpd2udqy	-1024(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttpd2udqy	-1032(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	%xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	%xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vcvttps2udq	(%ecx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	(%eax){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	2032(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2udq	2048(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	-2048(%edx), %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2udq	-2064(%edx), %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	508(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2udq	512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	-512(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2udq	-516(%edx){1to4}, %xmm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	%ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	%ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vcvttps2udq	(%ecx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	(%eax){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	4064(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2udq	4096(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	-4096(%edx), %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2udq	-4128(%edx), %ymm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	508(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2udq	512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vcvttps2udq	-512(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vcvttps2udq	-516(%edx){1to8}, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2d	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2d	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpermi2d	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2d	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2d	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2d	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2d	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2d	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2d	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2d	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2d	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2d	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2d	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2d	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2d	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermi2d	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2d	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2d	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2d	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2d	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2d	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2d	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2d	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2d	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2d	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2d	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2q	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2q	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpermi2q	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2q	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2q	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2q	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2q	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2q	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2q	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2q	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2q	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2q	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2q	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2q	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2q	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermi2q	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2q	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2q	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2q	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2q	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2q	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2q	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2q	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2q	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2q	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2q	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpermi2ps	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2ps	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2ps	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2ps	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2ps	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermi2ps	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2ps	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2ps	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2ps	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2ps	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2ps	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{F,VL}
	vpermi2pd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2pd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2pd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2pd	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2pd	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{F,VL}
	vpermi2pd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2pd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2pd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2pd	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vpermi2pd	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL} Disp8
	vpermi2pd	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{F,VL}
	vptestnmd	%xmm4, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	(%ecx), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	(%eax){1to4}, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	2032(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmd	2048(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	-2048(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmd	-2064(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	508(%edx){1to4}, %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmd	512(%edx){1to4}, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	-512(%edx){1to4}, %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmd	-516(%edx){1to4}, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	%ymm4, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	(%ecx), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	-123456(%esp,%esi,8), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	(%eax){1to8}, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	4064(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmd	4096(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	-4096(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmd	-4128(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	508(%edx){1to8}, %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmd	512(%edx){1to8}, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmd	-512(%edx){1to8}, %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmd	-516(%edx){1to8}, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	%xmm4, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	(%ecx), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	(%eax){1to2}, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	2032(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmq	2048(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	-2048(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmq	-2064(%edx), %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	1016(%edx){1to2}, %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmq	1024(%edx){1to2}, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	-1024(%edx){1to2}, %xmm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmq	-1032(%edx){1to2}, %xmm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	%ymm4, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	(%ecx), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	-123456(%esp,%esi,8), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	(%eax){1to4}, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	4064(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmq	4096(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	-4096(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmq	-4128(%edx), %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	1016(%edx){1to4}, %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmq	1024(%edx){1to4}, %ymm5, %k5{%k7}	 # AVX512{F,VL}
	vptestnmq	-1024(%edx){1to4}, %ymm5, %k5{%k7}	 # AVX512{F,VL} Disp8
	vptestnmq	-1032(%edx){1to4}, %ymm5, %k5{%k7}	 # AVX512{F,VL}

	.intel_syntax noprefix
	vaddpd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vaddpd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vaddpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vaddpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vaddpd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vaddpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vaddpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vaddpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vaddpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vaddpd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vaddpd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vaddpd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vaddpd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vaddpd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vaddpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vaddpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vaddpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vaddpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vaddpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vaddpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vaddpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vaddpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vaddpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vaddpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vaddpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vaddpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vaddps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vaddps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vaddps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vaddps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vaddps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vaddps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vaddps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vaddps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vaddps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vaddps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vaddps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vaddps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vaddps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vaddps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vaddps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vaddps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vaddps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vaddps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vaddps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vaddps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vaddps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vaddps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vaddps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vaddps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vaddps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vaddps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	valignd	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	valignd	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	valignd	xmm6{k7}, xmm5, xmm4, 123	 # AVX512{F,VL}
	valignd	xmm6{k7}, xmm5, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	valignd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	valignd	xmm6{k7}, xmm5, [eax]{1to4}, 123	 # AVX512{F,VL}
	valignd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	valignd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	valignd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	valignd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	valignd	xmm6{k7}, xmm5, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	valignd	xmm6{k7}, xmm5, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	valignd	xmm6{k7}, xmm5, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	valignd	xmm6{k7}, xmm5, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	valignd	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	valignd	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	valignd	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{F,VL}
	valignd	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	valignd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	valignd	ymm6{k7}, ymm5, [eax]{1to8}, 123	 # AVX512{F,VL}
	valignd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	valignd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	valignd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	valignd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	valignd	ymm6{k7}, ymm5, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	valignd	ymm6{k7}, ymm5, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	valignd	ymm6{k7}, ymm5, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	valignd	ymm6{k7}, ymm5, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vblendmpd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vblendmpd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vblendmpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vblendmpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vblendmpd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vblendmpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vblendmpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vblendmpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vblendmpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vblendmpd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vblendmpd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vblendmpd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vblendmpd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vblendmpd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vblendmpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vblendmpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vblendmpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vblendmpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vblendmpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vblendmpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vblendmpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vblendmpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vblendmpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vblendmpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vblendmpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vblendmpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vblendmps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vblendmps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vblendmps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vblendmps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vblendmps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vblendmps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vblendmps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vblendmps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vblendmps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vblendmps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vblendmps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vblendmps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vblendmps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vblendmps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vblendmps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vblendmps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vblendmps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vblendmps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vblendmps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vblendmps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vblendmps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vblendmps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vblendmps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vblendmps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vblendmps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vblendmps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vbroadcastf32x4	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vbroadcastf32x4	ymm6{k7}{z}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vbroadcastf32x4	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vbroadcastf32x4	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vbroadcastf32x4	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vbroadcastf32x4	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vbroadcastf32x4	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vbroadcasti32x4	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vbroadcasti32x4	ymm6{k7}{z}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vbroadcasti32x4	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vbroadcasti32x4	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vbroadcasti32x4	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vbroadcasti32x4	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vbroadcasti32x4	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vbroadcastsd	ymm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vbroadcastsd	ymm6{k7}{z}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vbroadcastsd	ymm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vbroadcastsd	ymm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vbroadcastsd	ymm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vbroadcastsd	ymm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vbroadcastsd	ymm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vbroadcastsd	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vbroadcastsd	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vbroadcastss	xmm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vbroadcastss	xmm6{k7}{z}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vbroadcastss	xmm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vbroadcastss	xmm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vbroadcastss	xmm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vbroadcastss	xmm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vbroadcastss	xmm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vbroadcastss	ymm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vbroadcastss	ymm6{k7}{z}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vbroadcastss	ymm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vbroadcastss	ymm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vbroadcastss	ymm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vbroadcastss	ymm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vbroadcastss	ymm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vbroadcastss	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vbroadcastss	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vbroadcastss	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vbroadcastss	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcmppd	k5{k7}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	vcmppd	k5{k7}, xmm5, xmm4, 123	 # AVX512{F,VL}
	vcmppd	k5{k7}, xmm5, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vcmppd	k5{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vcmppd	k5{k7}, xmm5, [eax]{1to2}, 123	 # AVX512{F,VL}
	vcmppd	k5{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vcmppd	k5{k7}, xmm5, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vcmppd	k5{k7}, xmm5, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vcmppd	k5{k7}, xmm5, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vcmppd	k5{k7}, xmm5, [edx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vcmppd	k5{k7}, xmm5, [edx+1024]{1to2}, 123	 # AVX512{F,VL}
	vcmppd	k5{k7}, xmm5, [edx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vcmppd	k5{k7}, xmm5, [edx-1032]{1to2}, 123	 # AVX512{F,VL}
	vcmppd	k5{k7}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vcmppd	k5{k7}, ymm5, ymm4, 123	 # AVX512{F,VL}
	vcmppd	k5{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vcmppd	k5{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vcmppd	k5{k7}, ymm5, [eax]{1to4}, 123	 # AVX512{F,VL}
	vcmppd	k5{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vcmppd	k5{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vcmppd	k5{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vcmppd	k5{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vcmppd	k5{k7}, ymm5, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vcmppd	k5{k7}, ymm5, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vcmppd	k5{k7}, ymm5, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vcmppd	k5{k7}, ymm5, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	vcmpps	k5{k7}, xmm5, xmm4, 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, xmm5, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, xmm5, [eax]{1to4}, 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vcmpps	k5{k7}, xmm5, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, xmm5, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vcmpps	k5{k7}, xmm5, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, xmm5, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vcmpps	k5{k7}, xmm5, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, xmm5, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vcmpps	k5{k7}, xmm5, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vcmpps	k5{k7}, ymm5, ymm4, 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, ymm5, [eax]{1to8}, 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vcmpps	k5{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vcmpps	k5{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, ymm5, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vcmpps	k5{k7}, ymm5, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vcmpps	k5{k7}, ymm5, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vcmpps	k5{k7}, ymm5, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vcompresspd	XMMWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vcompresspd	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vcompresspd	XMMWORD PTR [edx+1016]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vcompresspd	XMMWORD PTR [edx+1024]{k7}, xmm6	 # AVX512{F,VL}
	vcompresspd	XMMWORD PTR [edx-1024]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vcompresspd	XMMWORD PTR [edx-1032]{k7}, xmm6	 # AVX512{F,VL}
	vcompresspd	YMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vcompresspd	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vcompresspd	YMMWORD PTR [edx+1016]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vcompresspd	YMMWORD PTR [edx+1024]{k7}, ymm6	 # AVX512{F,VL}
	vcompresspd	YMMWORD PTR [edx-1024]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vcompresspd	YMMWORD PTR [edx-1032]{k7}, ymm6	 # AVX512{F,VL}
	vcompresspd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcompresspd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcompresspd	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vcompresspd	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vcompressps	XMMWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vcompressps	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vcompressps	XMMWORD PTR [edx+508]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vcompressps	XMMWORD PTR [edx+512]{k7}, xmm6	 # AVX512{F,VL}
	vcompressps	XMMWORD PTR [edx-512]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vcompressps	XMMWORD PTR [edx-516]{k7}, xmm6	 # AVX512{F,VL}
	vcompressps	YMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vcompressps	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vcompressps	YMMWORD PTR [edx+508]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vcompressps	YMMWORD PTR [edx+512]{k7}, ymm6	 # AVX512{F,VL}
	vcompressps	YMMWORD PTR [edx-512]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vcompressps	YMMWORD PTR [edx-516]{k7}, ymm6	 # AVX512{F,VL}
	vcompressps	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcompressps	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcompressps	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vcompressps	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vcvtdq2pd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcvtdq2pd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvtdq2pd	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtdq2pd	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtdq2pd	xmm6{k7}, [eax]{1to2}	 # AVX512{F,VL}
	vcvtdq2pd	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vcvtdq2pd	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vcvtdq2pd	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vcvtdq2pd	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vcvtdq2pd	xmm6{k7}, [edx+508]{1to2}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	xmm6{k7}, [edx+512]{1to2}	 # AVX512{F,VL}
	vcvtdq2pd	xmm6{k7}, [edx-512]{1to2}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	xmm6{k7}, [edx-516]{1to2}	 # AVX512{F,VL}
	vcvtdq2pd	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vcvtdq2pd	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvtdq2pd	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtdq2pd	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtdq2pd	ymm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vcvtdq2pd	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vcvtdq2pd	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vcvtdq2pd	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vcvtdq2pd	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vcvtdq2pd	ymm6{k7}, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	ymm6{k7}, [edx+512]{1to4}	 # AVX512{F,VL}
	vcvtdq2pd	ymm6{k7}, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvtdq2pd	ymm6{k7}, [edx-516]{1to4}	 # AVX512{F,VL}
	vcvtdq2ps	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcvtdq2ps	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvtdq2ps	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtdq2ps	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtdq2ps	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vcvtdq2ps	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vcvtdq2ps	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vcvtdq2ps	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vcvtdq2ps	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vcvtdq2ps	xmm6{k7}, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	xmm6{k7}, [edx+512]{1to4}	 # AVX512{F,VL}
	vcvtdq2ps	xmm6{k7}, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	xmm6{k7}, [edx-516]{1to4}	 # AVX512{F,VL}
	vcvtdq2ps	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vcvtdq2ps	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vcvtdq2ps	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtdq2ps	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtdq2ps	ymm6{k7}, [eax]{1to8}	 # AVX512{F,VL}
	vcvtdq2ps	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vcvtdq2ps	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vcvtdq2ps	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vcvtdq2ps	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vcvtdq2ps	ymm6{k7}, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	ymm6{k7}, [edx+512]{1to8}	 # AVX512{F,VL}
	vcvtdq2ps	ymm6{k7}, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vcvtdq2ps	ymm6{k7}, [edx-516]{1to8}	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, [eax]{1to2}	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, QWORD BCST [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm6{k7}, QWORD BCST [edx+1024]{1to2}	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, QWORD BCST [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm6{k7}, QWORD BCST [edx-1032]{1to2}	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, QWORD BCST [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm6{k7}, QWORD BCST [edx+1024]{1to4}	 # AVX512{F,VL}
	vcvtpd2dq	xmm6{k7}, QWORD BCST [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vcvtpd2dq	xmm6{k7}, QWORD BCST [edx-1032]{1to4}	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, [eax]{1to2}	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, QWORD BCST [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm6{k7}, QWORD BCST [edx+1024]{1to2}	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, QWORD BCST [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm6{k7}, QWORD BCST [edx-1032]{1to2}	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, QWORD BCST [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm6{k7}, QWORD BCST [edx+1024]{1to4}	 # AVX512{F,VL}
	vcvtpd2ps	xmm6{k7}, QWORD BCST [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vcvtpd2ps	xmm6{k7}, QWORD BCST [edx-1032]{1to4}	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, [eax]{1to2}	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, QWORD BCST [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm6{k7}, QWORD BCST [edx+1024]{1to2}	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, QWORD BCST [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm6{k7}, QWORD BCST [edx-1032]{1to2}	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, QWORD BCST [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm6{k7}, QWORD BCST [edx+1024]{1to4}	 # AVX512{F,VL}
	vcvtpd2udq	xmm6{k7}, QWORD BCST [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vcvtpd2udq	xmm6{k7}, QWORD BCST [edx-1032]{1to4}	 # AVX512{F,VL}
	vcvtph2ps	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcvtph2ps	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvtph2ps	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtph2ps	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtph2ps	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vcvtph2ps	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vcvtph2ps	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vcvtph2ps	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vcvtph2ps	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vcvtph2ps	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvtph2ps	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtph2ps	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtph2ps	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vcvtph2ps	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vcvtph2ps	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vcvtph2ps	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vcvtps2dq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcvtps2dq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvtps2dq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtps2dq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtps2dq	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vcvtps2dq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vcvtps2dq	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vcvtps2dq	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vcvtps2dq	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vcvtps2dq	xmm6{k7}, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvtps2dq	xmm6{k7}, [edx+512]{1to4}	 # AVX512{F,VL}
	vcvtps2dq	xmm6{k7}, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvtps2dq	xmm6{k7}, [edx-516]{1to4}	 # AVX512{F,VL}
	vcvtps2dq	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vcvtps2dq	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vcvtps2dq	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtps2dq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtps2dq	ymm6{k7}, [eax]{1to8}	 # AVX512{F,VL}
	vcvtps2dq	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vcvtps2dq	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vcvtps2dq	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vcvtps2dq	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vcvtps2dq	ymm6{k7}, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vcvtps2dq	ymm6{k7}, [edx+512]{1to8}	 # AVX512{F,VL}
	vcvtps2dq	ymm6{k7}, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vcvtps2dq	ymm6{k7}, [edx-516]{1to8}	 # AVX512{F,VL}
	vcvtps2pd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcvtps2pd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvtps2pd	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtps2pd	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtps2pd	xmm6{k7}, [eax]{1to2}	 # AVX512{F,VL}
	vcvtps2pd	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vcvtps2pd	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vcvtps2pd	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vcvtps2pd	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vcvtps2pd	xmm6{k7}, [edx+508]{1to2}	 # AVX512{F,VL} Disp8
	vcvtps2pd	xmm6{k7}, [edx+512]{1to2}	 # AVX512{F,VL}
	vcvtps2pd	xmm6{k7}, [edx-512]{1to2}	 # AVX512{F,VL} Disp8
	vcvtps2pd	xmm6{k7}, [edx-516]{1to2}	 # AVX512{F,VL}
	vcvtps2pd	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vcvtps2pd	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvtps2pd	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtps2pd	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtps2pd	ymm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vcvtps2pd	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vcvtps2pd	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vcvtps2pd	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vcvtps2pd	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vcvtps2pd	ymm6{k7}, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvtps2pd	ymm6{k7}, [edx+512]{1to4}	 # AVX512{F,VL}
	vcvtps2pd	ymm6{k7}, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvtps2pd	ymm6{k7}, [edx-516]{1to4}	 # AVX512{F,VL}
	vcvtps2ph	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vcvtps2ph	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vcvtps2ph	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vcvtps2ph	xmm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vcvtps2ph	xmm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vcvtps2ph	xmm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vcvtps2udq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcvtps2udq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvtps2udq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtps2udq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtps2udq	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vcvtps2udq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vcvtps2udq	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vcvtps2udq	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vcvtps2udq	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vcvtps2udq	xmm6{k7}, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvtps2udq	xmm6{k7}, [edx+512]{1to4}	 # AVX512{F,VL}
	vcvtps2udq	xmm6{k7}, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvtps2udq	xmm6{k7}, [edx-516]{1to4}	 # AVX512{F,VL}
	vcvtps2udq	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vcvtps2udq	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vcvtps2udq	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtps2udq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtps2udq	ymm6{k7}, [eax]{1to8}	 # AVX512{F,VL}
	vcvtps2udq	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vcvtps2udq	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vcvtps2udq	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vcvtps2udq	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vcvtps2udq	ymm6{k7}, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vcvtps2udq	ymm6{k7}, [edx+512]{1to8}	 # AVX512{F,VL}
	vcvtps2udq	ymm6{k7}, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vcvtps2udq	ymm6{k7}, [edx-516]{1to8}	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, [eax]{1to2}	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, QWORD BCST [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm6{k7}, QWORD BCST [edx+1024]{1to2}	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, QWORD BCST [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm6{k7}, QWORD BCST [edx-1032]{1to2}	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, QWORD BCST [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm6{k7}, QWORD BCST [edx+1024]{1to4}	 # AVX512{F,VL}
	vcvttpd2dq	xmm6{k7}, QWORD BCST [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vcvttpd2dq	xmm6{k7}, QWORD BCST [edx-1032]{1to4}	 # AVX512{F,VL}
	vcvttps2dq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcvttps2dq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvttps2dq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvttps2dq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvttps2dq	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vcvttps2dq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vcvttps2dq	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vcvttps2dq	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vcvttps2dq	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vcvttps2dq	xmm6{k7}, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvttps2dq	xmm6{k7}, [edx+512]{1to4}	 # AVX512{F,VL}
	vcvttps2dq	xmm6{k7}, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvttps2dq	xmm6{k7}, [edx-516]{1to4}	 # AVX512{F,VL}
	vcvttps2dq	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vcvttps2dq	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vcvttps2dq	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvttps2dq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvttps2dq	ymm6{k7}, [eax]{1to8}	 # AVX512{F,VL}
	vcvttps2dq	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vcvttps2dq	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vcvttps2dq	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vcvttps2dq	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vcvttps2dq	ymm6{k7}, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vcvttps2dq	ymm6{k7}, [edx+512]{1to8}	 # AVX512{F,VL}
	vcvttps2dq	ymm6{k7}, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vcvttps2dq	ymm6{k7}, [edx-516]{1to8}	 # AVX512{F,VL}
	vcvtudq2pd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcvtudq2pd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvtudq2pd	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtudq2pd	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtudq2pd	xmm6{k7}, [eax]{1to2}	 # AVX512{F,VL}
	vcvtudq2pd	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vcvtudq2pd	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vcvtudq2pd	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vcvtudq2pd	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vcvtudq2pd	xmm6{k7}, [edx+508]{1to2}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	xmm6{k7}, [edx+512]{1to2}	 # AVX512{F,VL}
	vcvtudq2pd	xmm6{k7}, [edx-512]{1to2}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	xmm6{k7}, [edx-516]{1to2}	 # AVX512{F,VL}
	vcvtudq2pd	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vcvtudq2pd	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvtudq2pd	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtudq2pd	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtudq2pd	ymm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vcvtudq2pd	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vcvtudq2pd	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vcvtudq2pd	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vcvtudq2pd	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vcvtudq2pd	ymm6{k7}, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	ymm6{k7}, [edx+512]{1to4}	 # AVX512{F,VL}
	vcvtudq2pd	ymm6{k7}, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvtudq2pd	ymm6{k7}, [edx-516]{1to4}	 # AVX512{F,VL}
	vcvtudq2ps	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcvtudq2ps	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvtudq2ps	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtudq2ps	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtudq2ps	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vcvtudq2ps	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vcvtudq2ps	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vcvtudq2ps	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vcvtudq2ps	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vcvtudq2ps	xmm6{k7}, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	xmm6{k7}, [edx+512]{1to4}	 # AVX512{F,VL}
	vcvtudq2ps	xmm6{k7}, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	xmm6{k7}, [edx-516]{1to4}	 # AVX512{F,VL}
	vcvtudq2ps	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vcvtudq2ps	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vcvtudq2ps	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvtudq2ps	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvtudq2ps	ymm6{k7}, [eax]{1to8}	 # AVX512{F,VL}
	vcvtudq2ps	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vcvtudq2ps	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vcvtudq2ps	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vcvtudq2ps	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vcvtudq2ps	ymm6{k7}, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	ymm6{k7}, [edx+512]{1to8}	 # AVX512{F,VL}
	vcvtudq2ps	ymm6{k7}, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vcvtudq2ps	ymm6{k7}, [edx-516]{1to8}	 # AVX512{F,VL}
	vdivpd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vdivpd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vdivpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vdivpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vdivpd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vdivpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vdivpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vdivpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vdivpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vdivpd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vdivpd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vdivpd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vdivpd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vdivpd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vdivpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vdivpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vdivpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vdivpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vdivpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vdivpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vdivpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vdivpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vdivpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vdivpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vdivpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vdivpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vdivps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vdivps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vdivps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vdivps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vdivps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vdivps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vdivps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vdivps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vdivps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vdivps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vdivps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vdivps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vdivps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vdivps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vdivps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vdivps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vdivps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vdivps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vdivps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vdivps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vdivps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vdivps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vdivps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vdivps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vdivps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vdivps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vexpandpd	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vexpandpd	xmm6{k7}{z}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vexpandpd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vexpandpd	xmm6{k7}, XMMWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vexpandpd	xmm6{k7}, XMMWORD PTR [edx+1024]	 # AVX512{F,VL}
	vexpandpd	xmm6{k7}, XMMWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vexpandpd	xmm6{k7}, XMMWORD PTR [edx-1032]	 # AVX512{F,VL}
	vexpandpd	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vexpandpd	ymm6{k7}{z}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vexpandpd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vexpandpd	ymm6{k7}, YMMWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vexpandpd	ymm6{k7}, YMMWORD PTR [edx+1024]	 # AVX512{F,VL}
	vexpandpd	ymm6{k7}, YMMWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vexpandpd	ymm6{k7}, YMMWORD PTR [edx-1032]	 # AVX512{F,VL}
	vexpandpd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vexpandpd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vexpandpd	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vexpandpd	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vexpandps	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vexpandps	xmm6{k7}{z}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vexpandps	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vexpandps	xmm6{k7}, XMMWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vexpandps	xmm6{k7}, XMMWORD PTR [edx+512]	 # AVX512{F,VL}
	vexpandps	xmm6{k7}, XMMWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vexpandps	xmm6{k7}, XMMWORD PTR [edx-516]	 # AVX512{F,VL}
	vexpandps	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vexpandps	ymm6{k7}{z}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vexpandps	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vexpandps	ymm6{k7}, YMMWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vexpandps	ymm6{k7}, YMMWORD PTR [edx+512]	 # AVX512{F,VL}
	vexpandps	ymm6{k7}, YMMWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vexpandps	ymm6{k7}, YMMWORD PTR [edx-516]	 # AVX512{F,VL}
	vexpandps	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vexpandps	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vexpandps	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vexpandps	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vextractf32x4	xmm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vextractf32x4	xmm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vextractf32x4	xmm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vextracti32x4	xmm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vextracti32x4	xmm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vextracti32x4	xmm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vfmadd132pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmadd132pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmadd132pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfmadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmadd132pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmadd132pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfmadd132pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmadd132pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfmadd132pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmadd132pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmadd132pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmadd132pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd132pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfmadd132pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd132pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfmadd132ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmadd132ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmadd132ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmadd132ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd132ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfmadd132ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd132ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfmadd132ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmadd132ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmadd132ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfmadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmadd132ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmadd132ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfmadd132ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmadd132ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfmadd213pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmadd213pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmadd213pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfmadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmadd213pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmadd213pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfmadd213pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmadd213pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfmadd213pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmadd213pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmadd213pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmadd213pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd213pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfmadd213pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd213pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfmadd213ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmadd213ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmadd213ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmadd213ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd213ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfmadd213ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd213ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfmadd213ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmadd213ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmadd213ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfmadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmadd213ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmadd213ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfmadd213ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmadd213ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfmadd231pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmadd231pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmadd231pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfmadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmadd231pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmadd231pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfmadd231pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmadd231pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfmadd231pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmadd231pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmadd231pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmadd231pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd231pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfmadd231pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd231pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfmadd231ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmadd231ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmadd231ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmadd231ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd231ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfmadd231ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmadd231ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfmadd231ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmadd231ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmadd231ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfmadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmadd231ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmadd231ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfmadd231ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmadd231ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfmaddsub132pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmaddsub132pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmaddsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmaddsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmaddsub132pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfmaddsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmaddsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmaddsub132pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfmaddsub132pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfmaddsub132pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmaddsub132pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmaddsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmaddsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmaddsub132pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmaddsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmaddsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmaddsub132pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfmaddsub132pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub132pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfmaddsub132ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmaddsub132ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmaddsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmaddsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmaddsub132ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmaddsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmaddsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmaddsub132ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfmaddsub132ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfmaddsub132ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmaddsub132ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmaddsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmaddsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmaddsub132ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfmaddsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmaddsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmaddsub132ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfmaddsub132ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmaddsub132ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfmaddsub213pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmaddsub213pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmaddsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmaddsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmaddsub213pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfmaddsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmaddsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmaddsub213pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfmaddsub213pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfmaddsub213pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmaddsub213pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmaddsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmaddsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmaddsub213pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmaddsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmaddsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmaddsub213pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfmaddsub213pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub213pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfmaddsub213ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmaddsub213ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmaddsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmaddsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmaddsub213ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmaddsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmaddsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmaddsub213ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfmaddsub213ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfmaddsub213ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmaddsub213ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmaddsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmaddsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmaddsub213ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfmaddsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmaddsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmaddsub213ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfmaddsub213ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmaddsub213ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfmaddsub231pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmaddsub231pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmaddsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmaddsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmaddsub231pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfmaddsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmaddsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmaddsub231pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfmaddsub231pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfmaddsub231pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmaddsub231pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmaddsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmaddsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmaddsub231pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmaddsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmaddsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmaddsub231pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfmaddsub231pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub231pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfmaddsub231ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmaddsub231ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmaddsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmaddsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmaddsub231ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmaddsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmaddsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmaddsub231ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfmaddsub231ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfmaddsub231ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmaddsub231ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmaddsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmaddsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmaddsub231ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfmaddsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmaddsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmaddsub231ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfmaddsub231ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmaddsub231ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfmsub132pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsub132pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsub132pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfmsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmsub132pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmsub132pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfmsub132pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmsub132pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfmsub132pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsub132pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsub132pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmsub132pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub132pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfmsub132pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub132pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfmsub132ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsub132ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsub132ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmsub132ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub132ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfmsub132ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub132ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfmsub132ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsub132ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsub132ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfmsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmsub132ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmsub132ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfmsub132ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmsub132ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfmsub213pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsub213pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsub213pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfmsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmsub213pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmsub213pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfmsub213pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmsub213pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfmsub213pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsub213pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsub213pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmsub213pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub213pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfmsub213pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub213pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfmsub213ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsub213ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsub213ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmsub213ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub213ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfmsub213ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub213ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfmsub213ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsub213ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsub213ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfmsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmsub213ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmsub213ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfmsub213ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmsub213ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfmsub231pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsub231pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsub231pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfmsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmsub231pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmsub231pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfmsub231pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmsub231pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfmsub231pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsub231pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsub231pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmsub231pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub231pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfmsub231pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub231pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfmsub231ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsub231ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsub231ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmsub231ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub231ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfmsub231ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmsub231ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfmsub231ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsub231ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsub231ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfmsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmsub231ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmsub231ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfmsub231ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmsub231ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfmsubadd132pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsubadd132pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsubadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsubadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsubadd132pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfmsubadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmsubadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmsubadd132pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfmsubadd132pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfmsubadd132pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsubadd132pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsubadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsubadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsubadd132pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmsubadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmsubadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmsubadd132pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfmsubadd132pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd132pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfmsubadd132ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsubadd132ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsubadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsubadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsubadd132ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmsubadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmsubadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmsubadd132ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfmsubadd132ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfmsubadd132ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsubadd132ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsubadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsubadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsubadd132ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfmsubadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmsubadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmsubadd132ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfmsubadd132ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmsubadd132ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfmsubadd213pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsubadd213pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsubadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsubadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsubadd213pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfmsubadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmsubadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmsubadd213pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfmsubadd213pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfmsubadd213pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsubadd213pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsubadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsubadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsubadd213pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmsubadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmsubadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmsubadd213pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfmsubadd213pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd213pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfmsubadd213ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsubadd213ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsubadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsubadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsubadd213ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmsubadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmsubadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmsubadd213ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfmsubadd213ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfmsubadd213ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsubadd213ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsubadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsubadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsubadd213ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfmsubadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmsubadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmsubadd213ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfmsubadd213ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmsubadd213ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfmsubadd231pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsubadd231pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsubadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsubadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsubadd231pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfmsubadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmsubadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmsubadd231pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfmsubadd231pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfmsubadd231pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsubadd231pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsubadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsubadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsubadd231pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmsubadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmsubadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmsubadd231pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfmsubadd231pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd231pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfmsubadd231ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsubadd231ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfmsubadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsubadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsubadd231ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfmsubadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfmsubadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfmsubadd231ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfmsubadd231ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfmsubadd231ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsubadd231ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfmsubadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfmsubadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfmsubadd231ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfmsubadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfmsubadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfmsubadd231ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfmsubadd231ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfmsubadd231ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfnmadd132pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmadd132pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmadd132pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfnmadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfnmadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfnmadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfnmadd132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfnmadd132pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfnmadd132pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfnmadd132pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmadd132pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmadd132pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfnmadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfnmadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfnmadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfnmadd132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfnmadd132pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfnmadd132pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd132pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfnmadd132ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmadd132ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmadd132ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfnmadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfnmadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfnmadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfnmadd132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfnmadd132ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfnmadd132ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfnmadd132ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmadd132ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmadd132ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfnmadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfnmadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfnmadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfnmadd132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfnmadd132ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfnmadd132ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfnmadd132ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfnmadd213pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmadd213pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmadd213pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfnmadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfnmadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfnmadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfnmadd213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfnmadd213pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfnmadd213pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfnmadd213pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmadd213pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmadd213pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfnmadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfnmadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfnmadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfnmadd213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfnmadd213pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfnmadd213pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd213pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfnmadd213ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmadd213ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmadd213ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfnmadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfnmadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfnmadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfnmadd213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfnmadd213ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfnmadd213ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfnmadd213ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmadd213ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmadd213ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfnmadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfnmadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfnmadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfnmadd213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfnmadd213ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfnmadd213ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfnmadd213ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfnmadd231pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmadd231pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmadd231pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfnmadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfnmadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfnmadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfnmadd231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfnmadd231pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfnmadd231pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfnmadd231pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmadd231pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmadd231pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfnmadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfnmadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfnmadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfnmadd231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfnmadd231pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfnmadd231pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd231pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfnmadd231ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmadd231ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmadd231ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfnmadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfnmadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfnmadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfnmadd231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfnmadd231ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfnmadd231ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfnmadd231ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmadd231ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmadd231ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfnmadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfnmadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfnmadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfnmadd231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfnmadd231ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfnmadd231ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfnmadd231ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfnmsub132pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmsub132pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmsub132pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfnmsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfnmsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfnmsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfnmsub132pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfnmsub132pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfnmsub132pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfnmsub132pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmsub132pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmsub132pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfnmsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfnmsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfnmsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfnmsub132pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfnmsub132pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfnmsub132pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub132pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfnmsub132ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmsub132ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmsub132ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfnmsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfnmsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfnmsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfnmsub132ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfnmsub132ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfnmsub132ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfnmsub132ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmsub132ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmsub132ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfnmsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfnmsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfnmsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfnmsub132ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfnmsub132ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfnmsub132ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfnmsub132ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfnmsub213pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmsub213pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmsub213pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfnmsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfnmsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfnmsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfnmsub213pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfnmsub213pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfnmsub213pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfnmsub213pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmsub213pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmsub213pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfnmsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfnmsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfnmsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfnmsub213pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfnmsub213pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfnmsub213pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub213pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfnmsub213ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmsub213ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmsub213ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfnmsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfnmsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfnmsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfnmsub213ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfnmsub213ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfnmsub213ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfnmsub213ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmsub213ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmsub213ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfnmsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfnmsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfnmsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfnmsub213ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfnmsub213ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfnmsub213ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfnmsub213ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfnmsub231pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmsub231pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmsub231pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vfnmsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfnmsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfnmsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfnmsub231pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfnmsub231pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vfnmsub231pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vfnmsub231pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmsub231pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmsub231pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vfnmsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfnmsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfnmsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfnmsub231pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfnmsub231pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vfnmsub231pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub231pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vfnmsub231ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmsub231ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vfnmsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmsub231ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vfnmsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vfnmsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vfnmsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vfnmsub231ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vfnmsub231ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vfnmsub231ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vfnmsub231ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmsub231ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vfnmsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vfnmsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vfnmsub231ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vfnmsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vfnmsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vfnmsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vfnmsub231ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vfnmsub231ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vfnmsub231ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vfnmsub231ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vgatherdpd	xmm6{k1}, [ebp+xmm7*8-123]	 # AVX512{F,VL}
	vgatherdpd	xmm6{k1}, [eax+xmm7+256]	 # AVX512{F,VL}
	vgatherdpd	xmm6{k1}, [ecx+xmm7*4+1024]	 # AVX512{F,VL}
	vgatherdpd	ymm6{k1}, [ebp+xmm7*8-123]	 # AVX512{F,VL}
	vgatherdpd	ymm6{k1}, [eax+xmm7+256]	 # AVX512{F,VL}
	vgatherdpd	ymm6{k1}, [ecx+xmm7*4+1024]	 # AVX512{F,VL}
	vgatherdps	xmm6{k1}, [ebp+xmm7*8-123]	 # AVX512{F,VL}
	vgatherdps	xmm6{k1}, [eax+xmm7+256]	 # AVX512{F,VL}
	vgatherdps	xmm6{k1}, [ecx+xmm7*4+1024]	 # AVX512{F,VL}
	vgatherdps	ymm6{k1}, [ebp+ymm7*8-123]	 # AVX512{F,VL}
	vgatherdps	ymm6{k1}, [eax+ymm7+256]	 # AVX512{F,VL}
	vgatherdps	ymm6{k1}, [ecx+ymm7*4+1024]	 # AVX512{F,VL}
	vgatherqpd	xmm6{k1}, [ebp+xmm7*8-123]	 # AVX512{F,VL}
	vgatherqpd	xmm6{k1}, [eax+xmm7+256]	 # AVX512{F,VL}
	vgatherqpd	xmm6{k1}, [ecx+xmm7*4+1024]	 # AVX512{F,VL}
	vgatherqpd	ymm6{k1}, [ebp+ymm7*8-123]	 # AVX512{F,VL}
	vgatherqpd	ymm6{k1}, [eax+ymm7+256]	 # AVX512{F,VL}
	vgatherqpd	ymm6{k1}, [ecx+ymm7*4+1024]	 # AVX512{F,VL}
	vgatherqps	xmm6{k1}, [ebp+xmm7*8-123]	 # AVX512{F,VL}
	vgatherqps	xmm6{k1}, [eax+xmm7+256]	 # AVX512{F,VL}
	vgatherqps	xmm6{k1}, [ecx+xmm7*4+1024]	 # AVX512{F,VL}
	vgatherqps	xmm6{k1}, [ebp+ymm7*8-123]	 # AVX512{F,VL}
	vgatherqps	xmm6{k1}, [eax+ymm7+256]	 # AVX512{F,VL}
	vgatherqps	xmm6{k1}, [ecx+ymm7*4+1024]	 # AVX512{F,VL}
	vgetexppd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vgetexppd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vgetexppd	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vgetexppd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vgetexppd	xmm6{k7}, [eax]{1to2}	 # AVX512{F,VL}
	vgetexppd	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vgetexppd	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vgetexppd	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vgetexppd	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vgetexppd	xmm6{k7}, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vgetexppd	xmm6{k7}, [edx+1024]{1to2}	 # AVX512{F,VL}
	vgetexppd	xmm6{k7}, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vgetexppd	xmm6{k7}, [edx-1032]{1to2}	 # AVX512{F,VL}
	vgetexppd	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vgetexppd	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vgetexppd	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vgetexppd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vgetexppd	ymm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vgetexppd	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vgetexppd	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vgetexppd	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vgetexppd	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vgetexppd	ymm6{k7}, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vgetexppd	ymm6{k7}, [edx+1024]{1to4}	 # AVX512{F,VL}
	vgetexppd	ymm6{k7}, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vgetexppd	ymm6{k7}, [edx-1032]{1to4}	 # AVX512{F,VL}
	vgetexpps	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vgetexpps	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vgetexpps	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vgetexpps	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vgetexpps	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vgetexpps	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vgetexpps	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vgetexpps	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vgetexpps	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vgetexpps	xmm6{k7}, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vgetexpps	xmm6{k7}, [edx+512]{1to4}	 # AVX512{F,VL}
	vgetexpps	xmm6{k7}, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vgetexpps	xmm6{k7}, [edx-516]{1to4}	 # AVX512{F,VL}
	vgetexpps	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vgetexpps	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vgetexpps	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vgetexpps	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vgetexpps	ymm6{k7}, [eax]{1to8}	 # AVX512{F,VL}
	vgetexpps	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vgetexpps	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vgetexpps	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vgetexpps	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vgetexpps	ymm6{k7}, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vgetexpps	ymm6{k7}, [edx+512]{1to8}	 # AVX512{F,VL}
	vgetexpps	ymm6{k7}, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vgetexpps	ymm6{k7}, [edx-516]{1to8}	 # AVX512{F,VL}
	vgetmantpd	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vgetmantpd	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vgetmantpd	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vgetmantpd	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vgetmantpd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vgetmantpd	xmm6{k7}, [eax]{1to2}, 123	 # AVX512{F,VL}
	vgetmantpd	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vgetmantpd	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vgetmantpd	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vgetmantpd	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vgetmantpd	xmm6{k7}, [edx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vgetmantpd	xmm6{k7}, [edx+1024]{1to2}, 123	 # AVX512{F,VL}
	vgetmantpd	xmm6{k7}, [edx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vgetmantpd	xmm6{k7}, [edx-1032]{1to2}, 123	 # AVX512{F,VL}
	vgetmantpd	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vgetmantpd	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vgetmantpd	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vgetmantpd	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vgetmantpd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vgetmantpd	ymm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vgetmantpd	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vgetmantpd	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vgetmantpd	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vgetmantpd	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vgetmantpd	ymm6{k7}, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vgetmantpd	ymm6{k7}, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vgetmantpd	ymm6{k7}, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vgetmantpd	ymm6{k7}, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vgetmantps	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vgetmantps	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vgetmantps	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vgetmantps	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vgetmantps	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vgetmantps	xmm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vgetmantps	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vgetmantps	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vgetmantps	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vgetmantps	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vgetmantps	xmm6{k7}, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vgetmantps	xmm6{k7}, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	vgetmantps	xmm6{k7}, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vgetmantps	xmm6{k7}, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	vgetmantps	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vgetmantps	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vgetmantps	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vgetmantps	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vgetmantps	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vgetmantps	ymm6{k7}, [eax]{1to8}, 123	 # AVX512{F,VL}
	vgetmantps	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vgetmantps	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vgetmantps	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vgetmantps	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vgetmantps	ymm6{k7}, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vgetmantps	ymm6{k7}, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vgetmantps	ymm6{k7}, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vgetmantps	ymm6{k7}, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vinsertf32x4	ymm6{k7}, ymm5, xmm4, 0xab	 # AVX512{F,VL}
	vinsertf32x4	ymm6{k7}{z}, ymm5, xmm4, 0xab	 # AVX512{F,VL}
	vinsertf32x4	ymm6{k7}, ymm5, xmm4, 123	 # AVX512{F,VL}
	vinsertf32x4	ymm6{k7}, ymm5, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vinsertf32x4	ymm6{k7}, ymm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vinsertf32x4	ymm6{k7}, ymm5, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vinsertf32x4	ymm6{k7}, ymm5, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vinsertf32x4	ymm6{k7}, ymm5, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vinsertf32x4	ymm6{k7}, ymm5, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vinserti32x4	ymm6{k7}, ymm5, xmm4, 0xab	 # AVX512{F,VL}
	vinserti32x4	ymm6{k7}{z}, ymm5, xmm4, 0xab	 # AVX512{F,VL}
	vinserti32x4	ymm6{k7}, ymm5, xmm4, 123	 # AVX512{F,VL}
	vinserti32x4	ymm6{k7}, ymm5, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vinserti32x4	ymm6{k7}, ymm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vinserti32x4	ymm6{k7}, ymm5, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vinserti32x4	ymm6{k7}, ymm5, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vinserti32x4	ymm6{k7}, ymm5, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vinserti32x4	ymm6{k7}, ymm5, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vmaxpd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vmaxpd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vmaxpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmaxpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmaxpd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vmaxpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vmaxpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vmaxpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vmaxpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vmaxpd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vmaxpd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vmaxpd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vmaxpd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vmaxpd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vmaxpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vmaxpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmaxpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmaxpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vmaxpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vmaxpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vmaxpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vmaxpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vmaxpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vmaxpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vmaxpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vmaxpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vmaxps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vmaxps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vmaxps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmaxps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmaxps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vmaxps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vmaxps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vmaxps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vmaxps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vmaxps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vmaxps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vmaxps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vmaxps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vmaxps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vmaxps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vmaxps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmaxps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmaxps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vmaxps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vmaxps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vmaxps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vmaxps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vmaxps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vmaxps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vmaxps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vmaxps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vminpd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vminpd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vminpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vminpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vminpd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vminpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vminpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vminpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vminpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vminpd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vminpd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vminpd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vminpd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vminpd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vminpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vminpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vminpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vminpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vminpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vminpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vminpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vminpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vminpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vminpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vminpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vminpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vminps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vminps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vminps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vminps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vminps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vminps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vminps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vminps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vminps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vminps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vminps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vminps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vminps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vminps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vminps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vminps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vminps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vminps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vminps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vminps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vminps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vminps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vminps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vminps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vminps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vminps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vmovapd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vmovapd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vmovapd	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovapd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovapd	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vmovapd	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vmovapd	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vmovapd	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vmovapd	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vmovapd	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vmovapd	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovapd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovapd	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vmovapd	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vmovapd	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vmovapd	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vmovaps	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vmovaps	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vmovaps	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovaps	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovaps	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vmovaps	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vmovaps	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vmovaps	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vmovaps	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vmovaps	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vmovaps	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovaps	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovaps	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vmovaps	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vmovaps	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vmovaps	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vmovddup	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vmovddup	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vmovddup	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vmovddup	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovddup	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vmovddup	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vmovddup	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vmovddup	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vmovddup	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vmovddup	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vmovddup	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovddup	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovddup	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vmovddup	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vmovddup	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vmovddup	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vmovdqa32	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vmovdqa32	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vmovdqa32	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovdqa32	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovdqa32	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vmovdqa32	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vmovdqa32	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vmovdqa32	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vmovdqa32	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vmovdqa32	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vmovdqa32	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovdqa32	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovdqa32	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vmovdqa32	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vmovdqa32	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vmovdqa32	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vmovdqa64	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vmovdqa64	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vmovdqa64	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovdqa64	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovdqa64	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vmovdqa64	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vmovdqa64	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vmovdqa64	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vmovdqa64	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vmovdqa64	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vmovdqa64	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovdqa64	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovdqa64	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vmovdqa64	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vmovdqa64	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vmovdqa64	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vmovdqu32	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vmovdqu32	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vmovdqu32	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovdqu32	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovdqu32	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vmovdqu32	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vmovdqu32	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vmovdqu32	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vmovdqu32	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vmovdqu32	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vmovdqu32	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovdqu32	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovdqu32	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vmovdqu32	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vmovdqu32	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vmovdqu32	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vmovdqu64	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vmovdqu64	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vmovdqu64	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovdqu64	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovdqu64	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vmovdqu64	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vmovdqu64	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vmovdqu64	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vmovdqu64	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vmovdqu64	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vmovdqu64	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovdqu64	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovdqu64	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vmovdqu64	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vmovdqu64	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vmovdqu64	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vmovshdup	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vmovshdup	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vmovshdup	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovshdup	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovshdup	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vmovshdup	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vmovshdup	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vmovshdup	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vmovshdup	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vmovshdup	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vmovshdup	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovshdup	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovshdup	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vmovshdup	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vmovshdup	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vmovshdup	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vmovsldup	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vmovsldup	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vmovsldup	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovsldup	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovsldup	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vmovsldup	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vmovsldup	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vmovsldup	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vmovsldup	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vmovsldup	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vmovsldup	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovsldup	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovsldup	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vmovsldup	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vmovsldup	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vmovsldup	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vmovupd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vmovupd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vmovupd	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovupd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovupd	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vmovupd	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vmovupd	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vmovupd	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vmovupd	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vmovupd	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vmovupd	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovupd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovupd	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vmovupd	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vmovupd	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vmovupd	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vmovups	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vmovups	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vmovups	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovups	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovups	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vmovups	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vmovups	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vmovups	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vmovups	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vmovups	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vmovups	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmovups	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmovups	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vmovups	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vmovups	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vmovups	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vmulpd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vmulpd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vmulpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmulpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmulpd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vmulpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vmulpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vmulpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vmulpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vmulpd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vmulpd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vmulpd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vmulpd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vmulpd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vmulpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vmulpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmulpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmulpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vmulpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vmulpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vmulpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vmulpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vmulpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vmulpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vmulpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vmulpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vmulps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vmulps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vmulps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmulps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmulps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vmulps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vmulps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vmulps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vmulps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vmulps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vmulps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vmulps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vmulps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vmulps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vmulps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vmulps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vmulps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vmulps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vmulps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vmulps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vmulps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vmulps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vmulps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vmulps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vmulps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vmulps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpabsd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpabsd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpabsd	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpabsd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpabsd	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vpabsd	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpabsd	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpabsd	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpabsd	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpabsd	xmm6{k7}, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpabsd	xmm6{k7}, [edx+512]{1to4}	 # AVX512{F,VL}
	vpabsd	xmm6{k7}, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpabsd	xmm6{k7}, [edx-516]{1to4}	 # AVX512{F,VL}
	vpabsd	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vpabsd	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpabsd	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpabsd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpabsd	ymm6{k7}, [eax]{1to8}	 # AVX512{F,VL}
	vpabsd	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpabsd	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpabsd	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpabsd	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpabsd	ymm6{k7}, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpabsd	ymm6{k7}, [edx+512]{1to8}	 # AVX512{F,VL}
	vpabsd	ymm6{k7}, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpabsd	ymm6{k7}, [edx-516]{1to8}	 # AVX512{F,VL}
	vpabsq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpabsq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpabsq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpabsq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpabsq	xmm6{k7}, [eax]{1to2}	 # AVX512{F,VL}
	vpabsq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpabsq	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpabsq	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpabsq	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpabsq	xmm6{k7}, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpabsq	xmm6{k7}, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpabsq	xmm6{k7}, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpabsq	xmm6{k7}, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpabsq	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vpabsq	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpabsq	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpabsq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpabsq	ymm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vpabsq	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpabsq	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpabsq	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpabsq	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpabsq	ymm6{k7}, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpabsq	ymm6{k7}, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpabsq	ymm6{k7}, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpabsq	ymm6{k7}, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpaddd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpaddd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpaddd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpaddd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpaddd	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpaddd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpaddd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpaddd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpaddd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpaddd	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpaddd	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpaddd	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpaddd	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpaddd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpaddd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpaddd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpaddd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpaddd	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpaddd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpaddd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpaddd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpaddd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpaddd	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpaddd	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpaddd	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpaddd	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpaddq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpaddq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpaddq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpaddq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpaddq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpaddq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpaddq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpaddq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpaddq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpaddq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpaddq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpaddq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpaddq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpaddq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpaddq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpaddq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpaddq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpaddq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpaddq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpaddq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpaddq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpaddq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpaddq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpaddq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpaddq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpaddq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpandd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpandd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpandd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpandd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpandd	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpandd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpandd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpandd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpandd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpandd	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpandd	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpandd	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpandd	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpandd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpandd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpandd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpandd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpandd	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpandd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpandd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpandd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpandd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpandd	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpandd	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpandd	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpandd	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpandnd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpandnd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpandnd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpandnd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpandnd	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpandnd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpandnd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpandnd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpandnd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpandnd	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpandnd	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpandnd	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpandnd	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpandnd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpandnd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpandnd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpandnd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpandnd	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpandnd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpandnd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpandnd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpandnd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpandnd	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpandnd	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpandnd	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpandnd	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpandnq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpandnq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpandnq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpandnq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpandnq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpandnq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpandnq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpandnq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpandnq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpandnq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpandnq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpandnq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpandnq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpandnq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpandnq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpandnq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpandnq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpandnq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpandnq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpandnq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpandnq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpandnq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpandnq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpandnq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpandnq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpandnq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpandq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpandq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpandq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpandq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpandq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpandq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpandq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpandq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpandq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpandq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpandq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpandq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpandq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpandq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpandq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpandq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpandq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpandq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpandq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpandq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpandq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpandq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpandq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpandq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpandq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpandq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpblendmd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpblendmd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpblendmd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpblendmd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpblendmd	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpblendmd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpblendmd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpblendmd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpblendmd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpblendmd	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpblendmd	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpblendmd	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpblendmd	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpblendmd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpblendmd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpblendmd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpblendmd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpblendmd	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpblendmd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpblendmd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpblendmd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpblendmd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpblendmd	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpblendmd	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpblendmd	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpblendmd	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpbroadcastd	xmm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpbroadcastd	xmm6{k7}{z}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpbroadcastd	xmm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpbroadcastd	xmm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpbroadcastd	xmm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vpbroadcastd	xmm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpbroadcastd	xmm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vpbroadcastd	ymm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpbroadcastd	ymm6{k7}{z}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpbroadcastd	ymm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpbroadcastd	ymm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpbroadcastd	ymm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vpbroadcastd	ymm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpbroadcastd	ymm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vpbroadcastd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpbroadcastd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpbroadcastd	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpbroadcastd	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpbroadcastd	xmm6{k7}, eax	 # AVX512{F,VL}
	vpbroadcastd	xmm6{k7}{z}, eax	 # AVX512{F,VL}
	vpbroadcastd	xmm6{k7}, ebp	 # AVX512{F,VL}
	vpbroadcastd	ymm6{k7}, eax	 # AVX512{F,VL}
	vpbroadcastd	ymm6{k7}{z}, eax	 # AVX512{F,VL}
	vpbroadcastd	ymm6{k7}, ebp	 # AVX512{F,VL}
	vpbroadcastq	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpbroadcastq	xmm6{k7}{z}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpbroadcastq	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpbroadcastq	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpbroadcastq	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpbroadcastq	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpbroadcastq	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpbroadcastq	ymm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpbroadcastq	ymm6{k7}{z}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpbroadcastq	ymm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpbroadcastq	ymm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpbroadcastq	ymm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpbroadcastq	ymm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpbroadcastq	ymm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpbroadcastq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpbroadcastq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpbroadcastq	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpbroadcastq	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpcmpd	k5{k7}, xmm6, xmm5, 0xab	 # AVX512{F,VL}
	vpcmpd	k5{k7}, xmm6, xmm5, 123	 # AVX512{F,VL}
	vpcmpd	k5{k7}, xmm6, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpcmpd	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpcmpd	k5{k7}, xmm6, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpcmpd	k5{k7}, xmm6, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5{k7}, xmm6, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vpcmpd	k5{k7}, xmm6, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5{k7}, xmm6, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vpcmpd	k5{k7}, xmm6, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5{k7}, xmm6, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	vpcmpd	k5{k7}, xmm6, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5{k7}, xmm6, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	vpcmpd	k5{k7}, ymm6, ymm5, 0xab	 # AVX512{F,VL}
	vpcmpd	k5{k7}, ymm6, ymm5, 123	 # AVX512{F,VL}
	vpcmpd	k5{k7}, ymm6, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpcmpd	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpcmpd	k5{k7}, ymm6, [eax]{1to8}, 123	 # AVX512{F,VL}
	vpcmpd	k5{k7}, ymm6, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5{k7}, ymm6, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpcmpd	k5{k7}, ymm6, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5{k7}, ymm6, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpcmpd	k5{k7}, ymm6, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5{k7}, ymm6, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vpcmpd	k5{k7}, ymm6, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpcmpd	k5{k7}, ymm6, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, xmm6, xmm5	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, xmm6, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, xmm6, [eax]{1to4}	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, xmm6, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5{k7}, xmm6, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, xmm6, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5{k7}, xmm6, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, xmm6, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5{k7}, xmm6, [edx+512]{1to4}	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, xmm6, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5{k7}, xmm6, [edx-516]{1to4}	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, ymm6, ymm5	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, ymm6, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, ymm6, [eax]{1to8}	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, ymm6, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5{k7}, ymm6, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, ymm6, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5{k7}, ymm6, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, ymm6, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5{k7}, ymm6, [edx+512]{1to8}	 # AVX512{F,VL}
	vpcmpeqd	k5{k7}, ymm6, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpcmpeqd	k5{k7}, ymm6, [edx-516]{1to8}	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, xmm6, xmm5	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, xmm6, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, xmm6, [eax]{1to2}	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, xmm6, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5{k7}, xmm6, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, xmm6, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5{k7}, xmm6, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, xmm6, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5{k7}, xmm6, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, xmm6, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5{k7}, xmm6, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, ymm6, ymm5	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, ymm6, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, ymm6, [eax]{1to4}	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, ymm6, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5{k7}, ymm6, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, ymm6, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5{k7}, ymm6, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, ymm6, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5{k7}, ymm6, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpcmpeqq	k5{k7}, ymm6, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpeqq	k5{k7}, ymm6, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, xmm6, xmm5	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, xmm6, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, xmm6, [eax]{1to4}	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, xmm6, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5{k7}, xmm6, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, xmm6, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5{k7}, xmm6, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, xmm6, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5{k7}, xmm6, [edx+512]{1to4}	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, xmm6, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5{k7}, xmm6, [edx-516]{1to4}	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, ymm6, ymm5	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, ymm6, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, ymm6, [eax]{1to8}	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, ymm6, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5{k7}, ymm6, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, ymm6, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5{k7}, ymm6, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, ymm6, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5{k7}, ymm6, [edx+512]{1to8}	 # AVX512{F,VL}
	vpcmpgtd	k5{k7}, ymm6, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpcmpgtd	k5{k7}, ymm6, [edx-516]{1to8}	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, xmm6, xmm5	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, xmm6, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, xmm6, [eax]{1to2}	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, xmm6, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5{k7}, xmm6, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, xmm6, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5{k7}, xmm6, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, xmm6, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5{k7}, xmm6, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, xmm6, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5{k7}, xmm6, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, ymm6, ymm5	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, ymm6, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, ymm6, [eax]{1to4}	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, ymm6, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5{k7}, ymm6, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, ymm6, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5{k7}, ymm6, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, ymm6, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5{k7}, ymm6, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpcmpgtq	k5{k7}, ymm6, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpcmpgtq	k5{k7}, ymm6, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpcmpq	k5{k7}, xmm6, xmm5, 0xab	 # AVX512{F,VL}
	vpcmpq	k5{k7}, xmm6, xmm5, 123	 # AVX512{F,VL}
	vpcmpq	k5{k7}, xmm6, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpcmpq	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpcmpq	k5{k7}, xmm6, [eax]{1to2}, 123	 # AVX512{F,VL}
	vpcmpq	k5{k7}, xmm6, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5{k7}, xmm6, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vpcmpq	k5{k7}, xmm6, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5{k7}, xmm6, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vpcmpq	k5{k7}, xmm6, [edx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5{k7}, xmm6, [edx+1024]{1to2}, 123	 # AVX512{F,VL}
	vpcmpq	k5{k7}, xmm6, [edx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5{k7}, xmm6, [edx-1032]{1to2}, 123	 # AVX512{F,VL}
	vpcmpq	k5{k7}, ymm6, ymm5, 0xab	 # AVX512{F,VL}
	vpcmpq	k5{k7}, ymm6, ymm5, 123	 # AVX512{F,VL}
	vpcmpq	k5{k7}, ymm6, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpcmpq	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpcmpq	k5{k7}, ymm6, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpcmpq	k5{k7}, ymm6, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5{k7}, ymm6, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpcmpq	k5{k7}, ymm6, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5{k7}, ymm6, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpcmpq	k5{k7}, ymm6, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5{k7}, ymm6, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpcmpq	k5{k7}, ymm6, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpq	k5{k7}, ymm6, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, xmm6, xmm5, 0xab	 # AVX512{F,VL}
	vpcmpud	k5{k7}, xmm6, xmm5, 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, xmm6, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, xmm6, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, xmm6, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5{k7}, xmm6, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, xmm6, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5{k7}, xmm6, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, xmm6, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5{k7}, xmm6, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, xmm6, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5{k7}, xmm6, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, ymm6, ymm5, 0xab	 # AVX512{F,VL}
	vpcmpud	k5{k7}, ymm6, ymm5, 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, ymm6, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, ymm6, [eax]{1to8}, 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, ymm6, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5{k7}, ymm6, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, ymm6, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5{k7}, ymm6, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, ymm6, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5{k7}, ymm6, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vpcmpud	k5{k7}, ymm6, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpcmpud	k5{k7}, ymm6, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, xmm6, xmm5, 0xab	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, xmm6, xmm5, 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, xmm6, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, xmm6, [eax]{1to2}, 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, xmm6, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5{k7}, xmm6, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, xmm6, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5{k7}, xmm6, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, xmm6, [edx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5{k7}, xmm6, [edx+1024]{1to2}, 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, xmm6, [edx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5{k7}, xmm6, [edx-1032]{1to2}, 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, ymm6, ymm5, 0xab	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, ymm6, ymm5, 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, ymm6, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, ymm6, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, ymm6, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5{k7}, ymm6, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, ymm6, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5{k7}, ymm6, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, ymm6, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5{k7}, ymm6, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpcmpuq	k5{k7}, ymm6, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpcmpuq	k5{k7}, ymm6, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpblendmq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpblendmq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpblendmq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpblendmq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpblendmq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpblendmq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpblendmq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpblendmq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpblendmq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpblendmq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpblendmq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpblendmq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpblendmq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpblendmq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpblendmq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpblendmq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpblendmq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpblendmq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpblendmq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpblendmq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpblendmq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpblendmq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpblendmq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpblendmq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpblendmq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpblendmq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpcompressd	XMMWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpcompressd	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpcompressd	XMMWORD PTR [edx+508]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpcompressd	XMMWORD PTR [edx+512]{k7}, xmm6	 # AVX512{F,VL}
	vpcompressd	XMMWORD PTR [edx-512]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpcompressd	XMMWORD PTR [edx-516]{k7}, xmm6	 # AVX512{F,VL}
	vpcompressd	YMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpcompressd	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpcompressd	YMMWORD PTR [edx+508]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpcompressd	YMMWORD PTR [edx+512]{k7}, ymm6	 # AVX512{F,VL}
	vpcompressd	YMMWORD PTR [edx-512]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpcompressd	YMMWORD PTR [edx-516]{k7}, ymm6	 # AVX512{F,VL}
	vpcompressd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpcompressd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpcompressd	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vpcompressd	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpermd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpermd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpermd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermd	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpermd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpermd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpermd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpermd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpermd	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpermd	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpermd	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpermd	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, [eax]{1to2}, 123	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vpermilpd	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vpermilpd	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, [edx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpermilpd	xmm6{k7}, [edx+1024]{1to2}, 123	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, [edx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpermilpd	xmm6{k7}, [edx-1032]{1to2}, 123	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpermilpd	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpermilpd	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermilpd	ymm6{k7}, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermilpd	ymm6{k7}, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpermilpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpermilpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpermilpd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpermilpd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpermilpd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpermilpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpermilpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpermilpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpermilpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpermilpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vpermilps	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vpermilps	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vpermilps	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermilps	xmm6{k7}, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermilps	xmm6{k7}, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vpermilps	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, [eax]{1to8}, 123	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpermilps	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpermilps	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpermilps	ymm6{k7}, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpermilps	ymm6{k7}, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpermilps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpermilps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpermilps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpermilps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpermilps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpermilps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpermilps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpermilps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpermilps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpermilps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpermilps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpermilps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vpermpd	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpermpd	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpermpd	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermpd	ymm6{k7}, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermpd	ymm6{k7}, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpermps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpermps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpermps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpermps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpermps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpermps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpermps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpermps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpermps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpermps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpermps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpermq	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vpermq	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vpermq	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vpermq	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpermq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpermq	ymm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpermq	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpermq	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpermq	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpermq	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpermq	ymm6{k7}, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermq	ymm6{k7}, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpermq	ymm6{k7}, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpermq	ymm6{k7}, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpexpandd	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpexpandd	xmm6{k7}{z}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpexpandd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpexpandd	xmm6{k7}, XMMWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpexpandd	xmm6{k7}, XMMWORD PTR [edx+512]	 # AVX512{F,VL}
	vpexpandd	xmm6{k7}, XMMWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpexpandd	xmm6{k7}, XMMWORD PTR [edx-516]	 # AVX512{F,VL}
	vpexpandd	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpexpandd	ymm6{k7}{z}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpexpandd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpexpandd	ymm6{k7}, YMMWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpexpandd	ymm6{k7}, YMMWORD PTR [edx+512]	 # AVX512{F,VL}
	vpexpandd	ymm6{k7}, YMMWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpexpandd	ymm6{k7}, YMMWORD PTR [edx-516]	 # AVX512{F,VL}
	vpexpandd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpexpandd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpexpandd	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vpexpandd	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpexpandq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpexpandq	xmm6{k7}{z}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpexpandq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpexpandq	xmm6{k7}, XMMWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpexpandq	xmm6{k7}, XMMWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpexpandq	xmm6{k7}, XMMWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpexpandq	xmm6{k7}, XMMWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpexpandq	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpexpandq	ymm6{k7}{z}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpexpandq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpexpandq	ymm6{k7}, YMMWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpexpandq	ymm6{k7}, YMMWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpexpandq	ymm6{k7}, YMMWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpexpandq	ymm6{k7}, YMMWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpexpandq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpexpandq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpexpandq	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vpexpandq	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpgatherdd	xmm6{k1}, [ebp+xmm7*8-123]	 # AVX512{F,VL}
	vpgatherdd	xmm6{k1}, [eax+xmm7+256]	 # AVX512{F,VL}
	vpgatherdd	xmm6{k1}, [ecx+xmm7*4+1024]	 # AVX512{F,VL}
	vpgatherdd	ymm6{k1}, [ebp+ymm7*8-123]	 # AVX512{F,VL}
	vpgatherdd	ymm6{k1}, [eax+ymm7+256]	 # AVX512{F,VL}
	vpgatherdd	ymm6{k1}, [ecx+ymm7*4+1024]	 # AVX512{F,VL}
	vpgatherdq	xmm6{k1}, [ebp+xmm7*8-123]	 # AVX512{F,VL}
	vpgatherdq	xmm6{k1}, [eax+xmm7+256]	 # AVX512{F,VL}
	vpgatherdq	xmm6{k1}, [ecx+xmm7*4+1024]	 # AVX512{F,VL}
	vpgatherdq	ymm6{k1}, [ebp+xmm7*8-123]	 # AVX512{F,VL}
	vpgatherdq	ymm6{k1}, [eax+xmm7+256]	 # AVX512{F,VL}
	vpgatherdq	ymm6{k1}, [ecx+xmm7*4+1024]	 # AVX512{F,VL}
	vpgatherqd	xmm6{k1}, [ebp+xmm7*8-123]	 # AVX512{F,VL}
	vpgatherqd	xmm6{k1}, [eax+xmm7+256]	 # AVX512{F,VL}
	vpgatherqd	xmm6{k1}, [ecx+xmm7*4+1024]	 # AVX512{F,VL}
	vpgatherqd	xmm6{k1}, [ebp+ymm7*8-123]	 # AVX512{F,VL}
	vpgatherqd	xmm6{k1}, [eax+ymm7+256]	 # AVX512{F,VL}
	vpgatherqd	xmm6{k1}, [ecx+ymm7*4+1024]	 # AVX512{F,VL}
	vpgatherqq	xmm6{k1}, [ebp+xmm7*8-123]	 # AVX512{F,VL}
	vpgatherqq	xmm6{k1}, [eax+xmm7+256]	 # AVX512{F,VL}
	vpgatherqq	xmm6{k1}, [ecx+xmm7*4+1024]	 # AVX512{F,VL}
	vpgatherqq	ymm6{k1}, [ebp+ymm7*8-123]	 # AVX512{F,VL}
	vpgatherqq	ymm6{k1}, [eax+ymm7+256]	 # AVX512{F,VL}
	vpgatherqq	ymm6{k1}, [ecx+ymm7*4+1024]	 # AVX512{F,VL}
	vpmaxsd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpmaxsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpmaxsd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmaxsd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmaxsd	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpmaxsd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpmaxsd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpmaxsd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpmaxsd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpmaxsd	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxsd	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpmaxsd	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxsd	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpmaxsd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpmaxsd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpmaxsd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmaxsd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmaxsd	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpmaxsd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpmaxsd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpmaxsd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpmaxsd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpmaxsd	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpmaxsd	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpmaxsd	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpmaxsd	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpmaxsq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpmaxsq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpmaxsq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmaxsq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmaxsq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpmaxsq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpmaxsq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpmaxsq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpmaxsq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpmaxsq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpmaxsq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpmaxsq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpmaxsq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpmaxsq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpmaxsq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpmaxsq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmaxsq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmaxsq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpmaxsq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpmaxsq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpmaxsq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpmaxsq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpmaxsq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxsq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpmaxsq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxsq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpmaxud	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpmaxud	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpmaxud	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmaxud	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmaxud	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpmaxud	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpmaxud	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpmaxud	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpmaxud	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpmaxud	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxud	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpmaxud	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxud	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpmaxud	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpmaxud	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpmaxud	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmaxud	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmaxud	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpmaxud	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpmaxud	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpmaxud	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpmaxud	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpmaxud	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpmaxud	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpmaxud	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpmaxud	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpmaxuq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpmaxuq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpmaxuq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmaxuq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmaxuq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpmaxuq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpmaxuq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpmaxuq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpmaxuq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpmaxuq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpmaxuq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpmaxuq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpmaxuq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpmaxuq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpmaxuq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpmaxuq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmaxuq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmaxuq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpmaxuq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpmaxuq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpmaxuq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpmaxuq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpmaxuq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxuq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpmaxuq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpmaxuq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpminsd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpminsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpminsd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpminsd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpminsd	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpminsd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpminsd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpminsd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpminsd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpminsd	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpminsd	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpminsd	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpminsd	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpminsd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpminsd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpminsd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpminsd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpminsd	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpminsd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpminsd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpminsd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpminsd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpminsd	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpminsd	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpminsd	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpminsd	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpminsq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpminsq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpminsq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpminsq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpminsq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpminsq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpminsq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpminsq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpminsq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpminsq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpminsq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpminsq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpminsq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpminsq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpminsq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpminsq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpminsq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpminsq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpminsq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpminsq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpminsq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpminsq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpminsq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpminsq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpminsq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpminsq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpminud	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpminud	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpminud	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpminud	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpminud	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpminud	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpminud	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpminud	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpminud	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpminud	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpminud	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpminud	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpminud	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpminud	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpminud	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpminud	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpminud	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpminud	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpminud	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpminud	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpminud	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpminud	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpminud	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpminud	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpminud	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpminud	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpminuq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpminuq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpminuq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpminuq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpminuq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpminuq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpminuq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpminuq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpminuq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpminuq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpminuq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpminuq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpminuq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpminuq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpminuq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpminuq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpminuq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpminuq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpminuq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpminuq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpminuq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpminuq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpminuq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpminuq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpminuq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpminuq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpmovsxbd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxbd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxbd	xmm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxbd	xmm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxbd	xmm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpmovsxbd	xmm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vpmovsxbd	xmm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpmovsxbd	xmm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vpmovsxbd	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxbd	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxbd	ymm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxbd	ymm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxbd	ymm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpmovsxbd	ymm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpmovsxbd	ymm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpmovsxbd	ymm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpmovsxbq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxbq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxbq	xmm6{k7}, WORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxbq	xmm6{k7}, WORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxbq	xmm6{k7}, WORD PTR [edx+254]	 # AVX512{F,VL} Disp8
	vpmovsxbq	xmm6{k7}, WORD PTR [edx+256]	 # AVX512{F,VL}
	vpmovsxbq	xmm6{k7}, WORD PTR [edx-256]	 # AVX512{F,VL} Disp8
	vpmovsxbq	xmm6{k7}, WORD PTR [edx-258]	 # AVX512{F,VL}
	vpmovsxbq	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxbq	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxbq	ymm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxbq	ymm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxbq	ymm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpmovsxbq	ymm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vpmovsxbq	ymm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpmovsxbq	ymm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vpmovsxdq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxdq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxdq	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxdq	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxdq	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpmovsxdq	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpmovsxdq	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpmovsxdq	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpmovsxdq	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxdq	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxdq	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxdq	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxdq	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpmovsxdq	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpmovsxdq	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpmovsxdq	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpmovsxwd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxwd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxwd	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxwd	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxwd	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpmovsxwd	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpmovsxwd	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpmovsxwd	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpmovsxwd	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxwd	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxwd	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxwd	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxwd	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpmovsxwd	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpmovsxwd	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpmovsxwd	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpmovsxwq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxwq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxwq	xmm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxwq	xmm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxwq	xmm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpmovsxwq	xmm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vpmovsxwq	xmm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpmovsxwq	xmm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vpmovsxwq	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsxwq	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsxwq	ymm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovsxwq	ymm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovsxwq	ymm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpmovsxwq	ymm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpmovsxwq	ymm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpmovsxwq	ymm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpmovzxbd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxbd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxbd	xmm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxbd	xmm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxbd	xmm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpmovzxbd	xmm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vpmovzxbd	xmm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpmovzxbd	xmm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vpmovzxbd	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxbd	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxbd	ymm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxbd	ymm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxbd	ymm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpmovzxbd	ymm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpmovzxbd	ymm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpmovzxbd	ymm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpmovzxbq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxbq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxbq	xmm6{k7}, WORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxbq	xmm6{k7}, WORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxbq	xmm6{k7}, WORD PTR [edx+254]	 # AVX512{F,VL} Disp8
	vpmovzxbq	xmm6{k7}, WORD PTR [edx+256]	 # AVX512{F,VL}
	vpmovzxbq	xmm6{k7}, WORD PTR [edx-256]	 # AVX512{F,VL} Disp8
	vpmovzxbq	xmm6{k7}, WORD PTR [edx-258]	 # AVX512{F,VL}
	vpmovzxbq	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxbq	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxbq	ymm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxbq	ymm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxbq	ymm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpmovzxbq	ymm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vpmovzxbq	ymm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpmovzxbq	ymm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vpmovzxdq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxdq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxdq	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxdq	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxdq	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpmovzxdq	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpmovzxdq	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpmovzxdq	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpmovzxdq	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxdq	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxdq	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxdq	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxdq	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpmovzxdq	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpmovzxdq	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpmovzxdq	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpmovzxwd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxwd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxwd	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxwd	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxwd	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpmovzxwd	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpmovzxwd	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpmovzxwd	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpmovzxwd	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxwd	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxwd	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxwd	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxwd	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpmovzxwd	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpmovzxwd	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpmovzxwd	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpmovzxwq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxwq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxwq	xmm6{k7}, DWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxwq	xmm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxwq	xmm6{k7}, DWORD PTR [edx+508]	 # AVX512{F,VL} Disp8
	vpmovzxwq	xmm6{k7}, DWORD PTR [edx+512]	 # AVX512{F,VL}
	vpmovzxwq	xmm6{k7}, DWORD PTR [edx-512]	 # AVX512{F,VL} Disp8
	vpmovzxwq	xmm6{k7}, DWORD PTR [edx-516]	 # AVX512{F,VL}
	vpmovzxwq	ymm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovzxwq	ymm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovzxwq	ymm6{k7}, QWORD PTR [ecx]	 # AVX512{F,VL}
	vpmovzxwq	ymm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmovzxwq	ymm6{k7}, QWORD PTR [edx+1016]	 # AVX512{F,VL} Disp8
	vpmovzxwq	ymm6{k7}, QWORD PTR [edx+1024]	 # AVX512{F,VL}
	vpmovzxwq	ymm6{k7}, QWORD PTR [edx-1024]	 # AVX512{F,VL} Disp8
	vpmovzxwq	ymm6{k7}, QWORD PTR [edx-1032]	 # AVX512{F,VL}
	vpmuldq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpmuldq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpmuldq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmuldq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmuldq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpmuldq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpmuldq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpmuldq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpmuldq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpmuldq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpmuldq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpmuldq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpmuldq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpmuldq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpmuldq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpmuldq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmuldq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmuldq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpmuldq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpmuldq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpmuldq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpmuldq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpmuldq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpmuldq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpmuldq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpmuldq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpmulld	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpmulld	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpmulld	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmulld	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmulld	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpmulld	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpmulld	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpmulld	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpmulld	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpmulld	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpmulld	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpmulld	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpmulld	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpmulld	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpmulld	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpmulld	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmulld	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmulld	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpmulld	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpmulld	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpmulld	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpmulld	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpmulld	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpmulld	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpmulld	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpmulld	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpmuludq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpmuludq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpmuludq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmuludq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmuludq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpmuludq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpmuludq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpmuludq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpmuludq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpmuludq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpmuludq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpmuludq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpmuludq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpmuludq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpmuludq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpmuludq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpmuludq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpmuludq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpmuludq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpmuludq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpmuludq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpmuludq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpmuludq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpmuludq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpmuludq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpmuludq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpord	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpord	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpord	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpord	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpord	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpord	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpord	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpord	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpord	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpord	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpord	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpord	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpord	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpord	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpord	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpord	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpord	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpord	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpord	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpord	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpord	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpord	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpord	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpord	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpord	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpord	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vporq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vporq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vporq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vporq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vporq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vporq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vporq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vporq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vporq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vporq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vporq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vporq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vporq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vporq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vporq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vporq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vporq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vporq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vporq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vporq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vporq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vporq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vporq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vporq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vporq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vporq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpscatterdd	[ebp+xmm7*8-123]{k1}, xmm6	 # AVX512{F,VL}
	vpscatterdd	[eax+xmm7+256]{k1}, xmm6	 # AVX512{F,VL}
	vpscatterdd	[ecx+xmm7*4+1024]{k1}, xmm6	 # AVX512{F,VL}
	vpscatterdd	[ebp+ymm7*8-123]{k1}, ymm6	 # AVX512{F,VL}
	vpscatterdd	[eax+ymm7+256]{k1}, ymm6	 # AVX512{F,VL}
	vpscatterdd	[ecx+ymm7*4+1024]{k1}, ymm6	 # AVX512{F,VL}
	vpscatterdq	[ebp+xmm7*8-123]{k1}, xmm6	 # AVX512{F,VL}
	vpscatterdq	[eax+xmm7+256]{k1}, xmm6	 # AVX512{F,VL}
	vpscatterdq	[ecx+xmm7*4+1024]{k1}, xmm6	 # AVX512{F,VL}
	vpscatterdq	[ebp+xmm7*8-123]{k1}, ymm6	 # AVX512{F,VL}
	vpscatterdq	[eax+xmm7+256]{k1}, ymm6	 # AVX512{F,VL}
	vpscatterdq	[ecx+xmm7*4+1024]{k1}, ymm6	 # AVX512{F,VL}
	vpscatterqd	[ebp+xmm7*8-123]{k1}, xmm6	 # AVX512{F,VL}
	vpscatterqd	[eax+xmm7+256]{k1}, xmm6	 # AVX512{F,VL}
	vpscatterqd	[ecx+xmm7*4+1024]{k1}, xmm6	 # AVX512{F,VL}
	vpscatterqd	[ebp+ymm7*8-123]{k1}, xmm6	 # AVX512{F,VL}
	vpscatterqd	[eax+ymm7+256]{k1}, xmm6	 # AVX512{F,VL}
	vpscatterqd	[ecx+ymm7*4+1024]{k1}, xmm6	 # AVX512{F,VL}
	vpscatterqq	[ebp+xmm7*8-123]{k1}, xmm6	 # AVX512{F,VL}
	vpscatterqq	[eax+xmm7+256]{k1}, xmm6	 # AVX512{F,VL}
	vpscatterqq	[ecx+xmm7*4+1024]{k1}, xmm6	 # AVX512{F,VL}
	vpscatterqq	[ebp+ymm7*8-123]{k1}, ymm6	 # AVX512{F,VL}
	vpscatterqq	[eax+ymm7+256]{k1}, ymm6	 # AVX512{F,VL}
	vpscatterqq	[ecx+ymm7*4+1024]{k1}, ymm6	 # AVX512{F,VL}
	vpshufd	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vpshufd	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vpshufd	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vpshufd	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpshufd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpshufd	xmm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpshufd	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vpshufd	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vpshufd	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vpshufd	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vpshufd	xmm6{k7}, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpshufd	xmm6{k7}, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	vpshufd	xmm6{k7}, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpshufd	xmm6{k7}, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	vpshufd	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vpshufd	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vpshufd	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vpshufd	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpshufd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpshufd	ymm6{k7}, [eax]{1to8}, 123	 # AVX512{F,VL}
	vpshufd	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpshufd	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpshufd	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpshufd	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpshufd	ymm6{k7}, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpshufd	ymm6{k7}, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vpshufd	ymm6{k7}, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpshufd	ymm6{k7}, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vpslld	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpslld	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpslld	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpslld	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpslld	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpslld	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpslld	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpslld	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpslld	ymm6{k7}, ymm5, xmm4	 # AVX512{F,VL}
	vpslld	ymm6{k7}{z}, ymm5, xmm4	 # AVX512{F,VL}
	vpslld	ymm6{k7}, ymm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpslld	ymm6{k7}, ymm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpslld	ymm6{k7}, ymm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpslld	ymm6{k7}, ymm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpslld	ymm6{k7}, ymm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpslld	ymm6{k7}, ymm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsllq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpsllq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpsllq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsllq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsllq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsllq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsllq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsllq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsllq	ymm6{k7}, ymm5, xmm4	 # AVX512{F,VL}
	vpsllq	ymm6{k7}{z}, ymm5, xmm4	 # AVX512{F,VL}
	vpsllq	ymm6{k7}, ymm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsllq	ymm6{k7}, ymm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsllq	ymm6{k7}, ymm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsllq	ymm6{k7}, ymm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsllq	ymm6{k7}, ymm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsllq	ymm6{k7}, ymm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsllvd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpsllvd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpsllvd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsllvd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsllvd	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpsllvd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsllvd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsllvd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsllvd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsllvd	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpsllvd	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpsllvd	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpsllvd	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpsllvd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpsllvd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpsllvd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsllvd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsllvd	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpsllvd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpsllvd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpsllvd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpsllvd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpsllvd	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpsllvd	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpsllvd	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpsllvd	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpsllvq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpsllvq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpsllvq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsllvq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsllvq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpsllvq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsllvq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsllvq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsllvq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsllvq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpsllvq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpsllvq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpsllvq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpsllvq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpsllvq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpsllvq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsllvq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsllvq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpsllvq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpsllvq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpsllvq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpsllvq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpsllvq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpsllvq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpsllvq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpsllvq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpsrad	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpsrad	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpsrad	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsrad	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsrad	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsrad	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsrad	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsrad	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsrad	ymm6{k7}, ymm5, xmm4	 # AVX512{F,VL}
	vpsrad	ymm6{k7}{z}, ymm5, xmm4	 # AVX512{F,VL}
	vpsrad	ymm6{k7}, ymm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsrad	ymm6{k7}, ymm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsrad	ymm6{k7}, ymm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsrad	ymm6{k7}, ymm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsrad	ymm6{k7}, ymm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsrad	ymm6{k7}, ymm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsraq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpsraq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpsraq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsraq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsraq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsraq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsraq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsraq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsraq	ymm6{k7}, ymm5, xmm4	 # AVX512{F,VL}
	vpsraq	ymm6{k7}{z}, ymm5, xmm4	 # AVX512{F,VL}
	vpsraq	ymm6{k7}, ymm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsraq	ymm6{k7}, ymm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsraq	ymm6{k7}, ymm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsraq	ymm6{k7}, ymm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsraq	ymm6{k7}, ymm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsraq	ymm6{k7}, ymm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsravd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpsravd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpsravd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsravd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsravd	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpsravd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsravd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsravd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsravd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsravd	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpsravd	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpsravd	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpsravd	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpsravd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpsravd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpsravd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsravd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsravd	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpsravd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpsravd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpsravd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpsravd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpsravd	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpsravd	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpsravd	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpsravd	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpsravq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpsravq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpsravq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsravq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsravq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpsravq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsravq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsravq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsravq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsravq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpsravq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpsravq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpsravq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpsravq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpsravq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpsravq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsravq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsravq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpsravq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpsravq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpsravq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpsravq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpsravq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpsravq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpsravq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpsravq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpsrld	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpsrld	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpsrld	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsrld	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsrld	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsrld	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsrld	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsrld	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsrld	ymm6{k7}, ymm5, xmm4	 # AVX512{F,VL}
	vpsrld	ymm6{k7}{z}, ymm5, xmm4	 # AVX512{F,VL}
	vpsrld	ymm6{k7}, ymm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsrld	ymm6{k7}, ymm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsrld	ymm6{k7}, ymm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsrld	ymm6{k7}, ymm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsrld	ymm6{k7}, ymm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsrld	ymm6{k7}, ymm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsrlq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsrlq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}, ymm5, xmm4	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}{z}, ymm5, xmm4	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}, ymm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}, ymm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}, ymm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsrlq	ymm6{k7}, ymm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}, ymm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsrlq	ymm6{k7}, ymm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsrlvd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpsrlvd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpsrlvd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsrlvd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsrlvd	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpsrlvd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsrlvd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsrlvd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsrlvd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsrlvd	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpsrlvd	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpsrlvd	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpsrlvd	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpsrlvd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpsrlvd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpsrlvd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsrlvd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsrlvd	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpsrlvd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpsrlvd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpsrlvd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpsrlvd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpsrlvd	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpsrlvd	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpsrlvd	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpsrlvd	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpsrlvq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpsrlvq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpsrlvq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsrlvq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsrlvq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpsrlvq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsrlvq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsrlvq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsrlvq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsrlvq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpsrlvq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpsrlvq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpsrlvq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpsrlvq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpsrlvq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpsrlvq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsrlvq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsrlvq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpsrlvq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpsrlvq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpsrlvq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpsrlvq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpsrlvq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpsrlvq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpsrlvq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpsrlvq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpsrld	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vpsrld	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vpsrld	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vpsrld	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpsrld	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpsrld	xmm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpsrld	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vpsrld	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vpsrld	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vpsrld	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vpsrld	xmm6{k7}, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsrld	xmm6{k7}, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	vpsrld	xmm6{k7}, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsrld	xmm6{k7}, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	vpsrld	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vpsrld	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vpsrld	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vpsrld	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpsrld	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpsrld	ymm6{k7}, [eax]{1to8}, 123	 # AVX512{F,VL}
	vpsrld	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpsrld	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpsrld	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpsrld	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpsrld	ymm6{k7}, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpsrld	ymm6{k7}, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vpsrld	ymm6{k7}, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpsrld	ymm6{k7}, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}, [eax]{1to2}, 123	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vpsrlq	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vpsrlq	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}, [edx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpsrlq	xmm6{k7}, [edx+1024]{1to2}, 123	 # AVX512{F,VL}
	vpsrlq	xmm6{k7}, [edx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpsrlq	xmm6{k7}, [edx-1032]{1to2}, 123	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpsrlq	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpsrlq	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsrlq	ymm6{k7}, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpsrlq	ymm6{k7}, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsrlq	ymm6{k7}, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpsubd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpsubd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpsubd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsubd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsubd	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpsubd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsubd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsubd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsubd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsubd	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpsubd	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpsubd	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpsubd	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpsubd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpsubd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpsubd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsubd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsubd	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpsubd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpsubd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpsubd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpsubd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpsubd	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpsubd	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpsubd	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpsubd	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpsubq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpsubq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpsubq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsubq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsubq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpsubq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpsubq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpsubq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpsubq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpsubq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpsubq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpsubq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpsubq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpsubq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpsubq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpsubq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpsubq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpsubq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpsubq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpsubq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpsubq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpsubq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpsubq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpsubq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpsubq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpsubq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vptestmd	k5{k7}, xmm6, xmm5	 # AVX512{F,VL}
	vptestmd	k5{k7}, xmm6, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vptestmd	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vptestmd	k5{k7}, xmm6, [eax]{1to4}	 # AVX512{F,VL}
	vptestmd	k5{k7}, xmm6, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vptestmd	k5{k7}, xmm6, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vptestmd	k5{k7}, xmm6, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vptestmd	k5{k7}, xmm6, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vptestmd	k5{k7}, xmm6, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vptestmd	k5{k7}, xmm6, [edx+512]{1to4}	 # AVX512{F,VL}
	vptestmd	k5{k7}, xmm6, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vptestmd	k5{k7}, xmm6, [edx-516]{1to4}	 # AVX512{F,VL}
	vptestmd	k5{k7}, ymm6, ymm5	 # AVX512{F,VL}
	vptestmd	k5{k7}, ymm6, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vptestmd	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vptestmd	k5{k7}, ymm6, [eax]{1to8}	 # AVX512{F,VL}
	vptestmd	k5{k7}, ymm6, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vptestmd	k5{k7}, ymm6, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vptestmd	k5{k7}, ymm6, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vptestmd	k5{k7}, ymm6, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vptestmd	k5{k7}, ymm6, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vptestmd	k5{k7}, ymm6, [edx+512]{1to8}	 # AVX512{F,VL}
	vptestmd	k5{k7}, ymm6, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vptestmd	k5{k7}, ymm6, [edx-516]{1to8}	 # AVX512{F,VL}
	vptestmq	k5{k7}, xmm6, xmm5	 # AVX512{F,VL}
	vptestmq	k5{k7}, xmm6, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vptestmq	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vptestmq	k5{k7}, xmm6, [eax]{1to2}	 # AVX512{F,VL}
	vptestmq	k5{k7}, xmm6, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vptestmq	k5{k7}, xmm6, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vptestmq	k5{k7}, xmm6, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vptestmq	k5{k7}, xmm6, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vptestmq	k5{k7}, xmm6, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vptestmq	k5{k7}, xmm6, [edx+1024]{1to2}	 # AVX512{F,VL}
	vptestmq	k5{k7}, xmm6, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vptestmq	k5{k7}, xmm6, [edx-1032]{1to2}	 # AVX512{F,VL}
	vptestmq	k5{k7}, ymm6, ymm5	 # AVX512{F,VL}
	vptestmq	k5{k7}, ymm6, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vptestmq	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vptestmq	k5{k7}, ymm6, [eax]{1to4}	 # AVX512{F,VL}
	vptestmq	k5{k7}, ymm6, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vptestmq	k5{k7}, ymm6, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vptestmq	k5{k7}, ymm6, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vptestmq	k5{k7}, ymm6, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vptestmq	k5{k7}, ymm6, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vptestmq	k5{k7}, ymm6, [edx+1024]{1to4}	 # AVX512{F,VL}
	vptestmq	k5{k7}, ymm6, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vptestmq	k5{k7}, ymm6, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpunpckhdq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpunpckhdq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpunpckhdq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpunpckhdq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpunpckhdq	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpunpckhdq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpunpckhdq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpunpckhdq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpunpckhdq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpunpckhdq	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpunpckhdq	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpunpckhdq	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpunpckhdq	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpunpckhdq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpunpckhdq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpunpckhdq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpunpckhdq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpunpckhdq	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpunpckhdq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpunpckhdq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpunpckhdq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpunpckhdq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpunpckhdq	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpunpckhdq	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpunpckhdq	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpunpckhdq	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpunpckhqdq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpunpckhqdq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpunpckhqdq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpunpckhqdq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpunpckhqdq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpunpckhqdq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpunpckhqdq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpunpckhqdq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpunpckhqdq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpunpckhqdq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpunpckhqdq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpunpckhqdq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpunpckhqdq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpunpckhqdq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpunpckhqdq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpunpckhqdq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpunpckhqdq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpunpckhqdq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpunpckhqdq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpunpckhqdq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpunpckhqdq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpunpckhqdq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpunpckhqdq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpunpckldq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpunpckldq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpunpckldq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpunpckldq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpunpckldq	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpunpckldq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpunpckldq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpunpckldq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpunpckldq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpunpckldq	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpunpckldq	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpunpckldq	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpunpckldq	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpunpckldq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpunpckldq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpunpckldq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpunpckldq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpunpckldq	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpunpckldq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpunpckldq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpunpckldq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpunpckldq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpunpckldq	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpunpckldq	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpunpckldq	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpunpckldq	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpunpcklqdq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpunpcklqdq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpunpcklqdq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpunpcklqdq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpunpcklqdq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpunpcklqdq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpunpcklqdq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpunpcklqdq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpunpcklqdq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpunpcklqdq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpunpcklqdq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpunpcklqdq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpunpcklqdq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpunpcklqdq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpunpcklqdq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpunpcklqdq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpunpcklqdq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpunpcklqdq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpunpcklqdq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpunpcklqdq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpunpcklqdq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpunpcklqdq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpunpcklqdq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpxord	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpxord	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpxord	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpxord	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpxord	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpxord	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpxord	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpxord	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpxord	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpxord	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpxord	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpxord	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpxord	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpxord	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpxord	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpxord	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpxord	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpxord	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpxord	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpxord	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpxord	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpxord	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpxord	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpxord	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpxord	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpxord	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpxorq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpxorq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpxorq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpxorq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpxorq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpxorq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpxorq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpxorq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpxorq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpxorq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpxorq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpxorq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpxorq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpxorq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpxorq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpxorq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpxorq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpxorq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpxorq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpxorq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpxorq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpxorq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpxorq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpxorq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpxorq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpxorq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vrcp14pd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vrcp14pd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vrcp14pd	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vrcp14pd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vrcp14pd	xmm6{k7}, [eax]{1to2}	 # AVX512{F,VL}
	vrcp14pd	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vrcp14pd	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vrcp14pd	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vrcp14pd	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vrcp14pd	xmm6{k7}, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vrcp14pd	xmm6{k7}, [edx+1024]{1to2}	 # AVX512{F,VL}
	vrcp14pd	xmm6{k7}, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vrcp14pd	xmm6{k7}, [edx-1032]{1to2}	 # AVX512{F,VL}
	vrcp14pd	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vrcp14pd	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vrcp14pd	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vrcp14pd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vrcp14pd	ymm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vrcp14pd	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vrcp14pd	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vrcp14pd	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vrcp14pd	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vrcp14pd	ymm6{k7}, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vrcp14pd	ymm6{k7}, [edx+1024]{1to4}	 # AVX512{F,VL}
	vrcp14pd	ymm6{k7}, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vrcp14pd	ymm6{k7}, [edx-1032]{1to4}	 # AVX512{F,VL}
	vrcp14ps	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vrcp14ps	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vrcp14ps	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vrcp14ps	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vrcp14ps	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vrcp14ps	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vrcp14ps	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vrcp14ps	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vrcp14ps	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vrcp14ps	xmm6{k7}, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vrcp14ps	xmm6{k7}, [edx+512]{1to4}	 # AVX512{F,VL}
	vrcp14ps	xmm6{k7}, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vrcp14ps	xmm6{k7}, [edx-516]{1to4}	 # AVX512{F,VL}
	vrcp14ps	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vrcp14ps	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vrcp14ps	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vrcp14ps	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vrcp14ps	ymm6{k7}, [eax]{1to8}	 # AVX512{F,VL}
	vrcp14ps	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vrcp14ps	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vrcp14ps	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vrcp14ps	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vrcp14ps	ymm6{k7}, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vrcp14ps	ymm6{k7}, [edx+512]{1to8}	 # AVX512{F,VL}
	vrcp14ps	ymm6{k7}, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vrcp14ps	ymm6{k7}, [edx-516]{1to8}	 # AVX512{F,VL}
	vrsqrt14pd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vrsqrt14pd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vrsqrt14pd	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vrsqrt14pd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vrsqrt14pd	xmm6{k7}, [eax]{1to2}	 # AVX512{F,VL}
	vrsqrt14pd	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vrsqrt14pd	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vrsqrt14pd	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vrsqrt14pd	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vrsqrt14pd	xmm6{k7}, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	xmm6{k7}, [edx+1024]{1to2}	 # AVX512{F,VL}
	vrsqrt14pd	xmm6{k7}, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	xmm6{k7}, [edx-1032]{1to2}	 # AVX512{F,VL}
	vrsqrt14pd	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vrsqrt14pd	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vrsqrt14pd	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vrsqrt14pd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vrsqrt14pd	ymm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vrsqrt14pd	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vrsqrt14pd	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vrsqrt14pd	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vrsqrt14pd	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vrsqrt14pd	ymm6{k7}, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	ymm6{k7}, [edx+1024]{1to4}	 # AVX512{F,VL}
	vrsqrt14pd	ymm6{k7}, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vrsqrt14pd	ymm6{k7}, [edx-1032]{1to4}	 # AVX512{F,VL}
	vrsqrt14ps	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vrsqrt14ps	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vrsqrt14ps	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vrsqrt14ps	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vrsqrt14ps	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vrsqrt14ps	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vrsqrt14ps	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vrsqrt14ps	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vrsqrt14ps	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vrsqrt14ps	xmm6{k7}, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	xmm6{k7}, [edx+512]{1to4}	 # AVX512{F,VL}
	vrsqrt14ps	xmm6{k7}, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	xmm6{k7}, [edx-516]{1to4}	 # AVX512{F,VL}
	vrsqrt14ps	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vrsqrt14ps	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vrsqrt14ps	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vrsqrt14ps	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vrsqrt14ps	ymm6{k7}, [eax]{1to8}	 # AVX512{F,VL}
	vrsqrt14ps	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vrsqrt14ps	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vrsqrt14ps	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vrsqrt14ps	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vrsqrt14ps	ymm6{k7}, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	ymm6{k7}, [edx+512]{1to8}	 # AVX512{F,VL}
	vrsqrt14ps	ymm6{k7}, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vrsqrt14ps	ymm6{k7}, [edx-516]{1to8}	 # AVX512{F,VL}
	vscatterdpd	[ebp+xmm7*8-123]{k1}, xmm6	 # AVX512{F,VL}
	vscatterdpd	[eax+xmm7+256]{k1}, xmm6	 # AVX512{F,VL}
	vscatterdpd	[ecx+xmm7*4+1024]{k1}, xmm6	 # AVX512{F,VL}
	vscatterdpd	[ebp+xmm7*8-123]{k1}, ymm6	 # AVX512{F,VL}
	vscatterdpd	[eax+xmm7+256]{k1}, ymm6	 # AVX512{F,VL}
	vscatterdpd	[ecx+xmm7*4+1024]{k1}, ymm6	 # AVX512{F,VL}
	vscatterdps	[ebp+xmm7*8-123]{k1}, xmm6	 # AVX512{F,VL}
	vscatterdps	[eax+xmm7+256]{k1}, xmm6	 # AVX512{F,VL}
	vscatterdps	[ecx+xmm7*4+1024]{k1}, xmm6	 # AVX512{F,VL}
	vscatterdps	[ebp+ymm7*8-123]{k1}, ymm6	 # AVX512{F,VL}
	vscatterdps	[eax+ymm7+256]{k1}, ymm6	 # AVX512{F,VL}
	vscatterdps	[ecx+ymm7*4+1024]{k1}, ymm6	 # AVX512{F,VL}
	vscatterqpd	[ebp+xmm7*8-123]{k1}, xmm6	 # AVX512{F,VL}
	vscatterqpd	[eax+xmm7+256]{k1}, xmm6	 # AVX512{F,VL}
	vscatterqpd	[ecx+xmm7*4+1024]{k1}, xmm6	 # AVX512{F,VL}
	vscatterqpd	[ebp+ymm7*8-123]{k1}, ymm6	 # AVX512{F,VL}
	vscatterqpd	[eax+ymm7+256]{k1}, ymm6	 # AVX512{F,VL}
	vscatterqpd	[ecx+ymm7*4+1024]{k1}, ymm6	 # AVX512{F,VL}
	vscatterqps	[ebp+xmm7*8-123]{k1}, xmm6	 # AVX512{F,VL}
	vscatterqps	[eax+xmm7+256]{k1}, xmm6	 # AVX512{F,VL}
	vscatterqps	[ecx+xmm7*4+1024]{k1}, xmm6	 # AVX512{F,VL}
	vscatterqps	[ebp+ymm7*8-123]{k1}, xmm6	 # AVX512{F,VL}
	vscatterqps	[eax+ymm7+256]{k1}, xmm6	 # AVX512{F,VL}
	vscatterqps	[ecx+ymm7*4+1024]{k1}, xmm6	 # AVX512{F,VL}
	vshufpd	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	vshufpd	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	vshufpd	xmm6{k7}, xmm5, xmm4, 123	 # AVX512{F,VL}
	vshufpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vshufpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vshufpd	xmm6{k7}, xmm5, [eax]{1to2}, 123	 # AVX512{F,VL}
	vshufpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vshufpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vshufpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vshufpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vshufpd	xmm6{k7}, xmm5, [edx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vshufpd	xmm6{k7}, xmm5, [edx+1024]{1to2}, 123	 # AVX512{F,VL}
	vshufpd	xmm6{k7}, xmm5, [edx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vshufpd	xmm6{k7}, xmm5, [edx-1032]{1to2}, 123	 # AVX512{F,VL}
	vshufpd	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vshufpd	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vshufpd	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{F,VL}
	vshufpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vshufpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vshufpd	ymm6{k7}, ymm5, [eax]{1to4}, 123	 # AVX512{F,VL}
	vshufpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vshufpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vshufpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vshufpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vshufpd	ymm6{k7}, ymm5, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshufpd	ymm6{k7}, ymm5, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vshufpd	ymm6{k7}, ymm5, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshufpd	ymm6{k7}, ymm5, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vshufps	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	vshufps	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	vshufps	xmm6{k7}, xmm5, xmm4, 123	 # AVX512{F,VL}
	vshufps	xmm6{k7}, xmm5, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vshufps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vshufps	xmm6{k7}, xmm5, [eax]{1to4}, 123	 # AVX512{F,VL}
	vshufps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vshufps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vshufps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vshufps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vshufps	xmm6{k7}, xmm5, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshufps	xmm6{k7}, xmm5, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	vshufps	xmm6{k7}, xmm5, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshufps	xmm6{k7}, xmm5, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	vshufps	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vshufps	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vshufps	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{F,VL}
	vshufps	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vshufps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vshufps	ymm6{k7}, ymm5, [eax]{1to8}, 123	 # AVX512{F,VL}
	vshufps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vshufps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vshufps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vshufps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vshufps	ymm6{k7}, ymm5, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vshufps	ymm6{k7}, ymm5, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vshufps	ymm6{k7}, ymm5, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vshufps	ymm6{k7}, ymm5, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vsqrtpd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vsqrtpd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vsqrtpd	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vsqrtpd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vsqrtpd	xmm6{k7}, [eax]{1to2}	 # AVX512{F,VL}
	vsqrtpd	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vsqrtpd	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vsqrtpd	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vsqrtpd	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vsqrtpd	xmm6{k7}, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vsqrtpd	xmm6{k7}, [edx+1024]{1to2}	 # AVX512{F,VL}
	vsqrtpd	xmm6{k7}, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vsqrtpd	xmm6{k7}, [edx-1032]{1to2}	 # AVX512{F,VL}
	vsqrtpd	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vsqrtpd	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vsqrtpd	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vsqrtpd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vsqrtpd	ymm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vsqrtpd	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vsqrtpd	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vsqrtpd	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vsqrtpd	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vsqrtpd	ymm6{k7}, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vsqrtpd	ymm6{k7}, [edx+1024]{1to4}	 # AVX512{F,VL}
	vsqrtpd	ymm6{k7}, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vsqrtpd	ymm6{k7}, [edx-1032]{1to4}	 # AVX512{F,VL}
	vsqrtps	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vsqrtps	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vsqrtps	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vsqrtps	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vsqrtps	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vsqrtps	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vsqrtps	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vsqrtps	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vsqrtps	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vsqrtps	xmm6{k7}, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vsqrtps	xmm6{k7}, [edx+512]{1to4}	 # AVX512{F,VL}
	vsqrtps	xmm6{k7}, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vsqrtps	xmm6{k7}, [edx-516]{1to4}	 # AVX512{F,VL}
	vsqrtps	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vsqrtps	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vsqrtps	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vsqrtps	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vsqrtps	ymm6{k7}, [eax]{1to8}	 # AVX512{F,VL}
	vsqrtps	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vsqrtps	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vsqrtps	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vsqrtps	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vsqrtps	ymm6{k7}, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vsqrtps	ymm6{k7}, [edx+512]{1to8}	 # AVX512{F,VL}
	vsqrtps	ymm6{k7}, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vsqrtps	ymm6{k7}, [edx-516]{1to8}	 # AVX512{F,VL}
	vsubpd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vsubpd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vsubpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vsubpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vsubpd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vsubpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vsubpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vsubpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vsubpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vsubpd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vsubpd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vsubpd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vsubpd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vsubpd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vsubpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vsubpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vsubpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vsubpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vsubpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vsubpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vsubpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vsubpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vsubpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vsubpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vsubpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vsubpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vsubps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vsubps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vsubps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vsubps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vsubps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vsubps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vsubps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vsubps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vsubps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vsubps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vsubps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vsubps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vsubps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vsubps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vsubps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vsubps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vsubps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vsubps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vsubps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vsubps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vsubps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vsubps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vsubps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vsubps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vsubps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vsubps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vunpckhpd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vunpckhpd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vunpckhpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vunpckhpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vunpckhpd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vunpckhpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vunpckhpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vunpckhpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vunpckhpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vunpckhpd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vunpckhpd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vunpckhpd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vunpckhpd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vunpckhpd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vunpckhpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vunpckhpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vunpckhpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vunpckhpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vunpckhpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vunpckhpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vunpckhpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vunpckhpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vunpckhpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vunpckhpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vunpckhpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vunpckhpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vunpckhps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vunpckhps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vunpckhps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vunpckhps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vunpckhps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vunpckhps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vunpckhps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vunpckhps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vunpckhps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vunpckhps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vunpckhps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vunpckhps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vunpckhps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vunpckhps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vunpckhps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vunpckhps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vunpckhps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vunpckhps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vunpckhps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vunpckhps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vunpckhps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vunpckhps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vunpckhps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vunpckhps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vunpckhps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vunpckhps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vunpcklpd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vunpcklpd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vunpcklpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vunpcklpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vunpcklpd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vunpcklpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vunpcklpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vunpcklpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vunpcklpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vunpcklpd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vunpcklpd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vunpcklpd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vunpcklpd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vunpcklpd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vunpcklpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vunpcklpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vunpcklpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vunpcklpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vunpcklpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vunpcklpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vunpcklpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vunpcklpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vunpcklpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vunpcklpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vunpcklpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vunpcklpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vunpcklps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vunpcklps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vunpcklps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vunpcklps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vunpcklps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vunpcklps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vunpcklps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vunpcklps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vunpcklps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vunpcklps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vunpcklps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vunpcklps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vunpcklps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vunpcklps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vunpcklps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vunpcklps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vunpcklps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vunpcklps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vunpcklps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vunpcklps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vunpcklps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vunpcklps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vunpcklps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vunpcklps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vunpcklps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vunpcklps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpternlogd	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	vpternlogd	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	vpternlogd	xmm6{k7}, xmm5, xmm4, 123	 # AVX512{F,VL}
	vpternlogd	xmm6{k7}, xmm5, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpternlogd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpternlogd	xmm6{k7}, xmm5, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpternlogd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vpternlogd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vpternlogd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vpternlogd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vpternlogd	xmm6{k7}, xmm5, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpternlogd	xmm6{k7}, xmm5, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	vpternlogd	xmm6{k7}, xmm5, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpternlogd	xmm6{k7}, xmm5, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	vpternlogd	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vpternlogd	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vpternlogd	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{F,VL}
	vpternlogd	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpternlogd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpternlogd	ymm6{k7}, ymm5, [eax]{1to8}, 123	 # AVX512{F,VL}
	vpternlogd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpternlogd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpternlogd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpternlogd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpternlogd	ymm6{k7}, ymm5, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpternlogd	ymm6{k7}, ymm5, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vpternlogd	ymm6{k7}, ymm5, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpternlogd	ymm6{k7}, ymm5, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vpternlogq	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	vpternlogq	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	vpternlogq	xmm6{k7}, xmm5, xmm4, 123	 # AVX512{F,VL}
	vpternlogq	xmm6{k7}, xmm5, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpternlogq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpternlogq	xmm6{k7}, xmm5, [eax]{1to2}, 123	 # AVX512{F,VL}
	vpternlogq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vpternlogq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vpternlogq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vpternlogq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vpternlogq	xmm6{k7}, xmm5, [edx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpternlogq	xmm6{k7}, xmm5, [edx+1024]{1to2}, 123	 # AVX512{F,VL}
	vpternlogq	xmm6{k7}, xmm5, [edx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpternlogq	xmm6{k7}, xmm5, [edx-1032]{1to2}, 123	 # AVX512{F,VL}
	vpternlogq	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vpternlogq	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vpternlogq	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{F,VL}
	vpternlogq	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpternlogq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpternlogq	ymm6{k7}, ymm5, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpternlogq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpternlogq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpternlogq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpternlogq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpternlogq	ymm6{k7}, ymm5, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpternlogq	ymm6{k7}, ymm5, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpternlogq	ymm6{k7}, ymm5, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpternlogq	ymm6{k7}, ymm5, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpmovqb	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovqb	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovqb	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vpmovqb	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpmovsqb	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsqb	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsqb	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vpmovsqb	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpmovusqb	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovusqb	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovusqb	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vpmovusqb	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpmovqw	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovqw	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovqw	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vpmovqw	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpmovsqw	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsqw	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsqw	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vpmovsqw	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpmovusqw	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovusqw	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovusqw	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vpmovusqw	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpmovqd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovqd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovqd	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vpmovqd	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpmovsqd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsqd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsqd	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vpmovsqd	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpmovusqd	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovusqd	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovusqd	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vpmovusqd	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpmovdb	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovdb	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovdb	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vpmovdb	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpmovsdb	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsdb	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsdb	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vpmovsdb	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpmovusdb	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovusdb	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovusdb	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vpmovusdb	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpmovdw	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovdw	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovdw	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vpmovdw	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpmovsdw	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovsdw	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovsdw	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vpmovsdw	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vpmovusdw	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpmovusdw	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpmovusdw	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vpmovusdw	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vshuff32x4	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vshuff32x4	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vshuff32x4	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{F,VL}
	vshuff32x4	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vshuff32x4	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vshuff32x4	ymm6{k7}, ymm5, [eax]{1to8}, 123	 # AVX512{F,VL}
	vshuff32x4	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vshuff32x4	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vshuff32x4	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vshuff32x4	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vshuff32x4	ymm6{k7}, ymm5, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vshuff32x4	ymm6{k7}, ymm5, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vshuff32x4	ymm6{k7}, ymm5, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vshuff32x4	ymm6{k7}, ymm5, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vshuff64x2	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vshuff64x2	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vshuff64x2	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{F,VL}
	vshuff64x2	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vshuff64x2	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vshuff64x2	ymm6{k7}, ymm5, [eax]{1to4}, 123	 # AVX512{F,VL}
	vshuff64x2	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vshuff64x2	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vshuff64x2	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vshuff64x2	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vshuff64x2	ymm6{k7}, ymm5, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshuff64x2	ymm6{k7}, ymm5, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vshuff64x2	ymm6{k7}, ymm5, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshuff64x2	ymm6{k7}, ymm5, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vshufi32x4	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vshufi32x4	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vshufi32x4	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{F,VL}
	vshufi32x4	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vshufi32x4	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vshufi32x4	ymm6{k7}, ymm5, [eax]{1to8}, 123	 # AVX512{F,VL}
	vshufi32x4	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vshufi32x4	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vshufi32x4	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vshufi32x4	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vshufi32x4	ymm6{k7}, ymm5, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vshufi32x4	ymm6{k7}, ymm5, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vshufi32x4	ymm6{k7}, ymm5, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vshufi32x4	ymm6{k7}, ymm5, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vshufi64x2	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vshufi64x2	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vshufi64x2	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{F,VL}
	vshufi64x2	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vshufi64x2	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vshufi64x2	ymm6{k7}, ymm5, [eax]{1to4}, 123	 # AVX512{F,VL}
	vshufi64x2	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vshufi64x2	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vshufi64x2	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vshufi64x2	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vshufi64x2	ymm6{k7}, ymm5, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshufi64x2	ymm6{k7}, ymm5, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vshufi64x2	ymm6{k7}, ymm5, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vshufi64x2	ymm6{k7}, ymm5, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpermq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpermq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpermq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpermq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpermq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpermq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpermq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpermq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpermq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpermq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpermq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpermpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpermpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpermpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpermpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpermpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpermpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpermt2d	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpermt2d	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpermt2d	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermt2d	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermt2d	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpermt2d	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpermt2d	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpermt2d	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpermt2d	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpermt2d	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2d	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpermt2d	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2d	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpermt2d	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpermt2d	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpermt2d	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermt2d	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermt2d	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpermt2d	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpermt2d	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpermt2d	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpermt2d	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpermt2d	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpermt2d	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpermt2d	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpermt2d	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpermt2q	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpermt2q	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpermt2q	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermt2q	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermt2q	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpermt2q	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpermt2q	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpermt2q	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpermt2q	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpermt2q	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpermt2q	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpermt2q	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpermt2q	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpermt2q	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpermt2q	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpermt2q	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermt2q	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermt2q	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpermt2q	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpermt2q	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpermt2q	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpermt2q	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpermt2q	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2q	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpermt2q	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2q	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpermt2ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpermt2ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpermt2ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermt2ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermt2ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpermt2ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpermt2ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpermt2ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpermt2ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpermt2ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpermt2ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpermt2ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpermt2ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpermt2ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermt2ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermt2ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpermt2ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpermt2ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpermt2ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpermt2ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpermt2ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpermt2ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpermt2ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpermt2ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpermt2pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpermt2pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpermt2pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermt2pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermt2pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpermt2pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpermt2pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpermt2pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpermt2pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpermt2pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpermt2pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpermt2pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpermt2pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpermt2pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpermt2pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpermt2pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermt2pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermt2pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpermt2pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpermt2pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpermt2pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpermt2pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpermt2pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpermt2pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpermt2pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	valignq	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	valignq	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	valignq	xmm6{k7}, xmm5, xmm4, 123	 # AVX512{F,VL}
	valignq	xmm6{k7}, xmm5, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	valignq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	valignq	xmm6{k7}, xmm5, [eax]{1to2}, 123	 # AVX512{F,VL}
	valignq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	valignq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	valignq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	valignq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	valignq	xmm6{k7}, xmm5, [edx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	valignq	xmm6{k7}, xmm5, [edx+1024]{1to2}, 123	 # AVX512{F,VL}
	valignq	xmm6{k7}, xmm5, [edx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	valignq	xmm6{k7}, xmm5, [edx-1032]{1to2}, 123	 # AVX512{F,VL}
	valignq	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	valignq	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	valignq	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{F,VL}
	valignq	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	valignq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	valignq	ymm6{k7}, ymm5, [eax]{1to4}, 123	 # AVX512{F,VL}
	valignq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	valignq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	valignq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	valignq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	valignq	ymm6{k7}, ymm5, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	valignq	ymm6{k7}, ymm5, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	valignq	ymm6{k7}, ymm5, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	valignq	ymm6{k7}, ymm5, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vscalefpd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vscalefpd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vscalefpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vscalefpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vscalefpd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vscalefpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vscalefpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vscalefpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vscalefpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vscalefpd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vscalefpd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vscalefpd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vscalefpd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vscalefpd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vscalefpd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vscalefpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vscalefpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vscalefpd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vscalefpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vscalefpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vscalefpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vscalefpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vscalefpd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vscalefpd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vscalefpd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vscalefpd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vscalefps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vscalefps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vscalefps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vscalefps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vscalefps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vscalefps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vscalefps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vscalefps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vscalefps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vscalefps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vscalefps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vscalefps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vscalefps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vscalefps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vscalefps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vscalefps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vscalefps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vscalefps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vscalefps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vscalefps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vscalefps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vscalefps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vscalefps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vscalefps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vscalefps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vscalefps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vfixupimmpd	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	vfixupimmpd	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	vfixupimmpd	xmm6{k7}, xmm5, xmm4, 123	 # AVX512{F,VL}
	vfixupimmpd	xmm6{k7}, xmm5, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vfixupimmpd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vfixupimmpd	xmm6{k7}, xmm5, [eax]{1to2}, 123	 # AVX512{F,VL}
	vfixupimmpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vfixupimmpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vfixupimmpd	xmm6{k7}, xmm5, [edx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	xmm6{k7}, xmm5, [edx+1024]{1to2}, 123	 # AVX512{F,VL}
	vfixupimmpd	xmm6{k7}, xmm5, [edx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	xmm6{k7}, xmm5, [edx-1032]{1to2}, 123	 # AVX512{F,VL}
	vfixupimmpd	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vfixupimmpd	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vfixupimmpd	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{F,VL}
	vfixupimmpd	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vfixupimmpd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vfixupimmpd	ymm6{k7}, ymm5, [eax]{1to4}, 123	 # AVX512{F,VL}
	vfixupimmpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vfixupimmpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vfixupimmpd	ymm6{k7}, ymm5, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	ymm6{k7}, ymm5, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vfixupimmpd	ymm6{k7}, ymm5, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vfixupimmpd	ymm6{k7}, ymm5, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vfixupimmps	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	vfixupimmps	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{F,VL}
	vfixupimmps	xmm6{k7}, xmm5, xmm4, 123	 # AVX512{F,VL}
	vfixupimmps	xmm6{k7}, xmm5, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vfixupimmps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vfixupimmps	xmm6{k7}, xmm5, [eax]{1to4}, 123	 # AVX512{F,VL}
	vfixupimmps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vfixupimmps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vfixupimmps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vfixupimmps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vfixupimmps	xmm6{k7}, xmm5, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vfixupimmps	xmm6{k7}, xmm5, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	vfixupimmps	xmm6{k7}, xmm5, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vfixupimmps	xmm6{k7}, xmm5, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	vfixupimmps	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vfixupimmps	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{F,VL}
	vfixupimmps	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{F,VL}
	vfixupimmps	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vfixupimmps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vfixupimmps	ymm6{k7}, ymm5, [eax]{1to8}, 123	 # AVX512{F,VL}
	vfixupimmps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vfixupimmps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vfixupimmps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vfixupimmps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vfixupimmps	ymm6{k7}, ymm5, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vfixupimmps	ymm6{k7}, ymm5, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vfixupimmps	ymm6{k7}, ymm5, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vfixupimmps	ymm6{k7}, ymm5, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vpslld	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vpslld	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vpslld	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vpslld	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpslld	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpslld	xmm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpslld	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vpslld	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vpslld	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vpslld	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vpslld	xmm6{k7}, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpslld	xmm6{k7}, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	vpslld	xmm6{k7}, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpslld	xmm6{k7}, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	vpslld	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vpslld	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vpslld	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vpslld	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpslld	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpslld	ymm6{k7}, [eax]{1to8}, 123	 # AVX512{F,VL}
	vpslld	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpslld	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpslld	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpslld	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpslld	ymm6{k7}, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpslld	ymm6{k7}, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vpslld	ymm6{k7}, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpslld	ymm6{k7}, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vpsllq	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vpsllq	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vpsllq	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vpsllq	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpsllq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpsllq	xmm6{k7}, [eax]{1to2}, 123	 # AVX512{F,VL}
	vpsllq	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vpsllq	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vpsllq	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vpsllq	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vpsllq	xmm6{k7}, [edx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpsllq	xmm6{k7}, [edx+1024]{1to2}, 123	 # AVX512{F,VL}
	vpsllq	xmm6{k7}, [edx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpsllq	xmm6{k7}, [edx-1032]{1to2}, 123	 # AVX512{F,VL}
	vpsllq	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vpsllq	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vpsllq	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vpsllq	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpsllq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpsllq	ymm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpsllq	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpsllq	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpsllq	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpsllq	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpsllq	ymm6{k7}, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsllq	ymm6{k7}, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpsllq	ymm6{k7}, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsllq	ymm6{k7}, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vpsrad	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vpsrad	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vpsrad	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vpsrad	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpsrad	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpsrad	xmm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpsrad	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vpsrad	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vpsrad	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vpsrad	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vpsrad	xmm6{k7}, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsrad	xmm6{k7}, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	vpsrad	xmm6{k7}, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsrad	xmm6{k7}, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	vpsrad	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vpsrad	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vpsrad	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vpsrad	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpsrad	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpsrad	ymm6{k7}, [eax]{1to8}, 123	 # AVX512{F,VL}
	vpsrad	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpsrad	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpsrad	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpsrad	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpsrad	ymm6{k7}, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpsrad	ymm6{k7}, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vpsrad	ymm6{k7}, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vpsrad	ymm6{k7}, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vpsraq	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vpsraq	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vpsraq	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vpsraq	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpsraq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpsraq	xmm6{k7}, [eax]{1to2}, 123	 # AVX512{F,VL}
	vpsraq	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vpsraq	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vpsraq	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vpsraq	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vpsraq	xmm6{k7}, [edx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpsraq	xmm6{k7}, [edx+1024]{1to2}, 123	 # AVX512{F,VL}
	vpsraq	xmm6{k7}, [edx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vpsraq	xmm6{k7}, [edx-1032]{1to2}, 123	 # AVX512{F,VL}
	vpsraq	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vpsraq	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vpsraq	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vpsraq	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vpsraq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vpsraq	ymm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vpsraq	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vpsraq	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vpsraq	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vpsraq	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vpsraq	ymm6{k7}, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsraq	ymm6{k7}, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vpsraq	ymm6{k7}, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vpsraq	ymm6{k7}, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vprolvd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vprolvd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vprolvd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vprolvd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vprolvd	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vprolvd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vprolvd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vprolvd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vprolvd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vprolvd	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vprolvd	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vprolvd	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vprolvd	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vprolvd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vprolvd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vprolvd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vprolvd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vprolvd	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vprolvd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vprolvd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vprolvd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vprolvd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vprolvd	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vprolvd	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vprolvd	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vprolvd	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vprold	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vprold	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vprold	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vprold	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vprold	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vprold	xmm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vprold	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vprold	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vprold	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vprold	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vprold	xmm6{k7}, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprold	xmm6{k7}, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	vprold	xmm6{k7}, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprold	xmm6{k7}, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	vprold	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vprold	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vprold	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vprold	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vprold	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vprold	ymm6{k7}, [eax]{1to8}, 123	 # AVX512{F,VL}
	vprold	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vprold	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vprold	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vprold	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vprold	ymm6{k7}, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vprold	ymm6{k7}, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vprold	ymm6{k7}, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vprold	ymm6{k7}, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vprolvq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vprolvq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vprolvq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vprolvq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vprolvq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vprolvq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vprolvq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vprolvq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vprolvq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vprolvq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vprolvq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vprolvq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vprolvq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vprolvq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vprolvq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vprolvq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vprolvq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vprolvq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vprolvq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vprolvq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vprolvq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vprolvq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vprolvq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vprolvq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vprolvq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vprolvq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vprolq	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vprolq	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vprolq	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vprolq	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vprolq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vprolq	xmm6{k7}, [eax]{1to2}, 123	 # AVX512{F,VL}
	vprolq	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vprolq	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vprolq	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vprolq	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vprolq	xmm6{k7}, [edx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vprolq	xmm6{k7}, [edx+1024]{1to2}, 123	 # AVX512{F,VL}
	vprolq	xmm6{k7}, [edx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vprolq	xmm6{k7}, [edx-1032]{1to2}, 123	 # AVX512{F,VL}
	vprolq	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vprolq	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vprolq	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vprolq	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vprolq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vprolq	ymm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vprolq	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vprolq	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vprolq	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vprolq	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vprolq	ymm6{k7}, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprolq	ymm6{k7}, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vprolq	ymm6{k7}, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprolq	ymm6{k7}, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vprorvd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vprorvd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vprorvd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vprorvd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vprorvd	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vprorvd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vprorvd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vprorvd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vprorvd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vprorvd	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vprorvd	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vprorvd	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vprorvd	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vprorvd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vprorvd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vprorvd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vprorvd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vprorvd	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vprorvd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vprorvd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vprorvd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vprorvd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vprorvd	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vprorvd	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vprorvd	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vprorvd	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vprord	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vprord	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vprord	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vprord	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vprord	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vprord	xmm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vprord	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vprord	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vprord	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vprord	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vprord	xmm6{k7}, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprord	xmm6{k7}, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	vprord	xmm6{k7}, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprord	xmm6{k7}, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	vprord	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vprord	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vprord	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vprord	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vprord	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vprord	ymm6{k7}, [eax]{1to8}, 123	 # AVX512{F,VL}
	vprord	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vprord	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vprord	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vprord	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vprord	ymm6{k7}, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vprord	ymm6{k7}, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vprord	ymm6{k7}, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vprord	ymm6{k7}, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vprorvq	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vprorvq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vprorvq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vprorvq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vprorvq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vprorvq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vprorvq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vprorvq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vprorvq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vprorvq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vprorvq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vprorvq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vprorvq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vprorvq	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vprorvq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vprorvq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vprorvq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vprorvq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vprorvq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vprorvq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vprorvq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vprorvq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vprorvq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vprorvq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vprorvq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vprorvq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vprorq	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vprorq	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vprorq	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vprorq	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vprorq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vprorq	xmm6{k7}, [eax]{1to2}, 123	 # AVX512{F,VL}
	vprorq	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vprorq	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vprorq	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vprorq	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vprorq	xmm6{k7}, [edx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vprorq	xmm6{k7}, [edx+1024]{1to2}, 123	 # AVX512{F,VL}
	vprorq	xmm6{k7}, [edx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vprorq	xmm6{k7}, [edx-1032]{1to2}, 123	 # AVX512{F,VL}
	vprorq	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vprorq	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vprorq	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vprorq	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vprorq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vprorq	ymm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vprorq	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vprorq	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vprorq	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vprorq	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vprorq	ymm6{k7}, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprorq	ymm6{k7}, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vprorq	ymm6{k7}, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vprorq	ymm6{k7}, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vrndscalepd	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vrndscalepd	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vrndscalepd	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vrndscalepd	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vrndscalepd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vrndscalepd	xmm6{k7}, [eax]{1to2}, 123	 # AVX512{F,VL}
	vrndscalepd	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vrndscalepd	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vrndscalepd	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vrndscalepd	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vrndscalepd	xmm6{k7}, [edx+1016]{1to2}, 123	 # AVX512{F,VL} Disp8
	vrndscalepd	xmm6{k7}, [edx+1024]{1to2}, 123	 # AVX512{F,VL}
	vrndscalepd	xmm6{k7}, [edx-1024]{1to2}, 123	 # AVX512{F,VL} Disp8
	vrndscalepd	xmm6{k7}, [edx-1032]{1to2}, 123	 # AVX512{F,VL}
	vrndscalepd	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vrndscalepd	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vrndscalepd	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vrndscalepd	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vrndscalepd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vrndscalepd	ymm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vrndscalepd	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vrndscalepd	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vrndscalepd	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vrndscalepd	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vrndscalepd	ymm6{k7}, [edx+1016]{1to4}, 123	 # AVX512{F,VL} Disp8
	vrndscalepd	ymm6{k7}, [edx+1024]{1to4}, 123	 # AVX512{F,VL}
	vrndscalepd	ymm6{k7}, [edx-1024]{1to4}, 123	 # AVX512{F,VL} Disp8
	vrndscalepd	ymm6{k7}, [edx-1032]{1to4}, 123	 # AVX512{F,VL}
	vrndscaleps	xmm6{k7}, xmm5, 0xab	 # AVX512{F,VL}
	vrndscaleps	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{F,VL}
	vrndscaleps	xmm6{k7}, xmm5, 123	 # AVX512{F,VL}
	vrndscaleps	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vrndscaleps	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vrndscaleps	xmm6{k7}, [eax]{1to4}, 123	 # AVX512{F,VL}
	vrndscaleps	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{F,VL} Disp8
	vrndscaleps	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{F,VL}
	vrndscaleps	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{F,VL} Disp8
	vrndscaleps	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{F,VL}
	vrndscaleps	xmm6{k7}, [edx+508]{1to4}, 123	 # AVX512{F,VL} Disp8
	vrndscaleps	xmm6{k7}, [edx+512]{1to4}, 123	 # AVX512{F,VL}
	vrndscaleps	xmm6{k7}, [edx-512]{1to4}, 123	 # AVX512{F,VL} Disp8
	vrndscaleps	xmm6{k7}, [edx-516]{1to4}, 123	 # AVX512{F,VL}
	vrndscaleps	ymm6{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vrndscaleps	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{F,VL}
	vrndscaleps	ymm6{k7}, ymm5, 123	 # AVX512{F,VL}
	vrndscaleps	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{F,VL}
	vrndscaleps	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{F,VL}
	vrndscaleps	ymm6{k7}, [eax]{1to8}, 123	 # AVX512{F,VL}
	vrndscaleps	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{F,VL} Disp8
	vrndscaleps	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{F,VL}
	vrndscaleps	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{F,VL} Disp8
	vrndscaleps	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{F,VL}
	vrndscaleps	ymm6{k7}, [edx+508]{1to8}, 123	 # AVX512{F,VL} Disp8
	vrndscaleps	ymm6{k7}, [edx+512]{1to8}, 123	 # AVX512{F,VL}
	vrndscaleps	ymm6{k7}, [edx-512]{1to8}, 123	 # AVX512{F,VL} Disp8
	vrndscaleps	ymm6{k7}, [edx-516]{1to8}, 123	 # AVX512{F,VL}
	vpcompressq	XMMWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpcompressq	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpcompressq	XMMWORD PTR [edx+1016]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpcompressq	XMMWORD PTR [edx+1024]{k7}, xmm6	 # AVX512{F,VL}
	vpcompressq	XMMWORD PTR [edx-1024]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpcompressq	XMMWORD PTR [edx-1032]{k7}, xmm6	 # AVX512{F,VL}
	vpcompressq	YMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpcompressq	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpcompressq	YMMWORD PTR [edx+1016]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpcompressq	YMMWORD PTR [edx+1024]{k7}, ymm6	 # AVX512{F,VL}
	vpcompressq	YMMWORD PTR [edx-1024]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpcompressq	YMMWORD PTR [edx-1032]{k7}, ymm6	 # AVX512{F,VL}
	vpcompressq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vpcompressq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vpcompressq	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vpcompressq	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vcvtps2ph	QWORD PTR [ecx]{k7}, xmm6, 0xab	 # AVX512{F,VL}
	vcvtps2ph	QWORD PTR [ecx]{k7}, xmm6, 123	 # AVX512{F,VL}
	vcvtps2ph	QWORD PTR [esp+esi*8-123456]{k7}, xmm6, 123	 # AVX512{F,VL}
	vcvtps2ph	QWORD PTR [edx+1016]{k7}, xmm6, 123	 # AVX512{F,VL} Disp8
	vcvtps2ph	QWORD PTR [edx+1024]{k7}, xmm6, 123	 # AVX512{F,VL}
	vcvtps2ph	QWORD PTR [edx-1024]{k7}, xmm6, 123	 # AVX512{F,VL} Disp8
	vcvtps2ph	QWORD PTR [edx-1032]{k7}, xmm6, 123	 # AVX512{F,VL}
	vcvtps2ph	XMMWORD PTR [ecx]{k7}, ymm6, 0xab	 # AVX512{F,VL}
	vcvtps2ph	XMMWORD PTR [ecx]{k7}, ymm6, 123	 # AVX512{F,VL}
	vcvtps2ph	XMMWORD PTR [esp+esi*8-123456]{k7}, ymm6, 123	 # AVX512{F,VL}
	vcvtps2ph	XMMWORD PTR [edx+2032]{k7}, ymm6, 123	 # AVX512{F,VL} Disp8
	vcvtps2ph	XMMWORD PTR [edx+2048]{k7}, ymm6, 123	 # AVX512{F,VL}
	vcvtps2ph	XMMWORD PTR [edx-2048]{k7}, ymm6, 123	 # AVX512{F,VL} Disp8
	vcvtps2ph	XMMWORD PTR [edx-2064]{k7}, ymm6, 123	 # AVX512{F,VL}
	vextractf32x4	XMMWORD PTR [ecx]{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vextractf32x4	XMMWORD PTR [ecx]{k7}, ymm5, 123	 # AVX512{F,VL}
	vextractf32x4	XMMWORD PTR [esp+esi*8-123456]{k7}, ymm5, 123	 # AVX512{F,VL}
	vextractf32x4	XMMWORD PTR [edx+2032]{k7}, ymm5, 123	 # AVX512{F,VL} Disp8
	vextractf32x4	XMMWORD PTR [edx+2048]{k7}, ymm5, 123	 # AVX512{F,VL}
	vextractf32x4	XMMWORD PTR [edx-2048]{k7}, ymm5, 123	 # AVX512{F,VL} Disp8
	vextractf32x4	XMMWORD PTR [edx-2064]{k7}, ymm5, 123	 # AVX512{F,VL}
	vextracti32x4	XMMWORD PTR [ecx]{k7}, ymm5, 0xab	 # AVX512{F,VL}
	vextracti32x4	XMMWORD PTR [ecx]{k7}, ymm5, 123	 # AVX512{F,VL}
	vextracti32x4	XMMWORD PTR [esp+esi*8-123456]{k7}, ymm5, 123	 # AVX512{F,VL}
	vextracti32x4	XMMWORD PTR [edx+2032]{k7}, ymm5, 123	 # AVX512{F,VL} Disp8
	vextracti32x4	XMMWORD PTR [edx+2048]{k7}, ymm5, 123	 # AVX512{F,VL}
	vextracti32x4	XMMWORD PTR [edx-2048]{k7}, ymm5, 123	 # AVX512{F,VL} Disp8
	vextracti32x4	XMMWORD PTR [edx-2064]{k7}, ymm5, 123	 # AVX512{F,VL}
	vmovapd	XMMWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vmovapd	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vmovapd	XMMWORD PTR [edx+2032]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovapd	XMMWORD PTR [edx+2048]{k7}, xmm6	 # AVX512{F,VL}
	vmovapd	XMMWORD PTR [edx-2048]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovapd	XMMWORD PTR [edx-2064]{k7}, xmm6	 # AVX512{F,VL}
	vmovapd	YMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vmovapd	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vmovapd	YMMWORD PTR [edx+4064]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovapd	YMMWORD PTR [edx+4096]{k7}, ymm6	 # AVX512{F,VL}
	vmovapd	YMMWORD PTR [edx-4096]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovapd	YMMWORD PTR [edx-4128]{k7}, ymm6	 # AVX512{F,VL}
	vmovaps	XMMWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vmovaps	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vmovaps	XMMWORD PTR [edx+2032]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovaps	XMMWORD PTR [edx+2048]{k7}, xmm6	 # AVX512{F,VL}
	vmovaps	XMMWORD PTR [edx-2048]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovaps	XMMWORD PTR [edx-2064]{k7}, xmm6	 # AVX512{F,VL}
	vmovaps	YMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vmovaps	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vmovaps	YMMWORD PTR [edx+4064]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovaps	YMMWORD PTR [edx+4096]{k7}, ymm6	 # AVX512{F,VL}
	vmovaps	YMMWORD PTR [edx-4096]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovaps	YMMWORD PTR [edx-4128]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqa32	XMMWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqa32	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqa32	XMMWORD PTR [edx+2032]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovdqa32	XMMWORD PTR [edx+2048]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqa32	XMMWORD PTR [edx-2048]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovdqa32	XMMWORD PTR [edx-2064]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqa32	YMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqa32	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqa32	YMMWORD PTR [edx+4064]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovdqa32	YMMWORD PTR [edx+4096]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqa32	YMMWORD PTR [edx-4096]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovdqa32	YMMWORD PTR [edx-4128]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqa64	XMMWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqa64	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqa64	XMMWORD PTR [edx+2032]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovdqa64	XMMWORD PTR [edx+2048]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqa64	XMMWORD PTR [edx-2048]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovdqa64	XMMWORD PTR [edx-2064]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqa64	YMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqa64	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqa64	YMMWORD PTR [edx+4064]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovdqa64	YMMWORD PTR [edx+4096]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqa64	YMMWORD PTR [edx-4096]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovdqa64	YMMWORD PTR [edx-4128]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqu32	XMMWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqu32	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqu32	XMMWORD PTR [edx+2032]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovdqu32	XMMWORD PTR [edx+2048]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqu32	XMMWORD PTR [edx-2048]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovdqu32	XMMWORD PTR [edx-2064]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqu32	YMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqu32	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqu32	YMMWORD PTR [edx+4064]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovdqu32	YMMWORD PTR [edx+4096]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqu32	YMMWORD PTR [edx-4096]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovdqu32	YMMWORD PTR [edx-4128]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqu64	XMMWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqu64	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqu64	XMMWORD PTR [edx+2032]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovdqu64	XMMWORD PTR [edx+2048]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqu64	XMMWORD PTR [edx-2048]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovdqu64	XMMWORD PTR [edx-2064]{k7}, xmm6	 # AVX512{F,VL}
	vmovdqu64	YMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqu64	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqu64	YMMWORD PTR [edx+4064]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovdqu64	YMMWORD PTR [edx+4096]{k7}, ymm6	 # AVX512{F,VL}
	vmovdqu64	YMMWORD PTR [edx-4096]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovdqu64	YMMWORD PTR [edx-4128]{k7}, ymm6	 # AVX512{F,VL}
	vmovupd	XMMWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vmovupd	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vmovupd	XMMWORD PTR [edx+2032]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovupd	XMMWORD PTR [edx+2048]{k7}, xmm6	 # AVX512{F,VL}
	vmovupd	XMMWORD PTR [edx-2048]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovupd	XMMWORD PTR [edx-2064]{k7}, xmm6	 # AVX512{F,VL}
	vmovupd	YMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vmovupd	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vmovupd	YMMWORD PTR [edx+4064]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovupd	YMMWORD PTR [edx+4096]{k7}, ymm6	 # AVX512{F,VL}
	vmovupd	YMMWORD PTR [edx-4096]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovupd	YMMWORD PTR [edx-4128]{k7}, ymm6	 # AVX512{F,VL}
	vmovups	XMMWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vmovups	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vmovups	XMMWORD PTR [edx+2032]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovups	XMMWORD PTR [edx+2048]{k7}, xmm6	 # AVX512{F,VL}
	vmovups	XMMWORD PTR [edx-2048]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vmovups	XMMWORD PTR [edx-2064]{k7}, xmm6	 # AVX512{F,VL}
	vmovups	YMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vmovups	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vmovups	YMMWORD PTR [edx+4064]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovups	YMMWORD PTR [edx+4096]{k7}, ymm6	 # AVX512{F,VL}
	vmovups	YMMWORD PTR [edx-4096]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vmovups	YMMWORD PTR [edx-4128]{k7}, ymm6	 # AVX512{F,VL}
	vpmovqb	WORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpmovqb	WORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpmovqb	WORD PTR [edx+254]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovqb	WORD PTR [edx+256]{k7}, xmm6	 # AVX512{F,VL}
	vpmovqb	WORD PTR [edx-256]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovqb	WORD PTR [edx-258]{k7}, xmm6	 # AVX512{F,VL}
	vpmovqb	DWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpmovqb	DWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpmovqb	DWORD PTR [edx+508]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovqb	DWORD PTR [edx+512]{k7}, ymm6	 # AVX512{F,VL}
	vpmovqb	DWORD PTR [edx-512]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovqb	DWORD PTR [edx-516]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsqb	WORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsqb	WORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsqb	WORD PTR [edx+254]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovsqb	WORD PTR [edx+256]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsqb	WORD PTR [edx-256]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovsqb	WORD PTR [edx-258]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsqb	DWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsqb	DWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsqb	DWORD PTR [edx+508]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovsqb	DWORD PTR [edx+512]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsqb	DWORD PTR [edx-512]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovsqb	DWORD PTR [edx-516]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusqb	WORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusqb	WORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusqb	WORD PTR [edx+254]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovusqb	WORD PTR [edx+256]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusqb	WORD PTR [edx-256]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovusqb	WORD PTR [edx-258]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusqb	DWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusqb	DWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusqb	DWORD PTR [edx+508]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovusqb	DWORD PTR [edx+512]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusqb	DWORD PTR [edx-512]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovusqb	DWORD PTR [edx-516]{k7}, ymm6	 # AVX512{F,VL}
	vpmovqw	DWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpmovqw	DWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpmovqw	DWORD PTR [edx+508]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovqw	DWORD PTR [edx+512]{k7}, xmm6	 # AVX512{F,VL}
	vpmovqw	DWORD PTR [edx-512]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovqw	DWORD PTR [edx-516]{k7}, xmm6	 # AVX512{F,VL}
	vpmovqw	QWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpmovqw	QWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpmovqw	QWORD PTR [edx+1016]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovqw	QWORD PTR [edx+1024]{k7}, ymm6	 # AVX512{F,VL}
	vpmovqw	QWORD PTR [edx-1024]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovqw	QWORD PTR [edx-1032]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsqw	DWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsqw	DWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsqw	DWORD PTR [edx+508]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovsqw	DWORD PTR [edx+512]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsqw	DWORD PTR [edx-512]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovsqw	DWORD PTR [edx-516]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsqw	QWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsqw	QWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsqw	QWORD PTR [edx+1016]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovsqw	QWORD PTR [edx+1024]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsqw	QWORD PTR [edx-1024]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovsqw	QWORD PTR [edx-1032]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusqw	DWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusqw	DWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusqw	DWORD PTR [edx+508]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovusqw	DWORD PTR [edx+512]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusqw	DWORD PTR [edx-512]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovusqw	DWORD PTR [edx-516]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusqw	QWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusqw	QWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusqw	QWORD PTR [edx+1016]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovusqw	QWORD PTR [edx+1024]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusqw	QWORD PTR [edx-1024]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovusqw	QWORD PTR [edx-1032]{k7}, ymm6	 # AVX512{F,VL}
	vpmovqd	QWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpmovqd	QWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpmovqd	QWORD PTR [edx+1016]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovqd	QWORD PTR [edx+1024]{k7}, xmm6	 # AVX512{F,VL}
	vpmovqd	QWORD PTR [edx-1024]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovqd	QWORD PTR [edx-1032]{k7}, xmm6	 # AVX512{F,VL}
	vpmovqd	XMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpmovqd	XMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpmovqd	XMMWORD PTR [edx+2032]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovqd	XMMWORD PTR [edx+2048]{k7}, ymm6	 # AVX512{F,VL}
	vpmovqd	XMMWORD PTR [edx-2048]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovqd	XMMWORD PTR [edx-2064]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsqd	QWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsqd	QWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsqd	QWORD PTR [edx+1016]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovsqd	QWORD PTR [edx+1024]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsqd	QWORD PTR [edx-1024]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovsqd	QWORD PTR [edx-1032]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsqd	XMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsqd	XMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsqd	XMMWORD PTR [edx+2032]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovsqd	XMMWORD PTR [edx+2048]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsqd	XMMWORD PTR [edx-2048]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovsqd	XMMWORD PTR [edx-2064]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusqd	QWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusqd	QWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusqd	QWORD PTR [edx+1016]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovusqd	QWORD PTR [edx+1024]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusqd	QWORD PTR [edx-1024]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovusqd	QWORD PTR [edx-1032]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusqd	XMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusqd	XMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusqd	XMMWORD PTR [edx+2032]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovusqd	XMMWORD PTR [edx+2048]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusqd	XMMWORD PTR [edx-2048]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovusqd	XMMWORD PTR [edx-2064]{k7}, ymm6	 # AVX512{F,VL}
	vpmovdb	DWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpmovdb	DWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpmovdb	DWORD PTR [edx+508]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovdb	DWORD PTR [edx+512]{k7}, xmm6	 # AVX512{F,VL}
	vpmovdb	DWORD PTR [edx-512]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovdb	DWORD PTR [edx-516]{k7}, xmm6	 # AVX512{F,VL}
	vpmovdb	QWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpmovdb	QWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpmovdb	QWORD PTR [edx+1016]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovdb	QWORD PTR [edx+1024]{k7}, ymm6	 # AVX512{F,VL}
	vpmovdb	QWORD PTR [edx-1024]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovdb	QWORD PTR [edx-1032]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsdb	DWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsdb	DWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsdb	DWORD PTR [edx+508]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovsdb	DWORD PTR [edx+512]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsdb	DWORD PTR [edx-512]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovsdb	DWORD PTR [edx-516]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsdb	QWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsdb	QWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsdb	QWORD PTR [edx+1016]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovsdb	QWORD PTR [edx+1024]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsdb	QWORD PTR [edx-1024]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovsdb	QWORD PTR [edx-1032]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusdb	DWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusdb	DWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusdb	DWORD PTR [edx+508]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovusdb	DWORD PTR [edx+512]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusdb	DWORD PTR [edx-512]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovusdb	DWORD PTR [edx-516]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusdb	QWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusdb	QWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusdb	QWORD PTR [edx+1016]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovusdb	QWORD PTR [edx+1024]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusdb	QWORD PTR [edx-1024]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovusdb	QWORD PTR [edx-1032]{k7}, ymm6	 # AVX512{F,VL}
	vpmovdw	QWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpmovdw	QWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpmovdw	QWORD PTR [edx+1016]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovdw	QWORD PTR [edx+1024]{k7}, xmm6	 # AVX512{F,VL}
	vpmovdw	QWORD PTR [edx-1024]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovdw	QWORD PTR [edx-1032]{k7}, xmm6	 # AVX512{F,VL}
	vpmovdw	XMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpmovdw	XMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpmovdw	XMMWORD PTR [edx+2032]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovdw	XMMWORD PTR [edx+2048]{k7}, ymm6	 # AVX512{F,VL}
	vpmovdw	XMMWORD PTR [edx-2048]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovdw	XMMWORD PTR [edx-2064]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsdw	QWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsdw	QWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsdw	QWORD PTR [edx+1016]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovsdw	QWORD PTR [edx+1024]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsdw	QWORD PTR [edx-1024]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovsdw	QWORD PTR [edx-1032]{k7}, xmm6	 # AVX512{F,VL}
	vpmovsdw	XMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsdw	XMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsdw	XMMWORD PTR [edx+2032]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovsdw	XMMWORD PTR [edx+2048]{k7}, ymm6	 # AVX512{F,VL}
	vpmovsdw	XMMWORD PTR [edx-2048]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovsdw	XMMWORD PTR [edx-2064]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusdw	QWORD PTR [ecx]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusdw	QWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusdw	QWORD PTR [edx+1016]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovusdw	QWORD PTR [edx+1024]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusdw	QWORD PTR [edx-1024]{k7}, xmm6	 # AVX512{F,VL} Disp8
	vpmovusdw	QWORD PTR [edx-1032]{k7}, xmm6	 # AVX512{F,VL}
	vpmovusdw	XMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusdw	XMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusdw	XMMWORD PTR [edx+2032]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovusdw	XMMWORD PTR [edx+2048]{k7}, ymm6	 # AVX512{F,VL}
	vpmovusdw	XMMWORD PTR [edx-2048]{k7}, ymm6	 # AVX512{F,VL} Disp8
	vpmovusdw	XMMWORD PTR [edx-2064]{k7}, ymm6	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, [eax]{1to2}	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, QWORD BCST [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm6{k7}, QWORD BCST [edx+1024]{1to2}	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, QWORD BCST [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm6{k7}, QWORD BCST [edx-1032]{1to2}	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, ymm5	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, QWORD BCST [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm6{k7}, QWORD BCST [edx+1024]{1to4}	 # AVX512{F,VL}
	vcvttpd2udq	xmm6{k7}, QWORD BCST [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vcvttpd2udq	xmm6{k7}, QWORD BCST [edx-1032]{1to4}	 # AVX512{F,VL}
	vcvttps2udq	xmm6{k7}, xmm5	 # AVX512{F,VL}
	vcvttps2udq	xmm6{k7}{z}, xmm5	 # AVX512{F,VL}
	vcvttps2udq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvttps2udq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvttps2udq	xmm6{k7}, [eax]{1to4}	 # AVX512{F,VL}
	vcvttps2udq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vcvttps2udq	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vcvttps2udq	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vcvttps2udq	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vcvttps2udq	xmm6{k7}, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vcvttps2udq	xmm6{k7}, [edx+512]{1to4}	 # AVX512{F,VL}
	vcvttps2udq	xmm6{k7}, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vcvttps2udq	xmm6{k7}, [edx-516]{1to4}	 # AVX512{F,VL}
	vcvttps2udq	ymm6{k7}, ymm5	 # AVX512{F,VL}
	vcvttps2udq	ymm6{k7}{z}, ymm5	 # AVX512{F,VL}
	vcvttps2udq	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vcvttps2udq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vcvttps2udq	ymm6{k7}, [eax]{1to8}	 # AVX512{F,VL}
	vcvttps2udq	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vcvttps2udq	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vcvttps2udq	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vcvttps2udq	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vcvttps2udq	ymm6{k7}, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vcvttps2udq	ymm6{k7}, [edx+512]{1to8}	 # AVX512{F,VL}
	vcvttps2udq	ymm6{k7}, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vcvttps2udq	ymm6{k7}, [edx-516]{1to8}	 # AVX512{F,VL}
	vpermi2d	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpermi2d	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpermi2d	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermi2d	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermi2d	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpermi2d	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpermi2d	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpermi2d	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpermi2d	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpermi2d	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2d	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpermi2d	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2d	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpermi2d	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpermi2d	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpermi2d	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermi2d	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermi2d	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpermi2d	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpermi2d	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpermi2d	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpermi2d	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpermi2d	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpermi2d	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpermi2d	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpermi2d	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpermi2q	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpermi2q	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpermi2q	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermi2q	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermi2q	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpermi2q	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpermi2q	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpermi2q	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpermi2q	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpermi2q	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpermi2q	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpermi2q	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpermi2q	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpermi2q	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpermi2q	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpermi2q	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermi2q	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermi2q	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpermi2q	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpermi2q	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpermi2q	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpermi2q	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpermi2q	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2q	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpermi2q	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2q	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vpermi2ps	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpermi2ps	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpermi2ps	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermi2ps	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermi2ps	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vpermi2ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpermi2ps	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpermi2ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpermi2ps	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpermi2ps	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2ps	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vpermi2ps	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2ps	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vpermi2ps	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpermi2ps	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpermi2ps	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermi2ps	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermi2ps	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vpermi2ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpermi2ps	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpermi2ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpermi2ps	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpermi2ps	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vpermi2ps	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vpermi2ps	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vpermi2ps	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vpermi2pd	xmm6{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vpermi2pd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{F,VL}
	vpermi2pd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermi2pd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermi2pd	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vpermi2pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vpermi2pd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vpermi2pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vpermi2pd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vpermi2pd	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vpermi2pd	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vpermi2pd	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vpermi2pd	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vpermi2pd	ymm6{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vpermi2pd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{F,VL}
	vpermi2pd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vpermi2pd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vpermi2pd	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vpermi2pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vpermi2pd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vpermi2pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vpermi2pd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vpermi2pd	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2pd	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vpermi2pd	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vpermi2pd	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
	vptestnmd	k5{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vptestnmd	k5{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vptestnmd	k5{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vptestnmd	k5{k7}, xmm5, [eax]{1to4}	 # AVX512{F,VL}
	vptestnmd	k5{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vptestnmd	k5{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vptestnmd	k5{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vptestnmd	k5{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vptestnmd	k5{k7}, xmm5, [edx+508]{1to4}	 # AVX512{F,VL} Disp8
	vptestnmd	k5{k7}, xmm5, [edx+512]{1to4}	 # AVX512{F,VL}
	vptestnmd	k5{k7}, xmm5, [edx-512]{1to4}	 # AVX512{F,VL} Disp8
	vptestnmd	k5{k7}, xmm5, [edx-516]{1to4}	 # AVX512{F,VL}
	vptestnmd	k5{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vptestnmd	k5{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vptestnmd	k5{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vptestnmd	k5{k7}, ymm5, [eax]{1to8}	 # AVX512{F,VL}
	vptestnmd	k5{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vptestnmd	k5{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vptestnmd	k5{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vptestnmd	k5{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vptestnmd	k5{k7}, ymm5, [edx+508]{1to8}	 # AVX512{F,VL} Disp8
	vptestnmd	k5{k7}, ymm5, [edx+512]{1to8}	 # AVX512{F,VL}
	vptestnmd	k5{k7}, ymm5, [edx-512]{1to8}	 # AVX512{F,VL} Disp8
	vptestnmd	k5{k7}, ymm5, [edx-516]{1to8}	 # AVX512{F,VL}
	vptestnmq	k5{k7}, xmm5, xmm4	 # AVX512{F,VL}
	vptestnmq	k5{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{F,VL}
	vptestnmq	k5{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vptestnmq	k5{k7}, xmm5, [eax]{1to2}	 # AVX512{F,VL}
	vptestnmq	k5{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{F,VL} Disp8
	vptestnmq	k5{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{F,VL}
	vptestnmq	k5{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{F,VL} Disp8
	vptestnmq	k5{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{F,VL}
	vptestnmq	k5{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{F,VL} Disp8
	vptestnmq	k5{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{F,VL}
	vptestnmq	k5{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{F,VL} Disp8
	vptestnmq	k5{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{F,VL}
	vptestnmq	k5{k7}, ymm5, ymm4	 # AVX512{F,VL}
	vptestnmq	k5{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{F,VL}
	vptestnmq	k5{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{F,VL}
	vptestnmq	k5{k7}, ymm5, [eax]{1to4}	 # AVX512{F,VL}
	vptestnmq	k5{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{F,VL} Disp8
	vptestnmq	k5{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{F,VL}
	vptestnmq	k5{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{F,VL} Disp8
	vptestnmq	k5{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{F,VL}
	vptestnmq	k5{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{F,VL} Disp8
	vptestnmq	k5{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{F,VL}
	vptestnmq	k5{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{F,VL} Disp8
	vptestnmq	k5{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{F,VL}
