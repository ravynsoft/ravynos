# Check i386 256bit integer AVX instructions

	.allow_index_reg
	.text
_start:

# Tests for op ymm, regl
	vpmovmskb %ymm4,%ecx

# Tests for op imm8, ymm, ymm
	vpslld $7,%ymm6,%ymm2
	vpslldq $7,%ymm6,%ymm2
	vpsllq $7,%ymm6,%ymm2
	vpsllw $7,%ymm6,%ymm2
	vpsrad $7,%ymm6,%ymm2
	vpsraw $7,%ymm6,%ymm2
	vpsrld $7,%ymm6,%ymm2
	vpsrldq $7,%ymm6,%ymm2
	vpsrlq $7,%ymm6,%ymm2
	vpsrlw $7,%ymm6,%ymm2

# Tests for op imm8, ymm/mem256, ymm
	vpshufd $7,%ymm6,%ymm2
	vpshufd $7,(%ecx),%ymm6
	vpshufhw $7,%ymm6,%ymm2
	vpshufhw $7,(%ecx),%ymm6
	vpshuflw $7,%ymm6,%ymm2
	vpshuflw $7,(%ecx),%ymm6

# Tests for op ymm/mem256, ymm, ymm
	vpackssdw %ymm4,%ymm6,%ymm2
	vpackssdw (%ecx),%ymm6,%ymm2
	vpacksswb %ymm4,%ymm6,%ymm2
	vpacksswb (%ecx),%ymm6,%ymm2
	vpackusdw %ymm4,%ymm6,%ymm2
	vpackusdw (%ecx),%ymm6,%ymm2
	vpackuswb %ymm4,%ymm6,%ymm2
	vpackuswb (%ecx),%ymm6,%ymm2
	vpaddb %ymm4,%ymm6,%ymm2
	vpaddb (%ecx),%ymm6,%ymm2
	vpaddw %ymm4,%ymm6,%ymm2
	vpaddw (%ecx),%ymm6,%ymm2
	vpaddd %ymm4,%ymm6,%ymm2
	vpaddd (%ecx),%ymm6,%ymm2
	vpaddq %ymm4,%ymm6,%ymm2
	vpaddq (%ecx),%ymm6,%ymm2
	vpaddsb %ymm4,%ymm6,%ymm2
	vpaddsb (%ecx),%ymm6,%ymm2
	vpaddsw %ymm4,%ymm6,%ymm2
	vpaddsw (%ecx),%ymm6,%ymm2
	vpaddusb %ymm4,%ymm6,%ymm2
	vpaddusb (%ecx),%ymm6,%ymm2
	vpaddusw %ymm4,%ymm6,%ymm2
	vpaddusw (%ecx),%ymm6,%ymm2
	vpand %ymm4,%ymm6,%ymm2
	vpand (%ecx),%ymm6,%ymm2
	vpandn %ymm4,%ymm6,%ymm2
	vpandn (%ecx),%ymm6,%ymm2
	vpavgb %ymm4,%ymm6,%ymm2
	vpavgb (%ecx),%ymm6,%ymm2
	vpavgw %ymm4,%ymm6,%ymm2
	vpavgw (%ecx),%ymm6,%ymm2
	vpcmpeqb %ymm4,%ymm6,%ymm2
	vpcmpeqb (%ecx),%ymm6,%ymm2
	vpcmpeqw %ymm4,%ymm6,%ymm2
	vpcmpeqw (%ecx),%ymm6,%ymm2
	vpcmpeqd %ymm4,%ymm6,%ymm2
	vpcmpeqd (%ecx),%ymm6,%ymm2
	vpcmpeqq %ymm4,%ymm6,%ymm2
	vpcmpeqq (%ecx),%ymm6,%ymm2
	vpcmpgtb %ymm4,%ymm6,%ymm2
	vpcmpgtb (%ecx),%ymm6,%ymm2
	vpcmpgtw %ymm4,%ymm6,%ymm2
	vpcmpgtw (%ecx),%ymm6,%ymm2
	vpcmpgtd %ymm4,%ymm6,%ymm2
	vpcmpgtd (%ecx),%ymm6,%ymm2
	vpcmpgtq %ymm4,%ymm6,%ymm2
	vpcmpgtq (%ecx),%ymm6,%ymm2
	vphaddw %ymm4,%ymm6,%ymm2
	vphaddw (%ecx),%ymm6,%ymm2
	vphaddd %ymm4,%ymm6,%ymm2
	vphaddd (%ecx),%ymm6,%ymm2
	vphaddsw %ymm4,%ymm6,%ymm2
	vphaddsw (%ecx),%ymm6,%ymm2
	vphsubw %ymm4,%ymm6,%ymm2
	vphsubw (%ecx),%ymm6,%ymm2
	vphsubd %ymm4,%ymm6,%ymm2
	vphsubd (%ecx),%ymm6,%ymm2
	vphsubsw %ymm4,%ymm6,%ymm2
	vphsubsw (%ecx),%ymm6,%ymm2
	vpmaddwd %ymm4,%ymm6,%ymm2
	vpmaddwd (%ecx),%ymm6,%ymm2
	vpmaddubsw %ymm4,%ymm6,%ymm2
	vpmaddubsw (%ecx),%ymm6,%ymm2
	vpmaxsb %ymm4,%ymm6,%ymm2
	vpmaxsb (%ecx),%ymm6,%ymm2
	vpmaxsw %ymm4,%ymm6,%ymm2
	vpmaxsw (%ecx),%ymm6,%ymm2
	vpmaxsd %ymm4,%ymm6,%ymm2
	vpmaxsd (%ecx),%ymm6,%ymm2
	vpmaxub %ymm4,%ymm6,%ymm2
	vpmaxub (%ecx),%ymm6,%ymm2
	vpmaxuw %ymm4,%ymm6,%ymm2
	vpmaxuw (%ecx),%ymm6,%ymm2
	vpmaxud %ymm4,%ymm6,%ymm2
	vpmaxud (%ecx),%ymm6,%ymm2
	vpminsb %ymm4,%ymm6,%ymm2
	vpminsb (%ecx),%ymm6,%ymm2
	vpminsw %ymm4,%ymm6,%ymm2
	vpminsw (%ecx),%ymm6,%ymm2
	vpminsd %ymm4,%ymm6,%ymm2
	vpminsd (%ecx),%ymm6,%ymm2
	vpminub %ymm4,%ymm6,%ymm2
	vpminub (%ecx),%ymm6,%ymm2
	vpminuw %ymm4,%ymm6,%ymm2
	vpminuw (%ecx),%ymm6,%ymm2
	vpminud %ymm4,%ymm6,%ymm2
	vpminud (%ecx),%ymm6,%ymm2
	vpmulhuw %ymm4,%ymm6,%ymm2
	vpmulhuw (%ecx),%ymm6,%ymm2
	vpmulhrsw %ymm4,%ymm6,%ymm2
	vpmulhrsw (%ecx),%ymm6,%ymm2
	vpmulhw %ymm4,%ymm6,%ymm2
	vpmulhw (%ecx),%ymm6,%ymm2
	vpmullw %ymm4,%ymm6,%ymm2
	vpmullw (%ecx),%ymm6,%ymm2
	vpmulld %ymm4,%ymm6,%ymm2
	vpmulld (%ecx),%ymm6,%ymm2
	vpmuludq %ymm4,%ymm6,%ymm2
	vpmuludq (%ecx),%ymm6,%ymm2
	vpmuldq %ymm4,%ymm6,%ymm2
	vpmuldq (%ecx),%ymm6,%ymm2
	vpor %ymm4,%ymm6,%ymm2
	vpor (%ecx),%ymm6,%ymm2
	vpsadbw %ymm4,%ymm6,%ymm2
	vpsadbw (%ecx),%ymm6,%ymm2
	vpshufb %ymm4,%ymm6,%ymm2
	vpshufb (%ecx),%ymm6,%ymm2
	vpsignb %ymm4,%ymm6,%ymm2
	vpsignb (%ecx),%ymm6,%ymm2
	vpsignw %ymm4,%ymm6,%ymm2
	vpsignw (%ecx),%ymm6,%ymm2
	vpsignd %ymm4,%ymm6,%ymm2
	vpsignd (%ecx),%ymm6,%ymm2
	vpsubb %ymm4,%ymm6,%ymm2
	vpsubb (%ecx),%ymm6,%ymm2
	vpsubw %ymm4,%ymm6,%ymm2
	vpsubw (%ecx),%ymm6,%ymm2
	vpsubd %ymm4,%ymm6,%ymm2
	vpsubd (%ecx),%ymm6,%ymm2
	vpsubq %ymm4,%ymm6,%ymm2
	vpsubq (%ecx),%ymm6,%ymm2
	vpsubsb %ymm4,%ymm6,%ymm2
	vpsubsb (%ecx),%ymm6,%ymm2
	vpsubsw %ymm4,%ymm6,%ymm2
	vpsubsw (%ecx),%ymm6,%ymm2
	vpsubusb %ymm4,%ymm6,%ymm2
	vpsubusb (%ecx),%ymm6,%ymm2
	vpsubusw %ymm4,%ymm6,%ymm2
	vpsubusw (%ecx),%ymm6,%ymm2
	vpunpckhbw %ymm4,%ymm6,%ymm2
	vpunpckhbw (%ecx),%ymm6,%ymm2
	vpunpckhwd %ymm4,%ymm6,%ymm2
	vpunpckhwd (%ecx),%ymm6,%ymm2
	vpunpckhdq %ymm4,%ymm6,%ymm2
	vpunpckhdq (%ecx),%ymm6,%ymm2
	vpunpckhqdq %ymm4,%ymm6,%ymm2
	vpunpckhqdq (%ecx),%ymm6,%ymm2
	vpunpcklbw %ymm4,%ymm6,%ymm2
	vpunpcklbw (%ecx),%ymm6,%ymm2
	vpunpcklwd %ymm4,%ymm6,%ymm2
	vpunpcklwd (%ecx),%ymm6,%ymm2
	vpunpckldq %ymm4,%ymm6,%ymm2
	vpunpckldq (%ecx),%ymm6,%ymm2
	vpunpcklqdq %ymm4,%ymm6,%ymm2
	vpunpcklqdq (%ecx),%ymm6,%ymm2
	vpxor %ymm4,%ymm6,%ymm2
	vpxor (%ecx),%ymm6,%ymm2

