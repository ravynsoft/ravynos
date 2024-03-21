# Check x86-64 256it integer AVX instructions

	.allow_index_reg
	.text
_start:

# Tests for op ymm, regl
	vpmovmskb %ymm4,%ecx

# Tests for op ymm, regq
	vpmovmskb %ymm4,%rcx

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
	vpshufd $7,(%rcx),%ymm6
	vpshufhw $7,%ymm6,%ymm2
	vpshufhw $7,(%rcx),%ymm6
	vpshuflw $7,%ymm6,%ymm2
	vpshuflw $7,(%rcx),%ymm6

# Tests for op ymm/mem256, ymm, ymm
	vpackssdw %ymm4,%ymm6,%ymm2
	vpackssdw (%rcx),%ymm6,%ymm2
	vpacksswb %ymm4,%ymm6,%ymm2
	vpacksswb (%rcx),%ymm6,%ymm2
	vpackusdw %ymm4,%ymm6,%ymm2
	vpackusdw (%rcx),%ymm6,%ymm2
	vpackuswb %ymm4,%ymm6,%ymm2
	vpackuswb (%rcx),%ymm6,%ymm2
	vpaddb %ymm4,%ymm6,%ymm2
	vpaddb (%rcx),%ymm6,%ymm2
	vpaddw %ymm4,%ymm6,%ymm2
	vpaddw (%rcx),%ymm6,%ymm2
	vpaddd %ymm4,%ymm6,%ymm2
	vpaddd (%rcx),%ymm6,%ymm2
	vpaddq %ymm4,%ymm6,%ymm2
	vpaddq (%rcx),%ymm6,%ymm2
	vpaddsb %ymm4,%ymm6,%ymm2
	vpaddsb (%rcx),%ymm6,%ymm2
	vpaddsw %ymm4,%ymm6,%ymm2
	vpaddsw (%rcx),%ymm6,%ymm2
	vpaddusb %ymm4,%ymm6,%ymm2
	vpaddusb (%rcx),%ymm6,%ymm2
	vpaddusw %ymm4,%ymm6,%ymm2
	vpaddusw (%rcx),%ymm6,%ymm2
	vpand %ymm4,%ymm6,%ymm2
	vpand (%rcx),%ymm6,%ymm2
	vpandn %ymm4,%ymm6,%ymm2
	vpandn (%rcx),%ymm6,%ymm2
	vpavgb %ymm4,%ymm6,%ymm2
	vpavgb (%rcx),%ymm6,%ymm2
	vpavgw %ymm4,%ymm6,%ymm2
	vpavgw (%rcx),%ymm6,%ymm2
	vpcmpeqb %ymm4,%ymm6,%ymm2
	vpcmpeqb (%rcx),%ymm6,%ymm2
	vpcmpeqw %ymm4,%ymm6,%ymm2
	vpcmpeqw (%rcx),%ymm6,%ymm2
	vpcmpeqd %ymm4,%ymm6,%ymm2
	vpcmpeqd (%rcx),%ymm6,%ymm2
	vpcmpeqq %ymm4,%ymm6,%ymm2
	vpcmpeqq (%rcx),%ymm6,%ymm2
	vpcmpgtb %ymm4,%ymm6,%ymm2
	vpcmpgtb (%rcx),%ymm6,%ymm2
	vpcmpgtw %ymm4,%ymm6,%ymm2
	vpcmpgtw (%rcx),%ymm6,%ymm2
	vpcmpgtd %ymm4,%ymm6,%ymm2
	vpcmpgtd (%rcx),%ymm6,%ymm2
	vpcmpgtq %ymm4,%ymm6,%ymm2
	vpcmpgtq (%rcx),%ymm6,%ymm2
	vphaddw %ymm4,%ymm6,%ymm2
	vphaddw (%rcx),%ymm6,%ymm2
	vphaddd %ymm4,%ymm6,%ymm2
	vphaddd (%rcx),%ymm6,%ymm2
	vphaddsw %ymm4,%ymm6,%ymm2
	vphaddsw (%rcx),%ymm6,%ymm2
	vphsubw %ymm4,%ymm6,%ymm2
	vphsubw (%rcx),%ymm6,%ymm2
	vphsubd %ymm4,%ymm6,%ymm2
	vphsubd (%rcx),%ymm6,%ymm2
	vphsubsw %ymm4,%ymm6,%ymm2
	vphsubsw (%rcx),%ymm6,%ymm2
	vpmaddwd %ymm4,%ymm6,%ymm2
	vpmaddwd (%rcx),%ymm6,%ymm2
	vpmaddubsw %ymm4,%ymm6,%ymm2
	vpmaddubsw (%rcx),%ymm6,%ymm2
	vpmaxsb %ymm4,%ymm6,%ymm2
	vpmaxsb (%rcx),%ymm6,%ymm2
	vpmaxsw %ymm4,%ymm6,%ymm2
	vpmaxsw (%rcx),%ymm6,%ymm2
	vpmaxsd %ymm4,%ymm6,%ymm2
	vpmaxsd (%rcx),%ymm6,%ymm2
	vpmaxub %ymm4,%ymm6,%ymm2
	vpmaxub (%rcx),%ymm6,%ymm2
	vpmaxuw %ymm4,%ymm6,%ymm2
	vpmaxuw (%rcx),%ymm6,%ymm2
	vpmaxud %ymm4,%ymm6,%ymm2
	vpmaxud (%rcx),%ymm6,%ymm2
	vpminsb %ymm4,%ymm6,%ymm2
	vpminsb (%rcx),%ymm6,%ymm2
	vpminsw %ymm4,%ymm6,%ymm2
	vpminsw (%rcx),%ymm6,%ymm2
	vpminsd %ymm4,%ymm6,%ymm2
	vpminsd (%rcx),%ymm6,%ymm2
	vpminub %ymm4,%ymm6,%ymm2
	vpminub (%rcx),%ymm6,%ymm2
	vpminuw %ymm4,%ymm6,%ymm2
	vpminuw (%rcx),%ymm6,%ymm2
	vpminud %ymm4,%ymm6,%ymm2
	vpminud (%rcx),%ymm6,%ymm2
	vpmulhuw %ymm4,%ymm6,%ymm2
	vpmulhuw (%rcx),%ymm6,%ymm2
	vpmulhrsw %ymm4,%ymm6,%ymm2
	vpmulhrsw (%rcx),%ymm6,%ymm2
	vpmulhw %ymm4,%ymm6,%ymm2
	vpmulhw (%rcx),%ymm6,%ymm2
	vpmullw %ymm4,%ymm6,%ymm2
	vpmullw (%rcx),%ymm6,%ymm2
	vpmulld %ymm4,%ymm6,%ymm2
	vpmulld (%rcx),%ymm6,%ymm2
	vpmuludq %ymm4,%ymm6,%ymm2
	vpmuludq (%rcx),%ymm6,%ymm2
	vpmuldq %ymm4,%ymm6,%ymm2
	vpmuldq (%rcx),%ymm6,%ymm2
	vpor %ymm4,%ymm6,%ymm2
	vpor (%rcx),%ymm6,%ymm2
	vpsadbw %ymm4,%ymm6,%ymm2
	vpsadbw (%rcx),%ymm6,%ymm2
	vpshufb %ymm4,%ymm6,%ymm2
	vpshufb (%rcx),%ymm6,%ymm2
	vpsignb %ymm4,%ymm6,%ymm2
	vpsignb (%rcx),%ymm6,%ymm2
	vpsignw %ymm4,%ymm6,%ymm2
	vpsignw (%rcx),%ymm6,%ymm2
	vpsignd %ymm4,%ymm6,%ymm2
	vpsignd (%rcx),%ymm6,%ymm2
	vpsubb %ymm4,%ymm6,%ymm2
	vpsubb (%rcx),%ymm6,%ymm2
	vpsubw %ymm4,%ymm6,%ymm2
	vpsubw (%rcx),%ymm6,%ymm2
	vpsubd %ymm4,%ymm6,%ymm2
	vpsubd (%rcx),%ymm6,%ymm2
	vpsubq %ymm4,%ymm6,%ymm2
	vpsubq (%rcx),%ymm6,%ymm2
	vpsubsb %ymm4,%ymm6,%ymm2
	vpsubsb (%rcx),%ymm6,%ymm2
	vpsubsw %ymm4,%ymm6,%ymm2
	vpsubsw (%rcx),%ymm6,%ymm2
	vpsubusb %ymm4,%ymm6,%ymm2
	vpsubusb (%rcx),%ymm6,%ymm2
	vpsubusw %ymm4,%ymm6,%ymm2
	vpsubusw (%rcx),%ymm6,%ymm2
	vpunpckhbw %ymm4,%ymm6,%ymm2
	vpunpckhbw (%rcx),%ymm6,%ymm2
	vpunpckhwd %ymm4,%ymm6,%ymm2
	vpunpckhwd (%rcx),%ymm6,%ymm2
	vpunpckhdq %ymm4,%ymm6,%ymm2
	vpunpckhdq (%rcx),%ymm6,%ymm2
	vpunpckhqdq %ymm4,%ymm6,%ymm2
	vpunpckhqdq (%rcx),%ymm6,%ymm2
	vpunpcklbw %ymm4,%ymm6,%ymm2
	vpunpcklbw (%rcx),%ymm6,%ymm2
	vpunpcklwd %ymm4,%ymm6,%ymm2
	vpunpcklwd (%rcx),%ymm6,%ymm2
	vpunpckldq %ymm4,%ymm6,%ymm2
	vpunpckldq (%rcx),%ymm6,%ymm2
	vpunpcklqdq %ymm4,%ymm6,%ymm2
	vpunpcklqdq (%rcx),%ymm6,%ymm2
	vpxor %ymm4,%ymm6,%ymm2
	vpxor (%rcx),%ymm6,%ymm2

