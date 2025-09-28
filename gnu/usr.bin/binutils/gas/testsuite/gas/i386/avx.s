# Check AVX instructions

	.allow_index_reg
	.text
_start:
# Tests for op
	vzeroall
	vzeroupper

# Tests for op mem64
	vldmxcsr (%ecx)
	vstmxcsr (%ecx)

# Tests for op mem256, mask,  ymm
# Tests for op ymm, mask, mem256
	vmaskmovpd (%ecx),%ymm4,%ymm6
	vmaskmovpd %ymm4,%ymm6,(%ecx)
	vmaskmovps (%ecx),%ymm4,%ymm6
	vmaskmovps %ymm4,%ymm6,(%ecx)

# Tests for op imm8, ymm/mem256, ymm
	vpermilpd $7,%ymm6,%ymm2
	vpermilpd $7,(%ecx),%ymm6
	vpermilps $7,%ymm6,%ymm2
	vpermilps $7,(%ecx),%ymm6
	vroundpd $7,%ymm6,%ymm2
	vroundpd $7,(%ecx),%ymm6
	vroundps $7,%ymm6,%ymm2
	vroundps $7,(%ecx),%ymm6

# Tests for op ymm/mem256, ymm, ymm
	vaddpd %ymm4,%ymm6,%ymm2
	vaddpd (%ecx),%ymm6,%ymm2
	vaddps %ymm4,%ymm6,%ymm2
	vaddps (%ecx),%ymm6,%ymm2
	vaddsubpd %ymm4,%ymm6,%ymm2
	vaddsubpd (%ecx),%ymm6,%ymm2
	vaddsubps %ymm4,%ymm6,%ymm2
	vaddsubps (%ecx),%ymm6,%ymm2
	vandnpd %ymm4,%ymm6,%ymm2
	vandnpd (%ecx),%ymm6,%ymm2
	vandnps %ymm4,%ymm6,%ymm2
	vandnps (%ecx),%ymm6,%ymm2
	vandpd %ymm4,%ymm6,%ymm2
	vandpd (%ecx),%ymm6,%ymm2
	vandps %ymm4,%ymm6,%ymm2
	vandps (%ecx),%ymm6,%ymm2
	vdivpd %ymm4,%ymm6,%ymm2
	vdivpd (%ecx),%ymm6,%ymm2
	vdivps %ymm4,%ymm6,%ymm2
	vdivps (%ecx),%ymm6,%ymm2
	vhaddpd %ymm4,%ymm6,%ymm2
	vhaddpd (%ecx),%ymm6,%ymm2
	vhaddps %ymm4,%ymm6,%ymm2
	vhaddps (%ecx),%ymm6,%ymm2
	vhsubpd %ymm4,%ymm6,%ymm2
	vhsubpd (%ecx),%ymm6,%ymm2
	vhsubps %ymm4,%ymm6,%ymm2
	vhsubps (%ecx),%ymm6,%ymm2
	vmaxpd %ymm4,%ymm6,%ymm2
	vmaxpd (%ecx),%ymm6,%ymm2
	vmaxps %ymm4,%ymm6,%ymm2
	vmaxps (%ecx),%ymm6,%ymm2
	vminpd %ymm4,%ymm6,%ymm2
	vminpd (%ecx),%ymm6,%ymm2
	vminps %ymm4,%ymm6,%ymm2
	vminps (%ecx),%ymm6,%ymm2
	vmulpd %ymm4,%ymm6,%ymm2
	vmulpd (%ecx),%ymm6,%ymm2
	vmulps %ymm4,%ymm6,%ymm2
	vmulps (%ecx),%ymm6,%ymm2
	vorpd %ymm4,%ymm6,%ymm2
	vorpd (%ecx),%ymm6,%ymm2
	vorps %ymm4,%ymm6,%ymm2
	vorps (%ecx),%ymm6,%ymm2
	vpermilpd %ymm4,%ymm6,%ymm2
	vpermilpd (%ecx),%ymm6,%ymm2
	vpermilps %ymm4,%ymm6,%ymm2
	vpermilps (%ecx),%ymm6,%ymm2
	vsubpd %ymm4,%ymm6,%ymm2
	vsubpd (%ecx),%ymm6,%ymm2
	vsubps %ymm4,%ymm6,%ymm2
	vsubps (%ecx),%ymm6,%ymm2
	vunpckhpd %ymm4,%ymm6,%ymm2
	vunpckhpd (%ecx),%ymm6,%ymm2
	vunpckhps %ymm4,%ymm6,%ymm2
	vunpckhps (%ecx),%ymm6,%ymm2
	vunpcklpd %ymm4,%ymm6,%ymm2
	vunpcklpd (%ecx),%ymm6,%ymm2
	vunpcklps %ymm4,%ymm6,%ymm2
	vunpcklps (%ecx),%ymm6,%ymm2
	vxorpd %ymm4,%ymm6,%ymm2
	vxorpd (%ecx),%ymm6,%ymm2
	vxorps %ymm4,%ymm6,%ymm2
	vxorps (%ecx),%ymm6,%ymm2
	vcmpeqpd %ymm4,%ymm6,%ymm2
	vcmpeqpd (%ecx),%ymm6,%ymm2
	vcmpeq_oqpd %ymm4,%ymm6,%ymm2
	vcmpeq_oqpd (%ecx),%ymm6,%ymm2
	vcmpltpd %ymm4,%ymm6,%ymm2
	vcmpltpd (%ecx),%ymm6,%ymm2
	vcmplt_ospd %ymm4,%ymm6,%ymm2
	vcmplt_ospd (%ecx),%ymm6,%ymm2
	vcmplepd %ymm4,%ymm6,%ymm2
	vcmplepd (%ecx),%ymm6,%ymm2
	vcmple_ospd %ymm4,%ymm6,%ymm2
	vcmple_ospd (%ecx),%ymm6,%ymm2
	vcmpunordpd %ymm4,%ymm6,%ymm2
	vcmpunordpd (%ecx),%ymm6,%ymm2
	vcmpunord_qpd %ymm4,%ymm6,%ymm2
	vcmpunord_qpd (%ecx),%ymm6,%ymm2
	vcmpneqpd %ymm4,%ymm6,%ymm2
	vcmpneqpd (%ecx),%ymm6,%ymm2
	vcmpneq_uqpd %ymm4,%ymm6,%ymm2
	vcmpneq_uqpd (%ecx),%ymm6,%ymm2
	vcmpnltpd %ymm4,%ymm6,%ymm2
	vcmpnltpd (%ecx),%ymm6,%ymm2
	vcmpnlt_uspd %ymm4,%ymm6,%ymm2
	vcmpnlt_uspd (%ecx),%ymm6,%ymm2
	vcmpnlepd %ymm4,%ymm6,%ymm2
	vcmpnlepd (%ecx),%ymm6,%ymm2
	vcmpnle_uspd %ymm4,%ymm6,%ymm2
	vcmpnle_uspd (%ecx),%ymm6,%ymm2
	vcmpordpd %ymm4,%ymm6,%ymm2
	vcmpordpd (%ecx),%ymm6,%ymm2
	vcmpord_qpd %ymm4,%ymm6,%ymm2
	vcmpord_qpd (%ecx),%ymm6,%ymm2
	vcmpeq_uqpd %ymm4,%ymm6,%ymm2
	vcmpeq_uqpd (%ecx),%ymm6,%ymm2
	vcmpngepd %ymm4,%ymm6,%ymm2
	vcmpngepd (%ecx),%ymm6,%ymm2
	vcmpnge_uspd %ymm4,%ymm6,%ymm2
	vcmpnge_uspd (%ecx),%ymm6,%ymm2
	vcmpngtpd %ymm4,%ymm6,%ymm2
	vcmpngtpd (%ecx),%ymm6,%ymm2
	vcmpngt_uspd %ymm4,%ymm6,%ymm2
	vcmpngt_uspd (%ecx),%ymm6,%ymm2
	vcmpfalsepd %ymm4,%ymm6,%ymm2
	vcmpfalsepd (%ecx),%ymm6,%ymm2
	vcmpfalse_oqpd %ymm4,%ymm6,%ymm2
	vcmpfalse_oqpd (%ecx),%ymm6,%ymm2
	vcmpneq_oqpd %ymm4,%ymm6,%ymm2
	vcmpneq_oqpd (%ecx),%ymm6,%ymm2
	vcmpgepd %ymm4,%ymm6,%ymm2
	vcmpgepd (%ecx),%ymm6,%ymm2
	vcmpge_ospd %ymm4,%ymm6,%ymm2
	vcmpge_ospd (%ecx),%ymm6,%ymm2
	vcmpgtpd %ymm4,%ymm6,%ymm2
	vcmpgtpd (%ecx),%ymm6,%ymm2
	vcmpgt_ospd %ymm4,%ymm6,%ymm2
	vcmpgt_ospd (%ecx),%ymm6,%ymm2
	vcmptruepd %ymm4,%ymm6,%ymm2
	vcmptruepd (%ecx),%ymm6,%ymm2
	vcmptrue_uqpd %ymm4,%ymm6,%ymm2
	vcmptrue_uqpd (%ecx),%ymm6,%ymm2
	vcmpeq_ospd %ymm4,%ymm6,%ymm2
	vcmpeq_ospd (%ecx),%ymm6,%ymm2
	vcmplt_oqpd %ymm4,%ymm6,%ymm2
	vcmplt_oqpd (%ecx),%ymm6,%ymm2
	vcmple_oqpd %ymm4,%ymm6,%ymm2
	vcmple_oqpd (%ecx),%ymm6,%ymm2
	vcmpunord_spd %ymm4,%ymm6,%ymm2
	vcmpunord_spd (%ecx),%ymm6,%ymm2
	vcmpneq_uspd %ymm4,%ymm6,%ymm2
	vcmpneq_uspd (%ecx),%ymm6,%ymm2
	vcmpnlt_uqpd %ymm4,%ymm6,%ymm2
	vcmpnlt_uqpd (%ecx),%ymm6,%ymm2
	vcmpnle_uqpd %ymm4,%ymm6,%ymm2
	vcmpnle_uqpd (%ecx),%ymm6,%ymm2
	vcmpord_spd %ymm4,%ymm6,%ymm2
	vcmpord_spd (%ecx),%ymm6,%ymm2
	vcmpeq_uspd %ymm4,%ymm6,%ymm2
	vcmpeq_uspd (%ecx),%ymm6,%ymm2
	vcmpnge_uqpd %ymm4,%ymm6,%ymm2
	vcmpnge_uqpd (%ecx),%ymm6,%ymm2
	vcmpngt_uqpd %ymm4,%ymm6,%ymm2
	vcmpngt_uqpd (%ecx),%ymm6,%ymm2
	vcmpfalse_ospd %ymm4,%ymm6,%ymm2
	vcmpfalse_ospd (%ecx),%ymm6,%ymm2
	vcmpneq_ospd %ymm4,%ymm6,%ymm2
	vcmpneq_ospd (%ecx),%ymm6,%ymm2
	vcmpge_oqpd %ymm4,%ymm6,%ymm2
	vcmpge_oqpd (%ecx),%ymm6,%ymm2
	vcmpgt_oqpd %ymm4,%ymm6,%ymm2
	vcmpgt_oqpd (%ecx),%ymm6,%ymm2
	vcmptrue_uspd %ymm4,%ymm6,%ymm2
	vcmptrue_uspd (%ecx),%ymm6,%ymm2
	vcmpeqps %ymm4,%ymm6,%ymm2
	vcmpeqps (%ecx),%ymm6,%ymm2
	vcmpeq_oqps %ymm4,%ymm6,%ymm2
	vcmpeq_oqps (%ecx),%ymm6,%ymm2
	vcmpltps %ymm4,%ymm6,%ymm2
	vcmpltps (%ecx),%ymm6,%ymm2
	vcmplt_osps %ymm4,%ymm6,%ymm2
	vcmplt_osps (%ecx),%ymm6,%ymm2
	vcmpleps %ymm4,%ymm6,%ymm2
	vcmpleps (%ecx),%ymm6,%ymm2
	vcmple_osps %ymm4,%ymm6,%ymm2
	vcmple_osps (%ecx),%ymm6,%ymm2
	vcmpunordps %ymm4,%ymm6,%ymm2
	vcmpunordps (%ecx),%ymm6,%ymm2
	vcmpunord_qps %ymm4,%ymm6,%ymm2
	vcmpunord_qps (%ecx),%ymm6,%ymm2
	vcmpneqps %ymm4,%ymm6,%ymm2
	vcmpneqps (%ecx),%ymm6,%ymm2
	vcmpneq_uqps %ymm4,%ymm6,%ymm2
	vcmpneq_uqps (%ecx),%ymm6,%ymm2
	vcmpnltps %ymm4,%ymm6,%ymm2
	vcmpnltps (%ecx),%ymm6,%ymm2
	vcmpnlt_usps %ymm4,%ymm6,%ymm2
	vcmpnlt_usps (%ecx),%ymm6,%ymm2
	vcmpnleps %ymm4,%ymm6,%ymm2
	vcmpnleps (%ecx),%ymm6,%ymm2
	vcmpnle_usps %ymm4,%ymm6,%ymm2
	vcmpnle_usps (%ecx),%ymm6,%ymm2
	vcmpordps %ymm4,%ymm6,%ymm2
	vcmpordps (%ecx),%ymm6,%ymm2
	vcmpord_qps %ymm4,%ymm6,%ymm2
	vcmpord_qps (%ecx),%ymm6,%ymm2
	vcmpeq_uqps %ymm4,%ymm6,%ymm2
	vcmpeq_uqps (%ecx),%ymm6,%ymm2
	vcmpngeps %ymm4,%ymm6,%ymm2
	vcmpngeps (%ecx),%ymm6,%ymm2
	vcmpnge_usps %ymm4,%ymm6,%ymm2
	vcmpnge_usps (%ecx),%ymm6,%ymm2
	vcmpngtps %ymm4,%ymm6,%ymm2
	vcmpngtps (%ecx),%ymm6,%ymm2
	vcmpngt_usps %ymm4,%ymm6,%ymm2
	vcmpngt_usps (%ecx),%ymm6,%ymm2
	vcmpfalseps %ymm4,%ymm6,%ymm2
	vcmpfalseps (%ecx),%ymm6,%ymm2
	vcmpfalse_oqps %ymm4,%ymm6,%ymm2
	vcmpfalse_oqps (%ecx),%ymm6,%ymm2
	vcmpneq_oqps %ymm4,%ymm6,%ymm2
	vcmpneq_oqps (%ecx),%ymm6,%ymm2
	vcmpgeps %ymm4,%ymm6,%ymm2
	vcmpgeps (%ecx),%ymm6,%ymm2
	vcmpge_osps %ymm4,%ymm6,%ymm2
	vcmpge_osps (%ecx),%ymm6,%ymm2
	vcmpgtps %ymm4,%ymm6,%ymm2
	vcmpgtps (%ecx),%ymm6,%ymm2
	vcmpgt_osps %ymm4,%ymm6,%ymm2
	vcmpgt_osps (%ecx),%ymm6,%ymm2
	vcmptrueps %ymm4,%ymm6,%ymm2
	vcmptrueps (%ecx),%ymm6,%ymm2
	vcmptrue_uqps %ymm4,%ymm6,%ymm2
	vcmptrue_uqps (%ecx),%ymm6,%ymm2
	vcmpeq_osps %ymm4,%ymm6,%ymm2
	vcmpeq_osps (%ecx),%ymm6,%ymm2
	vcmplt_oqps %ymm4,%ymm6,%ymm2
	vcmplt_oqps (%ecx),%ymm6,%ymm2
	vcmple_oqps %ymm4,%ymm6,%ymm2
	vcmple_oqps (%ecx),%ymm6,%ymm2
	vcmpunord_sps %ymm4,%ymm6,%ymm2
	vcmpunord_sps (%ecx),%ymm6,%ymm2
	vcmpneq_usps %ymm4,%ymm6,%ymm2
	vcmpneq_usps (%ecx),%ymm6,%ymm2
	vcmpnlt_uqps %ymm4,%ymm6,%ymm2
	vcmpnlt_uqps (%ecx),%ymm6,%ymm2
	vcmpnle_uqps %ymm4,%ymm6,%ymm2
	vcmpnle_uqps (%ecx),%ymm6,%ymm2
	vcmpord_sps %ymm4,%ymm6,%ymm2
	vcmpord_sps (%ecx),%ymm6,%ymm2
	vcmpeq_usps %ymm4,%ymm6,%ymm2
	vcmpeq_usps (%ecx),%ymm6,%ymm2
	vcmpnge_uqps %ymm4,%ymm6,%ymm2
	vcmpnge_uqps (%ecx),%ymm6,%ymm2
	vcmpngt_uqps %ymm4,%ymm6,%ymm2
	vcmpngt_uqps (%ecx),%ymm6,%ymm2
	vcmpfalse_osps %ymm4,%ymm6,%ymm2
	vcmpfalse_osps (%ecx),%ymm6,%ymm2
	vcmpneq_osps %ymm4,%ymm6,%ymm2
	vcmpneq_osps (%ecx),%ymm6,%ymm2
	vcmpge_oqps %ymm4,%ymm6,%ymm2
	vcmpge_oqps (%ecx),%ymm6,%ymm2
	vcmpgt_oqps %ymm4,%ymm6,%ymm2
	vcmpgt_oqps (%ecx),%ymm6,%ymm2
	vcmptrue_usps %ymm4,%ymm6,%ymm2
	vcmptrue_usps (%ecx),%ymm6,%ymm2
    vgf2p8mulb %ymm4, %ymm5, %ymm6
	vgf2p8mulb (%ecx), %ymm5, %ymm6
	vgf2p8mulb -123456(%esp,%esi,8), %ymm5, %ymm6
	vgf2p8mulb 4064(%edx), %ymm5, %ymm6
	vgf2p8mulb 4096(%edx), %ymm5, %ymm6
	vgf2p8mulb -4096(%edx), %ymm5, %ymm6
	vgf2p8mulb -4128(%edx), %ymm5, %ymm6

# Tests for op ymm/mem256, xmm
	vcvtpd2dqy %ymm4,%xmm4
	vcvtpd2dqy (%ecx),%xmm4
	vcvtpd2psy %ymm4,%xmm4
	vcvtpd2psy (%ecx),%xmm4
	vcvttpd2dqy %ymm4,%xmm4
	vcvttpd2dqy (%ecx),%xmm4

# Tests for op ymm/mem256, ymm
	vcvtdq2ps %ymm4,%ymm6
	vcvtdq2ps (%ecx),%ymm4
	vcvtps2dq %ymm4,%ymm6
	vcvtps2dq (%ecx),%ymm4
	vcvttps2dq %ymm4,%ymm6
	vcvttps2dq (%ecx),%ymm4
	vmovapd %ymm4,%ymm6
	vmovapd (%ecx),%ymm4
	vmovaps %ymm4,%ymm6
	vmovaps (%ecx),%ymm4
	vmovdqa %ymm4,%ymm6
	vmovdqa (%ecx),%ymm4
	vmovdqu %ymm4,%ymm6
	vmovdqu (%ecx),%ymm4
	vmovddup %ymm4,%ymm6
	vmovddup (%ecx),%ymm4
	vmovshdup %ymm4,%ymm6
	vmovshdup (%ecx),%ymm4
	vmovsldup %ymm4,%ymm6
	vmovsldup (%ecx),%ymm4
	vmovupd %ymm4,%ymm6
	vmovupd (%ecx),%ymm4
	vmovups %ymm4,%ymm6
	vmovups (%ecx),%ymm4
	vptest %ymm4,%ymm6
	vptest (%ecx),%ymm4
	vrcpps %ymm4,%ymm6
	vrcpps (%ecx),%ymm4
	vrsqrtps %ymm4,%ymm6
	vrsqrtps (%ecx),%ymm4
	vsqrtpd %ymm4,%ymm6
	vsqrtpd (%ecx),%ymm4
	vsqrtps %ymm4,%ymm6
	vsqrtps (%ecx),%ymm4
	vtestpd %ymm4,%ymm6
	vtestpd (%ecx),%ymm4
	vtestps %ymm4,%ymm6
	vtestps (%ecx),%ymm4

# Tests for op ymm, ymm/mem256
	vmovapd %ymm4,%ymm6
	vmovapd %ymm4,(%ecx)
	vmovaps %ymm4,%ymm6
	vmovaps %ymm4,(%ecx)
	vmovdqa %ymm4,%ymm6
	vmovdqa %ymm4,(%ecx)
	vmovdqu %ymm4,%ymm6
	vmovdqu %ymm4,(%ecx)
	vmovupd %ymm4,%ymm6
	vmovupd %ymm4,(%ecx)
	vmovups %ymm4,%ymm6
	vmovups %ymm4,(%ecx)

# Tests for op mem256, ymm
	vlddqu (%ecx),%ymm4

# Tests for op ymm, mem256
	vmovntdq %ymm4,(%ecx)
	vmovntpd %ymm4,(%ecx)
	vmovntps %ymm4,(%ecx)

# Tests for op imm8, ymm/mem256, ymm, ymm
	vblendpd $7,%ymm4,%ymm6,%ymm2
	vblendpd $7,(%ecx),%ymm6,%ymm2
	vblendps $7,%ymm4,%ymm6,%ymm2
	vblendps $7,(%ecx),%ymm6,%ymm2
	vcmppd $7,%ymm4,%ymm6,%ymm2
	vcmppd $7,(%ecx),%ymm6,%ymm2
	vcmpps $7,%ymm4,%ymm6,%ymm2
	vcmpps $7,(%ecx),%ymm6,%ymm2
	vdpps $7,%ymm4,%ymm6,%ymm2
	vdpps $7,(%ecx),%ymm6,%ymm2
	vperm2f128 $7,%ymm4,%ymm6,%ymm2
	vperm2f128 $7,(%ecx),%ymm6,%ymm2
	vshufpd $7,%ymm4,%ymm6,%ymm2
	vshufpd $7,(%ecx),%ymm6,%ymm2
	vshufps $7,%ymm4,%ymm6,%ymm2
	vshufps $7,(%ecx),%ymm6,%ymm2
    vgf2p8affineqb $0xab, %ymm4, %ymm5, %ymm6
	vgf2p8affineqb $123, %ymm4, %ymm5, %ymm6
	vgf2p8affineqb $123, (%ecx), %ymm5, %ymm6
	vgf2p8affineqb $123, -123456(%esp,%esi,8), %ymm5, %ymm6
	vgf2p8affineqb $123, 4064(%edx), %ymm5, %ymm6
	vgf2p8affineqb $123, 4096(%edx), %ymm5, %ymm6
	vgf2p8affineqb $123, -4096(%edx), %ymm5, %ymm6
	vgf2p8affineqb $123, -4128(%edx), %ymm5, %ymm6
	vgf2p8affineinvqb $0xab, %ymm4, %ymm5, %ymm6
	vgf2p8affineinvqb $123, %ymm4, %ymm5, %ymm6
	vgf2p8affineinvqb $123, (%ecx), %ymm5, %ymm6
	vgf2p8affineinvqb $123, -123456(%esp,%esi,8), %ymm5, %ymm6
	vgf2p8affineinvqb $123, 4064(%edx), %ymm5, %ymm6
	vgf2p8affineinvqb $123, 4096(%edx), %ymm5, %ymm6
	vgf2p8affineinvqb $123, -4096(%edx), %ymm5, %ymm6
	vgf2p8affineinvqb $123, -4128(%edx), %ymm5, %ymm6

# Tests for op ymm, ymm/mem256, ymm, ymm
	vblendvpd %ymm4,%ymm6,%ymm2,%ymm7
	vblendvpd %ymm4,(%ecx),%ymm2,%ymm7
	vblendvps %ymm4,%ymm6,%ymm2,%ymm7
	vblendvps %ymm4,(%ecx),%ymm2,%ymm7

# Tests for op imm8, xmm/mem128, ymm, ymm
	vinsertf128 $7,%xmm4,%ymm4,%ymm6
	vinsertf128 $7,(%ecx),%ymm4,%ymm6

# Tests for op imm8, ymm, xmm/mem128
	vextractf128 $7,%ymm4,%xmm4
	vextractf128 $7,%ymm4,(%ecx)

# Tests for op mem128, ymm
	vbroadcastf128 (%ecx),%ymm4

# Tests for op xmm/mem128, xmm
	vcvtdq2ps %xmm4,%xmm6
	vcvtdq2ps (%ecx),%xmm4
	vcvtpd2dqx %xmm4,%xmm6
	vcvtpd2dqx (%ecx),%xmm4
	vcvtpd2psx %xmm4,%xmm6
	vcvtpd2psx (%ecx),%xmm4
	vcvtps2dq %xmm4,%xmm6
	vcvtps2dq (%ecx),%xmm4
	vcvttpd2dqx %xmm4,%xmm6
	vcvttpd2dqx (%ecx),%xmm4
	vcvttps2dq %xmm4,%xmm6
	vcvttps2dq (%ecx),%xmm4
	vmovapd %xmm4,%xmm6
	vmovapd (%ecx),%xmm4
	vmovaps %xmm4,%xmm6
	vmovaps (%ecx),%xmm4
	vmovdqa %xmm4,%xmm6
	vmovdqa (%ecx),%xmm4
	vmovdqu %xmm4,%xmm6
	vmovdqu (%ecx),%xmm4
	vmovshdup %xmm4,%xmm6
	vmovshdup (%ecx),%xmm4
	vmovsldup %xmm4,%xmm6
	vmovsldup (%ecx),%xmm4
	vmovupd %xmm4,%xmm6
	vmovupd (%ecx),%xmm4
	vmovups %xmm4,%xmm6
	vmovups (%ecx),%xmm4
	vpabsb %xmm4,%xmm6
	vpabsb (%ecx),%xmm4
	vpabsw %xmm4,%xmm6
	vpabsw (%ecx),%xmm4
	vpabsd %xmm4,%xmm6
	vpabsd (%ecx),%xmm4
	vphminposuw %xmm4,%xmm6
	vphminposuw (%ecx),%xmm4
	vptest %xmm4,%xmm6
	vptest (%ecx),%xmm4
	vtestps %xmm4,%xmm6
	vtestps (%ecx),%xmm4
	vtestpd %xmm4,%xmm6
	vtestpd (%ecx),%xmm4
	vrcpps %xmm4,%xmm6
	vrcpps (%ecx),%xmm4
	vrsqrtps %xmm4,%xmm6
	vrsqrtps (%ecx),%xmm4
	vsqrtpd %xmm4,%xmm6
	vsqrtpd (%ecx),%xmm4
	vsqrtps %xmm4,%xmm6
	vsqrtps (%ecx),%xmm4
	vaesimc %xmm4,%xmm6
	vaesimc (%ecx),%xmm4