# Tests for op ymm/mem256, ymm
	vpabsb %ymm4,%ymm6
	vpabsb (%ecx),%ymm4
	vpabsw %ymm4,%ymm6
	vpabsw (%ecx),%ymm4
	vpabsd %ymm4,%ymm6
	vpabsd (%ecx),%ymm4

# Tests for op imm8, ymm/mem256, ymm, ymm
	vmpsadbw $7,%ymm4,%ymm6,%ymm2
	vmpsadbw $7,(%ecx),%ymm6,%ymm2
	vpalignr $7,%ymm4,%ymm6,%ymm2
	vpalignr $7,(%ecx),%ymm6,%ymm2
	vpblendw $7,%ymm4,%ymm6,%ymm2
	vpblendw $7,(%ecx),%ymm6,%ymm2

# Tests for op ymm, ymm/mem256, ymm, ymm
	vpblendvb %ymm4,%ymm6,%ymm2,%ymm7
	vpblendvb %ymm4,(%ecx),%ymm2,%ymm7

# Tests for op xmm/mem128, ymm, ymm
	vpsllw %xmm4,%ymm6,%ymm2
	vpsllw (%ecx),%ymm6,%ymm2
	vpslld %xmm4,%ymm6,%ymm2
	vpslld (%ecx),%ymm6,%ymm2
	vpsllq %xmm4,%ymm6,%ymm2
	vpsllq (%ecx),%ymm6,%ymm2
	vpsraw %xmm4,%ymm6,%ymm2
	vpsraw (%ecx),%ymm6,%ymm2
	vpsrad %xmm4,%ymm6,%ymm2
	vpsrad (%ecx),%ymm6,%ymm2
	vpsrlw %xmm4,%ymm6,%ymm2
	vpsrlw (%ecx),%ymm6,%ymm2
	vpsrld %xmm4,%ymm6,%ymm2
	vpsrld (%ecx),%ymm6,%ymm2
	vpsrlq %xmm4,%ymm6,%ymm2
	vpsrlq (%ecx),%ymm6,%ymm2

