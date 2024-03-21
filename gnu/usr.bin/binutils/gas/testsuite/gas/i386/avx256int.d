#objdump: -dw
#name: i386 256bit integer AVX insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c5 fd d7 cc          	vpmovmskb %ymm4,%ecx
[ 	]*[a-f0-9]+:	c5 ed 72 f6 07       	vpslld \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 73 fe 07       	vpslldq \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 73 f6 07       	vpsllq \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 71 f6 07       	vpsllw \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 72 e6 07       	vpsrad \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 71 e6 07       	vpsraw \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 72 d6 07       	vpsrld \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 73 de 07       	vpsrldq \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 73 d6 07       	vpsrlq \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 71 d6 07       	vpsrlw \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 fd 70 d6 07       	vpshufd \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 fd 70 31 07       	vpshufd \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c5 fe 70 d6 07       	vpshufhw \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 fe 70 31 07       	vpshufhw \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c5 ff 70 d6 07       	vpshuflw \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ff 70 31 07       	vpshuflw \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c5 cd 6b d4          	vpackssdw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6b 11          	vpackssdw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 63 d4          	vpacksswb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 63 11          	vpacksswb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 2b d4       	vpackusdw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 2b 11       	vpackusdw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 67 d4          	vpackuswb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 67 11          	vpackuswb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fc d4          	vpaddb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fc 11          	vpaddb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fd d4          	vpaddw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fd 11          	vpaddw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fe d4          	vpaddd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fe 11          	vpaddd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d4 d4          	vpaddq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d4 11          	vpaddq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ec d4          	vpaddsb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ec 11          	vpaddsb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ed d4          	vpaddsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ed 11          	vpaddsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd dc d4          	vpaddusb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd dc 11          	vpaddusb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd dd d4          	vpaddusw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd dd 11          	vpaddusw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd db d4          	vpand  %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd db 11          	vpand  \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd df d4          	vpandn %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd df 11          	vpandn \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e0 d4          	vpavgb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e0 11          	vpavgb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e3 d4          	vpavgw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e3 11          	vpavgw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 74 d4          	vpcmpeqb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 74 11          	vpcmpeqb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 75 d4          	vpcmpeqw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 75 11          	vpcmpeqw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 76 d4          	vpcmpeqd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 76 11          	vpcmpeqd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 29 d4       	vpcmpeqq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 29 11       	vpcmpeqq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 64 d4          	vpcmpgtb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 64 11          	vpcmpgtb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 65 d4          	vpcmpgtw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 65 11          	vpcmpgtw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 66 d4          	vpcmpgtd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 66 11          	vpcmpgtd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 37 d4       	vpcmpgtq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 37 11       	vpcmpgtq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 01 d4       	vphaddw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 01 11       	vphaddw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 02 d4       	vphaddd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 02 11       	vphaddd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 03 d4       	vphaddsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 03 11       	vphaddsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 05 d4       	vphsubw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 05 11       	vphsubw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 06 d4       	vphsubd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 06 11       	vphsubd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 07 d4       	vphsubsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 07 11       	vphsubsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f5 d4          	vpmaddwd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f5 11          	vpmaddwd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 04 d4       	vpmaddubsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 04 11       	vpmaddubsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3c d4       	vpmaxsb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3c 11       	vpmaxsb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ee d4          	vpmaxsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ee 11          	vpmaxsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3d d4       	vpmaxsd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3d 11       	vpmaxsd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd de d4          	vpmaxub %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd de 11          	vpmaxub \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3e d4       	vpmaxuw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3e 11       	vpmaxuw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3f d4       	vpmaxud %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3f 11       	vpmaxud \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 38 d4       	vpminsb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 38 11       	vpminsb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ea d4          	vpminsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ea 11          	vpminsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 39 d4       	vpminsd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 39 11       	vpminsd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd da d4          	vpminub %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd da 11          	vpminub \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3a d4       	vpminuw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3a 11       	vpminuw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3b d4       	vpminud %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3b 11       	vpminud \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e4 d4          	vpmulhuw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e4 11          	vpmulhuw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0b d4       	vpmulhrsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0b 11       	vpmulhrsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e5 d4          	vpmulhw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e5 11          	vpmulhw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d5 d4          	vpmullw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d5 11          	vpmullw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 40 d4       	vpmulld %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 40 11       	vpmulld \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f4 d4          	vpmuludq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f4 11          	vpmuludq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 28 d4       	vpmuldq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 28 11       	vpmuldq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd eb d4          	vpor   %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd eb 11          	vpor   \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f6 d4          	vpsadbw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f6 11          	vpsadbw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 00 d4       	vpshufb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 00 11       	vpshufb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 08 d4       	vpsignb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 08 11       	vpsignb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 09 d4       	vpsignw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 09 11       	vpsignw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0a d4       	vpsignd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0a 11       	vpsignd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f8 d4          	vpsubb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f8 11          	vpsubb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f9 d4          	vpsubw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f9 11          	vpsubw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fa d4          	vpsubd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fa 11          	vpsubd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fb d4          	vpsubq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fb 11          	vpsubq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e8 d4          	vpsubsb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e8 11          	vpsubsb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e9 d4          	vpsubsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e9 11          	vpsubsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d8 d4          	vpsubusb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d8 11          	vpsubusb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d9 d4          	vpsubusw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d9 11          	vpsubusw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 68 d4          	vpunpckhbw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 68 11          	vpunpckhbw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 69 d4          	vpunpckhwd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 69 11          	vpunpckhwd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6a d4          	vpunpckhdq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6a 11          	vpunpckhdq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6d d4          	vpunpckhqdq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6d 11          	vpunpckhqdq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 60 d4          	vpunpcklbw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 60 11          	vpunpcklbw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 61 d4          	vpunpcklwd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 61 11          	vpunpcklwd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 62 d4          	vpunpckldq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 62 11          	vpunpckldq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6c d4          	vpunpcklqdq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6c 11          	vpunpcklqdq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ef d4          	vpxor  %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ef 11          	vpxor  \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 7d 1c f4       	vpabsb %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 1c 21       	vpabsb \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 1d f4       	vpabsw %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 1d 21       	vpabsw \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 1e f4       	vpabsd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 1e 21       	vpabsd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e3 4d 42 d4 07    	vmpsadbw \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 42 11 07    	vmpsadbw \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0f d4 07    	vpalignr \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0f 11 07    	vpalignr \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0e d4 07    	vpblendw \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0e 11 07    	vpblendw \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 6d 4c fe 40    	vpblendvb %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 6d 4c 39 40    	vpblendvb %ymm4,\(%ecx\),%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c5 cd f1 d4          	vpsllw %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f1 11          	vpsllw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f2 d4          	vpslld %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f2 11          	vpslld \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f3 d4          	vpsllq %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f3 11          	vpsllq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e1 d4          	vpsraw %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e1 11          	vpsraw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e2 d4          	vpsrad %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e2 11          	vpsrad \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d1 d4          	vpsrlw %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d1 11          	vpsrlw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d2 d4          	vpsrld %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d2 11          	vpsrld \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d3 d4          	vpsrlq %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d3 11          	vpsrlq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 7d 20 e4       	vpmovsxbw %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 20 21       	vpmovsxbw \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 23 e4       	vpmovsxwd %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 23 21       	vpmovsxwd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 25 e4       	vpmovsxdq %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 25 21       	vpmovsxdq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 30 e4       	vpmovzxbw %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 30 21       	vpmovzxbw \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 33 e4       	vpmovzxwd %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 33 21       	vpmovzxwd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 35 e4       	vpmovzxdq %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 35 21       	vpmovzxdq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 21 f4       	vpmovsxbd %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 21 21       	vpmovsxbd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 24 f4       	vpmovsxwq %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 24 21       	vpmovsxwq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 31 f4       	vpmovzxbd %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 31 21       	vpmovzxbd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 34 f4       	vpmovzxwq %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 34 21       	vpmovzxwq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 22 e4       	vpmovsxbq %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 22 21       	vpmovsxbq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 32 e4       	vpmovzxbq %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 32 21       	vpmovzxbq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c5 fd d7 cc          	vpmovmskb %ymm4,%ecx
[ 	]*[a-f0-9]+:	c5 ed 72 f6 07       	vpslld \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 73 fe 07       	vpslldq \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 73 f6 07       	vpsllq \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 71 f6 07       	vpsllw \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 72 e6 07       	vpsrad \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 71 e6 07       	vpsraw \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 72 d6 07       	vpsrld \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 73 de 07       	vpsrldq \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 73 d6 07       	vpsrlq \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ed 71 d6 07       	vpsrlw \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 fd 70 d6 07       	vpshufd \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 fd 70 31 07       	vpshufd \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c5 fd 70 31 07       	vpshufd \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c5 fe 70 d6 07       	vpshufhw \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 fe 70 31 07       	vpshufhw \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c5 fe 70 31 07       	vpshufhw \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c5 ff 70 d6 07       	vpshuflw \$0x7,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 ff 70 31 07       	vpshuflw \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c5 ff 70 31 07       	vpshuflw \$0x7,\(%ecx\),%ymm6
[ 	]*[a-f0-9]+:	c5 cd 6b d4          	vpackssdw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6b 11          	vpackssdw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6b 11          	vpackssdw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 63 d4          	vpacksswb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 63 11          	vpacksswb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 63 11          	vpacksswb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 2b d4       	vpackusdw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 2b 11       	vpackusdw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 2b 11       	vpackusdw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 67 d4          	vpackuswb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 67 11          	vpackuswb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 67 11          	vpackuswb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fc d4          	vpaddb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fc 11          	vpaddb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fc 11          	vpaddb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fd d4          	vpaddw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fd 11          	vpaddw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fd 11          	vpaddw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fe d4          	vpaddd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fe 11          	vpaddd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fe 11          	vpaddd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d4 d4          	vpaddq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d4 11          	vpaddq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d4 11          	vpaddq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ec d4          	vpaddsb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ec 11          	vpaddsb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ec 11          	vpaddsb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ed d4          	vpaddsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ed 11          	vpaddsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ed 11          	vpaddsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd dc d4          	vpaddusb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd dc 11          	vpaddusb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd dc 11          	vpaddusb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd dd d4          	vpaddusw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd dd 11          	vpaddusw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd dd 11          	vpaddusw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd db d4          	vpand  %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd db 11          	vpand  \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd db 11          	vpand  \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd df d4          	vpandn %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd df 11          	vpandn \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd df 11          	vpandn \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e0 d4          	vpavgb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e0 11          	vpavgb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e0 11          	vpavgb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e3 d4          	vpavgw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e3 11          	vpavgw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e3 11          	vpavgw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 74 d4          	vpcmpeqb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 74 11          	vpcmpeqb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 74 11          	vpcmpeqb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 75 d4          	vpcmpeqw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 75 11          	vpcmpeqw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 75 11          	vpcmpeqw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 76 d4          	vpcmpeqd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 76 11          	vpcmpeqd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 76 11          	vpcmpeqd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 29 d4       	vpcmpeqq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 29 11       	vpcmpeqq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 29 11       	vpcmpeqq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 64 d4          	vpcmpgtb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 64 11          	vpcmpgtb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 64 11          	vpcmpgtb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 65 d4          	vpcmpgtw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 65 11          	vpcmpgtw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 65 11          	vpcmpgtw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 66 d4          	vpcmpgtd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 66 11          	vpcmpgtd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 66 11          	vpcmpgtd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 37 d4       	vpcmpgtq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 37 11       	vpcmpgtq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 37 11       	vpcmpgtq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 01 d4       	vphaddw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 01 11       	vphaddw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 01 11       	vphaddw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 02 d4       	vphaddd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 02 11       	vphaddd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 02 11       	vphaddd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 03 d4       	vphaddsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 03 11       	vphaddsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 03 11       	vphaddsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 05 d4       	vphsubw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 05 11       	vphsubw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 05 11       	vphsubw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 06 d4       	vphsubd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 06 11       	vphsubd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 06 11       	vphsubd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 07 d4       	vphsubsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 07 11       	vphsubsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 07 11       	vphsubsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f5 d4          	vpmaddwd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f5 11          	vpmaddwd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f5 11          	vpmaddwd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 04 d4       	vpmaddubsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 04 11       	vpmaddubsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 04 11       	vpmaddubsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3c d4       	vpmaxsb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3c 11       	vpmaxsb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3c 11       	vpmaxsb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ee d4          	vpmaxsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ee 11          	vpmaxsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ee 11          	vpmaxsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3d d4       	vpmaxsd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3d 11       	vpmaxsd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3d 11       	vpmaxsd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd de d4          	vpmaxub %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd de 11          	vpmaxub \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd de 11          	vpmaxub \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3e d4       	vpmaxuw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3e 11       	vpmaxuw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3e 11       	vpmaxuw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3f d4       	vpmaxud %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3f 11       	vpmaxud \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3f 11       	vpmaxud \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 38 d4       	vpminsb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 38 11       	vpminsb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 38 11       	vpminsb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ea d4          	vpminsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ea 11          	vpminsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ea 11          	vpminsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 39 d4       	vpminsd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 39 11       	vpminsd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 39 11       	vpminsd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd da d4          	vpminub %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd da 11          	vpminub \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd da 11          	vpminub \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3a d4       	vpminuw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3a 11       	vpminuw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3a 11       	vpminuw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3b d4       	vpminud %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3b 11       	vpminud \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 3b 11       	vpminud \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e4 d4          	vpmulhuw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e4 11          	vpmulhuw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e4 11          	vpmulhuw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0b d4       	vpmulhrsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0b 11       	vpmulhrsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0b 11       	vpmulhrsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e5 d4          	vpmulhw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e5 11          	vpmulhw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e5 11          	vpmulhw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d5 d4          	vpmullw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d5 11          	vpmullw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d5 11          	vpmullw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 40 d4       	vpmulld %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 40 11       	vpmulld \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 40 11       	vpmulld \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f4 d4          	vpmuludq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f4 11          	vpmuludq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f4 11          	vpmuludq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 28 d4       	vpmuldq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 28 11       	vpmuldq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 28 11       	vpmuldq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd eb d4          	vpor   %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd eb 11          	vpor   \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd eb 11          	vpor   \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f6 d4          	vpsadbw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f6 11          	vpsadbw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f6 11          	vpsadbw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 00 d4       	vpshufb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 00 11       	vpshufb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 00 11       	vpshufb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 08 d4       	vpsignb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 08 11       	vpsignb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 08 11       	vpsignb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 09 d4       	vpsignw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 09 11       	vpsignw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 09 11       	vpsignw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0a d4       	vpsignd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0a 11       	vpsignd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d 0a 11       	vpsignd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f8 d4          	vpsubb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f8 11          	vpsubb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f8 11          	vpsubb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f9 d4          	vpsubw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f9 11          	vpsubw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f9 11          	vpsubw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fa d4          	vpsubd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fa 11          	vpsubd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fa 11          	vpsubd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fb d4          	vpsubq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fb 11          	vpsubq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd fb 11          	vpsubq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e8 d4          	vpsubsb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e8 11          	vpsubsb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e8 11          	vpsubsb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e9 d4          	vpsubsw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e9 11          	vpsubsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e9 11          	vpsubsw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d8 d4          	vpsubusb %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d8 11          	vpsubusb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d8 11          	vpsubusb \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d9 d4          	vpsubusw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d9 11          	vpsubusw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d9 11          	vpsubusw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 68 d4          	vpunpckhbw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 68 11          	vpunpckhbw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 68 11          	vpunpckhbw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 69 d4          	vpunpckhwd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 69 11          	vpunpckhwd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 69 11          	vpunpckhwd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6a d4          	vpunpckhdq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6a 11          	vpunpckhdq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6a 11          	vpunpckhdq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6d d4          	vpunpckhqdq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6d 11          	vpunpckhqdq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6d 11          	vpunpckhqdq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 60 d4          	vpunpcklbw %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 60 11          	vpunpcklbw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 60 11          	vpunpcklbw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 61 d4          	vpunpcklwd %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 61 11          	vpunpcklwd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 61 11          	vpunpcklwd \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 62 d4          	vpunpckldq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 62 11          	vpunpckldq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 62 11          	vpunpckldq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6c d4          	vpunpcklqdq %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6c 11          	vpunpcklqdq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd 6c 11          	vpunpcklqdq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ef d4          	vpxor  %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ef 11          	vpxor  \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd ef 11          	vpxor  \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 7d 1c f4       	vpabsb %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 1c 21       	vpabsb \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 1c 21       	vpabsb \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 1d f4       	vpabsw %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 1d 21       	vpabsw \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 1d 21       	vpabsw \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 1e f4       	vpabsd %ymm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 1e 21       	vpabsd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 1e 21       	vpabsd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e3 4d 42 d4 07    	vmpsadbw \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 42 11 07    	vmpsadbw \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 42 11 07    	vmpsadbw \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0f d4 07    	vpalignr \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0f 11 07    	vpalignr \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0f 11 07    	vpalignr \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0e d4 07    	vpblendw \$0x7,%ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0e 11 07    	vpblendw \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 4d 0e 11 07    	vpblendw \$0x7,\(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e3 6d 4c fe 40    	vpblendvb %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 6d 4c 39 40    	vpblendvb %ymm4,\(%ecx\),%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c4 e3 6d 4c 39 40    	vpblendvb %ymm4,\(%ecx\),%ymm2,%ymm7
[ 	]*[a-f0-9]+:	c5 cd f1 d4          	vpsllw %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f1 11          	vpsllw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f1 11          	vpsllw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f2 d4          	vpslld %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f2 11          	vpslld \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f2 11          	vpslld \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f3 d4          	vpsllq %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f3 11          	vpsllq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd f3 11          	vpsllq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e1 d4          	vpsraw %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e1 11          	vpsraw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e1 11          	vpsraw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e2 d4          	vpsrad %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e2 11          	vpsrad \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd e2 11          	vpsrad \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d1 d4          	vpsrlw %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d1 11          	vpsrlw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d1 11          	vpsrlw \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d2 d4          	vpsrld %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d2 11          	vpsrld \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d2 11          	vpsrld \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d3 d4          	vpsrlq %xmm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d3 11          	vpsrlq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c5 cd d3 11          	vpsrlq \(%ecx\),%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 7d 20 e4       	vpmovsxbw %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 20 21       	vpmovsxbw \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 20 21       	vpmovsxbw \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 23 e4       	vpmovsxwd %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 23 21       	vpmovsxwd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 23 21       	vpmovsxwd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 25 e4       	vpmovsxdq %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 25 21       	vpmovsxdq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 25 21       	vpmovsxdq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 30 e4       	vpmovzxbw %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 30 21       	vpmovzxbw \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 30 21       	vpmovzxbw \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 33 e4       	vpmovzxwd %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 33 21       	vpmovzxwd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 33 21       	vpmovzxwd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 35 e4       	vpmovzxdq %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 35 21       	vpmovzxdq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 35 21       	vpmovzxdq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 21 f4       	vpmovsxbd %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 21 21       	vpmovsxbd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 21 21       	vpmovsxbd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 24 f4       	vpmovsxwq %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 24 21       	vpmovsxwq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 24 21       	vpmovsxwq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 31 f4       	vpmovzxbd %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 31 21       	vpmovzxbd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 31 21       	vpmovzxbd \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 34 f4       	vpmovzxwq %xmm4,%ymm6
[ 	]*[a-f0-9]+:	c4 e2 7d 34 21       	vpmovzxwq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 34 21       	vpmovzxwq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 22 e4       	vpmovsxbq %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 22 21       	vpmovsxbq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 22 21       	vpmovsxbq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 32 e4       	vpmovzxbq %xmm4,%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 32 21       	vpmovzxbq \(%ecx\),%ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 32 21       	vpmovzxbq \(%ecx\),%ymm4
#pass