# Tests for op xmm, xmm/mem128
	vmovapd %xmm4,%xmm6
	vmovapd %xmm4,(%ecx)
	vmovaps %xmm4,%xmm6
	vmovaps %xmm4,(%ecx)
	vmovdqa %xmm4,%xmm6
	vmovdqa %xmm4,(%ecx)
	vmovdqu %xmm4,%xmm6
	vmovdqu %xmm4,(%ecx)
	vmovupd %xmm4,%xmm6
	vmovupd %xmm4,(%ecx)
	vmovups %xmm4,%xmm6
	vmovups %xmm4,(%ecx)

# Tests for op mem128, xmm
	vlddqu (%ecx),%xmm4
	vmovntdqa (%ecx),%xmm4

# Tests for op xmm, mem128
	vmovntdq %xmm4,(%ecx)
	vmovntpd %xmm4,(%ecx)
	vmovntps %xmm4,(%ecx)

# Tests for op xmm/mem128, ymm
	vcvtdq2pd %xmm4,%ymm4
	vcvtdq2pd (%ecx),%ymm4
	vcvtps2pd %xmm4,%ymm4
	vcvtps2pd (%ecx),%ymm4

# Tests for op xmm/mem128, xmm, xmm
	vaddpd %xmm4,%xmm6,%xmm2
	vaddpd (%ecx),%xmm6,%xmm7
	vaddps %xmm4,%xmm6,%xmm2
	vaddps (%ecx),%xmm6,%xmm7
	vaddsubpd %xmm4,%xmm6,%xmm2
	vaddsubpd (%ecx),%xmm6,%xmm7
	vaddsubps %xmm4,%xmm6,%xmm2
	vaddsubps (%ecx),%xmm6,%xmm7
	vandnpd %xmm4,%xmm6,%xmm2
	vandnpd (%ecx),%xmm6,%xmm7
	vandnps %xmm4,%xmm6,%xmm2
	vandnps (%ecx),%xmm6,%xmm7
	vandpd %xmm4,%xmm6,%xmm2
	vandpd (%ecx),%xmm6,%xmm7
	vandps %xmm4,%xmm6,%xmm2
	vandps (%ecx),%xmm6,%xmm7
	vdivpd %xmm4,%xmm6,%xmm2
	vdivpd (%ecx),%xmm6,%xmm7
	vdivps %xmm4,%xmm6,%xmm2
	vdivps (%ecx),%xmm6,%xmm7
	vhaddpd %xmm4,%xmm6,%xmm2
	vhaddpd (%ecx),%xmm6,%xmm7
	vhaddps %xmm4,%xmm6,%xmm2
	vhaddps (%ecx),%xmm6,%xmm7
	vhsubpd %xmm4,%xmm6,%xmm2
	vhsubpd (%ecx),%xmm6,%xmm7
	vhsubps %xmm4,%xmm6,%xmm2
	vhsubps (%ecx),%xmm6,%xmm7
	vmaxpd %xmm4,%xmm6,%xmm2
	vmaxpd (%ecx),%xmm6,%xmm7
	vmaxps %xmm4,%xmm6,%xmm2
	vmaxps (%ecx),%xmm6,%xmm7
	vminpd %xmm4,%xmm6,%xmm2
	vminpd (%ecx),%xmm6,%xmm7
	vminps %xmm4,%xmm6,%xmm2
	vminps (%ecx),%xmm6,%xmm7
	vmulpd %xmm4,%xmm6,%xmm2
	vmulpd (%ecx),%xmm6,%xmm7
	vmulps %xmm4,%xmm6,%xmm2
	vmulps (%ecx),%xmm6,%xmm7
	vorpd %xmm4,%xmm6,%xmm2
	vorpd (%ecx),%xmm6,%xmm7
	vorps %xmm4,%xmm6,%xmm2
	vorps (%ecx),%xmm6,%xmm7
	vpacksswb %xmm4,%xmm6,%xmm2
	vpacksswb (%ecx),%xmm6,%xmm7
	vpackssdw %xmm4,%xmm6,%xmm2
	vpackssdw (%ecx),%xmm6,%xmm7
	vpackuswb %xmm4,%xmm6,%xmm2
	vpackuswb (%ecx),%xmm6,%xmm7
	vpackusdw %xmm4,%xmm6,%xmm2
	vpackusdw (%ecx),%xmm6,%xmm7
	vpaddb %xmm4,%xmm6,%xmm2
	vpaddb (%ecx),%xmm6,%xmm7
	vpaddw %xmm4,%xmm6,%xmm2
	vpaddw (%ecx),%xmm6,%xmm7
	vpaddd %xmm4,%xmm6,%xmm2
	vpaddd (%ecx),%xmm6,%xmm7
	vpaddq %xmm4,%xmm6,%xmm2
	vpaddq (%ecx),%xmm6,%xmm7
	vpaddsb %xmm4,%xmm6,%xmm2
	vpaddsb (%ecx),%xmm6,%xmm7
	vpaddsw %xmm4,%xmm6,%xmm2
	vpaddsw (%ecx),%xmm6,%xmm7
	vpaddusb %xmm4,%xmm6,%xmm2
	vpaddusb (%ecx),%xmm6,%xmm7
	vpaddusw %xmm4,%xmm6,%xmm2
	vpaddusw (%ecx),%xmm6,%xmm7
	vpand %xmm4,%xmm6,%xmm2
	vpand (%ecx),%xmm6,%xmm7
	vpandn %xmm4,%xmm6,%xmm2
	vpandn (%ecx),%xmm6,%xmm7
	vpavgb %xmm4,%xmm6,%xmm2
	vpavgb (%ecx),%xmm6,%xmm7
	vpavgw %xmm4,%xmm6,%xmm2
	vpavgw (%ecx),%xmm6,%xmm7
	vpclmullqlqdq %xmm4,%xmm6,%xmm2
	vpclmullqlqdq (%ecx),%xmm6,%xmm7
	vpclmulhqlqdq %xmm4,%xmm6,%xmm2
	vpclmulhqlqdq (%ecx),%xmm6,%xmm7
	vpclmullqhqdq %xmm4,%xmm6,%xmm2
	vpclmullqhqdq (%ecx),%xmm6,%xmm7
	vpclmulhqhqdq %xmm4,%xmm6,%xmm2
	vpclmulhqhqdq (%ecx),%xmm6,%xmm7
	vpcmpeqb %xmm4,%xmm6,%xmm2
	vpcmpeqb (%ecx),%xmm6,%xmm7
	vpcmpeqw %xmm4,%xmm6,%xmm2
	vpcmpeqw (%ecx),%xmm6,%xmm7
	vpcmpeqd %xmm4,%xmm6,%xmm2
	vpcmpeqd (%ecx),%xmm6,%xmm7
	vpcmpeqq %xmm4,%xmm6,%xmm2
	vpcmpeqq (%ecx),%xmm6,%xmm7
	vpcmpgtb %xmm4,%xmm6,%xmm2
	vpcmpgtb (%ecx),%xmm6,%xmm7
	vpcmpgtw %xmm4,%xmm6,%xmm2
	vpcmpgtw (%ecx),%xmm6,%xmm7
	vpcmpgtd %xmm4,%xmm6,%xmm2
	vpcmpgtd (%ecx),%xmm6,%xmm7
	vpcmpgtq %xmm4,%xmm6,%xmm2
	vpcmpgtq (%ecx),%xmm6,%xmm7
	vpermilpd %xmm4,%xmm6,%xmm2
	vpermilpd (%ecx),%xmm6,%xmm7
	vpermilps %xmm4,%xmm6,%xmm2
	vpermilps (%ecx),%xmm6,%xmm7
	vphaddw %xmm4,%xmm6,%xmm2
	vphaddw (%ecx),%xmm6,%xmm7
	vphaddd %xmm4,%xmm6,%xmm2
	vphaddd (%ecx),%xmm6,%xmm7
	vphaddsw %xmm4,%xmm6,%xmm2
	vphaddsw (%ecx),%xmm6,%xmm7
	vphsubw %xmm4,%xmm6,%xmm2
	vphsubw (%ecx),%xmm6,%xmm7
	vphsubd %xmm4,%xmm6,%xmm2
	vphsubd (%ecx),%xmm6,%xmm7
	vphsubsw %xmm4,%xmm6,%xmm2
	vphsubsw (%ecx),%xmm6,%xmm7
	vpmaddwd %xmm4,%xmm6,%xmm2
	vpmaddwd (%ecx),%xmm6,%xmm7
	vpmaddubsw %xmm4,%xmm6,%xmm2
	vpmaddubsw (%ecx),%xmm6,%xmm7
	vpmaxsb %xmm4,%xmm6,%xmm2
	vpmaxsb (%ecx),%xmm6,%xmm7
	vpmaxsw %xmm4,%xmm6,%xmm2
	vpmaxsw (%ecx),%xmm6,%xmm7
	vpmaxsd %xmm4,%xmm6,%xmm2
	vpmaxsd (%ecx),%xmm6,%xmm7
	vpmaxub %xmm4,%xmm6,%xmm2
	vpmaxub (%ecx),%xmm6,%xmm7
	vpmaxuw %xmm4,%xmm6,%xmm2
	vpmaxuw (%ecx),%xmm6,%xmm7
	vpmaxud %xmm4,%xmm6,%xmm2
	vpmaxud (%ecx),%xmm6,%xmm7
	vpminsb %xmm4,%xmm6,%xmm2
	vpminsb (%ecx),%xmm6,%xmm7
	vpminsw %xmm4,%xmm6,%xmm2
	vpminsw (%ecx),%xmm6,%xmm7
	vpminsd %xmm4,%xmm6,%xmm2
	vpminsd (%ecx),%xmm6,%xmm7
	vpminub %xmm4,%xmm6,%xmm2
	vpminub (%ecx),%xmm6,%xmm7
	vpminuw %xmm4,%xmm6,%xmm2
	vpminuw (%ecx),%xmm6,%xmm7
	vpminud %xmm4,%xmm6,%xmm2
	vpminud (%ecx),%xmm6,%xmm7
	vpmulhuw %xmm4,%xmm6,%xmm2
	vpmulhuw (%ecx),%xmm6,%xmm7
	vpmulhrsw %xmm4,%xmm6,%xmm2
	vpmulhrsw (%ecx),%xmm6,%xmm7
	vpmulhw %xmm4,%xmm6,%xmm2
	vpmulhw (%ecx),%xmm6,%xmm7
	vpmullw %xmm4,%xmm6,%xmm2
	vpmullw (%ecx),%xmm6,%xmm7
	vpmulld %xmm4,%xmm6,%xmm2
	vpmulld (%ecx),%xmm6,%xmm7
	vpmuludq %xmm4,%xmm6,%xmm2
	vpmuludq (%ecx),%xmm6,%xmm7
	vpmuldq %xmm4,%xmm6,%xmm2
	vpmuldq (%ecx),%xmm6,%xmm7
	vpor %xmm4,%xmm6,%xmm2
	vpor (%ecx),%xmm6,%xmm7
	vpsadbw %xmm4,%xmm6,%xmm2
	vpsadbw (%ecx),%xmm6,%xmm7
	vpshufb %xmm4,%xmm6,%xmm2
	vpshufb (%ecx),%xmm6,%xmm7
	vpsignb %xmm4,%xmm6,%xmm2
	vpsignb (%ecx),%xmm6,%xmm7
	vpsignw %xmm4,%xmm6,%xmm2
	vpsignw (%ecx),%xmm6,%xmm7
	vpsignd %xmm4,%xmm6,%xmm2
	vpsignd (%ecx),%xmm6,%xmm7
	vpsllw %xmm4,%xmm6,%xmm2
	vpsllw (%ecx),%xmm6,%xmm7
	vpslld %xmm4,%xmm6,%xmm2
	vpslld (%ecx),%xmm6,%xmm7
	vpsllq %xmm4,%xmm6,%xmm2
	vpsllq (%ecx),%xmm6,%xmm7
	vpsraw %xmm4,%xmm6,%xmm2
	vpsraw (%ecx),%xmm6,%xmm7
	vpsrad %xmm4,%xmm6,%xmm2
	vpsrad (%ecx),%xmm6,%xmm7
	vpsrlw %xmm4,%xmm6,%xmm2
	vpsrlw (%ecx),%xmm6,%xmm7
	vpsrld %xmm4,%xmm6,%xmm2
	vpsrld (%ecx),%xmm6,%xmm7
	vpsrlq %xmm4,%xmm6,%xmm2
	vpsrlq (%ecx),%xmm6,%xmm7
	vpsubb %xmm4,%xmm6,%xmm2
	vpsubb (%ecx),%xmm6,%xmm7
	vpsubw %xmm4,%xmm6,%xmm2
	vpsubw (%ecx),%xmm6,%xmm7
	vpsubd %xmm4,%xmm6,%xmm2
	vpsubd (%ecx),%xmm6,%xmm7
	vpsubq %xmm4,%xmm6,%xmm2
	vpsubq (%ecx),%xmm6,%xmm7
	vpsubsb %xmm4,%xmm6,%xmm2
	vpsubsb (%ecx),%xmm6,%xmm7
	vpsubsw %xmm4,%xmm6,%xmm2
	vpsubsw (%ecx),%xmm6,%xmm7
	vpsubusb %xmm4,%xmm6,%xmm2
	vpsubusb (%ecx),%xmm6,%xmm7
	vpsubusw %xmm4,%xmm6,%xmm2
	vpsubusw (%ecx),%xmm6,%xmm7
	vpunpckhbw %xmm4,%xmm6,%xmm2
	vpunpckhbw (%ecx),%xmm6,%xmm7
	vpunpckhwd %xmm4,%xmm6,%xmm2
	vpunpckhwd (%ecx),%xmm6,%xmm7
	vpunpckhdq %xmm4,%xmm6,%xmm2
	vpunpckhdq (%ecx),%xmm6,%xmm7
	vpunpckhqdq %xmm4,%xmm6,%xmm2
	vpunpckhqdq (%ecx),%xmm6,%xmm7
	vpunpcklbw %xmm4,%xmm6,%xmm2
	vpunpcklbw (%ecx),%xmm6,%xmm7
	vpunpcklwd %xmm4,%xmm6,%xmm2
	vpunpcklwd (%ecx),%xmm6,%xmm7
	vpunpckldq %xmm4,%xmm6,%xmm2
	vpunpckldq (%ecx),%xmm6,%xmm7
	vpunpcklqdq %xmm4,%xmm6,%xmm2
	vpunpcklqdq (%ecx),%xmm6,%xmm7
	vpxor %xmm4,%xmm6,%xmm2
	vpxor (%ecx),%xmm6,%xmm7
	vsubpd %xmm4,%xmm6,%xmm2
	vsubpd (%ecx),%xmm6,%xmm7
	vsubps %xmm4,%xmm6,%xmm2
	vsubps (%ecx),%xmm6,%xmm7
	vunpckhpd %xmm4,%xmm6,%xmm2
	vunpckhpd (%ecx),%xmm6,%xmm7
	vunpckhps %xmm4,%xmm6,%xmm2
	vunpckhps (%ecx),%xmm6,%xmm7
	vunpcklpd %xmm4,%xmm6,%xmm2
	vunpcklpd (%ecx),%xmm6,%xmm7
	vunpcklps %xmm4,%xmm6,%xmm2
	vunpcklps (%ecx),%xmm6,%xmm7
	vxorpd %xmm4,%xmm6,%xmm2
	vxorpd (%ecx),%xmm6,%xmm7
	vxorps %xmm4,%xmm6,%xmm2
	vxorps (%ecx),%xmm6,%xmm7
	vaesenc %xmm4,%xmm6,%xmm2
	vaesenc (%ecx),%xmm6,%xmm7
	vaesenclast %xmm4,%xmm6,%xmm2
	vaesenclast (%ecx),%xmm6,%xmm7
	vaesdec %xmm4,%xmm6,%xmm2
	vaesdec (%ecx),%xmm6,%xmm7
	vaesdeclast %xmm4,%xmm6,%xmm2
	vaesdeclast (%ecx),%xmm6,%xmm7
	vcmpeqpd %xmm4,%xmm6,%xmm2
	vcmpeqpd (%ecx),%xmm6,%xmm7
	vcmpltpd %xmm4,%xmm6,%xmm2
	vcmpltpd (%ecx),%xmm6,%xmm7
	vcmplepd %xmm4,%xmm6,%xmm2
	vcmplepd (%ecx),%xmm6,%xmm7
	vcmpunordpd %xmm4,%xmm6,%xmm2
	vcmpunordpd (%ecx),%xmm6,%xmm7
	vcmpneqpd %xmm4,%xmm6,%xmm2
	vcmpneqpd (%ecx),%xmm6,%xmm7
	vcmpnltpd %xmm4,%xmm6,%xmm2
	vcmpnltpd (%ecx),%xmm6,%xmm7
	vcmpnlepd %xmm4,%xmm6,%xmm2
	vcmpnlepd (%ecx),%xmm6,%xmm7
	vcmpordpd %xmm4,%xmm6,%xmm2
	vcmpordpd (%ecx),%xmm6,%xmm7
	vcmpeq_uqpd %xmm4,%xmm6,%xmm2
	vcmpeq_uqpd (%ecx),%xmm6,%xmm7
	vcmpngepd %xmm4,%xmm6,%xmm2
	vcmpngepd (%ecx),%xmm6,%xmm7
	vcmpngtpd %xmm4,%xmm6,%xmm2
	vcmpngtpd (%ecx),%xmm6,%xmm7
	vcmpfalsepd %xmm4,%xmm6,%xmm2
	vcmpfalsepd (%ecx),%xmm6,%xmm7
	vcmpneq_oqpd %xmm4,%xmm6,%xmm2
	vcmpneq_oqpd (%ecx),%xmm6,%xmm7
	vcmpgepd %xmm4,%xmm6,%xmm2
	vcmpgepd (%ecx),%xmm6,%xmm7
	vcmpgtpd %xmm4,%xmm6,%xmm2
	vcmpgtpd (%ecx),%xmm6,%xmm7
	vcmptruepd %xmm4,%xmm6,%xmm2
	vcmptruepd (%ecx),%xmm6,%xmm7
	vcmpeq_ospd %xmm4,%xmm6,%xmm2
	vcmpeq_ospd (%ecx),%xmm6,%xmm7
	vcmplt_oqpd %xmm4,%xmm6,%xmm2
	vcmplt_oqpd (%ecx),%xmm6,%xmm7
	vcmple_oqpd %xmm4,%xmm6,%xmm2
	vcmple_oqpd (%ecx),%xmm6,%xmm7
	vcmpunord_spd %xmm4,%xmm6,%xmm2
	vcmpunord_spd (%ecx),%xmm6,%xmm7
	vcmpneq_uspd %xmm4,%xmm6,%xmm2
	vcmpneq_uspd (%ecx),%xmm6,%xmm7
	vcmpnlt_uqpd %xmm4,%xmm6,%xmm2
	vcmpnlt_uqpd (%ecx),%xmm6,%xmm7
	vcmpnle_uqpd %xmm4,%xmm6,%xmm2
	vcmpnle_uqpd (%ecx),%xmm6,%xmm7
	vcmpord_spd %xmm4,%xmm6,%xmm2
	vcmpord_spd (%ecx),%xmm6,%xmm7
	vcmpeq_uspd %xmm4,%xmm6,%xmm2
	vcmpeq_uspd (%ecx),%xmm6,%xmm7
	vcmpnge_uqpd %xmm4,%xmm6,%xmm2
	vcmpnge_uqpd (%ecx),%xmm6,%xmm7
	vcmpngt_uqpd %xmm4,%xmm6,%xmm2
	vcmpngt_uqpd (%ecx),%xmm6,%xmm7
	vcmpfalse_ospd %xmm4,%xmm6,%xmm2
	vcmpfalse_ospd (%ecx),%xmm6,%xmm7
	vcmpneq_ospd %xmm4,%xmm6,%xmm2
	vcmpneq_ospd (%ecx),%xmm6,%xmm7
	vcmpge_oqpd %xmm4,%xmm6,%xmm2
	vcmpge_oqpd (%ecx),%xmm6,%xmm7
	vcmpgt_oqpd %xmm4,%xmm6,%xmm2
	vcmpgt_oqpd (%ecx),%xmm6,%xmm7
	vcmptrue_uspd %xmm4,%xmm6,%xmm2
	vcmptrue_uspd (%ecx),%xmm6,%xmm7
	vcmpeqps %xmm4,%xmm6,%xmm2
	vcmpeqps (%ecx),%xmm6,%xmm7
	vcmpltps %xmm4,%xmm6,%xmm2
	vcmpltps (%ecx),%xmm6,%xmm7
	vcmpleps %xmm4,%xmm6,%xmm2
	vcmpleps (%ecx),%xmm6,%xmm7
	vcmpunordps %xmm4,%xmm6,%xmm2
	vcmpunordps (%ecx),%xmm6,%xmm7
	vcmpneqps %xmm4,%xmm6,%xmm2
	vcmpneqps (%ecx),%xmm6,%xmm7
	vcmpnltps %xmm4,%xmm6,%xmm2
	vcmpnltps (%ecx),%xmm6,%xmm7
	vcmpnleps %xmm4,%xmm6,%xmm2
	vcmpnleps (%ecx),%xmm6,%xmm7
	vcmpordps %xmm4,%xmm6,%xmm2
	vcmpordps (%ecx),%xmm6,%xmm7
	vcmpeq_uqps %xmm4,%xmm6,%xmm2
	vcmpeq_uqps (%ecx),%xmm6,%xmm7
	vcmpngeps %xmm4,%xmm6,%xmm2
	vcmpngeps (%ecx),%xmm6,%xmm7
	vcmpngtps %xmm4,%xmm6,%xmm2
	vcmpngtps (%ecx),%xmm6,%xmm7
	vcmpfalseps %xmm4,%xmm6,%xmm2
	vcmpfalseps (%ecx),%xmm6,%xmm7
	vcmpneq_oqps %xmm4,%xmm6,%xmm2
	vcmpneq_oqps (%ecx),%xmm6,%xmm7
	vcmpgeps %xmm4,%xmm6,%xmm2
	vcmpgeps (%ecx),%xmm6,%xmm7
	vcmpgtps %xmm4,%xmm6,%xmm2
	vcmpgtps (%ecx),%xmm6,%xmm7
	vcmptrueps %xmm4,%xmm6,%xmm2
	vcmptrueps (%ecx),%xmm6,%xmm7
	vcmpeq_osps %xmm4,%xmm6,%xmm2
	vcmpeq_osps (%ecx),%xmm6,%xmm7
	vcmplt_oqps %xmm4,%xmm6,%xmm2
	vcmplt_oqps (%ecx),%xmm6,%xmm7
	vcmple_oqps %xmm4,%xmm6,%xmm2
	vcmple_oqps (%ecx),%xmm6,%xmm7
	vcmpunord_sps %xmm4,%xmm6,%xmm2
	vcmpunord_sps (%ecx),%xmm6,%xmm7
	vcmpneq_usps %xmm4,%xmm6,%xmm2
	vcmpneq_usps (%ecx),%xmm6,%xmm7
	vcmpnlt_uqps %xmm4,%xmm6,%xmm2
	vcmpnlt_uqps (%ecx),%xmm6,%xmm7
	vcmpnle_uqps %xmm4,%xmm6,%xmm2
	vcmpnle_uqps (%ecx),%xmm6,%xmm7
	vcmpord_sps %xmm4,%xmm6,%xmm2
	vcmpord_sps (%ecx),%xmm6,%xmm7
	vcmpeq_usps %xmm4,%xmm6,%xmm2
	vcmpeq_usps (%ecx),%xmm6,%xmm7
	vcmpnge_uqps %xmm4,%xmm6,%xmm2
	vcmpnge_uqps (%ecx),%xmm6,%xmm7
	vcmpngt_uqps %xmm4,%xmm6,%xmm2
	vcmpngt_uqps (%ecx),%xmm6,%xmm7
	vcmpfalse_osps %xmm4,%xmm6,%xmm2
	vcmpfalse_osps (%ecx),%xmm6,%xmm7
	vcmpneq_osps %xmm4,%xmm6,%xmm2
	vcmpneq_osps (%ecx),%xmm6,%xmm7
	vcmpge_oqps %xmm4,%xmm6,%xmm2
	vcmpge_oqps (%ecx),%xmm6,%xmm7
	vcmpgt_oqps %xmm4,%xmm6,%xmm2
	vcmpgt_oqps (%ecx),%xmm6,%xmm7
	vcmptrue_usps %xmm4,%xmm6,%xmm2
	vcmptrue_usps (%ecx),%xmm6,%xmm7
    vgf2p8mulb %xmm4, %xmm5, %xmm6
	vgf2p8mulb (%ecx), %xmm5, %xmm6
	vgf2p8mulb -123456(%esp,%esi,8), %xmm5, %xmm6
	vgf2p8mulb 2032(%edx), %xmm5, %xmm6
	vgf2p8mulb 2048(%edx), %xmm5, %xmm6
	vgf2p8mulb -2048(%edx), %xmm5, %xmm6
	vgf2p8mulb -2064(%edx), %xmm5, %xmm6

# Tests for op mem128, xmm, xmm
	vmaskmovps (%ecx),%xmm4,%xmm6
	vmaskmovpd (%ecx),%xmm4,%xmm6

# Tests for op imm8, xmm/mem128, xmm
	vaeskeygenassist $7,%xmm4,%xmm6
	vaeskeygenassist $7,(%ecx),%xmm6
	vpcmpestri $7,%xmm4,%xmm6
	vpcmpestri $7,(%ecx),%xmm6
	vpcmpestrm $7,%xmm4,%xmm6
	vpcmpestrm $7,(%ecx),%xmm6
	vpcmpistri $7,%xmm4,%xmm6
	vpcmpistri $7,(%ecx),%xmm6
	vpcmpistrm $7,%xmm4,%xmm6
	vpcmpistrm $7,(%ecx),%xmm6
	vpermilpd $7,%xmm4,%xmm6
	vpermilpd $7,(%ecx),%xmm6
	vpermilps $7,%xmm4,%xmm6
	vpermilps $7,(%ecx),%xmm6
	vpshufd $7,%xmm4,%xmm6
	vpshufd $7,(%ecx),%xmm6
	vpshufhw $7,%xmm4,%xmm6
	vpshufhw $7,(%ecx),%xmm6
	vpshuflw $7,%xmm4,%xmm6
	vpshuflw $7,(%ecx),%xmm6
	vroundpd $7,%xmm4,%xmm6
	vroundpd $7,(%ecx),%xmm6
	vroundps $7,%xmm4,%xmm6
	vroundps $7,(%ecx),%xmm6

