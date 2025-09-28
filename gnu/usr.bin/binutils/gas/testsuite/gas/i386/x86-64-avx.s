# Check 64bit AVX instructions

	.allow_index_reg
	.text
_start:
# Tests for op
	vzeroall
	vzeroupper

# Tests for op mem64
	vldmxcsr (%rcx)
	vstmxcsr (%rcx)

# Tests for op mem256, mask,  ymm
# Tests for op ymm, mask, mem256
	vmaskmovpd (%rcx),%ymm4,%ymm6
	vmaskmovpd %ymm4,%ymm6,(%rcx)
	vmaskmovps (%rcx),%ymm4,%ymm6
	vmaskmovps %ymm4,%ymm6,(%rcx)

# Tests for op imm8, ymm/mem256, ymm
	vpermilpd $7,%ymm6,%ymm2
	vpermilpd $7,(%rcx),%ymm6
	vpermilps $7,%ymm6,%ymm2
	vpermilps $7,(%rcx),%ymm6
	vroundpd $7,%ymm6,%ymm2
	vroundpd $7,(%rcx),%ymm6
	vroundps $7,%ymm6,%ymm2
	vroundps $7,(%rcx),%ymm6

# Tests for op ymm/mem256, ymm, ymm
	vaddpd %ymm4,%ymm6,%ymm2
	vaddpd (%rcx),%ymm6,%ymm2
	vaddps %ymm4,%ymm6,%ymm2
	vaddps (%rcx),%ymm6,%ymm2
	vaddsubpd %ymm4,%ymm6,%ymm2
	vaddsubpd (%rcx),%ymm6,%ymm2
	vaddsubps %ymm4,%ymm6,%ymm2
	vaddsubps (%rcx),%ymm6,%ymm2
	vandnpd %ymm4,%ymm6,%ymm2
	vandnpd (%rcx),%ymm6,%ymm2
	vandnps %ymm4,%ymm6,%ymm2
	vandnps (%rcx),%ymm6,%ymm2
	vandpd %ymm4,%ymm6,%ymm2
	vandpd (%rcx),%ymm6,%ymm2
	vandps %ymm4,%ymm6,%ymm2
	vandps (%rcx),%ymm6,%ymm2
	vdivpd %ymm4,%ymm6,%ymm2
	vdivpd (%rcx),%ymm6,%ymm2
	vdivps %ymm4,%ymm6,%ymm2
	vdivps (%rcx),%ymm6,%ymm2
	vhaddpd %ymm4,%ymm6,%ymm2
	vhaddpd (%rcx),%ymm6,%ymm2
	vhaddps %ymm4,%ymm6,%ymm2
	vhaddps (%rcx),%ymm6,%ymm2
	vhsubpd %ymm4,%ymm6,%ymm2
	vhsubpd (%rcx),%ymm6,%ymm2
	vhsubps %ymm4,%ymm6,%ymm2
	vhsubps (%rcx),%ymm6,%ymm2
	vmaxpd %ymm4,%ymm6,%ymm2
	vmaxpd (%rcx),%ymm6,%ymm2
	vmaxps %ymm4,%ymm6,%ymm2
	vmaxps (%rcx),%ymm6,%ymm2
	vminpd %ymm4,%ymm6,%ymm2
	vminpd (%rcx),%ymm6,%ymm2
	vminps %ymm4,%ymm6,%ymm2
	vminps (%rcx),%ymm6,%ymm2
	vmulpd %ymm4,%ymm6,%ymm2
	vmulpd (%rcx),%ymm6,%ymm2
	vmulps %ymm4,%ymm6,%ymm2
	vmulps (%rcx),%ymm6,%ymm2
	vorpd %ymm4,%ymm6,%ymm2
	vorpd (%rcx),%ymm6,%ymm2
	vorps %ymm4,%ymm6,%ymm2
	vorps (%rcx),%ymm6,%ymm2
	vpermilpd %ymm4,%ymm6,%ymm2
	vpermilpd (%rcx),%ymm6,%ymm2
	vpermilps %ymm4,%ymm6,%ymm2
	vpermilps (%rcx),%ymm6,%ymm2
	vsubpd %ymm4,%ymm6,%ymm2
	vsubpd (%rcx),%ymm6,%ymm2
	vsubps %ymm4,%ymm6,%ymm2
	vsubps (%rcx),%ymm6,%ymm2
	vunpckhpd %ymm4,%ymm6,%ymm2
	vunpckhpd (%rcx),%ymm6,%ymm2
	vunpckhps %ymm4,%ymm6,%ymm2
	vunpckhps (%rcx),%ymm6,%ymm2
	vunpcklpd %ymm4,%ymm6,%ymm2
	vunpcklpd (%rcx),%ymm6,%ymm2
	vunpcklps %ymm4,%ymm6,%ymm2
	vunpcklps (%rcx),%ymm6,%ymm2
	vxorpd %ymm4,%ymm6,%ymm2
	vxorpd (%rcx),%ymm6,%ymm2
	vxorps %ymm4,%ymm6,%ymm2
	vxorps (%rcx),%ymm6,%ymm2
	vcmpeqpd %ymm4,%ymm6,%ymm2
	vcmpeqpd (%rcx),%ymm6,%ymm2
	vcmpltpd %ymm4,%ymm6,%ymm2
	vcmpltpd (%rcx),%ymm6,%ymm2
	vcmplepd %ymm4,%ymm6,%ymm2
	vcmplepd (%rcx),%ymm6,%ymm2
	vcmpunordpd %ymm4,%ymm6,%ymm2
	vcmpunordpd (%rcx),%ymm6,%ymm2
	vcmpneqpd %ymm4,%ymm6,%ymm2
	vcmpneqpd (%rcx),%ymm6,%ymm2
	vcmpnltpd %ymm4,%ymm6,%ymm2
	vcmpnltpd (%rcx),%ymm6,%ymm2
	vcmpnlepd %ymm4,%ymm6,%ymm2
	vcmpnlepd (%rcx),%ymm6,%ymm2
	vcmpordpd %ymm4,%ymm6,%ymm2
	vcmpordpd (%rcx),%ymm6,%ymm2
	vcmpeq_uqpd %ymm4,%ymm6,%ymm2
	vcmpeq_uqpd (%rcx),%ymm6,%ymm2
	vcmpngepd %ymm4,%ymm6,%ymm2
	vcmpngepd (%rcx),%ymm6,%ymm2
	vcmpngtpd %ymm4,%ymm6,%ymm2
	vcmpngtpd (%rcx),%ymm6,%ymm2
	vcmpfalsepd %ymm4,%ymm6,%ymm2
	vcmpfalsepd (%rcx),%ymm6,%ymm2
	vcmpneq_oqpd %ymm4,%ymm6,%ymm2
	vcmpneq_oqpd (%rcx),%ymm6,%ymm2
	vcmpgepd %ymm4,%ymm6,%ymm2
	vcmpgepd (%rcx),%ymm6,%ymm2
	vcmpgtpd %ymm4,%ymm6,%ymm2
	vcmpgtpd (%rcx),%ymm6,%ymm2
	vcmptruepd %ymm4,%ymm6,%ymm2
	vcmptruepd (%rcx),%ymm6,%ymm2
	vcmpeq_ospd %ymm4,%ymm6,%ymm2
	vcmpeq_ospd (%rcx),%ymm6,%ymm2
	vcmplt_oqpd %ymm4,%ymm6,%ymm2
	vcmplt_oqpd (%rcx),%ymm6,%ymm2
	vcmple_oqpd %ymm4,%ymm6,%ymm2
	vcmple_oqpd (%rcx),%ymm6,%ymm2
	vcmpunord_spd %ymm4,%ymm6,%ymm2
	vcmpunord_spd (%rcx),%ymm6,%ymm2
	vcmpneq_uspd %ymm4,%ymm6,%ymm2
	vcmpneq_uspd (%rcx),%ymm6,%ymm2
	vcmpnlt_uqpd %ymm4,%ymm6,%ymm2
	vcmpnlt_uqpd (%rcx),%ymm6,%ymm2
	vcmpnle_uqpd %ymm4,%ymm6,%ymm2
	vcmpnle_uqpd (%rcx),%ymm6,%ymm2
	vcmpord_spd %ymm4,%ymm6,%ymm2
	vcmpord_spd (%rcx),%ymm6,%ymm2
	vcmpeq_uspd %ymm4,%ymm6,%ymm2
	vcmpeq_uspd (%rcx),%ymm6,%ymm2
	vcmpnge_uqpd %ymm4,%ymm6,%ymm2
	vcmpnge_uqpd (%rcx),%ymm6,%ymm2
	vcmpngt_uqpd %ymm4,%ymm6,%ymm2
	vcmpngt_uqpd (%rcx),%ymm6,%ymm2
	vcmpfalse_ospd %ymm4,%ymm6,%ymm2
	vcmpfalse_ospd (%rcx),%ymm6,%ymm2
	vcmpneq_ospd %ymm4,%ymm6,%ymm2
	vcmpneq_ospd (%rcx),%ymm6,%ymm2
	vcmpge_oqpd %ymm4,%ymm6,%ymm2
	vcmpge_oqpd (%rcx),%ymm6,%ymm2
	vcmpgt_oqpd %ymm4,%ymm6,%ymm2
	vcmpgt_oqpd (%rcx),%ymm6,%ymm2
	vcmptrue_uspd %ymm4,%ymm6,%ymm2
	vcmptrue_uspd (%rcx),%ymm6,%ymm2
	vcmpeqps %ymm4,%ymm6,%ymm2
	vcmpeqps (%rcx),%ymm6,%ymm2
	vcmpltps %ymm4,%ymm6,%ymm2
	vcmpltps (%rcx),%ymm6,%ymm2
	vcmpleps %ymm4,%ymm6,%ymm2
	vcmpleps (%rcx),%ymm6,%ymm2
	vcmpunordps %ymm4,%ymm6,%ymm2
	vcmpunordps (%rcx),%ymm6,%ymm2
	vcmpneqps %ymm4,%ymm6,%ymm2
	vcmpneqps (%rcx),%ymm6,%ymm2
	vcmpnltps %ymm4,%ymm6,%ymm2
	vcmpnltps (%rcx),%ymm6,%ymm2
	vcmpnleps %ymm4,%ymm6,%ymm2
	vcmpnleps (%rcx),%ymm6,%ymm2
	vcmpordps %ymm4,%ymm6,%ymm2
	vcmpordps (%rcx),%ymm6,%ymm2
	vcmpeq_uqps %ymm4,%ymm6,%ymm2
	vcmpeq_uqps (%rcx),%ymm6,%ymm2
	vcmpngeps %ymm4,%ymm6,%ymm2
	vcmpngeps (%rcx),%ymm6,%ymm2
	vcmpngtps %ymm4,%ymm6,%ymm2
	vcmpngtps (%rcx),%ymm6,%ymm2
	vcmpfalseps %ymm4,%ymm6,%ymm2
	vcmpfalseps (%rcx),%ymm6,%ymm2
	vcmpneq_oqps %ymm4,%ymm6,%ymm2
	vcmpneq_oqps (%rcx),%ymm6,%ymm2
	vcmpgeps %ymm4,%ymm6,%ymm2
	vcmpgeps (%rcx),%ymm6,%ymm2
	vcmpgtps %ymm4,%ymm6,%ymm2
	vcmpgtps (%rcx),%ymm6,%ymm2
	vcmptrueps %ymm4,%ymm6,%ymm2
	vcmptrueps (%rcx),%ymm6,%ymm2
	vcmpeq_osps %ymm4,%ymm6,%ymm2
	vcmpeq_osps (%rcx),%ymm6,%ymm2
	vcmplt_oqps %ymm4,%ymm6,%ymm2
	vcmplt_oqps (%rcx),%ymm6,%ymm2
	vcmple_oqps %ymm4,%ymm6,%ymm2
	vcmple_oqps (%rcx),%ymm6,%ymm2
	vcmpunord_sps %ymm4,%ymm6,%ymm2
	vcmpunord_sps (%rcx),%ymm6,%ymm2
	vcmpneq_usps %ymm4,%ymm6,%ymm2
	vcmpneq_usps (%rcx),%ymm6,%ymm2
	vcmpnlt_uqps %ymm4,%ymm6,%ymm2
	vcmpnlt_uqps (%rcx),%ymm6,%ymm2
	vcmpnle_uqps %ymm4,%ymm6,%ymm2
	vcmpnle_uqps (%rcx),%ymm6,%ymm2
	vcmpord_sps %ymm4,%ymm6,%ymm2
	vcmpord_sps (%rcx),%ymm6,%ymm2
	vcmpeq_usps %ymm4,%ymm6,%ymm2
	vcmpeq_usps (%rcx),%ymm6,%ymm2
	vcmpnge_uqps %ymm4,%ymm6,%ymm2
	vcmpnge_uqps (%rcx),%ymm6,%ymm2
	vcmpngt_uqps %ymm4,%ymm6,%ymm2
	vcmpngt_uqps (%rcx),%ymm6,%ymm2
	vcmpfalse_osps %ymm4,%ymm6,%ymm2
	vcmpfalse_osps (%rcx),%ymm6,%ymm2
	vcmpneq_osps %ymm4,%ymm6,%ymm2
	vcmpneq_osps (%rcx),%ymm6,%ymm2
	vcmpge_oqps %ymm4,%ymm6,%ymm2
	vcmpge_oqps (%rcx),%ymm6,%ymm2
	vcmpgt_oqps %ymm4,%ymm6,%ymm2
	vcmpgt_oqps (%rcx),%ymm6,%ymm2
	vcmptrue_usps %ymm4,%ymm6,%ymm2
	vcmptrue_usps (%rcx),%ymm6,%ymm2
    vgf2p8mulb %ymm4, %ymm5, %ymm6
	vgf2p8mulb (%rcx), %ymm5, %ymm6
	vgf2p8mulb -123456(%rax,%r14,8), %ymm5, %ymm6
	vgf2p8mulb 4064(%rdx), %ymm5, %ymm6
	vgf2p8mulb 4096(%rdx), %ymm5, %ymm6
	vgf2p8mulb -4096(%rdx), %ymm5, %ymm6
	vgf2p8mulb -4128(%rdx), %ymm5, %ymm6

# Tests for op ymm/mem256, xmm
	vcvtpd2dqy %ymm4,%xmm4
	vcvtpd2dqy (%rcx),%xmm4
	vcvtpd2psy %ymm4,%xmm4
	vcvtpd2psy (%rcx),%xmm4
	vcvttpd2dqy %ymm4,%xmm4
	vcvttpd2dqy (%rcx),%xmm4

# Tests for op ymm/mem256, ymm
	vcvtdq2ps %ymm4,%ymm6
	vcvtdq2ps (%rcx),%ymm4
	vcvtps2dq %ymm4,%ymm6
	vcvtps2dq (%rcx),%ymm4
	vcvttps2dq %ymm4,%ymm6
	vcvttps2dq (%rcx),%ymm4
	vmovapd %ymm4,%ymm6
	vmovapd (%rcx),%ymm4
	vmovaps %ymm4,%ymm6
	vmovaps (%rcx),%ymm4
	vmovdqa %ymm4,%ymm6
	vmovdqa (%rcx),%ymm4
	vmovdqu %ymm4,%ymm6
	vmovdqu (%rcx),%ymm4
	vmovddup %ymm4,%ymm6
	vmovddup (%rcx),%ymm4
	vmovshdup %ymm4,%ymm6
	vmovshdup (%rcx),%ymm4
	vmovsldup %ymm4,%ymm6
	vmovsldup (%rcx),%ymm4
	vmovupd %ymm4,%ymm6
	vmovupd (%rcx),%ymm4
	vmovups %ymm4,%ymm6
	vmovups (%rcx),%ymm4
	vptest %ymm4,%ymm6
	vptest (%rcx),%ymm4
	vrcpps %ymm4,%ymm6
	vrcpps (%rcx),%ymm4
	vrsqrtps %ymm4,%ymm6
	vrsqrtps (%rcx),%ymm4
	vsqrtpd %ymm4,%ymm6
	vsqrtpd (%rcx),%ymm4
	vsqrtps %ymm4,%ymm6
	vsqrtps (%rcx),%ymm4
	vtestpd %ymm4,%ymm6
	vtestpd (%rcx),%ymm4
	vtestps %ymm4,%ymm6
	vtestps (%rcx),%ymm4

# Tests for op ymm, ymm/mem256
	vmovapd %ymm4,%ymm6
	vmovapd %ymm4,(%rcx)
	vmovaps %ymm4,%ymm6
	vmovaps %ymm4,(%rcx)
	vmovdqa %ymm4,%ymm6
	vmovdqa %ymm4,(%rcx)
	vmovdqu %ymm4,%ymm6
	vmovdqu %ymm4,(%rcx)
	vmovupd %ymm4,%ymm6
	vmovupd %ymm4,(%rcx)
	vmovups %ymm4,%ymm6
	vmovups %ymm4,(%rcx)

# Tests for op mem256, ymm
	vlddqu (%rcx),%ymm4

# Tests for op ymm, mem256
	vmovntdq %ymm4,(%rcx)
	vmovntpd %ymm4,(%rcx)
	vmovntps %ymm4,(%rcx)

# Tests for op imm8, ymm/mem256, ymm, ymm
	vblendpd $7,%ymm4,%ymm6,%ymm2
	vblendpd $7,(%rcx),%ymm6,%ymm2
	vblendps $7,%ymm4,%ymm6,%ymm2
	vblendps $7,(%rcx),%ymm6,%ymm2
	vcmppd $7,%ymm4,%ymm6,%ymm2
	vcmppd $7,(%rcx),%ymm6,%ymm2
	vcmpps $7,%ymm4,%ymm6,%ymm2
	vcmpps $7,(%rcx),%ymm6,%ymm2
	vdpps $7,%ymm4,%ymm6,%ymm2
	vdpps $7,(%rcx),%ymm6,%ymm2
	vperm2f128 $7,%ymm4,%ymm6,%ymm2
	vperm2f128 $7,(%rcx),%ymm6,%ymm2
	vshufpd $7,%ymm4,%ymm6,%ymm2
	vshufpd $7,(%rcx),%ymm6,%ymm2
	vshufps $7,%ymm4,%ymm6,%ymm2
	vshufps $7,(%rcx),%ymm6,%ymm2
    vgf2p8affineqb $0xab, %ymm4, %ymm5, %ymm6
	vgf2p8affineqb $123, %ymm4, %ymm5, %ymm6
	vgf2p8affineqb $123, (%rcx), %ymm5, %ymm6
	vgf2p8affineqb $123, -123456(%rax,%r14,8), %ymm5, %ymm6
	vgf2p8affineqb $123, 4064(%rdx), %ymm5, %ymm6
	vgf2p8affineqb $123, 4096(%rdx), %ymm5, %ymm6
	vgf2p8affineqb $123, -4096(%rdx), %ymm5, %ymm6
	vgf2p8affineqb $123, -4128(%rdx), %ymm5, %ymm6
	vgf2p8affineinvqb $0xab, %ymm4, %ymm5, %ymm6
	vgf2p8affineinvqb $123, %ymm4, %ymm5, %ymm6
	vgf2p8affineinvqb $123, (%rcx), %ymm5, %ymm6
	vgf2p8affineinvqb $123, -123456(%rax,%r14,8), %ymm5, %ymm6
	vgf2p8affineinvqb $123, 4064(%rdx), %ymm5, %ymm6
	vgf2p8affineinvqb $123, 4096(%rdx), %ymm5, %ymm6
	vgf2p8affineinvqb $123, -4096(%rdx), %ymm5, %ymm6
	vgf2p8affineinvqb $123, -4128(%rdx), %ymm5, %ymm6

# Tests for op ymm, ymm/mem256, ymm, ymm
	vblendvpd %ymm4,%ymm6,%ymm2,%ymm7
	vblendvpd %ymm4,(%rcx),%ymm2,%ymm7
	vblendvps %ymm4,%ymm6,%ymm2,%ymm7
	vblendvps %ymm4,(%rcx),%ymm2,%ymm7

# Tests for op imm8, xmm/mem128, ymm, ymm
	vinsertf128 $7,%xmm4,%ymm4,%ymm6
	vinsertf128 $7,(%rcx),%ymm4,%ymm6

# Tests for op imm8, ymm, xmm/mem128
	vextractf128 $7,%ymm4,%xmm4
	vextractf128 $7,%ymm4,(%rcx)

# Tests for op mem128, ymm
	vbroadcastf128 (%rcx),%ymm4

# Tests for op xmm/mem128, xmm
	vcvtdq2ps %xmm4,%xmm6
	vcvtdq2ps (%rcx),%xmm4
	vcvtpd2dqx %xmm4,%xmm6
	vcvtpd2dqx (%rcx),%xmm4
	vcvtpd2psx %xmm4,%xmm6
	vcvtpd2psx (%rcx),%xmm4
	vcvtps2dq %xmm4,%xmm6
	vcvtps2dq (%rcx),%xmm4
	vcvttpd2dqx %xmm4,%xmm6
	vcvttpd2dqx (%rcx),%xmm4
	vcvttps2dq %xmm4,%xmm6
	vcvttps2dq (%rcx),%xmm4
	vmovapd %xmm4,%xmm6
	vmovapd (%rcx),%xmm4
	vmovaps %xmm4,%xmm6
	vmovaps (%rcx),%xmm4
	vmovdqa %xmm4,%xmm6
	vmovdqa (%rcx),%xmm4
	vmovdqu %xmm4,%xmm6
	vmovdqu (%rcx),%xmm4
	vmovshdup %xmm4,%xmm6
	vmovshdup (%rcx),%xmm4
	vmovsldup %xmm4,%xmm6
	vmovsldup (%rcx),%xmm4
	vmovupd %xmm4,%xmm6
	vmovupd (%rcx),%xmm4
	vmovups %xmm4,%xmm6
	vmovups (%rcx),%xmm4
	vpabsb %xmm4,%xmm6
	vpabsb (%rcx),%xmm4
	vpabsw %xmm4,%xmm6
	vpabsw (%rcx),%xmm4
	vpabsd %xmm4,%xmm6
	vpabsd (%rcx),%xmm4
	vphminposuw %xmm4,%xmm6
	vphminposuw (%rcx),%xmm4
	vptest %xmm4,%xmm6
	vptest (%rcx),%xmm4
	vtestps %xmm4,%xmm6
	vtestps (%rcx),%xmm4
	vtestpd %xmm4,%xmm6
	vtestpd (%rcx),%xmm4
	vrcpps %xmm4,%xmm6
	vrcpps (%rcx),%xmm4
	vrsqrtps %xmm4,%xmm6
	vrsqrtps (%rcx),%xmm4
	vsqrtpd %xmm4,%xmm6
	vsqrtpd (%rcx),%xmm4
	vsqrtps %xmm4,%xmm6
	vsqrtps (%rcx),%xmm4
	vaesimc %xmm4,%xmm6
	vaesimc (%rcx),%xmm4

# Tests for op xmm, xmm/mem128
	vmovapd %xmm4,%xmm6
	vmovapd %xmm4,(%rcx)
	vmovaps %xmm4,%xmm6
	vmovaps %xmm4,(%rcx)
	vmovdqa %xmm4,%xmm6
	vmovdqa %xmm4,(%rcx)
	vmovdqu %xmm4,%xmm6
	vmovdqu %xmm4,(%rcx)
	vmovupd %xmm4,%xmm6
	vmovupd %xmm4,(%rcx)
	vmovups %xmm4,%xmm6
	vmovups %xmm4,(%rcx)

# Tests for op mem128, xmm
	vlddqu (%rcx),%xmm4
	vmovntdqa (%rcx),%xmm4

# Tests for op xmm, mem128
	vmovntdq %xmm4,(%rcx)
	vmovntpd %xmm4,(%rcx)
	vmovntps %xmm4,(%rcx)

# Tests for op xmm/mem128, ymm
	vcvtdq2pd %xmm4,%ymm4
	vcvtdq2pd (%rcx),%ymm4
	vcvtps2pd %xmm4,%ymm4
	vcvtps2pd (%rcx),%ymm4

