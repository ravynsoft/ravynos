# Check i386 AVX2 instructions

	.allow_index_reg
	.text
_start:

# Tests for op mem256, mask,  ymm
# Tests for op ymm, mask, mem256
	vpmaskmovd (%ecx),%ymm4,%ymm6
	vpmaskmovd %ymm4,%ymm6,(%ecx)
	vpmaskmovq (%ecx),%ymm4,%ymm6
	vpmaskmovq %ymm4,%ymm6,(%ecx)

# Tests for op imm8, ymm/mem256, ymm
	vpermpd $7,%ymm6,%ymm2
	vpermpd $7,(%ecx),%ymm6
	vpermq $7,%ymm6,%ymm2
	vpermq $7,(%ecx),%ymm6

# Tests for op ymm/mem256, ymm, ymm
	vpermd %ymm4,%ymm6,%ymm2
	vpermd (%ecx),%ymm6,%ymm2
	vpermps %ymm4,%ymm6,%ymm2
	vpermps (%ecx),%ymm6,%ymm2
	vpsllvd %ymm4,%ymm6,%ymm2
	vpsllvd (%ecx),%ymm6,%ymm2
	vpsllvq %ymm4,%ymm6,%ymm2
	vpsllvq (%ecx),%ymm6,%ymm2
	vpsravd %ymm4,%ymm6,%ymm2
	vpsravd (%ecx),%ymm6,%ymm2
	vpsrlvd %ymm4,%ymm6,%ymm2
	vpsrlvd (%ecx),%ymm6,%ymm2
	vpsrlvq %ymm4,%ymm6,%ymm2
	vpsrlvq (%ecx),%ymm6,%ymm2

# Tests for op mem256, ymm
	vmovntdqa (%ecx),%ymm4

# Tests for op ymm, xmm
	vbroadcastsd %xmm4,%ymm6
	vbroadcastss %xmm4,%ymm6

# Tests for op imm8, ymm/mem256, ymm, ymm
	vpblendd $7,%ymm4,%ymm6,%ymm2
	vpblendd $7,(%ecx),%ymm6,%ymm2
	vperm2i128 $7,%ymm4,%ymm6,%ymm2
	vperm2i128 $7,(%ecx),%ymm6,%ymm2

# Tests for op imm8, xmm/mem128, ymm, ymm
	vinserti128 $7,%xmm4,%ymm4,%ymm6
	vinserti128 $7,(%ecx),%ymm4,%ymm6

# Tests for op mem128, ymm
	vbroadcasti128 (%ecx),%ymm4

# Tests for op xmm/mem128, xmm, xmm
	vpsllvd %xmm4,%xmm6,%xmm2
	vpsllvd (%ecx),%xmm6,%xmm7
	vpsllvq %xmm4,%xmm6,%xmm2
	vpsllvq (%ecx),%xmm6,%xmm7
	vpsravd %xmm4,%xmm6,%xmm2
	vpsravd (%ecx),%xmm6,%xmm7
	vpsrlvd %xmm4,%xmm6,%xmm2
	vpsrlvd (%ecx),%xmm6,%xmm7
	vpsrlvq %xmm4,%xmm6,%xmm2
	vpsrlvq (%ecx),%xmm6,%xmm7

# Tests for op mem128, xmm, xmm
	vpmaskmovd (%ecx),%xmm4,%xmm6
	vpmaskmovq (%ecx),%xmm4,%xmm6

# Tests for op imm8, ymm, xmm128/mem
	vextracti128 $7,%ymm4,%xmm6
	vextracti128 $7,%ymm4,(%ecx)

# Tests for op xmm, xmm, mem128
	vpmaskmovd %xmm4,%xmm6,(%ecx)
	vpmaskmovq %xmm4,%xmm6,(%ecx)

# Tests for op imm8, xmm/mem128, xmm, xmm
	vpblendd $7,%xmm4,%xmm6,%xmm2
	vpblendd $7,(%ecx),%xmm6,%xmm2

# Tests for op xmm/mem64, xmm
	vpbroadcastq %xmm4,%xmm6
	vpbroadcastq (%ecx),%xmm4

# Tests for op xmm/mem64, ymm
	vpbroadcastq %xmm4,%ymm6
	vpbroadcastq (%ecx),%ymm4