# Tests for op xmm, xmm, mem128
	vmaskmovps %xmm4,%xmm6,(%ecx)
	vmaskmovpd %xmm4,%xmm6,(%ecx)

# Tests for op imm8, xmm/mem128, xmm, xmm
	vblendpd $7,%xmm4,%xmm6,%xmm2
	vblendpd $7,(%ecx),%xmm6,%xmm2
	vblendps $7,%xmm4,%xmm6,%xmm2
	vblendps $7,(%ecx),%xmm6,%xmm2
	vcmppd $7,%xmm4,%xmm6,%xmm2
	vcmppd $7,(%ecx),%xmm6,%xmm2
	vcmpps $7,%xmm4,%xmm6,%xmm2
	vcmpps $7,(%ecx),%xmm6,%xmm2
	vdppd $7,%xmm4,%xmm6,%xmm2
	vdppd $7,(%ecx),%xmm6,%xmm2
	vdpps $7,%xmm4,%xmm6,%xmm2
	vdpps $7,(%ecx),%xmm6,%xmm2
	vmpsadbw $7,%xmm4,%xmm6,%xmm2
	vmpsadbw $7,(%ecx),%xmm6,%xmm2
	vpalignr $7,%xmm4,%xmm6,%xmm2
	vpalignr $7,(%ecx),%xmm6,%xmm2
	vpblendw $7,%xmm4,%xmm6,%xmm2
	vpblendw $7,(%ecx),%xmm6,%xmm2
	vpclmulqdq $7,%xmm4,%xmm6,%xmm2
	vpclmulqdq $7,(%ecx),%xmm6,%xmm2
	vshufpd $7,%xmm4,%xmm6,%xmm2
	vshufpd $7,(%ecx),%xmm6,%xmm2
	vshufps $7,%xmm4,%xmm6,%xmm2
	vshufps $7,(%ecx),%xmm6,%xmm2
    vgf2p8affineqb $0xab, %xmm4, %xmm5, %xmm6
	vgf2p8affineqb $123, %xmm4, %xmm5, %xmm6
	vgf2p8affineqb $123, (%ecx), %xmm5, %xmm6
	vgf2p8affineqb $123, -123456(%esp,%esi,8), %xmm5, %xmm6
	vgf2p8affineqb $123, 2032(%edx), %xmm5, %xmm6
	vgf2p8affineqb $123, 2048(%edx), %xmm5, %xmm6
	vgf2p8affineqb $123, -2048(%edx), %xmm5, %xmm6
	vgf2p8affineqb $123, -2064(%edx), %xmm5, %xmm6
	vgf2p8affineinvqb $0xab, %xmm4, %xmm5, %xmm6
	vgf2p8affineinvqb $123, %xmm4, %xmm5, %xmm6
	vgf2p8affineinvqb $123, (%ecx), %xmm5, %xmm6
	vgf2p8affineinvqb $123, -123456(%esp,%esi,8), %xmm5, %xmm6
	vgf2p8affineinvqb $123, 2032(%edx), %xmm5, %xmm6
	vgf2p8affineinvqb $123, 2048(%edx), %xmm5, %xmm6
	vgf2p8affineinvqb $123, -2048(%edx), %xmm5, %xmm6
	vgf2p8affineinvqb $123, -2064(%edx), %xmm5, %xmm6

# Tests for op xmm, xmm/mem128, xmm, xmm
	vblendvpd %xmm4,%xmm6,%xmm2,%xmm7
	vblendvpd %xmm4,(%ecx),%xmm2,%xmm7
	vblendvps %xmm4,%xmm6,%xmm2,%xmm7
	vblendvps %xmm4,(%ecx),%xmm2,%xmm7
	vpblendvb %xmm4,%xmm6,%xmm2,%xmm7
	vpblendvb %xmm4,(%ecx),%xmm2,%xmm7

# Tests for op mem64, ymm
	vbroadcastsd (%ecx),%ymm4

# Tests for op xmm/mem64, xmm
	vcomisd %xmm4,%xmm6
	vcomisd (%ecx),%xmm4
	vcvtdq2pd %xmm4,%xmm6
	vcvtdq2pd (%ecx),%xmm4
	vcvtps2pd %xmm4,%xmm6
	vcvtps2pd (%ecx),%xmm4
	vmovddup %xmm4,%xmm6
	vmovddup (%ecx),%xmm4
	vpmovsxbw %xmm4,%xmm6
	vpmovsxbw (%ecx),%xmm4
	vpmovsxwd %xmm4,%xmm6
	vpmovsxwd (%ecx),%xmm4
	vpmovsxdq %xmm4,%xmm6
	vpmovsxdq (%ecx),%xmm4
	vpmovzxbw %xmm4,%xmm6
	vpmovzxbw (%ecx),%xmm4
	vpmovzxwd %xmm4,%xmm6
	vpmovzxwd (%ecx),%xmm4
	vpmovzxdq %xmm4,%xmm6
	vpmovzxdq (%ecx),%xmm4
	vucomisd %xmm4,%xmm6
	vucomisd (%ecx),%xmm4

# Tests for op mem64, xmm
	vmovsd (%ecx),%xmm4

# Tests for op xmm, mem64
	vmovlpd %xmm4,(%ecx)
	vmovlps %xmm4,(%ecx)
	vmovhpd %xmm4,(%ecx)
	vmovhps %xmm4,(%ecx)
	vmovsd %xmm4,(%ecx)

# Tests for op xmm, regq/mem64
# Tests for op regq/mem64, xmm
	vmovq %xmm4,(%ecx)
	vmovq (%ecx),%xmm4

# Tests for op xmm/mem64, regl
	vcvtsd2si %xmm4,%ecx
	vcvtsd2si (%ecx),%ecx
	vcvttsd2si %xmm4,%ecx
	vcvttsd2si (%ecx),%ecx

# Tests for op mem64, xmm, xmm
	vmovlpd (%ecx),%xmm4,%xmm6
	vmovlps (%ecx),%xmm4,%xmm6
	vmovhpd (%ecx),%xmm4,%xmm6
	vmovhps (%ecx),%xmm4,%xmm6

# Tests for op imm8, xmm/mem64, xmm, xmm
	vcmpsd $7,%xmm4,%xmm6,%xmm2
	vcmpsd $7,(%ecx),%xmm6,%xmm2
	vroundsd $7,%xmm4,%xmm6,%xmm2
	vroundsd $7,(%ecx),%xmm6,%xmm2

# Tests for op xmm/mem64, xmm, xmm
	vaddsd %xmm4,%xmm6,%xmm2
	vaddsd (%ecx),%xmm6,%xmm2
	vcvtsd2ss %xmm4,%xmm6,%xmm2
	vcvtsd2ss (%ecx),%xmm6,%xmm2
	vdivsd %xmm4,%xmm6,%xmm2
	vdivsd (%ecx),%xmm6,%xmm2
	vmaxsd %xmm4,%xmm6,%xmm2
	vmaxsd (%ecx),%xmm6,%xmm2
	vminsd %xmm4,%xmm6,%xmm2
	vminsd (%ecx),%xmm6,%xmm2
	vmulsd %xmm4,%xmm6,%xmm2
	vmulsd (%ecx),%xmm6,%xmm2
	vsqrtsd %xmm4,%xmm6,%xmm2
	vsqrtsd (%ecx),%xmm6,%xmm2
	vsubsd %xmm4,%xmm6,%xmm2
	vsubsd (%ecx),%xmm6,%xmm2
	vcmpeqsd %xmm4,%xmm6,%xmm2
	vcmpeqsd (%ecx),%xmm6,%xmm2
	vcmpeq_oqsd %xmm4,%xmm6,%xmm2
	vcmpeq_oqsd (%ecx),%xmm6,%xmm2
	vcmpltsd %xmm4,%xmm6,%xmm2
	vcmpltsd (%ecx),%xmm6,%xmm2
	vcmplt_ossd %xmm4,%xmm6,%xmm2
	vcmplt_ossd (%ecx),%xmm6,%xmm2
	vcmplesd %xmm4,%xmm6,%xmm2
	vcmplesd (%ecx),%xmm6,%xmm2
	vcmple_ossd %xmm4,%xmm6,%xmm2
	vcmple_ossd (%ecx),%xmm6,%xmm2
	vcmpunordsd %xmm4,%xmm6,%xmm2
	vcmpunordsd (%ecx),%xmm6,%xmm2
	vcmpunord_qsd %xmm4,%xmm6,%xmm2
	vcmpunord_qsd (%ecx),%xmm6,%xmm2
	vcmpneqsd %xmm4,%xmm6,%xmm2
	vcmpneqsd (%ecx),%xmm6,%xmm2
	vcmpneq_uqsd %xmm4,%xmm6,%xmm2
	vcmpneq_uqsd (%ecx),%xmm6,%xmm2
	vcmpnltsd %xmm4,%xmm6,%xmm2
	vcmpnltsd (%ecx),%xmm6,%xmm2
	vcmpnlt_ussd %xmm4,%xmm6,%xmm2
	vcmpnlt_ussd (%ecx),%xmm6,%xmm2
	vcmpnlesd %xmm4,%xmm6,%xmm2
	vcmpnlesd (%ecx),%xmm6,%xmm2
	vcmpnle_ussd %xmm4,%xmm6,%xmm2
	vcmpnle_ussd (%ecx),%xmm6,%xmm2
	vcmpordsd %xmm4,%xmm6,%xmm2
	vcmpordsd (%ecx),%xmm6,%xmm2
	vcmpord_qsd %xmm4,%xmm6,%xmm2
	vcmpord_qsd (%ecx),%xmm6,%xmm2
	vcmpeq_uqsd %xmm4,%xmm6,%xmm2
	vcmpeq_uqsd (%ecx),%xmm6,%xmm2
	vcmpngesd %xmm4,%xmm6,%xmm2
	vcmpngesd (%ecx),%xmm6,%xmm2
	vcmpnge_ussd %xmm4,%xmm6,%xmm2
	vcmpnge_ussd (%ecx),%xmm6,%xmm2
	vcmpngtsd %xmm4,%xmm6,%xmm2
	vcmpngtsd (%ecx),%xmm6,%xmm2
	vcmpngt_ussd %xmm4,%xmm6,%xmm2
	vcmpngt_ussd (%ecx),%xmm6,%xmm2
	vcmpfalsesd %xmm4,%xmm6,%xmm2
	vcmpfalsesd (%ecx),%xmm6,%xmm2
	vcmpfalse_oqsd %xmm4,%xmm6,%xmm2
	vcmpfalse_oqsd (%ecx),%xmm6,%xmm2
	vcmpneq_oqsd %xmm4,%xmm6,%xmm2
	vcmpneq_oqsd (%ecx),%xmm6,%xmm2
	vcmpgesd %xmm4,%xmm6,%xmm2
	vcmpgesd (%ecx),%xmm6,%xmm2
	vcmpge_ossd %xmm4,%xmm6,%xmm2
	vcmpge_ossd (%ecx),%xmm6,%xmm2
	vcmpgtsd %xmm4,%xmm6,%xmm2
	vcmpgtsd (%ecx),%xmm6,%xmm2
	vcmpgt_ossd %xmm4,%xmm6,%xmm2
	vcmpgt_ossd (%ecx),%xmm6,%xmm2
	vcmptruesd %xmm4,%xmm6,%xmm2
	vcmptruesd (%ecx),%xmm6,%xmm2
	vcmptrue_uqsd %xmm4,%xmm6,%xmm2
	vcmptrue_uqsd (%ecx),%xmm6,%xmm2
	vcmpeq_ossd %xmm4,%xmm6,%xmm2
	vcmpeq_ossd (%ecx),%xmm6,%xmm2
	vcmplt_oqsd %xmm4,%xmm6,%xmm2
	vcmplt_oqsd (%ecx),%xmm6,%xmm2
	vcmple_oqsd %xmm4,%xmm6,%xmm2
	vcmple_oqsd (%ecx),%xmm6,%xmm2
	vcmpunord_ssd %xmm4,%xmm6,%xmm2
	vcmpunord_ssd (%ecx),%xmm6,%xmm2
	vcmpneq_ussd %xmm4,%xmm6,%xmm2
	vcmpneq_ussd (%ecx),%xmm6,%xmm2
	vcmpnlt_uqsd %xmm4,%xmm6,%xmm2
	vcmpnlt_uqsd (%ecx),%xmm6,%xmm2
	vcmpnle_uqsd %xmm4,%xmm6,%xmm2
	vcmpnle_uqsd (%ecx),%xmm6,%xmm2
	vcmpord_ssd %xmm4,%xmm6,%xmm2
	vcmpord_ssd (%ecx),%xmm6,%xmm2
	vcmpeq_ussd %xmm4,%xmm6,%xmm2
	vcmpeq_ussd (%ecx),%xmm6,%xmm2
	vcmpnge_uqsd %xmm4,%xmm6,%xmm2
	vcmpnge_uqsd (%ecx),%xmm6,%xmm2
	vcmpngt_uqsd %xmm4,%xmm6,%xmm2
	vcmpngt_uqsd (%ecx),%xmm6,%xmm2
	vcmpfalse_ossd %xmm4,%xmm6,%xmm2
	vcmpfalse_ossd (%ecx),%xmm6,%xmm2
	vcmpneq_ossd %xmm4,%xmm6,%xmm2
	vcmpneq_ossd (%ecx),%xmm6,%xmm2
	vcmpge_oqsd %xmm4,%xmm6,%xmm2
	vcmpge_oqsd (%ecx),%xmm6,%xmm2
	vcmpgt_oqsd %xmm4,%xmm6,%xmm2
	vcmpgt_oqsd (%ecx),%xmm6,%xmm2
	vcmptrue_ussd %xmm4,%xmm6,%xmm2
	vcmptrue_ussd (%ecx),%xmm6,%xmm2

# Tests for op mem64
	vldmxcsr (%ecx)
	vstmxcsr (%ecx)

# Tests for op xmm/mem32, xmm, xmm
	vaddss %xmm4,%xmm6,%xmm2
	vaddss (%ecx),%xmm6,%xmm2
	vcvtss2sd %xmm4,%xmm6,%xmm2
	vcvtss2sd (%ecx),%xmm6,%xmm2
	vdivss %xmm4,%xmm6,%xmm2
	vdivss (%ecx),%xmm6,%xmm2
	vmaxss %xmm4,%xmm6,%xmm2
	vmaxss (%ecx),%xmm6,%xmm2
	vminss %xmm4,%xmm6,%xmm2
	vminss (%ecx),%xmm6,%xmm2
	vmulss %xmm4,%xmm6,%xmm2
	vmulss (%ecx),%xmm6,%xmm2
	vrcpss %xmm4,%xmm6,%xmm2
	vrcpss (%ecx),%xmm6,%xmm2
	vrsqrtss %xmm4,%xmm6,%xmm2
	vrsqrtss (%ecx),%xmm6,%xmm2
	vsqrtss %xmm4,%xmm6,%xmm2
	vsqrtss (%ecx),%xmm6,%xmm2
	vsubss %xmm4,%xmm6,%xmm2
	vsubss (%ecx),%xmm6,%xmm2
	vcmpeqss %xmm4,%xmm6,%xmm2
	vcmpeqss (%ecx),%xmm6,%xmm2
	vcmpeq_oqss %xmm4,%xmm6,%xmm2
	vcmpeq_oqss (%ecx),%xmm6,%xmm2
	vcmpltss %xmm4,%xmm6,%xmm2
	vcmpltss (%ecx),%xmm6,%xmm2
	vcmplt_osss %xmm4,%xmm6,%xmm2
	vcmplt_osss (%ecx),%xmm6,%xmm2
	vcmpless %xmm4,%xmm6,%xmm2
	vcmpless (%ecx),%xmm6,%xmm2
	vcmple_osss %xmm4,%xmm6,%xmm2
	vcmple_osss (%ecx),%xmm6,%xmm2
	vcmpunordss %xmm4,%xmm6,%xmm2
	vcmpunordss (%ecx),%xmm6,%xmm2
	vcmpunord_qss %xmm4,%xmm6,%xmm2
	vcmpunord_qss (%ecx),%xmm6,%xmm2
	vcmpneqss %xmm4,%xmm6,%xmm2
	vcmpneqss (%ecx),%xmm6,%xmm2
	vcmpneq_uqss %xmm4,%xmm6,%xmm2
	vcmpneq_uqss (%ecx),%xmm6,%xmm2
	vcmpnltss %xmm4,%xmm6,%xmm2
	vcmpnltss (%ecx),%xmm6,%xmm2
	vcmpnlt_usss %xmm4,%xmm6,%xmm2
	vcmpnlt_usss (%ecx),%xmm6,%xmm2
	vcmpnless %xmm4,%xmm6,%xmm2
	vcmpnless (%ecx),%xmm6,%xmm2
	vcmpnle_usss %xmm4,%xmm6,%xmm2
	vcmpnle_usss (%ecx),%xmm6,%xmm2
	vcmpordss %xmm4,%xmm6,%xmm2
	vcmpordss (%ecx),%xmm6,%xmm2
	vcmpord_qss %xmm4,%xmm6,%xmm2
	vcmpord_qss (%ecx),%xmm6,%xmm2
	vcmpeq_uqss %xmm4,%xmm6,%xmm2
	vcmpeq_uqss (%ecx),%xmm6,%xmm2
	vcmpngess %xmm4,%xmm6,%xmm2
	vcmpngess (%ecx),%xmm6,%xmm2
	vcmpnge_usss %xmm4,%xmm6,%xmm2
	vcmpnge_usss (%ecx),%xmm6,%xmm2
	vcmpngtss %xmm4,%xmm6,%xmm2
	vcmpngtss (%ecx),%xmm6,%xmm2
	vcmpngt_usss %xmm4,%xmm6,%xmm2
	vcmpngt_usss (%ecx),%xmm6,%xmm2
	vcmpfalsess %xmm4,%xmm6,%xmm2
	vcmpfalsess (%ecx),%xmm6,%xmm2
	vcmpfalse_oqss %xmm4,%xmm6,%xmm2
	vcmpfalse_oqss (%ecx),%xmm6,%xmm2
	vcmpneq_oqss %xmm4,%xmm6,%xmm2
	vcmpneq_oqss (%ecx),%xmm6,%xmm2
	vcmpgess %xmm4,%xmm6,%xmm2
	vcmpgess (%ecx),%xmm6,%xmm2
	vcmpge_osss %xmm4,%xmm6,%xmm2
	vcmpge_osss (%ecx),%xmm6,%xmm2
	vcmpgtss %xmm4,%xmm6,%xmm2
	vcmpgtss (%ecx),%xmm6,%xmm2
	vcmpgt_osss %xmm4,%xmm6,%xmm2
	vcmpgt_osss (%ecx),%xmm6,%xmm2
	vcmptruess %xmm4,%xmm6,%xmm2
	vcmptruess (%ecx),%xmm6,%xmm2
	vcmptrue_uqss %xmm4,%xmm6,%xmm2
	vcmptrue_uqss (%ecx),%xmm6,%xmm2
	vcmpeq_osss %xmm4,%xmm6,%xmm2
	vcmpeq_osss (%ecx),%xmm6,%xmm2
	vcmplt_oqss %xmm4,%xmm6,%xmm2
	vcmplt_oqss (%ecx),%xmm6,%xmm2
	vcmple_oqss %xmm4,%xmm6,%xmm2
	vcmple_oqss (%ecx),%xmm6,%xmm2
	vcmpunord_sss %xmm4,%xmm6,%xmm2
	vcmpunord_sss (%ecx),%xmm6,%xmm2
	vcmpneq_usss %xmm4,%xmm6,%xmm2
	vcmpneq_usss (%ecx),%xmm6,%xmm2
	vcmpnlt_uqss %xmm4,%xmm6,%xmm2
	vcmpnlt_uqss (%ecx),%xmm6,%xmm2
	vcmpnle_uqss %xmm4,%xmm6,%xmm2
	vcmpnle_uqss (%ecx),%xmm6,%xmm2
	vcmpord_sss %xmm4,%xmm6,%xmm2
	vcmpord_sss (%ecx),%xmm6,%xmm2
	vcmpeq_usss %xmm4,%xmm6,%xmm2
	vcmpeq_usss (%ecx),%xmm6,%xmm2
	vcmpnge_uqss %xmm4,%xmm6,%xmm2
	vcmpnge_uqss (%ecx),%xmm6,%xmm2
	vcmpngt_uqss %xmm4,%xmm6,%xmm2
	vcmpngt_uqss (%ecx),%xmm6,%xmm2
	vcmpfalse_osss %xmm4,%xmm6,%xmm2
	vcmpfalse_osss (%ecx),%xmm6,%xmm2
	vcmpneq_osss %xmm4,%xmm6,%xmm2
	vcmpneq_osss (%ecx),%xmm6,%xmm2
	vcmpge_oqss %xmm4,%xmm6,%xmm2
	vcmpge_oqss (%ecx),%xmm6,%xmm2
	vcmpgt_oqss %xmm4,%xmm6,%xmm2
	vcmpgt_oqss (%ecx),%xmm6,%xmm2
	vcmptrue_usss %xmm4,%xmm6,%xmm2
	vcmptrue_usss (%ecx),%xmm6,%xmm2

# Tests for op mem32, ymm
	vbroadcastss (%ecx),%ymm4

# Tests for op xmm/mem32, xmm
	vcomiss %xmm4,%xmm6
	vcomiss (%ecx),%xmm4
	vpmovsxbd %xmm4,%xmm6
	vpmovsxbd (%ecx),%xmm4
	vpmovsxwq %xmm4,%xmm6
	vpmovsxwq (%ecx),%xmm4
	vpmovzxbd %xmm4,%xmm6
	vpmovzxbd (%ecx),%xmm4
	vpmovzxwq %xmm4,%xmm6
	vpmovzxwq (%ecx),%xmm4
	vucomiss %xmm4,%xmm6
	vucomiss (%ecx),%xmm4

# Tests for op mem32, xmm
	vbroadcastss (%ecx),%xmm4
	vmovss (%ecx),%xmm4

# Tests for op xmm, mem32
	vmovss %xmm4,(%ecx)

# Tests for op xmm, regl/mem32
# Tests for op regl/mem32, xmm
	vmovd %xmm4,%ecx
	vmovd %xmm4,(%ecx)
	vmovd %ecx,%xmm4
	vmovd (%ecx),%xmm4

# Tests for op xmm/mem32, regl
	vcvtss2si %xmm4,%ecx
	vcvtss2si (%ecx),%ecx
	vcvttss2si %xmm4,%ecx
	vcvttss2si (%ecx),%ecx

# Tests for op imm8, xmm, regq/mem32
	vextractps $7,%xmm4,(%ecx)

# Tests for op imm8, xmm, regl/mem32
	vpextrd $7,%xmm4,%ecx
	vpextrd $7,%xmm4,(%ecx)
	vextractps $7,%xmm4,%ecx
	vextractps $7,%xmm4,(%ecx)

# Tests for op imm8, regl/mem32, xmm, xmm
	vpinsrd $7,%ecx,%xmm4,%xmm6
	vpinsrd $7,(%ecx),%xmm4,%xmm6

# Tests for op regl/mem32, xmm, xmm
	vcvtsi2sd %ecx,%xmm4,%xmm6
	vcvtsi2sd (%ecx),%xmm4,%xmm6
	vcvtsi2ss %ecx,%xmm4,%xmm6
	vcvtsi2ss (%ecx),%xmm4,%xmm6

# Tests for op imm8, xmm/mem32, xmm, xmm
	vcmpss $7,%xmm4,%xmm6,%xmm2
	vcmpss $7,(%ecx),%xmm6,%xmm2
	vinsertps $7,%xmm4,%xmm6,%xmm2
	vinsertps $7,(%ecx),%xmm6,%xmm2
	vroundss $7,%xmm4,%xmm6,%xmm2
	vroundss $7,(%ecx),%xmm6,%xmm2

# Tests for op xmm/m16, xmm
	vpmovsxbq %xmm4,%xmm6
	vpmovsxbq (%ecx),%xmm4
	vpmovzxbq %xmm4,%xmm6
	vpmovzxbq (%ecx),%xmm4

# Tests for op imm8, xmm, regl/mem16
	vpextrw $7,%xmm4,%ecx
	vpextrw $7,%xmm4,(%ecx)

# Tests for op imm8, xmm, regq/mem16
	vpextrw $7,%xmm4,(%ecx)

# Tests for op imm8, regl/mem16, xmm, xmm
	vpinsrw $7,%ecx,%xmm4,%xmm6
	vpinsrw $7,(%ecx),%xmm4,%xmm6

# Tests for op imm8, xmm, regl/mem8
	vpextrb $7,%xmm4,%ecx
	vpextrb $7,%xmm4,(%ecx)

# Tests for op imm8, regl/mem8, xmm, xmm
	vpinsrb $7,%ecx,%xmm4,%xmm6
	vpinsrb $7,(%ecx),%xmm4,%xmm6

# Tests for op imm8, xmm, regq/mem8
	vpextrb $7,%xmm4,(%ecx)

# Tests for op xmm, xmm
	vmaskmovdqu %xmm4,%xmm6
	vmovq %xmm4,%xmm6

# Tests for op xmm, regl
	vmovmskpd %xmm4,%ecx
	vmovmskps %xmm4,%ecx
	vpmovmskb %xmm4,%ecx

# Tests for op xmm, xmm, xmm
	vmovhlps %xmm4,%xmm6,%xmm2
	vmovlhps %xmm4,%xmm6,%xmm2
	vmovsd %xmm4,%xmm6,%xmm2
	vmovss %xmm4,%xmm6,%xmm2

# Tests for op imm8, xmm, xmm
	vpslld $7,%xmm4,%xmm6
	vpslldq $7,%xmm4,%xmm6
	vpsllq $7,%xmm4,%xmm6
	vpsllw $7,%xmm4,%xmm6
	vpsrad $7,%xmm4,%xmm6
	vpsraw $7,%xmm4,%xmm6
	vpsrld $7,%xmm4,%xmm6
	vpsrldq $7,%xmm4,%xmm6
	vpsrlq $7,%xmm4,%xmm6
	vpsrlw $7,%xmm4,%xmm6

# Tests for op imm8, xmm, regl
	vpextrw $7,%xmm4,%ecx

# Tests for op ymm, regl
	vmovmskpd %ymm4,%ecx
	vmovmskps %ymm4,%ecx