# Tests for op ymm/mem256, ymm
	vpabsb %ymm4,%ymm6
	vpabsb (%rcx),%ymm4
	vpabsw %ymm4,%ymm6
	vpabsw (%rcx),%ymm4
	vpabsd %ymm4,%ymm6
	vpabsd (%rcx),%ymm4

# Tests for op imm8, ymm/mem256, ymm, ymm
	vmpsadbw $7,%ymm4,%ymm6,%ymm2
	vmpsadbw $7,(%rcx),%ymm6,%ymm2
	vpalignr $7,%ymm4,%ymm6,%ymm2
	vpalignr $7,(%rcx),%ymm6,%ymm2
	vpblendw $7,%ymm4,%ymm6,%ymm2
	vpblendw $7,(%rcx),%ymm6,%ymm2

# Tests for op ymm, ymm/mem256, ymm, ymm
	vpblendvb %ymm4,%ymm6,%ymm2,%ymm7
	vpblendvb %ymm4,(%rcx),%ymm2,%ymm7

# Tests for op xmm/mem128, ymm, ymm
	vpsllw %xmm4,%ymm6,%ymm2
	vpsllw (%rcx),%ymm6,%ymm2
	vpslld %xmm4,%ymm6,%ymm2
	vpslld (%rcx),%ymm6,%ymm2
	vpsllq %xmm4,%ymm6,%ymm2
	vpsllq (%rcx),%ymm6,%ymm2
	vpsraw %xmm4,%ymm6,%ymm2
	vpsraw (%rcx),%ymm6,%ymm2
	vpsrad %xmm4,%ymm6,%ymm2
	vpsrad (%rcx),%ymm6,%ymm2
	vpsrlw %xmm4,%ymm6,%ymm2
	vpsrlw (%rcx),%ymm6,%ymm2
	vpsrld %xmm4,%ymm6,%ymm2
	vpsrld (%rcx),%ymm6,%ymm2
	vpsrlq %xmm4,%ymm6,%ymm2
	vpsrlq (%rcx),%ymm6,%ymm2

