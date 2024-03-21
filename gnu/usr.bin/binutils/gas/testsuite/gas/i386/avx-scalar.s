# Check AVX scalar instructions

	.allow_index_reg
	.text
_start:

# Tests for op xmm/mem64, xmm
	vcomisd %xmm4,%xmm6
	vcomisd (%ecx),%xmm4
	vucomisd %xmm4,%xmm6
	vucomisd (%ecx),%xmm4

# Tests for op mem64, xmm
	vmovsd (%ecx),%xmm4

# Tests for op xmm, mem64
	vmovsd %xmm4,(%ecx)

# Tests for op xmm/mem64, regl
	vcvtsd2si %xmm4,%ecx
	vcvtsd2si (%ecx),%ecx
	vcvttsd2si %xmm4,%ecx
	vcvttsd2si (%ecx),%ecx

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
	vcmpltsd %xmm4,%xmm6,%xmm2
	vcmpltsd (%ecx),%xmm6,%xmm2
	vcmplesd %xmm4,%xmm6,%xmm2
	vcmplesd (%ecx),%xmm6,%xmm2
	vcmpunordsd %xmm4,%xmm6,%xmm2
	vcmpunordsd (%ecx),%xmm6,%xmm2
	vcmpneqsd %xmm4,%xmm6,%xmm2
	vcmpneqsd (%ecx),%xmm6,%xmm2
	vcmpnltsd %xmm4,%xmm6,%xmm2
	vcmpnltsd (%ecx),%xmm6,%xmm2
	vcmpnlesd %xmm4,%xmm6,%xmm2
	vcmpnlesd (%ecx),%xmm6,%xmm2
	vcmpordsd %xmm4,%xmm6,%xmm2
	vcmpordsd (%ecx),%xmm6,%xmm2
	vcmpeq_uqsd %xmm4,%xmm6,%xmm2
	vcmpeq_uqsd (%ecx),%xmm6,%xmm2
	vcmpngesd %xmm4,%xmm6,%xmm2
	vcmpngesd (%ecx),%xmm6,%xmm2
	vcmpngtsd %xmm4,%xmm6,%xmm2
	vcmpngtsd (%ecx),%xmm6,%xmm2
	vcmpfalsesd %xmm4,%xmm6,%xmm2
	vcmpfalsesd (%ecx),%xmm6,%xmm2
	vcmpneq_oqsd %xmm4,%xmm6,%xmm2
	vcmpneq_oqsd (%ecx),%xmm6,%xmm2
	vcmpgesd %xmm4,%xmm6,%xmm2
	vcmpgesd (%ecx),%xmm6,%xmm2
	vcmpgtsd %xmm4,%xmm6,%xmm2
	vcmpgtsd (%ecx),%xmm6,%xmm2
	vcmptruesd %xmm4,%xmm6,%xmm2
	vcmptruesd (%ecx),%xmm6,%xmm2
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
	vcmpltss %xmm4,%xmm6,%xmm2
	vcmpltss (%ecx),%xmm6,%xmm2
	vcmpless %xmm4,%xmm6,%xmm2
	vcmpless (%ecx),%xmm6,%xmm2
	vcmpunordss %xmm4,%xmm6,%xmm2
	vcmpunordss (%ecx),%xmm6,%xmm2
	vcmpneqss %xmm4,%xmm6,%xmm2
	vcmpneqss (%ecx),%xmm6,%xmm2
	vcmpnltss %xmm4,%xmm6,%xmm2
	vcmpnltss (%ecx),%xmm6,%xmm2
	vcmpnless %xmm4,%xmm6,%xmm2
	vcmpnless (%ecx),%xmm6,%xmm2
	vcmpordss %xmm4,%xmm6,%xmm2
	vcmpordss (%ecx),%xmm6,%xmm2
	vcmpeq_uqss %xmm4,%xmm6,%xmm2
	vcmpeq_uqss (%ecx),%xmm6,%xmm2
	vcmpngess %xmm4,%xmm6,%xmm2
	vcmpngess (%ecx),%xmm6,%xmm2
	vcmpngtss %xmm4,%xmm6,%xmm2
	vcmpngtss (%ecx),%xmm6,%xmm2
	vcmpfalsess %xmm4,%xmm6,%xmm2
	vcmpfalsess (%ecx),%xmm6,%xmm2
	vcmpneq_oqss %xmm4,%xmm6,%xmm2
	vcmpneq_oqss (%ecx),%xmm6,%xmm2
	vcmpgess %xmm4,%xmm6,%xmm2
	vcmpgess (%ecx),%xmm6,%xmm2
	vcmpgtss %xmm4,%xmm6,%xmm2
	vcmpgtss (%ecx),%xmm6,%xmm2
	vcmptruess %xmm4,%xmm6,%xmm2
	vcmptruess (%ecx),%xmm6,%xmm2
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

# Tests for op xmm/mem32, xmm
	vcomiss %xmm4,%xmm6
	vcomiss (%ecx),%xmm4
	vucomiss %xmm4,%xmm6
	vucomiss (%ecx),%xmm4

# Tests for op mem32, xmm
	vmovss (%ecx),%xmm4

# Tests for op xmm, mem32
	vmovss %xmm4,(%ecx)