# Tests for op xmm/mem128, ymm
	vpmovsxbw %xmm4,%ymm4
	vpmovsxbw (%ecx),%ymm4
	vpmovsxwd %xmm4,%ymm4
	vpmovsxwd (%ecx),%ymm4
	vpmovsxdq %xmm4,%ymm4
	vpmovsxdq (%ecx),%ymm4
	vpmovzxbw %xmm4,%ymm4
	vpmovzxbw (%ecx),%ymm4
	vpmovzxwd %xmm4,%ymm4
	vpmovzxwd (%ecx),%ymm4
	vpmovzxdq %xmm4,%ymm4
	vpmovzxdq (%ecx),%ymm4

# Tests for op xmm/mem64, ymm
	vpmovsxbd %xmm4,%ymm6
	vpmovsxbd (%ecx),%ymm4
	vpmovsxwq %xmm4,%ymm6
	vpmovsxwq (%ecx),%ymm4
	vpmovzxbd %xmm4,%ymm6
	vpmovzxbd (%ecx),%ymm4
	vpmovzxwq %xmm4,%ymm6
	vpmovzxwq (%ecx),%ymm4

# Tests for op xmm/mem32, ymm
	vpmovsxbq %xmm4,%ymm4
	vpmovsxbq (%ecx),%ymm4
	vpmovzxbq %xmm4,%ymm4
	vpmovzxbq (%ecx),%ymm4

	.intel_syntax noprefix