# Tests for op xmm/mem128, ymm
	vpmovsxbw %xmm4,%ymm4
	vpmovsxbw (%rcx),%ymm4
	vpmovsxwd %xmm4,%ymm4
	vpmovsxwd (%rcx),%ymm4
	vpmovsxdq %xmm4,%ymm4
	vpmovsxdq (%rcx),%ymm4
	vpmovzxbw %xmm4,%ymm4
	vpmovzxbw (%rcx),%ymm4
	vpmovzxwd %xmm4,%ymm4
	vpmovzxwd (%rcx),%ymm4
	vpmovzxdq %xmm4,%ymm4
	vpmovzxdq (%rcx),%ymm4

# Tests for op xmm/mem64, ymm
	vpmovsxbd %xmm4,%ymm6
	vpmovsxbd (%rcx),%ymm4
	vpmovsxwq %xmm4,%ymm6
	vpmovsxwq (%rcx),%ymm4
	vpmovzxbd %xmm4,%ymm6
	vpmovzxbd (%rcx),%ymm4
	vpmovzxwq %xmm4,%ymm6
	vpmovzxwq (%rcx),%ymm4

# Tests for op xmm/mem32, ymm
	vpmovsxbq %xmm4,%ymm4
	vpmovsxbq (%rcx),%ymm4
	vpmovzxbq %xmm4,%ymm4
	vpmovzxbq (%rcx),%ymm4

	.intel_syntax noprefix

