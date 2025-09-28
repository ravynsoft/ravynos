# Check 64bit AVX scalar instructions

	.allow_index_reg
	.text
_start:

# Tests for op xmm/mem64, xmm
	vcomisd %xmm4,%xmm6
	vcomisd (%rcx),%xmm4
	vucomisd %xmm4,%xmm6
	vucomisd (%rcx),%xmm4

# Tests for op mem64, xmm
	vmovsd (%rcx),%xmm4

# Tests for op xmm, mem64
	vmovsd %xmm4,(%rcx)

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

# Tests for op xmm/mem32, xmm
	vcomiss %xmm4,%xmm6
	vcomiss (%rcx),%xmm4
	vucomiss %xmm4,%xmm6
	vucomiss (%rcx),%xmm4

# Tests for op mem32, xmm
	vmovss (%rcx),%xmm4

# Tests for op xmm, mem32
	vmovss %xmm4,(%rcx)

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

# Tests for op regl/mem32, xmm, xmm
	vcvtsi2sd %ecx,%xmm4,%xmm6
	vcvtsi2sdl (%rcx),%xmm4,%xmm6
	vcvtsi2ss %ecx,%xmm4,%xmm6
	vcvtsi2ssl (%rcx),%xmm4,%xmm6

# Tests for op imm8, xmm/mem32, xmm, xmm
	vcmpss $7,%xmm4,%xmm6,%xmm2
	vcmpss $7,(%rcx),%xmm6,%xmm2
	vroundss $7,%xmm4,%xmm6,%xmm2
	vroundss $7,(%rcx),%xmm6,%xmm2

# Tests for op xmm, xmm, xmm
	vmovsd %xmm4,%xmm6,%xmm2
	vmovss %xmm4,%xmm6,%xmm2

#Tests with different memory and register operands.
	vcvtsi2sdl 0x12345678,%xmm8,%xmm15
	vcvtsi2sdl (%rbp),%xmm8,%xmm15
	vcvtsi2sdl (%rsp),%xmm8,%xmm15
	vcvtsi2sdl 0x99(%rbp),%xmm8,%xmm15
	vcvtsi2sdl 0x99(%r15),%xmm8,%xmm15
	vcvtsi2sdl 0x99(%rip),%xmm8,%xmm15
	vcvtsi2sdl 0x99(%rsp),%xmm8,%xmm15
	vcvtsi2sdl 0x99(%r12),%xmm8,%xmm15
	vcvtsi2sdl -0x99(,%riz),%xmm8,%xmm15
	vcvtsi2sdl -0x99(,%riz,2),%xmm8,%xmm15
	vcvtsi2sdl -0x99(%rbx,%riz),%xmm8,%xmm15
	vcvtsi2sdl -0x99(%rbx,%riz,2),%xmm8,%xmm15
	vcvtsi2sdl -0x99(%r12,%r15,4),%xmm8,%xmm15
	vcvtsi2sdl -0x99(%r8,%r15,8),%xmm8,%xmm15
	vcvtsi2sdl -0x99(%rbp,%r13,4),%xmm8,%xmm15
	vcvtsi2sdl -0x99(%rsp,%r12,1),%xmm8,%xmm15
# Tests for all register operands.
	vcvtsd2si %xmm8,%r8d
	vcvtsi2sdl %r8d,%xmm8,%xmm15
# Tests for different memory/register operand
	vcvtsd2si (%rcx),%r8
	vcvtss2si (%rcx),%r8

	.intel_syntax noprefix

# Tests for op xmm/mem64, xmm
	vcomisd xmm6,xmm4
	vcomisd xmm4,QWORD PTR [rcx]
	vcomisd xmm4,[rcx]
	vucomisd xmm6,xmm4
	vucomisd xmm4,QWORD PTR [rcx]
	vucomisd xmm4,[rcx]

# Tests for op mem64, xmm
	vmovsd xmm4,QWORD PTR [rcx]
	vmovsd xmm4,[rcx]

# Tests for op xmm, mem64
	vmovsd QWORD PTR [rcx],xmm4
	vmovsd [rcx],xmm4

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

# Tests for op xmm/mem32, xmm
	vcomiss xmm6,xmm4
	vcomiss xmm4,DWORD PTR [rcx]
	vcomiss xmm4,[rcx]
	vucomiss xmm6,xmm4
	vucomiss xmm4,DWORD PTR [rcx]
	vucomiss xmm4,[rcx]

# Tests for op mem32, xmm
	vmovss xmm4,DWORD PTR [rcx]
	vmovss xmm4,[rcx]

# Tests for op xmm, mem32
	vmovss DWORD PTR [rcx],xmm4
	vmovss [rcx],xmm4

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

# Tests for op regl/mem32, xmm, xmm
	vcvtsi2sd xmm6,xmm4,ecx
	vcvtsi2sd xmm6,xmm4,DWORD PTR [rcx]
	vcvtsi2ss xmm6,xmm4,ecx
	vcvtsi2ss xmm6,xmm4,DWORD PTR [rcx]

# Tests for op imm8, xmm/mem32, xmm, xmm
	vcmpss xmm2,xmm6,xmm4,7
	vcmpss xmm2,xmm6,DWORD PTR [rcx],7
	vcmpss xmm2,xmm6,[rcx],7
	vroundss xmm2,xmm6,xmm4,7
	vroundss xmm2,xmm6,DWORD PTR [rcx],7
	vroundss xmm2,xmm6,[rcx],7

# Tests for op xmm, xmm, xmm
	vmovsd xmm2,xmm6,xmm4
	vmovss xmm2,xmm6,xmm4

#Tests with different memory and register operands.
	vcvtsi2sd xmm15,xmm8,DWORD PTR ds:0x12345678
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rbp]
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rbp+0x99]
	vcvtsi2sd xmm15,xmm8,DWORD PTR [r15+0x99]
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rip+0x99]
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rsp+0x99]
	vcvtsi2sd xmm15,xmm8,DWORD PTR [r12+0x99]
	vcvtsi2sd xmm15,xmm8,DWORD PTR [riz*1-0x99]
	vcvtsi2sd xmm15,xmm8,DWORD PTR [riz*2-0x99]
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rbx+riz*1-0x99]
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rbx+riz*2-0x99]
	vcvtsi2sd xmm15,xmm8,DWORD PTR [r12+r15*4-0x99]
	vcvtsi2sd xmm15,xmm8,DWORD PTR [r8+r15*8-0x99]
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rbp+r12*4-0x99]
	vcvtsi2sd xmm15,xmm8,DWORD PTR [rsp+r13*1-0x99]
# Tests for all register operands.
	vcvtsd2si r8d,xmm8
	vcvtsi2sd xmm15,xmm8,r8d
# Tests for different memory/register operand
	vcvtsd2si r8,QWORD PTR  [rcx]
	vcvtss2si r8,DWORD PTR  [rcx]