# Tests for op xmm/mem128, xmm, xmm
	vaddpd %xmm4,%xmm6,%xmm2
	vaddpd (%rcx),%xmm6,%xmm7
	vaddps %xmm4,%xmm6,%xmm2
	vaddps (%rcx),%xmm6,%xmm7
	vaddsubpd %xmm4,%xmm6,%xmm2
	vaddsubpd (%rcx),%xmm6,%xmm7
	vaddsubps %xmm4,%xmm6,%xmm2
	vaddsubps (%rcx),%xmm6,%xmm7
	vandnpd %xmm4,%xmm6,%xmm2
	vandnpd (%rcx),%xmm6,%xmm7
	vandnps %xmm4,%xmm6,%xmm2
	vandnps (%rcx),%xmm6,%xmm7
	vandpd %xmm4,%xmm6,%xmm2
	vandpd (%rcx),%xmm6,%xmm7
	vandps %xmm4,%xmm6,%xmm2
	vandps (%rcx),%xmm6,%xmm7
	vdivpd %xmm4,%xmm6,%xmm2
	vdivpd (%rcx),%xmm6,%xmm7
	vdivps %xmm4,%xmm6,%xmm2
	vdivps (%rcx),%xmm6,%xmm7
	vhaddpd %xmm4,%xmm6,%xmm2
	vhaddpd (%rcx),%xmm6,%xmm7
	vhaddps %xmm4,%xmm6,%xmm2
	vhaddps (%rcx),%xmm6,%xmm7
	vhsubpd %xmm4,%xmm6,%xmm2
	vhsubpd (%rcx),%xmm6,%xmm7
	vhsubps %xmm4,%xmm6,%xmm2
	vhsubps (%rcx),%xmm6,%xmm7
	vmaxpd %xmm4,%xmm6,%xmm2
	vmaxpd (%rcx),%xmm6,%xmm7
	vmaxps %xmm4,%xmm6,%xmm2
	vmaxps (%rcx),%xmm6,%xmm7
	vminpd %xmm4,%xmm6,%xmm2
	vminpd (%rcx),%xmm6,%xmm7
	vminps %xmm4,%xmm6,%xmm2
	vminps (%rcx),%xmm6,%xmm7
	vmulpd %xmm4,%xmm6,%xmm2
	vmulpd (%rcx),%xmm6,%xmm7
	vmulps %xmm4,%xmm6,%xmm2
	vmulps (%rcx),%xmm6,%xmm7
	vorpd %xmm4,%xmm6,%xmm2
	vorpd (%rcx),%xmm6,%xmm7
	vorps %xmm4,%xmm6,%xmm2
	vorps (%rcx),%xmm6,%xmm7
	vpacksswb %xmm4,%xmm6,%xmm2
	vpacksswb (%rcx),%xmm6,%xmm7
	vpackssdw %xmm4,%xmm6,%xmm2
	vpackssdw (%rcx),%xmm6,%xmm7
	vpackuswb %xmm4,%xmm6,%xmm2
	vpackuswb (%rcx),%xmm6,%xmm7
	vpackusdw %xmm4,%xmm6,%xmm2
	vpackusdw (%rcx),%xmm6,%xmm7
	vpaddb %xmm4,%xmm6,%xmm2
	vpaddb (%rcx),%xmm6,%xmm7
	vpaddw %xmm4,%xmm6,%xmm2
	vpaddw (%rcx),%xmm6,%xmm7
	vpaddd %xmm4,%xmm6,%xmm2
	vpaddd (%rcx),%xmm6,%xmm7
	vpaddq %xmm4,%xmm6,%xmm2
	vpaddq (%rcx),%xmm6,%xmm7
	vpaddsb %xmm4,%xmm6,%xmm2
	vpaddsb (%rcx),%xmm6,%xmm7
	vpaddsw %xmm4,%xmm6,%xmm2
	vpaddsw (%rcx),%xmm6,%xmm7
	vpaddusb %xmm4,%xmm6,%xmm2
	vpaddusb (%rcx),%xmm6,%xmm7
	vpaddusw %xmm4,%xmm6,%xmm2
	vpaddusw (%rcx),%xmm6,%xmm7
	vpand %xmm4,%xmm6,%xmm2
	vpand (%rcx),%xmm6,%xmm7
	vpandn %xmm4,%xmm6,%xmm2
	vpandn (%rcx),%xmm6,%xmm7
	vpavgb %xmm4,%xmm6,%xmm2
	vpavgb (%rcx),%xmm6,%xmm7
	vpavgw %xmm4,%xmm6,%xmm2
	vpavgw (%rcx),%xmm6,%xmm7
	vpclmullqlqdq %xmm4,%xmm6,%xmm2
	vpclmullqlqdq (%rcx),%xmm6,%xmm7
	vpclmulhqlqdq %xmm4,%xmm6,%xmm2
	vpclmulhqlqdq (%rcx),%xmm6,%xmm7
	vpclmullqhqdq %xmm4,%xmm6,%xmm2
	vpclmullqhqdq (%rcx),%xmm6,%xmm7
	vpclmulhqhqdq %xmm4,%xmm6,%xmm2
	vpclmulhqhqdq (%rcx),%xmm6,%xmm7
	vpcmpeqb %xmm4,%xmm6,%xmm2
	vpcmpeqb (%rcx),%xmm6,%xmm7
	vpcmpeqw %xmm4,%xmm6,%xmm2
	vpcmpeqw (%rcx),%xmm6,%xmm7
	vpcmpeqd %xmm4,%xmm6,%xmm2
	vpcmpeqd (%rcx),%xmm6,%xmm7
	vpcmpeqq %xmm4,%xmm6,%xmm2
	vpcmpeqq (%rcx),%xmm6,%xmm7
	vpcmpgtb %xmm4,%xmm6,%xmm2
	vpcmpgtb (%rcx),%xmm6,%xmm7
	vpcmpgtw %xmm4,%xmm6,%xmm2
	vpcmpgtw (%rcx),%xmm6,%xmm7
	vpcmpgtd %xmm4,%xmm6,%xmm2
	vpcmpgtd (%rcx),%xmm6,%xmm7
	vpcmpgtq %xmm4,%xmm6,%xmm2
	vpcmpgtq (%rcx),%xmm6,%xmm7
	vpermilpd %xmm4,%xmm6,%xmm2
	vpermilpd (%rcx),%xmm6,%xmm7
	vpermilps %xmm4,%xmm6,%xmm2
	vpermilps (%rcx),%xmm6,%xmm7
	vphaddw %xmm4,%xmm6,%xmm2
	vphaddw (%rcx),%xmm6,%xmm7
	vphaddd %xmm4,%xmm6,%xmm2
	vphaddd (%rcx),%xmm6,%xmm7
	vphaddsw %xmm4,%xmm6,%xmm2
	vphaddsw (%rcx),%xmm6,%xmm7
	vphsubw %xmm4,%xmm6,%xmm2
	vphsubw (%rcx),%xmm6,%xmm7
	vphsubd %xmm4,%xmm6,%xmm2
	vphsubd (%rcx),%xmm6,%xmm7
	vphsubsw %xmm4,%xmm6,%xmm2
	vphsubsw (%rcx),%xmm6,%xmm7
	vpmaddwd %xmm4,%xmm6,%xmm2
	vpmaddwd (%rcx),%xmm6,%xmm7
	vpmaddubsw %xmm4,%xmm6,%xmm2
	vpmaddubsw (%rcx),%xmm6,%xmm7
	vpmaxsb %xmm4,%xmm6,%xmm2
	vpmaxsb (%rcx),%xmm6,%xmm7
	vpmaxsw %xmm4,%xmm6,%xmm2
	vpmaxsw (%rcx),%xmm6,%xmm7
	vpmaxsd %xmm4,%xmm6,%xmm2
	vpmaxsd (%rcx),%xmm6,%xmm7
	vpmaxub %xmm4,%xmm6,%xmm2
	vpmaxub (%rcx),%xmm6,%xmm7
	vpmaxuw %xmm4,%xmm6,%xmm2
	vpmaxuw (%rcx),%xmm6,%xmm7
	vpmaxud %xmm4,%xmm6,%xmm2
	vpmaxud (%rcx),%xmm6,%xmm7
	vpminsb %xmm4,%xmm6,%xmm2
	vpminsb (%rcx),%xmm6,%xmm7
	vpminsw %xmm4,%xmm6,%xmm2
	vpminsw (%rcx),%xmm6,%xmm7
	vpminsd %xmm4,%xmm6,%xmm2
	vpminsd (%rcx),%xmm6,%xmm7
	vpminub %xmm4,%xmm6,%xmm2
	vpminub (%rcx),%xmm6,%xmm7
	vpminuw %xmm4,%xmm6,%xmm2
	vpminuw (%rcx),%xmm6,%xmm7
	vpminud %xmm4,%xmm6,%xmm2
	vpminud (%rcx),%xmm6,%xmm7
	vpmulhuw %xmm4,%xmm6,%xmm2
	vpmulhuw (%rcx),%xmm6,%xmm7
	vpmulhrsw %xmm4,%xmm6,%xmm2
	vpmulhrsw (%rcx),%xmm6,%xmm7
	vpmulhw %xmm4,%xmm6,%xmm2
	vpmulhw (%rcx),%xmm6,%xmm7
	vpmullw %xmm4,%xmm6,%xmm2
	vpmullw (%rcx),%xmm6,%xmm7
	vpmulld %xmm4,%xmm6,%xmm2
	vpmulld (%rcx),%xmm6,%xmm7
	vpmuludq %xmm4,%xmm6,%xmm2
	vpmuludq (%rcx),%xmm6,%xmm7
	vpmuldq %xmm4,%xmm6,%xmm2
	vpmuldq (%rcx),%xmm6,%xmm7
	vpor %xmm4,%xmm6,%xmm2
	vpor (%rcx),%xmm6,%xmm7
	vpsadbw %xmm4,%xmm6,%xmm2
	vpsadbw (%rcx),%xmm6,%xmm7
	vpshufb %xmm4,%xmm6,%xmm2
	vpshufb (%rcx),%xmm6,%xmm7
	vpsignb %xmm4,%xmm6,%xmm2
	vpsignb (%rcx),%xmm6,%xmm7
	vpsignw %xmm4,%xmm6,%xmm2
	vpsignw (%rcx),%xmm6,%xmm7
	vpsignd %xmm4,%xmm6,%xmm2
	vpsignd (%rcx),%xmm6,%xmm7
	vpsllw %xmm4,%xmm6,%xmm2
	vpsllw (%rcx),%xmm6,%xmm7
	vpslld %xmm4,%xmm6,%xmm2
	vpslld (%rcx),%xmm6,%xmm7
	vpsllq %xmm4,%xmm6,%xmm2
	vpsllq (%rcx),%xmm6,%xmm7
	vpsraw %xmm4,%xmm6,%xmm2
	vpsraw (%rcx),%xmm6,%xmm7
	vpsrad %xmm4,%xmm6,%xmm2
	vpsrad (%rcx),%xmm6,%xmm7
	vpsrlw %xmm4,%xmm6,%xmm2
	vpsrlw (%rcx),%xmm6,%xmm7
	vpsrld %xmm4,%xmm6,%xmm2
	vpsrld (%rcx),%xmm6,%xmm7
	vpsrlq %xmm4,%xmm6,%xmm2
	vpsrlq (%rcx),%xmm6,%xmm7
	vpsubb %xmm4,%xmm6,%xmm2
	vpsubb (%rcx),%xmm6,%xmm7
	vpsubw %xmm4,%xmm6,%xmm2
	vpsubw (%rcx),%xmm6,%xmm7
	vpsubd %xmm4,%xmm6,%xmm2
	vpsubd (%rcx),%xmm6,%xmm7
	vpsubq %xmm4,%xmm6,%xmm2
	vpsubq (%rcx),%xmm6,%xmm7
	vpsubsb %xmm4,%xmm6,%xmm2
	vpsubsb (%rcx),%xmm6,%xmm7
	vpsubsw %xmm4,%xmm6,%xmm2
	vpsubsw (%rcx),%xmm6,%xmm7
	vpsubusb %xmm4,%xmm6,%xmm2
	vpsubusb (%rcx),%xmm6,%xmm7
	vpsubusw %xmm4,%xmm6,%xmm2
	vpsubusw (%rcx),%xmm6,%xmm7
	vpunpckhbw %xmm4,%xmm6,%xmm2
	vpunpckhbw (%rcx),%xmm6,%xmm7
	vpunpckhwd %xmm4,%xmm6,%xmm2
	vpunpckhwd (%rcx),%xmm6,%xmm7
	vpunpckhdq %xmm4,%xmm6,%xmm2
	vpunpckhdq (%rcx),%xmm6,%xmm7
	vpunpckhqdq %xmm4,%xmm6,%xmm2
	vpunpckhqdq (%rcx),%xmm6,%xmm7
	vpunpcklbw %xmm4,%xmm6,%xmm2
	vpunpcklbw (%rcx),%xmm6,%xmm7
	vpunpcklwd %xmm4,%xmm6,%xmm2
	vpunpcklwd (%rcx),%xmm6,%xmm7
	vpunpckldq %xmm4,%xmm6,%xmm2
	vpunpckldq (%rcx),%xmm6,%xmm7
	vpunpcklqdq %xmm4,%xmm6,%xmm2
	vpunpcklqdq (%rcx),%xmm6,%xmm7
	vpxor %xmm4,%xmm6,%xmm2
	vpxor (%rcx),%xmm6,%xmm7
	vsubpd %xmm4,%xmm6,%xmm2
	vsubpd (%rcx),%xmm6,%xmm7
	vsubps %xmm4,%xmm6,%xmm2
	vsubps (%rcx),%xmm6,%xmm7
	vunpckhpd %xmm4,%xmm6,%xmm2
	vunpckhpd (%rcx),%xmm6,%xmm7
	vunpckhps %xmm4,%xmm6,%xmm2
	vunpckhps (%rcx),%xmm6,%xmm7
	vunpcklpd %xmm4,%xmm6,%xmm2
	vunpcklpd (%rcx),%xmm6,%xmm7
	vunpcklps %xmm4,%xmm6,%xmm2
	vunpcklps (%rcx),%xmm6,%xmm7
	vxorpd %xmm4,%xmm6,%xmm2
	vxorpd (%rcx),%xmm6,%xmm7
	vxorps %xmm4,%xmm6,%xmm2
	vxorps (%rcx),%xmm6,%xmm7
	vaesenc %xmm4,%xmm6,%xmm2
	vaesenc (%rcx),%xmm6,%xmm7
	vaesenclast %xmm4,%xmm6,%xmm2
	vaesenclast (%rcx),%xmm6,%xmm7
	vaesdec %xmm4,%xmm6,%xmm2
	vaesdec (%rcx),%xmm6,%xmm7
	vaesdeclast %xmm4,%xmm6,%xmm2
	vaesdeclast (%rcx),%xmm6,%xmm7
	vcmpeqpd %xmm4,%xmm6,%xmm2
	vcmpeqpd (%rcx),%xmm6,%xmm7
	vcmpltpd %xmm4,%xmm6,%xmm2
	vcmpltpd (%rcx),%xmm6,%xmm7
	vcmplepd %xmm4,%xmm6,%xmm2
	vcmplepd (%rcx),%xmm6,%xmm7
	vcmpunordpd %xmm4,%xmm6,%xmm2
	vcmpunordpd (%rcx),%xmm6,%xmm7
	vcmpneqpd %xmm4,%xmm6,%xmm2
	vcmpneqpd (%rcx),%xmm6,%xmm7
	vcmpnltpd %xmm4,%xmm6,%xmm2
	vcmpnltpd (%rcx),%xmm6,%xmm7
	vcmpnlepd %xmm4,%xmm6,%xmm2
	vcmpnlepd (%rcx),%xmm6,%xmm7
	vcmpordpd %xmm4,%xmm6,%xmm2
	vcmpordpd (%rcx),%xmm6,%xmm7
	vcmpeq_uqpd %xmm4,%xmm6,%xmm2
	vcmpeq_uqpd (%rcx),%xmm6,%xmm7
	vcmpngepd %xmm4,%xmm6,%xmm2
	vcmpngepd (%rcx),%xmm6,%xmm7
	vcmpngtpd %xmm4,%xmm6,%xmm2
	vcmpngtpd (%rcx),%xmm6,%xmm7
	vcmpfalsepd %xmm4,%xmm6,%xmm2
	vcmpfalsepd (%rcx),%xmm6,%xmm7
	vcmpneq_oqpd %xmm4,%xmm6,%xmm2
	vcmpneq_oqpd (%rcx),%xmm6,%xmm7
	vcmpgepd %xmm4,%xmm6,%xmm2
	vcmpgepd (%rcx),%xmm6,%xmm7
	vcmpgtpd %xmm4,%xmm6,%xmm2
	vcmpgtpd (%rcx),%xmm6,%xmm7
	vcmptruepd %xmm4,%xmm6,%xmm2
	vcmptruepd (%rcx),%xmm6,%xmm7
	vcmpeq_ospd %xmm4,%xmm6,%xmm2
	vcmpeq_ospd (%rcx),%xmm6,%xmm7
	vcmplt_oqpd %xmm4,%xmm6,%xmm2
	vcmplt_oqpd (%rcx),%xmm6,%xmm7
	vcmple_oqpd %xmm4,%xmm6,%xmm2
	vcmple_oqpd (%rcx),%xmm6,%xmm7
	vcmpunord_spd %xmm4,%xmm6,%xmm2
	vcmpunord_spd (%rcx),%xmm6,%xmm7
	vcmpneq_uspd %xmm4,%xmm6,%xmm2
	vcmpneq_uspd (%rcx),%xmm6,%xmm7
	vcmpnlt_uqpd %xmm4,%xmm6,%xmm2
	vcmpnlt_uqpd (%rcx),%xmm6,%xmm7
	vcmpnle_uqpd %xmm4,%xmm6,%xmm2
	vcmpnle_uqpd (%rcx),%xmm6,%xmm7
	vcmpord_spd %xmm4,%xmm6,%xmm2
	vcmpord_spd (%rcx),%xmm6,%xmm7
	vcmpeq_uspd %xmm4,%xmm6,%xmm2
	vcmpeq_uspd (%rcx),%xmm6,%xmm7
	vcmpnge_uqpd %xmm4,%xmm6,%xmm2
	vcmpnge_uqpd (%rcx),%xmm6,%xmm7
	vcmpngt_uqpd %xmm4,%xmm6,%xmm2
	vcmpngt_uqpd (%rcx),%xmm6,%xmm7
	vcmpfalse_ospd %xmm4,%xmm6,%xmm2
	vcmpfalse_ospd (%rcx),%xmm6,%xmm7
	vcmpneq_ospd %xmm4,%xmm6,%xmm2
	vcmpneq_ospd (%rcx),%xmm6,%xmm7
	vcmpge_oqpd %xmm4,%xmm6,%xmm2
	vcmpge_oqpd (%rcx),%xmm6,%xmm7
	vcmpgt_oqpd %xmm4,%xmm6,%xmm2
	vcmpgt_oqpd (%rcx),%xmm6,%xmm7
	vcmptrue_uspd %xmm4,%xmm6,%xmm2
	vcmptrue_uspd (%rcx),%xmm6,%xmm7
	vcmpeqps %xmm4,%xmm6,%xmm2
	vcmpeqps (%rcx),%xmm6,%xmm7
	vcmpltps %xmm4,%xmm6,%xmm2
	vcmpltps (%rcx),%xmm6,%xmm7
	vcmpleps %xmm4,%xmm6,%xmm2
	vcmpleps (%rcx),%xmm6,%xmm7
	vcmpunordps %xmm4,%xmm6,%xmm2
	vcmpunordps (%rcx),%xmm6,%xmm7
	vcmpneqps %xmm4,%xmm6,%xmm2
	vcmpneqps (%rcx),%xmm6,%xmm7
	vcmpnltps %xmm4,%xmm6,%xmm2
	vcmpnltps (%rcx),%xmm6,%xmm7
	vcmpnleps %xmm4,%xmm6,%xmm2
	vcmpnleps (%rcx),%xmm6,%xmm7
	vcmpordps %xmm4,%xmm6,%xmm2
	vcmpordps (%rcx),%xmm6,%xmm7
	vcmpeq_uqps %xmm4,%xmm6,%xmm2
	vcmpeq_uqps (%rcx),%xmm6,%xmm7
	vcmpngeps %xmm4,%xmm6,%xmm2
	vcmpngeps (%rcx),%xmm6,%xmm7
	vcmpngtps %xmm4,%xmm6,%xmm2
	vcmpngtps (%rcx),%xmm6,%xmm7
	vcmpfalseps %xmm4,%xmm6,%xmm2
	vcmpfalseps (%rcx),%xmm6,%xmm7
	vcmpneq_oqps %xmm4,%xmm6,%xmm2
	vcmpneq_oqps (%rcx),%xmm6,%xmm7
	vcmpgeps %xmm4,%xmm6,%xmm2
	vcmpgeps (%rcx),%xmm6,%xmm7
	vcmpgtps %xmm4,%xmm6,%xmm2
	vcmpgtps (%rcx),%xmm6,%xmm7
	vcmptrueps %xmm4,%xmm6,%xmm2
	vcmptrueps (%rcx),%xmm6,%xmm7
	vcmpeq_osps %xmm4,%xmm6,%xmm2
	vcmpeq_osps (%rcx),%xmm6,%xmm7
	vcmplt_oqps %xmm4,%xmm6,%xmm2
	vcmplt_oqps (%rcx),%xmm6,%xmm7
	vcmple_oqps %xmm4,%xmm6,%xmm2
	vcmple_oqps (%rcx),%xmm6,%xmm7
	vcmpunord_sps %xmm4,%xmm6,%xmm2
	vcmpunord_sps (%rcx),%xmm6,%xmm7
	vcmpneq_usps %xmm4,%xmm6,%xmm2
	vcmpneq_usps (%rcx),%xmm6,%xmm7
	vcmpnlt_uqps %xmm4,%xmm6,%xmm2
	vcmpnlt_uqps (%rcx),%xmm6,%xmm7
	vcmpnle_uqps %xmm4,%xmm6,%xmm2
	vcmpnle_uqps (%rcx),%xmm6,%xmm7
	vcmpord_sps %xmm4,%xmm6,%xmm2
	vcmpord_sps (%rcx),%xmm6,%xmm7
	vcmpeq_usps %xmm4,%xmm6,%xmm2
	vcmpeq_usps (%rcx),%xmm6,%xmm7
	vcmpnge_uqps %xmm4,%xmm6,%xmm2
	vcmpnge_uqps (%rcx),%xmm6,%xmm7
	vcmpngt_uqps %xmm4,%xmm6,%xmm2
	vcmpngt_uqps (%rcx),%xmm6,%xmm7
	vcmpfalse_osps %xmm4,%xmm6,%xmm2
	vcmpfalse_osps (%rcx),%xmm6,%xmm7
	vcmpneq_osps %xmm4,%xmm6,%xmm2
	vcmpneq_osps (%rcx),%xmm6,%xmm7
	vcmpge_oqps %xmm4,%xmm6,%xmm2
	vcmpge_oqps (%rcx),%xmm6,%xmm7
	vcmpgt_oqps %xmm4,%xmm6,%xmm2
	vcmpgt_oqps (%rcx),%xmm6,%xmm7
	vcmptrue_usps %xmm4,%xmm6,%xmm2
	vcmptrue_usps (%rcx),%xmm6,%xmm7
    vgf2p8mulb %xmm4, %xmm5, %xmm6
	vgf2p8mulb (%rcx), %xmm5, %xmm6
	vgf2p8mulb -123456(%rax,%r14,8), %xmm5, %xmm6
	vgf2p8mulb 2032(%rdx), %xmm5, %xmm6
	vgf2p8mulb 2048(%rdx), %xmm5, %xmm6
	vgf2p8mulb -2048(%rdx), %xmm5, %xmm6
	vgf2p8mulb -2064(%rdx), %xmm5, %xmm6

# Tests for op mem128, xmm, xmm
	vmaskmovps (%rcx),%xmm4,%xmm6
	vmaskmovpd (%rcx),%xmm4,%xmm6

# Tests for op imm8, xmm/mem128, xmm
	vaeskeygenassist $7,%xmm4,%xmm6
	vaeskeygenassist $7,(%rcx),%xmm6
	vpcmpestri $7,%xmm4,%xmm6
	vpcmpestri $7,(%rcx),%xmm6
	vpcmpestriq $7,%xmm4,%xmm6
	vpcmpestril $7,(%rcx),%xmm6
	vpcmpestrm $7,%xmm4,%xmm6
	vpcmpestrm $7,(%rcx),%xmm6
	vpcmpestrmq $7,%xmm4,%xmm6
	vpcmpestrml $7,(%rcx),%xmm6
	vpcmpistri $7,%xmm4,%xmm6
	vpcmpistri $7,(%rcx),%xmm6
	vpcmpistrm $7,%xmm4,%xmm6
	vpcmpistrm $7,(%rcx),%xmm6
	vpermilpd $7,%xmm4,%xmm6
	vpermilpd $7,(%rcx),%xmm6
	vpermilps $7,%xmm4,%xmm6
	vpermilps $7,(%rcx),%xmm6
	vpshufd $7,%xmm4,%xmm6
	vpshufd $7,(%rcx),%xmm6
	vpshufhw $7,%xmm4,%xmm6
	vpshufhw $7,(%rcx),%xmm6
	vpshuflw $7,%xmm4,%xmm6
	vpshuflw $7,(%rcx),%xmm6
	vroundpd $7,%xmm4,%xmm6
	vroundpd $7,(%rcx),%xmm6
	vroundps $7,%xmm4,%xmm6
	vroundps $7,(%rcx),%xmm6

# Tests for op xmm, xmm, mem128
	vmaskmovps %xmm4,%xmm6,(%rcx)
	vmaskmovpd %xmm4,%xmm6,(%rcx)

# Tests for op imm8, xmm/mem128, xmm, xmm
	vblendpd $7,%xmm4,%xmm6,%xmm2
	vblendpd $7,(%rcx),%xmm6,%xmm2
	vblendps $7,%xmm4,%xmm6,%xmm2
	vblendps $7,(%rcx),%xmm6,%xmm2
	vcmppd $7,%xmm4,%xmm6,%xmm2
	vcmppd $7,(%rcx),%xmm6,%xmm2
	vcmpps $7,%xmm4,%xmm6,%xmm2
	vcmpps $7,(%rcx),%xmm6,%xmm2
	vdppd $7,%xmm4,%xmm6,%xmm2
	vdppd $7,(%rcx),%xmm6,%xmm2
	vdpps $7,%xmm4,%xmm6,%xmm2
	vdpps $7,(%rcx),%xmm6,%xmm2
	vmpsadbw $7,%xmm4,%xmm6,%xmm2
	vmpsadbw $7,(%rcx),%xmm6,%xmm2
	vpalignr $7,%xmm4,%xmm6,%xmm2
	vpalignr $7,(%rcx),%xmm6,%xmm2
	vpblendw $7,%xmm4,%xmm6,%xmm2
	vpblendw $7,(%rcx),%xmm6,%xmm2
	vpclmulqdq $7,%xmm4,%xmm6,%xmm2
	vpclmulqdq $7,(%rcx),%xmm6,%xmm2
	vshufpd $7,%xmm4,%xmm6,%xmm2
	vshufpd $7,(%rcx),%xmm6,%xmm2
	vshufps $7,%xmm4,%xmm6,%xmm2
	vshufps $7,(%rcx),%xmm6,%xmm2
    vgf2p8affineqb $0xab, %xmm4, %xmm5, %xmm6
	vgf2p8affineqb $123, %xmm4, %xmm5, %xmm6
	vgf2p8affineqb $123, (%rcx), %xmm5, %xmm6
	vgf2p8affineqb $123, -123456(%rax,%r14,8), %xmm5, %xmm6
	vgf2p8affineqb $123, 2032(%rdx), %xmm5, %xmm6
	vgf2p8affineqb $123, 2048(%rdx), %xmm5, %xmm6
	vgf2p8affineqb $123, -2048(%rdx), %xmm5, %xmm6
	vgf2p8affineqb $123, -2064(%rdx), %xmm5, %xmm6
	vgf2p8affineinvqb $0xab, %xmm4, %xmm5, %xmm6
	vgf2p8affineinvqb $123, %xmm4, %xmm5, %xmm6
	vgf2p8affineinvqb $123, (%rcx), %xmm5, %xmm6
	vgf2p8affineinvqb $123, -123456(%rax,%r14,8), %xmm5, %xmm6
	vgf2p8affineinvqb $123, 2032(%rdx), %xmm5, %xmm6
	vgf2p8affineinvqb $123, 2048(%rdx), %xmm5, %xmm6
	vgf2p8affineinvqb $123, -2048(%rdx), %xmm5, %xmm6
	vgf2p8affineinvqb $123, -2064(%rdx), %xmm5, %xmm6

# Tests for op xmm, xmm/mem128, xmm, xmm
	vblendvpd %xmm4,%xmm6,%xmm2,%xmm7
	vblendvpd %xmm4,(%rcx),%xmm2,%xmm7
	vblendvps %xmm4,%xmm6,%xmm2,%xmm7
	vblendvps %xmm4,(%rcx),%xmm2,%xmm7
	vpblendvb %xmm4,%xmm6,%xmm2,%xmm7
	vpblendvb %xmm4,(%rcx),%xmm2,%xmm7

# Tests for op mem64, ymm
	vbroadcastsd (%rcx),%ymm4

# Tests for op xmm/mem64, xmm
	vcomisd %xmm4,%xmm6
	vcomisd (%rcx),%xmm4
	vcvtdq2pd %xmm4,%xmm6
	vcvtdq2pd (%rcx),%xmm4
	vcvtps2pd %xmm4,%xmm6
	vcvtps2pd (%rcx),%xmm4
	vmovddup %xmm4,%xmm6
	vmovddup (%rcx),%xmm4
	vpmovsxbw %xmm4,%xmm6
	vpmovsxbw (%rcx),%xmm4
	vpmovsxwd %xmm4,%xmm6
	vpmovsxwd (%rcx),%xmm4
	vpmovsxdq %xmm4,%xmm6
	vpmovsxdq (%rcx),%xmm4
	vpmovzxbw %xmm4,%xmm6
	vpmovzxbw (%rcx),%xmm4
	vpmovzxwd %xmm4,%xmm6
	vpmovzxwd (%rcx),%xmm4
	vpmovzxdq %xmm4,%xmm6
	vpmovzxdq (%rcx),%xmm4
	vucomisd %xmm4,%xmm6
	vucomisd (%rcx),%xmm4

# Tests for op mem64, xmm
	vmovsd (%rcx),%xmm4

# Tests for op xmm, mem64
	vmovlpd %xmm4,(%rcx)
	vmovlps %xmm4,(%rcx)
	vmovhpd %xmm4,(%rcx)
	vmovhps %xmm4,(%rcx)
	vmovsd %xmm4,(%rcx)

# Tests for op xmm, regq/mem64
# Tests for op regq/mem64, xmm
	vmovd %xmm4,%rcx
	vmovd %rcx,%xmm4
	vmovq %xmm4,%rcx
	vmovq %rcx,%xmm4
	vmovq %xmm4,(%rcx)
	vmovq (%rcx),%xmm4

# Tests for op xmm/mem64, regl
	vcvtsd2si %xmm4,%ecx
	vcvtsd2si (%rcx),%ecx
	vcvttsd2si %xmm4,%ecx
	vcvttsd2si (%rcx),%ecx

# Tests for op xmm/mem64, regq
	vcvtsd2si %xmm4,%rcx
	vcvtsd2si (%rcx),%rcx
	vcvttsd2si %xmm4,%rcx
	vcvttsd2si (%rcx),%rcx

# Tests for op regq/mem64, xmm, xmm
	vcvtsi2sdq %rcx,%xmm4,%xmm6
	vcvtsi2sdq (%rcx),%xmm4,%xmm6
	vcvtsi2ssq %rcx,%xmm4,%xmm6
	vcvtsi2ssq (%rcx),%xmm4,%xmm6