# Tests for op ymm, regl
	vpmovmskb ecx,ymm4

# Tests for op ymm, regq
	vpmovmskb rcx,ymm4

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
	vpshufd ymm6,YMMWORD PTR [rcx],7
	vpshufd ymm6,[rcx],7
	vpshufhw ymm2,ymm6,7
	vpshufhw ymm6,YMMWORD PTR [rcx],7
	vpshufhw ymm6,[rcx],7
	vpshuflw ymm2,ymm6,7
	vpshuflw ymm6,YMMWORD PTR [rcx],7
	vpshuflw ymm6,[rcx],7

# Tests for op ymm/mem256, ymm, ymm
	vpackssdw ymm2,ymm6,ymm4
	vpackssdw ymm2,ymm6,YMMWORD PTR [rcx]
	vpackssdw ymm2,ymm6,[rcx]
	vpacksswb ymm2,ymm6,ymm4
	vpacksswb ymm2,ymm6,YMMWORD PTR [rcx]
	vpacksswb ymm2,ymm6,[rcx]
	vpackusdw ymm2,ymm6,ymm4
	vpackusdw ymm2,ymm6,YMMWORD PTR [rcx]
	vpackusdw ymm2,ymm6,[rcx]
	vpackuswb ymm2,ymm6,ymm4
	vpackuswb ymm2,ymm6,YMMWORD PTR [rcx]
	vpackuswb ymm2,ymm6,[rcx]
	vpaddb ymm2,ymm6,ymm4
	vpaddb ymm2,ymm6,YMMWORD PTR [rcx]
	vpaddb ymm2,ymm6,[rcx]
	vpaddw ymm2,ymm6,ymm4
	vpaddw ymm2,ymm6,YMMWORD PTR [rcx]
	vpaddw ymm2,ymm6,[rcx]
	vpaddd ymm2,ymm6,ymm4
	vpaddd ymm2,ymm6,YMMWORD PTR [rcx]
	vpaddd ymm2,ymm6,[rcx]
	vpaddq ymm2,ymm6,ymm4
	vpaddq ymm2,ymm6,YMMWORD PTR [rcx]
	vpaddq ymm2,ymm6,[rcx]
	vpaddsb ymm2,ymm6,ymm4
	vpaddsb ymm2,ymm6,YMMWORD PTR [rcx]
	vpaddsb ymm2,ymm6,[rcx]
	vpaddsw ymm2,ymm6,ymm4
	vpaddsw ymm2,ymm6,YMMWORD PTR [rcx]
	vpaddsw ymm2,ymm6,[rcx]
	vpaddusb ymm2,ymm6,ymm4
	vpaddusb ymm2,ymm6,YMMWORD PTR [rcx]
	vpaddusb ymm2,ymm6,[rcx]
	vpaddusw ymm2,ymm6,ymm4
	vpaddusw ymm2,ymm6,YMMWORD PTR [rcx]
	vpaddusw ymm2,ymm6,[rcx]
	vpand ymm2,ymm6,ymm4
	vpand ymm2,ymm6,YMMWORD PTR [rcx]
	vpand ymm2,ymm6,[rcx]
	vpandn ymm2,ymm6,ymm4
	vpandn ymm2,ymm6,YMMWORD PTR [rcx]
	vpandn ymm2,ymm6,[rcx]
	vpavgb ymm2,ymm6,ymm4
	vpavgb ymm2,ymm6,YMMWORD PTR [rcx]
	vpavgb ymm2,ymm6,[rcx]
	vpavgw ymm2,ymm6,ymm4
	vpavgw ymm2,ymm6,YMMWORD PTR [rcx]
	vpavgw ymm2,ymm6,[rcx]
	vpcmpeqb ymm2,ymm6,ymm4
	vpcmpeqb ymm2,ymm6,YMMWORD PTR [rcx]
	vpcmpeqb ymm2,ymm6,[rcx]
	vpcmpeqw ymm2,ymm6,ymm4
	vpcmpeqw ymm2,ymm6,YMMWORD PTR [rcx]
	vpcmpeqw ymm2,ymm6,[rcx]
	vpcmpeqd ymm2,ymm6,ymm4
	vpcmpeqd ymm2,ymm6,YMMWORD PTR [rcx]
	vpcmpeqd ymm2,ymm6,[rcx]
	vpcmpeqq ymm2,ymm6,ymm4
	vpcmpeqq ymm2,ymm6,YMMWORD PTR [rcx]
	vpcmpeqq ymm2,ymm6,[rcx]
	vpcmpgtb ymm2,ymm6,ymm4
	vpcmpgtb ymm2,ymm6,YMMWORD PTR [rcx]
	vpcmpgtb ymm2,ymm6,[rcx]
	vpcmpgtw ymm2,ymm6,ymm4
	vpcmpgtw ymm2,ymm6,YMMWORD PTR [rcx]
	vpcmpgtw ymm2,ymm6,[rcx]
	vpcmpgtd ymm2,ymm6,ymm4
	vpcmpgtd ymm2,ymm6,YMMWORD PTR [rcx]
	vpcmpgtd ymm2,ymm6,[rcx]
	vpcmpgtq ymm2,ymm6,ymm4
	vpcmpgtq ymm2,ymm6,YMMWORD PTR [rcx]
	vpcmpgtq ymm2,ymm6,[rcx]
	vphaddw ymm2,ymm6,ymm4
	vphaddw ymm2,ymm6,YMMWORD PTR [rcx]
	vphaddw ymm2,ymm6,[rcx]
	vphaddd ymm2,ymm6,ymm4
	vphaddd ymm2,ymm6,YMMWORD PTR [rcx]
	vphaddd ymm2,ymm6,[rcx]
	vphaddsw ymm2,ymm6,ymm4
	vphaddsw ymm2,ymm6,YMMWORD PTR [rcx]
	vphaddsw ymm2,ymm6,[rcx]
	vphsubw ymm2,ymm6,ymm4
	vphsubw ymm2,ymm6,YMMWORD PTR [rcx]
	vphsubw ymm2,ymm6,[rcx]
	vphsubd ymm2,ymm6,ymm4
	vphsubd ymm2,ymm6,YMMWORD PTR [rcx]
	vphsubd ymm2,ymm6,[rcx]
	vphsubsw ymm2,ymm6,ymm4
	vphsubsw ymm2,ymm6,YMMWORD PTR [rcx]
	vphsubsw ymm2,ymm6,[rcx]
	vpmaddwd ymm2,ymm6,ymm4
	vpmaddwd ymm2,ymm6,YMMWORD PTR [rcx]
	vpmaddwd ymm2,ymm6,[rcx]
	vpmaddubsw ymm2,ymm6,ymm4
	vpmaddubsw ymm2,ymm6,YMMWORD PTR [rcx]
	vpmaddubsw ymm2,ymm6,[rcx]
	vpmaxsb ymm2,ymm6,ymm4
	vpmaxsb ymm2,ymm6,YMMWORD PTR [rcx]
	vpmaxsb ymm2,ymm6,[rcx]
	vpmaxsw ymm2,ymm6,ymm4
	vpmaxsw ymm2,ymm6,YMMWORD PTR [rcx]
	vpmaxsw ymm2,ymm6,[rcx]
	vpmaxsd ymm2,ymm6,ymm4
	vpmaxsd ymm2,ymm6,YMMWORD PTR [rcx]
	vpmaxsd ymm2,ymm6,[rcx]
	vpmaxub ymm2,ymm6,ymm4
	vpmaxub ymm2,ymm6,YMMWORD PTR [rcx]
	vpmaxub ymm2,ymm6,[rcx]
	vpmaxuw ymm2,ymm6,ymm4
	vpmaxuw ymm2,ymm6,YMMWORD PTR [rcx]
	vpmaxuw ymm2,ymm6,[rcx]
	vpmaxud ymm2,ymm6,ymm4
	vpmaxud ymm2,ymm6,YMMWORD PTR [rcx]
	vpmaxud ymm2,ymm6,[rcx]
	vpminsb ymm2,ymm6,ymm4
	vpminsb ymm2,ymm6,YMMWORD PTR [rcx]
	vpminsb ymm2,ymm6,[rcx]
	vpminsw ymm2,ymm6,ymm4
	vpminsw ymm2,ymm6,YMMWORD PTR [rcx]
	vpminsw ymm2,ymm6,[rcx]
	vpminsd ymm2,ymm6,ymm4
	vpminsd ymm2,ymm6,YMMWORD PTR [rcx]
	vpminsd ymm2,ymm6,[rcx]
	vpminub ymm2,ymm6,ymm4
	vpminub ymm2,ymm6,YMMWORD PTR [rcx]
	vpminub ymm2,ymm6,[rcx]
	vpminuw ymm2,ymm6,ymm4
	vpminuw ymm2,ymm6,YMMWORD PTR [rcx]
	vpminuw ymm2,ymm6,[rcx]
	vpminud ymm2,ymm6,ymm4
	vpminud ymm2,ymm6,YMMWORD PTR [rcx]
	vpminud ymm2,ymm6,[rcx]
	vpmulhuw ymm2,ymm6,ymm4
	vpmulhuw ymm2,ymm6,YMMWORD PTR [rcx]
	vpmulhuw ymm2,ymm6,[rcx]
	vpmulhrsw ymm2,ymm6,ymm4
	vpmulhrsw ymm2,ymm6,YMMWORD PTR [rcx]
	vpmulhrsw ymm2,ymm6,[rcx]
	vpmulhw ymm2,ymm6,ymm4
	vpmulhw ymm2,ymm6,YMMWORD PTR [rcx]
	vpmulhw ymm2,ymm6,[rcx]
	vpmullw ymm2,ymm6,ymm4
	vpmullw ymm2,ymm6,YMMWORD PTR [rcx]
	vpmullw ymm2,ymm6,[rcx]
	vpmulld ymm2,ymm6,ymm4
	vpmulld ymm2,ymm6,YMMWORD PTR [rcx]
	vpmulld ymm2,ymm6,[rcx]
	vpmuludq ymm2,ymm6,ymm4
	vpmuludq ymm2,ymm6,YMMWORD PTR [rcx]
	vpmuludq ymm2,ymm6,[rcx]
	vpmuldq ymm2,ymm6,ymm4
	vpmuldq ymm2,ymm6,YMMWORD PTR [rcx]
	vpmuldq ymm2,ymm6,[rcx]
	vpor ymm2,ymm6,ymm4
	vpor ymm2,ymm6,YMMWORD PTR [rcx]
	vpor ymm2,ymm6,[rcx]
	vpsadbw ymm2,ymm6,ymm4
	vpsadbw ymm2,ymm6,YMMWORD PTR [rcx]
	vpsadbw ymm2,ymm6,[rcx]
	vpshufb ymm2,ymm6,ymm4
	vpshufb ymm2,ymm6,YMMWORD PTR [rcx]
	vpshufb ymm2,ymm6,[rcx]
	vpsignb ymm2,ymm6,ymm4
	vpsignb ymm2,ymm6,YMMWORD PTR [rcx]
	vpsignb ymm2,ymm6,[rcx]
	vpsignw ymm2,ymm6,ymm4
	vpsignw ymm2,ymm6,YMMWORD PTR [rcx]
	vpsignw ymm2,ymm6,[rcx]
	vpsignd ymm2,ymm6,ymm4
	vpsignd ymm2,ymm6,YMMWORD PTR [rcx]
	vpsignd ymm2,ymm6,[rcx]
	vpsubb ymm2,ymm6,ymm4
	vpsubb ymm2,ymm6,YMMWORD PTR [rcx]
	vpsubb ymm2,ymm6,[rcx]
	vpsubw ymm2,ymm6,ymm4
	vpsubw ymm2,ymm6,YMMWORD PTR [rcx]
	vpsubw ymm2,ymm6,[rcx]
	vpsubd ymm2,ymm6,ymm4
	vpsubd ymm2,ymm6,YMMWORD PTR [rcx]
	vpsubd ymm2,ymm6,[rcx]
	vpsubq ymm2,ymm6,ymm4
	vpsubq ymm2,ymm6,YMMWORD PTR [rcx]
	vpsubq ymm2,ymm6,[rcx]
	vpsubsb ymm2,ymm6,ymm4
	vpsubsb ymm2,ymm6,YMMWORD PTR [rcx]
	vpsubsb ymm2,ymm6,[rcx]
	vpsubsw ymm2,ymm6,ymm4
	vpsubsw ymm2,ymm6,YMMWORD PTR [rcx]
	vpsubsw ymm2,ymm6,[rcx]
	vpsubusb ymm2,ymm6,ymm4
	vpsubusb ymm2,ymm6,YMMWORD PTR [rcx]
	vpsubusb ymm2,ymm6,[rcx]
	vpsubusw ymm2,ymm6,ymm4
	vpsubusw ymm2,ymm6,YMMWORD PTR [rcx]
	vpsubusw ymm2,ymm6,[rcx]
	vpunpckhbw ymm2,ymm6,ymm4
	vpunpckhbw ymm2,ymm6,YMMWORD PTR [rcx]
	vpunpckhbw ymm2,ymm6,[rcx]
	vpunpckhwd ymm2,ymm6,ymm4
	vpunpckhwd ymm2,ymm6,YMMWORD PTR [rcx]
	vpunpckhwd ymm2,ymm6,[rcx]
	vpunpckhdq ymm2,ymm6,ymm4
	vpunpckhdq ymm2,ymm6,YMMWORD PTR [rcx]
	vpunpckhdq ymm2,ymm6,[rcx]
	vpunpckhqdq ymm2,ymm6,ymm4
	vpunpckhqdq ymm2,ymm6,YMMWORD PTR [rcx]
	vpunpckhqdq ymm2,ymm6,[rcx]
	vpunpcklbw ymm2,ymm6,ymm4
	vpunpcklbw ymm2,ymm6,YMMWORD PTR [rcx]
	vpunpcklbw ymm2,ymm6,[rcx]
	vpunpcklwd ymm2,ymm6,ymm4
	vpunpcklwd ymm2,ymm6,YMMWORD PTR [rcx]
	vpunpcklwd ymm2,ymm6,[rcx]
	vpunpckldq ymm2,ymm6,ymm4
	vpunpckldq ymm2,ymm6,YMMWORD PTR [rcx]
	vpunpckldq ymm2,ymm6,[rcx]
	vpunpcklqdq ymm2,ymm6,ymm4
	vpunpcklqdq ymm2,ymm6,YMMWORD PTR [rcx]
	vpunpcklqdq ymm2,ymm6,[rcx]
	vpxor ymm2,ymm6,ymm4
	vpxor ymm2,ymm6,YMMWORD PTR [rcx]
	vpxor ymm2,ymm6,[rcx]