# Default instructions without suffixes.
	vcvtpd2dq %xmm4,%xmm6
	vcvtpd2dq %ymm4,%xmm6
	vcvtpd2ps %xmm4,%xmm6
	vcvtpd2ps %ymm4,%xmm6
	vcvttpd2dq %xmm4,%xmm6
	vcvttpd2dq %ymm4,%xmm6

#Tests with different memory and register operands.
	vldmxcsr 0x1234
	vmovdqa 0x1234,%xmm0
	vmovdqa %xmm0,0x1234
	vmovd %xmm0,0x1234
	vcvtsd2si 0x1234,%eax
	vcvtdq2pd 0x1234,%ymm0
	vcvtpd2psy 0x1234,%xmm0
	vpavgb 0x1234,%xmm0,%xmm7
	vaeskeygenassist $7,0x1234,%xmm0
	vpextrb $7,%xmm0,0x1234
	vcvtsi2sdl 0x1234,%xmm0,%xmm7
	vpclmulqdq $7,0x1234,%xmm0,%xmm7
	vblendvps %xmm0,0x1234,%xmm4,%xmm6
	vpinsrb $7,0x1234,%xmm0,%xmm7
	vmovdqa 0x1234,%ymm0
	vmovdqa %ymm0,0x1234
	vpermilpd 0x1234,%ymm0,%ymm7
	vroundpd $7,0x1234,%ymm0
	vextractf128 $7,%ymm0,0x1234
	vperm2f128 $7,0x1234,%ymm0,%ymm7
	vblendvpd %ymm0,0x1234,%ymm4,%ymm6
	vldmxcsr (%ebp)
	vmovdqa (%ebp),%xmm0
	vmovdqa %xmm0,(%ebp)
	vmovd %xmm0,(%ebp)
	vcvtsd2si (%ebp),%eax
	vcvtdq2pd (%ebp),%ymm0
	vcvtpd2psy (%ebp),%xmm0
	vpavgb (%ebp),%xmm0,%xmm7
	vaeskeygenassist $7,(%ebp),%xmm0
	vpextrb $7,%xmm0,(%ebp)
	vcvtsi2sdl (%ebp),%xmm0,%xmm7
	vpclmulqdq $7,(%ebp),%xmm0,%xmm7
	vblendvps %xmm0,(%ebp),%xmm4,%xmm6
	vpinsrb $7,(%ebp),%xmm0,%xmm7
	vmovdqa (%ebp),%ymm0
	vmovdqa %ymm0,(%ebp)
	vpermilpd (%ebp),%ymm0,%ymm7
	vroundpd $7,(%ebp),%ymm0
	vextractf128 $7,%ymm0,(%ebp)
	vperm2f128 $7,(%ebp),%ymm0,%ymm7
	vblendvpd %ymm0,(%ebp),%ymm4,%ymm6
	vldmxcsr (%esp)
	vmovdqa (%esp),%xmm0
	vmovdqa %xmm0,(%esp)
	vmovd %xmm0,(%esp)
	vcvtsd2si (%esp),%eax
	vcvtdq2pd (%esp),%ymm0
	vcvtpd2psy (%esp),%xmm0
	vpavgb (%esp),%xmm0,%xmm7
	vaeskeygenassist $7,(%esp),%xmm0
	vpextrb $7,%xmm0,(%esp)
	vcvtsi2sdl (%esp),%xmm0,%xmm7
	vpclmulqdq $7,(%esp),%xmm0,%xmm7
	vblendvps %xmm0,(%esp),%xmm4,%xmm6
	vpinsrb $7,(%esp),%xmm0,%xmm7
	vmovdqa (%esp),%ymm0
	vmovdqa %ymm0,(%esp)
	vpermilpd (%esp),%ymm0,%ymm7
	vroundpd $7,(%esp),%ymm0
	vextractf128 $7,%ymm0,(%esp)
	vperm2f128 $7,(%esp),%ymm0,%ymm7
	vblendvpd %ymm0,(%esp),%ymm4,%ymm6
	vldmxcsr 0x99(%ebp)
	vmovdqa 0x99(%ebp),%xmm0
	vmovdqa %xmm0,0x99(%ebp)
	vmovd %xmm0,0x99(%ebp)
	vcvtsd2si 0x99(%ebp),%eax
	vcvtdq2pd 0x99(%ebp),%ymm0
	vcvtpd2psy 0x99(%ebp),%xmm0
	vpavgb 0x99(%ebp),%xmm0,%xmm7
	vaeskeygenassist $7,0x99(%ebp),%xmm0
	vpextrb $7,%xmm0,0x99(%ebp)
	vcvtsi2sdl 0x99(%ebp),%xmm0,%xmm7
	vpclmulqdq $7,0x99(%ebp),%xmm0,%xmm7
	vblendvps %xmm0,0x99(%ebp),%xmm4,%xmm6
	vpinsrb $7,0x99(%ebp),%xmm0,%xmm7
	vmovdqa 0x99(%ebp),%ymm0
	vmovdqa %ymm0,0x99(%ebp)
	vpermilpd 0x99(%ebp),%ymm0,%ymm7
	vroundpd $7,0x99(%ebp),%ymm0
	vextractf128 $7,%ymm0,0x99(%ebp)
	vperm2f128 $7,0x99(%ebp),%ymm0,%ymm7
	vblendvpd %ymm0,0x99(%ebp),%ymm4,%ymm6
	vldmxcsr 0x99(,%eiz)
	vmovdqa 0x99(,%eiz),%xmm0
	vmovdqa %xmm0,0x99(,%eiz)
	vmovd %xmm0,0x99(,%eiz)
	vcvtsd2si 0x99(,%eiz),%eax
	vcvtdq2pd 0x99(,%eiz),%ymm0
	vcvtpd2psy 0x99(,%eiz),%xmm0
	vpavgb 0x99(,%eiz),%xmm0,%xmm7
	vaeskeygenassist $7,0x99(,%eiz),%xmm0
	vpextrb $7,%xmm0,0x99(,%eiz)
	vcvtsi2sdl 0x99(,%eiz),%xmm0,%xmm7
	vpclmulqdq $7,0x99(,%eiz),%xmm0,%xmm7
	vblendvps %xmm0,0x99(,%eiz),%xmm4,%xmm6
	vpinsrb $7,0x99(,%eiz),%xmm0,%xmm7
	vmovdqa 0x99(,%eiz),%ymm0
	vmovdqa %ymm0,0x99(,%eiz)
	vpermilpd 0x99(,%eiz),%ymm0,%ymm7
	vroundpd $7,0x99(,%eiz),%ymm0
	vextractf128 $7,%ymm0,0x99(,%eiz)
	vperm2f128 $7,0x99(,%eiz),%ymm0,%ymm7
	vblendvpd %ymm0,0x99(,%eiz),%ymm4,%ymm6
	vldmxcsr 0x99(,%eiz,2)
	vmovdqa 0x99(,%eiz,2),%xmm0
	vmovdqa %xmm0,0x99(,%eiz,2)
	vmovd %xmm0,0x99(,%eiz,2)
	vcvtsd2si 0x99(,%eiz,2),%eax
	vcvtdq2pd 0x99(,%eiz,2),%ymm0
	vcvtpd2psy 0x99(,%eiz,2),%xmm0
	vpavgb 0x99(,%eiz,2),%xmm0,%xmm7
	vaeskeygenassist $7,0x99(,%eiz,2),%xmm0
	vpextrb $7,%xmm0,0x99(,%eiz,2)
	vcvtsi2sdl 0x99(,%eiz,2),%xmm0,%xmm7
	vpclmulqdq $7,0x99(,%eiz,2),%xmm0,%xmm7
	vblendvps %xmm0,0x99(,%eiz,2),%xmm4,%xmm6
	vpinsrb $7,0x99(,%eiz,2),%xmm0,%xmm7
	vmovdqa 0x99(,%eiz,2),%ymm0
	vmovdqa %ymm0,0x99(,%eiz,2)
	vpermilpd 0x99(,%eiz,2),%ymm0,%ymm7
	vroundpd $7,0x99(,%eiz,2),%ymm0
	vextractf128 $7,%ymm0,0x99(,%eiz,2)
	vperm2f128 $7,0x99(,%eiz,2),%ymm0,%ymm7
	vblendvpd %ymm0,0x99(,%eiz,2),%ymm4,%ymm6
	vldmxcsr 0x99(%eax,%eiz)
	vmovdqa 0x99(%eax,%eiz),%xmm0
	vmovdqa %xmm0,0x99(%eax,%eiz)
	vmovd %xmm0,0x99(%eax,%eiz)
	vcvtsd2si 0x99(%eax,%eiz),%eax
	vcvtdq2pd 0x99(%eax,%eiz),%ymm0
	vcvtpd2psy 0x99(%eax,%eiz),%xmm0
	vpavgb 0x99(%eax,%eiz),%xmm0,%xmm7
	vaeskeygenassist $7,0x99(%eax,%eiz),%xmm0
	vpextrb $7,%xmm0,0x99(%eax,%eiz)
	vcvtsi2sdl 0x99(%eax,%eiz),%xmm0,%xmm7
	vpclmulqdq $7,0x99(%eax,%eiz),%xmm0,%xmm7
	vblendvps %xmm0,0x99(%eax,%eiz),%xmm4,%xmm6
	vpinsrb $7,0x99(%eax,%eiz),%xmm0,%xmm7
	vmovdqa 0x99(%eax,%eiz),%ymm0
	vmovdqa %ymm0,0x99(%eax,%eiz)
	vpermilpd 0x99(%eax,%eiz),%ymm0,%ymm7
	vroundpd $7,0x99(%eax,%eiz),%ymm0
	vextractf128 $7,%ymm0,0x99(%eax,%eiz)
	vperm2f128 $7,0x99(%eax,%eiz),%ymm0,%ymm7
	vblendvpd %ymm0,0x99(%eax,%eiz),%ymm4,%ymm6
	vldmxcsr 0x99(%eax,%eiz,2)
	vmovdqa 0x99(%eax,%eiz,2),%xmm0
	vmovdqa %xmm0,0x99(%eax,%eiz,2)
	vmovd %xmm0,0x99(%eax,%eiz,2)
	vcvtsd2si 0x99(%eax,%eiz,2),%eax
	vcvtdq2pd 0x99(%eax,%eiz,2),%ymm0
	vcvtpd2psy 0x99(%eax,%eiz,2),%xmm0
	vpavgb 0x99(%eax,%eiz,2),%xmm0,%xmm7
	vaeskeygenassist $7,0x99(%eax,%eiz,2),%xmm0
	vpextrb $7,%xmm0,0x99(%eax,%eiz,2)
	vcvtsi2sdl 0x99(%eax,%eiz,2),%xmm0,%xmm7
	vpclmulqdq $7,0x99(%eax,%eiz,2),%xmm0,%xmm7
	vblendvps %xmm0,0x99(%eax,%eiz,2),%xmm4,%xmm6
	vpinsrb $7,0x99(%eax,%eiz,2),%xmm0,%xmm7
	vmovdqa 0x99(%eax,%eiz,2),%ymm0
	vmovdqa %ymm0,0x99(%eax,%eiz,2)
	vpermilpd 0x99(%eax,%eiz,2),%ymm0,%ymm7
	vroundpd $7,0x99(%eax,%eiz,2),%ymm0
	vextractf128 $7,%ymm0,0x99(%eax,%eiz,2)
	vperm2f128 $7,0x99(%eax,%eiz,2),%ymm0,%ymm7
	vblendvpd %ymm0,0x99(%eax,%eiz,2),%ymm4,%ymm6
	vldmxcsr 0x99(%eax,%ebx,4)
	vmovdqa 0x99(%eax,%ebx,4),%xmm0
	vmovdqa %xmm0,0x99(%eax,%ebx,4)
	vmovd %xmm0,0x99(%eax,%ebx,4)
	vcvtsd2si 0x99(%eax,%ebx,4),%eax
	vcvtdq2pd 0x99(%eax,%ebx,4),%ymm0
	vcvtpd2psy 0x99(%eax,%ebx,4),%xmm0
	vpavgb 0x99(%eax,%ebx,4),%xmm0,%xmm7
	vaeskeygenassist $7,0x99(%eax,%ebx,4),%xmm0
	vpextrb $7,%xmm0,0x99(%eax,%ebx,4)
	vcvtsi2sdl 0x99(%eax,%ebx,4),%xmm0,%xmm7
	vpclmulqdq $7,0x99(%eax,%ebx,4),%xmm0,%xmm7
	vblendvps %xmm0,0x99(%eax,%ebx,4),%xmm4,%xmm6
	vpinsrb $7,0x99(%eax,%ebx,4),%xmm0,%xmm7
	vmovdqa 0x99(%eax,%ebx,4),%ymm0
	vmovdqa %ymm0,0x99(%eax,%ebx,4)
	vpermilpd 0x99(%eax,%ebx,4),%ymm0,%ymm7
	vroundpd $7,0x99(%eax,%ebx,4),%ymm0
	vextractf128 $7,%ymm0,0x99(%eax,%ebx,4)
	vperm2f128 $7,0x99(%eax,%ebx,4),%ymm0,%ymm7
	vblendvpd %ymm0,0x99(%eax,%ebx,4),%ymm4,%ymm6
	vldmxcsr 0x99(%esp,%ecx,8)
	vmovdqa 0x99(%esp,%ecx,8),%xmm0
	vmovdqa %xmm0,0x99(%esp,%ecx,8)
	vmovd %xmm0,0x99(%esp,%ecx,8)
	vcvtsd2si 0x99(%esp,%ecx,8),%eax
	vcvtdq2pd 0x99(%esp,%ecx,8),%ymm0
	vcvtpd2psy 0x99(%esp,%ecx,8),%xmm0
	vpavgb 0x99(%esp,%ecx,8),%xmm0,%xmm7
	vaeskeygenassist $7,0x99(%esp,%ecx,8),%xmm0
	vpextrb $7,%xmm0,0x99(%esp,%ecx,8)
	vcvtsi2sdl 0x99(%esp,%ecx,8),%xmm0,%xmm7
	vpclmulqdq $7,0x99(%esp,%ecx,8),%xmm0,%xmm7
	vblendvps %xmm0,0x99(%esp,%ecx,8),%xmm4,%xmm6
	vpinsrb $7,0x99(%esp,%ecx,8),%xmm0,%xmm7
	vmovdqa 0x99(%esp,%ecx,8),%ymm0
	vmovdqa %ymm0,0x99(%esp,%ecx,8)
	vpermilpd 0x99(%esp,%ecx,8),%ymm0,%ymm7
	vroundpd $7,0x99(%esp,%ecx,8),%ymm0
	vextractf128 $7,%ymm0,0x99(%esp,%ecx,8)
	vperm2f128 $7,0x99(%esp,%ecx,8),%ymm0,%ymm7
	vblendvpd %ymm0,0x99(%esp,%ecx,8),%ymm4,%ymm6
	vldmxcsr 0x99(%ebp,%edx,1)
	vmovdqa 0x99(%ebp,%edx,1),%xmm0
	vmovdqa %xmm0,0x99(%ebp,%edx,1)
	vmovd %xmm0,0x99(%ebp,%edx,1)
	vcvtsd2si 0x99(%ebp,%edx,1),%eax
	vcvtdq2pd 0x99(%ebp,%edx,1),%ymm0
	vcvtpd2psy 0x99(%ebp,%edx,1),%xmm0
	vpavgb 0x99(%ebp,%edx,1),%xmm0,%xmm7
	vaeskeygenassist $7,0x99(%ebp,%edx,1),%xmm0
	vpextrb $7,%xmm0,0x99(%ebp,%edx,1)
	vcvtsi2sdl 0x99(%ebp,%edx,1),%xmm0,%xmm7
	vpclmulqdq $7,0x99(%ebp,%edx,1),%xmm0,%xmm7
	vblendvps %xmm0,0x99(%ebp,%edx,1),%xmm4,%xmm6
	vpinsrb $7,0x99(%ebp,%edx,1),%xmm0,%xmm7
	vmovdqa 0x99(%ebp,%edx,1),%ymm0
	vmovdqa %ymm0,0x99(%ebp,%edx,1)
	vpermilpd 0x99(%ebp,%edx,1),%ymm0,%ymm7
	vroundpd $7,0x99(%ebp,%edx,1),%ymm0
	vextractf128 $7,%ymm0,0x99(%ebp,%edx,1)
	vperm2f128 $7,0x99(%ebp,%edx,1),%ymm0,%ymm7
	vblendvpd %ymm0,0x99(%ebp,%edx,1),%ymm4,%ymm6
# Tests for all register operands.
	vmovmskpd %xmm0,%eax
	vpslld $7,%xmm0,%xmm7
	vmovmskps %ymm0,%eax

	.intel_syntax noprefix

# Tests for op mem64
	vldmxcsr DWORD PTR [ecx]
	vldmxcsr [ecx]
	vstmxcsr DWORD PTR [ecx]
	vstmxcsr [ecx]

# Tests for op mem256, mask,  ymm
# Tests for op ymm, mask, mem256
	vmaskmovpd ymm6,ymm4,YMMWORD PTR [ecx]
	vmaskmovpd YMMWORD PTR [ecx],ymm6,ymm4
	vmaskmovpd ymm6,ymm4,[ecx]
	vmaskmovpd [ecx],ymm6,ymm4
	vmaskmovps ymm6,ymm4,YMMWORD PTR [ecx]
	vmaskmovps YMMWORD PTR [ecx],ymm6,ymm4
	vmaskmovps ymm6,ymm4,[ecx]
	vmaskmovps [ecx],ymm6,ymm4

# Tests for op imm8, ymm/mem256, ymm
	vpermilpd ymm2,ymm6,7
	vpermilpd ymm6,YMMWORD PTR [ecx],7
	vpermilpd ymm6,[ecx],7
	vpermilps ymm2,ymm6,7
	vpermilps ymm6,YMMWORD PTR [ecx],7
	vpermilps ymm6,[ecx],7
	vroundpd ymm2,ymm6,7
	vroundpd ymm6,YMMWORD PTR [ecx],7
	vroundpd ymm6,[ecx],7
	vroundps ymm2,ymm6,7
	vroundps ymm6,YMMWORD PTR [ecx],7
	vroundps ymm6,[ecx],7

# Tests for op ymm/mem256, ymm, ymm
	vaddpd ymm2,ymm6,ymm4
	vaddpd ymm2,ymm6,YMMWORD PTR [ecx]
	vaddpd ymm2,ymm6,[ecx]
	vaddps ymm2,ymm6,ymm4
	vaddps ymm2,ymm6,YMMWORD PTR [ecx]
	vaddps ymm2,ymm6,[ecx]
	vaddsubpd ymm2,ymm6,ymm4
	vaddsubpd ymm2,ymm6,YMMWORD PTR [ecx]
	vaddsubpd ymm2,ymm6,[ecx]
	vaddsubps ymm2,ymm6,ymm4
	vaddsubps ymm2,ymm6,YMMWORD PTR [ecx]
	vaddsubps ymm2,ymm6,[ecx]
	vandnpd ymm2,ymm6,ymm4
	vandnpd ymm2,ymm6,YMMWORD PTR [ecx]
	vandnpd ymm2,ymm6,[ecx]
	vandnps ymm2,ymm6,ymm4
	vandnps ymm2,ymm6,YMMWORD PTR [ecx]
	vandnps ymm2,ymm6,[ecx]
	vandpd ymm2,ymm6,ymm4
	vandpd ymm2,ymm6,YMMWORD PTR [ecx]
	vandpd ymm2,ymm6,[ecx]
	vandps ymm2,ymm6,ymm4
	vandps ymm2,ymm6,YMMWORD PTR [ecx]
	vandps ymm2,ymm6,[ecx]
	vdivpd ymm2,ymm6,ymm4
	vdivpd ymm2,ymm6,YMMWORD PTR [ecx]
	vdivpd ymm2,ymm6,[ecx]
	vdivps ymm2,ymm6,ymm4
	vdivps ymm2,ymm6,YMMWORD PTR [ecx]
	vdivps ymm2,ymm6,[ecx]
	vhaddpd ymm2,ymm6,ymm4
	vhaddpd ymm2,ymm6,YMMWORD PTR [ecx]
	vhaddpd ymm2,ymm6,[ecx]
	vhaddps ymm2,ymm6,ymm4
	vhaddps ymm2,ymm6,YMMWORD PTR [ecx]
	vhaddps ymm2,ymm6,[ecx]
	vhsubpd ymm2,ymm6,ymm4
	vhsubpd ymm2,ymm6,YMMWORD PTR [ecx]
	vhsubpd ymm2,ymm6,[ecx]
	vhsubps ymm2,ymm6,ymm4
	vhsubps ymm2,ymm6,YMMWORD PTR [ecx]
	vhsubps ymm2,ymm6,[ecx]
	vmaxpd ymm2,ymm6,ymm4
	vmaxpd ymm2,ymm6,YMMWORD PTR [ecx]
	vmaxpd ymm2,ymm6,[ecx]
	vmaxps ymm2,ymm6,ymm4
	vmaxps ymm2,ymm6,YMMWORD PTR [ecx]
	vmaxps ymm2,ymm6,[ecx]
	vminpd ymm2,ymm6,ymm4
	vminpd ymm2,ymm6,YMMWORD PTR [ecx]
	vminpd ymm2,ymm6,[ecx]
	vminps ymm2,ymm6,ymm4
	vminps ymm2,ymm6,YMMWORD PTR [ecx]
	vminps ymm2,ymm6,[ecx]
	vmulpd ymm2,ymm6,ymm4
	vmulpd ymm2,ymm6,YMMWORD PTR [ecx]
	vmulpd ymm2,ymm6,[ecx]
	vmulps ymm2,ymm6,ymm4
	vmulps ymm2,ymm6,YMMWORD PTR [ecx]
	vmulps ymm2,ymm6,[ecx]
	vorpd ymm2,ymm6,ymm4
	vorpd ymm2,ymm6,YMMWORD PTR [ecx]
	vorpd ymm2,ymm6,[ecx]
	vorps ymm2,ymm6,ymm4
	vorps ymm2,ymm6,YMMWORD PTR [ecx]
	vorps ymm2,ymm6,[ecx]
	vpermilpd ymm2,ymm6,ymm4
	vpermilpd ymm2,ymm6,YMMWORD PTR [ecx]
	vpermilpd ymm2,ymm6,[ecx]
	vpermilps ymm2,ymm6,ymm4
	vpermilps ymm2,ymm6,YMMWORD PTR [ecx]
	vpermilps ymm2,ymm6,[ecx]
	vsubpd ymm2,ymm6,ymm4
	vsubpd ymm2,ymm6,YMMWORD PTR [ecx]
	vsubpd ymm2,ymm6,[ecx]
	vsubps ymm2,ymm6,ymm4
	vsubps ymm2,ymm6,YMMWORD PTR [ecx]
	vsubps ymm2,ymm6,[ecx]
	vunpckhpd ymm2,ymm6,ymm4
	vunpckhpd ymm2,ymm6,YMMWORD PTR [ecx]
	vunpckhpd ymm2,ymm6,[ecx]
	vunpckhps ymm2,ymm6,ymm4
	vunpckhps ymm2,ymm6,YMMWORD PTR [ecx]
	vunpckhps ymm2,ymm6,[ecx]
	vunpcklpd ymm2,ymm6,ymm4
	vunpcklpd ymm2,ymm6,YMMWORD PTR [ecx]
	vunpcklpd ymm2,ymm6,[ecx]
	vunpcklps ymm2,ymm6,ymm4
	vunpcklps ymm2,ymm6,YMMWORD PTR [ecx]
	vunpcklps ymm2,ymm6,[ecx]
	vxorpd ymm2,ymm6,ymm4
	vxorpd ymm2,ymm6,YMMWORD PTR [ecx]
	vxorpd ymm2,ymm6,[ecx]
	vxorps ymm2,ymm6,ymm4
	vxorps ymm2,ymm6,YMMWORD PTR [ecx]
	vxorps ymm2,ymm6,[ecx]
	vcmpeqpd ymm2,ymm6,ymm4
	vcmpeqpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpeqpd ymm2,ymm6,[ecx]
	vcmpltpd ymm2,ymm6,ymm4
	vcmpltpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpltpd ymm2,ymm6,[ecx]
	vcmplepd ymm2,ymm6,ymm4
	vcmplepd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmplepd ymm2,ymm6,[ecx]
	vcmpunordpd ymm2,ymm6,ymm4
	vcmpunordpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpunordpd ymm2,ymm6,[ecx]
	vcmpneqpd ymm2,ymm6,ymm4
	vcmpneqpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpneqpd ymm2,ymm6,[ecx]
	vcmpnltpd ymm2,ymm6,ymm4
	vcmpnltpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpnltpd ymm2,ymm6,[ecx]
	vcmpnlepd ymm2,ymm6,ymm4
	vcmpnlepd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpnlepd ymm2,ymm6,[ecx]
	vcmpordpd ymm2,ymm6,ymm4
	vcmpordpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpordpd ymm2,ymm6,[ecx]
	vcmpeq_uqpd ymm2,ymm6,ymm4
	vcmpeq_uqpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpeq_uqpd ymm2,ymm6,[ecx]
	vcmpngepd ymm2,ymm6,ymm4
	vcmpngepd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpngepd ymm2,ymm6,[ecx]
	vcmpngtpd ymm2,ymm6,ymm4
	vcmpngtpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpngtpd ymm2,ymm6,[ecx]
	vcmpfalsepd ymm2,ymm6,ymm4
	vcmpfalsepd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpfalsepd ymm2,ymm6,[ecx]
	vcmpneq_oqpd ymm2,ymm6,ymm4
	vcmpneq_oqpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpneq_oqpd ymm2,ymm6,[ecx]
	vcmpgepd ymm2,ymm6,ymm4
	vcmpgepd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpgepd ymm2,ymm6,[ecx]
	vcmpgtpd ymm2,ymm6,ymm4
	vcmpgtpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpgtpd ymm2,ymm6,[ecx]
	vcmptruepd ymm2,ymm6,ymm4
	vcmptruepd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmptruepd ymm2,ymm6,[ecx]
	vcmpeq_ospd ymm2,ymm6,ymm4
	vcmpeq_ospd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpeq_ospd ymm2,ymm6,[ecx]
	vcmplt_oqpd ymm2,ymm6,ymm4
	vcmplt_oqpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmplt_oqpd ymm2,ymm6,[ecx]
	vcmple_oqpd ymm2,ymm6,ymm4
	vcmple_oqpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmple_oqpd ymm2,ymm6,[ecx]
	vcmpunord_spd ymm2,ymm6,ymm4
	vcmpunord_spd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpunord_spd ymm2,ymm6,[ecx]
	vcmpneq_uspd ymm2,ymm6,ymm4
	vcmpneq_uspd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpneq_uspd ymm2,ymm6,[ecx]
	vcmpnlt_uqpd ymm2,ymm6,ymm4
	vcmpnlt_uqpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpnlt_uqpd ymm2,ymm6,[ecx]
	vcmpnle_uqpd ymm2,ymm6,ymm4
	vcmpnle_uqpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpnle_uqpd ymm2,ymm6,[ecx]
	vcmpord_spd ymm2,ymm6,ymm4
	vcmpord_spd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpord_spd ymm2,ymm6,[ecx]
	vcmpeq_uspd ymm2,ymm6,ymm4
	vcmpeq_uspd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpeq_uspd ymm2,ymm6,[ecx]
	vcmpnge_uqpd ymm2,ymm6,ymm4
	vcmpnge_uqpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpnge_uqpd ymm2,ymm6,[ecx]
	vcmpngt_uqpd ymm2,ymm6,ymm4
	vcmpngt_uqpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpngt_uqpd ymm2,ymm6,[ecx]
	vcmpfalse_ospd ymm2,ymm6,ymm4
	vcmpfalse_ospd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpfalse_ospd ymm2,ymm6,[ecx]
	vcmpneq_ospd ymm2,ymm6,ymm4
	vcmpneq_ospd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpneq_ospd ymm2,ymm6,[ecx]
	vcmpge_oqpd ymm2,ymm6,ymm4
	vcmpge_oqpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpge_oqpd ymm2,ymm6,[ecx]
	vcmpgt_oqpd ymm2,ymm6,ymm4
	vcmpgt_oqpd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpgt_oqpd ymm2,ymm6,[ecx]
	vcmptrue_uspd ymm2,ymm6,ymm4
	vcmptrue_uspd ymm2,ymm6,YMMWORD PTR [ecx]
	vcmptrue_uspd ymm2,ymm6,[ecx]
	vcmpeqps ymm2,ymm6,ymm4
	vcmpeqps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpeqps ymm2,ymm6,[ecx]
	vcmpltps ymm2,ymm6,ymm4
	vcmpltps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpltps ymm2,ymm6,[ecx]
	vcmpleps ymm2,ymm6,ymm4
	vcmpleps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpleps ymm2,ymm6,[ecx]
	vcmpunordps ymm2,ymm6,ymm4
	vcmpunordps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpunordps ymm2,ymm6,[ecx]
	vcmpneqps ymm2,ymm6,ymm4
	vcmpneqps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpneqps ymm2,ymm6,[ecx]
	vcmpnltps ymm2,ymm6,ymm4
	vcmpnltps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpnltps ymm2,ymm6,[ecx]
	vcmpnleps ymm2,ymm6,ymm4
	vcmpnleps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpnleps ymm2,ymm6,[ecx]
	vcmpordps ymm2,ymm6,ymm4
	vcmpordps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpordps ymm2,ymm6,[ecx]
	vcmpeq_uqps ymm2,ymm6,ymm4
	vcmpeq_uqps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpeq_uqps ymm2,ymm6,[ecx]
	vcmpngeps ymm2,ymm6,ymm4
	vcmpngeps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpngeps ymm2,ymm6,[ecx]
	vcmpngtps ymm2,ymm6,ymm4
	vcmpngtps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpngtps ymm2,ymm6,[ecx]
	vcmpfalseps ymm2,ymm6,ymm4
	vcmpfalseps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpfalseps ymm2,ymm6,[ecx]
	vcmpneq_oqps ymm2,ymm6,ymm4
	vcmpneq_oqps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpneq_oqps ymm2,ymm6,[ecx]
	vcmpgeps ymm2,ymm6,ymm4
	vcmpgeps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpgeps ymm2,ymm6,[ecx]
	vcmpgtps ymm2,ymm6,ymm4
	vcmpgtps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpgtps ymm2,ymm6,[ecx]
	vcmptrueps ymm2,ymm6,ymm4
	vcmptrueps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmptrueps ymm2,ymm6,[ecx]
	vcmpeq_osps ymm2,ymm6,ymm4
	vcmpeq_osps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpeq_osps ymm2,ymm6,[ecx]
	vcmplt_oqps ymm2,ymm6,ymm4
	vcmplt_oqps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmplt_oqps ymm2,ymm6,[ecx]
	vcmple_oqps ymm2,ymm6,ymm4
	vcmple_oqps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmple_oqps ymm2,ymm6,[ecx]
	vcmpunord_sps ymm2,ymm6,ymm4
	vcmpunord_sps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpunord_sps ymm2,ymm6,[ecx]
	vcmpneq_usps ymm2,ymm6,ymm4
	vcmpneq_usps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpneq_usps ymm2,ymm6,[ecx]
	vcmpnlt_uqps ymm2,ymm6,ymm4
	vcmpnlt_uqps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpnlt_uqps ymm2,ymm6,[ecx]
	vcmpnle_uqps ymm2,ymm6,ymm4
	vcmpnle_uqps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpnle_uqps ymm2,ymm6,[ecx]
	vcmpord_sps ymm2,ymm6,ymm4
	vcmpord_sps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpord_sps ymm2,ymm6,[ecx]
	vcmpeq_usps ymm2,ymm6,ymm4
	vcmpeq_usps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpeq_usps ymm2,ymm6,[ecx]
	vcmpnge_uqps ymm2,ymm6,ymm4
	vcmpnge_uqps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpnge_uqps ymm2,ymm6,[ecx]
	vcmpngt_uqps ymm2,ymm6,ymm4
	vcmpngt_uqps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpngt_uqps ymm2,ymm6,[ecx]
	vcmpfalse_osps ymm2,ymm6,ymm4
	vcmpfalse_osps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpfalse_osps ymm2,ymm6,[ecx]
	vcmpneq_osps ymm2,ymm6,ymm4
	vcmpneq_osps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpneq_osps ymm2,ymm6,[ecx]
	vcmpge_oqps ymm2,ymm6,ymm4
	vcmpge_oqps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpge_oqps ymm2,ymm6,[ecx]
	vcmpgt_oqps ymm2,ymm6,ymm4
	vcmpgt_oqps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmpgt_oqps ymm2,ymm6,[ecx]
	vcmptrue_usps ymm2,ymm6,ymm4
	vcmptrue_usps ymm2,ymm6,YMMWORD PTR [ecx]
	vcmptrue_usps ymm2,ymm6,[ecx]
	vgf2p8mulb ymm6, ymm5, ymm4
	vgf2p8mulb ymm6, ymm5, YMMWORD PTR [ecx]
	vgf2p8mulb ymm6, ymm5, [ecx]
	vgf2p8mulb ymm6, ymm5, YMMWORD PTR [esp+esi*8-123456]
	vgf2p8mulb ymm6, ymm5, YMMWORD PTR [edx+4064]
	vgf2p8mulb ymm6, ymm5, YMMWORD PTR [edx+4096]
	vgf2p8mulb ymm6, ymm5, YMMWORD PTR [edx-4096]
	vgf2p8mulb ymm6, ymm5, YMMWORD PTR [edx-4128]