# Tests for op imm8, regq/mem64, xmm, xmm
	vpinsrq $7,%rcx,%xmm4,%xmm6
	vpinsrq $7,(%rcx),%xmm4,%xmm6

# Testsf for op imm8, xmm, regq/mem64
	vpextrq $7,%xmm4,%rcx
	vpextrq $7,%xmm4,(%rcx)

# Tests for op mem64, xmm, xmm
	vmovlpd (%rcx),%xmm4,%xmm6
	vmovlps (%rcx),%xmm4,%xmm6
	vmovhpd (%rcx),%xmm4,%xmm6
	vmovhps (%rcx),%xmm4,%xmm6

# Tests for op imm8, xmm/mem64, xmm, xmm
	vcmpsd $7,%xmm4,%xmm6,%xmm2
	vcmpsd $7,(%rcx),%xmm6,%xmm2
	vroundsd $7,%xmm4,%xmm6,%xmm2
	vroundsd $7,(%rcx),%xmm6,%xmm2

# Tests for op xmm/mem64, xmm, xmm
	vaddsd %xmm4,%xmm6,%xmm2
	vaddsd (%rcx),%xmm6,%xmm2
	vcvtsd2ss %xmm4,%xmm6,%xmm2
	vcvtsd2ss (%rcx),%xmm6,%xmm2
	vdivsd %xmm4,%xmm6,%xmm2
	vdivsd (%rcx),%xmm6,%xmm2
	vmaxsd %xmm4,%xmm6,%xmm2
	vmaxsd (%rcx),%xmm6,%xmm2
	vminsd %xmm4,%xmm6,%xmm2
	vminsd (%rcx),%xmm6,%xmm2
	vmulsd %xmm4,%xmm6,%xmm2
	vmulsd (%rcx),%xmm6,%xmm2
	vsqrtsd %xmm4,%xmm6,%xmm2
	vsqrtsd (%rcx),%xmm6,%xmm2
	vsubsd %xmm4,%xmm6,%xmm2
	vsubsd (%rcx),%xmm6,%xmm2
	vcmpeqsd %xmm4,%xmm6,%xmm2
	vcmpeqsd (%rcx),%xmm6,%xmm2
	vcmpltsd %xmm4,%xmm6,%xmm2
	vcmpltsd (%rcx),%xmm6,%xmm2
	vcmplesd %xmm4,%xmm6,%xmm2
	vcmplesd (%rcx),%xmm6,%xmm2
	vcmpunordsd %xmm4,%xmm6,%xmm2
	vcmpunordsd (%rcx),%xmm6,%xmm2
	vcmpneqsd %xmm4,%xmm6,%xmm2
	vcmpneqsd (%rcx),%xmm6,%xmm2
	vcmpnltsd %xmm4,%xmm6,%xmm2
	vcmpnltsd (%rcx),%xmm6,%xmm2
	vcmpnlesd %xmm4,%xmm6,%xmm2
	vcmpnlesd (%rcx),%xmm6,%xmm2
	vcmpordsd %xmm4,%xmm6,%xmm2
	vcmpordsd (%rcx),%xmm6,%xmm2
	vcmpeq_uqsd %xmm4,%xmm6,%xmm2
	vcmpeq_uqsd (%rcx),%xmm6,%xmm2
	vcmpngesd %xmm4,%xmm6,%xmm2
	vcmpngesd (%rcx),%xmm6,%xmm2
	vcmpngtsd %xmm4,%xmm6,%xmm2
	vcmpngtsd (%rcx),%xmm6,%xmm2
	vcmpfalsesd %xmm4,%xmm6,%xmm2
	vcmpfalsesd (%rcx),%xmm6,%xmm2
	vcmpneq_oqsd %xmm4,%xmm6,%xmm2
	vcmpneq_oqsd (%rcx),%xmm6,%xmm2
	vcmpgesd %xmm4,%xmm6,%xmm2
	vcmpgesd (%rcx),%xmm6,%xmm2
	vcmpgtsd %xmm4,%xmm6,%xmm2
	vcmpgtsd (%rcx),%xmm6,%xmm2
	vcmptruesd %xmm4,%xmm6,%xmm2
	vcmptruesd (%rcx),%xmm6,%xmm2
	vcmpeq_ossd %xmm4,%xmm6,%xmm2
	vcmpeq_ossd (%rcx),%xmm6,%xmm2
	vcmplt_oqsd %xmm4,%xmm6,%xmm2
	vcmplt_oqsd (%rcx),%xmm6,%xmm2
	vcmple_oqsd %xmm4,%xmm6,%xmm2
	vcmple_oqsd (%rcx),%xmm6,%xmm2
	vcmpunord_ssd %xmm4,%xmm6,%xmm2
	vcmpunord_ssd (%rcx),%xmm6,%xmm2
	vcmpneq_ussd %xmm4,%xmm6,%xmm2
	vcmpneq_ussd (%rcx),%xmm6,%xmm2
	vcmpnlt_uqsd %xmm4,%xmm6,%xmm2
	vcmpnlt_uqsd (%rcx),%xmm6,%xmm2
	vcmpnle_uqsd %xmm4,%xmm6,%xmm2
	vcmpnle_uqsd (%rcx),%xmm6,%xmm2
	vcmpord_ssd %xmm4,%xmm6,%xmm2
	vcmpord_ssd (%rcx),%xmm6,%xmm2
	vcmpeq_ussd %xmm4,%xmm6,%xmm2
	vcmpeq_ussd (%rcx),%xmm6,%xmm2
	vcmpnge_uqsd %xmm4,%xmm6,%xmm2
	vcmpnge_uqsd (%rcx),%xmm6,%xmm2
	vcmpngt_uqsd %xmm4,%xmm6,%xmm2
	vcmpngt_uqsd (%rcx),%xmm6,%xmm2
	vcmpfalse_ossd %xmm4,%xmm6,%xmm2
	vcmpfalse_ossd (%rcx),%xmm6,%xmm2
	vcmpneq_ossd %xmm4,%xmm6,%xmm2
	vcmpneq_ossd (%rcx),%xmm6,%xmm2
	vcmpge_oqsd %xmm4,%xmm6,%xmm2
	vcmpge_oqsd (%rcx),%xmm6,%xmm2
	vcmpgt_oqsd %xmm4,%xmm6,%xmm2
	vcmpgt_oqsd (%rcx),%xmm6,%xmm2
	vcmptrue_ussd %xmm4,%xmm6,%xmm2
	vcmptrue_ussd (%rcx),%xmm6,%xmm2

# Tests for op mem64
	vldmxcsr (%rcx)
	vstmxcsr (%rcx)

# Tests for op xmm/mem32, xmm, xmm
	vaddss %xmm4,%xmm6,%xmm2
	vaddss (%rcx),%xmm6,%xmm2
	vcvtss2sd %xmm4,%xmm6,%xmm2
	vcvtss2sd (%rcx),%xmm6,%xmm2
	vdivss %xmm4,%xmm6,%xmm2
	vdivss (%rcx),%xmm6,%xmm2
	vmaxss %xmm4,%xmm6,%xmm2
	vmaxss (%rcx),%xmm6,%xmm2
	vminss %xmm4,%xmm6,%xmm2
	vminss (%rcx),%xmm6,%xmm2
	vmulss %xmm4,%xmm6,%xmm2
	vmulss (%rcx),%xmm6,%xmm2
	vrcpss %xmm4,%xmm6,%xmm2
	vrcpss (%rcx),%xmm6,%xmm2
	vrsqrtss %xmm4,%xmm6,%xmm2
	vrsqrtss (%rcx),%xmm6,%xmm2
	vsqrtss %xmm4,%xmm6,%xmm2
	vsqrtss (%rcx),%xmm6,%xmm2
	vsubss %xmm4,%xmm6,%xmm2
	vsubss (%rcx),%xmm6,%xmm2
	vcmpeqss %xmm4,%xmm6,%xmm2
	vcmpeqss (%rcx),%xmm6,%xmm2
	vcmpltss %xmm4,%xmm6,%xmm2
	vcmpltss (%rcx),%xmm6,%xmm2
	vcmpless %xmm4,%xmm6,%xmm2
	vcmpless (%rcx),%xmm6,%xmm2
	vcmpunordss %xmm4,%xmm6,%xmm2
	vcmpunordss (%rcx),%xmm6,%xmm2
	vcmpneqss %xmm4,%xmm6,%xmm2
	vcmpneqss (%rcx),%xmm6,%xmm2
	vcmpnltss %xmm4,%xmm6,%xmm2
	vcmpnltss (%rcx),%xmm6,%xmm2
	vcmpnless %xmm4,%xmm6,%xmm2
	vcmpnless (%rcx),%xmm6,%xmm2
	vcmpordss %xmm4,%xmm6,%xmm2
	vcmpordss (%rcx),%xmm6,%xmm2
	vcmpeq_uqss %xmm4,%xmm6,%xmm2
	vcmpeq_uqss (%rcx),%xmm6,%xmm2
	vcmpngess %xmm4,%xmm6,%xmm2
	vcmpngess (%rcx),%xmm6,%xmm2
	vcmpngtss %xmm4,%xmm6,%xmm2
	vcmpngtss (%rcx),%xmm6,%xmm2
	vcmpfalsess %xmm4,%xmm6,%xmm2
	vcmpfalsess (%rcx),%xmm6,%xmm2
	vcmpneq_oqss %xmm4,%xmm6,%xmm2
	vcmpneq_oqss (%rcx),%xmm6,%xmm2
	vcmpgess %xmm4,%xmm6,%xmm2
	vcmpgess (%rcx),%xmm6,%xmm2
	vcmpgtss %xmm4,%xmm6,%xmm2
	vcmpgtss (%rcx),%xmm6,%xmm2
	vcmptruess %xmm4,%xmm6,%xmm2
	vcmptruess (%rcx),%xmm6,%xmm2
	vcmpeq_osss %xmm4,%xmm6,%xmm2
	vcmpeq_osss (%rcx),%xmm6,%xmm2
	vcmplt_oqss %xmm4,%xmm6,%xmm2
	vcmplt_oqss (%rcx),%xmm6,%xmm2
	vcmple_oqss %xmm4,%xmm6,%xmm2
	vcmple_oqss (%rcx),%xmm6,%xmm2
	vcmpunord_sss %xmm4,%xmm6,%xmm2
	vcmpunord_sss (%rcx),%xmm6,%xmm2
	vcmpneq_usss %xmm4,%xmm6,%xmm2
	vcmpneq_usss (%rcx),%xmm6,%xmm2
	vcmpnlt_uqss %xmm4,%xmm6,%xmm2
	vcmpnlt_uqss (%rcx),%xmm6,%xmm2
	vcmpnle_uqss %xmm4,%xmm6,%xmm2
	vcmpnle_uqss (%rcx),%xmm6,%xmm2
	vcmpord_sss %xmm4,%xmm6,%xmm2
	vcmpord_sss (%rcx),%xmm6,%xmm2
	vcmpeq_usss %xmm4,%xmm6,%xmm2
	vcmpeq_usss (%rcx),%xmm6,%xmm2
	vcmpnge_uqss %xmm4,%xmm6,%xmm2
	vcmpnge_uqss (%rcx),%xmm6,%xmm2
	vcmpngt_uqss %xmm4,%xmm6,%xmm2
	vcmpngt_uqss (%rcx),%xmm6,%xmm2
	vcmpfalse_osss %xmm4,%xmm6,%xmm2
	vcmpfalse_osss (%rcx),%xmm6,%xmm2
	vcmpneq_osss %xmm4,%xmm6,%xmm2
	vcmpneq_osss (%rcx),%xmm6,%xmm2
	vcmpge_oqss %xmm4,%xmm6,%xmm2
	vcmpge_oqss (%rcx),%xmm6,%xmm2
	vcmpgt_oqss %xmm4,%xmm6,%xmm2
	vcmpgt_oqss (%rcx),%xmm6,%xmm2
	vcmptrue_usss %xmm4,%xmm6,%xmm2
	vcmptrue_usss (%rcx),%xmm6,%xmm2

# Tests for op mem32, ymm
	vbroadcastss (%rcx),%ymm4

# Tests for op xmm/mem32, xmm
	vcomiss %xmm4,%xmm6
	vcomiss (%rcx),%xmm4
	vpmovsxbd %xmm4,%xmm6
	vpmovsxbd (%rcx),%xmm4
	vpmovsxwq %xmm4,%xmm6
	vpmovsxwq (%rcx),%xmm4
	vpmovzxbd %xmm4,%xmm6
	vpmovzxbd (%rcx),%xmm4
	vpmovzxwq %xmm4,%xmm6
	vpmovzxwq (%rcx),%xmm4
	vucomiss %xmm4,%xmm6
	vucomiss (%rcx),%xmm4

# Tests for op mem32, xmm
	vbroadcastss (%rcx),%xmm4
	vmovss (%rcx),%xmm4

# Tests for op xmm, mem32
	vmovss %xmm4,(%rcx)

# Tests for op xmm, regl/mem32
# Tests for op regl/mem32, xmm
	vmovd %xmm4,%ecx
	vmovd %xmm4,(%rcx)
	vmovd %ecx,%xmm4
	vmovd (%rcx),%xmm4

# Tests for op xmm/mem32, regl
	vcvtss2si %xmm4,%ecx
	vcvtss2si (%rcx),%ecx
	vcvttss2si %xmm4,%ecx
	vcvttss2si (%rcx),%ecx

# Tests for op xmm/mem32, regq
	vcvtss2si %xmm4,%rcx
	vcvtss2si (%rcx),%rcx
	vcvttss2si %xmm4,%rcx
	vcvttss2si (%rcx),%rcx

# Tests for op xmm, regq
	vmovmskpd %xmm4,%rcx
	vmovmskps %xmm4,%rcx
	vpmovmskb %xmm4,%rcx

# Tests for op imm8, xmm, regq/mem32
	vextractps $7,%xmm4,%rcx
	vextractps $7,%xmm4,(%rcx)

# Tests for op imm8, xmm, regl/mem32
	vpextrd $7,%xmm4,%ecx
	vpextrd $7,%xmm4,(%rcx)
	vextractps $7,%xmm4,%ecx
	vextractps $7,%xmm4,(%rcx)

# Tests for op imm8, regl/mem32, xmm, xmm
	vpinsrd $7,%ecx,%xmm4,%xmm6
	vpinsrd $7,(%rcx),%xmm4,%xmm6

# Tests for op regl/mem32, xmm, xmm
	vcvtsi2sd %ecx,%xmm4,%xmm6
	vcvtsi2sdl (%rcx),%xmm4,%xmm6
	vcvtsi2ss %ecx,%xmm4,%xmm6
	vcvtsi2ssl (%rcx),%xmm4,%xmm6

# Tests for op imm8, xmm/mem32, xmm, xmm
	vcmpss $7,%xmm4,%xmm6,%xmm2
	vcmpss $7,(%rcx),%xmm6,%xmm2
	vinsertps $7,%xmm4,%xmm6,%xmm2
	vinsertps $7,(%rcx),%xmm6,%xmm2
	vroundss $7,%xmm4,%xmm6,%xmm2
	vroundss $7,(%rcx),%xmm6,%xmm2

# Tests for op xmm/m16, xmm
	vpmovsxbq %xmm4,%xmm6
	vpmovsxbq (%rcx),%xmm4
	vpmovzxbq %xmm4,%xmm6
	vpmovzxbq (%rcx),%xmm4

# Tests for op imm8, xmm, regl/mem16
	vpextrw $7,%xmm4,%ecx
	vpextrw $7,%xmm4,(%rcx)

# Tests for op imm8, xmm, regq/mem16
	vpextrw $7,%xmm4,%rcx
	vpextrw $7,%xmm4,(%rcx)

# Tests for op imm8, regl/mem16, xmm, xmm
	vpinsrw $7,%ecx,%xmm4,%xmm6
	vpinsrw $7,(%rcx),%xmm4,%xmm6


	vpinsrw $7,%rcx,%xmm4,%xmm6
	vpinsrw $7,(%rcx),%xmm4,%xmm6

# Tests for op imm8, xmm, regl/mem8
	vpextrb $7,%xmm4,%ecx
	vpextrb $7,%xmm4,(%rcx)

# Tests for op imm8, regl/mem8, xmm, xmm
	vpinsrb $7,%ecx,%xmm4,%xmm6
	vpinsrb $7,(%rcx),%xmm4,%xmm6

# Tests for op imm8, xmm, regq
	vpextrw $7,%xmm4,%rcx

# Tests for op imm8, xmm, regq/mem8
	vpextrb $7,%xmm4,%rcx
	vpextrb $7,%xmm4,(%rcx)

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

# Tests for op ymm, regq
	vmovmskpd %ymm4,%rcx
	vmovmskps %ymm4,%rcx

# Default instructions without suffixes.
	vcvtpd2dq %xmm4,%xmm6
	vcvtpd2dq %ymm4,%xmm6
	vcvtpd2ps %xmm4,%xmm6
	vcvtpd2ps %ymm4,%xmm6
	vcvttpd2dq %xmm4,%xmm6
	vcvttpd2dq %ymm4,%xmm6