# Tests for op ymm, regl
	vpmovmskb ecx,ymm4

# Tests for op imm8, ymm, ymm
	vpslld ymm2,ymm6,7
	vpslldq ymm2,ymm6,7
	vpsllq ymm2,ymm6,7
	vpsllw ymm2,ymm6,7
	vpsrad ymm2,ymm6,7
	vpsraw ymm2,ymm6,7
	vpsrld ymm2,ymm6,7
	vpsrldq ymm2,ymm6,7
	vpsrlq ymm2,ymm6,7
	vpsrlw ymm2,ymm6,7

# Tests for op imm8, ymm/mem256, ymm
	vpshufd ymm2,ymm6,7
	vpshufd ymm6,YMMWORD PTR [ecx],7
	vpshufd ymm6,[ecx],7
	vpshufhw ymm2,ymm6,7
	vpshufhw ymm6,YMMWORD PTR [ecx],7
	vpshufhw ymm6,[ecx],7
	vpshuflw ymm2,ymm6,7
	vpshuflw ymm6,YMMWORD PTR [ecx],7
	vpshuflw ymm6,[ecx],7

# Tests for op ymm/mem256, ymm, ymm
	vpackssdw ymm2,ymm6,ymm4
	vpackssdw ymm2,ymm6,YMMWORD PTR [ecx]
	vpackssdw ymm2,ymm6,[ecx]
	vpacksswb ymm2,ymm6,ymm4
	vpacksswb ymm2,ymm6,YMMWORD PTR [ecx]
	vpacksswb ymm2,ymm6,[ecx]
	vpackusdw ymm2,ymm6,ymm4
	vpackusdw ymm2,ymm6,YMMWORD PTR [ecx]
	vpackusdw ymm2,ymm6,[ecx]
	vpackuswb ymm2,ymm6,ymm4
	vpackuswb ymm2,ymm6,YMMWORD PTR [ecx]
	vpackuswb ymm2,ymm6,[ecx]
	vpaddb ymm2,ymm6,ymm4
	vpaddb ymm2,ymm6,YMMWORD PTR [ecx]
	vpaddb ymm2,ymm6,[ecx]
	vpaddw ymm2,ymm6,ymm4
	vpaddw ymm2,ymm6,YMMWORD PTR [ecx]
	vpaddw ymm2,ymm6,[ecx]
	vpaddd ymm2,ymm6,ymm4
	vpaddd ymm2,ymm6,YMMWORD PTR [ecx]
	vpaddd ymm2,ymm6,[ecx]
	vpaddq ymm2,ymm6,ymm4
	vpaddq ymm2,ymm6,YMMWORD PTR [ecx]
	vpaddq ymm2,ymm6,[ecx]
	vpaddsb ymm2,ymm6,ymm4
	vpaddsb ymm2,ymm6,YMMWORD PTR [ecx]
	vpaddsb ymm2,ymm6,[ecx]
	vpaddsw ymm2,ymm6,ymm4
	vpaddsw ymm2,ymm6,YMMWORD PTR [ecx]
	vpaddsw ymm2,ymm6,[ecx]
	vpaddusb ymm2,ymm6,ymm4
	vpaddusb ymm2,ymm6,YMMWORD PTR [ecx]
	vpaddusb ymm2,ymm6,[ecx]
	vpaddusw ymm2,ymm6,ymm4
	vpaddusw ymm2,ymm6,YMMWORD PTR [ecx]
	vpaddusw ymm2,ymm6,[ecx]
	vpand ymm2,ymm6,ymm4
	vpand ymm2,ymm6,YMMWORD PTR [ecx]
	vpand ymm2,ymm6,[ecx]
	vpandn ymm2,ymm6,ymm4
	vpandn ymm2,ymm6,YMMWORD PTR [ecx]
	vpandn ymm2,ymm6,[ecx]
	vpavgb ymm2,ymm6,ymm4
	vpavgb ymm2,ymm6,YMMWORD PTR [ecx]
	vpavgb ymm2,ymm6,[ecx]
	vpavgw ymm2,ymm6,ymm4
	vpavgw ymm2,ymm6,YMMWORD PTR [ecx]
	vpavgw ymm2,ymm6,[ecx]
	vpcmpeqb ymm2,ymm6,ymm4
	vpcmpeqb ymm2,ymm6,YMMWORD PTR [ecx]
	vpcmpeqb ymm2,ymm6,[ecx]
	vpcmpeqw ymm2,ymm6,ymm4
	vpcmpeqw ymm2,ymm6,YMMWORD PTR [ecx]
	vpcmpeqw ymm2,ymm6,[ecx]
	vpcmpeqd ymm2,ymm6,ymm4
	vpcmpeqd ymm2,ymm6,YMMWORD PTR [ecx]
	vpcmpeqd ymm2,ymm6,[ecx]
	vpcmpeqq ymm2,ymm6,ymm4
	vpcmpeqq ymm2,ymm6,YMMWORD PTR [ecx]
	vpcmpeqq ymm2,ymm6,[ecx]
	vpcmpgtb ymm2,ymm6,ymm4
	vpcmpgtb ymm2,ymm6,YMMWORD PTR [ecx]
	vpcmpgtb ymm2,ymm6,[ecx]
	vpcmpgtw ymm2,ymm6,ymm4
	vpcmpgtw ymm2,ymm6,YMMWORD PTR [ecx]
	vpcmpgtw ymm2,ymm6,[ecx]
	vpcmpgtd ymm2,ymm6,ymm4
	vpcmpgtd ymm2,ymm6,YMMWORD PTR [ecx]
	vpcmpgtd ymm2,ymm6,[ecx]
	vpcmpgtq ymm2,ymm6,ymm4
	vpcmpgtq ymm2,ymm6,YMMWORD PTR [ecx]
	vpcmpgtq ymm2,ymm6,[ecx]
	vphaddw ymm2,ymm6,ymm4
	vphaddw ymm2,ymm6,YMMWORD PTR [ecx]
	vphaddw ymm2,ymm6,[ecx]
	vphaddd ymm2,ymm6,ymm4
	vphaddd ymm2,ymm6,YMMWORD PTR [ecx]
	vphaddd ymm2,ymm6,[ecx]
	vphaddsw ymm2,ymm6,ymm4
	vphaddsw ymm2,ymm6,YMMWORD PTR [ecx]
	vphaddsw ymm2,ymm6,[ecx]
	vphsubw ymm2,ymm6,ymm4
	vphsubw ymm2,ymm6,YMMWORD PTR [ecx]
	vphsubw ymm2,ymm6,[ecx]
	vphsubd ymm2,ymm6,ymm4
	vphsubd ymm2,ymm6,YMMWORD PTR [ecx]
	vphsubd ymm2,ymm6,[ecx]
	vphsubsw ymm2,ymm6,ymm4
	vphsubsw ymm2,ymm6,YMMWORD PTR [ecx]
	vphsubsw ymm2,ymm6,[ecx]
	vpmaddwd ymm2,ymm6,ymm4
	vpmaddwd ymm2,ymm6,YMMWORD PTR [ecx]
	vpmaddwd ymm2,ymm6,[ecx]
	vpmaddubsw ymm2,ymm6,ymm4
	vpmaddubsw ymm2,ymm6,YMMWORD PTR [ecx]
	vpmaddubsw ymm2,ymm6,[ecx]
	vpmaxsb ymm2,ymm6,ymm4
	vpmaxsb ymm2,ymm6,YMMWORD PTR [ecx]
	vpmaxsb ymm2,ymm6,[ecx]
	vpmaxsw ymm2,ymm6,ymm4
	vpmaxsw ymm2,ymm6,YMMWORD PTR [ecx]
	vpmaxsw ymm2,ymm6,[ecx]
	vpmaxsd ymm2,ymm6,ymm4
	vpmaxsd ymm2,ymm6,YMMWORD PTR [ecx]
	vpmaxsd ymm2,ymm6,[ecx]
	vpmaxub ymm2,ymm6,ymm4
	vpmaxub ymm2,ymm6,YMMWORD PTR [ecx]
	vpmaxub ymm2,ymm6,[ecx]
	vpmaxuw ymm2,ymm6,ymm4
	vpmaxuw ymm2,ymm6,YMMWORD PTR [ecx]
	vpmaxuw ymm2,ymm6,[ecx]
	vpmaxud ymm2,ymm6,ymm4
	vpmaxud ymm2,ymm6,YMMWORD PTR [ecx]
	vpmaxud ymm2,ymm6,[ecx]
	vpminsb ymm2,ymm6,ymm4
	vpminsb ymm2,ymm6,YMMWORD PTR [ecx]
	vpminsb ymm2,ymm6,[ecx]
	vpminsw ymm2,ymm6,ymm4
	vpminsw ymm2,ymm6,YMMWORD PTR [ecx]
	vpminsw ymm2,ymm6,[ecx]
	vpminsd ymm2,ymm6,ymm4
	vpminsd ymm2,ymm6,YMMWORD PTR [ecx]
	vpminsd ymm2,ymm6,[ecx]
	vpminub ymm2,ymm6,ymm4
	vpminub ymm2,ymm6,YMMWORD PTR [ecx]
	vpminub ymm2,ymm6,[ecx]
	vpminuw ymm2,ymm6,ymm4
	vpminuw ymm2,ymm6,YMMWORD PTR [ecx]
	vpminuw ymm2,ymm6,[ecx]
	vpminud ymm2,ymm6,ymm4
	vpminud ymm2,ymm6,YMMWORD PTR [ecx]
	vpminud ymm2,ymm6,[ecx]
	vpmulhuw ymm2,ymm6,ymm4
	vpmulhuw ymm2,ymm6,YMMWORD PTR [ecx]
	vpmulhuw ymm2,ymm6,[ecx]
	vpmulhrsw ymm2,ymm6,ymm4
	vpmulhrsw ymm2,ymm6,YMMWORD PTR [ecx]
	vpmulhrsw ymm2,ymm6,[ecx]
	vpmulhw ymm2,ymm6,ymm4
	vpmulhw ymm2,ymm6,YMMWORD PTR [ecx]
	vpmulhw ymm2,ymm6,[ecx]
	vpmullw ymm2,ymm6,ymm4
	vpmullw ymm2,ymm6,YMMWORD PTR [ecx]
	vpmullw ymm2,ymm6,[ecx]
	vpmulld ymm2,ymm6,ymm4
	vpmulld ymm2,ymm6,YMMWORD PTR [ecx]
	vpmulld ymm2,ymm6,[ecx]
	vpmuludq ymm2,ymm6,ymm4
	vpmuludq ymm2,ymm6,YMMWORD PTR [ecx]
	vpmuludq ymm2,ymm6,[ecx]
	vpmuldq ymm2,ymm6,ymm4
	vpmuldq ymm2,ymm6,YMMWORD PTR [ecx]
	vpmuldq ymm2,ymm6,[ecx]
	vpor ymm2,ymm6,ymm4
	vpor ymm2,ymm6,YMMWORD PTR [ecx]
	vpor ymm2,ymm6,[ecx]
	vpsadbw ymm2,ymm6,ymm4
	vpsadbw ymm2,ymm6,YMMWORD PTR [ecx]
	vpsadbw ymm2,ymm6,[ecx]
	vpshufb ymm2,ymm6,ymm4
	vpshufb ymm2,ymm6,YMMWORD PTR [ecx]
	vpshufb ymm2,ymm6,[ecx]
	vpsignb ymm2,ymm6,ymm4
	vpsignb ymm2,ymm6,YMMWORD PTR [ecx]
	vpsignb ymm2,ymm6,[ecx]
	vpsignw ymm2,ymm6,ymm4
	vpsignw ymm2,ymm6,YMMWORD PTR [ecx]
	vpsignw ymm2,ymm6,[ecx]
	vpsignd ymm2,ymm6,ymm4
	vpsignd ymm2,ymm6,YMMWORD PTR [ecx]
	vpsignd ymm2,ymm6,[ecx]
	vpsubb ymm2,ymm6,ymm4
	vpsubb ymm2,ymm6,YMMWORD PTR [ecx]
	vpsubb ymm2,ymm6,[ecx]
	vpsubw ymm2,ymm6,ymm4
	vpsubw ymm2,ymm6,YMMWORD PTR [ecx]
	vpsubw ymm2,ymm6,[ecx]
	vpsubd ymm2,ymm6,ymm4
	vpsubd ymm2,ymm6,YMMWORD PTR [ecx]
	vpsubd ymm2,ymm6,[ecx]
	vpsubq ymm2,ymm6,ymm4
	vpsubq ymm2,ymm6,YMMWORD PTR [ecx]
	vpsubq ymm2,ymm6,[ecx]
	vpsubsb ymm2,ymm6,ymm4
	vpsubsb ymm2,ymm6,YMMWORD PTR [ecx]
	vpsubsb ymm2,ymm6,[ecx]
	vpsubsw ymm2,ymm6,ymm4
	vpsubsw ymm2,ymm6,YMMWORD PTR [ecx]
	vpsubsw ymm2,ymm6,[ecx]
	vpsubusb ymm2,ymm6,ymm4
	vpsubusb ymm2,ymm6,YMMWORD PTR [ecx]
	vpsubusb ymm2,ymm6,[ecx]
	vpsubusw ymm2,ymm6,ymm4
	vpsubusw ymm2,ymm6,YMMWORD PTR [ecx]
	vpsubusw ymm2,ymm6,[ecx]
	vpunpckhbw ymm2,ymm6,ymm4
	vpunpckhbw ymm2,ymm6,YMMWORD PTR [ecx]
	vpunpckhbw ymm2,ymm6,[ecx]
	vpunpckhwd ymm2,ymm6,ymm4
	vpunpckhwd ymm2,ymm6,YMMWORD PTR [ecx]
	vpunpckhwd ymm2,ymm6,[ecx]
	vpunpckhdq ymm2,ymm6,ymm4
	vpunpckhdq ymm2,ymm6,YMMWORD PTR [ecx]
	vpunpckhdq ymm2,ymm6,[ecx]
	vpunpckhqdq ymm2,ymm6,ymm4
	vpunpckhqdq ymm2,ymm6,YMMWORD PTR [ecx]
	vpunpckhqdq ymm2,ymm6,[ecx]
	vpunpcklbw ymm2,ymm6,ymm4
	vpunpcklbw ymm2,ymm6,YMMWORD PTR [ecx]
	vpunpcklbw ymm2,ymm6,[ecx]
	vpunpcklwd ymm2,ymm6,ymm4
	vpunpcklwd ymm2,ymm6,YMMWORD PTR [ecx]
	vpunpcklwd ymm2,ymm6,[ecx]
	vpunpckldq ymm2,ymm6,ymm4
	vpunpckldq ymm2,ymm6,YMMWORD PTR [ecx]
	vpunpckldq ymm2,ymm6,[ecx]
	vpunpcklqdq ymm2,ymm6,ymm4
	vpunpcklqdq ymm2,ymm6,YMMWORD PTR [ecx]
	vpunpcklqdq ymm2,ymm6,[ecx]
	vpxor ymm2,ymm6,ymm4
	vpxor ymm2,ymm6,YMMWORD PTR [ecx]
	vpxor ymm2,ymm6,[ecx]