# Tests for op ymm/mem256, xmm
	vcvtpd2dq xmm4,ymm4
	vcvtpd2dq xmm4,YMMWORD PTR [ecx]
	vcvtpd2ps xmm4,ymm4
	vcvtpd2ps xmm4,YMMWORD PTR [ecx]
	vcvttpd2dq xmm4,ymm4
	vcvttpd2dq xmm4,YMMWORD PTR [ecx]

# Tests for op ymm/mem256, ymm
	vcvtdq2ps ymm6,ymm4
	vcvtdq2ps ymm4,YMMWORD PTR [ecx]
	vcvtdq2ps ymm4,[ecx]
	vcvtps2dq ymm6,ymm4
	vcvtps2dq ymm4,YMMWORD PTR [ecx]
	vcvtps2dq ymm4,[ecx]
	vcvttps2dq ymm6,ymm4
	vcvttps2dq ymm4,YMMWORD PTR [ecx]
	vcvttps2dq ymm4,[ecx]
	vmovapd ymm6,ymm4
	vmovapd ymm4,YMMWORD PTR [ecx]
	vmovapd ymm4,[ecx]
	vmovaps ymm6,ymm4
	vmovaps ymm4,YMMWORD PTR [ecx]
	vmovaps ymm4,[ecx]
	vmovdqa ymm6,ymm4
	vmovdqa ymm4,YMMWORD PTR [ecx]
	vmovdqa ymm4,[ecx]
	vmovdqu ymm6,ymm4
	vmovdqu ymm4,YMMWORD PTR [ecx]
	vmovdqu ymm4,[ecx]
	vmovddup ymm6,ymm4
	vmovddup ymm4,YMMWORD PTR [ecx]
	vmovddup ymm4,[ecx]
	vmovshdup ymm6,ymm4
	vmovshdup ymm4,YMMWORD PTR [ecx]
	vmovshdup ymm4,[ecx]
	vmovsldup ymm6,ymm4
	vmovsldup ymm4,YMMWORD PTR [ecx]
	vmovsldup ymm4,[ecx]
	vmovupd ymm6,ymm4
	vmovupd ymm4,YMMWORD PTR [ecx]
	vmovupd ymm4,[ecx]
	vmovups ymm6,ymm4
	vmovups ymm4,YMMWORD PTR [ecx]
	vmovups ymm4,[ecx]
	vptest ymm6,ymm4
	vptest ymm4,YMMWORD PTR [ecx]
	vptest ymm4,[ecx]
	vrcpps ymm6,ymm4
	vrcpps ymm4,YMMWORD PTR [ecx]
	vrcpps ymm4,[ecx]
	vrsqrtps ymm6,ymm4
	vrsqrtps ymm4,YMMWORD PTR [ecx]
	vrsqrtps ymm4,[ecx]
	vsqrtpd ymm6,ymm4
	vsqrtpd ymm4,YMMWORD PTR [ecx]
	vsqrtpd ymm4,[ecx]
	vsqrtps ymm6,ymm4
	vsqrtps ymm4,YMMWORD PTR [ecx]
	vsqrtps ymm4,[ecx]
	vtestpd ymm6,ymm4
	vtestpd ymm4,YMMWORD PTR [ecx]
	vtestpd ymm4,[ecx]
	vtestps ymm6,ymm4
	vtestps ymm4,YMMWORD PTR [ecx]
	vtestps ymm4,[ecx]

# Tests for op ymm, ymm/mem256
	vmovapd ymm6,ymm4
	vmovapd YMMWORD PTR [ecx],ymm4
	vmovapd [ecx],ymm4
	vmovaps ymm6,ymm4
	vmovaps YMMWORD PTR [ecx],ymm4
	vmovaps [ecx],ymm4
	vmovdqa ymm6,ymm4
	vmovdqa YMMWORD PTR [ecx],ymm4
	vmovdqa [ecx],ymm4
	vmovdqu ymm6,ymm4
	vmovdqu YMMWORD PTR [ecx],ymm4
	vmovdqu [ecx],ymm4
	vmovupd ymm6,ymm4
	vmovupd YMMWORD PTR [ecx],ymm4
	vmovupd [ecx],ymm4
	vmovups ymm6,ymm4
	vmovups YMMWORD PTR [ecx],ymm4
	vmovups [ecx],ymm4

# Tests for op mem256, ymm
	vlddqu ymm4,YMMWORD PTR [ecx]
	vlddqu ymm4,[ecx]

# Tests for op ymm, mem256
	vmovntdq YMMWORD PTR [ecx],ymm4
	vmovntdq [ecx],ymm4
	vmovntpd YMMWORD PTR [ecx],ymm4
	vmovntpd [ecx],ymm4
	vmovntps YMMWORD PTR [ecx],ymm4
	vmovntps [ecx],ymm4

# Tests for op imm8, ymm/mem256, ymm, ymm
	vblendpd ymm2,ymm6,ymm4,7
	vblendpd ymm2,ymm6,YMMWORD PTR [ecx],7
	vblendpd ymm2,ymm6,[ecx],7
	vblendps ymm2,ymm6,ymm4,7
	vblendps ymm2,ymm6,YMMWORD PTR [ecx],7
	vblendps ymm2,ymm6,[ecx],7
	vcmppd ymm2,ymm6,ymm4,7
	vcmppd ymm2,ymm6,YMMWORD PTR [ecx],7
	vcmppd ymm2,ymm6,[ecx],7
	vcmpps ymm2,ymm6,ymm4,7
	vcmpps ymm2,ymm6,YMMWORD PTR [ecx],7
	vcmpps ymm2,ymm6,[ecx],7
	vdpps ymm2,ymm6,ymm4,7
	vdpps ymm2,ymm6,YMMWORD PTR [ecx],7
	vdpps ymm2,ymm6,[ecx],7
	vperm2f128 ymm2,ymm6,ymm4,7
	vperm2f128 ymm2,ymm6,YMMWORD PTR [ecx],7
	vperm2f128 ymm2,ymm6,[ecx],7
	vshufpd ymm2,ymm6,ymm4,7
	vshufpd ymm2,ymm6,YMMWORD PTR [ecx],7
	vshufpd ymm2,ymm6,[ecx],7
	vshufps ymm2,ymm6,ymm4,7
	vshufps ymm2,ymm6,YMMWORD PTR [ecx],7
	vshufps ymm2,ymm6,[ecx],7
	vgf2p8affineqb ymm6, ymm5, ymm4, 0xab
	vgf2p8affineqb ymm6, ymm5, ymm4, 123
	vgf2p8affineqb ymm6, ymm5, YMMWORD PTR [ecx], 123
	vgf2p8affineqb ymm6, ymm5, [ecx], 123
	vgf2p8affineqb ymm6, ymm5, YMMWORD PTR [esp+esi*8-123456], 123
	vgf2p8affineqb ymm6, ymm5, YMMWORD PTR [edx+4064], 123
	vgf2p8affineqb ymm6, ymm5, YMMWORD PTR [edx+4096], 123
	vgf2p8affineqb ymm6, ymm5, YMMWORD PTR [edx-4096], 123
	vgf2p8affineqb ymm6, ymm5, YMMWORD PTR [edx-4128], 123
	vgf2p8affineinvqb ymm6, ymm5, ymm4, 0xab
	vgf2p8affineinvqb ymm6, ymm5, ymm4, 123
	vgf2p8affineinvqb ymm6, ymm5, YMMWORD PTR [ecx], 123
	vgf2p8affineinvqb ymm6, ymm5, [ecx], 123
	vgf2p8affineinvqb ymm6, ymm5, YMMWORD PTR [esp+esi*8-123456], 123
	vgf2p8affineinvqb ymm6, ymm5, YMMWORD PTR [edx+4064], 123
	vgf2p8affineinvqb ymm6, ymm5, YMMWORD PTR [edx+4096], 123
	vgf2p8affineinvqb ymm6, ymm5, YMMWORD PTR [edx-4096], 123
	vgf2p8affineinvqb ymm6, ymm5, YMMWORD PTR [edx-4128], 123

# Tests for op ymm, ymm/mem256, ymm, ymm
	vblendvpd ymm7,ymm2,ymm6,ymm4
	vblendvpd ymm7,ymm2,YMMWORD PTR [ecx],ymm4
	vblendvpd ymm7,ymm2,[ecx],ymm4
	vblendvps ymm7,ymm2,ymm6,ymm4
	vblendvps ymm7,ymm2,YMMWORD PTR [ecx],ymm4
	vblendvps ymm7,ymm2,[ecx],ymm4

# Tests for op imm8, xmm/mem128, ymm, ymm
	vinsertf128 ymm6,ymm4,xmm4,7
	vinsertf128 ymm6,ymm4,XMMWORD PTR [ecx],7
	vinsertf128 ymm6,ymm4,[ecx],7

# Tests for op imm8, ymm, xmm/mem128
	vextractf128 xmm4,ymm4,7
	vextractf128 XMMWORD PTR [ecx],ymm4,7
	vextractf128 [ecx],ymm4,7

# Tests for op mem128, ymm
	vbroadcastf128 ymm4,XMMWORD PTR [ecx]
	vbroadcastf128 ymm4,[ecx]

# Tests for op xmm/mem128, xmm
	vcvtdq2ps xmm6,xmm4
	vcvtdq2ps xmm4,XMMWORD PTR [ecx]
	vcvtdq2ps xmm4,[ecx]
	vcvtpd2dq xmm6,xmm4
	vcvtpd2dq xmm4,XMMWORD PTR [ecx]
	vcvtpd2ps xmm6,xmm4
	vcvtpd2ps xmm4,XMMWORD PTR [ecx]
	vcvtps2dq xmm6,xmm4
	vcvtps2dq xmm4,XMMWORD PTR [ecx]
	vcvtps2dq xmm4,[ecx]
	vcvttpd2dq xmm6,xmm4
	vcvttpd2dq xmm4,XMMWORD PTR [ecx]
	vcvttps2dq xmm6,xmm4
	vcvttps2dq xmm4,XMMWORD PTR [ecx]
	vcvttps2dq xmm4,[ecx]
	vmovapd xmm6,xmm4
	vmovapd xmm4,XMMWORD PTR [ecx]
	vmovapd xmm4,[ecx]
	vmovaps xmm6,xmm4
	vmovaps xmm4,XMMWORD PTR [ecx]
	vmovaps xmm4,[ecx]
	vmovdqa xmm6,xmm4
	vmovdqa xmm4,XMMWORD PTR [ecx]
	vmovdqa xmm4,[ecx]
	vmovdqu xmm6,xmm4
	vmovdqu xmm4,XMMWORD PTR [ecx]
	vmovdqu xmm4,[ecx]
	vmovshdup xmm6,xmm4
	vmovshdup xmm4,XMMWORD PTR [ecx]
	vmovshdup xmm4,[ecx]
	vmovsldup xmm6,xmm4
	vmovsldup xmm4,XMMWORD PTR [ecx]
	vmovsldup xmm4,[ecx]
	vmovupd xmm6,xmm4
	vmovupd xmm4,XMMWORD PTR [ecx]
	vmovupd xmm4,[ecx]
	vmovups xmm6,xmm4
	vmovups xmm4,XMMWORD PTR [ecx]
	vmovups xmm4,[ecx]
	vpabsb xmm6,xmm4
	vpabsb xmm4,XMMWORD PTR [ecx]
	vpabsb xmm4,[ecx]
	vpabsw xmm6,xmm4
	vpabsw xmm4,XMMWORD PTR [ecx]
	vpabsw xmm4,[ecx]
	vpabsd xmm6,xmm4
	vpabsd xmm4,XMMWORD PTR [ecx]
	vpabsd xmm4,[ecx]
	vphminposuw xmm6,xmm4
	vphminposuw xmm4,XMMWORD PTR [ecx]
	vphminposuw xmm4,[ecx]
	vptest xmm6,xmm4
	vptest xmm4,XMMWORD PTR [ecx]
	vptest xmm4,[ecx]
	vtestps xmm6,xmm4
	vtestps xmm4,XMMWORD PTR [ecx]
	vtestps xmm4,[ecx]
	vtestpd xmm6,xmm4
	vtestpd xmm4,XMMWORD PTR [ecx]
	vtestpd xmm4,[ecx]
	vrcpps xmm6,xmm4
	vrcpps xmm4,XMMWORD PTR [ecx]
	vrcpps xmm4,[ecx]
	vrsqrtps xmm6,xmm4
	vrsqrtps xmm4,XMMWORD PTR [ecx]
	vrsqrtps xmm4,[ecx]
	vsqrtpd xmm6,xmm4
	vsqrtpd xmm4,XMMWORD PTR [ecx]
	vsqrtpd xmm4,[ecx]
	vsqrtps xmm6,xmm4
	vsqrtps xmm4,XMMWORD PTR [ecx]
	vsqrtps xmm4,[ecx]
	vaesimc xmm6,xmm4
	vaesimc xmm4,XMMWORD PTR [ecx]
	vaesimc xmm4,[ecx]

# Tests for op xmm, xmm/mem128
	vmovapd xmm6,xmm4
	vmovapd XMMWORD PTR [ecx],xmm4
	vmovapd [ecx],xmm4
	vmovaps xmm6,xmm4
	vmovaps XMMWORD PTR [ecx],xmm4
	vmovaps [ecx],xmm4
	vmovdqa xmm6,xmm4
	vmovdqa XMMWORD PTR [ecx],xmm4
	vmovdqa [ecx],xmm4
	vmovdqu xmm6,xmm4
	vmovdqu XMMWORD PTR [ecx],xmm4
	vmovdqu [ecx],xmm4
	vmovupd xmm6,xmm4
	vmovupd XMMWORD PTR [ecx],xmm4
	vmovupd [ecx],xmm4
	vmovups xmm6,xmm4
	vmovups XMMWORD PTR [ecx],xmm4
	vmovups [ecx],xmm4

# Tests for op mem128, xmm
	vlddqu xmm4,XMMWORD PTR [ecx]
	vlddqu xmm4,[ecx]
	vmovntdqa xmm4,XMMWORD PTR [ecx]
	vmovntdqa xmm4,[ecx]

# Tests for op xmm, mem128
	vmovntdq XMMWORD PTR [ecx],xmm4
	vmovntdq [ecx],xmm4
	vmovntpd XMMWORD PTR [ecx],xmm4
	vmovntpd [ecx],xmm4
	vmovntps XMMWORD PTR [ecx],xmm4
	vmovntps [ecx],xmm4

# Tests for op xmm/mem128, ymm
	vcvtdq2pd ymm4,xmm4
	vcvtdq2pd ymm4,XMMWORD PTR [ecx]
	vcvtdq2pd ymm4,[ecx]
	vcvtps2pd ymm4,xmm4
	vcvtps2pd ymm4,XMMWORD PTR [ecx]
	vcvtps2pd ymm4,[ecx]