#Tests with different memory and register operands.
	vldmxcsr 0x12345678
	vmovdqa 0x12345678,%xmm8
	vmovdqa %xmm8,0x12345678
	vmovd %xmm8,0x12345678
	vcvtsd2si 0x12345678,%r8d
	vcvtdq2pd 0x12345678,%ymm8
	vcvtpd2psy 0x12345678,%xmm8
	vpavgb 0x12345678,%xmm8,%xmm15
	vaeskeygenassist $7,0x12345678,%xmm8
	vpextrb $7,%xmm8,0x12345678
	vcvtsi2sdl 0x12345678,%xmm8,%xmm15
	vpclmulqdq $7,0x12345678,%xmm8,%xmm15
	vblendvps %xmm8,0x12345678,%xmm12,%xmm14
	vpinsrb $7,0x12345678,%xmm8,%xmm15
	vmovdqa 0x12345678,%ymm8
	vmovdqa %ymm8,0x12345678
	vpermilpd 0x12345678,%ymm8,%ymm15
	vroundpd $7,0x12345678,%ymm8
	vextractf128 $7,%ymm8,0x12345678
	vperm2f128 $7,0x12345678,%ymm8,%ymm15
	vblendvpd %ymm8,0x12345678,%ymm12,%ymm14
	vldmxcsr (%rbp)
	vmovdqa (%rbp),%xmm8
	vmovdqa %xmm8,(%rbp)
	vmovd %xmm8,(%rbp)
	vcvtsd2si (%rbp),%r8d
	vcvtdq2pd (%rbp),%ymm8
	vcvtpd2psy (%rbp),%xmm8
	vpavgb (%rbp),%xmm8,%xmm15
	vaeskeygenassist $7,(%rbp),%xmm8
	vpextrb $7,%xmm8,(%rbp)
	vcvtsi2sdl (%rbp),%xmm8,%xmm15
	vpclmulqdq $7,(%rbp),%xmm8,%xmm15
	vblendvps %xmm8,(%rbp),%xmm12,%xmm14
	vpinsrb $7,(%rbp),%xmm8,%xmm15
	vmovdqa (%rbp),%ymm8
	vmovdqa %ymm8,(%rbp)
	vpermilpd (%rbp),%ymm8,%ymm15
	vroundpd $7,(%rbp),%ymm8
	vextractf128 $7,%ymm8,(%rbp)
	vperm2f128 $7,(%rbp),%ymm8,%ymm15
	vblendvpd %ymm8,(%rbp),%ymm12,%ymm14
	vldmxcsr (%rsp)
	vmovdqa (%rsp),%xmm8
	vmovdqa %xmm8,(%rsp)
	vmovd %xmm8,(%rsp)
	vcvtsd2si (%rsp),%r8d
	vcvtdq2pd (%rsp),%ymm8
	vcvtpd2psy (%rsp),%xmm8
	vpavgb (%rsp),%xmm8,%xmm15
	vaeskeygenassist $7,(%rsp),%xmm8
	vpextrb $7,%xmm8,(%rsp)
	vcvtsi2sdl (%rsp),%xmm8,%xmm15
	vpclmulqdq $7,(%rsp),%xmm8,%xmm15
	vblendvps %xmm8,(%rsp),%xmm12,%xmm14
	vpinsrb $7,(%rsp),%xmm8,%xmm15
	vmovdqa (%rsp),%ymm8
	vmovdqa %ymm8,(%rsp)
	vpermilpd (%rsp),%ymm8,%ymm15
	vroundpd $7,(%rsp),%ymm8
	vextractf128 $7,%ymm8,(%rsp)
	vperm2f128 $7,(%rsp),%ymm8,%ymm15
	vblendvpd %ymm8,(%rsp),%ymm12,%ymm14
	vldmxcsr 0x99(%rbp)
	vmovdqa 0x99(%rbp),%xmm8
	vmovdqa %xmm8,0x99(%rbp)
	vmovd %xmm8,0x99(%rbp)
	vcvtsd2si 0x99(%rbp),%r8d
	vcvtdq2pd 0x99(%rbp),%ymm8
	vcvtpd2psy 0x99(%rbp),%xmm8
	vpavgb 0x99(%rbp),%xmm8,%xmm15
	vaeskeygenassist $7,0x99(%rbp),%xmm8
	vpextrb $7,%xmm8,0x99(%rbp)
	vcvtsi2sdl 0x99(%rbp),%xmm8,%xmm15
	vpclmulqdq $7,0x99(%rbp),%xmm8,%xmm15
	vblendvps %xmm8,0x99(%rbp),%xmm12,%xmm14
	vpinsrb $7,0x99(%rbp),%xmm8,%xmm15
	vmovdqa 0x99(%rbp),%ymm8
	vmovdqa %ymm8,0x99(%rbp)
	vpermilpd 0x99(%rbp),%ymm8,%ymm15
	vroundpd $7,0x99(%rbp),%ymm8
	vextractf128 $7,%ymm8,0x99(%rbp)
	vperm2f128 $7,0x99(%rbp),%ymm8,%ymm15
	vblendvpd %ymm8,0x99(%rbp),%ymm12,%ymm14
	vldmxcsr 0x99(%r15)
	vmovdqa 0x99(%r15),%xmm8
	vmovdqa %xmm8,0x99(%r15)
	vmovd %xmm8,0x99(%r15)
	vcvtsd2si 0x99(%r15),%r8d
	vcvtdq2pd 0x99(%r15),%ymm8
	vcvtpd2psy 0x99(%r15),%xmm8
	vpavgb 0x99(%r15),%xmm8,%xmm15
	vaeskeygenassist $7,0x99(%r15),%xmm8
	vpextrb $7,%xmm8,0x99(%r15)
	vcvtsi2sdl 0x99(%r15),%xmm8,%xmm15
	vpclmulqdq $7,0x99(%r15),%xmm8,%xmm15
	vblendvps %xmm8,0x99(%r15),%xmm12,%xmm14
	vpinsrb $7,0x99(%r15),%xmm8,%xmm15
	vmovdqa 0x99(%r15),%ymm8
	vmovdqa %ymm8,0x99(%r15)
	vpermilpd 0x99(%r15),%ymm8,%ymm15
	vroundpd $7,0x99(%r15),%ymm8
	vextractf128 $7,%ymm8,0x99(%r15)
	vperm2f128 $7,0x99(%r15),%ymm8,%ymm15
	vblendvpd %ymm8,0x99(%r15),%ymm12,%ymm14
	vldmxcsr 0x99(%rip)
	vmovdqa 0x99(%rip),%xmm8
	vmovdqa %xmm8,0x99(%rip)
	vmovd %xmm8,0x99(%rip)
	vcvtsd2si 0x99(%rip),%r8d
	vcvtdq2pd 0x99(%rip),%ymm8
	vcvtpd2psy 0x99(%rip),%xmm8
	vpavgb 0x99(%rip),%xmm8,%xmm15
	vaeskeygenassist $7,0x99(%rip),%xmm8
	vpextrb $7,%xmm8,0x99(%rip)
	vcvtsi2sdl 0x99(%rip),%xmm8,%xmm15
	vpclmulqdq $7,0x99(%rip),%xmm8,%xmm15
	vblendvps %xmm8,0x99(%rip),%xmm12,%xmm14
	vpinsrb $7,0x99(%rip),%xmm8,%xmm15
	vmovdqa 0x99(%rip),%ymm8
	vmovdqa %ymm8,0x99(%rip)
	vpermilpd 0x99(%rip),%ymm8,%ymm15
	vroundpd $7,0x99(%rip),%ymm8
	vextractf128 $7,%ymm8,0x99(%rip)
	vperm2f128 $7,0x99(%rip),%ymm8,%ymm15
	vblendvpd %ymm8,0x99(%rip),%ymm12,%ymm14
	vldmxcsr 0x99(%rsp)
	vmovdqa 0x99(%rsp),%xmm8
	vmovdqa %xmm8,0x99(%rsp)
	vmovd %xmm8,0x99(%rsp)
	vcvtsd2si 0x99(%rsp),%r8d
	vcvtdq2pd 0x99(%rsp),%ymm8
	vcvtpd2psy 0x99(%rsp),%xmm8
	vpavgb 0x99(%rsp),%xmm8,%xmm15
	vaeskeygenassist $7,0x99(%rsp),%xmm8
	vpextrb $7,%xmm8,0x99(%rsp)
	vcvtsi2sdl 0x99(%rsp),%xmm8,%xmm15
	vpclmulqdq $7,0x99(%rsp),%xmm8,%xmm15
	vblendvps %xmm8,0x99(%rsp),%xmm12,%xmm14
	vpinsrb $7,0x99(%rsp),%xmm8,%xmm15
	vmovdqa 0x99(%rsp),%ymm8
	vmovdqa %ymm8,0x99(%rsp)
	vpermilpd 0x99(%rsp),%ymm8,%ymm15
	vroundpd $7,0x99(%rsp),%ymm8
	vextractf128 $7,%ymm8,0x99(%rsp)
	vperm2f128 $7,0x99(%rsp),%ymm8,%ymm15
	vblendvpd %ymm8,0x99(%rsp),%ymm12,%ymm14
	vldmxcsr 0x99(%r12)
	vmovdqa 0x99(%r12),%xmm8
	vmovdqa %xmm8,0x99(%r12)
	vmovd %xmm8,0x99(%r12)
	vcvtsd2si 0x99(%r12),%r8d
	vcvtdq2pd 0x99(%r12),%ymm8
	vcvtpd2psy 0x99(%r12),%xmm8
	vpavgb 0x99(%r12),%xmm8,%xmm15
	vaeskeygenassist $7,0x99(%r12),%xmm8
	vpextrb $7,%xmm8,0x99(%r12)
	vcvtsi2sdl 0x99(%r12),%xmm8,%xmm15
	vpclmulqdq $7,0x99(%r12),%xmm8,%xmm15
	vblendvps %xmm8,0x99(%r12),%xmm12,%xmm14
	vpinsrb $7,0x99(%r12),%xmm8,%xmm15
	vmovdqa 0x99(%r12),%ymm8
	vmovdqa %ymm8,0x99(%r12)
	vpermilpd 0x99(%r12),%ymm8,%ymm15
	vroundpd $7,0x99(%r12),%ymm8
	vextractf128 $7,%ymm8,0x99(%r12)
	vperm2f128 $7,0x99(%r12),%ymm8,%ymm15
	vblendvpd %ymm8,0x99(%r12),%ymm12,%ymm14
	vldmxcsr -0x99(,%riz)
	vmovdqa -0x99(,%riz),%xmm8
	vmovdqa %xmm8,-0x99(,%riz)
	vmovd %xmm8,-0x99(,%riz)
	vcvtsd2si -0x99(,%riz),%r8d
	vcvtdq2pd -0x99(,%riz),%ymm8
	vcvtpd2psy -0x99(,%riz),%xmm8
	vpavgb -0x99(,%riz),%xmm8,%xmm15
	vaeskeygenassist $7,-0x99(,%riz),%xmm8
	vpextrb $7,%xmm8,-0x99(,%riz)
	vcvtsi2sdl -0x99(,%riz),%xmm8,%xmm15
	vpclmulqdq $7,-0x99(,%riz),%xmm8,%xmm15
	vblendvps %xmm8,-0x99(,%riz),%xmm12,%xmm14
	vpinsrb $7,-0x99(,%riz),%xmm8,%xmm15
	vmovdqa -0x99(,%riz),%ymm8
	vmovdqa %ymm8,-0x99(,%riz)
	vpermilpd -0x99(,%riz),%ymm8,%ymm15
	vroundpd $7,-0x99(,%riz),%ymm8
	vextractf128 $7,%ymm8,-0x99(,%riz)
	vperm2f128 $7,-0x99(,%riz),%ymm8,%ymm15
	vblendvpd %ymm8,-0x99(,%riz),%ymm12,%ymm14
	vldmxcsr -0x99(,%riz,2)
	vmovdqa -0x99(,%riz,2),%xmm8
	vmovdqa %xmm8,-0x99(,%riz,2)
	vmovd %xmm8,-0x99(,%riz,2)
	vcvtsd2si -0x99(,%riz,2),%r8d
	vcvtdq2pd -0x99(,%riz,2),%ymm8
	vcvtpd2psy -0x99(,%riz,2),%xmm8
	vpavgb -0x99(,%riz,2),%xmm8,%xmm15
	vaeskeygenassist $7,-0x99(,%riz,2),%xmm8
	vpextrb $7,%xmm8,-0x99(,%riz,2)
	vcvtsi2sdl -0x99(,%riz,2),%xmm8,%xmm15
	vpclmulqdq $7,-0x99(,%riz,2),%xmm8,%xmm15
	vblendvps %xmm8,-0x99(,%riz,2),%xmm12,%xmm14
	vpinsrb $7,-0x99(,%riz,2),%xmm8,%xmm15
	vmovdqa -0x99(,%riz,2),%ymm8
	vmovdqa %ymm8,-0x99(,%riz,2)
	vpermilpd -0x99(,%riz,2),%ymm8,%ymm15
	vroundpd $7,-0x99(,%riz,2),%ymm8
	vextractf128 $7,%ymm8,-0x99(,%riz,2)
	vperm2f128 $7,-0x99(,%riz,2),%ymm8,%ymm15
	vblendvpd %ymm8,-0x99(,%riz,2),%ymm12,%ymm14
	vldmxcsr -0x99(%rbx,%riz)
	vmovdqa -0x99(%rbx,%riz),%xmm8
	vmovdqa %xmm8,-0x99(%rbx,%riz)
	vmovd %xmm8,-0x99(%rbx,%riz)
	vcvtsd2si -0x99(%rbx,%riz),%r8d
	vcvtdq2pd -0x99(%rbx,%riz),%ymm8
	vcvtpd2psy -0x99(%rbx,%riz),%xmm8
	vpavgb -0x99(%rbx,%riz),%xmm8,%xmm15
	vaeskeygenassist $7,-0x99(%rbx,%riz),%xmm8
	vpextrb $7,%xmm8,-0x99(%rbx,%riz)
	vcvtsi2sdl -0x99(%rbx,%riz),%xmm8,%xmm15
	vpclmulqdq $7,-0x99(%rbx,%riz),%xmm8,%xmm15
	vblendvps %xmm8,-0x99(%rbx,%riz),%xmm12,%xmm14
	vpinsrb $7,-0x99(%rbx,%riz),%xmm8,%xmm15
	vmovdqa -0x99(%rbx,%riz),%ymm8
	vmovdqa %ymm8,-0x99(%rbx,%riz)
	vpermilpd -0x99(%rbx,%riz),%ymm8,%ymm15
	vroundpd $7,-0x99(%rbx,%riz),%ymm8
	vextractf128 $7,%ymm8,-0x99(%rbx,%riz)
	vperm2f128 $7,-0x99(%rbx,%riz),%ymm8,%ymm15
	vblendvpd %ymm8,-0x99(%rbx,%riz),%ymm12,%ymm14
	vldmxcsr -0x99(%rbx,%riz,2)
	vmovdqa -0x99(%rbx,%riz,2),%xmm8
	vmovdqa %xmm8,-0x99(%rbx,%riz,2)
	vmovd %xmm8,-0x99(%rbx,%riz,2)
	vcvtsd2si -0x99(%rbx,%riz,2),%r8d
	vcvtdq2pd -0x99(%rbx,%riz,2),%ymm8
	vcvtpd2psy -0x99(%rbx,%riz,2),%xmm8
	vpavgb -0x99(%rbx,%riz,2),%xmm8,%xmm15
	vaeskeygenassist $7,-0x99(%rbx,%riz,2),%xmm8
	vpextrb $7,%xmm8,-0x99(%rbx,%riz,2)
	vcvtsi2sdl -0x99(%rbx,%riz,2),%xmm8,%xmm15
	vpclmulqdq $7,-0x99(%rbx,%riz,2),%xmm8,%xmm15
	vblendvps %xmm8,-0x99(%rbx,%riz,2),%xmm12,%xmm14
	vpinsrb $7,-0x99(%rbx,%riz,2),%xmm8,%xmm15
	vmovdqa -0x99(%rbx,%riz,2),%ymm8
	vmovdqa %ymm8,-0x99(%rbx,%riz,2)
	vpermilpd -0x99(%rbx,%riz,2),%ymm8,%ymm15
	vroundpd $7,-0x99(%rbx,%riz,2),%ymm8
	vextractf128 $7,%ymm8,-0x99(%rbx,%riz,2)
	vperm2f128 $7,-0x99(%rbx,%riz,2),%ymm8,%ymm15
	vblendvpd %ymm8,-0x99(%rbx,%riz,2),%ymm12,%ymm14
	vldmxcsr -0x99(%r12,%r15,4)
	vmovdqa -0x99(%r12,%r15,4),%xmm8
	vmovdqa %xmm8,-0x99(%r12,%r15,4)
	vmovd %xmm8,-0x99(%r12,%r15,4)
	vcvtsd2si -0x99(%r12,%r15,4),%r8d
	vcvtdq2pd -0x99(%r12,%r15,4),%ymm8
	vcvtpd2psy -0x99(%r12,%r15,4),%xmm8
	vpavgb -0x99(%r12,%r15,4),%xmm8,%xmm15
	vaeskeygenassist $7,-0x99(%r12,%r15,4),%xmm8
	vpextrb $7,%xmm8,-0x99(%r12,%r15,4)
	vcvtsi2sdl -0x99(%r12,%r15,4),%xmm8,%xmm15
	vpclmulqdq $7,-0x99(%r12,%r15,4),%xmm8,%xmm15
	vblendvps %xmm8,-0x99(%r12,%r15,4),%xmm12,%xmm14
	vpinsrb $7,-0x99(%r12,%r15,4),%xmm8,%xmm15
	vmovdqa -0x99(%r12,%r15,4),%ymm8
	vmovdqa %ymm8,-0x99(%r12,%r15,4)
	vpermilpd -0x99(%r12,%r15,4),%ymm8,%ymm15
	vroundpd $7,-0x99(%r12,%r15,4),%ymm8
	vextractf128 $7,%ymm8,-0x99(%r12,%r15,4)
	vperm2f128 $7,-0x99(%r12,%r15,4),%ymm8,%ymm15
	vblendvpd %ymm8,-0x99(%r12,%r15,4),%ymm12,%ymm14
	vldmxcsr -0x99(%r8,%r15,8)
	vmovdqa -0x99(%r8,%r15,8),%xmm8
	vmovdqa %xmm8,-0x99(%r8,%r15,8)
	vmovd %xmm8,-0x99(%r8,%r15,8)
	vcvtsd2si -0x99(%r8,%r15,8),%r8d
	vcvtdq2pd -0x99(%r8,%r15,8),%ymm8
	vcvtpd2psy -0x99(%r8,%r15,8),%xmm8
	vpavgb -0x99(%r8,%r15,8),%xmm8,%xmm15
	vaeskeygenassist $7,-0x99(%r8,%r15,8),%xmm8
	vpextrb $7,%xmm8,-0x99(%r8,%r15,8)
	vcvtsi2sdl -0x99(%r8,%r15,8),%xmm8,%xmm15
	vpclmulqdq $7,-0x99(%r8,%r15,8),%xmm8,%xmm15
	vblendvps %xmm8,-0x99(%r8,%r15,8),%xmm12,%xmm14
	vpinsrb $7,-0x99(%r8,%r15,8),%xmm8,%xmm15
	vmovdqa -0x99(%r8,%r15,8),%ymm8
	vmovdqa %ymm8,-0x99(%r8,%r15,8)
	vpermilpd -0x99(%r8,%r15,8),%ymm8,%ymm15
	vroundpd $7,-0x99(%r8,%r15,8),%ymm8
	vextractf128 $7,%ymm8,-0x99(%r8,%r15,8)
	vperm2f128 $7,-0x99(%r8,%r15,8),%ymm8,%ymm15
	vblendvpd %ymm8,-0x99(%r8,%r15,8),%ymm12,%ymm14
	vldmxcsr -0x99(%rbp,%r13,4)
	vmovdqa -0x99(%rbp,%r13,4),%xmm8
	vmovdqa %xmm8,-0x99(%rbp,%r13,4)
	vmovd %xmm8,-0x99(%rbp,%r13,4)
	vcvtsd2si -0x99(%rbp,%r13,4),%r8d
	vcvtdq2pd -0x99(%rbp,%r13,4),%ymm8
	vcvtpd2psy -0x99(%rbp,%r13,4),%xmm8
	vpavgb -0x99(%rbp,%r13,4),%xmm8,%xmm15
	vaeskeygenassist $7,-0x99(%rbp,%r13,4),%xmm8
	vpextrb $7,%xmm8,-0x99(%rbp,%r13,4)
	vcvtsi2sdl -0x99(%rbp,%r13,4),%xmm8,%xmm15
	vpclmulqdq $7,-0x99(%rbp,%r13,4),%xmm8,%xmm15
	vblendvps %xmm8,-0x99(%rbp,%r13,4),%xmm12,%xmm14
	vpinsrb $7,-0x99(%rbp,%r13,4),%xmm8,%xmm15
	vmovdqa -0x99(%rbp,%r13,4),%ymm8
	vmovdqa %ymm8,-0x99(%rbp,%r13,4)
	vpermilpd -0x99(%rbp,%r13,4),%ymm8,%ymm15
	vroundpd $7,-0x99(%rbp,%r13,4),%ymm8
	vextractf128 $7,%ymm8,-0x99(%rbp,%r13,4)
	vperm2f128 $7,-0x99(%rbp,%r13,4),%ymm8,%ymm15
	vblendvpd %ymm8,-0x99(%rbp,%r13,4),%ymm12,%ymm14
	vldmxcsr -0x99(%rsp,%r12,1)
	vmovdqa -0x99(%rsp,%r12,1),%xmm8
	vmovdqa %xmm8,-0x99(%rsp,%r12,1)
	vmovd %xmm8,-0x99(%rsp,%r12,1)
	vcvtsd2si -0x99(%rsp,%r12,1),%r8d
	vcvtdq2pd -0x99(%rsp,%r12,1),%ymm8
	vcvtpd2psy -0x99(%rsp,%r12,1),%xmm8
	vpavgb -0x99(%rsp,%r12,1),%xmm8,%xmm15
	vaeskeygenassist $7,-0x99(%rsp,%r12,1),%xmm8
	vpextrb $7,%xmm8,-0x99(%rsp,%r12,1)
	vcvtsi2sdl -0x99(%rsp,%r12,1),%xmm8,%xmm15
	vpclmulqdq $7,-0x99(%rsp,%r12,1),%xmm8,%xmm15
	vblendvps %xmm8,-0x99(%rsp,%r12,1),%xmm12,%xmm14
	vpinsrb $7,-0x99(%rsp,%r12,1),%xmm8,%xmm15
	vmovdqa -0x99(%rsp,%r12,1),%ymm8
	vmovdqa %ymm8,-0x99(%rsp,%r12,1)
	vpermilpd -0x99(%rsp,%r12,1),%ymm8,%ymm15
	vroundpd $7,-0x99(%rsp,%r12,1),%ymm8
	vextractf128 $7,%ymm8,-0x99(%rsp,%r12,1)
	vperm2f128 $7,-0x99(%rsp,%r12,1),%ymm8,%ymm15
	vblendvpd %ymm8,-0x99(%rsp,%r12,1),%ymm12,%ymm14
# Tests for all register operands.
	vmovmskpd %xmm8,%r8d
	vpslld $7,%xmm8,%xmm15
	vmovmskps %ymm8,%r8d
	vmovdqa %xmm8,%xmm15
	vmovd %xmm8,%r8d
	vcvtsd2si %xmm8,%r8d
	vcvtdq2pd %xmm8,%ymm8
	vcvtpd2psy %ymm8,%xmm8
	vaeskeygenassist $7,%xmm8,%xmm15
	vpextrb $7,%xmm8,%r8d
	vcvtsi2sdl %r8d,%xmm8,%xmm15
	vpclmulqdq $7,%xmm8,%xmm15,%xmm12
	vblendvps %xmm8,%xmm8,%xmm12,%xmm14
	vpinsrb $7,%r8d,%xmm8,%xmm15
	vmovdqa %ymm8,%ymm15
	vpermilpd %ymm8,%ymm15,%ymm12
	vroundpd $7,%ymm8,%ymm15
	vextractf128 $7,%ymm8,%xmm8
	vperm2f128 $7,%ymm8,%ymm15,%ymm12
	vblendvpd %ymm8,%ymm15,%ymm12,%ymm14
	vinsertf128 $7,%xmm8,%ymm8,%ymm15
# Tests for different memory/register operand
	vcvtsd2si (%rcx),%r8
	vextractps $10,%xmm8,%r8
	vcvtss2si (%rcx),%r8
	vpinsrw $7,%r8,%xmm15,%xmm8

	.intel_syntax noprefix

# Tests for op mem64
	vldmxcsr DWORD PTR [rcx]
	vldmxcsr [rcx]
	vstmxcsr DWORD PTR [rcx]
	vstmxcsr [rcx]

# Tests for op mem256, mask,  ymm
# Tests for op ymm, mask, mem256
	vmaskmovpd ymm6,ymm4,YMMWORD PTR [rcx]
	vmaskmovpd YMMWORD PTR [rcx],ymm6,ymm4
	vmaskmovpd ymm6,ymm4,[rcx]
	vmaskmovpd [rcx],ymm6,ymm4
	vmaskmovps ymm6,ymm4,YMMWORD PTR [rcx]
	vmaskmovps YMMWORD PTR [rcx],ymm6,ymm4
	vmaskmovps ymm6,ymm4,[rcx]
	vmaskmovps [rcx],ymm6,ymm4

# Tests for op imm8, ymm/mem256, ymm
	vpermilpd ymm2,ymm6,7
	vpermilpd ymm6,YMMWORD PTR [rcx],7
	vpermilpd ymm6,[rcx],7
	vpermilps ymm2,ymm6,7
	vpermilps ymm6,YMMWORD PTR [rcx],7
	vpermilps ymm6,[rcx],7
	vroundpd ymm2,ymm6,7
	vroundpd ymm6,YMMWORD PTR [rcx],7
	vroundpd ymm6,[rcx],7
	vroundps ymm2,ymm6,7
	vroundps ymm6,YMMWORD PTR [rcx],7
	vroundps ymm6,[rcx],7

# Tests for op ymm/mem256, ymm, ymm
	vaddpd ymm2,ymm6,ymm4
	vaddpd ymm2,ymm6,YMMWORD PTR [rcx]
	vaddpd ymm2,ymm6,[rcx]
	vaddps ymm2,ymm6,ymm4
	vaddps ymm2,ymm6,YMMWORD PTR [rcx]
	vaddps ymm2,ymm6,[rcx]
	vaddsubpd ymm2,ymm6,ymm4
	vaddsubpd ymm2,ymm6,YMMWORD PTR [rcx]
	vaddsubpd ymm2,ymm6,[rcx]
	vaddsubps ymm2,ymm6,ymm4
	vaddsubps ymm2,ymm6,YMMWORD PTR [rcx]
	vaddsubps ymm2,ymm6,[rcx]
	vandnpd ymm2,ymm6,ymm4
	vandnpd ymm2,ymm6,YMMWORD PTR [rcx]
	vandnpd ymm2,ymm6,[rcx]
	vandnps ymm2,ymm6,ymm4
	vandnps ymm2,ymm6,YMMWORD PTR [rcx]
	vandnps ymm2,ymm6,[rcx]
	vandpd ymm2,ymm6,ymm4
	vandpd ymm2,ymm6,YMMWORD PTR [rcx]
	vandpd ymm2,ymm6,[rcx]
	vandps ymm2,ymm6,ymm4
	vandps ymm2,ymm6,YMMWORD PTR [rcx]
	vandps ymm2,ymm6,[rcx]
	vdivpd ymm2,ymm6,ymm4
	vdivpd ymm2,ymm6,YMMWORD PTR [rcx]
	vdivpd ymm2,ymm6,[rcx]
	vdivps ymm2,ymm6,ymm4
	vdivps ymm2,ymm6,YMMWORD PTR [rcx]
	vdivps ymm2,ymm6,[rcx]
	vhaddpd ymm2,ymm6,ymm4
	vhaddpd ymm2,ymm6,YMMWORD PTR [rcx]
	vhaddpd ymm2,ymm6,[rcx]
	vhaddps ymm2,ymm6,ymm4
	vhaddps ymm2,ymm6,YMMWORD PTR [rcx]
	vhaddps ymm2,ymm6,[rcx]
	vhsubpd ymm2,ymm6,ymm4
	vhsubpd ymm2,ymm6,YMMWORD PTR [rcx]
	vhsubpd ymm2,ymm6,[rcx]
	vhsubps ymm2,ymm6,ymm4
	vhsubps ymm2,ymm6,YMMWORD PTR [rcx]
	vhsubps ymm2,ymm6,[rcx]
	vmaxpd ymm2,ymm6,ymm4
	vmaxpd ymm2,ymm6,YMMWORD PTR [rcx]
	vmaxpd ymm2,ymm6,[rcx]
	vmaxps ymm2,ymm6,ymm4
	vmaxps ymm2,ymm6,YMMWORD PTR [rcx]
	vmaxps ymm2,ymm6,[rcx]
	vminpd ymm2,ymm6,ymm4
	vminpd ymm2,ymm6,YMMWORD PTR [rcx]
	vminpd ymm2,ymm6,[rcx]
	vminps ymm2,ymm6,ymm4
	vminps ymm2,ymm6,YMMWORD PTR [rcx]
	vminps ymm2,ymm6,[rcx]
	vmulpd ymm2,ymm6,ymm4
	vmulpd ymm2,ymm6,YMMWORD PTR [rcx]
	vmulpd ymm2,ymm6,[rcx]
	vmulps ymm2,ymm6,ymm4
	vmulps ymm2,ymm6,YMMWORD PTR [rcx]
	vmulps ymm2,ymm6,[rcx]
	vorpd ymm2,ymm6,ymm4
	vorpd ymm2,ymm6,YMMWORD PTR [rcx]
	vorpd ymm2,ymm6,[rcx]
	vorps ymm2,ymm6,ymm4
	vorps ymm2,ymm6,YMMWORD PTR [rcx]
	vorps ymm2,ymm6,[rcx]
	vpermilpd ymm2,ymm6,ymm4
	vpermilpd ymm2,ymm6,YMMWORD PTR [rcx]
	vpermilpd ymm2,ymm6,[rcx]
	vpermilps ymm2,ymm6,ymm4
	vpermilps ymm2,ymm6,YMMWORD PTR [rcx]
	vpermilps ymm2,ymm6,[rcx]
	vsubpd ymm2,ymm6,ymm4
	vsubpd ymm2,ymm6,YMMWORD PTR [rcx]
	vsubpd ymm2,ymm6,[rcx]
	vsubps ymm2,ymm6,ymm4
	vsubps ymm2,ymm6,YMMWORD PTR [rcx]
	vsubps ymm2,ymm6,[rcx]
	vunpckhpd ymm2,ymm6,ymm4
	vunpckhpd ymm2,ymm6,YMMWORD PTR [rcx]
	vunpckhpd ymm2,ymm6,[rcx]
	vunpckhps ymm2,ymm6,ymm4
	vunpckhps ymm2,ymm6,YMMWORD PTR [rcx]
	vunpckhps ymm2,ymm6,[rcx]
	vunpcklpd ymm2,ymm6,ymm4
	vunpcklpd ymm2,ymm6,YMMWORD PTR [rcx]
	vunpcklpd ymm2,ymm6,[rcx]
	vunpcklps ymm2,ymm6,ymm4
	vunpcklps ymm2,ymm6,YMMWORD PTR [rcx]
	vunpcklps ymm2,ymm6,[rcx]
	vxorpd ymm2,ymm6,ymm4
	vxorpd ymm2,ymm6,YMMWORD PTR [rcx]
	vxorpd ymm2,ymm6,[rcx]
	vxorps ymm2,ymm6,ymm4
	vxorps ymm2,ymm6,YMMWORD PTR [rcx]
	vxorps ymm2,ymm6,[rcx]
	vcmpeqpd ymm2,ymm6,ymm4
	vcmpeqpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpeqpd ymm2,ymm6,[rcx]
	vcmpltpd ymm2,ymm6,ymm4
	vcmpltpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpltpd ymm2,ymm6,[rcx]
	vcmplepd ymm2,ymm6,ymm4
	vcmplepd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmplepd ymm2,ymm6,[rcx]
	vcmpunordpd ymm2,ymm6,ymm4
	vcmpunordpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpunordpd ymm2,ymm6,[rcx]
	vcmpneqpd ymm2,ymm6,ymm4
	vcmpneqpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpneqpd ymm2,ymm6,[rcx]
	vcmpnltpd ymm2,ymm6,ymm4
	vcmpnltpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpnltpd ymm2,ymm6,[rcx]
	vcmpnlepd ymm2,ymm6,ymm4
	vcmpnlepd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpnlepd ymm2,ymm6,[rcx]
	vcmpordpd ymm2,ymm6,ymm4
	vcmpordpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpordpd ymm2,ymm6,[rcx]
	vcmpeq_uqpd ymm2,ymm6,ymm4
	vcmpeq_uqpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpeq_uqpd ymm2,ymm6,[rcx]
	vcmpngepd ymm2,ymm6,ymm4
	vcmpngepd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpngepd ymm2,ymm6,[rcx]
	vcmpngtpd ymm2,ymm6,ymm4
	vcmpngtpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpngtpd ymm2,ymm6,[rcx]
	vcmpfalsepd ymm2,ymm6,ymm4
	vcmpfalsepd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpfalsepd ymm2,ymm6,[rcx]
	vcmpneq_oqpd ymm2,ymm6,ymm4
	vcmpneq_oqpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpneq_oqpd ymm2,ymm6,[rcx]
	vcmpgepd ymm2,ymm6,ymm4
	vcmpgepd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpgepd ymm2,ymm6,[rcx]
	vcmpgtpd ymm2,ymm6,ymm4
	vcmpgtpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpgtpd ymm2,ymm6,[rcx]
	vcmptruepd ymm2,ymm6,ymm4
	vcmptruepd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmptruepd ymm2,ymm6,[rcx]
	vcmpeq_ospd ymm2,ymm6,ymm4
	vcmpeq_ospd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpeq_ospd ymm2,ymm6,[rcx]
	vcmplt_oqpd ymm2,ymm6,ymm4
	vcmplt_oqpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmplt_oqpd ymm2,ymm6,[rcx]
	vcmple_oqpd ymm2,ymm6,ymm4
	vcmple_oqpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmple_oqpd ymm2,ymm6,[rcx]
	vcmpunord_spd ymm2,ymm6,ymm4
	vcmpunord_spd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpunord_spd ymm2,ymm6,[rcx]
	vcmpneq_uspd ymm2,ymm6,ymm4
	vcmpneq_uspd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpneq_uspd ymm2,ymm6,[rcx]
	vcmpnlt_uqpd ymm2,ymm6,ymm4
	vcmpnlt_uqpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpnlt_uqpd ymm2,ymm6,[rcx]
	vcmpnle_uqpd ymm2,ymm6,ymm4
	vcmpnle_uqpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpnle_uqpd ymm2,ymm6,[rcx]
	vcmpord_spd ymm2,ymm6,ymm4
	vcmpord_spd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpord_spd ymm2,ymm6,[rcx]
	vcmpeq_uspd ymm2,ymm6,ymm4
	vcmpeq_uspd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpeq_uspd ymm2,ymm6,[rcx]
	vcmpnge_uqpd ymm2,ymm6,ymm4
	vcmpnge_uqpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpnge_uqpd ymm2,ymm6,[rcx]
	vcmpngt_uqpd ymm2,ymm6,ymm4
	vcmpngt_uqpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpngt_uqpd ymm2,ymm6,[rcx]
	vcmpfalse_ospd ymm2,ymm6,ymm4
	vcmpfalse_ospd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpfalse_ospd ymm2,ymm6,[rcx]
	vcmpneq_ospd ymm2,ymm6,ymm4
	vcmpneq_ospd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpneq_ospd ymm2,ymm6,[rcx]
	vcmpge_oqpd ymm2,ymm6,ymm4
	vcmpge_oqpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpge_oqpd ymm2,ymm6,[rcx]
	vcmpgt_oqpd ymm2,ymm6,ymm4
	vcmpgt_oqpd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpgt_oqpd ymm2,ymm6,[rcx]
	vcmptrue_uspd ymm2,ymm6,ymm4
	vcmptrue_uspd ymm2,ymm6,YMMWORD PTR [rcx]
	vcmptrue_uspd ymm2,ymm6,[rcx]
	vcmpeqps ymm2,ymm6,ymm4
	vcmpeqps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpeqps ymm2,ymm6,[rcx]
	vcmpltps ymm2,ymm6,ymm4
	vcmpltps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpltps ymm2,ymm6,[rcx]
	vcmpleps ymm2,ymm6,ymm4
	vcmpleps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpleps ymm2,ymm6,[rcx]
	vcmpunordps ymm2,ymm6,ymm4
	vcmpunordps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpunordps ymm2,ymm6,[rcx]
	vcmpneqps ymm2,ymm6,ymm4
	vcmpneqps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpneqps ymm2,ymm6,[rcx]
	vcmpnltps ymm2,ymm6,ymm4
	vcmpnltps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpnltps ymm2,ymm6,[rcx]
	vcmpnleps ymm2,ymm6,ymm4
	vcmpnleps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpnleps ymm2,ymm6,[rcx]
	vcmpordps ymm2,ymm6,ymm4
	vcmpordps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpordps ymm2,ymm6,[rcx]
	vcmpeq_uqps ymm2,ymm6,ymm4
	vcmpeq_uqps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpeq_uqps ymm2,ymm6,[rcx]
	vcmpngeps ymm2,ymm6,ymm4
	vcmpngeps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpngeps ymm2,ymm6,[rcx]
	vcmpngtps ymm2,ymm6,ymm4
	vcmpngtps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpngtps ymm2,ymm6,[rcx]
	vcmpfalseps ymm2,ymm6,ymm4
	vcmpfalseps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpfalseps ymm2,ymm6,[rcx]
	vcmpneq_oqps ymm2,ymm6,ymm4
	vcmpneq_oqps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpneq_oqps ymm2,ymm6,[rcx]
	vcmpgeps ymm2,ymm6,ymm4
	vcmpgeps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpgeps ymm2,ymm6,[rcx]
	vcmpgtps ymm2,ymm6,ymm4
	vcmpgtps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpgtps ymm2,ymm6,[rcx]
	vcmptrueps ymm2,ymm6,ymm4
	vcmptrueps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmptrueps ymm2,ymm6,[rcx]
	vcmpeq_osps ymm2,ymm6,ymm4
	vcmpeq_osps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpeq_osps ymm2,ymm6,[rcx]
	vcmplt_oqps ymm2,ymm6,ymm4
	vcmplt_oqps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmplt_oqps ymm2,ymm6,[rcx]
	vcmple_oqps ymm2,ymm6,ymm4
	vcmple_oqps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmple_oqps ymm2,ymm6,[rcx]
	vcmpunord_sps ymm2,ymm6,ymm4
	vcmpunord_sps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpunord_sps ymm2,ymm6,[rcx]
	vcmpneq_usps ymm2,ymm6,ymm4
	vcmpneq_usps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpneq_usps ymm2,ymm6,[rcx]
	vcmpnlt_uqps ymm2,ymm6,ymm4
	vcmpnlt_uqps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpnlt_uqps ymm2,ymm6,[rcx]
	vcmpnle_uqps ymm2,ymm6,ymm4
	vcmpnle_uqps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpnle_uqps ymm2,ymm6,[rcx]
	vcmpord_sps ymm2,ymm6,ymm4
	vcmpord_sps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpord_sps ymm2,ymm6,[rcx]
	vcmpeq_usps ymm2,ymm6,ymm4
	vcmpeq_usps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpeq_usps ymm2,ymm6,[rcx]
	vcmpnge_uqps ymm2,ymm6,ymm4
	vcmpnge_uqps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpnge_uqps ymm2,ymm6,[rcx]
	vcmpngt_uqps ymm2,ymm6,ymm4
	vcmpngt_uqps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpngt_uqps ymm2,ymm6,[rcx]
	vcmpfalse_osps ymm2,ymm6,ymm4
	vcmpfalse_osps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpfalse_osps ymm2,ymm6,[rcx]
	vcmpneq_osps ymm2,ymm6,ymm4
	vcmpneq_osps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpneq_osps ymm2,ymm6,[rcx]
	vcmpge_oqps ymm2,ymm6,ymm4
	vcmpge_oqps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpge_oqps ymm2,ymm6,[rcx]
	vcmpgt_oqps ymm2,ymm6,ymm4
	vcmpgt_oqps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmpgt_oqps ymm2,ymm6,[rcx]
	vcmptrue_usps ymm2,ymm6,ymm4
	vcmptrue_usps ymm2,ymm6,YMMWORD PTR [rcx]
	vcmptrue_usps ymm2,ymm6,[rcx]
    vgf2p8mulb ymm6, ymm5, ymm4
	vgf2p8mulb ymm6, ymm5, YMMWORD PTR [rcx]
	vgf2p8mulb ymm6, ymm5, [rcx]
	vgf2p8mulb ymm6, ymm5, YMMWORD PTR [rax+r14*8-123456]
	vgf2p8mulb ymm6, ymm5, YMMWORD PTR [rdx+4064]
	vgf2p8mulb ymm6, ymm5, YMMWORD PTR [rdx+4096]
	vgf2p8mulb ymm6, ymm5, YMMWORD PTR [rdx-4096]
	vgf2p8mulb ymm6, ymm5, YMMWORD PTR [rdx-4128]