# Tests for op ymm/mem256, ymm
	vpabsb ymm6,ymm4
	vpabsb ymm4,YMMWORD PTR [rcx]
	vpabsb ymm4,[rcx]
	vpabsw ymm6,ymm4
	vpabsw ymm4,YMMWORD PTR [rcx]
	vpabsw ymm4,[rcx]
	vpabsd ymm6,ymm4
	vpabsd ymm4,YMMWORD PTR [rcx]
	vpabsd ymm4,[rcx]

# Tests for op imm8, ymm/mem256, ymm, ymm
	vmpsadbw ymm2,ymm6,ymm4,7
	vmpsadbw ymm2,ymm6,YMMWORD PTR [rcx],7
	vmpsadbw ymm2,ymm6,[rcx],7
	vpalignr ymm2,ymm6,ymm4,7
	vpalignr ymm2,ymm6,YMMWORD PTR [rcx],7
	vpalignr ymm2,ymm6,[rcx],7
	vpblendw ymm2,ymm6,ymm4,7
	vpblendw ymm2,ymm6,YMMWORD PTR [rcx],7
	vpblendw ymm2,ymm6,[rcx],7

# Tests for op ymm, ymm/mem256, ymm, ymm
	vpblendvb ymm7,ymm2,ymm6,ymm4
	vpblendvb ymm7,ymm2,YMMWORD PTR [rcx],ymm4
	vpblendvb ymm7,ymm2,[rcx],ymm4