# Tests for op xmm/mem128, xmm, xmm
	vaddpd xmm2,xmm6,xmm4
	vaddpd xmm7,xmm6,XMMWORD PTR [ecx]
	vaddpd xmm7,xmm6,[ecx]
	vaddps xmm2,xmm6,xmm4
	vaddps xmm7,xmm6,XMMWORD PTR [ecx]
	vaddps xmm7,xmm6,[ecx]
	vaddsubpd xmm2,xmm6,xmm4
	vaddsubpd xmm7,xmm6,XMMWORD PTR [ecx]
	vaddsubpd xmm7,xmm6,[ecx]
	vaddsubps xmm2,xmm6,xmm4
	vaddsubps xmm7,xmm6,XMMWORD PTR [ecx]
	vaddsubps xmm7,xmm6,[ecx]
	vandnpd xmm2,xmm6,xmm4
	vandnpd xmm7,xmm6,XMMWORD PTR [ecx]
	vandnpd xmm7,xmm6,[ecx]
	vandnps xmm2,xmm6,xmm4
	vandnps xmm7,xmm6,XMMWORD PTR [ecx]
	vandnps xmm7,xmm6,[ecx]
	vandpd xmm2,xmm6,xmm4
	vandpd xmm7,xmm6,XMMWORD PTR [ecx]
	vandpd xmm7,xmm6,[ecx]
	vandps xmm2,xmm6,xmm4
	vandps xmm7,xmm6,XMMWORD PTR [ecx]
	vandps xmm7,xmm6,[ecx]
	vdivpd xmm2,xmm6,xmm4
	vdivpd xmm7,xmm6,XMMWORD PTR [ecx]
	vdivpd xmm7,xmm6,[ecx]
	vdivps xmm2,xmm6,xmm4
	vdivps xmm7,xmm6,XMMWORD PTR [ecx]
	vdivps xmm7,xmm6,[ecx]
	vhaddpd xmm2,xmm6,xmm4
	vhaddpd xmm7,xmm6,XMMWORD PTR [ecx]
	vhaddpd xmm7,xmm6,[ecx]
	vhaddps xmm2,xmm6,xmm4
	vhaddps xmm7,xmm6,XMMWORD PTR [ecx]
	vhaddps xmm7,xmm6,[ecx]
	vhsubpd xmm2,xmm6,xmm4
	vhsubpd xmm7,xmm6,XMMWORD PTR [ecx]
	vhsubpd xmm7,xmm6,[ecx]
	vhsubps xmm2,xmm6,xmm4
	vhsubps xmm7,xmm6,XMMWORD PTR [ecx]
	vhsubps xmm7,xmm6,[ecx]
	vmaxpd xmm2,xmm6,xmm4
	vmaxpd xmm7,xmm6,XMMWORD PTR [ecx]
	vmaxpd xmm7,xmm6,[ecx]
	vmaxps xmm2,xmm6,xmm4
	vmaxps xmm7,xmm6,XMMWORD PTR [ecx]
	vmaxps xmm7,xmm6,[ecx]
	vminpd xmm2,xmm6,xmm4
	vminpd xmm7,xmm6,XMMWORD PTR [ecx]
	vminpd xmm7,xmm6,[ecx]
	vminps xmm2,xmm6,xmm4
	vminps xmm7,xmm6,XMMWORD PTR [ecx]
	vminps xmm7,xmm6,[ecx]
	vmulpd xmm2,xmm6,xmm4
	vmulpd xmm7,xmm6,XMMWORD PTR [ecx]
	vmulpd xmm7,xmm6,[ecx]
	vmulps xmm2,xmm6,xmm4
	vmulps xmm7,xmm6,XMMWORD PTR [ecx]
	vmulps xmm7,xmm6,[ecx]
	vorpd xmm2,xmm6,xmm4
	vorpd xmm7,xmm6,XMMWORD PTR [ecx]
	vorpd xmm7,xmm6,[ecx]
	vorps xmm2,xmm6,xmm4
	vorps xmm7,xmm6,XMMWORD PTR [ecx]
	vorps xmm7,xmm6,[ecx]
	vpacksswb xmm2,xmm6,xmm4
	vpacksswb xmm7,xmm6,XMMWORD PTR [ecx]
	vpacksswb xmm7,xmm6,[ecx]
	vpackssdw xmm2,xmm6,xmm4
	vpackssdw xmm7,xmm6,XMMWORD PTR [ecx]
	vpackssdw xmm7,xmm6,[ecx]
	vpackuswb xmm2,xmm6,xmm4
	vpackuswb xmm7,xmm6,XMMWORD PTR [ecx]
	vpackuswb xmm7,xmm6,[ecx]
	vpackusdw xmm2,xmm6,xmm4
	vpackusdw xmm7,xmm6,XMMWORD PTR [ecx]
	vpackusdw xmm7,xmm6,[ecx]
	vpaddb xmm2,xmm6,xmm4
	vpaddb xmm7,xmm6,XMMWORD PTR [ecx]
	vpaddb xmm7,xmm6,[ecx]
	vpaddw xmm2,xmm6,xmm4
	vpaddw xmm7,xmm6,XMMWORD PTR [ecx]
	vpaddw xmm7,xmm6,[ecx]
	vpaddd xmm2,xmm6,xmm4
	vpaddd xmm7,xmm6,XMMWORD PTR [ecx]
	vpaddd xmm7,xmm6,[ecx]
	vpaddq xmm2,xmm6,xmm4
	vpaddq xmm7,xmm6,XMMWORD PTR [ecx]
	vpaddq xmm7,xmm6,[ecx]
	vpaddsb xmm2,xmm6,xmm4
	vpaddsb xmm7,xmm6,XMMWORD PTR [ecx]
	vpaddsb xmm7,xmm6,[ecx]
	vpaddsw xmm2,xmm6,xmm4
	vpaddsw xmm7,xmm6,XMMWORD PTR [ecx]
	vpaddsw xmm7,xmm6,[ecx]
	vpaddusb xmm2,xmm6,xmm4
	vpaddusb xmm7,xmm6,XMMWORD PTR [ecx]
	vpaddusb xmm7,xmm6,[ecx]
	vpaddusw xmm2,xmm6,xmm4
	vpaddusw xmm7,xmm6,XMMWORD PTR [ecx]
	vpaddusw xmm7,xmm6,[ecx]
	vpand xmm2,xmm6,xmm4
	vpand xmm7,xmm6,XMMWORD PTR [ecx]
	vpand xmm7,xmm6,[ecx]
	vpandn xmm2,xmm6,xmm4
	vpandn xmm7,xmm6,XMMWORD PTR [ecx]
	vpandn xmm7,xmm6,[ecx]
	vpavgb xmm2,xmm6,xmm4
	vpavgb xmm7,xmm6,XMMWORD PTR [ecx]
	vpavgb xmm7,xmm6,[ecx]
	vpavgw xmm2,xmm6,xmm4
	vpavgw xmm7,xmm6,XMMWORD PTR [ecx]
	vpavgw xmm7,xmm6,[ecx]
	vpclmullqlqdq xmm2,xmm6,xmm4
	vpclmullqlqdq xmm7,xmm6,XMMWORD PTR [ecx]
	vpclmullqlqdq xmm7,xmm6,[ecx]
	vpclmulhqlqdq xmm2,xmm6,xmm4
	vpclmulhqlqdq xmm7,xmm6,XMMWORD PTR [ecx]
	vpclmulhqlqdq xmm7,xmm6,[ecx]
	vpclmullqhqdq xmm2,xmm6,xmm4
	vpclmullqhqdq xmm7,xmm6,XMMWORD PTR [ecx]
	vpclmullqhqdq xmm7,xmm6,[ecx]
	vpclmulhqhqdq xmm2,xmm6,xmm4
	vpclmulhqhqdq xmm7,xmm6,XMMWORD PTR [ecx]
	vpclmulhqhqdq xmm7,xmm6,[ecx]
	vpcmpeqb xmm2,xmm6,xmm4
	vpcmpeqb xmm7,xmm6,XMMWORD PTR [ecx]
	vpcmpeqb xmm7,xmm6,[ecx]
	vpcmpeqw xmm2,xmm6,xmm4
	vpcmpeqw xmm7,xmm6,XMMWORD PTR [ecx]
	vpcmpeqw xmm7,xmm6,[ecx]
	vpcmpeqd xmm2,xmm6,xmm4
	vpcmpeqd xmm7,xmm6,XMMWORD PTR [ecx]
	vpcmpeqd xmm7,xmm6,[ecx]
	vpcmpeqq xmm2,xmm6,xmm4
	vpcmpeqq xmm7,xmm6,XMMWORD PTR [ecx]
	vpcmpeqq xmm7,xmm6,[ecx]
	vpcmpgtb xmm2,xmm6,xmm4
	vpcmpgtb xmm7,xmm6,XMMWORD PTR [ecx]
	vpcmpgtb xmm7,xmm6,[ecx]
	vpcmpgtw xmm2,xmm6,xmm4
	vpcmpgtw xmm7,xmm6,XMMWORD PTR [ecx]
	vpcmpgtw xmm7,xmm6,[ecx]
	vpcmpgtd xmm2,xmm6,xmm4
	vpcmpgtd xmm7,xmm6,XMMWORD PTR [ecx]
	vpcmpgtd xmm7,xmm6,[ecx]
	vpcmpgtq xmm2,xmm6,xmm4
	vpcmpgtq xmm7,xmm6,XMMWORD PTR [ecx]
	vpcmpgtq xmm7,xmm6,[ecx]
	vpermilpd xmm2,xmm6,xmm4
	vpermilpd xmm7,xmm6,XMMWORD PTR [ecx]
	vpermilpd xmm7,xmm6,[ecx]
	vpermilps xmm2,xmm6,xmm4
	vpermilps xmm7,xmm6,XMMWORD PTR [ecx]
	vpermilps xmm7,xmm6,[ecx]
	vphaddw xmm2,xmm6,xmm4
	vphaddw xmm7,xmm6,XMMWORD PTR [ecx]
	vphaddw xmm7,xmm6,[ecx]
	vphaddd xmm2,xmm6,xmm4
	vphaddd xmm7,xmm6,XMMWORD PTR [ecx]
	vphaddd xmm7,xmm6,[ecx]
	vphaddsw xmm2,xmm6,xmm4
	vphaddsw xmm7,xmm6,XMMWORD PTR [ecx]
	vphaddsw xmm7,xmm6,[ecx]
	vphsubw xmm2,xmm6,xmm4
	vphsubw xmm7,xmm6,XMMWORD PTR [ecx]
	vphsubw xmm7,xmm6,[ecx]
	vphsubd xmm2,xmm6,xmm4
	vphsubd xmm7,xmm6,XMMWORD PTR [ecx]
	vphsubd xmm7,xmm6,[ecx]
	vphsubsw xmm2,xmm6,xmm4
	vphsubsw xmm7,xmm6,XMMWORD PTR [ecx]
	vphsubsw xmm7,xmm6,[ecx]
	vpmaddwd xmm2,xmm6,xmm4
	vpmaddwd xmm7,xmm6,XMMWORD PTR [ecx]
	vpmaddwd xmm7,xmm6,[ecx]
	vpmaddubsw xmm2,xmm6,xmm4
	vpmaddubsw xmm7,xmm6,XMMWORD PTR [ecx]
	vpmaddubsw xmm7,xmm6,[ecx]
	vpmaxsb xmm2,xmm6,xmm4
	vpmaxsb xmm7,xmm6,XMMWORD PTR [ecx]
	vpmaxsb xmm7,xmm6,[ecx]
	vpmaxsw xmm2,xmm6,xmm4
	vpmaxsw xmm7,xmm6,XMMWORD PTR [ecx]
	vpmaxsw xmm7,xmm6,[ecx]
	vpmaxsd xmm2,xmm6,xmm4
	vpmaxsd xmm7,xmm6,XMMWORD PTR [ecx]
	vpmaxsd xmm7,xmm6,[ecx]
	vpmaxub xmm2,xmm6,xmm4
	vpmaxub xmm7,xmm6,XMMWORD PTR [ecx]
	vpmaxub xmm7,xmm6,[ecx]
	vpmaxuw xmm2,xmm6,xmm4
	vpmaxuw xmm7,xmm6,XMMWORD PTR [ecx]
	vpmaxuw xmm7,xmm6,[ecx]
	vpmaxud xmm2,xmm6,xmm4
	vpmaxud xmm7,xmm6,XMMWORD PTR [ecx]
	vpmaxud xmm7,xmm6,[ecx]
	vpminsb xmm2,xmm6,xmm4
	vpminsb xmm7,xmm6,XMMWORD PTR [ecx]
	vpminsb xmm7,xmm6,[ecx]
	vpminsw xmm2,xmm6,xmm4
	vpminsw xmm7,xmm6,XMMWORD PTR [ecx]
	vpminsw xmm7,xmm6,[ecx]
	vpminsd xmm2,xmm6,xmm4
	vpminsd xmm7,xmm6,XMMWORD PTR [ecx]
	vpminsd xmm7,xmm6,[ecx]
	vpminub xmm2,xmm6,xmm4
	vpminub xmm7,xmm6,XMMWORD PTR [ecx]
	vpminub xmm7,xmm6,[ecx]
	vpminuw xmm2,xmm6,xmm4
	vpminuw xmm7,xmm6,XMMWORD PTR [ecx]
	vpminuw xmm7,xmm6,[ecx]
	vpminud xmm2,xmm6,xmm4
	vpminud xmm7,xmm6,XMMWORD PTR [ecx]
	vpminud xmm7,xmm6,[ecx]
	vpmulhuw xmm2,xmm6,xmm4
	vpmulhuw xmm7,xmm6,XMMWORD PTR [ecx]
	vpmulhuw xmm7,xmm6,[ecx]
	vpmulhrsw xmm2,xmm6,xmm4
	vpmulhrsw xmm7,xmm6,XMMWORD PTR [ecx]
	vpmulhrsw xmm7,xmm6,[ecx]
	vpmulhw xmm2,xmm6,xmm4
	vpmulhw xmm7,xmm6,XMMWORD PTR [ecx]
	vpmulhw xmm7,xmm6,[ecx]
	vpmullw xmm2,xmm6,xmm4
	vpmullw xmm7,xmm6,XMMWORD PTR [ecx]
	vpmullw xmm7,xmm6,[ecx]
	vpmulld xmm2,xmm6,xmm4
	vpmulld xmm7,xmm6,XMMWORD PTR [ecx]
	vpmulld xmm7,xmm6,[ecx]
	vpmuludq xmm2,xmm6,xmm4
	vpmuludq xmm7,xmm6,XMMWORD PTR [ecx]
	vpmuludq xmm7,xmm6,[ecx]
	vpmuldq xmm2,xmm6,xmm4
	vpmuldq xmm7,xmm6,XMMWORD PTR [ecx]
	vpmuldq xmm7,xmm6,[ecx]
	vpor xmm2,xmm6,xmm4
	vpor xmm7,xmm6,XMMWORD PTR [ecx]
	vpor xmm7,xmm6,[ecx]
	vpsadbw xmm2,xmm6,xmm4
	vpsadbw xmm7,xmm6,XMMWORD PTR [ecx]
	vpsadbw xmm7,xmm6,[ecx]
	vpshufb xmm2,xmm6,xmm4
	vpshufb xmm7,xmm6,XMMWORD PTR [ecx]
	vpshufb xmm7,xmm6,[ecx]
	vpsignb xmm2,xmm6,xmm4
	vpsignb xmm7,xmm6,XMMWORD PTR [ecx]
	vpsignb xmm7,xmm6,[ecx]
	vpsignw xmm2,xmm6,xmm4
	vpsignw xmm7,xmm6,XMMWORD PTR [ecx]
	vpsignw xmm7,xmm6,[ecx]
	vpsignd xmm2,xmm6,xmm4
	vpsignd xmm7,xmm6,XMMWORD PTR [ecx]
	vpsignd xmm7,xmm6,[ecx]
	vpsllw xmm2,xmm6,xmm4
	vpsllw xmm7,xmm6,XMMWORD PTR [ecx]
	vpsllw xmm7,xmm6,[ecx]
	vpslld xmm2,xmm6,xmm4
	vpslld xmm7,xmm6,XMMWORD PTR [ecx]
	vpslld xmm7,xmm6,[ecx]
	vpsllq xmm2,xmm6,xmm4
	vpsllq xmm7,xmm6,XMMWORD PTR [ecx]
	vpsllq xmm7,xmm6,[ecx]
	vpsraw xmm2,xmm6,xmm4
	vpsraw xmm7,xmm6,XMMWORD PTR [ecx]
	vpsraw xmm7,xmm6,[ecx]
	vpsrad xmm2,xmm6,xmm4
	vpsrad xmm7,xmm6,XMMWORD PTR [ecx]
	vpsrad xmm7,xmm6,[ecx]
	vpsrlw xmm2,xmm6,xmm4
	vpsrlw xmm7,xmm6,XMMWORD PTR [ecx]
	vpsrlw xmm7,xmm6,[ecx]
	vpsrld xmm2,xmm6,xmm4
	vpsrld xmm7,xmm6,XMMWORD PTR [ecx]
	vpsrld xmm7,xmm6,[ecx]
	vpsrlq xmm2,xmm6,xmm4
	vpsrlq xmm7,xmm6,XMMWORD PTR [ecx]
	vpsrlq xmm7,xmm6,[ecx]
	vpsubb xmm2,xmm6,xmm4
	vpsubb xmm7,xmm6,XMMWORD PTR [ecx]
	vpsubb xmm7,xmm6,[ecx]
	vpsubw xmm2,xmm6,xmm4
	vpsubw xmm7,xmm6,XMMWORD PTR [ecx]
	vpsubw xmm7,xmm6,[ecx]
	vpsubd xmm2,xmm6,xmm4
	vpsubd xmm7,xmm6,XMMWORD PTR [ecx]
	vpsubd xmm7,xmm6,[ecx]
	vpsubq xmm2,xmm6,xmm4
	vpsubq xmm7,xmm6,XMMWORD PTR [ecx]
	vpsubq xmm7,xmm6,[ecx]
	vpsubsb xmm2,xmm6,xmm4
	vpsubsb xmm7,xmm6,XMMWORD PTR [ecx]
	vpsubsb xmm7,xmm6,[ecx]
	vpsubsw xmm2,xmm6,xmm4
	vpsubsw xmm7,xmm6,XMMWORD PTR [ecx]
	vpsubsw xmm7,xmm6,[ecx]
	vpsubusb xmm2,xmm6,xmm4
	vpsubusb xmm7,xmm6,XMMWORD PTR [ecx]
	vpsubusb xmm7,xmm6,[ecx]
	vpsubusw xmm2,xmm6,xmm4
	vpsubusw xmm7,xmm6,XMMWORD PTR [ecx]
	vpsubusw xmm7,xmm6,[ecx]
	vpunpckhbw xmm2,xmm6,xmm4
	vpunpckhbw xmm7,xmm6,XMMWORD PTR [ecx]
	vpunpckhbw xmm7,xmm6,[ecx]
	vpunpckhwd xmm2,xmm6,xmm4
	vpunpckhwd xmm7,xmm6,XMMWORD PTR [ecx]
	vpunpckhwd xmm7,xmm6,[ecx]
	vpunpckhdq xmm2,xmm6,xmm4
	vpunpckhdq xmm7,xmm6,XMMWORD PTR [ecx]
	vpunpckhdq xmm7,xmm6,[ecx]
	vpunpckhqdq xmm2,xmm6,xmm4
	vpunpckhqdq xmm7,xmm6,XMMWORD PTR [ecx]
	vpunpckhqdq xmm7,xmm6,[ecx]
	vpunpcklbw xmm2,xmm6,xmm4
	vpunpcklbw xmm7,xmm6,XMMWORD PTR [ecx]
	vpunpcklbw xmm7,xmm6,[ecx]
	vpunpcklwd xmm2,xmm6,xmm4
	vpunpcklwd xmm7,xmm6,XMMWORD PTR [ecx]
	vpunpcklwd xmm7,xmm6,[ecx]
	vpunpckldq xmm2,xmm6,xmm4
	vpunpckldq xmm7,xmm6,XMMWORD PTR [ecx]
	vpunpckldq xmm7,xmm6,[ecx]
	vpunpcklqdq xmm2,xmm6,xmm4
	vpunpcklqdq xmm7,xmm6,XMMWORD PTR [ecx]
	vpunpcklqdq xmm7,xmm6,[ecx]
	vpxor xmm2,xmm6,xmm4
	vpxor xmm7,xmm6,XMMWORD PTR [ecx]
	vpxor xmm7,xmm6,[ecx]
	vsubpd xmm2,xmm6,xmm4
	vsubpd xmm7,xmm6,XMMWORD PTR [ecx]
	vsubpd xmm7,xmm6,[ecx]
	vsubps xmm2,xmm6,xmm4
	vsubps xmm7,xmm6,XMMWORD PTR [ecx]
	vsubps xmm7,xmm6,[ecx]
	vunpckhpd xmm2,xmm6,xmm4
	vunpckhpd xmm7,xmm6,XMMWORD PTR [ecx]
	vunpckhpd xmm7,xmm6,[ecx]
	vunpckhps xmm2,xmm6,xmm4
	vunpckhps xmm7,xmm6,XMMWORD PTR [ecx]
	vunpckhps xmm7,xmm6,[ecx]
	vunpcklpd xmm2,xmm6,xmm4
	vunpcklpd xmm7,xmm6,XMMWORD PTR [ecx]
	vunpcklpd xmm7,xmm6,[ecx]
	vunpcklps xmm2,xmm6,xmm4
	vunpcklps xmm7,xmm6,XMMWORD PTR [ecx]
	vunpcklps xmm7,xmm6,[ecx]
	vxorpd xmm2,xmm6,xmm4
	vxorpd xmm7,xmm6,XMMWORD PTR [ecx]
	vxorpd xmm7,xmm6,[ecx]
	vxorps xmm2,xmm6,xmm4
	vxorps xmm7,xmm6,XMMWORD PTR [ecx]
	vxorps xmm7,xmm6,[ecx]
	vaesenc xmm2,xmm6,xmm4
	vaesenc xmm7,xmm6,XMMWORD PTR [ecx]
	vaesenc xmm7,xmm6,[ecx]
	vaesenclast xmm2,xmm6,xmm4
	vaesenclast xmm7,xmm6,XMMWORD PTR [ecx]
	vaesenclast xmm7,xmm6,[ecx]
	vaesdec xmm2,xmm6,xmm4
	vaesdec xmm7,xmm6,XMMWORD PTR [ecx]
	vaesdec xmm7,xmm6,[ecx]
	vaesdeclast xmm2,xmm6,xmm4
	vaesdeclast xmm7,xmm6,XMMWORD PTR [ecx]
	vaesdeclast xmm7,xmm6,[ecx]
	vcmpeqpd xmm2,xmm6,xmm4
	vcmpeqpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpeqpd xmm7,xmm6,[ecx]
	vcmpltpd xmm2,xmm6,xmm4
	vcmpltpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpltpd xmm7,xmm6,[ecx]
	vcmplepd xmm2,xmm6,xmm4
	vcmplepd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmplepd xmm7,xmm6,[ecx]
	vcmpunordpd xmm2,xmm6,xmm4
	vcmpunordpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpunordpd xmm7,xmm6,[ecx]
	vcmpneqpd xmm2,xmm6,xmm4
	vcmpneqpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpneqpd xmm7,xmm6,[ecx]
	vcmpnltpd xmm2,xmm6,xmm4
	vcmpnltpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpnltpd xmm7,xmm6,[ecx]
	vcmpnlepd xmm2,xmm6,xmm4
	vcmpnlepd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpnlepd xmm7,xmm6,[ecx]
	vcmpordpd xmm2,xmm6,xmm4
	vcmpordpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpordpd xmm7,xmm6,[ecx]
	vcmpeq_uqpd xmm2,xmm6,xmm4
	vcmpeq_uqpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpeq_uqpd xmm7,xmm6,[ecx]
	vcmpngepd xmm2,xmm6,xmm4
	vcmpngepd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpngepd xmm7,xmm6,[ecx]
	vcmpngtpd xmm2,xmm6,xmm4
	vcmpngtpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpngtpd xmm7,xmm6,[ecx]
	vcmpfalsepd xmm2,xmm6,xmm4
	vcmpfalsepd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpfalsepd xmm7,xmm6,[ecx]
	vcmpneq_oqpd xmm2,xmm6,xmm4
	vcmpneq_oqpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpneq_oqpd xmm7,xmm6,[ecx]
	vcmpgepd xmm2,xmm6,xmm4
	vcmpgepd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpgepd xmm7,xmm6,[ecx]
	vcmpgtpd xmm2,xmm6,xmm4
	vcmpgtpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpgtpd xmm7,xmm6,[ecx]
	vcmptruepd xmm2,xmm6,xmm4
	vcmptruepd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmptruepd xmm7,xmm6,[ecx]
	vcmpeq_ospd xmm2,xmm6,xmm4
	vcmpeq_ospd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpeq_ospd xmm7,xmm6,[ecx]
	vcmplt_oqpd xmm2,xmm6,xmm4
	vcmplt_oqpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmplt_oqpd xmm7,xmm6,[ecx]
	vcmple_oqpd xmm2,xmm6,xmm4
	vcmple_oqpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmple_oqpd xmm7,xmm6,[ecx]
	vcmpunord_spd xmm2,xmm6,xmm4
	vcmpunord_spd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpunord_spd xmm7,xmm6,[ecx]
	vcmpneq_uspd xmm2,xmm6,xmm4
	vcmpneq_uspd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpneq_uspd xmm7,xmm6,[ecx]
	vcmpnlt_uqpd xmm2,xmm6,xmm4
	vcmpnlt_uqpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpnlt_uqpd xmm7,xmm6,[ecx]
	vcmpnle_uqpd xmm2,xmm6,xmm4
	vcmpnle_uqpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpnle_uqpd xmm7,xmm6,[ecx]
	vcmpord_spd xmm2,xmm6,xmm4
	vcmpord_spd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpord_spd xmm7,xmm6,[ecx]
	vcmpeq_uspd xmm2,xmm6,xmm4
	vcmpeq_uspd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpeq_uspd xmm7,xmm6,[ecx]
	vcmpnge_uqpd xmm2,xmm6,xmm4
	vcmpnge_uqpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpnge_uqpd xmm7,xmm6,[ecx]
	vcmpngt_uqpd xmm2,xmm6,xmm4
	vcmpngt_uqpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpngt_uqpd xmm7,xmm6,[ecx]
	vcmpfalse_ospd xmm2,xmm6,xmm4
	vcmpfalse_ospd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpfalse_ospd xmm7,xmm6,[ecx]
	vcmpneq_ospd xmm2,xmm6,xmm4
	vcmpneq_ospd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpneq_ospd xmm7,xmm6,[ecx]
	vcmpge_oqpd xmm2,xmm6,xmm4
	vcmpge_oqpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpge_oqpd xmm7,xmm6,[ecx]
	vcmpgt_oqpd xmm2,xmm6,xmm4
	vcmpgt_oqpd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpgt_oqpd xmm7,xmm6,[ecx]
	vcmptrue_uspd xmm2,xmm6,xmm4
	vcmptrue_uspd xmm7,xmm6,XMMWORD PTR [ecx]
	vcmptrue_uspd xmm7,xmm6,[ecx]
	vcmpeqps xmm2,xmm6,xmm4
	vcmpeqps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpeqps xmm7,xmm6,[ecx]
	vcmpltps xmm2,xmm6,xmm4
	vcmpltps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpltps xmm7,xmm6,[ecx]
	vcmpleps xmm2,xmm6,xmm4
	vcmpleps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpleps xmm7,xmm6,[ecx]
	vcmpunordps xmm2,xmm6,xmm4
	vcmpunordps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpunordps xmm7,xmm6,[ecx]
	vcmpneqps xmm2,xmm6,xmm4
	vcmpneqps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpneqps xmm7,xmm6,[ecx]
	vcmpnltps xmm2,xmm6,xmm4
	vcmpnltps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpnltps xmm7,xmm6,[ecx]
	vcmpnleps xmm2,xmm6,xmm4
	vcmpnleps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpnleps xmm7,xmm6,[ecx]
	vcmpordps xmm2,xmm6,xmm4
	vcmpordps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpordps xmm7,xmm6,[ecx]
	vcmpeq_uqps xmm2,xmm6,xmm4
	vcmpeq_uqps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpeq_uqps xmm7,xmm6,[ecx]
	vcmpngeps xmm2,xmm6,xmm4
	vcmpngeps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpngeps xmm7,xmm6,[ecx]
	vcmpngtps xmm2,xmm6,xmm4
	vcmpngtps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpngtps xmm7,xmm6,[ecx]
	vcmpfalseps xmm2,xmm6,xmm4
	vcmpfalseps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpfalseps xmm7,xmm6,[ecx]
	vcmpneq_oqps xmm2,xmm6,xmm4
	vcmpneq_oqps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpneq_oqps xmm7,xmm6,[ecx]
	vcmpgeps xmm2,xmm6,xmm4
	vcmpgeps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpgeps xmm7,xmm6,[ecx]
	vcmpgtps xmm2,xmm6,xmm4
	vcmpgtps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpgtps xmm7,xmm6,[ecx]
	vcmptrueps xmm2,xmm6,xmm4
	vcmptrueps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmptrueps xmm7,xmm6,[ecx]
	vcmpeq_osps xmm2,xmm6,xmm4
	vcmpeq_osps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpeq_osps xmm7,xmm6,[ecx]
	vcmplt_oqps xmm2,xmm6,xmm4
	vcmplt_oqps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmplt_oqps xmm7,xmm6,[ecx]
	vcmple_oqps xmm2,xmm6,xmm4
	vcmple_oqps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmple_oqps xmm7,xmm6,[ecx]
	vcmpunord_sps xmm2,xmm6,xmm4
	vcmpunord_sps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpunord_sps xmm7,xmm6,[ecx]
	vcmpneq_usps xmm2,xmm6,xmm4
	vcmpneq_usps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpneq_usps xmm7,xmm6,[ecx]
	vcmpnlt_uqps xmm2,xmm6,xmm4
	vcmpnlt_uqps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpnlt_uqps xmm7,xmm6,[ecx]
	vcmpnle_uqps xmm2,xmm6,xmm4
	vcmpnle_uqps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpnle_uqps xmm7,xmm6,[ecx]
	vcmpord_sps xmm2,xmm6,xmm4
	vcmpord_sps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpord_sps xmm7,xmm6,[ecx]
	vcmpeq_usps xmm2,xmm6,xmm4
	vcmpeq_usps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpeq_usps xmm7,xmm6,[ecx]
	vcmpnge_uqps xmm2,xmm6,xmm4
	vcmpnge_uqps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpnge_uqps xmm7,xmm6,[ecx]
	vcmpngt_uqps xmm2,xmm6,xmm4
	vcmpngt_uqps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpngt_uqps xmm7,xmm6,[ecx]
	vcmpfalse_osps xmm2,xmm6,xmm4
	vcmpfalse_osps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpfalse_osps xmm7,xmm6,[ecx]
	vcmpneq_osps xmm2,xmm6,xmm4
	vcmpneq_osps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpneq_osps xmm7,xmm6,[ecx]
	vcmpge_oqps xmm2,xmm6,xmm4
	vcmpge_oqps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpge_oqps xmm7,xmm6,[ecx]
	vcmpgt_oqps xmm2,xmm6,xmm4
	vcmpgt_oqps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmpgt_oqps xmm7,xmm6,[ecx]
	vcmptrue_usps xmm2,xmm6,xmm4
	vcmptrue_usps xmm7,xmm6,XMMWORD PTR [ecx]
	vcmptrue_usps xmm7,xmm6,[ecx]
    vgf2p8mulb xmm6, xmm5, xmm4
	vgf2p8mulb xmm6, xmm5, XMMWORD PTR [ecx]
	vgf2p8mulb xmm6, xmm5, [ecx]
	vgf2p8mulb xmm6, xmm5, XMMWORD PTR [esp+esi*8-123456]
	vgf2p8mulb xmm6, xmm5, XMMWORD PTR [edx+2032]
	vgf2p8mulb xmm6, xmm5, XMMWORD PTR [edx+2048]
	vgf2p8mulb xmm6, xmm5, XMMWORD PTR [edx-2048]
	vgf2p8mulb xmm6, xmm5, XMMWORD PTR [edx-2064]