# Tests for op ymm/mem256, xmm
	vcvtpd2dq xmm4,ymm4
	vcvtpd2dq xmm4,YMMWORD PTR [rcx]
	vcvtpd2ps xmm4,ymm4
	vcvtpd2ps xmm4,YMMWORD PTR [rcx]
	vcvttpd2dq xmm4,ymm4
	vcvttpd2dq xmm4,YMMWORD PTR [rcx]

# Tests for op ymm/mem256, ymm
	vcvtdq2ps ymm6,ymm4
	vcvtdq2ps ymm4,YMMWORD PTR [rcx]
	vcvtdq2ps ymm4,[rcx]
	vcvtps2dq ymm6,ymm4
	vcvtps2dq ymm4,YMMWORD PTR [rcx]
	vcvtps2dq ymm4,[rcx]
	vcvttps2dq ymm6,ymm4
	vcvttps2dq ymm4,YMMWORD PTR [rcx]
	vcvttps2dq ymm4,[rcx]
	vmovapd ymm6,ymm4
	vmovapd ymm4,YMMWORD PTR [rcx]
	vmovapd ymm4,[rcx]
	vmovaps ymm6,ymm4
	vmovaps ymm4,YMMWORD PTR [rcx]
	vmovaps ymm4,[rcx]
	vmovdqa ymm6,ymm4
	vmovdqa ymm4,YMMWORD PTR [rcx]
	vmovdqa ymm4,[rcx]
	vmovdqu ymm6,ymm4
	vmovdqu ymm4,YMMWORD PTR [rcx]
	vmovdqu ymm4,[rcx]
	vmovddup ymm6,ymm4
	vmovddup ymm4,YMMWORD PTR [rcx]
	vmovddup ymm4,[rcx]
	vmovshdup ymm6,ymm4
	vmovshdup ymm4,YMMWORD PTR [rcx]
	vmovshdup ymm4,[rcx]
	vmovsldup ymm6,ymm4
	vmovsldup ymm4,YMMWORD PTR [rcx]
	vmovsldup ymm4,[rcx]
	vmovupd ymm6,ymm4
	vmovupd ymm4,YMMWORD PTR [rcx]
	vmovupd ymm4,[rcx]
	vmovups ymm6,ymm4
	vmovups ymm4,YMMWORD PTR [rcx]
	vmovups ymm4,[rcx]
	vptest ymm6,ymm4
	vptest ymm4,YMMWORD PTR [rcx]
	vptest ymm4,[rcx]
	vrcpps ymm6,ymm4
	vrcpps ymm4,YMMWORD PTR [rcx]
	vrcpps ymm4,[rcx]
	vrsqrtps ymm6,ymm4
	vrsqrtps ymm4,YMMWORD PTR [rcx]
	vrsqrtps ymm4,[rcx]
	vsqrtpd ymm6,ymm4
	vsqrtpd ymm4,YMMWORD PTR [rcx]
	vsqrtpd ymm4,[rcx]
	vsqrtps ymm6,ymm4
	vsqrtps ymm4,YMMWORD PTR [rcx]
	vsqrtps ymm4,[rcx]
	vtestpd ymm6,ymm4
	vtestpd ymm4,YMMWORD PTR [rcx]
	vtestpd ymm4,[rcx]
	vtestps ymm6,ymm4
	vtestps ymm4,YMMWORD PTR [rcx]
	vtestps ymm4,[rcx]

# Tests for op ymm, ymm/mem256
	vmovapd ymm6,ymm4
	vmovapd YMMWORD PTR [rcx],ymm4
	vmovapd [rcx],ymm4
	vmovaps ymm6,ymm4
	vmovaps YMMWORD PTR [rcx],ymm4
	vmovaps [rcx],ymm4
	vmovdqa ymm6,ymm4
	vmovdqa YMMWORD PTR [rcx],ymm4
	vmovdqa [rcx],ymm4
	vmovdqu ymm6,ymm4
	vmovdqu YMMWORD PTR [rcx],ymm4
	vmovdqu [rcx],ymm4
	vmovupd ymm6,ymm4
	vmovupd YMMWORD PTR [rcx],ymm4
	vmovupd [rcx],ymm4
	vmovups ymm6,ymm4
	vmovups YMMWORD PTR [rcx],ymm4
	vmovups [rcx],ymm4

# Tests for op mem256, ymm
	vlddqu ymm4,YMMWORD PTR [rcx]
	vlddqu ymm4,[rcx]

# Tests for op ymm, mem256
	vmovntdq YMMWORD PTR [rcx],ymm4
	vmovntdq [rcx],ymm4
	vmovntpd YMMWORD PTR [rcx],ymm4
	vmovntpd [rcx],ymm4
	vmovntps YMMWORD PTR [rcx],ymm4
	vmovntps [rcx],ymm4

# Tests for op imm8, ymm/mem256, ymm, ymm
	vblendpd ymm2,ymm6,ymm4,7
	vblendpd ymm2,ymm6,YMMWORD PTR [rcx],7
	vblendpd ymm2,ymm6,[rcx],7
	vblendps ymm2,ymm6,ymm4,7
	vblendps ymm2,ymm6,YMMWORD PTR [rcx],7
	vblendps ymm2,ymm6,[rcx],7
	vcmppd ymm2,ymm6,ymm4,7
	vcmppd ymm2,ymm6,YMMWORD PTR [rcx],7
	vcmppd ymm2,ymm6,[rcx],7
	vcmpps ymm2,ymm6,ymm4,7
	vcmpps ymm2,ymm6,YMMWORD PTR [rcx],7
	vcmpps ymm2,ymm6,[rcx],7
	vdpps ymm2,ymm6,ymm4,7
	vdpps ymm2,ymm6,YMMWORD PTR [rcx],7
	vdpps ymm2,ymm6,[rcx],7
	vperm2f128 ymm2,ymm6,ymm4,7
	vperm2f128 ymm2,ymm6,YMMWORD PTR [rcx],7
	vperm2f128 ymm2,ymm6,[rcx],7
	vshufpd ymm2,ymm6,ymm4,7
	vshufpd ymm2,ymm6,YMMWORD PTR [rcx],7
	vshufpd ymm2,ymm6,[rcx],7
	vshufps ymm2,ymm6,ymm4,7
	vshufps ymm2,ymm6,YMMWORD PTR [rcx],7
	vshufps ymm2,ymm6,[rcx],7
    vgf2p8affineqb ymm6, ymm5, ymm4, 0xab
	vgf2p8affineqb ymm6, ymm5, ymm4, 123
	vgf2p8affineqb ymm6, ymm5, YMMWORD PTR [rcx], 123
	vgf2p8affineqb ymm6, ymm5, [rcx], 123
	vgf2p8affineqb ymm6, ymm5, YMMWORD PTR [rax+r14*8-123456], 123
	vgf2p8affineqb ymm6, ymm5, YMMWORD PTR [rdx+4064], 123
	vgf2p8affineqb ymm6, ymm5, YMMWORD PTR [rdx+4096], 123
	vgf2p8affineqb ymm6, ymm5, YMMWORD PTR [rdx-4096], 123
	vgf2p8affineqb ymm6, ymm5, YMMWORD PTR [rdx-4128], 123
	vgf2p8affineinvqb ymm6, ymm5, ymm4, 0xab
	vgf2p8affineinvqb ymm6, ymm5, ymm4, 123
	vgf2p8affineinvqb ymm6, ymm5, YMMWORD PTR [rcx], 123
	vgf2p8affineinvqb ymm6, ymm5, [rcx], 123
	vgf2p8affineinvqb ymm6, ymm5, YMMWORD PTR [rax+r14*8-123456], 123
	vgf2p8affineinvqb ymm6, ymm5, YMMWORD PTR [rdx+4064], 123
	vgf2p8affineinvqb ymm6, ymm5, YMMWORD PTR [rdx+4096], 123
	vgf2p8affineinvqb ymm6, ymm5, YMMWORD PTR [rdx-4096], 123
	vgf2p8affineinvqb ymm6, ymm5, YMMWORD PTR [rdx-4128], 123

# Tests for op ymm, ymm/mem256, ymm, ymm
	vblendvpd ymm7,ymm2,ymm6,ymm4
	vblendvpd ymm7,ymm2,YMMWORD PTR [rcx],ymm4
	vblendvpd ymm7,ymm2,[rcx],ymm4
	vblendvps ymm7,ymm2,ymm6,ymm4
	vblendvps ymm7,ymm2,YMMWORD PTR [rcx],ymm4
	vblendvps ymm7,ymm2,[rcx],ymm4

# Tests for op imm8, xmm/mem128, ymm, ymm
	vinsertf128 ymm6,ymm4,xmm4,7
	vinsertf128 ymm6,ymm4,XMMWORD PTR [rcx],7
	vinsertf128 ymm6,ymm4,[rcx],7

# Tests for op imm8, ymm, xmm/mem128
	vextractf128 xmm4,ymm4,7
	vextractf128 XMMWORD PTR [rcx],ymm4,7
	vextractf128 [rcx],ymm4,7

# Tests for op mem128, ymm
	vbroadcastf128 ymm4,XMMWORD PTR [rcx]
	vbroadcastf128 ymm4,[rcx]

# Tests for op xmm/mem128, xmm
	vcvtdq2ps xmm6,xmm4
	vcvtdq2ps xmm4,XMMWORD PTR [rcx]
	vcvtdq2ps xmm4,[rcx]
	vcvtpd2dq xmm6,xmm4
	vcvtpd2dq xmm4,XMMWORD PTR [rcx]
	vcvtpd2ps xmm6,xmm4
	vcvtpd2ps xmm4,XMMWORD PTR [rcx]
	vcvtps2dq xmm6,xmm4
	vcvtps2dq xmm4,XMMWORD PTR [rcx]
	vcvtps2dq xmm4,[rcx]
	vcvttpd2dq xmm6,xmm4
	vcvttpd2dq xmm4,XMMWORD PTR [rcx]
	vcvttps2dq xmm6,xmm4
	vcvttps2dq xmm4,XMMWORD PTR [rcx]
	vcvttps2dq xmm4,[rcx]
	vmovapd xmm6,xmm4
	vmovapd xmm4,XMMWORD PTR [rcx]
	vmovapd xmm4,[rcx]
	vmovaps xmm6,xmm4
	vmovaps xmm4,XMMWORD PTR [rcx]
	vmovaps xmm4,[rcx]
	vmovdqa xmm6,xmm4
	vmovdqa xmm4,XMMWORD PTR [rcx]
	vmovdqa xmm4,[rcx]
	vmovdqu xmm6,xmm4
	vmovdqu xmm4,XMMWORD PTR [rcx]
	vmovdqu xmm4,[rcx]
	vmovshdup xmm6,xmm4
	vmovshdup xmm4,XMMWORD PTR [rcx]
	vmovshdup xmm4,[rcx]
	vmovsldup xmm6,xmm4
	vmovsldup xmm4,XMMWORD PTR [rcx]
	vmovsldup xmm4,[rcx]
	vmovupd xmm6,xmm4
	vmovupd xmm4,XMMWORD PTR [rcx]
	vmovupd xmm4,[rcx]
	vmovups xmm6,xmm4
	vmovups xmm4,XMMWORD PTR [rcx]
	vmovups xmm4,[rcx]
	vpabsb xmm6,xmm4
	vpabsb xmm4,XMMWORD PTR [rcx]
	vpabsb xmm4,[rcx]
	vpabsw xmm6,xmm4
	vpabsw xmm4,XMMWORD PTR [rcx]
	vpabsw xmm4,[rcx]
	vpabsd xmm6,xmm4
	vpabsd xmm4,XMMWORD PTR [rcx]
	vpabsd xmm4,[rcx]
	vphminposuw xmm6,xmm4
	vphminposuw xmm4,XMMWORD PTR [rcx]
	vphminposuw xmm4,[rcx]
	vptest xmm6,xmm4
	vptest xmm4,XMMWORD PTR [rcx]
	vptest xmm4,[rcx]
	vtestps xmm6,xmm4
	vtestps xmm4,XMMWORD PTR [rcx]
	vtestps xmm4,[rcx]
	vtestpd xmm6,xmm4
	vtestpd xmm4,XMMWORD PTR [rcx]
	vtestpd xmm4,[rcx]
	vrcpps xmm6,xmm4
	vrcpps xmm4,XMMWORD PTR [rcx]
	vrcpps xmm4,[rcx]
	vrsqrtps xmm6,xmm4
	vrsqrtps xmm4,XMMWORD PTR [rcx]
	vrsqrtps xmm4,[rcx]
	vsqrtpd xmm6,xmm4
	vsqrtpd xmm4,XMMWORD PTR [rcx]
	vsqrtpd xmm4,[rcx]
	vsqrtps xmm6,xmm4
	vsqrtps xmm4,XMMWORD PTR [rcx]
	vsqrtps xmm4,[rcx]
	vaesimc xmm6,xmm4
	vaesimc xmm4,XMMWORD PTR [rcx]
	vaesimc xmm4,[rcx]

# Tests for op xmm, xmm/mem128
	vmovapd xmm6,xmm4
	vmovapd XMMWORD PTR [rcx],xmm4
	vmovapd [rcx],xmm4
	vmovaps xmm6,xmm4
	vmovaps XMMWORD PTR [rcx],xmm4
	vmovaps [rcx],xmm4
	vmovdqa xmm6,xmm4
	vmovdqa XMMWORD PTR [rcx],xmm4
	vmovdqa [rcx],xmm4
	vmovdqu xmm6,xmm4
	vmovdqu XMMWORD PTR [rcx],xmm4
	vmovdqu [rcx],xmm4
	vmovupd xmm6,xmm4
	vmovupd XMMWORD PTR [rcx],xmm4
	vmovupd [rcx],xmm4
	vmovups xmm6,xmm4
	vmovups XMMWORD PTR [rcx],xmm4
	vmovups [rcx],xmm4

# Tests for op mem128, xmm
	vlddqu xmm4,XMMWORD PTR [rcx]
	vlddqu xmm4,[rcx]
	vmovntdqa xmm4,XMMWORD PTR [rcx]
	vmovntdqa xmm4,[rcx]

# Tests for op xmm, mem128
	vmovntdq XMMWORD PTR [rcx],xmm4
	vmovntdq [rcx],xmm4
	vmovntpd XMMWORD PTR [rcx],xmm4
	vmovntpd [rcx],xmm4
	vmovntps XMMWORD PTR [rcx],xmm4
	vmovntps [rcx],xmm4

# Tests for op xmm/mem128, ymm
	vcvtdq2pd ymm4,xmm4
	vcvtdq2pd ymm4,XMMWORD PTR [rcx]
	vcvtdq2pd ymm4,[rcx]
	vcvtps2pd ymm4,xmm4
	vcvtps2pd ymm4,XMMWORD PTR [rcx]
	vcvtps2pd ymm4,[rcx]