# Tests for op ymm/mem256, ymm
	vpabsb ymm6,ymm4
	vpabsb ymm4,YMMWORD PTR [ecx]
	vpabsb ymm4,[ecx]
	vpabsw ymm6,ymm4
	vpabsw ymm4,YMMWORD PTR [ecx]
	vpabsw ymm4,[ecx]
	vpabsd ymm6,ymm4
	vpabsd ymm4,YMMWORD PTR [ecx]
	vpabsd ymm4,[ecx]

# Tests for op imm8, ymm/mem256, ymm, ymm
	vmpsadbw ymm2,ymm6,ymm4,7
	vmpsadbw ymm2,ymm6,YMMWORD PTR [ecx],7
	vmpsadbw ymm2,ymm6,[ecx],7
	vpalignr ymm2,ymm6,ymm4,7
	vpalignr ymm2,ymm6,YMMWORD PTR [ecx],7
	vpalignr ymm2,ymm6,[ecx],7
	vpblendw ymm2,ymm6,ymm4,7
	vpblendw ymm2,ymm6,YMMWORD PTR [ecx],7
	vpblendw ymm2,ymm6,[ecx],7

# Tests for op ymm, ymm/mem256, ymm, ymm
	vpblendvb ymm7,ymm2,ymm6,ymm4
	vpblendvb ymm7,ymm2,YMMWORD PTR [ecx],ymm4
	vpblendvb ymm7,ymm2,[ecx],ymm4