# Tests for op xmm/mem32, regl
	vcvtss2si %xmm4,%ecx
	vcvtss2si (%ecx),%ecx
	vcvttss2si %xmm4,%ecx
	vcvttss2si (%ecx),%ecx

# Tests for op regl/mem32, xmm, xmm
	vcvtsi2sd %ecx,%xmm4,%xmm6
	vcvtsi2sd (%ecx),%xmm4,%xmm6
	vcvtsi2ss %ecx,%xmm4,%xmm6
	vcvtsi2ss (%ecx),%xmm4,%xmm6

# Tests for op imm8, xmm/mem32, xmm, xmm
	vcmpss $7,%xmm4,%xmm6,%xmm2
	vcmpss $7,(%ecx),%xmm6,%xmm2
	vroundss $7,%xmm4,%xmm6,%xmm2
	vroundss $7,(%ecx),%xmm6,%xmm2

# Tests for op xmm, xmm, xmm
	vmovsd %xmm4,%xmm6,%xmm2
	vmovss %xmm4,%xmm6,%xmm2

#Tests with different memory and register operands.
	vcvtsi2sdl 0x1234,%xmm0,%xmm7
	vcvtsi2sdl (%ebp),%xmm0,%xmm7
	vcvtsi2sdl (%esp),%xmm0,%xmm7
	vcvtsi2sdl 0x99(%ebp),%xmm0,%xmm7
	vcvtsi2sdl 0x99(,%eiz),%xmm0,%xmm7
	vcvtsi2sdl 0x99(,%eiz,2),%xmm0,%xmm7
	vcvtsi2sdl 0x99(%eax,%eiz),%xmm0,%xmm7
	vcvtsi2sdl 0x99(%eax,%eiz,2),%xmm0,%xmm7
	vcvtsi2sdl 0x99(%eax,%ebx,4),%xmm0,%xmm7
	vcvtsi2sdl 0x99(%esp,%ecx,8),%xmm0,%xmm7
	vcvtsi2sdl 0x99(%ebp,%edx,1),%xmm0,%xmm7

	.intel_syntax noprefix

# Tests for op xmm/mem64, xmm
	vcomisd xmm6,xmm4
	vcomisd xmm4,QWORD PTR [ecx]
	vcomisd xmm4,[ecx]
	vucomisd xmm6,xmm4
	vucomisd xmm4,QWORD PTR [ecx]
	vucomisd xmm4,[ecx]

# Tests for op mem64, xmm
	vmovsd xmm4,QWORD PTR [ecx]
	vmovsd xmm4,[ecx]

# Tests for op xmm, mem64
	vmovsd QWORD PTR [ecx],xmm4
	vmovsd [ecx],xmm4

# Tests for op xmm/mem64, regl
	vcvtsd2si ecx,xmm4
	vcvtsd2si ecx,QWORD PTR [ecx]
	vcvtsd2si ecx,[ecx]
	vcvttsd2si ecx,xmm4
	vcvttsd2si ecx,QWORD PTR [ecx]
	vcvttsd2si ecx,[ecx]

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

# Tests for op xmm/mem32, xmm
	vcomiss xmm6,xmm4
	vcomiss xmm4,DWORD PTR [ecx]
	vcomiss xmm4,[ecx]
	vucomiss xmm6,xmm4
	vucomiss xmm4,DWORD PTR [ecx]
	vucomiss xmm4,[ecx]

# Tests for op mem32, xmm
	vmovss xmm4,DWORD PTR [ecx]
	vmovss xmm4,[ecx]

# Tests for op xmm, mem32
	vmovss DWORD PTR [ecx],xmm4
	vmovss [ecx],xmm4

# Tests for op xmm/mem32, regl
	vcvtss2si ecx,xmm4
	vcvtss2si ecx,DWORD PTR [ecx]
	vcvtss2si ecx,[ecx]
	vcvttss2si ecx,xmm4
	vcvttss2si ecx,DWORD PTR [ecx]
	vcvttss2si ecx,[ecx]

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
	vroundss xmm2,xmm6,xmm4,7
	vroundss xmm2,xmm6,DWORD PTR [ecx],7
	vroundss xmm2,xmm6,[ecx],7

# Tests for op xmm, xmm, xmm
	vmovsd xmm2,xmm6,xmm4
	vmovss xmm2,xmm6,xmm4

#Tests with different memory and register operands.
	vcvtsi2sd xmm7,xmm0,DWORD PTR ds:0x1234
	vcvtsi2sd xmm7,xmm0,DWORD PTR [ebp]
	vcvtsi2sd xmm7,xmm0,DWORD PTR [ebp+0x99]
	vcvtsi2sd xmm7,xmm0,DWORD PTR [eiz*1+0x99]
	vcvtsi2sd xmm7,xmm0,DWORD PTR [eiz*2+0x99]
	vcvtsi2sd xmm7,xmm0,DWORD PTR [eax+eiz*1+0x99]
	vcvtsi2sd xmm7,xmm0,DWORD PTR [eax+eiz*2+0x99]
	vcvtsi2sd xmm7,xmm0,DWORD PTR [eax+ebx*4+0x99]
	vcvtsi2sd xmm7,xmm0,DWORD PTR [esp+ecx*8+0x99]
	vcvtsi2sd xmm7,xmm0,DWORD PTR [ebp+edx*1+0x99]