# Tests for op xmm/mem128, xmm, xmm
	vaddpd xmm2,xmm6,xmm4
	vaddpd xmm7,xmm6,XMMWORD PTR [rcx]
	vaddpd xmm7,xmm6,[rcx]
	vaddps xmm2,xmm6,xmm4
	vaddps xmm7,xmm6,XMMWORD PTR [rcx]
	vaddps xmm7,xmm6,[rcx]
	vaddsubpd xmm2,xmm6,xmm4
	vaddsubpd xmm7,xmm6,XMMWORD PTR [rcx]
	vaddsubpd xmm7,xmm6,[rcx]
	vaddsubps xmm2,xmm6,xmm4
	vaddsubps xmm7,xmm6,XMMWORD PTR [rcx]
	vaddsubps xmm7,xmm6,[rcx]
	vandnpd xmm2,xmm6,xmm4
	vandnpd xmm7,xmm6,XMMWORD PTR [rcx]
	vandnpd xmm7,xmm6,[rcx]
	vandnps xmm2,xmm6,xmm4
	vandnps xmm7,xmm6,XMMWORD PTR [rcx]
	vandnps xmm7,xmm6,[rcx]
	vandpd xmm2,xmm6,xmm4
	vandpd xmm7,xmm6,XMMWORD PTR [rcx]
	vandpd xmm7,xmm6,[rcx]
	vandps xmm2,xmm6,xmm4
	vandps xmm7,xmm6,XMMWORD PTR [rcx]
	vandps xmm7,xmm6,[rcx]
	vdivpd xmm2,xmm6,xmm4
	vdivpd xmm7,xmm6,XMMWORD PTR [rcx]
	vdivpd xmm7,xmm6,[rcx]
	vdivps xmm2,xmm6,xmm4
	vdivps xmm7,xmm6,XMMWORD PTR [rcx]
	vdivps xmm7,xmm6,[rcx]
	vhaddpd xmm2,xmm6,xmm4
	vhaddpd xmm7,xmm6,XMMWORD PTR [rcx]
	vhaddpd xmm7,xmm6,[rcx]
	vhaddps xmm2,xmm6,xmm4
	vhaddps xmm7,xmm6,XMMWORD PTR [rcx]
	vhaddps xmm7,xmm6,[rcx]
	vhsubpd xmm2,xmm6,xmm4
	vhsubpd xmm7,xmm6,XMMWORD PTR [rcx]
	vhsubpd xmm7,xmm6,[rcx]
	vhsubps xmm2,xmm6,xmm4
	vhsubps xmm7,xmm6,XMMWORD PTR [rcx]
	vhsubps xmm7,xmm6,[rcx]
	vmaxpd xmm2,xmm6,xmm4
	vmaxpd xmm7,xmm6,XMMWORD PTR [rcx]
	vmaxpd xmm7,xmm6,[rcx]
	vmaxps xmm2,xmm6,xmm4
	vmaxps xmm7,xmm6,XMMWORD PTR [rcx]
	vmaxps xmm7,xmm6,[rcx]
	vminpd xmm2,xmm6,xmm4
	vminpd xmm7,xmm6,XMMWORD PTR [rcx]
	vminpd xmm7,xmm6,[rcx]
	vminps xmm2,xmm6,xmm4
	vminps xmm7,xmm6,XMMWORD PTR [rcx]
	vminps xmm7,xmm6,[rcx]
	vmulpd xmm2,xmm6,xmm4
	vmulpd xmm7,xmm6,XMMWORD PTR [rcx]
	vmulpd xmm7,xmm6,[rcx]
	vmulps xmm2,xmm6,xmm4
	vmulps xmm7,xmm6,XMMWORD PTR [rcx]
	vmulps xmm7,xmm6,[rcx]
	vorpd xmm2,xmm6,xmm4
	vorpd xmm7,xmm6,XMMWORD PTR [rcx]
	vorpd xmm7,xmm6,[rcx]
	vorps xmm2,xmm6,xmm4
	vorps xmm7,xmm6,XMMWORD PTR [rcx]
	vorps xmm7,xmm6,[rcx]
	vpacksswb xmm2,xmm6,xmm4
	vpacksswb xmm7,xmm6,XMMWORD PTR [rcx]
	vpacksswb xmm7,xmm6,[rcx]
	vpackssdw xmm2,xmm6,xmm4
	vpackssdw xmm7,xmm6,XMMWORD PTR [rcx]
	vpackssdw xmm7,xmm6,[rcx]
	vpackuswb xmm2,xmm6,xmm4
	vpackuswb xmm7,xmm6,XMMWORD PTR [rcx]
	vpackuswb xmm7,xmm6,[rcx]
	vpackusdw xmm2,xmm6,xmm4
	vpackusdw xmm7,xmm6,XMMWORD PTR [rcx]
	vpackusdw xmm7,xmm6,[rcx]
	vpaddb xmm2,xmm6,xmm4
	vpaddb xmm7,xmm6,XMMWORD PTR [rcx]
	vpaddb xmm7,xmm6,[rcx]
	vpaddw xmm2,xmm6,xmm4
	vpaddw xmm7,xmm6,XMMWORD PTR [rcx]
	vpaddw xmm7,xmm6,[rcx]
	vpaddd xmm2,xmm6,xmm4
	vpaddd xmm7,xmm6,XMMWORD PTR [rcx]
	vpaddd xmm7,xmm6,[rcx]
	vpaddq xmm2,xmm6,xmm4
	vpaddq xmm7,xmm6,XMMWORD PTR [rcx]
	vpaddq xmm7,xmm6,[rcx]
	vpaddsb xmm2,xmm6,xmm4
	vpaddsb xmm7,xmm6,XMMWORD PTR [rcx]
	vpaddsb xmm7,xmm6,[rcx]
	vpaddsw xmm2,xmm6,xmm4
	vpaddsw xmm7,xmm6,XMMWORD PTR [rcx]
	vpaddsw xmm7,xmm6,[rcx]
	vpaddusb xmm2,xmm6,xmm4
	vpaddusb xmm7,xmm6,XMMWORD PTR [rcx]
	vpaddusb xmm7,xmm6,[rcx]
	vpaddusw xmm2,xmm6,xmm4
	vpaddusw xmm7,xmm6,XMMWORD PTR [rcx]
	vpaddusw xmm7,xmm6,[rcx]
	vpand xmm2,xmm6,xmm4
	vpand xmm7,xmm6,XMMWORD PTR [rcx]
	vpand xmm7,xmm6,[rcx]
	vpandn xmm2,xmm6,xmm4
	vpandn xmm7,xmm6,XMMWORD PTR [rcx]
	vpandn xmm7,xmm6,[rcx]
	vpavgb xmm2,xmm6,xmm4
	vpavgb xmm7,xmm6,XMMWORD PTR [rcx]
	vpavgb xmm7,xmm6,[rcx]
	vpavgw xmm2,xmm6,xmm4
	vpavgw xmm7,xmm6,XMMWORD PTR [rcx]
	vpavgw xmm7,xmm6,[rcx]
	vpclmullqlqdq xmm2,xmm6,xmm4
	vpclmullqlqdq xmm7,xmm6,XMMWORD PTR [rcx]
	vpclmullqlqdq xmm7,xmm6,[rcx]
	vpclmulhqlqdq xmm2,xmm6,xmm4
	vpclmulhqlqdq xmm7,xmm6,XMMWORD PTR [rcx]
	vpclmulhqlqdq xmm7,xmm6,[rcx]
	vpclmullqhqdq xmm2,xmm6,xmm4
	vpclmullqhqdq xmm7,xmm6,XMMWORD PTR [rcx]
	vpclmullqhqdq xmm7,xmm6,[rcx]
	vpclmulhqhqdq xmm2,xmm6,xmm4
	vpclmulhqhqdq xmm7,xmm6,XMMWORD PTR [rcx]
	vpclmulhqhqdq xmm7,xmm6,[rcx]
	vpcmpeqb xmm2,xmm6,xmm4
	vpcmpeqb xmm7,xmm6,XMMWORD PTR [rcx]
	vpcmpeqb xmm7,xmm6,[rcx]
	vpcmpeqw xmm2,xmm6,xmm4
	vpcmpeqw xmm7,xmm6,XMMWORD PTR [rcx]
	vpcmpeqw xmm7,xmm6,[rcx]
	vpcmpeqd xmm2,xmm6,xmm4
	vpcmpeqd xmm7,xmm6,XMMWORD PTR [rcx]
	vpcmpeqd xmm7,xmm6,[rcx]
	vpcmpeqq xmm2,xmm6,xmm4
	vpcmpeqq xmm7,xmm6,XMMWORD PTR [rcx]
	vpcmpeqq xmm7,xmm6,[rcx]
	vpcmpgtb xmm2,xmm6,xmm4
	vpcmpgtb xmm7,xmm6,XMMWORD PTR [rcx]
	vpcmpgtb xmm7,xmm6,[rcx]
	vpcmpgtw xmm2,xmm6,xmm4
	vpcmpgtw xmm7,xmm6,XMMWORD PTR [rcx]
	vpcmpgtw xmm7,xmm6,[rcx]
	vpcmpgtd xmm2,xmm6,xmm4
	vpcmpgtd xmm7,xmm6,XMMWORD PTR [rcx]
	vpcmpgtd xmm7,xmm6,[rcx]
	vpcmpgtq xmm2,xmm6,xmm4
	vpcmpgtq xmm7,xmm6,XMMWORD PTR [rcx]
	vpcmpgtq xmm7,xmm6,[rcx]
	vpermilpd xmm2,xmm6,xmm4
	vpermilpd xmm7,xmm6,XMMWORD PTR [rcx]
	vpermilpd xmm7,xmm6,[rcx]
	vpermilps xmm2,xmm6,xmm4
	vpermilps xmm7,xmm6,XMMWORD PTR [rcx]
	vpermilps xmm7,xmm6,[rcx]
	vphaddw xmm2,xmm6,xmm4
	vphaddw xmm7,xmm6,XMMWORD PTR [rcx]
	vphaddw xmm7,xmm6,[rcx]
	vphaddd xmm2,xmm6,xmm4
	vphaddd xmm7,xmm6,XMMWORD PTR [rcx]
	vphaddd xmm7,xmm6,[rcx]
	vphaddsw xmm2,xmm6,xmm4
	vphaddsw xmm7,xmm6,XMMWORD PTR [rcx]
	vphaddsw xmm7,xmm6,[rcx]
	vphsubw xmm2,xmm6,xmm4
	vphsubw xmm7,xmm6,XMMWORD PTR [rcx]
	vphsubw xmm7,xmm6,[rcx]
	vphsubd xmm2,xmm6,xmm4
	vphsubd xmm7,xmm6,XMMWORD PTR [rcx]
	vphsubd xmm7,xmm6,[rcx]
	vphsubsw xmm2,xmm6,xmm4
	vphsubsw xmm7,xmm6,XMMWORD PTR [rcx]
	vphsubsw xmm7,xmm6,[rcx]
	vpmaddwd xmm2,xmm6,xmm4
	vpmaddwd xmm7,xmm6,XMMWORD PTR [rcx]
	vpmaddwd xmm7,xmm6,[rcx]
	vpmaddubsw xmm2,xmm6,xmm4
	vpmaddubsw xmm7,xmm6,XMMWORD PTR [rcx]
	vpmaddubsw xmm7,xmm6,[rcx]
	vpmaxsb xmm2,xmm6,xmm4
	vpmaxsb xmm7,xmm6,XMMWORD PTR [rcx]
	vpmaxsb xmm7,xmm6,[rcx]
	vpmaxsw xmm2,xmm6,xmm4
	vpmaxsw xmm7,xmm6,XMMWORD PTR [rcx]
	vpmaxsw xmm7,xmm6,[rcx]
	vpmaxsd xmm2,xmm6,xmm4
	vpmaxsd xmm7,xmm6,XMMWORD PTR [rcx]
	vpmaxsd xmm7,xmm6,[rcx]
	vpmaxub xmm2,xmm6,xmm4
	vpmaxub xmm7,xmm6,XMMWORD PTR [rcx]
	vpmaxub xmm7,xmm6,[rcx]
	vpmaxuw xmm2,xmm6,xmm4
	vpmaxuw xmm7,xmm6,XMMWORD PTR [rcx]
	vpmaxuw xmm7,xmm6,[rcx]
	vpmaxud xmm2,xmm6,xmm4
	vpmaxud xmm7,xmm6,XMMWORD PTR [rcx]
	vpmaxud xmm7,xmm6,[rcx]
	vpminsb xmm2,xmm6,xmm4
	vpminsb xmm7,xmm6,XMMWORD PTR [rcx]
	vpminsb xmm7,xmm6,[rcx]
	vpminsw xmm2,xmm6,xmm4
	vpminsw xmm7,xmm6,XMMWORD PTR [rcx]
	vpminsw xmm7,xmm6,[rcx]
	vpminsd xmm2,xmm6,xmm4
	vpminsd xmm7,xmm6,XMMWORD PTR [rcx]
	vpminsd xmm7,xmm6,[rcx]
	vpminub xmm2,xmm6,xmm4
	vpminub xmm7,xmm6,XMMWORD PTR [rcx]
	vpminub xmm7,xmm6,[rcx]
	vpminuw xmm2,xmm6,xmm4
	vpminuw xmm7,xmm6,XMMWORD PTR [rcx]
	vpminuw xmm7,xmm6,[rcx]
	vpminud xmm2,xmm6,xmm4
	vpminud xmm7,xmm6,XMMWORD PTR [rcx]
	vpminud xmm7,xmm6,[rcx]
	vpmulhuw xmm2,xmm6,xmm4
	vpmulhuw xmm7,xmm6,XMMWORD PTR [rcx]
	vpmulhuw xmm7,xmm6,[rcx]
	vpmulhrsw xmm2,xmm6,xmm4
	vpmulhrsw xmm7,xmm6,XMMWORD PTR [rcx]
	vpmulhrsw xmm7,xmm6,[rcx]
	vpmulhw xmm2,xmm6,xmm4
	vpmulhw xmm7,xmm6,XMMWORD PTR [rcx]
	vpmulhw xmm7,xmm6,[rcx]
	vpmullw xmm2,xmm6,xmm4
	vpmullw xmm7,xmm6,XMMWORD PTR [rcx]
	vpmullw xmm7,xmm6,[rcx]
	vpmulld xmm2,xmm6,xmm4
	vpmulld xmm7,xmm6,XMMWORD PTR [rcx]
	vpmulld xmm7,xmm6,[rcx]
	vpmuludq xmm2,xmm6,xmm4
	vpmuludq xmm7,xmm6,XMMWORD PTR [rcx]
	vpmuludq xmm7,xmm6,[rcx]
	vpmuldq xmm2,xmm6,xmm4
	vpmuldq xmm7,xmm6,XMMWORD PTR [rcx]
	vpmuldq xmm7,xmm6,[rcx]
	vpor xmm2,xmm6,xmm4
	vpor xmm7,xmm6,XMMWORD PTR [rcx]
	vpor xmm7,xmm6,[rcx]
	vpsadbw xmm2,xmm6,xmm4
	vpsadbw xmm7,xmm6,XMMWORD PTR [rcx]
	vpsadbw xmm7,xmm6,[rcx]
	vpshufb xmm2,xmm6,xmm4
	vpshufb xmm7,xmm6,XMMWORD PTR [rcx]
	vpshufb xmm7,xmm6,[rcx]
	vpsignb xmm2,xmm6,xmm4
	vpsignb xmm7,xmm6,XMMWORD PTR [rcx]
	vpsignb xmm7,xmm6,[rcx]
	vpsignw xmm2,xmm6,xmm4
	vpsignw xmm7,xmm6,XMMWORD PTR [rcx]
	vpsignw xmm7,xmm6,[rcx]
	vpsignd xmm2,xmm6,xmm4
	vpsignd xmm7,xmm6,XMMWORD PTR [rcx]
	vpsignd xmm7,xmm6,[rcx]
	vpsllw xmm2,xmm6,xmm4
	vpsllw xmm7,xmm6,XMMWORD PTR [rcx]
	vpsllw xmm7,xmm6,[rcx]
	vpslld xmm2,xmm6,xmm4
	vpslld xmm7,xmm6,XMMWORD PTR [rcx]
	vpslld xmm7,xmm6,[rcx]
	vpsllq xmm2,xmm6,xmm4
	vpsllq xmm7,xmm6,XMMWORD PTR [rcx]
	vpsllq xmm7,xmm6,[rcx]
	vpsraw xmm2,xmm6,xmm4
	vpsraw xmm7,xmm6,XMMWORD PTR [rcx]
	vpsraw xmm7,xmm6,[rcx]
	vpsrad xmm2,xmm6,xmm4
	vpsrad xmm7,xmm6,XMMWORD PTR [rcx]
	vpsrad xmm7,xmm6,[rcx]
	vpsrlw xmm2,xmm6,xmm4
	vpsrlw xmm7,xmm6,XMMWORD PTR [rcx]
	vpsrlw xmm7,xmm6,[rcx]
	vpsrld xmm2,xmm6,xmm4
	vpsrld xmm7,xmm6,XMMWORD PTR [rcx]
	vpsrld xmm7,xmm6,[rcx]
	vpsrlq xmm2,xmm6,xmm4
	vpsrlq xmm7,xmm6,XMMWORD PTR [rcx]
	vpsrlq xmm7,xmm6,[rcx]
	vpsubb xmm2,xmm6,xmm4
	vpsubb xmm7,xmm6,XMMWORD PTR [rcx]
	vpsubb xmm7,xmm6,[rcx]
	vpsubw xmm2,xmm6,xmm4
	vpsubw xmm7,xmm6,XMMWORD PTR [rcx]
	vpsubw xmm7,xmm6,[rcx]
	vpsubd xmm2,xmm6,xmm4
	vpsubd xmm7,xmm6,XMMWORD PTR [rcx]
	vpsubd xmm7,xmm6,[rcx]
	vpsubq xmm2,xmm6,xmm4
	vpsubq xmm7,xmm6,XMMWORD PTR [rcx]
	vpsubq xmm7,xmm6,[rcx]
	vpsubsb xmm2,xmm6,xmm4
	vpsubsb xmm7,xmm6,XMMWORD PTR [rcx]
	vpsubsb xmm7,xmm6,[rcx]
	vpsubsw xmm2,xmm6,xmm4
	vpsubsw xmm7,xmm6,XMMWORD PTR [rcx]
	vpsubsw xmm7,xmm6,[rcx]
	vpsubusb xmm2,xmm6,xmm4
	vpsubusb xmm7,xmm6,XMMWORD PTR [rcx]
	vpsubusb xmm7,xmm6,[rcx]
	vpsubusw xmm2,xmm6,xmm4
	vpsubusw xmm7,xmm6,XMMWORD PTR [rcx]
	vpsubusw xmm7,xmm6,[rcx]
	vpunpckhbw xmm2,xmm6,xmm4
	vpunpckhbw xmm7,xmm6,XMMWORD PTR [rcx]
	vpunpckhbw xmm7,xmm6,[rcx]
	vpunpckhwd xmm2,xmm6,xmm4
	vpunpckhwd xmm7,xmm6,XMMWORD PTR [rcx]
	vpunpckhwd xmm7,xmm6,[rcx]
	vpunpckhdq xmm2,xmm6,xmm4
	vpunpckhdq xmm7,xmm6,XMMWORD PTR [rcx]
	vpunpckhdq xmm7,xmm6,[rcx]
	vpunpckhqdq xmm2,xmm6,xmm4
	vpunpckhqdq xmm7,xmm6,XMMWORD PTR [rcx]
	vpunpckhqdq xmm7,xmm6,[rcx]
	vpunpcklbw xmm2,xmm6,xmm4
	vpunpcklbw xmm7,xmm6,XMMWORD PTR [rcx]
	vpunpcklbw xmm7,xmm6,[rcx]
	vpunpcklwd xmm2,xmm6,xmm4
	vpunpcklwd xmm7,xmm6,XMMWORD PTR [rcx]
	vpunpcklwd xmm7,xmm6,[rcx]
	vpunpckldq xmm2,xmm6,xmm4
	vpunpckldq xmm7,xmm6,XMMWORD PTR [rcx]
	vpunpckldq xmm7,xmm6,[rcx]
	vpunpcklqdq xmm2,xmm6,xmm4
	vpunpcklqdq xmm7,xmm6,XMMWORD PTR [rcx]
	vpunpcklqdq xmm7,xmm6,[rcx]
	vpxor xmm2,xmm6,xmm4
	vpxor xmm7,xmm6,XMMWORD PTR [rcx]
	vpxor xmm7,xmm6,[rcx]
	vsubpd xmm2,xmm6,xmm4
	vsubpd xmm7,xmm6,XMMWORD PTR [rcx]
	vsubpd xmm7,xmm6,[rcx]
	vsubps xmm2,xmm6,xmm4
	vsubps xmm7,xmm6,XMMWORD PTR [rcx]
	vsubps xmm7,xmm6,[rcx]
	vunpckhpd xmm2,xmm6,xmm4
	vunpckhpd xmm7,xmm6,XMMWORD PTR [rcx]
	vunpckhpd xmm7,xmm6,[rcx]
	vunpckhps xmm2,xmm6,xmm4
	vunpckhps xmm7,xmm6,XMMWORD PTR [rcx]
	vunpckhps xmm7,xmm6,[rcx]
	vunpcklpd xmm2,xmm6,xmm4
	vunpcklpd xmm7,xmm6,XMMWORD PTR [rcx]
	vunpcklpd xmm7,xmm6,[rcx]
	vunpcklps xmm2,xmm6,xmm4
	vunpcklps xmm7,xmm6,XMMWORD PTR [rcx]
	vunpcklps xmm7,xmm6,[rcx]
	vxorpd xmm2,xmm6,xmm4
	vxorpd xmm7,xmm6,XMMWORD PTR [rcx]
	vxorpd xmm7,xmm6,[rcx]
	vxorps xmm2,xmm6,xmm4
	vxorps xmm7,xmm6,XMMWORD PTR [rcx]
	vxorps xmm7,xmm6,[rcx]
	vaesenc xmm2,xmm6,xmm4
	vaesenc xmm7,xmm6,XMMWORD PTR [rcx]
	vaesenc xmm7,xmm6,[rcx]
	vaesenclast xmm2,xmm6,xmm4
	vaesenclast xmm7,xmm6,XMMWORD PTR [rcx]
	vaesenclast xmm7,xmm6,[rcx]
	vaesdec xmm2,xmm6,xmm4
	vaesdec xmm7,xmm6,XMMWORD PTR [rcx]
	vaesdec xmm7,xmm6,[rcx]
	vaesdeclast xmm2,xmm6,xmm4
	vaesdeclast xmm7,xmm6,XMMWORD PTR [rcx]
	vaesdeclast xmm7,xmm6,[rcx]
	vcmpeqpd xmm2,xmm6,xmm4
	vcmpeqpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpeqpd xmm7,xmm6,[rcx]
	vcmpltpd xmm2,xmm6,xmm4
	vcmpltpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpltpd xmm7,xmm6,[rcx]
	vcmplepd xmm2,xmm6,xmm4
	vcmplepd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmplepd xmm7,xmm6,[rcx]
	vcmpunordpd xmm2,xmm6,xmm4
	vcmpunordpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpunordpd xmm7,xmm6,[rcx]
	vcmpneqpd xmm2,xmm6,xmm4
	vcmpneqpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpneqpd xmm7,xmm6,[rcx]
	vcmpnltpd xmm2,xmm6,xmm4
	vcmpnltpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpnltpd xmm7,xmm6,[rcx]
	vcmpnlepd xmm2,xmm6,xmm4
	vcmpnlepd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpnlepd xmm7,xmm6,[rcx]
	vcmpordpd xmm2,xmm6,xmm4
	vcmpordpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpordpd xmm7,xmm6,[rcx]
	vcmpeq_uqpd xmm2,xmm6,xmm4
	vcmpeq_uqpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpeq_uqpd xmm7,xmm6,[rcx]
	vcmpngepd xmm2,xmm6,xmm4
	vcmpngepd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpngepd xmm7,xmm6,[rcx]
	vcmpngtpd xmm2,xmm6,xmm4
	vcmpngtpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpngtpd xmm7,xmm6,[rcx]
	vcmpfalsepd xmm2,xmm6,xmm4
	vcmpfalsepd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpfalsepd xmm7,xmm6,[rcx]
	vcmpneq_oqpd xmm2,xmm6,xmm4
	vcmpneq_oqpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpneq_oqpd xmm7,xmm6,[rcx]
	vcmpgepd xmm2,xmm6,xmm4
	vcmpgepd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpgepd xmm7,xmm6,[rcx]
	vcmpgtpd xmm2,xmm6,xmm4
	vcmpgtpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpgtpd xmm7,xmm6,[rcx]
	vcmptruepd xmm2,xmm6,xmm4
	vcmptruepd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmptruepd xmm7,xmm6,[rcx]
	vcmpeq_ospd xmm2,xmm6,xmm4
	vcmpeq_ospd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpeq_ospd xmm7,xmm6,[rcx]
	vcmplt_oqpd xmm2,xmm6,xmm4
	vcmplt_oqpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmplt_oqpd xmm7,xmm6,[rcx]
	vcmple_oqpd xmm2,xmm6,xmm4
	vcmple_oqpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmple_oqpd xmm7,xmm6,[rcx]
	vcmpunord_spd xmm2,xmm6,xmm4
	vcmpunord_spd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpunord_spd xmm7,xmm6,[rcx]
	vcmpneq_uspd xmm2,xmm6,xmm4
	vcmpneq_uspd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpneq_uspd xmm7,xmm6,[rcx]
	vcmpnlt_uqpd xmm2,xmm6,xmm4
	vcmpnlt_uqpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpnlt_uqpd xmm7,xmm6,[rcx]
	vcmpnle_uqpd xmm2,xmm6,xmm4
	vcmpnle_uqpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpnle_uqpd xmm7,xmm6,[rcx]
	vcmpord_spd xmm2,xmm6,xmm4
	vcmpord_spd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpord_spd xmm7,xmm6,[rcx]
	vcmpeq_uspd xmm2,xmm6,xmm4
	vcmpeq_uspd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpeq_uspd xmm7,xmm6,[rcx]
	vcmpnge_uqpd xmm2,xmm6,xmm4
	vcmpnge_uqpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpnge_uqpd xmm7,xmm6,[rcx]
	vcmpngt_uqpd xmm2,xmm6,xmm4
	vcmpngt_uqpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpngt_uqpd xmm7,xmm6,[rcx]
	vcmpfalse_ospd xmm2,xmm6,xmm4
	vcmpfalse_ospd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpfalse_ospd xmm7,xmm6,[rcx]
	vcmpneq_ospd xmm2,xmm6,xmm4
	vcmpneq_ospd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpneq_ospd xmm7,xmm6,[rcx]
	vcmpge_oqpd xmm2,xmm6,xmm4
	vcmpge_oqpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpge_oqpd xmm7,xmm6,[rcx]
	vcmpgt_oqpd xmm2,xmm6,xmm4
	vcmpgt_oqpd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpgt_oqpd xmm7,xmm6,[rcx]
	vcmptrue_uspd xmm2,xmm6,xmm4
	vcmptrue_uspd xmm7,xmm6,XMMWORD PTR [rcx]
	vcmptrue_uspd xmm7,xmm6,[rcx]
	vcmpeqps xmm2,xmm6,xmm4
	vcmpeqps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpeqps xmm7,xmm6,[rcx]
	vcmpltps xmm2,xmm6,xmm4
	vcmpltps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpltps xmm7,xmm6,[rcx]
	vcmpleps xmm2,xmm6,xmm4
	vcmpleps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpleps xmm7,xmm6,[rcx]
	vcmpunordps xmm2,xmm6,xmm4
	vcmpunordps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpunordps xmm7,xmm6,[rcx]
	vcmpneqps xmm2,xmm6,xmm4
	vcmpneqps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpneqps xmm7,xmm6,[rcx]
	vcmpnltps xmm2,xmm6,xmm4
	vcmpnltps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpnltps xmm7,xmm6,[rcx]
	vcmpnleps xmm2,xmm6,xmm4
	vcmpnleps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpnleps xmm7,xmm6,[rcx]
	vcmpordps xmm2,xmm6,xmm4
	vcmpordps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpordps xmm7,xmm6,[rcx]
	vcmpeq_uqps xmm2,xmm6,xmm4
	vcmpeq_uqps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpeq_uqps xmm7,xmm6,[rcx]
	vcmpngeps xmm2,xmm6,xmm4
	vcmpngeps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpngeps xmm7,xmm6,[rcx]
	vcmpngtps xmm2,xmm6,xmm4
	vcmpngtps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpngtps xmm7,xmm6,[rcx]
	vcmpfalseps xmm2,xmm6,xmm4
	vcmpfalseps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpfalseps xmm7,xmm6,[rcx]
	vcmpneq_oqps xmm2,xmm6,xmm4
	vcmpneq_oqps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpneq_oqps xmm7,xmm6,[rcx]
	vcmpgeps xmm2,xmm6,xmm4
	vcmpgeps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpgeps xmm7,xmm6,[rcx]
	vcmpgtps xmm2,xmm6,xmm4
	vcmpgtps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpgtps xmm7,xmm6,[rcx]
	vcmptrueps xmm2,xmm6,xmm4
	vcmptrueps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmptrueps xmm7,xmm6,[rcx]
	vcmpeq_osps xmm2,xmm6,xmm4
	vcmpeq_osps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpeq_osps xmm7,xmm6,[rcx]
	vcmplt_oqps xmm2,xmm6,xmm4
	vcmplt_oqps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmplt_oqps xmm7,xmm6,[rcx]
	vcmple_oqps xmm2,xmm6,xmm4
	vcmple_oqps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmple_oqps xmm7,xmm6,[rcx]
	vcmpunord_sps xmm2,xmm6,xmm4
	vcmpunord_sps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpunord_sps xmm7,xmm6,[rcx]
	vcmpneq_usps xmm2,xmm6,xmm4
	vcmpneq_usps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpneq_usps xmm7,xmm6,[rcx]
	vcmpnlt_uqps xmm2,xmm6,xmm4
	vcmpnlt_uqps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpnlt_uqps xmm7,xmm6,[rcx]
	vcmpnle_uqps xmm2,xmm6,xmm4
	vcmpnle_uqps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpnle_uqps xmm7,xmm6,[rcx]
	vcmpord_sps xmm2,xmm6,xmm4
	vcmpord_sps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpord_sps xmm7,xmm6,[rcx]
	vcmpeq_usps xmm2,xmm6,xmm4
	vcmpeq_usps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpeq_usps xmm7,xmm6,[rcx]
	vcmpnge_uqps xmm2,xmm6,xmm4
	vcmpnge_uqps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpnge_uqps xmm7,xmm6,[rcx]
	vcmpngt_uqps xmm2,xmm6,xmm4
	vcmpngt_uqps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpngt_uqps xmm7,xmm6,[rcx]
	vcmpfalse_osps xmm2,xmm6,xmm4
	vcmpfalse_osps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpfalse_osps xmm7,xmm6,[rcx]
	vcmpneq_osps xmm2,xmm6,xmm4
	vcmpneq_osps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpneq_osps xmm7,xmm6,[rcx]
	vcmpge_oqps xmm2,xmm6,xmm4
	vcmpge_oqps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpge_oqps xmm7,xmm6,[rcx]
	vcmpgt_oqps xmm2,xmm6,xmm4
	vcmpgt_oqps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmpgt_oqps xmm7,xmm6,[rcx]
	vcmptrue_usps xmm2,xmm6,xmm4
	vcmptrue_usps xmm7,xmm6,XMMWORD PTR [rcx]
	vcmptrue_usps xmm7,xmm6,[rcx]
    vgf2p8mulb xmm6, xmm5, xmm4
	vgf2p8mulb xmm6, xmm5, XMMWORD PTR [rcx]
	vgf2p8mulb xmm6, xmm5, [rcx]
	vgf2p8mulb xmm6, xmm5, XMMWORD PTR [rax+r14*8-123456]
	vgf2p8mulb xmm6, xmm5, XMMWORD PTR [rdx+2032]
	vgf2p8mulb xmm6, xmm5, XMMWORD PTR [rdx+2048]
	vgf2p8mulb xmm6, xmm5, XMMWORD PTR [rdx-2048]
	vgf2p8mulb xmm6, xmm5, XMMWORD PTR [rdx-2064]


# Tests for op mem128, xmm, xmm
	vmaskmovps xmm6,xmm4,XMMWORD PTR [rcx]
	vmaskmovps xmm6,xmm4,[rcx]
	vmaskmovpd xmm6,xmm4,XMMWORD PTR [rcx]
	vmaskmovpd xmm6,xmm4,[rcx]

# Tests for op imm8, xmm/mem128, xmm
	vaeskeygenassist xmm6,xmm4,7
	vaeskeygenassist xmm6,XMMWORD PTR [rcx],7
	vaeskeygenassist xmm6,[rcx],7
	vpcmpestri xmm6,xmm4,7
	vpcmpestri xmm6,XMMWORD PTR [rcx],7
	vpcmpestri xmm6,[rcx],7
	vpcmpestrm xmm6,xmm4,7
	vpcmpestrm xmm6,XMMWORD PTR [rcx],7
	vpcmpestrm xmm6,[rcx],7
	vpcmpistri xmm6,xmm4,7
	vpcmpistri xmm6,XMMWORD PTR [rcx],7
	vpcmpistri xmm6,[rcx],7
	vpcmpistrm xmm6,xmm4,7
	vpcmpistrm xmm6,XMMWORD PTR [rcx],7
	vpcmpistrm xmm6,[rcx],7
	vpermilpd xmm6,xmm4,7
	vpermilpd xmm6,XMMWORD PTR [rcx],7
	vpermilpd xmm6,[rcx],7
	vpermilps xmm6,xmm4,7
	vpermilps xmm6,XMMWORD PTR [rcx],7
	vpermilps xmm6,[rcx],7
	vpshufd xmm6,xmm4,7
	vpshufd xmm6,XMMWORD PTR [rcx],7
	vpshufd xmm6,[rcx],7
	vpshufhw xmm6,xmm4,7
	vpshufhw xmm6,XMMWORD PTR [rcx],7
	vpshufhw xmm6,[rcx],7
	vpshuflw xmm6,xmm4,7
	vpshuflw xmm6,XMMWORD PTR [rcx],7
	vpshuflw xmm6,[rcx],7
	vroundpd xmm6,xmm4,7
	vroundpd xmm6,XMMWORD PTR [rcx],7
	vroundpd xmm6,[rcx],7
	vroundps xmm6,xmm4,7
	vroundps xmm6,XMMWORD PTR [rcx],7
	vroundps xmm6,[rcx],7

# Tests for op xmm, xmm, mem128
	vmaskmovps XMMWORD PTR [rcx],xmm6,xmm4
	vmaskmovps [rcx],xmm6,xmm4
	vmaskmovpd XMMWORD PTR [rcx],xmm6,xmm4
	vmaskmovpd [rcx],xmm6,xmm4

# Tests for op imm8, xmm/mem128, xmm, xmm
	vblendpd xmm2,xmm6,xmm4,7
	vblendpd xmm2,xmm6,XMMWORD PTR [rcx],7
	vblendpd xmm2,xmm6,[rcx],7
	vblendps xmm2,xmm6,xmm4,7
	vblendps xmm2,xmm6,XMMWORD PTR [rcx],7
	vblendps xmm2,xmm6,[rcx],7
	vcmppd xmm2,xmm6,xmm4,7
	vcmppd xmm2,xmm6,XMMWORD PTR [rcx],7
	vcmppd xmm2,xmm6,[rcx],7
	vcmpps xmm2,xmm6,xmm4,7
	vcmpps xmm2,xmm6,XMMWORD PTR [rcx],7
	vcmpps xmm2,xmm6,[rcx],7
	vdppd xmm2,xmm6,xmm4,7
	vdppd xmm2,xmm6,XMMWORD PTR [rcx],7
	vdppd xmm2,xmm6,[rcx],7
	vdpps xmm2,xmm6,xmm4,7
	vdpps xmm2,xmm6,XMMWORD PTR [rcx],7
	vdpps xmm2,xmm6,[rcx],7
	vmpsadbw xmm2,xmm6,xmm4,7
	vmpsadbw xmm2,xmm6,XMMWORD PTR [rcx],7
	vmpsadbw xmm2,xmm6,[rcx],7
	vpalignr xmm2,xmm6,xmm4,7
	vpalignr xmm2,xmm6,XMMWORD PTR [rcx],7
	vpalignr xmm2,xmm6,[rcx],7
	vpblendw xmm2,xmm6,xmm4,7
	vpblendw xmm2,xmm6,XMMWORD PTR [rcx],7
	vpblendw xmm2,xmm6,[rcx],7
	vpclmulqdq xmm2,xmm6,xmm4,7
	vpclmulqdq xmm2,xmm6,XMMWORD PTR [rcx],7
	vpclmulqdq xmm2,xmm6,[rcx],7
	vshufpd xmm2,xmm6,xmm4,7
	vshufpd xmm2,xmm6,XMMWORD PTR [rcx],7
	vshufpd xmm2,xmm6,[rcx],7
	vshufps xmm2,xmm6,xmm4,7
	vshufps xmm2,xmm6,XMMWORD PTR [rcx],7
	vshufps xmm2,xmm6,[rcx],7
    vgf2p8affineqb xmm6, xmm5, xmm4, 0xab
	vgf2p8affineqb xmm6, xmm5, xmm4, 123
	vgf2p8affineqb xmm6, xmm5, XMMWORD PTR [rcx], 123
	vgf2p8affineqb xmm6, xmm5, [rcx], 123
	vgf2p8affineqb xmm6, xmm5, XMMWORD PTR [rax+r14*8-123456], 123
	vgf2p8affineqb xmm6, xmm5, XMMWORD PTR [rdx+2032], 123
	vgf2p8affineqb xmm6, xmm5, XMMWORD PTR [rdx+2048], 123
	vgf2p8affineqb xmm6, xmm5, XMMWORD PTR [rdx-2048], 123
	vgf2p8affineqb xmm6, xmm5, XMMWORD PTR [rdx-2064], 123
	vgf2p8affineinvqb xmm6, xmm5, xmm4, 0xab
	vgf2p8affineinvqb xmm6, xmm5, xmm4, 123
	vgf2p8affineinvqb xmm6, xmm5, XMMWORD PTR [rcx], 123
	vgf2p8affineinvqb xmm6, xmm5, [rcx], 123
	vgf2p8affineinvqb xmm6, xmm5, XMMWORD PTR [rax+r14*8-123456], 123
	vgf2p8affineinvqb xmm6, xmm5, XMMWORD PTR [rdx+2032], 123
	vgf2p8affineinvqb xmm6, xmm5, XMMWORD PTR [rdx+2048], 123
	vgf2p8affineinvqb xmm6, xmm5, XMMWORD PTR [rdx-2048], 123
	vgf2p8affineinvqb xmm6, xmm5, XMMWORD PTR [rdx-2064], 123