# Tests for op mem128, xmm, xmm
	vmaskmovps xmm6,xmm4,XMMWORD PTR [ecx]
	vmaskmovps xmm6,xmm4,[ecx]
	vmaskmovpd xmm6,xmm4,XMMWORD PTR [ecx]
	vmaskmovpd xmm6,xmm4,[ecx]

# Tests for op imm8, xmm/mem128, xmm
	vaeskeygenassist xmm6,xmm4,7
	vaeskeygenassist xmm6,XMMWORD PTR [ecx],7
	vaeskeygenassist xmm6,[ecx],7
	vpcmpestri xmm6,xmm4,7
	vpcmpestri xmm6,XMMWORD PTR [ecx],7
	vpcmpestri xmm6,[ecx],7
	vpcmpestrm xmm6,xmm4,7
	vpcmpestrm xmm6,XMMWORD PTR [ecx],7
	vpcmpestrm xmm6,[ecx],7
	vpcmpistri xmm6,xmm4,7
	vpcmpistri xmm6,XMMWORD PTR [ecx],7
	vpcmpistri xmm6,[ecx],7
	vpcmpistrm xmm6,xmm4,7
	vpcmpistrm xmm6,XMMWORD PTR [ecx],7
	vpcmpistrm xmm6,[ecx],7
	vpermilpd xmm6,xmm4,7
	vpermilpd xmm6,XMMWORD PTR [ecx],7
	vpermilpd xmm6,[ecx],7
	vpermilps xmm6,xmm4,7
	vpermilps xmm6,XMMWORD PTR [ecx],7
	vpermilps xmm6,[ecx],7
	vpshufd xmm6,xmm4,7
	vpshufd xmm6,XMMWORD PTR [ecx],7
	vpshufd xmm6,[ecx],7
	vpshufhw xmm6,xmm4,7
	vpshufhw xmm6,XMMWORD PTR [ecx],7
	vpshufhw xmm6,[ecx],7
	vpshuflw xmm6,xmm4,7
	vpshuflw xmm6,XMMWORD PTR [ecx],7
	vpshuflw xmm6,[ecx],7
	vroundpd xmm6,xmm4,7
	vroundpd xmm6,XMMWORD PTR [ecx],7
	vroundpd xmm6,[ecx],7
	vroundps xmm6,xmm4,7
	vroundps xmm6,XMMWORD PTR [ecx],7
	vroundps xmm6,[ecx],7

# Tests for op xmm, xmm, mem128
	vmaskmovps XMMWORD PTR [ecx],xmm6,xmm4
	vmaskmovps [ecx],xmm6,xmm4
	vmaskmovpd XMMWORD PTR [ecx],xmm6,xmm4
	vmaskmovpd [ecx],xmm6,xmm4

# Tests for op imm8, xmm/mem128, xmm, xmm
	vblendpd xmm2,xmm6,xmm4,7
	vblendpd xmm2,xmm6,XMMWORD PTR [ecx],7
	vblendpd xmm2,xmm6,[ecx],7
	vblendps xmm2,xmm6,xmm4,7
	vblendps xmm2,xmm6,XMMWORD PTR [ecx],7
	vblendps xmm2,xmm6,[ecx],7
	vcmppd xmm2,xmm6,xmm4,7
	vcmppd xmm2,xmm6,XMMWORD PTR [ecx],7
	vcmppd xmm2,xmm6,[ecx],7
	vcmpps xmm2,xmm6,xmm4,7
	vcmpps xmm2,xmm6,XMMWORD PTR [ecx],7
	vcmpps xmm2,xmm6,[ecx],7
	vdppd xmm2,xmm6,xmm4,7
	vdppd xmm2,xmm6,XMMWORD PTR [ecx],7
	vdppd xmm2,xmm6,[ecx],7
	vdpps xmm2,xmm6,xmm4,7
	vdpps xmm2,xmm6,XMMWORD PTR [ecx],7
	vdpps xmm2,xmm6,[ecx],7
	vmpsadbw xmm2,xmm6,xmm4,7
	vmpsadbw xmm2,xmm6,XMMWORD PTR [ecx],7
	vmpsadbw xmm2,xmm6,[ecx],7
	vpalignr xmm2,xmm6,xmm4,7
	vpalignr xmm2,xmm6,XMMWORD PTR [ecx],7
	vpalignr xmm2,xmm6,[ecx],7
	vpblendw xmm2,xmm6,xmm4,7
	vpblendw xmm2,xmm6,XMMWORD PTR [ecx],7
	vpblendw xmm2,xmm6,[ecx],7
	vpclmulqdq xmm2,xmm6,xmm4,7
	vpclmulqdq xmm2,xmm6,XMMWORD PTR [ecx],7
	vpclmulqdq xmm2,xmm6,[ecx],7
	vshufpd xmm2,xmm6,xmm4,7
	vshufpd xmm2,xmm6,XMMWORD PTR [ecx],7
	vshufpd xmm2,xmm6,[ecx],7
	vshufps xmm2,xmm6,xmm4,7
	vshufps xmm2,xmm6,XMMWORD PTR [ecx],7
	vshufps xmm2,xmm6,[ecx],7
	vgf2p8affineqb xmm6, xmm5, xmm4, 0xab
	vgf2p8affineqb xmm6, xmm5, xmm4, 123
	vgf2p8affineqb xmm6, xmm5, XMMWORD PTR [ecx], 123
	vgf2p8affineqb xmm6, xmm5, [ecx], 123
	vgf2p8affineqb xmm6, xmm5, XMMWORD PTR [esp+esi*8-123456], 123
	vgf2p8affineqb xmm6, xmm5, XMMWORD PTR [edx+2032], 123
	vgf2p8affineqb xmm6, xmm5, XMMWORD PTR [edx+2048], 123
	vgf2p8affineqb xmm6, xmm5, XMMWORD PTR [edx-2048], 123
	vgf2p8affineqb xmm6, xmm5, XMMWORD PTR [edx-2064], 123
	vgf2p8affineinvqb xmm6, xmm5, xmm4, 0xab
	vgf2p8affineinvqb xmm6, xmm5, xmm4, 123
	vgf2p8affineinvqb xmm6, xmm5, XMMWORD PTR [ecx], 123
	vgf2p8affineinvqb xmm6, xmm5, [ecx], 123
	vgf2p8affineinvqb xmm6, xmm5, XMMWORD PTR [esp+esi*8-123456], 123
	vgf2p8affineinvqb xmm6, xmm5, XMMWORD PTR [edx+2032], 123
	vgf2p8affineinvqb xmm6, xmm5, XMMWORD PTR [edx+2048], 123
	vgf2p8affineinvqb xmm6, xmm5, XMMWORD PTR [edx-2048], 123
	vgf2p8affineinvqb xmm6, xmm5, XMMWORD PTR [edx-2064], 123

# Tests for op xmm, xmm/mem128, xmm, xmm
	vblendvpd xmm7,xmm2,xmm6,xmm4
	vblendvpd xmm7,xmm2,XMMWORD PTR [ecx],xmm4
	vblendvpd xmm7,xmm2,[ecx],xmm4
	vblendvps xmm7,xmm2,xmm6,xmm4
	vblendvps xmm7,xmm2,XMMWORD PTR [ecx],xmm4
	vblendvps xmm7,xmm2,[ecx],xmm4
	vpblendvb xmm7,xmm2,xmm6,xmm4
	vpblendvb xmm7,xmm2,XMMWORD PTR [ecx],xmm4
	vpblendvb xmm7,xmm2,[ecx],xmm4

# Tests for op mem64, ymm
	vbroadcastsd ymm4,QWORD PTR [ecx]
	vbroadcastsd ymm4,[ecx]

# Tests for op xmm/mem64, xmm
	vcomisd xmm6,xmm4
	vcomisd xmm4,QWORD PTR [ecx]
	vcomisd xmm4,[ecx]
	vcvtdq2pd xmm6,xmm4
	vcvtdq2pd xmm4,QWORD PTR [ecx]
	vcvtdq2pd xmm4,[ecx]
	vcvtps2pd xmm6,xmm4
	vcvtps2pd xmm4,QWORD PTR [ecx]
	vcvtps2pd xmm4,[ecx]
	vmovddup xmm6,xmm4
	vmovddup xmm4,QWORD PTR [ecx]
	vmovddup xmm4,[ecx]
	vpmovsxbw xmm6,xmm4
	vpmovsxbw xmm4,QWORD PTR [ecx]
	vpmovsxbw xmm4,[ecx]
	vpmovsxwd xmm6,xmm4
	vpmovsxwd xmm4,QWORD PTR [ecx]
	vpmovsxwd xmm4,[ecx]
	vpmovsxdq xmm6,xmm4
	vpmovsxdq xmm4,QWORD PTR [ecx]
	vpmovsxdq xmm4,[ecx]
	vpmovzxbw xmm6,xmm4
	vpmovzxbw xmm4,QWORD PTR [ecx]
	vpmovzxbw xmm4,[ecx]
	vpmovzxwd xmm6,xmm4
	vpmovzxwd xmm4,QWORD PTR [ecx]
	vpmovzxwd xmm4,[ecx]
	vpmovzxdq xmm6,xmm4
	vpmovzxdq xmm4,QWORD PTR [ecx]
	vpmovzxdq xmm4,[ecx]
	vucomisd xmm6,xmm4
	vucomisd xmm4,QWORD PTR [ecx]
	vucomisd xmm4,[ecx]

# Tests for op mem64, xmm
	vmovsd xmm4,QWORD PTR [ecx]
	vmovsd xmm4,[ecx]

# Tests for op xmm, mem64
	vmovlpd QWORD PTR [ecx],xmm4
	vmovlpd [ecx],xmm4
	vmovlps QWORD PTR [ecx],xmm4
	vmovlps [ecx],xmm4
	vmovhpd QWORD PTR [ecx],xmm4
	vmovhpd [ecx],xmm4
	vmovhps QWORD PTR [ecx],xmm4
	vmovhps [ecx],xmm4
	vmovsd QWORD PTR [ecx],xmm4
	vmovsd [ecx],xmm4

# Tests for op xmm, regq/mem64
# Tests for op regq/mem64, xmm
	vmovq QWORD PTR [ecx],xmm4
	vmovq xmm4,QWORD PTR [ecx]
	vmovq [ecx],xmm4
	vmovq xmm4,[ecx]

# Tests for op xmm/mem64, regl
	vcvtsd2si ecx,xmm4
	vcvtsd2si ecx,QWORD PTR [ecx]
	vcvtsd2si ecx,[ecx]
	vcvttsd2si ecx,xmm4
	vcvttsd2si ecx,QWORD PTR [ecx]
	vcvttsd2si ecx,[ecx]

# Tests for op mem64, xmm, xmm
	vmovlpd xmm6,xmm4,QWORD PTR [ecx]
	vmovlpd xmm6,xmm4,[ecx]
	vmovlps xmm6,xmm4,QWORD PTR [ecx]
	vmovlps xmm6,xmm4,[ecx]
	vmovhpd xmm6,xmm4,QWORD PTR [ecx]
	vmovhpd xmm6,xmm4,[ecx]
	vmovhps xmm6,xmm4,QWORD PTR [ecx]
	vmovhps xmm6,xmm4,[ecx]

# Tests for op imm8, xmm/mem64, xmm, xmm
	vcmpsd xmm2,xmm6,xmm4,7
	vcmpsd xmm2,xmm6,QWORD PTR [ecx],7
	vcmpsd xmm2,xmm6,[ecx],7
	vroundsd xmm2,xmm6,xmm4,7
	vroundsd xmm2,xmm6,QWORD PTR [ecx],7
	vroundsd xmm2,xmm6,[ecx],7

# Tests for op xmm/mem64, xmm, xmm
	vaddsd xmm2,xmm6,xmm4
	vaddsd xmm2,xmm6,QWORD PTR [ecx]
	vaddsd xmm2,xmm6,[ecx]
	vcvtsd2ss xmm2,xmm6,xmm4
	vcvtsd2ss xmm2,xmm6,QWORD PTR [ecx]
	vcvtsd2ss xmm2,xmm6,[ecx]
	vdivsd xmm2,xmm6,xmm4
	vdivsd xmm2,xmm6,QWORD PTR [ecx]
	vdivsd xmm2,xmm6,[ecx]
	vmaxsd xmm2,xmm6,xmm4
	vmaxsd xmm2,xmm6,QWORD PTR [ecx]
	vmaxsd xmm2,xmm6,[ecx]
	vminsd xmm2,xmm6,xmm4
	vminsd xmm2,xmm6,QWORD PTR [ecx]
	vminsd xmm2,xmm6,[ecx]
	vmulsd xmm2,xmm6,xmm4
	vmulsd xmm2,xmm6,QWORD PTR [ecx]
	vmulsd xmm2,xmm6,[ecx]
	vsqrtsd xmm2,xmm6,xmm4
	vsqrtsd xmm2,xmm6,QWORD PTR [ecx]
	vsqrtsd xmm2,xmm6,[ecx]
	vsubsd xmm2,xmm6,xmm4
	vsubsd xmm2,xmm6,QWORD PTR [ecx]
	vsubsd xmm2,xmm6,[ecx]
	vcmpeqsd xmm2,xmm6,xmm4
	vcmpeqsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpeqsd xmm2,xmm6,[ecx]
	vcmpltsd xmm2,xmm6,xmm4
	vcmpltsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpltsd xmm2,xmm6,[ecx]
	vcmplesd xmm2,xmm6,xmm4
	vcmplesd xmm2,xmm6,QWORD PTR [ecx]
	vcmplesd xmm2,xmm6,[ecx]
	vcmpunordsd xmm2,xmm6,xmm4
	vcmpunordsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpunordsd xmm2,xmm6,[ecx]
	vcmpneqsd xmm2,xmm6,xmm4
	vcmpneqsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpneqsd xmm2,xmm6,[ecx]
	vcmpnltsd xmm2,xmm6,xmm4
	vcmpnltsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpnltsd xmm2,xmm6,[ecx]
	vcmpnlesd xmm2,xmm6,xmm4
	vcmpnlesd xmm2,xmm6,QWORD PTR [ecx]
	vcmpnlesd xmm2,xmm6,[ecx]
	vcmpordsd xmm2,xmm6,xmm4
	vcmpordsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpordsd xmm2,xmm6,[ecx]
	vcmpeq_uqsd xmm2,xmm6,xmm4
	vcmpeq_uqsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpeq_uqsd xmm2,xmm6,[ecx]
	vcmpngesd xmm2,xmm6,xmm4
	vcmpngesd xmm2,xmm6,QWORD PTR [ecx]
	vcmpngesd xmm2,xmm6,[ecx]
	vcmpngtsd xmm2,xmm6,xmm4
	vcmpngtsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpngtsd xmm2,xmm6,[ecx]
	vcmpfalsesd xmm2,xmm6,xmm4
	vcmpfalsesd xmm2,xmm6,QWORD PTR [ecx]
	vcmpfalsesd xmm2,xmm6,[ecx]
	vcmpneq_oqsd xmm2,xmm6,xmm4
	vcmpneq_oqsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpneq_oqsd xmm2,xmm6,[ecx]
	vcmpgesd xmm2,xmm6,xmm4
	vcmpgesd xmm2,xmm6,QWORD PTR [ecx]
	vcmpgesd xmm2,xmm6,[ecx]
	vcmpgtsd xmm2,xmm6,xmm4
	vcmpgtsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpgtsd xmm2,xmm6,[ecx]
	vcmptruesd xmm2,xmm6,xmm4
	vcmptruesd xmm2,xmm6,QWORD PTR [ecx]
	vcmptruesd xmm2,xmm6,[ecx]
	vcmpeq_ossd xmm2,xmm6,xmm4
	vcmpeq_ossd xmm2,xmm6,QWORD PTR [ecx]
	vcmpeq_ossd xmm2,xmm6,[ecx]
	vcmplt_oqsd xmm2,xmm6,xmm4
	vcmplt_oqsd xmm2,xmm6,QWORD PTR [ecx]
	vcmplt_oqsd xmm2,xmm6,[ecx]
	vcmple_oqsd xmm2,xmm6,xmm4
	vcmple_oqsd xmm2,xmm6,QWORD PTR [ecx]
	vcmple_oqsd xmm2,xmm6,[ecx]
	vcmpunord_ssd xmm2,xmm6,xmm4
	vcmpunord_ssd xmm2,xmm6,QWORD PTR [ecx]
	vcmpunord_ssd xmm2,xmm6,[ecx]
	vcmpneq_ussd xmm2,xmm6,xmm4
	vcmpneq_ussd xmm2,xmm6,QWORD PTR [ecx]
	vcmpneq_ussd xmm2,xmm6,[ecx]
	vcmpnlt_uqsd xmm2,xmm6,xmm4
	vcmpnlt_uqsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpnlt_uqsd xmm2,xmm6,[ecx]
	vcmpnle_uqsd xmm2,xmm6,xmm4
	vcmpnle_uqsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpnle_uqsd xmm2,xmm6,[ecx]
	vcmpord_ssd xmm2,xmm6,xmm4
	vcmpord_ssd xmm2,xmm6,QWORD PTR [ecx]
	vcmpord_ssd xmm2,xmm6,[ecx]
	vcmpeq_ussd xmm2,xmm6,xmm4
	vcmpeq_ussd xmm2,xmm6,QWORD PTR [ecx]
	vcmpeq_ussd xmm2,xmm6,[ecx]
	vcmpnge_uqsd xmm2,xmm6,xmm4
	vcmpnge_uqsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpnge_uqsd xmm2,xmm6,[ecx]
	vcmpngt_uqsd xmm2,xmm6,xmm4
	vcmpngt_uqsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpngt_uqsd xmm2,xmm6,[ecx]
	vcmpfalse_ossd xmm2,xmm6,xmm4
	vcmpfalse_ossd xmm2,xmm6,QWORD PTR [ecx]
	vcmpfalse_ossd xmm2,xmm6,[ecx]
	vcmpneq_ossd xmm2,xmm6,xmm4
	vcmpneq_ossd xmm2,xmm6,QWORD PTR [ecx]
	vcmpneq_ossd xmm2,xmm6,[ecx]
	vcmpge_oqsd xmm2,xmm6,xmm4
	vcmpge_oqsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpge_oqsd xmm2,xmm6,[ecx]
	vcmpgt_oqsd xmm2,xmm6,xmm4
	vcmpgt_oqsd xmm2,xmm6,QWORD PTR [ecx]
	vcmpgt_oqsd xmm2,xmm6,[ecx]
	vcmptrue_ussd xmm2,xmm6,xmm4
	vcmptrue_ussd xmm2,xmm6,QWORD PTR [ecx]
	vcmptrue_ussd xmm2,xmm6,[ecx]

# Tests for op mem64
	vldmxcsr DWORD PTR [ecx]
	vldmxcsr [ecx]
	vstmxcsr DWORD PTR [ecx]
	vstmxcsr [ecx]

# Tests for op xmm/mem32, xmm, xmm
	vaddss xmm2,xmm6,xmm4
	vaddss xmm2,xmm6,DWORD PTR [ecx]
	vaddss xmm2,xmm6,[ecx]
	vcvtss2sd xmm2,xmm6,xmm4
	vcvtss2sd xmm2,xmm6,DWORD PTR [ecx]
	vcvtss2sd xmm2,xmm6,[ecx]
	vdivss xmm2,xmm6,xmm4
	vdivss xmm2,xmm6,DWORD PTR [ecx]
	vdivss xmm2,xmm6,[ecx]
	vmaxss xmm2,xmm6,xmm4
	vmaxss xmm2,xmm6,DWORD PTR [ecx]
	vmaxss xmm2,xmm6,[ecx]
	vminss xmm2,xmm6,xmm4
	vminss xmm2,xmm6,DWORD PTR [ecx]
	vminss xmm2,xmm6,[ecx]
	vmulss xmm2,xmm6,xmm4
	vmulss xmm2,xmm6,DWORD PTR [ecx]
	vmulss xmm2,xmm6,[ecx]
	vrcpss xmm2,xmm6,xmm4
	vrcpss xmm2,xmm6,DWORD PTR [ecx]
	vrcpss xmm2,xmm6,[ecx]
	vrsqrtss xmm2,xmm6,xmm4
	vrsqrtss xmm2,xmm6,DWORD PTR [ecx]
	vrsqrtss xmm2,xmm6,[ecx]
	vsqrtss xmm2,xmm6,xmm4
	vsqrtss xmm2,xmm6,DWORD PTR [ecx]
	vsqrtss xmm2,xmm6,[ecx]
	vsubss xmm2,xmm6,xmm4
	vsubss xmm2,xmm6,DWORD PTR [ecx]
	vsubss xmm2,xmm6,[ecx]
	vcmpeqss xmm2,xmm6,xmm4
	vcmpeqss xmm2,xmm6,DWORD PTR [ecx]
	vcmpeqss xmm2,xmm6,[ecx]
	vcmpltss xmm2,xmm6,xmm4
	vcmpltss xmm2,xmm6,DWORD PTR [ecx]
	vcmpltss xmm2,xmm6,[ecx]
	vcmpless xmm2,xmm6,xmm4
	vcmpless xmm2,xmm6,DWORD PTR [ecx]
	vcmpless xmm2,xmm6,[ecx]
	vcmpunordss xmm2,xmm6,xmm4
	vcmpunordss xmm2,xmm6,DWORD PTR [ecx]
	vcmpunordss xmm2,xmm6,[ecx]
	vcmpneqss xmm2,xmm6,xmm4
	vcmpneqss xmm2,xmm6,DWORD PTR [ecx]
	vcmpneqss xmm2,xmm6,[ecx]
	vcmpnltss xmm2,xmm6,xmm4
	vcmpnltss xmm2,xmm6,DWORD PTR [ecx]
	vcmpnltss xmm2,xmm6,[ecx]
	vcmpnless xmm2,xmm6,xmm4
	vcmpnless xmm2,xmm6,DWORD PTR [ecx]
	vcmpnless xmm2,xmm6,[ecx]
	vcmpordss xmm2,xmm6,xmm4
	vcmpordss xmm2,xmm6,DWORD PTR [ecx]
	vcmpordss xmm2,xmm6,[ecx]
	vcmpeq_uqss xmm2,xmm6,xmm4
	vcmpeq_uqss xmm2,xmm6,DWORD PTR [ecx]
	vcmpeq_uqss xmm2,xmm6,[ecx]
	vcmpngess xmm2,xmm6,xmm4
	vcmpngess xmm2,xmm6,DWORD PTR [ecx]
	vcmpngess xmm2,xmm6,[ecx]
	vcmpngtss xmm2,xmm6,xmm4
	vcmpngtss xmm2,xmm6,DWORD PTR [ecx]
	vcmpngtss xmm2,xmm6,[ecx]
	vcmpfalsess xmm2,xmm6,xmm4
	vcmpfalsess xmm2,xmm6,DWORD PTR [ecx]
	vcmpfalsess xmm2,xmm6,[ecx]
	vcmpneq_oqss xmm2,xmm6,xmm4
	vcmpneq_oqss xmm2,xmm6,DWORD PTR [ecx]
	vcmpneq_oqss xmm2,xmm6,[ecx]
	vcmpgess xmm2,xmm6,xmm4
	vcmpgess xmm2,xmm6,DWORD PTR [ecx]
	vcmpgess xmm2,xmm6,[ecx]
	vcmpgtss xmm2,xmm6,xmm4
	vcmpgtss xmm2,xmm6,DWORD PTR [ecx]
	vcmpgtss xmm2,xmm6,[ecx]
	vcmptruess xmm2,xmm6,xmm4
	vcmptruess xmm2,xmm6,DWORD PTR [ecx]
	vcmptruess xmm2,xmm6,[ecx]
	vcmpeq_osss xmm2,xmm6,xmm4
	vcmpeq_osss xmm2,xmm6,DWORD PTR [ecx]
	vcmpeq_osss xmm2,xmm6,[ecx]
	vcmplt_oqss xmm2,xmm6,xmm4
	vcmplt_oqss xmm2,xmm6,DWORD PTR [ecx]
	vcmplt_oqss xmm2,xmm6,[ecx]
	vcmple_oqss xmm2,xmm6,xmm4
	vcmple_oqss xmm2,xmm6,DWORD PTR [ecx]
	vcmple_oqss xmm2,xmm6,[ecx]
	vcmpunord_sss xmm2,xmm6,xmm4
	vcmpunord_sss xmm2,xmm6,DWORD PTR [ecx]
	vcmpunord_sss xmm2,xmm6,[ecx]
	vcmpneq_usss xmm2,xmm6,xmm4
	vcmpneq_usss xmm2,xmm6,DWORD PTR [ecx]
	vcmpneq_usss xmm2,xmm6,[ecx]
	vcmpnlt_uqss xmm2,xmm6,xmm4
	vcmpnlt_uqss xmm2,xmm6,DWORD PTR [ecx]
	vcmpnlt_uqss xmm2,xmm6,[ecx]
	vcmpnle_uqss xmm2,xmm6,xmm4
	vcmpnle_uqss xmm2,xmm6,DWORD PTR [ecx]
	vcmpnle_uqss xmm2,xmm6,[ecx]
	vcmpord_sss xmm2,xmm6,xmm4
	vcmpord_sss xmm2,xmm6,DWORD PTR [ecx]
	vcmpord_sss xmm2,xmm6,[ecx]
	vcmpeq_usss xmm2,xmm6,xmm4
	vcmpeq_usss xmm2,xmm6,DWORD PTR [ecx]
	vcmpeq_usss xmm2,xmm6,[ecx]
	vcmpnge_uqss xmm2,xmm6,xmm4
	vcmpnge_uqss xmm2,xmm6,DWORD PTR [ecx]
	vcmpnge_uqss xmm2,xmm6,[ecx]
	vcmpngt_uqss xmm2,xmm6,xmm4
	vcmpngt_uqss xmm2,xmm6,DWORD PTR [ecx]
	vcmpngt_uqss xmm2,xmm6,[ecx]
	vcmpfalse_osss xmm2,xmm6,xmm4
	vcmpfalse_osss xmm2,xmm6,DWORD PTR [ecx]
	vcmpfalse_osss xmm2,xmm6,[ecx]
	vcmpneq_osss xmm2,xmm6,xmm4
	vcmpneq_osss xmm2,xmm6,DWORD PTR [ecx]
	vcmpneq_osss xmm2,xmm6,[ecx]
	vcmpge_oqss xmm2,xmm6,xmm4
	vcmpge_oqss xmm2,xmm6,DWORD PTR [ecx]
	vcmpge_oqss xmm2,xmm6,[ecx]
	vcmpgt_oqss xmm2,xmm6,xmm4
	vcmpgt_oqss xmm2,xmm6,DWORD PTR [ecx]
	vcmpgt_oqss xmm2,xmm6,[ecx]
	vcmptrue_usss xmm2,xmm6,xmm4
	vcmptrue_usss xmm2,xmm6,DWORD PTR [ecx]
	vcmptrue_usss xmm2,xmm6,[ecx]