# Tests for op xmm/mem32, ymm
	vpbroadcastd %xmm4,%ymm4
	vpbroadcastd (%ecx),%ymm4

# Tests for op xmm/mem32, xmm
	vpbroadcastd %xmm4,%xmm6
	vpbroadcastd (%ecx),%xmm4

# Tests for op xmm/m16, xmm
	vpbroadcastw %xmm4,%xmm6
	vpbroadcastw (%ecx),%xmm4

# Tests for op xmm/m16, ymm
	vpbroadcastw %xmm4,%ymm6
	vpbroadcastw (%ecx),%ymm4

# Tests for op xmm/m8, xmm
	vpbroadcastb %xmm4,%xmm6
	vpbroadcastb (%ecx),%xmm4

# Tests for op xmm/m8, ymm
	vpbroadcastb %xmm4,%ymm6
	vpbroadcastb (%ecx),%ymm4

# Tests for op xmm, xmm
	vbroadcastss %xmm4,%xmm6

	.intel_syntax noprefix

# Tests for op mem256, mask,  ymm
# Tests for op ymm, mask, mem256
	vpmaskmovd ymm6,ymm4,YMMWORD PTR [ecx]
	vpmaskmovd YMMWORD PTR [ecx],ymm6,ymm4
	vpmaskmovd ymm6,ymm4,[ecx]
	vpmaskmovd [ecx],ymm6,ymm4
	vpmaskmovq ymm6,ymm4,YMMWORD PTR [ecx]
	vpmaskmovq YMMWORD PTR [ecx],ymm6,ymm4
	vpmaskmovq ymm6,ymm4,[ecx]
	vpmaskmovq [ecx],ymm6,ymm4

# Tests for op imm8, ymm/mem256, ymm
	vpermpd ymm2,ymm6,7
	vpermpd ymm6,YMMWORD PTR [ecx],7
	vpermpd ymm6,[ecx],7
	vpermq ymm2,ymm6,7
	vpermq ymm6,YMMWORD PTR [ecx],7
	vpermq ymm6,[ecx],7

# Tests for op ymm/mem256, ymm, ymm
	vpermd ymm2,ymm6,ymm4
	vpermd ymm2,ymm6,YMMWORD PTR [ecx]
	vpermd ymm2,ymm6,[ecx]
	vpermps ymm2,ymm6,ymm4
	vpermps ymm2,ymm6,YMMWORD PTR [ecx]
	vpermps ymm2,ymm6,[ecx]
	vpsllvd ymm2,ymm6,ymm4
	vpsllvd ymm2,ymm6,YMMWORD PTR [ecx]
	vpsllvd ymm2,ymm6,[ecx]
	vpsllvq ymm2,ymm6,ymm4
	vpsllvq ymm2,ymm6,YMMWORD PTR [ecx]
	vpsllvq ymm2,ymm6,[ecx]
	vpsravd ymm2,ymm6,ymm4
	vpsravd ymm2,ymm6,YMMWORD PTR [ecx]
	vpsravd ymm2,ymm6,[ecx]
	vpsrlvd ymm2,ymm6,ymm4
	vpsrlvd ymm2,ymm6,YMMWORD PTR [ecx]
	vpsrlvd ymm2,ymm6,[ecx]
	vpsrlvq ymm2,ymm6,ymm4
	vpsrlvq ymm2,ymm6,YMMWORD PTR [ecx]
	vpsrlvq ymm2,ymm6,[ecx]

# Tests for op mem256, ymm
	vmovntdqa ymm4,YMMWORD PTR [ecx]
	vmovntdqa ymm4,[ecx]

# Tests for op ymm, xmm
	vbroadcastsd ymm6,xmm4
	vbroadcastss ymm6,xmm4

# Tests for op imm8, ymm/mem256, ymm, ymm
	vpblendd ymm2,ymm6,ymm4,7
	vpblendd ymm2,ymm6,YMMWORD PTR [ecx],7
	vpblendd ymm2,ymm6,[ecx],7
	vperm2i128 ymm2,ymm6,ymm4,7
	vperm2i128 ymm2,ymm6,YMMWORD PTR [ecx],7
	vperm2i128 ymm2,ymm6,[ecx],7