# Tests for op xmm, xmm/mem128, xmm, xmm
	vblendvpd xmm7,xmm2,xmm6,xmm4
	vblendvpd xmm7,xmm2,XMMWORD PTR [rcx],xmm4
	vblendvpd xmm7,xmm2,[rcx],xmm4
	vblendvps xmm7,xmm2,xmm6,xmm4
	vblendvps xmm7,xmm2,XMMWORD PTR [rcx],xmm4
	vblendvps xmm7,xmm2,[rcx],xmm4
	vpblendvb xmm7,xmm2,xmm6,xmm4
	vpblendvb xmm7,xmm2,XMMWORD PTR [rcx],xmm4
	vpblendvb xmm7,xmm2,[rcx],xmm4

# Tests for op mem64, ymm
	vbroadcastsd ymm4,QWORD PTR [rcx]
	vbroadcastsd ymm4,[rcx]

# Tests for op xmm/mem64, xmm
	vcomisd xmm6,xmm4
	vcomisd xmm4,QWORD PTR [rcx]
	vcomisd xmm4,[rcx]
	vcvtdq2pd xmm6,xmm4
	vcvtdq2pd xmm4,QWORD PTR [rcx]
	vcvtdq2pd xmm4,[rcx]
	vcvtps2pd xmm6,xmm4
	vcvtps2pd xmm4,QWORD PTR [rcx]
	vcvtps2pd xmm4,[rcx]
	vmovddup xmm6,xmm4
	vmovddup xmm4,QWORD PTR [rcx]
	vmovddup xmm4,[rcx]
	vpmovsxbw xmm6,xmm4
	vpmovsxbw xmm4,QWORD PTR [rcx]
	vpmovsxbw xmm4,[rcx]
	vpmovsxwd xmm6,xmm4
	vpmovsxwd xmm4,QWORD PTR [rcx]
	vpmovsxwd xmm4,[rcx]
	vpmovsxdq xmm6,xmm4
	vpmovsxdq xmm4,QWORD PTR [rcx]
	vpmovsxdq xmm4,[rcx]
	vpmovzxbw xmm6,xmm4
	vpmovzxbw xmm4,QWORD PTR [rcx]
	vpmovzxbw xmm4,[rcx]
	vpmovzxwd xmm6,xmm4
	vpmovzxwd xmm4,QWORD PTR [rcx]
	vpmovzxwd xmm4,[rcx]
	vpmovzxdq xmm6,xmm4
	vpmovzxdq xmm4,QWORD PTR [rcx]
	vpmovzxdq xmm4,[rcx]
	vucomisd xmm6,xmm4
	vucomisd xmm4,QWORD PTR [rcx]
	vucomisd xmm4,[rcx]

# Tests for op mem64, xmm
	vmovsd xmm4,QWORD PTR [rcx]
	vmovsd xmm4,[rcx]

# Tests for op xmm, mem64
	vmovlpd QWORD PTR [rcx],xmm4
	vmovlpd [rcx],xmm4
	vmovlps QWORD PTR [rcx],xmm4
	vmovlps [rcx],xmm4
	vmovhpd QWORD PTR [rcx],xmm4
	vmovhpd [rcx],xmm4
	vmovhps QWORD PTR [rcx],xmm4
	vmovhps [rcx],xmm4
	vmovsd QWORD PTR [rcx],xmm4
	vmovsd [rcx],xmm4

# Tests for op xmm, regq/mem64
# Tests for op regq/mem64, xmm
	vmovd rcx,xmm4
	vmovd xmm4,rcx
	vmovd [rcx],xmm4
	vmovd xmm4,[rcx]
	vmovq rcx,xmm4
	vmovq xmm4,rcx
	vmovq QWORD PTR [rcx],xmm4
	vmovq xmm4,QWORD PTR [rcx]
	vmovq [rcx],xmm4
	vmovq xmm4,[rcx]

# Tests for op xmm/mem64, regl
	vcvtsd2si ecx,xmm4
	vcvtsd2si ecx,QWORD PTR [rcx]
	vcvtsd2si ecx,[rcx]
	vcvttsd2si ecx,xmm4
	vcvttsd2si ecx,QWORD PTR [rcx]
	vcvttsd2si ecx,[rcx]

# Tests for op xmm/mem64, regq
	vcvtsd2si rcx,xmm4
	vcvtsd2si rcx,QWORD PTR [rcx]
	vcvtsd2si rcx,[rcx]
	vcvttsd2si rcx,xmm4
	vcvttsd2si rcx,QWORD PTR [rcx]
	vcvttsd2si rcx,[rcx]

# Tests for op regq/mem64, xmm, xmm
	vcvtsi2sdq xmm6,xmm4,rcx
	vcvtsi2sdq xmm6,xmm4,QWORD PTR [rcx]
	vcvtsi2sdq xmm6,xmm4,[rcx]
	vcvtsi2ssq xmm6,xmm4,rcx
	vcvtsi2ssq xmm6,xmm4,QWORD PTR [rcx]
	vcvtsi2ssq xmm6,xmm4,[rcx]

# Tests for op imm8, regq/mem64, xmm, xmm
	vpinsrq xmm6,xmm4,rcx,7
	vpinsrq xmm6,xmm4,QWORD PTR [rcx],7
	vpinsrq xmm6,xmm4,[rcx],7

# Testsf for op imm8, xmm, regq/mem64
	vpextrq rcx,xmm4,7
	vpextrq QWORD PTR [rcx],xmm4,7
	vpextrq [rcx],xmm4,7

# Tests for op mem64, xmm, xmm
	vmovlpd xmm6,xmm4,QWORD PTR [rcx]
	vmovlpd xmm6,xmm4,[rcx]
	vmovlps xmm6,xmm4,QWORD PTR [rcx]
	vmovlps xmm6,xmm4,[rcx]
	vmovhpd xmm6,xmm4,QWORD PTR [rcx]
	vmovhpd xmm6,xmm4,[rcx]
	vmovhps xmm6,xmm4,QWORD PTR [rcx]
	vmovhps xmm6,xmm4,[rcx]

# Tests for op imm8, xmm/mem64, xmm, xmm
	vcmpsd xmm2,xmm6,xmm4,7
	vcmpsd xmm2,xmm6,QWORD PTR [rcx],7
	vcmpsd xmm2,xmm6,[rcx],7
	vroundsd xmm2,xmm6,xmm4,7
	vroundsd xmm2,xmm6,QWORD PTR [rcx],7
	vroundsd xmm2,xmm6,[rcx],7

# Tests for op xmm/mem64, xmm, xmm
	vaddsd xmm2,xmm6,xmm4
	vaddsd xmm2,xmm6,QWORD PTR [rcx]
	vaddsd xmm2,xmm6,[rcx]
	vcvtsd2ss xmm2,xmm6,xmm4
	vcvtsd2ss xmm2,xmm6,QWORD PTR [rcx]
	vcvtsd2ss xmm2,xmm6,[rcx]
	vdivsd xmm2,xmm6,xmm4
	vdivsd xmm2,xmm6,QWORD PTR [rcx]
	vdivsd xmm2,xmm6,[rcx]
	vmaxsd xmm2,xmm6,xmm4
	vmaxsd xmm2,xmm6,QWORD PTR [rcx]
	vmaxsd xmm2,xmm6,[rcx]
	vminsd xmm2,xmm6,xmm4
	vminsd xmm2,xmm6,QWORD PTR [rcx]
	vminsd xmm2,xmm6,[rcx]
	vmulsd xmm2,xmm6,xmm4
	vmulsd xmm2,xmm6,QWORD PTR [rcx]
	vmulsd xmm2,xmm6,[rcx]
	vsqrtsd xmm2,xmm6,xmm4
	vsqrtsd xmm2,xmm6,QWORD PTR [rcx]
	vsqrtsd xmm2,xmm6,[rcx]
	vsubsd xmm2,xmm6,xmm4
	vsubsd xmm2,xmm6,QWORD PTR [rcx]
	vsubsd xmm2,xmm6,[rcx]
	vcmpeqsd xmm2,xmm6,xmm4
	vcmpeqsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpeqsd xmm2,xmm6,[rcx]
	vcmpltsd xmm2,xmm6,xmm4
	vcmpltsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpltsd xmm2,xmm6,[rcx]
	vcmplesd xmm2,xmm6,xmm4
	vcmplesd xmm2,xmm6,QWORD PTR [rcx]
	vcmplesd xmm2,xmm6,[rcx]
	vcmpunordsd xmm2,xmm6,xmm4
	vcmpunordsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpunordsd xmm2,xmm6,[rcx]
	vcmpneqsd xmm2,xmm6,xmm4
	vcmpneqsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpneqsd xmm2,xmm6,[rcx]
	vcmpnltsd xmm2,xmm6,xmm4
	vcmpnltsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpnltsd xmm2,xmm6,[rcx]
	vcmpnlesd xmm2,xmm6,xmm4
	vcmpnlesd xmm2,xmm6,QWORD PTR [rcx]
	vcmpnlesd xmm2,xmm6,[rcx]
	vcmpordsd xmm2,xmm6,xmm4
	vcmpordsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpordsd xmm2,xmm6,[rcx]
	vcmpeq_uqsd xmm2,xmm6,xmm4
	vcmpeq_uqsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpeq_uqsd xmm2,xmm6,[rcx]
	vcmpngesd xmm2,xmm6,xmm4
	vcmpngesd xmm2,xmm6,QWORD PTR [rcx]
	vcmpngesd xmm2,xmm6,[rcx]
	vcmpngtsd xmm2,xmm6,xmm4
	vcmpngtsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpngtsd xmm2,xmm6,[rcx]
	vcmpfalsesd xmm2,xmm6,xmm4
	vcmpfalsesd xmm2,xmm6,QWORD PTR [rcx]
	vcmpfalsesd xmm2,xmm6,[rcx]
	vcmpneq_oqsd xmm2,xmm6,xmm4
	vcmpneq_oqsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpneq_oqsd xmm2,xmm6,[rcx]
	vcmpgesd xmm2,xmm6,xmm4
	vcmpgesd xmm2,xmm6,QWORD PTR [rcx]
	vcmpgesd xmm2,xmm6,[rcx]
	vcmpgtsd xmm2,xmm6,xmm4
	vcmpgtsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpgtsd xmm2,xmm6,[rcx]
	vcmptruesd xmm2,xmm6,xmm4
	vcmptruesd xmm2,xmm6,QWORD PTR [rcx]
	vcmptruesd xmm2,xmm6,[rcx]
	vcmpeq_ossd xmm2,xmm6,xmm4
	vcmpeq_ossd xmm2,xmm6,QWORD PTR [rcx]
	vcmpeq_ossd xmm2,xmm6,[rcx]
	vcmplt_oqsd xmm2,xmm6,xmm4
	vcmplt_oqsd xmm2,xmm6,QWORD PTR [rcx]
	vcmplt_oqsd xmm2,xmm6,[rcx]
	vcmple_oqsd xmm2,xmm6,xmm4
	vcmple_oqsd xmm2,xmm6,QWORD PTR [rcx]
	vcmple_oqsd xmm2,xmm6,[rcx]
	vcmpunord_ssd xmm2,xmm6,xmm4
	vcmpunord_ssd xmm2,xmm6,QWORD PTR [rcx]
	vcmpunord_ssd xmm2,xmm6,[rcx]
	vcmpneq_ussd xmm2,xmm6,xmm4
	vcmpneq_ussd xmm2,xmm6,QWORD PTR [rcx]
	vcmpneq_ussd xmm2,xmm6,[rcx]
	vcmpnlt_uqsd xmm2,xmm6,xmm4
	vcmpnlt_uqsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpnlt_uqsd xmm2,xmm6,[rcx]
	vcmpnle_uqsd xmm2,xmm6,xmm4
	vcmpnle_uqsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpnle_uqsd xmm2,xmm6,[rcx]
	vcmpord_ssd xmm2,xmm6,xmm4
	vcmpord_ssd xmm2,xmm6,QWORD PTR [rcx]
	vcmpord_ssd xmm2,xmm6,[rcx]
	vcmpeq_ussd xmm2,xmm6,xmm4
	vcmpeq_ussd xmm2,xmm6,QWORD PTR [rcx]
	vcmpeq_ussd xmm2,xmm6,[rcx]
	vcmpnge_uqsd xmm2,xmm6,xmm4
	vcmpnge_uqsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpnge_uqsd xmm2,xmm6,[rcx]
	vcmpngt_uqsd xmm2,xmm6,xmm4
	vcmpngt_uqsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpngt_uqsd xmm2,xmm6,[rcx]
	vcmpfalse_ossd xmm2,xmm6,xmm4
	vcmpfalse_ossd xmm2,xmm6,QWORD PTR [rcx]
	vcmpfalse_ossd xmm2,xmm6,[rcx]
	vcmpneq_ossd xmm2,xmm6,xmm4
	vcmpneq_ossd xmm2,xmm6,QWORD PTR [rcx]
	vcmpneq_ossd xmm2,xmm6,[rcx]
	vcmpge_oqsd xmm2,xmm6,xmm4
	vcmpge_oqsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpge_oqsd xmm2,xmm6,[rcx]
	vcmpgt_oqsd xmm2,xmm6,xmm4
	vcmpgt_oqsd xmm2,xmm6,QWORD PTR [rcx]
	vcmpgt_oqsd xmm2,xmm6,[rcx]
	vcmptrue_ussd xmm2,xmm6,xmm4
	vcmptrue_ussd xmm2,xmm6,QWORD PTR [rcx]
	vcmptrue_ussd xmm2,xmm6,[rcx]

# Tests for op mem64
	vldmxcsr DWORD PTR [rcx]
	vldmxcsr [rcx]
	vstmxcsr DWORD PTR [rcx]
	vstmxcsr [rcx]

# Tests for op xmm/mem32, xmm, xmm
	vaddss xmm2,xmm6,xmm4
	vaddss xmm2,xmm6,DWORD PTR [rcx]
	vaddss xmm2,xmm6,[rcx]
	vcvtss2sd xmm2,xmm6,xmm4
	vcvtss2sd xmm2,xmm6,DWORD PTR [rcx]
	vcvtss2sd xmm2,xmm6,[rcx]
	vdivss xmm2,xmm6,xmm4
	vdivss xmm2,xmm6,DWORD PTR [rcx]
	vdivss xmm2,xmm6,[rcx]
	vmaxss xmm2,xmm6,xmm4
	vmaxss xmm2,xmm6,DWORD PTR [rcx]
	vmaxss xmm2,xmm6,[rcx]
	vminss xmm2,xmm6,xmm4
	vminss xmm2,xmm6,DWORD PTR [rcx]
	vminss xmm2,xmm6,[rcx]
	vmulss xmm2,xmm6,xmm4
	vmulss xmm2,xmm6,DWORD PTR [rcx]
	vmulss xmm2,xmm6,[rcx]
	vrcpss xmm2,xmm6,xmm4
	vrcpss xmm2,xmm6,DWORD PTR [rcx]
	vrcpss xmm2,xmm6,[rcx]
	vrsqrtss xmm2,xmm6,xmm4
	vrsqrtss xmm2,xmm6,DWORD PTR [rcx]
	vrsqrtss xmm2,xmm6,[rcx]
	vsqrtss xmm2,xmm6,xmm4
	vsqrtss xmm2,xmm6,DWORD PTR [rcx]
	vsqrtss xmm2,xmm6,[rcx]
	vsubss xmm2,xmm6,xmm4
	vsubss xmm2,xmm6,DWORD PTR [rcx]
	vsubss xmm2,xmm6,[rcx]
	vcmpeqss xmm2,xmm6,xmm4
	vcmpeqss xmm2,xmm6,DWORD PTR [rcx]
	vcmpeqss xmm2,xmm6,[rcx]
	vcmpltss xmm2,xmm6,xmm4
	vcmpltss xmm2,xmm6,DWORD PTR [rcx]
	vcmpltss xmm2,xmm6,[rcx]
	vcmpless xmm2,xmm6,xmm4
	vcmpless xmm2,xmm6,DWORD PTR [rcx]
	vcmpless xmm2,xmm6,[rcx]
	vcmpunordss xmm2,xmm6,xmm4
	vcmpunordss xmm2,xmm6,DWORD PTR [rcx]
	vcmpunordss xmm2,xmm6,[rcx]
	vcmpneqss xmm2,xmm6,xmm4
	vcmpneqss xmm2,xmm6,DWORD PTR [rcx]
	vcmpneqss xmm2,xmm6,[rcx]
	vcmpnltss xmm2,xmm6,xmm4
	vcmpnltss xmm2,xmm6,DWORD PTR [rcx]
	vcmpnltss xmm2,xmm6,[rcx]
	vcmpnless xmm2,xmm6,xmm4
	vcmpnless xmm2,xmm6,DWORD PTR [rcx]
	vcmpnless xmm2,xmm6,[rcx]
	vcmpordss xmm2,xmm6,xmm4
	vcmpordss xmm2,xmm6,DWORD PTR [rcx]
	vcmpordss xmm2,xmm6,[rcx]
	vcmpeq_uqss xmm2,xmm6,xmm4
	vcmpeq_uqss xmm2,xmm6,DWORD PTR [rcx]
	vcmpeq_uqss xmm2,xmm6,[rcx]
	vcmpngess xmm2,xmm6,xmm4
	vcmpngess xmm2,xmm6,DWORD PTR [rcx]
	vcmpngess xmm2,xmm6,[rcx]
	vcmpngtss xmm2,xmm6,xmm4
	vcmpngtss xmm2,xmm6,DWORD PTR [rcx]
	vcmpngtss xmm2,xmm6,[rcx]
	vcmpfalsess xmm2,xmm6,xmm4
	vcmpfalsess xmm2,xmm6,DWORD PTR [rcx]
	vcmpfalsess xmm2,xmm6,[rcx]
	vcmpneq_oqss xmm2,xmm6,xmm4
	vcmpneq_oqss xmm2,xmm6,DWORD PTR [rcx]
	vcmpneq_oqss xmm2,xmm6,[rcx]
	vcmpgess xmm2,xmm6,xmm4
	vcmpgess xmm2,xmm6,DWORD PTR [rcx]
	vcmpgess xmm2,xmm6,[rcx]
	vcmpgtss xmm2,xmm6,xmm4
	vcmpgtss xmm2,xmm6,DWORD PTR [rcx]
	vcmpgtss xmm2,xmm6,[rcx]
	vcmptruess xmm2,xmm6,xmm4
	vcmptruess xmm2,xmm6,DWORD PTR [rcx]
	vcmptruess xmm2,xmm6,[rcx]
	vcmpeq_osss xmm2,xmm6,xmm4
	vcmpeq_osss xmm2,xmm6,DWORD PTR [rcx]
	vcmpeq_osss xmm2,xmm6,[rcx]
	vcmplt_oqss xmm2,xmm6,xmm4
	vcmplt_oqss xmm2,xmm6,DWORD PTR [rcx]
	vcmplt_oqss xmm2,xmm6,[rcx]
	vcmple_oqss xmm2,xmm6,xmm4
	vcmple_oqss xmm2,xmm6,DWORD PTR [rcx]
	vcmple_oqss xmm2,xmm6,[rcx]
	vcmpunord_sss xmm2,xmm6,xmm4
	vcmpunord_sss xmm2,xmm6,DWORD PTR [rcx]
	vcmpunord_sss xmm2,xmm6,[rcx]
	vcmpneq_usss xmm2,xmm6,xmm4
	vcmpneq_usss xmm2,xmm6,DWORD PTR [rcx]
	vcmpneq_usss xmm2,xmm6,[rcx]
	vcmpnlt_uqss xmm2,xmm6,xmm4
	vcmpnlt_uqss xmm2,xmm6,DWORD PTR [rcx]
	vcmpnlt_uqss xmm2,xmm6,[rcx]
	vcmpnle_uqss xmm2,xmm6,xmm4
	vcmpnle_uqss xmm2,xmm6,DWORD PTR [rcx]
	vcmpnle_uqss xmm2,xmm6,[rcx]
	vcmpord_sss xmm2,xmm6,xmm4
	vcmpord_sss xmm2,xmm6,DWORD PTR [rcx]
	vcmpord_sss xmm2,xmm6,[rcx]
	vcmpeq_usss xmm2,xmm6,xmm4
	vcmpeq_usss xmm2,xmm6,DWORD PTR [rcx]
	vcmpeq_usss xmm2,xmm6,[rcx]
	vcmpnge_uqss xmm2,xmm6,xmm4
	vcmpnge_uqss xmm2,xmm6,DWORD PTR [rcx]
	vcmpnge_uqss xmm2,xmm6,[rcx]
	vcmpngt_uqss xmm2,xmm6,xmm4
	vcmpngt_uqss xmm2,xmm6,DWORD PTR [rcx]
	vcmpngt_uqss xmm2,xmm6,[rcx]
	vcmpfalse_osss xmm2,xmm6,xmm4
	vcmpfalse_osss xmm2,xmm6,DWORD PTR [rcx]
	vcmpfalse_osss xmm2,xmm6,[rcx]
	vcmpneq_osss xmm2,xmm6,xmm4
	vcmpneq_osss xmm2,xmm6,DWORD PTR [rcx]
	vcmpneq_osss xmm2,xmm6,[rcx]
	vcmpge_oqss xmm2,xmm6,xmm4
	vcmpge_oqss xmm2,xmm6,DWORD PTR [rcx]
	vcmpge_oqss xmm2,xmm6,[rcx]
	vcmpgt_oqss xmm2,xmm6,xmm4
	vcmpgt_oqss xmm2,xmm6,DWORD PTR [rcx]
	vcmpgt_oqss xmm2,xmm6,[rcx]
	vcmptrue_usss xmm2,xmm6,xmm4
	vcmptrue_usss xmm2,xmm6,DWORD PTR [rcx]
	vcmptrue_usss xmm2,xmm6,[rcx]

# Tests for op mem32, ymm
	vbroadcastss ymm4,DWORD PTR [rcx]
	vbroadcastss ymm4,[rcx]

# Tests for op xmm/mem32, xmm
	vcomiss xmm6,xmm4
	vcomiss xmm4,DWORD PTR [rcx]
	vcomiss xmm4,[rcx]
	vpmovsxbd xmm6,xmm4
	vpmovsxbd xmm4,DWORD PTR [rcx]
	vpmovsxbd xmm4,[rcx]
	vpmovsxwq xmm6,xmm4
	vpmovsxwq xmm4,DWORD PTR [rcx]
	vpmovsxwq xmm4,[rcx]
	vpmovzxbd xmm6,xmm4
	vpmovzxbd xmm4,DWORD PTR [rcx]
	vpmovzxbd xmm4,[rcx]
	vpmovzxwq xmm6,xmm4
	vpmovzxwq xmm4,DWORD PTR [rcx]
	vpmovzxwq xmm4,[rcx]
	vucomiss xmm6,xmm4
	vucomiss xmm4,DWORD PTR [rcx]
	vucomiss xmm4,[rcx]

# Tests for op mem32, xmm
	vbroadcastss xmm4,DWORD PTR [rcx]
	vbroadcastss xmm4,[rcx]
	vmovss xmm4,DWORD PTR [rcx]
	vmovss xmm4,[rcx]

# Tests for op xmm, mem32
	vmovss DWORD PTR [rcx],xmm4
	vmovss [rcx],xmm4

# Tests for op xmm, regl/mem32
# Tests for op regl/mem32, xmm
	vmovd ecx,xmm4
	vmovd DWORD PTR [rcx],xmm4
	vmovd xmm4,ecx
	vmovd xmm4,DWORD PTR [rcx]
	vmovd [rcx],xmm4
	vmovd xmm4,[rcx]

# Tests for op xmm/mem32, regl
	vcvtss2si ecx,xmm4
	vcvtss2si ecx,DWORD PTR [rcx]
	vcvtss2si ecx,[rcx]
	vcvttss2si ecx,xmm4
	vcvttss2si ecx,DWORD PTR [rcx]
	vcvttss2si ecx,[rcx]

# Tests for op xmm/mem32, regq
	vcvtss2si rcx,xmm4
	vcvtss2si rcx,DWORD PTR [rcx]
	vcvtss2si rcx,[rcx]
	vcvttss2si rcx,xmm4
	vcvttss2si rcx,DWORD PTR [rcx]
	vcvttss2si rcx,[rcx]

# Tests for op xmm, regq
	vmovmskpd rcx,xmm4
	vmovmskps rcx,xmm4
	vpmovmskb rcx,xmm4

# Tests for op imm8, xmm, regq/mem32
	vextractps rcx,xmm4,7
	vextractps DWORD PTR [rcx],xmm4,7
	vextractps [rcx],xmm4,7

# Tests for op imm8, xmm, regl/mem32
	vpextrd ecx,xmm4,7
	vpextrd DWORD PTR [rcx],xmm4,7
	vpextrd [rcx],xmm4,7
	vextractps ecx,xmm4,7
	vextractps DWORD PTR [rcx],xmm4,7
	vextractps [rcx],xmm4,7

# Tests for op imm8, regl/mem32, xmm, xmm
	vpinsrd xmm6,xmm4,ecx,7
	vpinsrd xmm6,xmm4,DWORD PTR [rcx],7
	vpinsrd xmm6,xmm4,[rcx],7

# Tests for op regl/mem32, xmm, xmm
	vcvtsi2sd xmm6,xmm4,ecx
	vcvtsi2sd xmm6,xmm4,DWORD PTR [rcx]
	vcvtsi2ss xmm6,xmm4,ecx
	vcvtsi2ss xmm6,xmm4,DWORD PTR [rcx]

# Tests for op imm8, xmm/mem32, xmm, xmm
	vcmpss xmm2,xmm6,xmm4,7
	vcmpss xmm2,xmm6,DWORD PTR [rcx],7
	vcmpss xmm2,xmm6,[rcx],7
	vinsertps xmm2,xmm6,xmm4,7
	vinsertps xmm2,xmm6,DWORD PTR [rcx],7
	vinsertps xmm2,xmm6,[rcx],7
	vroundss xmm2,xmm6,xmm4,7
	vroundss xmm2,xmm6,DWORD PTR [rcx],7
	vroundss xmm2,xmm6,[rcx],7

# Tests for op xmm/m16, xmm
	vpmovsxbq xmm6,xmm4
	vpmovsxbq xmm4,WORD PTR [rcx]
	vpmovsxbq xmm4,[rcx]
	vpmovzxbq xmm6,xmm4
	vpmovzxbq xmm4,WORD PTR [rcx]
	vpmovzxbq xmm4,[rcx]

# Tests for op imm8, xmm, regl/mem16
	vpextrw ecx,xmm4,7
	vpextrw WORD PTR [rcx],xmm4,7
	vpextrw [rcx],xmm4,7

# Tests for op imm8, xmm, regq/mem16
	vpextrw rcx,xmm4,7
	vpextrw WORD PTR [rcx],xmm4,7
	vpextrw [rcx],xmm4,7

# Tests for op imm8, regl/mem16, xmm, xmm
	vpinsrw xmm6,xmm4,ecx,7
	vpinsrw xmm6,xmm4,WORD PTR [rcx],7
	vpinsrw xmm6,xmm4,[rcx],7


	vpinsrw xmm6,xmm4,rcx,7
	vpinsrw xmm6,xmm4,WORD PTR [rcx],7
	vpinsrw xmm6,xmm4,[rcx],7