# Tests for op xmm/mem128, ymm, ymm
	vpsllw ymm2,ymm6,xmm4
	vpsllw ymm2,ymm6,XMMWORD PTR [ecx]
	vpsllw ymm2,ymm6,[ecx]
	vpslld ymm2,ymm6,xmm4
	vpslld ymm2,ymm6,XMMWORD PTR [ecx]
	vpslld ymm2,ymm6,[ecx]
	vpsllq ymm2,ymm6,xmm4
	vpsllq ymm2,ymm6,XMMWORD PTR [ecx]
	vpsllq ymm2,ymm6,[ecx]
	vpsraw ymm2,ymm6,xmm4
	vpsraw ymm2,ymm6,XMMWORD PTR [ecx]
	vpsraw ymm2,ymm6,[ecx]
	vpsrad ymm2,ymm6,xmm4
	vpsrad ymm2,ymm6,XMMWORD PTR [ecx]
	vpsrad ymm2,ymm6,[ecx]
	vpsrlw ymm2,ymm6,xmm4
	vpsrlw ymm2,ymm6,XMMWORD PTR [ecx]
	vpsrlw ymm2,ymm6,[ecx]
	vpsrld ymm2,ymm6,xmm4
	vpsrld ymm2,ymm6,XMMWORD PTR [ecx]
	vpsrld ymm2,ymm6,[ecx]
	vpsrlq ymm2,ymm6,xmm4
	vpsrlq ymm2,ymm6,XMMWORD PTR [ecx]
	vpsrlq ymm2,ymm6,[ecx]