# Tests for op mem32, ymm
	vbroadcastss ymm4,DWORD PTR [ecx]
	vbroadcastss ymm4,[ecx]

# Tests for op xmm/mem32, xmm
	vcomiss xmm6,xmm4
	vcomiss xmm4,DWORD PTR [ecx]
	vcomiss xmm4,[ecx]
	vpmovsxbd xmm6,xmm4
	vpmovsxbd xmm4,DWORD PTR [ecx]
	vpmovsxbd xmm4,[ecx]
	vpmovsxwq xmm6,xmm4
	vpmovsxwq xmm4,DWORD PTR [ecx]
	vpmovsxwq xmm4,[ecx]
	vpmovzxbd xmm6,xmm4
	vpmovzxbd xmm4,DWORD PTR [ecx]
	vpmovzxbd xmm4,[ecx]
	vpmovzxwq xmm6,xmm4
	vpmovzxwq xmm4,DWORD PTR [ecx]
	vpmovzxwq xmm4,[ecx]
	vucomiss xmm6,xmm4
	vucomiss xmm4,DWORD PTR [ecx]
	vucomiss xmm4,[ecx]

# Tests for op mem32, xmm
	vbroadcastss xmm4,DWORD PTR [ecx]
	vbroadcastss xmm4,[ecx]
	vmovss xmm4,DWORD PTR [ecx]
	vmovss xmm4,[ecx]

# Tests for op xmm, mem32
	vmovss DWORD PTR [ecx],xmm4
	vmovss [ecx],xmm4

# Tests for op xmm, regl/mem32
# Tests for op regl/mem32, xmm
	vmovd ecx,xmm4
	vmovd DWORD PTR [ecx],xmm4
	vmovd xmm4,ecx
	vmovd xmm4,DWORD PTR [ecx]
	vmovd [ecx],xmm4
	vmovd xmm4,[ecx]

# Tests for op xmm/mem32, regl
	vcvtss2si ecx,xmm4
	vcvtss2si ecx,DWORD PTR [ecx]
	vcvtss2si ecx,[ecx]
	vcvttss2si ecx,xmm4
	vcvttss2si ecx,DWORD PTR [ecx]
	vcvttss2si ecx,[ecx]

# Tests for op imm8, xmm, regq/mem32
	vextractps DWORD PTR [ecx],xmm4,7
	vextractps [ecx],xmm4,7

# Tests for op imm8, xmm, regl/mem32
	vpextrd ecx,xmm4,7
	vpextrd DWORD PTR [ecx],xmm4,7
	vpextrd [ecx],xmm4,7
	vextractps ecx,xmm4,7
	vextractps DWORD PTR [ecx],xmm4,7
	vextractps [ecx],xmm4,7

# Tests for op imm8, regl/mem32, xmm, xmm
	vpinsrd xmm6,xmm4,ecx,7
	vpinsrd xmm6,xmm4,DWORD PTR [ecx],7
	vpinsrd xmm6,xmm4,[ecx],7

# Tests for op regl/mem32, xmm, xmm
	vcvtsi2sd xmm6,xmm4,ecx
	vcvtsi2sd xmm6,xmm4,DWORD PTR [ecx]
	vcvtsi2sd xmm6,xmm4,[ecx]
	vcvtsi2ss xmm6,xmm4,ecx
	vcvtsi2ss xmm6,xmm4,DWORD PTR [ecx]
	vcvtsi2ss xmm6,xmm4,[ecx]

# Tests for op imm8, xmm/mem32, xmm, xmm
	vcmpss xmm2,xmm6,xmm4,7
	vcmpss xmm2,xmm6,DWORD PTR [ecx],7
	vcmpss xmm2,xmm6,[ecx],7
	vinsertps xmm2,xmm6,xmm4,7
	vinsertps xmm2,xmm6,DWORD PTR [ecx],7
	vinsertps xmm2,xmm6,[ecx],7
	vroundss xmm2,xmm6,xmm4,7
	vroundss xmm2,xmm6,DWORD PTR [ecx],7
	vroundss xmm2,xmm6,[ecx],7

# Tests for op xmm/m16, xmm
	vpmovsxbq xmm6,xmm4
	vpmovsxbq xmm4,WORD PTR [ecx]
	vpmovsxbq xmm4,[ecx]
	vpmovzxbq xmm6,xmm4
	vpmovzxbq xmm4,WORD PTR [ecx]
	vpmovzxbq xmm4,[ecx]

# Tests for op imm8, xmm, regl/mem16
	vpextrw ecx,xmm4,7
	vpextrw WORD PTR [ecx],xmm4,7
	vpextrw [ecx],xmm4,7

# Tests for op imm8, xmm, regq/mem16
	vpextrw WORD PTR [ecx],xmm4,7
	vpextrw [ecx],xmm4,7

# Tests for op imm8, regl/mem16, xmm, xmm
	vpinsrw xmm6,xmm4,ecx,7
	vpinsrw xmm6,xmm4,WORD PTR [ecx],7
	vpinsrw xmm6,xmm4,[ecx],7

# Tests for op imm8, xmm, regl/mem8
	vpextrb ecx,xmm4,7
	vpextrb BYTE PTR [ecx],xmm4,7
	vpextrb [ecx],xmm4,7

# Tests for op imm8, regl/mem8, xmm, xmm
	vpinsrb xmm6,xmm4,ecx,7
	vpinsrb xmm6,xmm4,BYTE PTR [ecx],7
	vpinsrb xmm6,xmm4,[ecx],7

# Tests for op imm8, xmm, regq/mem8
	vpextrb BYTE PTR [ecx],xmm4,7
	vpextrb [ecx],xmm4,7

# Tests for op xmm, xmm
	vmaskmovdqu xmm6,xmm4
	vmovq xmm6,xmm4

# Tests for op xmm, regl
	vmovmskpd ecx,xmm4
	vmovmskps ecx,xmm4
	vpmovmskb ecx,xmm4

# Tests for op xmm, xmm, xmm
	vmovhlps xmm2,xmm6,xmm4
	vmovlhps xmm2,xmm6,xmm4
	vmovsd xmm2,xmm6,xmm4
	vmovss xmm2,xmm6,xmm4

# Tests for op imm8, xmm, xmm
	vpslld xmm6,xmm4,7
	vpslldq xmm6,xmm4,7
	vpsllq xmm6,xmm4,7
	vpsllw xmm6,xmm4,7
	vpsrad xmm6,xmm4,7
	vpsraw xmm6,xmm4,7
	vpsrld xmm6,xmm4,7
	vpsrldq xmm6,xmm4,7
	vpsrlq xmm6,xmm4,7
	vpsrlw xmm6,xmm4,7

# Tests for op imm8, xmm, regl
	vpextrw ecx,xmm4,7

# Tests for op ymm, regl
	vmovmskpd ecx,ymm4
	vmovmskps ecx,ymm4

# Default instructions without suffixes.
	vcvtpd2dq xmm6,xmm4
	vcvtpd2dq xmm6,ymm4
	vcvtpd2ps xmm6,xmm4
	vcvtpd2ps xmm6,ymm4
	vcvttpd2dq xmm6,xmm4
	vcvttpd2dq xmm6,ymm4

#Tests with different memory and register operands.
	vldmxcsr DWORD PTR ds:0x1234
	vmovdqa xmm0,XMMWORD PTR ds:0x1234
	vmovdqa XMMWORD PTR ds:0x1234,xmm0
	vmovd DWORD PTR ds:0x1234,xmm0
	vcvtsd2si eax,QWORD PTR ds:0x1234
	vcvtdq2pd ymm0,XMMWORD PTR ds:0x1234
	vcvtpd2ps xmm0,YMMWORD PTR ds:0x1234
	vpavgb xmm7,xmm0,XMMWORD PTR ds:0x1234
	vaeskeygenassist xmm0,XMMWORD PTR ds:0x1234,7
	vpextrb ds:0x1234,xmm0,7
	vcvtsi2sd xmm7,xmm0,DWORD PTR ds:0x1234
	vpclmulqdq xmm7,xmm0,XMMWORD PTR ds:0x1234,7
	vblendvps xmm6,xmm4,XMMWORD PTR ds:0x1234,xmm0
	vpinsrb xmm7,xmm0,ds:0x1234,7
	vmovdqa ymm0,YMMWORD PTR ds:0x1234
	vmovdqa YMMWORD PTR ds:0x1234,ymm0
	vpermilpd ymm7,ymm0,YMMWORD PTR ds:0x1234
	vroundpd ymm0,YMMWORD PTR ds:0x1234,7
	vextractf128 XMMWORD PTR ds:0x1234,ymm0,7
	vperm2f128 ymm7,ymm0,YMMWORD PTR ds:0x1234,7
	vblendvpd ymm6,ymm4,YMMWORD PTR ds:0x1234,ymm0
	vldmxcsr DWORD PTR [ebp]
	vmovdqa xmm0,XMMWORD PTR [ebp]
	vmovdqa XMMWORD PTR [ebp],xmm0
	vmovd DWORD PTR [ebp],xmm0
	vcvtsd2si eax,QWORD PTR [ebp]
	vcvtdq2pd ymm0,XMMWORD PTR [ebp]
	vcvtpd2ps xmm0,YMMWORD PTR [ebp]
	vpavgb xmm7,xmm0,XMMWORD PTR [ebp]
	vaeskeygenassist xmm0,XMMWORD PTR [ebp],7
	vpextrb [ebp],xmm0,7
	vcvtsi2sd xmm7,xmm0,DWORD PTR [ebp]
	vpclmulqdq xmm7,xmm0,XMMWORD PTR [ebp],7
	vblendvps xmm6,xmm4,XMMWORD PTR [ebp],xmm0
	vpinsrb xmm7,xmm0,[ebp],7
	vmovdqa ymm0,YMMWORD PTR [ebp]
	vmovdqa YMMWORD PTR [ebp],ymm0
	vpermilpd ymm7,ymm0,YMMWORD PTR [ebp]
	vroundpd ymm0,YMMWORD PTR [ebp],7
	vextractf128 XMMWORD PTR [ebp],ymm0,7
	vperm2f128 ymm7,ymm0,YMMWORD PTR [ebp],7
	vblendvpd ymm6,ymm4,YMMWORD PTR [ebp],ymm0
	vldmxcsr DWORD PTR [ebp+0x99]
	vmovdqa xmm0,XMMWORD PTR [ebp+0x99]
	vmovdqa XMMWORD PTR [ebp+0x99],xmm0
	vmovd DWORD PTR [ebp+0x99],xmm0
	vcvtsd2si eax,QWORD PTR [ebp+0x99]
	vcvtdq2pd ymm0,XMMWORD PTR [ebp+0x99]
	vcvtpd2ps xmm0,YMMWORD PTR [ebp+0x99]
	vpavgb xmm7,xmm0,XMMWORD PTR [ebp+0x99]
	vaeskeygenassist xmm0,XMMWORD PTR [ebp+0x99],7
	vpextrb [ebp+0x99],xmm0,7
	vcvtsi2sd xmm7,xmm0,DWORD PTR [ebp+0x99]
	vpclmulqdq xmm7,xmm0,XMMWORD PTR [ebp+0x99],7
	vblendvps xmm6,xmm4,XMMWORD PTR [ebp+0x99],xmm0
	vpinsrb xmm7,xmm0,[ebp+0x99],7
	vmovdqa ymm0,YMMWORD PTR [ebp+0x99]
	vmovdqa YMMWORD PTR [ebp+0x99],ymm0
	vpermilpd ymm7,ymm0,YMMWORD PTR [ebp+0x99]
	vroundpd ymm0,YMMWORD PTR [ebp+0x99],7
	vextractf128 XMMWORD PTR [ebp+0x99],ymm0,7
	vperm2f128 ymm7,ymm0,YMMWORD PTR [ebp+0x99],7
	vblendvpd ymm6,ymm4,YMMWORD PTR [ebp+0x99],ymm0
	vldmxcsr DWORD PTR [eiz*1+0x99]
	vmovdqa xmm0,XMMWORD PTR [eiz*1+0x99]
	vmovdqa XMMWORD PTR [eiz*1+0x99],xmm0
	vmovd DWORD PTR [eiz*1+0x99],xmm0
	vcvtsd2si eax,QWORD PTR [eiz*1+0x99]
	vcvtdq2pd ymm0,XMMWORD PTR [eiz*1+0x99]
	vcvtpd2ps xmm0,YMMWORD PTR [eiz*1+0x99]
	vpavgb xmm7,xmm0,XMMWORD PTR [eiz*1+0x99]
	vaeskeygenassist xmm0,XMMWORD PTR [eiz*1+0x99],7
	vpextrb [eiz*1+0x99],xmm0,7
	vcvtsi2sd xmm7,xmm0,DWORD PTR [eiz*1+0x99]
	vpclmulqdq xmm7,xmm0,XMMWORD PTR [eiz*1+0x99],7
	vblendvps xmm6,xmm4,XMMWORD PTR [eiz*1+0x99],xmm0
	vpinsrb xmm7,xmm0,[eiz*1+0x99],7
	vmovdqa ymm0,YMMWORD PTR [eiz*1+0x99]
	vmovdqa YMMWORD PTR [eiz*1+0x99],ymm0
	vpermilpd ymm7,ymm0,YMMWORD PTR [eiz*1+0x99]
	vroundpd ymm0,YMMWORD PTR [eiz*1+0x99],7
	vextractf128 XMMWORD PTR [eiz*1+0x99],ymm0,7
	vperm2f128 ymm7,ymm0,YMMWORD PTR [eiz*1+0x99],7
	vblendvpd ymm6,ymm4,YMMWORD PTR [eiz*1+0x99],ymm0
	vldmxcsr DWORD PTR [eiz*2+0x99]
	vmovdqa xmm0,XMMWORD PTR [eiz*2+0x99]
	vmovdqa XMMWORD PTR [eiz*2+0x99],xmm0
	vmovd DWORD PTR [eiz*2+0x99],xmm0
	vcvtsd2si eax,QWORD PTR [eiz*2+0x99]
	vcvtdq2pd ymm0,XMMWORD PTR [eiz*2+0x99]
	vcvtpd2ps xmm0,YMMWORD PTR [eiz*2+0x99]
	vpavgb xmm7,xmm0,XMMWORD PTR [eiz*2+0x99]
	vaeskeygenassist xmm0,XMMWORD PTR [eiz*2+0x99],7
	vpextrb [eiz*2+0x99],xmm0,7
	vcvtsi2sd xmm7,xmm0,DWORD PTR [eiz*2+0x99]
	vpclmulqdq xmm7,xmm0,XMMWORD PTR [eiz*2+0x99],7
	vblendvps xmm6,xmm4,XMMWORD PTR [eiz*2+0x99],xmm0
	vpinsrb xmm7,xmm0,[eiz*2+0x99],7
	vmovdqa ymm0,YMMWORD PTR [eiz*2+0x99]
	vmovdqa YMMWORD PTR [eiz*2+0x99],ymm0
	vpermilpd ymm7,ymm0,YMMWORD PTR [eiz*2+0x99]
	vroundpd ymm0,YMMWORD PTR [eiz*2+0x99],7
	vextractf128 XMMWORD PTR [eiz*2+0x99],ymm0,7
	vperm2f128 ymm7,ymm0,YMMWORD PTR [eiz*2+0x99],7
	vblendvpd ymm6,ymm4,YMMWORD PTR [eiz*2+0x99],ymm0
	vldmxcsr DWORD PTR [eax+eiz*1+0x99]
	vmovdqa xmm0,XMMWORD PTR [eax+eiz*1+0x99]
	vmovdqa XMMWORD PTR [eax+eiz*1+0x99],xmm0
	vmovd DWORD PTR [eax+eiz*1+0x99],xmm0
	vcvtsd2si eax,QWORD PTR [eax+eiz*1+0x99]
	vcvtdq2pd ymm0,XMMWORD PTR [eax+eiz*1+0x99]
	vcvtpd2ps xmm0,YMMWORD PTR [eax+eiz*1+0x99]
	vpavgb xmm7,xmm0,XMMWORD PTR [eax+eiz*1+0x99]
	vaeskeygenassist xmm0,XMMWORD PTR [eax+eiz*1+0x99],7
	vpextrb [eax+eiz*1+0x99],xmm0,7
	vcvtsi2sd xmm7,xmm0,DWORD PTR [eax+eiz*1+0x99]
	vpclmulqdq xmm7,xmm0,XMMWORD PTR [eax+eiz*1+0x99],7
	vblendvps xmm6,xmm4,XMMWORD PTR [eax+eiz*1+0x99],xmm0
	vpinsrb xmm7,xmm0,[eax+eiz*1+0x99],7
	vmovdqa ymm0,YMMWORD PTR [eax+eiz*1+0x99]
	vmovdqa YMMWORD PTR [eax+eiz*1+0x99],ymm0
	vpermilpd ymm7,ymm0,YMMWORD PTR [eax+eiz*1+0x99]
	vroundpd ymm0,YMMWORD PTR [eax+eiz*1+0x99],7
	vextractf128 XMMWORD PTR [eax+eiz*1+0x99],ymm0,7
	vperm2f128 ymm7,ymm0,YMMWORD PTR [eax+eiz*1+0x99],7
	vblendvpd ymm6,ymm4,YMMWORD PTR [eax+eiz*1+0x99],ymm0
	vldmxcsr DWORD PTR [eax+eiz*2+0x99]
	vmovdqa xmm0,XMMWORD PTR [eax+eiz*2+0x99]
	vmovdqa XMMWORD PTR [eax+eiz*2+0x99],xmm0
	vmovd DWORD PTR [eax+eiz*2+0x99],xmm0
	vcvtsd2si eax,QWORD PTR [eax+eiz*2+0x99]
	vcvtdq2pd ymm0,XMMWORD PTR [eax+eiz*2+0x99]
	vcvtpd2ps xmm0,YMMWORD PTR [eax+eiz*2+0x99]
	vpavgb xmm7,xmm0,XMMWORD PTR [eax+eiz*2+0x99]
	vaeskeygenassist xmm0,XMMWORD PTR [eax+eiz*2+0x99],7
	vpextrb [eax+eiz*2+0x99],xmm0,7
	vcvtsi2sd xmm7,xmm0,DWORD PTR [eax+eiz*2+0x99]
	vpclmulqdq xmm7,xmm0,XMMWORD PTR [eax+eiz*2+0x99],7
	vblendvps xmm6,xmm4,XMMWORD PTR [eax+eiz*2+0x99],xmm0
	vpinsrb xmm7,xmm0,[eax+eiz*2+0x99],7
	vmovdqa ymm0,YMMWORD PTR [eax+eiz*2+0x99]
	vmovdqa YMMWORD PTR [eax+eiz*2+0x99],ymm0
	vpermilpd ymm7,ymm0,YMMWORD PTR [eax+eiz*2+0x99]
	vroundpd ymm0,YMMWORD PTR [eax+eiz*2+0x99],7
	vextractf128 XMMWORD PTR [eax+eiz*2+0x99],ymm0,7
	vperm2f128 ymm7,ymm0,YMMWORD PTR [eax+eiz*2+0x99],7
	vblendvpd ymm6,ymm4,YMMWORD PTR [eax+eiz*2+0x99],ymm0
	vldmxcsr DWORD PTR [eax+ebx*4+0x99]
	vmovdqa xmm0,XMMWORD PTR [eax+ebx*4+0x99]
	vmovdqa XMMWORD PTR [eax+ebx*4+0x99],xmm0
	vmovd DWORD PTR [eax+ebx*4+0x99],xmm0
	vcvtsd2si eax,QWORD PTR [eax+ebx*4+0x99]
	vcvtdq2pd ymm0,XMMWORD PTR [eax+ebx*4+0x99]
	vcvtpd2ps xmm0,YMMWORD PTR [eax+ebx*4+0x99]
	vpavgb xmm7,xmm0,XMMWORD PTR [eax+ebx*4+0x99]
	vaeskeygenassist xmm0,XMMWORD PTR [eax+ebx*4+0x99],7
	vpextrb [eax+ebx*4+0x99],xmm0,7
	vcvtsi2sd xmm7,xmm0,DWORD PTR [eax+ebx*4+0x99]
	vpclmulqdq xmm7,xmm0,XMMWORD PTR [eax+ebx*4+0x99],7
	vblendvps xmm6,xmm4,XMMWORD PTR [eax+ebx*4+0x99],xmm0
	vpinsrb xmm7,xmm0,[eax+ebx*4+0x99],7
	vmovdqa ymm0,YMMWORD PTR [eax+ebx*4+0x99]
	vmovdqa YMMWORD PTR [eax+ebx*4+0x99],ymm0
	vpermilpd ymm7,ymm0,YMMWORD PTR [eax+ebx*4+0x99]
	vroundpd ymm0,YMMWORD PTR [eax+ebx*4+0x99],7
	vextractf128 XMMWORD PTR [eax+ebx*4+0x99],ymm0,7
	vperm2f128 ymm7,ymm0,YMMWORD PTR [eax+ebx*4+0x99],7
	vblendvpd ymm6,ymm4,YMMWORD PTR [eax+ebx*4+0x99],ymm0
	vldmxcsr DWORD PTR [esp+ecx*8+0x99]
	vmovdqa xmm0,XMMWORD PTR [esp+ecx*8+0x99]
	vmovdqa XMMWORD PTR [esp+ecx*8+0x99],xmm0
	vmovd DWORD PTR [esp+ecx*8+0x99],xmm0
	vcvtsd2si eax,QWORD PTR [esp+ecx*8+0x99]
	vcvtdq2pd ymm0,XMMWORD PTR [esp+ecx*8+0x99]
	vcvtpd2ps xmm0,YMMWORD PTR [esp+ecx*8+0x99]
	vpavgb xmm7,xmm0,XMMWORD PTR [esp+ecx*8+0x99]
	vaeskeygenassist xmm0,XMMWORD PTR [esp+ecx*8+0x99],7
	vpextrb [esp+ecx*8+0x99],xmm0,7
	vcvtsi2sd xmm7,xmm0,DWORD PTR [esp+ecx*8+0x99]
	vpclmulqdq xmm7,xmm0,XMMWORD PTR [esp+ecx*8+0x99],7
	vblendvps xmm6,xmm4,XMMWORD PTR [esp+ecx*8+0x99],xmm0
	vpinsrb xmm7,xmm0,[esp+ecx*8+0x99],7
	vmovdqa ymm0,YMMWORD PTR [esp+ecx*8+0x99]
	vmovdqa YMMWORD PTR [esp+ecx*8+0x99],ymm0
	vpermilpd ymm7,ymm0,YMMWORD PTR [esp+ecx*8+0x99]
	vroundpd ymm0,YMMWORD PTR [esp+ecx*8+0x99],7
	vextractf128 XMMWORD PTR [esp+ecx*8+0x99],ymm0,7
	vperm2f128 ymm7,ymm0,YMMWORD PTR [esp+ecx*8+0x99],7
	vblendvpd ymm6,ymm4,YMMWORD PTR [esp+ecx*8+0x99],ymm0
	vldmxcsr DWORD PTR [ebp+edx*1+0x99]
	vmovdqa xmm0,XMMWORD PTR [ebp+edx*1+0x99]
	vmovdqa XMMWORD PTR [ebp+edx*1+0x99],xmm0
	vmovd DWORD PTR [ebp+edx*1+0x99],xmm0
	vcvtsd2si eax,QWORD PTR [ebp+edx*1+0x99]
	vcvtdq2pd ymm0,XMMWORD PTR [ebp+edx*1+0x99]
	vcvtpd2ps xmm0,YMMWORD PTR [ebp+edx*1+0x99]
	vpavgb xmm7,xmm0,XMMWORD PTR [ebp+edx*1+0x99]
	vaeskeygenassist xmm0,XMMWORD PTR [ebp+edx*1+0x99],7
	vpextrb [ebp+edx*1+0x99],xmm0,7
	vcvtsi2sd xmm7,xmm0,DWORD PTR [ebp+edx*1+0x99]
	vpclmulqdq xmm7,xmm0,XMMWORD PTR [ebp+edx*1+0x99],7
	vblendvps xmm6,xmm4,XMMWORD PTR [ebp+edx*1+0x99],xmm0
	vpinsrb xmm7,xmm0,[ebp+edx*1+0x99],7
	vmovdqa ymm0,YMMWORD PTR [ebp+edx*1+0x99]
	vmovdqa YMMWORD PTR [ebp+edx*1+0x99],ymm0
	vpermilpd ymm7,ymm0,YMMWORD PTR [ebp+edx*1+0x99]
	vroundpd ymm0,YMMWORD PTR [ebp+edx*1+0x99],7
	vextractf128 XMMWORD PTR [ebp+edx*1+0x99],ymm0,7
	vperm2f128 ymm7,ymm0,YMMWORD PTR [ebp+edx*1+0x99],7
	vblendvpd ymm6,ymm4,YMMWORD PTR [ebp+edx*1+0x99],ymm0
# Tests for all register operands.
	vmovmskpd eax,xmm0
	vpslld xmm7,xmm0,7
	vmovmskps eax,ymm0
