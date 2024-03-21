#objdump: -dwMintel
#name: i386 256bit integer AVX insns (Intel disassembly)
#source: avx256int.s

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c5 fd d7 cc          	vpmovmskb ecx,ymm4
[ 	]*[a-f0-9]+:	c5 ed 72 f6 07       	vpslld ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 73 fe 07       	vpslldq ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 73 f6 07       	vpsllq ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 71 f6 07       	vpsllw ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 72 e6 07       	vpsrad ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 71 e6 07       	vpsraw ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 72 d6 07       	vpsrld ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 73 de 07       	vpsrldq ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 73 d6 07       	vpsrlq ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 71 d6 07       	vpsrlw ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 fd 70 d6 07       	vpshufd ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 fd 70 31 07       	vpshufd ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 fe 70 d6 07       	vpshufhw ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 fe 70 31 07       	vpshufhw ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 ff 70 d6 07       	vpshuflw ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ff 70 31 07       	vpshuflw ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cd 6b d4          	vpackssdw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 6b 11          	vpackssdw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 63 d4          	vpacksswb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 63 11          	vpacksswb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 2b d4       	vpackusdw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 2b 11       	vpackusdw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 67 d4          	vpackuswb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 67 11          	vpackuswb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd fc d4          	vpaddb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd fc 11          	vpaddb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd fd d4          	vpaddw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd fd 11          	vpaddw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd fe d4          	vpaddd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd fe 11          	vpaddd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d4 d4          	vpaddq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd d4 11          	vpaddq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd ec d4          	vpaddsb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd ec 11          	vpaddsb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd ed d4          	vpaddsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd ed 11          	vpaddsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd dc d4          	vpaddusb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd dc 11          	vpaddusb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd dd d4          	vpaddusw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd dd 11          	vpaddusw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd db d4          	vpand  ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd db 11          	vpand  ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd df d4          	vpandn ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd df 11          	vpandn ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e0 d4          	vpavgb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd e0 11          	vpavgb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e3 d4          	vpavgw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd e3 11          	vpavgw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 74 d4          	vpcmpeqb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 74 11          	vpcmpeqb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 75 d4          	vpcmpeqw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 75 11          	vpcmpeqw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 76 d4          	vpcmpeqd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 76 11          	vpcmpeqd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 29 d4       	vpcmpeqq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 29 11       	vpcmpeqq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 64 d4          	vpcmpgtb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 64 11          	vpcmpgtb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 65 d4          	vpcmpgtw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 65 11          	vpcmpgtw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 66 d4          	vpcmpgtd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 66 11          	vpcmpgtd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 37 d4       	vpcmpgtq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 37 11       	vpcmpgtq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 01 d4       	vphaddw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 01 11       	vphaddw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 02 d4       	vphaddd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 02 11       	vphaddd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 03 d4       	vphaddsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 03 11       	vphaddsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 05 d4       	vphsubw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 05 11       	vphsubw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 06 d4       	vphsubd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 06 11       	vphsubd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 07 d4       	vphsubsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 07 11       	vphsubsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f5 d4          	vpmaddwd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd f5 11          	vpmaddwd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 04 d4       	vpmaddubsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 04 11       	vpmaddubsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3c d4       	vpmaxsb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 3c 11       	vpmaxsb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd ee d4          	vpmaxsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd ee 11          	vpmaxsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3d d4       	vpmaxsd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 3d 11       	vpmaxsd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd de d4          	vpmaxub ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd de 11          	vpmaxub ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3e d4       	vpmaxuw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 3e 11       	vpmaxuw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3f d4       	vpmaxud ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 3f 11       	vpmaxud ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 38 d4       	vpminsb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 38 11       	vpminsb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd ea d4          	vpminsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd ea 11          	vpminsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 39 d4       	vpminsd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 39 11       	vpminsd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd da d4          	vpminub ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd da 11          	vpminub ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3a d4       	vpminuw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 3a 11       	vpminuw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3b d4       	vpminud ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 3b 11       	vpminud ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e4 d4          	vpmulhuw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd e4 11          	vpmulhuw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0b d4       	vpmulhrsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 0b 11       	vpmulhrsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e5 d4          	vpmulhw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd e5 11          	vpmulhw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d5 d4          	vpmullw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd d5 11          	vpmullw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 40 d4       	vpmulld ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 40 11       	vpmulld ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f4 d4          	vpmuludq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd f4 11          	vpmuludq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 28 d4       	vpmuldq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 28 11       	vpmuldq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd eb d4          	vpor   ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd eb 11          	vpor   ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f6 d4          	vpsadbw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd f6 11          	vpsadbw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 00 d4       	vpshufb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 00 11       	vpshufb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 08 d4       	vpsignb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 08 11       	vpsignb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 09 d4       	vpsignw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 09 11       	vpsignw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0a d4       	vpsignd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 0a 11       	vpsignd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f8 d4          	vpsubb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd f8 11          	vpsubb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f9 d4          	vpsubw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd f9 11          	vpsubw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd fa d4          	vpsubd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd fa 11          	vpsubd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd fb d4          	vpsubq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd fb 11          	vpsubq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e8 d4          	vpsubsb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd e8 11          	vpsubsb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e9 d4          	vpsubsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd e9 11          	vpsubsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d8 d4          	vpsubusb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd d8 11          	vpsubusb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d9 d4          	vpsubusw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd d9 11          	vpsubusw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 68 d4          	vpunpckhbw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 68 11          	vpunpckhbw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 69 d4          	vpunpckhwd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 69 11          	vpunpckhwd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 6a d4          	vpunpckhdq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 6a 11          	vpunpckhdq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 6d d4          	vpunpckhqdq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 6d 11          	vpunpckhqdq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 60 d4          	vpunpcklbw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 60 11          	vpunpcklbw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 61 d4          	vpunpcklwd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 61 11          	vpunpcklwd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 62 d4          	vpunpckldq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 62 11          	vpunpckldq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 6c d4          	vpunpcklqdq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 6c 11          	vpunpcklqdq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd ef d4          	vpxor  ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd ef 11          	vpxor  ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 1c f4       	vpabsb ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 1c 21       	vpabsb ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 1d f4       	vpabsw ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 1d 21       	vpabsw ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 1e f4       	vpabsd ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 1e 21       	vpabsd ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 4d 42 d4 07    	vmpsadbw ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 42 11 07    	vmpsadbw ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0f d4 07    	vpalignr ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0f 11 07    	vpalignr ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0e d4 07    	vpblendw ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0e 11 07    	vpblendw ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 6d 4c fe 40    	vpblendvb ymm7,ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4c 39 40    	vpblendvb ymm7,ymm2,YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 cd f1 d4          	vpsllw ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd f1 11          	vpsllw ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f2 d4          	vpslld ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd f2 11          	vpslld ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f3 d4          	vpsllq ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd f3 11          	vpsllq ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e1 d4          	vpsraw ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd e1 11          	vpsraw ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e2 d4          	vpsrad ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd e2 11          	vpsrad ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d1 d4          	vpsrlw ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd d1 11          	vpsrlw ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d2 d4          	vpsrld ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd d2 11          	vpsrld ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d3 d4          	vpsrlq ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd d3 11          	vpsrlq ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 20 e4       	vpmovsxbw ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 20 21       	vpmovsxbw ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 23 e4       	vpmovsxwd ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 23 21       	vpmovsxwd ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 25 e4       	vpmovsxdq ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 25 21       	vpmovsxdq ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 30 e4       	vpmovzxbw ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 30 21       	vpmovzxbw ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 33 e4       	vpmovzxwd ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 33 21       	vpmovzxwd ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 35 e4       	vpmovzxdq ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 35 21       	vpmovzxdq ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 21 f4       	vpmovsxbd ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 21 21       	vpmovsxbd ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 24 f4       	vpmovsxwq ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 24 21       	vpmovsxwq ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 31 f4       	vpmovzxbd ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 31 21       	vpmovzxbd ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 34 f4       	vpmovzxwq ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 34 21       	vpmovzxwq ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 22 e4       	vpmovsxbq ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 22 21       	vpmovsxbq ymm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 32 e4       	vpmovzxbq ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 32 21       	vpmovzxbq ymm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 fd d7 cc          	vpmovmskb ecx,ymm4
[ 	]*[a-f0-9]+:	c5 ed 72 f6 07       	vpslld ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 73 fe 07       	vpslldq ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 73 f6 07       	vpsllq ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 71 f6 07       	vpsllw ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 72 e6 07       	vpsrad ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 71 e6 07       	vpsraw ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 72 d6 07       	vpsrld ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 73 de 07       	vpsrldq ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 73 d6 07       	vpsrlq ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ed 71 d6 07       	vpsrlw ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 fd 70 d6 07       	vpshufd ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 fd 70 31 07       	vpshufd ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 fd 70 31 07       	vpshufd ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 fe 70 d6 07       	vpshufhw ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 fe 70 31 07       	vpshufhw ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 fe 70 31 07       	vpshufhw ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 ff 70 d6 07       	vpshuflw ymm2,ymm6,0x7
[ 	]*[a-f0-9]+:	c5 ff 70 31 07       	vpshuflw ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 ff 70 31 07       	vpshuflw ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c5 cd 6b d4          	vpackssdw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 6b 11          	vpackssdw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 6b 11          	vpackssdw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 63 d4          	vpacksswb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 63 11          	vpacksswb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 63 11          	vpacksswb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 2b d4       	vpackusdw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 2b 11       	vpackusdw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 2b 11       	vpackusdw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 67 d4          	vpackuswb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 67 11          	vpackuswb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 67 11          	vpackuswb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd fc d4          	vpaddb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd fc 11          	vpaddb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd fc 11          	vpaddb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd fd d4          	vpaddw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd fd 11          	vpaddw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd fd 11          	vpaddw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd fe d4          	vpaddd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd fe 11          	vpaddd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd fe 11          	vpaddd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d4 d4          	vpaddq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd d4 11          	vpaddq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d4 11          	vpaddq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd ec d4          	vpaddsb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd ec 11          	vpaddsb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd ec 11          	vpaddsb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd ed d4          	vpaddsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd ed 11          	vpaddsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd ed 11          	vpaddsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd dc d4          	vpaddusb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd dc 11          	vpaddusb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd dc 11          	vpaddusb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd dd d4          	vpaddusw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd dd 11          	vpaddusw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd dd 11          	vpaddusw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd db d4          	vpand  ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd db 11          	vpand  ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd db 11          	vpand  ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd df d4          	vpandn ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd df 11          	vpandn ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd df 11          	vpandn ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e0 d4          	vpavgb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd e0 11          	vpavgb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e0 11          	vpavgb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e3 d4          	vpavgw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd e3 11          	vpavgw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e3 11          	vpavgw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 74 d4          	vpcmpeqb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 74 11          	vpcmpeqb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 74 11          	vpcmpeqb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 75 d4          	vpcmpeqw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 75 11          	vpcmpeqw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 75 11          	vpcmpeqw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 76 d4          	vpcmpeqd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 76 11          	vpcmpeqd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 76 11          	vpcmpeqd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 29 d4       	vpcmpeqq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 29 11       	vpcmpeqq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 29 11       	vpcmpeqq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 64 d4          	vpcmpgtb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 64 11          	vpcmpgtb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 64 11          	vpcmpgtb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 65 d4          	vpcmpgtw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 65 11          	vpcmpgtw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 65 11          	vpcmpgtw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 66 d4          	vpcmpgtd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 66 11          	vpcmpgtd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 66 11          	vpcmpgtd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 37 d4       	vpcmpgtq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 37 11       	vpcmpgtq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 37 11       	vpcmpgtq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 01 d4       	vphaddw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 01 11       	vphaddw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 01 11       	vphaddw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 02 d4       	vphaddd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 02 11       	vphaddd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 02 11       	vphaddd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 03 d4       	vphaddsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 03 11       	vphaddsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 03 11       	vphaddsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 05 d4       	vphsubw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 05 11       	vphsubw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 05 11       	vphsubw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 06 d4       	vphsubd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 06 11       	vphsubd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 06 11       	vphsubd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 07 d4       	vphsubsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 07 11       	vphsubsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 07 11       	vphsubsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f5 d4          	vpmaddwd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd f5 11          	vpmaddwd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f5 11          	vpmaddwd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 04 d4       	vpmaddubsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 04 11       	vpmaddubsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 04 11       	vpmaddubsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3c d4       	vpmaxsb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 3c 11       	vpmaxsb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3c 11       	vpmaxsb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd ee d4          	vpmaxsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd ee 11          	vpmaxsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd ee 11          	vpmaxsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3d d4       	vpmaxsd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 3d 11       	vpmaxsd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3d 11       	vpmaxsd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd de d4          	vpmaxub ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd de 11          	vpmaxub ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd de 11          	vpmaxub ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3e d4       	vpmaxuw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 3e 11       	vpmaxuw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3e 11       	vpmaxuw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3f d4       	vpmaxud ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 3f 11       	vpmaxud ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3f 11       	vpmaxud ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 38 d4       	vpminsb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 38 11       	vpminsb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 38 11       	vpminsb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd ea d4          	vpminsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd ea 11          	vpminsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd ea 11          	vpminsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 39 d4       	vpminsd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 39 11       	vpminsd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 39 11       	vpminsd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd da d4          	vpminub ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd da 11          	vpminub ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd da 11          	vpminub ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3a d4       	vpminuw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 3a 11       	vpminuw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3a 11       	vpminuw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3b d4       	vpminud ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 3b 11       	vpminud ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 3b 11       	vpminud ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e4 d4          	vpmulhuw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd e4 11          	vpmulhuw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e4 11          	vpmulhuw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0b d4       	vpmulhrsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 0b 11       	vpmulhrsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0b 11       	vpmulhrsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e5 d4          	vpmulhw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd e5 11          	vpmulhw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e5 11          	vpmulhw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d5 d4          	vpmullw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd d5 11          	vpmullw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d5 11          	vpmullw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 40 d4       	vpmulld ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 40 11       	vpmulld ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 40 11       	vpmulld ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f4 d4          	vpmuludq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd f4 11          	vpmuludq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f4 11          	vpmuludq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 28 d4       	vpmuldq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 28 11       	vpmuldq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 28 11       	vpmuldq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd eb d4          	vpor   ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd eb 11          	vpor   ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd eb 11          	vpor   ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f6 d4          	vpsadbw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd f6 11          	vpsadbw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f6 11          	vpsadbw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 00 d4       	vpshufb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 00 11       	vpshufb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 00 11       	vpshufb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 08 d4       	vpsignb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 08 11       	vpsignb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 08 11       	vpsignb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 09 d4       	vpsignw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 09 11       	vpsignw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 09 11       	vpsignw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0a d4       	vpsignd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 4d 0a 11       	vpsignd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 4d 0a 11       	vpsignd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f8 d4          	vpsubb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd f8 11          	vpsubb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f8 11          	vpsubb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f9 d4          	vpsubw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd f9 11          	vpsubw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f9 11          	vpsubw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd fa d4          	vpsubd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd fa 11          	vpsubd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd fa 11          	vpsubd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd fb d4          	vpsubq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd fb 11          	vpsubq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd fb 11          	vpsubq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e8 d4          	vpsubsb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd e8 11          	vpsubsb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e8 11          	vpsubsb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e9 d4          	vpsubsw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd e9 11          	vpsubsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e9 11          	vpsubsw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d8 d4          	vpsubusb ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd d8 11          	vpsubusb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d8 11          	vpsubusb ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d9 d4          	vpsubusw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd d9 11          	vpsubusw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d9 11          	vpsubusw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 68 d4          	vpunpckhbw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 68 11          	vpunpckhbw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 68 11          	vpunpckhbw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 69 d4          	vpunpckhwd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 69 11          	vpunpckhwd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 69 11          	vpunpckhwd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 6a d4          	vpunpckhdq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 6a 11          	vpunpckhdq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 6a 11          	vpunpckhdq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 6d d4          	vpunpckhqdq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 6d 11          	vpunpckhqdq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 6d 11          	vpunpckhqdq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 60 d4          	vpunpcklbw ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 60 11          	vpunpcklbw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 60 11          	vpunpcklbw ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 61 d4          	vpunpcklwd ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 61 11          	vpunpcklwd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 61 11          	vpunpcklwd ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 62 d4          	vpunpckldq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 62 11          	vpunpckldq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 62 11          	vpunpckldq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 6c d4          	vpunpcklqdq ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd 6c 11          	vpunpcklqdq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd 6c 11          	vpunpcklqdq ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd ef d4          	vpxor  ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c5 cd ef 11          	vpxor  ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd ef 11          	vpxor  ymm2,ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 1c f4       	vpabsb ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 1c 21       	vpabsb ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 1c 21       	vpabsb ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 1d f4       	vpabsw ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 1d 21       	vpabsw ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 1d 21       	vpabsw ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 1e f4       	vpabsd ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e2 7d 1e 21       	vpabsd ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 1e 21       	vpabsd ymm4,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 4d 42 d4 07    	vmpsadbw ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 42 11 07    	vmpsadbw ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 42 11 07    	vmpsadbw ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0f d4 07    	vpalignr ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0f 11 07    	vpalignr ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0f 11 07    	vpalignr ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0e d4 07    	vpblendw ymm2,ymm6,ymm4,0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0e 11 07    	vpblendw ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 4d 0e 11 07    	vpblendw ymm2,ymm6,YMMWORD PTR \[ecx\],0x7
[ 	]*[a-f0-9]+:	c4 e3 6d 4c fe 40    	vpblendvb ymm7,ymm2,ymm6,ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4c 39 40    	vpblendvb ymm7,ymm2,YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c4 e3 6d 4c 39 40    	vpblendvb ymm7,ymm2,YMMWORD PTR \[ecx\],ymm4
[ 	]*[a-f0-9]+:	c5 cd f1 d4          	vpsllw ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd f1 11          	vpsllw ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f1 11          	vpsllw ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f2 d4          	vpslld ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd f2 11          	vpslld ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f2 11          	vpslld ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f3 d4          	vpsllq ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd f3 11          	vpsllq ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd f3 11          	vpsllq ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e1 d4          	vpsraw ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd e1 11          	vpsraw ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e1 11          	vpsraw ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e2 d4          	vpsrad ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd e2 11          	vpsrad ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd e2 11          	vpsrad ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d1 d4          	vpsrlw ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd d1 11          	vpsrlw ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d1 11          	vpsrlw ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d2 d4          	vpsrld ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd d2 11          	vpsrld ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d2 11          	vpsrld ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d3 d4          	vpsrlq ymm2,ymm6,xmm4
[ 	]*[a-f0-9]+:	c5 cd d3 11          	vpsrlq ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c5 cd d3 11          	vpsrlq ymm2,ymm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 20 e4       	vpmovsxbw ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 20 21       	vpmovsxbw ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 20 21       	vpmovsxbw ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 23 e4       	vpmovsxwd ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 23 21       	vpmovsxwd ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 23 21       	vpmovsxwd ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 25 e4       	vpmovsxdq ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 25 21       	vpmovsxdq ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 25 21       	vpmovsxdq ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 30 e4       	vpmovzxbw ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 30 21       	vpmovzxbw ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 30 21       	vpmovzxbw ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 33 e4       	vpmovzxwd ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 33 21       	vpmovzxwd ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 33 21       	vpmovzxwd ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 35 e4       	vpmovzxdq ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 35 21       	vpmovzxdq ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 35 21       	vpmovzxdq ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 21 f4       	vpmovsxbd ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 21 21       	vpmovsxbd ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 21 21       	vpmovsxbd ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 24 f4       	vpmovsxwq ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 24 21       	vpmovsxwq ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 24 21       	vpmovsxwq ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 31 f4       	vpmovzxbd ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 31 21       	vpmovzxbd ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 31 21       	vpmovzxbd ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 34 f4       	vpmovzxwq ymm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 34 21       	vpmovzxwq ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 34 21       	vpmovzxwq ymm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 22 e4       	vpmovsxbq ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 22 21       	vpmovsxbq ymm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 22 21       	vpmovsxbq ymm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 32 e4       	vpmovzxbq ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 32 21       	vpmovzxbq ymm4,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 32 21       	vpmovzxbq ymm4,DWORD PTR \[ecx\]
#pass