# Tests for op xmm/mem128, ymm
	vpmovsxbw ymm4,xmm4
	vpmovsxbw ymm4,XMMWORD PTR [ecx]
	vpmovsxbw ymm4,[ecx]
	vpmovsxwd ymm4,xmm4
	vpmovsxwd ymm4,XMMWORD PTR [ecx]
	vpmovsxwd ymm4,[ecx]
	vpmovsxdq ymm4,xmm4
	vpmovsxdq ymm4,XMMWORD PTR [ecx]
	vpmovsxdq ymm4,[ecx]
	vpmovzxbw ymm4,xmm4
	vpmovzxbw ymm4,XMMWORD PTR [ecx]
	vpmovzxbw ymm4,[ecx]
	vpmovzxwd ymm4,xmm4
	vpmovzxwd ymm4,XMMWORD PTR [ecx]
	vpmovzxwd ymm4,[ecx]
	vpmovzxdq ymm4,xmm4
	vpmovzxdq ymm4,XMMWORD PTR [ecx]
	vpmovzxdq ymm4,[ecx]

# Tests for op xmm/mem64, ymm
	vpmovsxbd ymm6,xmm4
	vpmovsxbd ymm4,QWORD PTR [ecx]
	vpmovsxbd ymm4,[ecx]
	vpmovsxwq ymm6,xmm4
	vpmovsxwq ymm4,QWORD PTR [ecx]
	vpmovsxwq ymm4,[ecx]
	vpmovzxbd ymm6,xmm4
	vpmovzxbd ymm4,QWORD PTR [ecx]
	vpmovzxbd ymm4,[ecx]
	vpmovzxwq ymm6,xmm4
	vpmovzxwq ymm4,QWORD PTR [ecx]
	vpmovzxwq ymm4,[ecx]

# Tests for op xmm/mem32, ymm
	vpmovsxbq ymm4,xmm4
	vpmovsxbq ymm4,DWORD PTR [ecx]
	vpmovsxbq ymm4,[ecx]
	vpmovzxbq ymm4,xmm4
	vpmovzxbq ymm4,DWORD PTR [ecx]
	vpmovzxbq ymm4,[ecx]