# Tests for op xmm/mem128, ymm, ymm
	vpsllw ymm2,ymm6,xmm4
	vpsllw ymm2,ymm6,XMMWORD PTR [rcx]
	vpsllw ymm2,ymm6,[rcx]
	vpslld ymm2,ymm6,xmm4
	vpslld ymm2,ymm6,XMMWORD PTR [rcx]
	vpslld ymm2,ymm6,[rcx]
	vpsllq ymm2,ymm6,xmm4
	vpsllq ymm2,ymm6,XMMWORD PTR [rcx]
	vpsllq ymm2,ymm6,[rcx]
	vpsraw ymm2,ymm6,xmm4
	vpsraw ymm2,ymm6,XMMWORD PTR [rcx]
	vpsraw ymm2,ymm6,[rcx]
	vpsrad ymm2,ymm6,xmm4
	vpsrad ymm2,ymm6,XMMWORD PTR [rcx]
	vpsrad ymm2,ymm6,[rcx]
	vpsrlw ymm2,ymm6,xmm4
	vpsrlw ymm2,ymm6,XMMWORD PTR [rcx]
	vpsrlw ymm2,ymm6,[rcx]
	vpsrld ymm2,ymm6,xmm4
	vpsrld ymm2,ymm6,XMMWORD PTR [rcx]
	vpsrld ymm2,ymm6,[rcx]
	vpsrlq ymm2,ymm6,xmm4
	vpsrlq ymm2,ymm6,XMMWORD PTR [rcx]
	vpsrlq ymm2,ymm6,[rcx]

