#as: -mvexwig=1
#objdump: -dw
#name: i386 AVX2 WIG insns with -mvexwig=1

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	c4 e2 fd 2a 21       	vmovntdqa \(%ecx\),%ymm4
 +[a-f0-9]+:	c4 e3 cd 42 d4 07    	vmpsadbw \$0x7,%ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 fd 1c f4       	vpabsb %ymm4,%ymm6
 +[a-f0-9]+:	c4 e2 fd 1e f4       	vpabsd %ymm4,%ymm6
 +[a-f0-9]+:	c4 e2 fd 1d f4       	vpabsw %ymm4,%ymm6
 +[a-f0-9]+:	c4 e1 cd 6b d4       	vpackssdw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 63 d4       	vpacksswb %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 2b d4       	vpackusdw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 67 d4       	vpackuswb %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd fc d4       	vpaddb %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd fe d4       	vpaddd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd d4 d4       	vpaddq %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd ec d4       	vpaddsb %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd ed d4       	vpaddsw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd dc d4       	vpaddusb %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd dd d4       	vpaddusw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd fd d4       	vpaddw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e3 cd 0f d4 07    	vpalignr \$0x7,%ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd db d4       	vpand  %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd df d4       	vpandn %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd e0 d4       	vpavgb %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd e3 d4       	vpavgw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e3 cd 0e d4 07    	vpblendw \$0x7,%ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 74 d4       	vpcmpeqb %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 76 d4       	vpcmpeqd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 29 d4       	vpcmpeqq %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 75 d4       	vpcmpeqw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 64 d4       	vpcmpgtb %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 66 d4       	vpcmpgtd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 37 d4       	vpcmpgtq %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 65 d4       	vpcmpgtw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 02 d4       	vphaddd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 03 d4       	vphaddsw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 01 d4       	vphaddw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 06 d4       	vphsubd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 07 d4       	vphsubsw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 05 d4       	vphsubw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 04 d4       	vpmaddubsw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd f5 d4       	vpmaddwd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 3c d4       	vpmaxsb %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 3d d4       	vpmaxsd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd ee d4       	vpmaxsw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd de d4       	vpmaxub %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 3f d4       	vpmaxud %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 3e d4       	vpmaxuw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 38 d4       	vpminsb %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 39 d4       	vpminsd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd ea d4       	vpminsw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd da d4       	vpminub %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 3b d4       	vpminud %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 3a d4       	vpminuw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 fd d7 cc       	vpmovmskb %ymm4,%ecx
 +[a-f0-9]+:	c4 e2 fd 21 f4       	vpmovsxbd %xmm4,%ymm6
 +[a-f0-9]+:	c4 e2 fd 22 e4       	vpmovsxbq %xmm4,%ymm4
 +[a-f0-9]+:	c4 e2 fd 20 e4       	vpmovsxbw %xmm4,%ymm4
 +[a-f0-9]+:	c4 e2 fd 25 e4       	vpmovsxdq %xmm4,%ymm4
 +[a-f0-9]+:	c4 e2 fd 23 e4       	vpmovsxwd %xmm4,%ymm4
 +[a-f0-9]+:	c4 e2 fd 24 f4       	vpmovsxwq %xmm4,%ymm6
 +[a-f0-9]+:	c4 e2 fd 31 f4       	vpmovzxbd %xmm4,%ymm6
 +[a-f0-9]+:	c4 e2 fd 32 e4       	vpmovzxbq %xmm4,%ymm4
 +[a-f0-9]+:	c4 e2 fd 30 e4       	vpmovzxbw %xmm4,%ymm4
 +[a-f0-9]+:	c4 e2 fd 35 e4       	vpmovzxdq %xmm4,%ymm4
 +[a-f0-9]+:	c4 e2 fd 33 e4       	vpmovzxwd %xmm4,%ymm4
 +[a-f0-9]+:	c4 e2 fd 34 f4       	vpmovzxwq %xmm4,%ymm6
 +[a-f0-9]+:	c4 e2 cd 28 d4       	vpmuldq %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 0b d4       	vpmulhrsw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd e4 d4       	vpmulhuw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd e5 d4       	vpmulhw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 40 d4       	vpmulld %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd d5 d4       	vpmullw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd f4 d4       	vpmuludq %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd eb d4       	vpor   %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd f6 d4       	vpsadbw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 00 d4       	vpshufb %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 fd 70 d6 07    	vpshufd \$0x7,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 fe 70 d6 07    	vpshufhw \$0x7,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 ff 70 d6 07    	vpshuflw \$0x7,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 08 d4       	vpsignb %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 0a d4       	vpsignd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e2 cd 09 d4       	vpsignw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 ed 72 f6 07    	vpslld \$0x7,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 ed 73 fe 07    	vpslldq \$0x7,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 ed 73 f6 07    	vpsllq \$0x7,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 ed 71 f6 07    	vpsllw \$0x7,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 ed 72 e6 07    	vpsrad \$0x7,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 ed 71 e6 07    	vpsraw \$0x7,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 ed 72 d6 07    	vpsrld \$0x7,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 ed 73 de 07    	vpsrldq \$0x7,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 ed 73 d6 07    	vpsrlq \$0x7,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 ed 71 d6 07    	vpsrlw \$0x7,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd f8 d4       	vpsubb %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd fa d4       	vpsubd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd fb d4       	vpsubq %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd e8 d4       	vpsubsb %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd e9 d4       	vpsubsw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd d8 d4       	vpsubusb %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd d9 d4       	vpsubusw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd f9 d4       	vpsubw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 68 d4       	vpunpckhbw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 6a d4       	vpunpckhdq %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 6d d4       	vpunpckhqdq %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 69 d4       	vpunpckhwd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 60 d4       	vpunpcklbw %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 62 d4       	vpunpckldq %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 6c d4       	vpunpcklqdq %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd 61 d4       	vpunpcklwd %ymm4,%ymm6,%ymm2
 +[a-f0-9]+:	c4 e1 cd ef d4       	vpxor  %ymm4,%ymm6,%ymm2
#pass