# Tests for op imm8, xmm, regl/mem8
	vpextrb ecx,xmm4,7
	vpextrb BYTE PTR [rcx],xmm4,7
	vpextrb [rcx],xmm4,7

# Tests for op imm8, regl/mem8, xmm, xmm
	vpinsrb xmm6,xmm4,ecx,7
	vpinsrb xmm6,xmm4,BYTE PTR [rcx],7
	vpinsrb xmm6,xmm4,[rcx],7

# Tests for op imm8, xmm, regq
	vpextrw rcx,xmm4,7

# Tests for op imm8, xmm, regq/mem8
	vpextrb rcx,xmm4,7
	vpextrb BYTE PTR [rcx],xmm4,7
	vpextrb [rcx],xmm4,7

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

# Tests for op ymm, regq
	vmovmskpd rcx,ymm4
	vmovmskps rcx,ymm4

# Default instructions without suffixes.
	vcvtpd2dq xmm6,xmm4
	vcvtpd2dq xmm6,ymm4
	vcvtpd2ps xmm6,xmm4
	vcvtpd2ps xmm6,ymm4
	vcvttpd2dq xmm6,xmm4
	vcvttpd2dq xmm6,ymm4

#Tests with different memory and register operands.
	vldmxcsr DWORD PTR ds:0x12345678
	vmovdqa xmm8,XMMWORD PTR ds:0x12345678
	vmovdqa XMMWORD PTR ds:0x12345678,xmm8
	vmovd DWORD PTR ds:0x12345678,xmm8
	vcvtsd2si r8d,QWORD PTR ds:0x12345678
	vcvtdq2pd ymm8,XMMWORD PTR ds:0x12345678
	vcvtpd2ps xmm8,YMMWORD PTR ds:0x12345678
	vpavgb xmm15,xmm8,XMMWORD PTR ds:0x12345678
	vaeskeygenassist xmm8,XMMWORD PTR ds:0x12345678,7
	vpextrb ds:0x12345678,xmm8,7
	vcvtsi2sd xmm15,xmm8,DWORD PTR ds:0x12345678
	vpclmulqdq xmm15,xmm8,XMMWORD PTR ds:0x12345678,7
	vblendvps xmm14,xmm12,XMMWORD PTR ds:0x12345678,xmm8
	vpinsrb xmm15,xmm8,ds:0x12345678,7
	vmovdqa ymm8,YMMWORD PTR ds:0x12345678
	vmovdqa YMMWORD PTR ds:0x12345678,ymm8
	vpermilpd ymm15,ymm8,YMMWORD PTR ds:0x12345678
	vroundpd ymm8,YMMWORD PTR ds:0x12345678,7
	vextractf128 XMMWORD PTR ds:0x12345678,ymm8,7
	vperm2f128 ymm15,ymm8,YMMWORD PTR ds:0x12345678,7
	vblendvpd ymm14,ymm12,YMMWORD PTR ds:0x12345678,ymm8
	vldmxcsr DWORD PTR [rbp]
	vmovdqa xmm8,XMMWORD PTR [rbp]
	vmovdqa XMMWORD PTR [rbp],xmm8
	vmovd DWORD PTR [rbp],xmm8
	vcvtsd2si r8d,QWORD PTR [rbp]
	vcvtdq2pd ymm8,XMMWORD PTR [rbp]
	vcvtpd2ps xmm8,YMMWORD PTR [rbp]
	vpavgb xmm15,xmm8,XMMWORD PTR [rbp]
	vaeskeygenassist xmm8,XMMWORD PTR [rbp],7
	vpextrb [rbp],xmm8,7
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rbp]
	vpclmulqdq xmm15,xmm8,XMMWORD PTR [rbp],7
	vblendvps xmm14,xmm12,XMMWORD PTR [rbp],xmm8
	vpinsrb xmm15,xmm8,[rbp],7
	vmovdqa ymm8,YMMWORD PTR [rbp]
	vmovdqa YMMWORD PTR [rbp],ymm8
	vpermilpd ymm15,ymm8,YMMWORD PTR [rbp]
	vroundpd ymm8,YMMWORD PTR [rbp],7
	vextractf128 XMMWORD PTR [rbp],ymm8,7
	vperm2f128 ymm15,ymm8,YMMWORD PTR [rbp],7
	vblendvpd ymm14,ymm12,YMMWORD PTR [rbp],ymm8
	vldmxcsr DWORD PTR [rbp+0x99]
	vmovdqa xmm8,XMMWORD PTR [rbp+0x99]
	vmovdqa XMMWORD PTR [rbp+0x99],xmm8
	vmovd DWORD PTR [rbp+0x99],xmm8
	vcvtsd2si r8d,QWORD PTR [rbp+0x99]
	vcvtdq2pd ymm8,XMMWORD PTR [rbp+0x99]
	vcvtpd2ps xmm8,YMMWORD PTR [rbp+0x99]
	vpavgb xmm15,xmm8,XMMWORD PTR [rbp+0x99]
	vaeskeygenassist xmm8,XMMWORD PTR [rbp+0x99],7
	vpextrb [rbp+0x99],xmm8,7
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rbp+0x99]
	vpclmulqdq xmm15,xmm8,XMMWORD PTR [rbp+0x99],7
	vblendvps xmm14,xmm12,XMMWORD PTR [rbp+0x99],xmm8
	vpinsrb xmm15,xmm8,[rbp+0x99],7
	vmovdqa ymm8,YMMWORD PTR [rbp+0x99]
	vmovdqa YMMWORD PTR [rbp+0x99],ymm8
	vpermilpd ymm15,ymm8,YMMWORD PTR [rbp+0x99]
	vroundpd ymm8,YMMWORD PTR [rbp+0x99],7
	vextractf128 XMMWORD PTR [rbp+0x99],ymm8,7
	vperm2f128 ymm15,ymm8,YMMWORD PTR [rbp+0x99],7
	vblendvpd ymm14,ymm12,YMMWORD PTR [rbp+0x99],ymm8
	vldmxcsr DWORD PTR [r15+0x99]
	vmovdqa xmm8,XMMWORD PTR [r15+0x99]
	vmovdqa XMMWORD PTR [r15+0x99],xmm8
	vmovd DWORD PTR [r15+0x99],xmm8
	vcvtsd2si r8d,QWORD PTR [r15+0x99]
	vcvtdq2pd ymm8,XMMWORD PTR [r15+0x99]
	vcvtpd2ps xmm8,YMMWORD PTR [r15+0x99]
	vpavgb xmm15,xmm8,XMMWORD PTR [r15+0x99]
	vaeskeygenassist xmm8,XMMWORD PTR [r15+0x99],7
	vpextrb [r15+0x99],xmm8,7
	vcvtsi2sd xmm15,xmm8,DWORD PTR [r15+0x99]
	vpclmulqdq xmm15,xmm8,XMMWORD PTR [r15+0x99],7
	vblendvps xmm14,xmm12,XMMWORD PTR [r15+0x99],xmm8
	vpinsrb xmm15,xmm8,[r15+0x99],7
	vmovdqa ymm8,YMMWORD PTR [r15+0x99]
	vmovdqa YMMWORD PTR [r15+0x99],ymm8
	vpermilpd ymm15,ymm8,YMMWORD PTR [r15+0x99]
	vroundpd ymm8,YMMWORD PTR [r15+0x99],7
	vextractf128 XMMWORD PTR [r15+0x99],ymm8,7
	vperm2f128 ymm15,ymm8,YMMWORD PTR [r15+0x99],7
	vblendvpd ymm14,ymm12,YMMWORD PTR [r15+0x99],ymm8
	vldmxcsr DWORD PTR [rip+0x99]
	vmovdqa xmm8,XMMWORD PTR [rip+0x99]
	vmovdqa XMMWORD PTR [rip+0x99],xmm8
	vmovd DWORD PTR [rip+0x99],xmm8
	vcvtsd2si r8d,QWORD PTR [rip+0x99]
	vcvtdq2pd ymm8,XMMWORD PTR [rip+0x99]
	vcvtpd2ps xmm8,YMMWORD PTR [rip+0x99]
	vpavgb xmm15,xmm8,XMMWORD PTR [rip+0x99]
	vaeskeygenassist xmm8,XMMWORD PTR [rip+0x99],7
	vpextrb [rip+0x99],xmm8,7
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rip+0x99]
	vpclmulqdq xmm15,xmm8,XMMWORD PTR [rip+0x99],7
	vblendvps xmm14,xmm12,XMMWORD PTR [rip+0x99],xmm8
	vpinsrb xmm15,xmm8,[rip+0x99],7
	vmovdqa ymm8,YMMWORD PTR [rip+0x99]
	vmovdqa YMMWORD PTR [rip+0x99],ymm8
	vpermilpd ymm15,ymm8,YMMWORD PTR [rip+0x99]
	vroundpd ymm8,YMMWORD PTR [rip+0x99],7
	vextractf128 XMMWORD PTR [rip+0x99],ymm8,7
	vperm2f128 ymm15,ymm8,YMMWORD PTR [rip+0x99],7
	vblendvpd ymm14,ymm12,YMMWORD PTR [rip+0x99],ymm8
	vldmxcsr DWORD PTR [rsp+0x99]
	vmovdqa xmm8,XMMWORD PTR [rsp+0x99]
	vmovdqa XMMWORD PTR [rsp+0x99],xmm8
	vmovd DWORD PTR [rsp+0x99],xmm8
	vcvtsd2si r8d,QWORD PTR [rsp+0x99]
	vcvtdq2pd ymm8,XMMWORD PTR [rsp+0x99]
	vcvtpd2ps xmm8,YMMWORD PTR [rsp+0x99]
	vpavgb xmm15,xmm8,XMMWORD PTR [rsp+0x99]
	vaeskeygenassist xmm8,XMMWORD PTR [rsp+0x99],7
	vpextrb [rsp+0x99],xmm8,7
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rsp+0x99]
	vpclmulqdq xmm15,xmm8,XMMWORD PTR [rsp+0x99],7
	vblendvps xmm14,xmm12,XMMWORD PTR [rsp+0x99],xmm8
	vpinsrb xmm15,xmm8,[rsp+0x99],7
	vmovdqa ymm8,YMMWORD PTR [rsp+0x99]
	vmovdqa YMMWORD PTR [rsp+0x99],ymm8
	vpermilpd ymm15,ymm8,YMMWORD PTR [rsp+0x99]
	vroundpd ymm8,YMMWORD PTR [rsp+0x99],7
	vextractf128 XMMWORD PTR [rsp+0x99],ymm8,7
	vperm2f128 ymm15,ymm8,YMMWORD PTR [rsp+0x99],7
	vblendvpd ymm14,ymm12,YMMWORD PTR [rsp+0x99],ymm8
	vldmxcsr DWORD PTR [r12+0x99]
	vmovdqa xmm8,XMMWORD PTR [r12+0x99]
	vmovdqa XMMWORD PTR [r12+0x99],xmm8
	vmovd DWORD PTR [r12+0x99],xmm8
	vcvtsd2si r8d,QWORD PTR [r12+0x99]
	vcvtdq2pd ymm8,XMMWORD PTR [r12+0x99]
	vcvtpd2ps xmm8,YMMWORD PTR [r12+0x99]
	vpavgb xmm15,xmm8,XMMWORD PTR [r12+0x99]
	vaeskeygenassist xmm8,XMMWORD PTR [r12+0x99],7
	vpextrb [r12+0x99],xmm8,7
	vcvtsi2sd xmm15,xmm8,DWORD PTR [r12+0x99]
	vpclmulqdq xmm15,xmm8,XMMWORD PTR [r12+0x99],7
	vblendvps xmm14,xmm12,XMMWORD PTR [r12+0x99],xmm8
	vpinsrb xmm15,xmm8,[r12+0x99],7
	vmovdqa ymm8,YMMWORD PTR [r12+0x99]
	vmovdqa YMMWORD PTR [r12+0x99],ymm8
	vpermilpd ymm15,ymm8,YMMWORD PTR [r12+0x99]
	vroundpd ymm8,YMMWORD PTR [r12+0x99],7
	vextractf128 XMMWORD PTR [r12+0x99],ymm8,7
	vperm2f128 ymm15,ymm8,YMMWORD PTR [r12+0x99],7
	vblendvpd ymm14,ymm12,YMMWORD PTR [r12+0x99],ymm8
	vldmxcsr DWORD PTR [riz*1-0x99]
	vmovdqa xmm8,XMMWORD PTR [riz*1-0x99]
	vmovdqa XMMWORD PTR [riz*1-0x99],xmm8
	vmovd DWORD PTR [riz*1-0x99],xmm8
	vcvtsd2si r8d,QWORD PTR [riz*1-0x99]
	vcvtdq2pd ymm8,XMMWORD PTR [riz*1-0x99]
	vcvtpd2ps xmm8,YMMWORD PTR [riz*1-0x99]
	vpavgb xmm15,xmm8,XMMWORD PTR [riz*1-0x99]
	vaeskeygenassist xmm8,XMMWORD PTR [riz*1-0x99],7
	vpextrb [riz*1-0x99],xmm8,7
	vcvtsi2sd xmm15,xmm8,DWORD PTR [riz*1-0x99]
	vpclmulqdq xmm15,xmm8,XMMWORD PTR [riz*1-0x99],7
	vblendvps xmm14,xmm12,XMMWORD PTR [riz*1-0x99],xmm8
	vpinsrb xmm15,xmm8,[riz*1-0x99],7
	vmovdqa ymm8,YMMWORD PTR [riz*1-0x99]
	vmovdqa YMMWORD PTR [riz*1-0x99],ymm8
	vpermilpd ymm15,ymm8,YMMWORD PTR [riz*1-0x99]
	vroundpd ymm8,YMMWORD PTR [riz*1-0x99],7
	vextractf128 XMMWORD PTR [riz*1-0x99],ymm8,7
	vperm2f128 ymm15,ymm8,YMMWORD PTR [riz*1-0x99],7
	vblendvpd ymm14,ymm12,YMMWORD PTR [riz*1-0x99],ymm8
	vldmxcsr DWORD PTR [riz*2-0x99]
	vmovdqa xmm8,XMMWORD PTR [riz*2-0x99]
	vmovdqa XMMWORD PTR [riz*2-0x99],xmm8
	vmovd DWORD PTR [riz*2-0x99],xmm8
	vcvtsd2si r8d,QWORD PTR [riz*2-0x99]
	vcvtdq2pd ymm8,XMMWORD PTR [riz*2-0x99]
	vcvtpd2ps xmm8,YMMWORD PTR [riz*2-0x99]
	vpavgb xmm15,xmm8,XMMWORD PTR [riz*2-0x99]
	vaeskeygenassist xmm8,XMMWORD PTR [riz*2-0x99],7
	vpextrb [riz*2-0x99],xmm8,7
	vcvtsi2sd xmm15,xmm8,DWORD PTR [riz*2-0x99]
	vpclmulqdq xmm15,xmm8,XMMWORD PTR [riz*2-0x99],7
	vblendvps xmm14,xmm12,XMMWORD PTR [riz*2-0x99],xmm8
	vpinsrb xmm15,xmm8,[riz*2-0x99],7
	vmovdqa ymm8,YMMWORD PTR [riz*2-0x99]
	vmovdqa YMMWORD PTR [riz*2-0x99],ymm8
	vpermilpd ymm15,ymm8,YMMWORD PTR [riz*2-0x99]
	vroundpd ymm8,YMMWORD PTR [riz*2-0x99],7
	vextractf128 XMMWORD PTR [riz*2-0x99],ymm8,7
	vperm2f128 ymm15,ymm8,YMMWORD PTR [riz*2-0x99],7
	vblendvpd ymm14,ymm12,YMMWORD PTR [riz*2-0x99],ymm8
	vldmxcsr DWORD PTR [rbx+riz*1-0x99]
	vmovdqa xmm8,XMMWORD PTR [rbx+riz*1-0x99]
	vmovdqa XMMWORD PTR [rbx+riz*1-0x99],xmm8
	vmovd DWORD PTR [rbx+riz*1-0x99],xmm8
	vcvtsd2si r8d,QWORD PTR [rbx+riz*1-0x99]
	vcvtdq2pd ymm8,XMMWORD PTR [rbx+riz*1-0x99]
	vcvtpd2ps xmm8,YMMWORD PTR [rbx+riz*1-0x99]
	vpavgb xmm15,xmm8,XMMWORD PTR [rbx+riz*1-0x99]
	vaeskeygenassist xmm8,XMMWORD PTR [rbx+riz*1-0x99],7
	vpextrb [rbx+riz*1-0x99],xmm8,7
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rbx+riz*1-0x99]
	vpclmulqdq xmm15,xmm8,XMMWORD PTR [rbx+riz*1-0x99],7
	vblendvps xmm14,xmm12,XMMWORD PTR [rbx+riz*1-0x99],xmm8
	vpinsrb xmm15,xmm8,[rbx+riz*1-0x99],7
	vmovdqa ymm8,YMMWORD PTR [rbx+riz*1-0x99]
	vmovdqa YMMWORD PTR [rbx+riz*1-0x99],ymm8
	vpermilpd ymm15,ymm8,YMMWORD PTR [rbx+riz*1-0x99]
	vroundpd ymm8,YMMWORD PTR [rbx+riz*1-0x99],7
	vextractf128 XMMWORD PTR [rbx+riz*1-0x99],ymm8,7
	vperm2f128 ymm15,ymm8,YMMWORD PTR [rbx+riz*1-0x99],7
	vblendvpd ymm14,ymm12,YMMWORD PTR [rbx+riz*1-0x99],ymm8
	vldmxcsr DWORD PTR [rbx+riz*2-0x99]
	vmovdqa xmm8,XMMWORD PTR [rbx+riz*2-0x99]
	vmovdqa XMMWORD PTR [rbx+riz*2-0x99],xmm8
	vmovd DWORD PTR [rbx+riz*2-0x99],xmm8
	vcvtsd2si r8d,QWORD PTR [rbx+riz*2-0x99]
	vcvtdq2pd ymm8,XMMWORD PTR [rbx+riz*2-0x99]
	vcvtpd2ps xmm8,YMMWORD PTR [rbx+riz*2-0x99]
	vpavgb xmm15,xmm8,XMMWORD PTR [rbx+riz*2-0x99]
	vaeskeygenassist xmm8,XMMWORD PTR [rbx+riz*2-0x99],7
	vpextrb [rbx+riz*2-0x99],xmm8,7
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rbx+riz*2-0x99]
	vpclmulqdq xmm15,xmm8,XMMWORD PTR [rbx+riz*2-0x99],7
	vblendvps xmm14,xmm12,XMMWORD PTR [rbx+riz*2-0x99],xmm8
	vpinsrb xmm15,xmm8,[rbx+riz*2-0x99],7
	vmovdqa ymm8,YMMWORD PTR [rbx+riz*2-0x99]
	vmovdqa YMMWORD PTR [rbx+riz*2-0x99],ymm8
	vpermilpd ymm15,ymm8,YMMWORD PTR [rbx+riz*2-0x99]
	vroundpd ymm8,YMMWORD PTR [rbx+riz*2-0x99],7
	vextractf128 XMMWORD PTR [rbx+riz*2-0x99],ymm8,7
	vperm2f128 ymm15,ymm8,YMMWORD PTR [rbx+riz*2-0x99],7
	vblendvpd ymm14,ymm12,YMMWORD PTR [rbx+riz*2-0x99],ymm8
	vldmxcsr DWORD PTR [r12+r15*4-0x99]
	vmovdqa xmm8,XMMWORD PTR [r12+r15*4-0x99]
	vmovdqa XMMWORD PTR [r12+r15*4-0x99],xmm8
	vmovd DWORD PTR [r12+r15*4-0x99],xmm8
	vcvtsd2si r8d,QWORD PTR [r12+r15*4-0x99]
	vcvtdq2pd ymm8,XMMWORD PTR [r12+r15*4-0x99]
	vcvtpd2ps xmm8,YMMWORD PTR [r12+r15*4-0x99]
	vpavgb xmm15,xmm8,XMMWORD PTR [r12+r15*4-0x99]
	vaeskeygenassist xmm8,XMMWORD PTR [r12+r15*4-0x99],7
	vpextrb [r12+r15*4-0x99],xmm8,7
	vcvtsi2sd xmm15,xmm8,DWORD PTR [r12+r15*4-0x99]
	vpclmulqdq xmm15,xmm8,XMMWORD PTR [r12+r15*4-0x99],7
	vblendvps xmm14,xmm12,XMMWORD PTR [r12+r15*4-0x99],xmm8
	vpinsrb xmm15,xmm8,[r12+r15*4-0x99],7
	vmovdqa ymm8,YMMWORD PTR [r12+r15*4-0x99]
	vmovdqa YMMWORD PTR [r12+r15*4-0x99],ymm8
	vpermilpd ymm15,ymm8,YMMWORD PTR [r12+r15*4-0x99]
	vroundpd ymm8,YMMWORD PTR [r12+r15*4-0x99],7
	vextractf128 XMMWORD PTR [r12+r15*4-0x99],ymm8,7
	vperm2f128 ymm15,ymm8,YMMWORD PTR [r12+r15*4-0x99],7
	vblendvpd ymm14,ymm12,YMMWORD PTR [r12+r15*4-0x99],ymm8
	vldmxcsr DWORD PTR [r8+r15*8-0x99]
	vmovdqa xmm8,XMMWORD PTR [r8+r15*8-0x99]
	vmovdqa XMMWORD PTR [r8+r15*8-0x99],xmm8
	vmovd DWORD PTR [r8+r15*8-0x99],xmm8
	vcvtsd2si r8d,QWORD PTR [r8+r15*8-0x99]
	vcvtdq2pd ymm8,XMMWORD PTR [r8+r15*8-0x99]
	vcvtpd2ps xmm8,YMMWORD PTR [r8+r15*8-0x99]
	vpavgb xmm15,xmm8,XMMWORD PTR [r8+r15*8-0x99]
	vaeskeygenassist xmm8,XMMWORD PTR [r8+r15*8-0x99],7
	vpextrb [r8+r15*8-0x99],xmm8,7
	vcvtsi2sd xmm15,xmm8,DWORD PTR [r8+r15*8-0x99]
	vpclmulqdq xmm15,xmm8,XMMWORD PTR [r8+r15*8-0x99],7
	vblendvps xmm14,xmm12,XMMWORD PTR [r8+r15*8-0x99],xmm8
	vpinsrb xmm15,xmm8,[r8+r15*8-0x99],7
	vmovdqa ymm8,YMMWORD PTR [r8+r15*8-0x99]
	vmovdqa YMMWORD PTR [r8+r15*8-0x99],ymm8
	vpermilpd ymm15,ymm8,YMMWORD PTR [r8+r15*8-0x99]
	vroundpd ymm8,YMMWORD PTR [r8+r15*8-0x99],7
	vextractf128 XMMWORD PTR [r8+r15*8-0x99],ymm8,7
	vperm2f128 ymm15,ymm8,YMMWORD PTR [r8+r15*8-0x99],7
	vblendvpd ymm14,ymm12,YMMWORD PTR [r8+r15*8-0x99],ymm8
	vldmxcsr DWORD PTR [rbp+r12*4-0x99]
	vmovdqa xmm8,XMMWORD PTR [rbp+r12*4-0x99]
	vmovdqa XMMWORD PTR [rbp+r12*4-0x99],xmm8
	vmovd DWORD PTR [rbp+r12*4-0x99],xmm8
	vcvtsd2si r8d,QWORD PTR [rbp+r12*4-0x99]
	vcvtdq2pd ymm8,XMMWORD PTR [rbp+r12*4-0x99]
	vcvtpd2ps xmm8,YMMWORD PTR [rbp+r12*4-0x99]
	vpavgb xmm15,xmm8,XMMWORD PTR [rbp+r12*4-0x99]
	vaeskeygenassist xmm8,XMMWORD PTR [rbp+r12*4-0x99],7
	vpextrb [rbp+r12*4-0x99],xmm8,7
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rbp+r12*4-0x99]
	vpclmulqdq xmm15,xmm8,XMMWORD PTR [rbp+r12*4-0x99],7
	vblendvps xmm14,xmm12,XMMWORD PTR [rbp+r12*4-0x99],xmm8
	vpinsrb xmm15,xmm8,[rbp+r12*4-0x99],7
	vmovdqa ymm8,YMMWORD PTR [rbp+r12*4-0x99]
	vmovdqa YMMWORD PTR [rbp+r12*4-0x99],ymm8
	vpermilpd ymm15,ymm8,YMMWORD PTR [rbp+r12*4-0x99]
	vroundpd ymm8,YMMWORD PTR [rbp+r12*4-0x99],7
	vextractf128 XMMWORD PTR [rbp+r12*4-0x99],ymm8,7
	vperm2f128 ymm15,ymm8,YMMWORD PTR [rbp+r12*4-0x99],7
	vblendvpd ymm14,ymm12,YMMWORD PTR [rbp+r12*4-0x99],ymm8
	vldmxcsr DWORD PTR [rsp+r13*1-0x99]
	vmovdqa xmm8,XMMWORD PTR [rsp+r13*1-0x99]
	vmovdqa XMMWORD PTR [rsp+r13*1-0x99],xmm8
	vmovd DWORD PTR [rsp+r13*1-0x99],xmm8
	vcvtsd2si r8d,QWORD PTR [rsp+r13*1-0x99]
	vcvtdq2pd ymm8,XMMWORD PTR [rsp+r13*1-0x99]
	vcvtpd2ps xmm8,YMMWORD PTR [rsp+r13*1-0x99]
	vpavgb xmm15,xmm8,XMMWORD PTR [rsp+r13*1-0x99]
	vaeskeygenassist xmm8,XMMWORD PTR [rsp+r13*1-0x99],7
	vpextrb [rsp+r13*1-0x99],xmm8,7
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rsp+r13*1-0x99]
	vpclmulqdq xmm15,xmm8,XMMWORD PTR [rsp+r13*1-0x99],7
	vblendvps xmm14,xmm12,XMMWORD PTR [rsp+r13*1-0x99],xmm8
	vpinsrb xmm15,xmm8,[rsp+r13*1-0x99],7
	vmovdqa ymm8,YMMWORD PTR [rsp+r13*1-0x99]
	vmovdqa YMMWORD PTR [rsp+r13*1-0x99],ymm8
	vpermilpd ymm15,ymm8,YMMWORD PTR [rsp+r13*1-0x99]
	vroundpd ymm8,YMMWORD PTR [rsp+r13*1-0x99],7
	vextractf128 XMMWORD PTR [rsp+r13*1-0x99],ymm8,7
	vperm2f128 ymm15,ymm8,YMMWORD PTR [rsp+r13*1-0x99],7
	vblendvpd ymm14,ymm12,YMMWORD PTR [rsp+r13*1-0x99],ymm8
# Tests for all register operands.
	vmovmskpd r8d,xmm8
	vpslld xmm15,xmm8,7
	vmovmskps r8d,ymm8
	vmovdqa xmm15,xmm8
	vmovd r8d,xmm8
	vcvtsd2si r8d,xmm8
	vcvtdq2pd ymm8,xmm8
	vcvtpd2ps xmm8,ymm8
	vaeskeygenassist xmm15,xmm8,7
	vpextrb r8d,xmm8,7
	vcvtsi2sd xmm15,xmm8,r8d
	vpclmulqdq xmm12,xmm15,xmm8,7
	vblendvps xmm14,xmm12,xmm8,xmm8
	vpinsrb xmm15,xmm8,r8d,7
	vmovdqa ymm15,ymm8
	vpermilpd ymm12,ymm15,ymm8
	vroundpd ymm15,ymm8,7
	vextractf128 xmm8,ymm8,7
	vperm2f128 ymm12,ymm15,ymm8,7
	vblendvpd ymm14,ymm12,ymm15,ymm8
	vinsertf128 ymm15,ymm8,xmm8,7
# Tests for different memory/register operand
	vcvtsd2si r8,QWORD PTR  [rcx]
	vextractps r8,xmm8,10
	vcvtss2si r8,DWORD PTR  [rcx]
	vpinsrw xmm8,xmm15,r8,7