# Tests for op xmm/mem128, ymm
	vpmovsxbw ymm4,xmm4
	vpmovsxbw ymm4,XMMWORD PTR [rcx]
	vpmovsxbw ymm4,[rcx]
	vpmovsxwd ymm4,xmm4
	vpmovsxwd ymm4,XMMWORD PTR [rcx]
	vpmovsxwd ymm4,[rcx]
	vpmovsxdq ymm4,xmm4
	vpmovsxdq ymm4,XMMWORD PTR [rcx]
	vpmovsxdq ymm4,[rcx]
	vpmovzxbw ymm4,xmm4
	vpmovzxbw ymm4,XMMWORD PTR [rcx]
	vpmovzxbw ymm4,[rcx]
	vpmovzxwd ymm4,xmm4
	vpmovzxwd ymm4,XMMWORD PTR [rcx]
	vpmovzxwd ymm4,[rcx]
	vpmovzxdq ymm4,xmm4
	vpmovzxdq ymm4,XMMWORD PTR [rcx]
	vpmovzxdq ymm4,[rcx]

# Tests for op xmm/mem64, ymm
	vpmovsxbd ymm6,xmm4
	vpmovsxbd ymm4,QWORD PTR [rcx]
	vpmovsxbd ymm4,[rcx]
	vpmovsxwq ymm6,xmm4
	vpmovsxwq ymm4,QWORD PTR [rcx]
	vpmovsxwq ymm4,[rcx]
	vpmovzxbd ymm6,xmm4
	vpmovzxbd ymm4,QWORD PTR [rcx]
	vpmovzxbd ymm4,[rcx]
	vpmovzxwq ymm6,xmm4
	vpmovzxwq ymm4,QWORD PTR [rcx]
	vpmovzxwq ymm4,[rcx]

# Tests for op xmm/mem32, ymm
	vpmovsxbq ymm4,xmm4
	vpmovsxbq ymm4,DWORD PTR [rcx]
	vpmovsxbq ymm4,[rcx]
	vpmovzxbq ymm4,xmm4
	vpmovzxbq ymm4,DWORD PTR [rcx]
	vpmovzxbq ymm4,[rcx]