# Tests for op imm8, xmm/mem128, ymm, ymm
	vinserti128 ymm6,ymm4,xmm4,7
	vinserti128 ymm6,ymm4,XMMWORD PTR [ecx],7
	vinserti128 ymm6,ymm4,[ecx],7

# Tests for op mem128, ymm
	vbroadcasti128 ymm4,XMMWORD PTR [ecx]
	vbroadcasti128 ymm4,[ecx]

# Tests for op xmm/mem128, xmm, xmm
	vpsllvd xmm2,xmm6,xmm4
	vpsllvd xmm7,xmm6,XMMWORD PTR [ecx]
	vpsllvd xmm7,xmm6,[ecx]
	vpsllvq xmm2,xmm6,xmm4
	vpsllvq xmm7,xmm6,XMMWORD PTR [ecx]
	vpsllvq xmm7,xmm6,[ecx]
	vpsravd xmm2,xmm6,xmm4
	vpsravd xmm7,xmm6,XMMWORD PTR [ecx]
	vpsravd xmm7,xmm6,[ecx]
	vpsrlvd xmm2,xmm6,xmm4
	vpsrlvd xmm7,xmm6,XMMWORD PTR [ecx]
	vpsrlvd xmm7,xmm6,[ecx]
	vpsrlvq xmm2,xmm6,xmm4
	vpsrlvq xmm7,xmm6,XMMWORD PTR [ecx]
	vpsrlvq xmm7,xmm6,[ecx]

# Tests for op mem128, xmm, xmm
	vpmaskmovd xmm6,xmm4,XMMWORD PTR [ecx]
	vpmaskmovd xmm6,xmm4,[ecx]
	vpmaskmovq xmm6,xmm4,XMMWORD PTR [ecx]
	vpmaskmovq xmm6,xmm4,[ecx]

# Tests for op imm8, ymm, xmm128/mem
	vextracti128 xmm6,ymm4,7
	vextracti128 XMMWORD PTR [ecx],ymm4,7
	vextracti128 [ecx],ymm4,7

# Tests for op xmm, xmm, mem128
	vpmaskmovd XMMWORD PTR [ecx],xmm6,xmm4
	vpmaskmovd [ecx],xmm6,xmm4
	vpmaskmovq XMMWORD PTR [ecx],xmm6,xmm4
	vpmaskmovq [ecx],xmm6,xmm4

# Tests for op imm8, xmm/mem128, xmm, xmm
	vpblendd xmm2,xmm6,xmm4,7
	vpblendd xmm2,xmm6,XMMWORD PTR [ecx],7
	vpblendd xmm2,xmm6,[ecx],7

# Tests for op xmm/mem64, xmm
	vpbroadcastq xmm6,xmm4
	vpbroadcastq xmm4,QWORD PTR [ecx]
	vpbroadcastq xmm4,[ecx]

# Tests for op xmm/mem64, ymm
	vpbroadcastq ymm6,xmm4
	vpbroadcastq ymm4,QWORD PTR [ecx]
	vpbroadcastq ymm4,[ecx]

# Tests for op xmm/mem32, ymm
	vpbroadcastd ymm4,xmm4
	vpbroadcastd ymm4,DWORD PTR [ecx]
	vpbroadcastd ymm4,[ecx]

# Tests for op xmm/mem32, xmm
	vpbroadcastd xmm6,xmm4
	vpbroadcastd xmm4,DWORD PTR [ecx]
	vpbroadcastd xmm4,[ecx]

# Tests for op xmm/m16, xmm
	vpbroadcastw xmm6,xmm4
	vpbroadcastw xmm4,WORD PTR [ecx]
	vpbroadcastw xmm4,[ecx]

# Tests for op xmm/m16, ymm
	vpbroadcastw ymm6,xmm4
	vpbroadcastw ymm4,WORD PTR [ecx]
	vpbroadcastw ymm4,[ecx]

# Tests for op xmm/m8, xmm
	vpbroadcastb xmm6,xmm4
	vpbroadcastb xmm4,BYTE PTR [ecx]
	vpbroadcastb xmm4,[ecx]

# Tests for op xmm/m8, ymm
	vpbroadcastb ymm6,xmm4
	vpbroadcastb ymm4,BYTE PTR [ecx]
	vpbroadcastb ymm4,[ecx]

# Tests for op xmm, xmm
	vbroadcastss xmm6,xmm4
