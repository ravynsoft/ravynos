#as:
#objdump: -dw -Mintel
#name: i386 AVX512-FP16 insns (Intel disassembly)
#source: avx512_fp16.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 58 f4[ 	 ]*vaddph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 58 f4[ 	 ]*vaddph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 58 f4[ 	 ]*vaddph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 58 b4 f4 00 00 00 10[ 	 ]*vaddph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 58 31[ 	 ]*vaddph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 58 71 7f[ 	 ]*vaddph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 58 72 80[ 	 ]*vaddph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 58 f4[ 	 ]*vaddsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 58 f4[ 	 ]*vaddsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 58 f4[ 	 ]*vaddsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 58 b4 f4 00 00 00 10[ 	 ]*vaddsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 58 31[ 	 ]*vaddsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 58 71 7f[ 	 ]*vaddsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 58 72 80[ 	 ]*vaddsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 48 c2 ec 7b[ 	 ]*vcmpph k5,zmm5,zmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 c2 ec 7b[ 	 ]*vcmpph k5,zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 1f c2 ec 7b[ 	 ]*vcmpph k5\{k7\},zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 4f c2 ac f4 00 00 00 10 7b[ 	 ]*vcmpph k5\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 58 c2 29 7b[ 	 ]*vcmpph k5,zmm5,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 48 c2 69 7f 7b[ 	 ]*vcmpph k5,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 5f c2 6a 80 7b[ 	 ]*vcmpph k5\{k7\},zmm5,WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 08 c2 ec 7b[ 	 ]*vcmpsh k5,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 18 c2 ec 7b[ 	 ]*vcmpsh k5,xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 7b[ 	 ]*vcmpsh k5\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 00 00 00 10 7b[ 	 ]*vcmpsh k5\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 08 c2 29 7b[ 	 ]*vcmpsh k5,xmm5,WORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 08 c2 69 7f 7b[ 	 ]*vcmpsh k5,xmm5,WORD PTR \[ecx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 6a 80 7b[ 	 ]*vcmpsh k5\{k7\},xmm5,WORD PTR \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f f5[ 	 ]*vcomish xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 2f f5[ 	 ]*vcomish xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f b4 f4 00 00 00 10[ 	 ]*vcomish xmm6,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f 31[ 	 ]*vcomish xmm6,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f 71 7f[ 	 ]*vcomish xmm6,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f 72 80[ 	 ]*vcomish xmm6,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5b f5[ 	 ]*vcvtdq2ph ymm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5b f5[ 	 ]*vcvtdq2ph ymm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5b f5[ 	 ]*vcvtdq2ph ymm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 5b b4 f4 00 00 00 10[ 	 ]*vcvtdq2ph ymm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 5b 31[ 	 ]*vcvtdq2ph ymm6,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5b 71 7f[ 	 ]*vcvtdq2ph ymm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 5b 72 80[ 	 ]*vcvtdq2ph ymm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 48 5a f5[ 	 ]*vcvtpd2ph xmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 18 5a f5[ 	 ]*vcvtpd2ph xmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 9f 5a f5[ 	 ]*vcvtpd2ph xmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 4f 5a b4 f4 00 00 00 10[ 	 ]*vcvtpd2ph xmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 58 5a 31[ 	 ]*vcvtpd2ph xmm6,QWORD BCST \[ecx\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 48 5a 71 7f[ 	 ]*vcvtpd2ph xmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd df 5a 72 80[ 	 ]*vcvtpd2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 5b f5[ 	 ]*vcvtph2dq zmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 5b f5[ 	 ]*vcvtph2dq zmm6,ymm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 5b f5[ 	 ]*vcvtph2dq zmm6\{k7\}\{z\},ymm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 5b b4 f4 00 00 00 10[ 	 ]*vcvtph2dq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 5b 31[ 	 ]*vcvtph2dq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 5b 71 7f[ 	 ]*vcvtph2dq zmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 5b 72 80[ 	 ]*vcvtph2dq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5a f5[ 	 ]*vcvtph2pd zmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5a f5[ 	 ]*vcvtph2pd zmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5a f5[ 	 ]*vcvtph2pd zmm6\{k7\}\{z\},xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 5a b4 f4 00 00 00 10[ 	 ]*vcvtph2pd zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 5a 31[ 	 ]*vcvtph2pd zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5a 71 7f[ 	 ]*vcvtph2pd zmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 5a 72 80[ 	 ]*vcvtph2pd zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 13 f5[ 	 ]*vcvtph2psx zmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 13 f5[ 	 ]*vcvtph2psx zmm6,ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 13 f5[ 	 ]*vcvtph2psx zmm6\{k7\}\{z\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 13 b4 f4 00 00 00 10[ 	 ]*vcvtph2psx zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 13 31[ 	 ]*vcvtph2psx zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 13 71 7f[ 	 ]*vcvtph2psx zmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 13 72 80[ 	 ]*vcvtph2psx zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7b f5[ 	 ]*vcvtph2qq zmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7b f5[ 	 ]*vcvtph2qq zmm6,xmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7b f5[ 	 ]*vcvtph2qq zmm6\{k7\}\{z\},xmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7b b4 f4 00 00 00 10[ 	 ]*vcvtph2qq zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7b 31[ 	 ]*vcvtph2qq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7b 71 7f[ 	 ]*vcvtph2qq zmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7b 72 80[ 	 ]*vcvtph2qq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 79 f5[ 	 ]*vcvtph2udq zmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 79 f5[ 	 ]*vcvtph2udq zmm6,ymm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 79 f5[ 	 ]*vcvtph2udq zmm6\{k7\}\{z\},ymm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 79 b4 f4 00 00 00 10[ 	 ]*vcvtph2udq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 79 31[ 	 ]*vcvtph2udq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 79 71 7f[ 	 ]*vcvtph2udq zmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 79 72 80[ 	 ]*vcvtph2udq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 79 f5[ 	 ]*vcvtph2uqq zmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 79 f5[ 	 ]*vcvtph2uqq zmm6,xmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 79 f5[ 	 ]*vcvtph2uqq zmm6\{k7\}\{z\},xmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 79 b4 f4 00 00 00 10[ 	 ]*vcvtph2uqq zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 79 31[ 	 ]*vcvtph2uqq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 79 71 7f[ 	 ]*vcvtph2uqq zmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 79 72 80[ 	 ]*vcvtph2uqq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7d f5[ 	 ]*vcvtph2uw zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7d f5[ 	 ]*vcvtph2uw zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7d f5[ 	 ]*vcvtph2uw zmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 7d b4 f4 00 00 00 10[ 	 ]*vcvtph2uw zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 7d 31[ 	 ]*vcvtph2uw zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7d 71 7f[ 	 ]*vcvtph2uw zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 7d 72 80[ 	 ]*vcvtph2uw zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7d f5[ 	 ]*vcvtph2w zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7d f5[ 	 ]*vcvtph2w zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7d f5[ 	 ]*vcvtph2w zmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7d b4 f4 00 00 00 10[ 	 ]*vcvtph2w zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7d 31[ 	 ]*vcvtph2w zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7d 71 7f[ 	 ]*vcvtph2w zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7d 72 80[ 	 ]*vcvtph2w zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 1d f5[ 	 ]*vcvtps2phx ymm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 1d f5[ 	 ]*vcvtps2phx ymm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 1d f5[ 	 ]*vcvtps2phx ymm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 1d b4 f4 00 00 00 10[ 	 ]*vcvtps2phx ymm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 1d 31[ 	 ]*vcvtps2phx ymm6,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 1d 71 7f[ 	 ]*vcvtps2phx ymm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 1d 72 80[ 	 ]*vcvtps2phx ymm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 48 5b f5[ 	 ]*vcvtqq2ph xmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 18 5b f5[ 	 ]*vcvtqq2ph xmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 9f 5b f5[ 	 ]*vcvtqq2ph xmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 4f 5b b4 f4 00 00 00 10[ 	 ]*vcvtqq2ph xmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 58 5b 31[ 	 ]*vcvtqq2ph xmm6,QWORD BCST \[ecx\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 48 5b 71 7f[ 	 ]*vcvtqq2ph xmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc df 5b 72 80[ 	 ]*vcvtqq2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 08 5a f4[ 	 ]*vcvtsd2sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 18 5a f4[ 	 ]*vcvtsd2sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 9f 5a f4[ 	 ]*vcvtsd2sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 0f 5a b4 f4 00 00 00 10[ 	 ]*vcvtsd2sh xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 08 5a 31[ 	 ]*vcvtsd2sh xmm6,xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 08 5a 71 7f[ 	 ]*vcvtsd2sh xmm6,xmm5,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 8f 5a 72 80[ 	 ]*vcvtsd2sh xmm6\{k7\}\{z\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5a f4[ 	 ]*vcvtsh2sd xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5a f4[ 	 ]*vcvtsh2sd xmm6,xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5a f4[ 	 ]*vcvtsh2sd xmm6\{k7\}\{z\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5a b4 f4 00 00 00 10[ 	 ]*vcvtsh2sd xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5a 31[ 	 ]*vcvtsh2sd xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5a 71 7f[ 	 ]*vcvtsh2sd xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5a 72 80[ 	 ]*vcvtsh2sd xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d d6[ 	 ]*vcvtsh2si edx,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 2d d6[ 	 ]*vcvtsh2si edx,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 94 f4 00 00 00 10[ 	 ]*vcvtsh2si edx,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 11[ 	 ]*vcvtsh2si edx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 51 7f[ 	 ]*vcvtsh2si edx,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 52 80[ 	 ]*vcvtsh2si edx,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 08 13 f4[ 	 ]*vcvtsh2ss xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 18 13 f4[ 	 ]*vcvtsh2ss xmm6,xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 9f 13 f4[ 	 ]*vcvtsh2ss xmm6\{k7\}\{z\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 0f 13 b4 f4 00 00 00 10[ 	 ]*vcvtsh2ss xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 08 13 31[ 	 ]*vcvtsh2ss xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 08 13 71 7f[ 	 ]*vcvtsh2ss xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 8f 13 72 80[ 	 ]*vcvtsh2ss xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 d6[ 	 ]*vcvtsh2usi edx,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 79 d6[ 	 ]*vcvtsh2usi edx,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 94 f4 00 00 00 10[ 	 ]*vcvtsh2usi edx,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 11[ 	 ]*vcvtsh2usi edx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 51 7f[ 	 ]*vcvtsh2usi edx,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 52 80[ 	 ]*vcvtsh2usi edx,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a f2[ 	 ]*vcvtsi2sh xmm6,xmm5,edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 2a f2[ 	 ]*vcvtsi2sh xmm6,xmm5,edx\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a b4 f4 00 00 00 10[ 	 ]*vcvtsi2sh xmm6,xmm5,DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a 31[ 	 ]*vcvtsi2sh xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a 71 7f[ 	 ]*vcvtsi2sh xmm6,xmm5,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a 72 80[ 	 ]*vcvtsi2sh xmm6,xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 1d f4[ 	 ]*vcvtss2sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 1d f4[ 	 ]*vcvtss2sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 1d f4[ 	 ]*vcvtss2sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 1d b4 f4 00 00 00 10[ 	 ]*vcvtss2sh xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 1d 31[ 	 ]*vcvtss2sh xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 1d 71 7f[ 	 ]*vcvtss2sh xmm6,xmm5,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 1d 72 80[ 	 ]*vcvtss2sh xmm6\{k7\}\{z\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 5b f5[ 	 ]*vcvttph2dq zmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 5b f5[ 	 ]*vcvttph2dq zmm6,ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 5b f5[ 	 ]*vcvttph2dq zmm6\{k7\}\{z\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 4f 5b b4 f4 00 00 00 10[ 	 ]*vcvttph2dq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 58 5b 31[ 	 ]*vcvttph2dq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 5b 71 7f[ 	 ]*vcvttph2dq zmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e df 5b 72 80[ 	 ]*vcvttph2dq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7a f5[ 	 ]*vcvttph2qq zmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7a f5[ 	 ]*vcvttph2qq zmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7a f5[ 	 ]*vcvttph2qq zmm6\{k7\}\{z\},xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7a b4 f4 00 00 00 10[ 	 ]*vcvttph2qq zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7a 31[ 	 ]*vcvttph2qq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7a 71 7f[ 	 ]*vcvttph2qq zmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7a 72 80[ 	 ]*vcvttph2qq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 78 f5[ 	 ]*vcvttph2udq zmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 78 f5[ 	 ]*vcvttph2udq zmm6,ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 78 f5[ 	 ]*vcvttph2udq zmm6\{k7\}\{z\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 78 b4 f4 00 00 00 10[ 	 ]*vcvttph2udq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 78 31[ 	 ]*vcvttph2udq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 78 71 7f[ 	 ]*vcvttph2udq zmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 78 72 80[ 	 ]*vcvttph2udq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 78 f5[ 	 ]*vcvttph2uqq zmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 78 f5[ 	 ]*vcvttph2uqq zmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 78 f5[ 	 ]*vcvttph2uqq zmm6\{k7\}\{z\},xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 78 b4 f4 00 00 00 10[ 	 ]*vcvttph2uqq zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 78 31[ 	 ]*vcvttph2uqq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 78 71 7f[ 	 ]*vcvttph2uqq zmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 78 72 80[ 	 ]*vcvttph2uqq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7c f5[ 	 ]*vcvttph2uw zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7c f5[ 	 ]*vcvttph2uw zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7c f5[ 	 ]*vcvttph2uw zmm6\{k7\}\{z\},zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 7c b4 f4 00 00 00 10[ 	 ]*vcvttph2uw zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 7c 31[ 	 ]*vcvttph2uw zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7c 71 7f[ 	 ]*vcvttph2uw zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 7c 72 80[ 	 ]*vcvttph2uw zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7c f5[ 	 ]*vcvttph2w zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7c f5[ 	 ]*vcvttph2w zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7c f5[ 	 ]*vcvttph2w zmm6\{k7\}\{z\},zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7c b4 f4 00 00 00 10[ 	 ]*vcvttph2w zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7c 31[ 	 ]*vcvttph2w zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7c 71 7f[ 	 ]*vcvttph2w zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7c 72 80[ 	 ]*vcvttph2w zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c d6[ 	 ]*vcvttsh2si edx,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 2c d6[ 	 ]*vcvttsh2si edx,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 94 f4 00 00 00 10[ 	 ]*vcvttsh2si edx,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 11[ 	 ]*vcvttsh2si edx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 51 7f[ 	 ]*vcvttsh2si edx,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 52 80[ 	 ]*vcvttsh2si edx,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 d6[ 	 ]*vcvttsh2usi edx,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 78 d6[ 	 ]*vcvttsh2usi edx,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 94 f4 00 00 00 10[ 	 ]*vcvttsh2usi edx,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 11[ 	 ]*vcvttsh2usi edx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 51 7f[ 	 ]*vcvttsh2usi edx,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 52 80[ 	 ]*vcvttsh2usi edx,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7a f5[ 	 ]*vcvtudq2ph ymm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7a f5[ 	 ]*vcvtudq2ph ymm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7a f5[ 	 ]*vcvtudq2ph ymm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 4f 7a b4 f4 00 00 00 10[ 	 ]*vcvtudq2ph ymm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 58 7a 31[ 	 ]*vcvtudq2ph ymm6,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7a 71 7f[ 	 ]*vcvtudq2ph ymm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f df 7a 72 80[ 	 ]*vcvtudq2ph ymm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 48 7a f5[ 	 ]*vcvtuqq2ph xmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 18 7a f5[ 	 ]*vcvtuqq2ph xmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 9f 7a f5[ 	 ]*vcvtuqq2ph xmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 4f 7a b4 f4 00 00 00 10[ 	 ]*vcvtuqq2ph xmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 58 7a 31[ 	 ]*vcvtuqq2ph xmm6,QWORD BCST \[ecx\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 48 7a 71 7f[ 	 ]*vcvtuqq2ph xmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff df 7a 72 80[ 	 ]*vcvtuqq2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b f2[ 	 ]*vcvtusi2sh xmm6,xmm5,edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 7b f2[ 	 ]*vcvtusi2sh xmm6,xmm5,edx\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b b4 f4 00 00 00 10[ 	 ]*vcvtusi2sh xmm6,xmm5,DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b 31[ 	 ]*vcvtusi2sh xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b 71 7f[ 	 ]*vcvtusi2sh xmm6,xmm5,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b 72 80[ 	 ]*vcvtusi2sh xmm6,xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7d f5[ 	 ]*vcvtuw2ph zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7d f5[ 	 ]*vcvtuw2ph zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7d f5[ 	 ]*vcvtuw2ph zmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 4f 7d b4 f4 00 00 00 10[ 	 ]*vcvtuw2ph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 58 7d 31[ 	 ]*vcvtuw2ph zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7d 71 7f[ 	 ]*vcvtuw2ph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f df 7d 72 80[ 	 ]*vcvtuw2ph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 7d f5[ 	 ]*vcvtw2ph zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 7d f5[ 	 ]*vcvtw2ph zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 7d f5[ 	 ]*vcvtw2ph zmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 4f 7d b4 f4 00 00 00 10[ 	 ]*vcvtw2ph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 58 7d 31[ 	 ]*vcvtw2ph zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 7d 71 7f[ 	 ]*vcvtw2ph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e df 7d 72 80[ 	 ]*vcvtw2ph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5e f4[ 	 ]*vdivph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5e f4[ 	 ]*vdivph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5e f4[ 	 ]*vdivph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5e b4 f4 00 00 00 10[ 	 ]*vdivph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5e 31[ 	 ]*vdivph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5e 71 7f[ 	 ]*vdivph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5e 72 80[ 	 ]*vdivph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5e f4[ 	 ]*vdivsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5e f4[ 	 ]*vdivsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5e f4[ 	 ]*vdivsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5e b4 f4 00 00 00 10[ 	 ]*vdivsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5e 31[ 	 ]*vdivsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5e 71 7f[ 	 ]*vdivsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5e 72 80[ 	 ]*vdivsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 56 f4[ 	 ]*vfcmaddcph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 56 f4[ 	 ]*vfcmaddcph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f 56 f4[ 	 ]*vfcmaddcph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 4f 56 b4 f4 00 00 00 10[ 	 ]*vfcmaddcph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 58 56 31[ 	 ]*vfcmaddcph zmm6,zmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 56 71 7f[ 	 ]*vfcmaddcph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 df 56 72 80[ 	 ]*vfcmaddcph zmm6\{k7\}\{z\},zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 57 f4[ 	 ]*vfcmaddcsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 57 f4[ 	 ]*vfcmaddcsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f 57 f4[ 	 ]*vfcmaddcsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f 57 b4 f4 00 00 00 10[ 	 ]*vfcmaddcsh xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 57 31[ 	 ]*vfcmaddcsh xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 57 71 7f[ 	 ]*vfcmaddcsh xmm6,xmm5,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f 57 72 80[ 	 ]*vfcmaddcsh xmm6\{k7\}\{z\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 d6 f4[ 	 ]*vfcmulcph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 d6 f4[ 	 ]*vfcmulcph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f d6 f4[ 	 ]*vfcmulcph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 4f d6 b4 f4 00 00 00 10[ 	 ]*vfcmulcph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 58 d6 31[ 	 ]*vfcmulcph zmm6,zmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 d6 71 7f[ 	 ]*vfcmulcph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 df d6 72 80[ 	 ]*vfcmulcph zmm6\{k7\}\{z\},zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d7 f4[ 	 ]*vfcmulcsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 d7 f4[ 	 ]*vfcmulcsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f d7 f4[ 	 ]*vfcmulcsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f d7 b4 f4 00 00 00 10[ 	 ]*vfcmulcsh xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d7 31[ 	 ]*vfcmulcsh xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d7 71 7f[ 	 ]*vfcmulcsh xmm6,xmm5,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f d7 72 80[ 	 ]*vfcmulcsh xmm6\{k7\}\{z\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 98 f4[ 	 ]*vfmadd132ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 98 f4[ 	 ]*vfmadd132ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 98 f4[ 	 ]*vfmadd132ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 98 b4 f4 00 00 00 10[ 	 ]*vfmadd132ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 98 31[ 	 ]*vfmadd132ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 98 71 7f[ 	 ]*vfmadd132ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 98 72 80[ 	 ]*vfmadd132ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 99 f4[ 	 ]*vfmadd132sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 99 f4[ 	 ]*vfmadd132sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 99 f4[ 	 ]*vfmadd132sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 99 b4 f4 00 00 00 10[ 	 ]*vfmadd132sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 99 31[ 	 ]*vfmadd132sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 99 71 7f[ 	 ]*vfmadd132sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 99 72 80[ 	 ]*vfmadd132sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a8 f4[ 	 ]*vfmadd213ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a8 f4[ 	 ]*vfmadd213ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a8 f4[ 	 ]*vfmadd213ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f a8 b4 f4 00 00 00 10[ 	 ]*vfmadd213ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 a8 31[ 	 ]*vfmadd213ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a8 71 7f[ 	 ]*vfmadd213ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df a8 72 80[ 	 ]*vfmadd213ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a9 f4[ 	 ]*vfmadd213sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a9 f4[ 	 ]*vfmadd213sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a9 f4[ 	 ]*vfmadd213sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a9 b4 f4 00 00 00 10[ 	 ]*vfmadd213sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a9 31[ 	 ]*vfmadd213sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a9 71 7f[ 	 ]*vfmadd213sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a9 72 80[ 	 ]*vfmadd213sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b8 f4[ 	 ]*vfmadd231ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b8 f4[ 	 ]*vfmadd231ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b8 f4[ 	 ]*vfmadd231ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f b8 b4 f4 00 00 00 10[ 	 ]*vfmadd231ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 b8 31[ 	 ]*vfmadd231ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b8 71 7f[ 	 ]*vfmadd231ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df b8 72 80[ 	 ]*vfmadd231ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b9 f4[ 	 ]*vfmadd231sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b9 f4[ 	 ]*vfmadd231sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b9 f4[ 	 ]*vfmadd231sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b9 b4 f4 00 00 00 10[ 	 ]*vfmadd231sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b9 31[ 	 ]*vfmadd231sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b9 71 7f[ 	 ]*vfmadd231sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b9 72 80[ 	 ]*vfmadd231sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 56 f4[ 	 ]*vfmaddcph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 56 f4[ 	 ]*vfmaddcph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f 56 f4[ 	 ]*vfmaddcph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 4f 56 b4 f4 00 00 00 10[ 	 ]*vfmaddcph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 58 56 31[ 	 ]*vfmaddcph zmm6,zmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 56 71 7f[ 	 ]*vfmaddcph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 df 56 72 80[ 	 ]*vfmaddcph zmm6\{k7\}\{z\},zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 57 f4[ 	 ]*vfmaddcsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 57 f4[ 	 ]*vfmaddcsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f 57 f4[ 	 ]*vfmaddcsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f 57 b4 f4 00 00 00 10[ 	 ]*vfmaddcsh xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 57 31[ 	 ]*vfmaddcsh xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 57 71 7f[ 	 ]*vfmaddcsh xmm6,xmm5,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f 57 72 80[ 	 ]*vfmaddcsh xmm6\{k7\}\{z\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 96 f4[ 	 ]*vfmaddsub132ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 96 f4[ 	 ]*vfmaddsub132ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 96 f4[ 	 ]*vfmaddsub132ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 96 b4 f4 00 00 00 10[ 	 ]*vfmaddsub132ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 96 31[ 	 ]*vfmaddsub132ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 96 71 7f[ 	 ]*vfmaddsub132ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 96 72 80[ 	 ]*vfmaddsub132ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a6 f4[ 	 ]*vfmaddsub213ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a6 f4[ 	 ]*vfmaddsub213ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a6 f4[ 	 ]*vfmaddsub213ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f a6 b4 f4 00 00 00 10[ 	 ]*vfmaddsub213ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 a6 31[ 	 ]*vfmaddsub213ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a6 71 7f[ 	 ]*vfmaddsub213ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df a6 72 80[ 	 ]*vfmaddsub213ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b6 f4[ 	 ]*vfmaddsub231ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b6 f4[ 	 ]*vfmaddsub231ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b6 f4[ 	 ]*vfmaddsub231ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f b6 b4 f4 00 00 00 10[ 	 ]*vfmaddsub231ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 b6 31[ 	 ]*vfmaddsub231ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b6 71 7f[ 	 ]*vfmaddsub231ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df b6 72 80[ 	 ]*vfmaddsub231ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9a f4[ 	 ]*vfmsub132ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9a f4[ 	 ]*vfmsub132ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9a f4[ 	 ]*vfmsub132ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 9a b4 f4 00 00 00 10[ 	 ]*vfmsub132ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 9a 31[ 	 ]*vfmsub132ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9a 71 7f[ 	 ]*vfmsub132ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 9a 72 80[ 	 ]*vfmsub132ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9b f4[ 	 ]*vfmsub132sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9b f4[ 	 ]*vfmsub132sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9b f4[ 	 ]*vfmsub132sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9b b4 f4 00 00 00 10[ 	 ]*vfmsub132sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9b 31[ 	 ]*vfmsub132sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9b 71 7f[ 	 ]*vfmsub132sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9b 72 80[ 	 ]*vfmsub132sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 aa f4[ 	 ]*vfmsub213ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 aa f4[ 	 ]*vfmsub213ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f aa f4[ 	 ]*vfmsub213ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f aa b4 f4 00 00 00 10[ 	 ]*vfmsub213ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 aa 31[ 	 ]*vfmsub213ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 aa 71 7f[ 	 ]*vfmsub213ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df aa 72 80[ 	 ]*vfmsub213ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ab f4[ 	 ]*vfmsub213sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ab f4[ 	 ]*vfmsub213sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ab f4[ 	 ]*vfmsub213sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ab b4 f4 00 00 00 10[ 	 ]*vfmsub213sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ab 31[ 	 ]*vfmsub213sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ab 71 7f[ 	 ]*vfmsub213sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ab 72 80[ 	 ]*vfmsub213sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ba f4[ 	 ]*vfmsub231ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ba f4[ 	 ]*vfmsub231ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ba f4[ 	 ]*vfmsub231ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f ba b4 f4 00 00 00 10[ 	 ]*vfmsub231ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 ba 31[ 	 ]*vfmsub231ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ba 71 7f[ 	 ]*vfmsub231ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df ba 72 80[ 	 ]*vfmsub231ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bb f4[ 	 ]*vfmsub231sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bb f4[ 	 ]*vfmsub231sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bb f4[ 	 ]*vfmsub231sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bb b4 f4 00 00 00 10[ 	 ]*vfmsub231sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bb 31[ 	 ]*vfmsub231sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bb 71 7f[ 	 ]*vfmsub231sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bb 72 80[ 	 ]*vfmsub231sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 97 f4[ 	 ]*vfmsubadd132ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 97 f4[ 	 ]*vfmsubadd132ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 97 f4[ 	 ]*vfmsubadd132ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 97 b4 f4 00 00 00 10[ 	 ]*vfmsubadd132ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 97 31[ 	 ]*vfmsubadd132ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 97 71 7f[ 	 ]*vfmsubadd132ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 97 72 80[ 	 ]*vfmsubadd132ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a7 f4[ 	 ]*vfmsubadd213ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a7 f4[ 	 ]*vfmsubadd213ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a7 f4[ 	 ]*vfmsubadd213ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f a7 b4 f4 00 00 00 10[ 	 ]*vfmsubadd213ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 a7 31[ 	 ]*vfmsubadd213ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a7 71 7f[ 	 ]*vfmsubadd213ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df a7 72 80[ 	 ]*vfmsubadd213ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b7 f4[ 	 ]*vfmsubadd231ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b7 f4[ 	 ]*vfmsubadd231ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b7 f4[ 	 ]*vfmsubadd231ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f b7 b4 f4 00 00 00 10[ 	 ]*vfmsubadd231ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 b7 31[ 	 ]*vfmsubadd231ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b7 71 7f[ 	 ]*vfmsubadd231ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df b7 72 80[ 	 ]*vfmsubadd231ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 d6 f4[ 	 ]*vfmulcph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 d6 f4[ 	 ]*vfmulcph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f d6 f4[ 	 ]*vfmulcph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 4f d6 b4 f4 00 00 00 10[ 	 ]*vfmulcph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 58 d6 31[ 	 ]*vfmulcph zmm6,zmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 d6 71 7f[ 	 ]*vfmulcph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 df d6 72 80[ 	 ]*vfmulcph zmm6\{k7\}\{z\},zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d7 f4[ 	 ]*vfmulcsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 d7 f4[ 	 ]*vfmulcsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f d7 f4[ 	 ]*vfmulcsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f d7 b4 f4 00 00 00 10[ 	 ]*vfmulcsh xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d7 31[ 	 ]*vfmulcsh xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d7 71 7f[ 	 ]*vfmulcsh xmm6,xmm5,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f d7 72 80[ 	 ]*vfmulcsh xmm6\{k7\}\{z\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9c f4[ 	 ]*vfnmadd132ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9c f4[ 	 ]*vfnmadd132ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9c f4[ 	 ]*vfnmadd132ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 9c b4 f4 00 00 00 10[ 	 ]*vfnmadd132ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 9c 31[ 	 ]*vfnmadd132ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9c 71 7f[ 	 ]*vfnmadd132ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 9c 72 80[ 	 ]*vfnmadd132ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9d f4[ 	 ]*vfnmadd132sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9d f4[ 	 ]*vfnmadd132sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9d f4[ 	 ]*vfnmadd132sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9d b4 f4 00 00 00 10[ 	 ]*vfnmadd132sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9d 31[ 	 ]*vfnmadd132sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9d 71 7f[ 	 ]*vfnmadd132sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9d 72 80[ 	 ]*vfnmadd132sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ac f4[ 	 ]*vfnmadd213ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ac f4[ 	 ]*vfnmadd213ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ac f4[ 	 ]*vfnmadd213ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f ac b4 f4 00 00 00 10[ 	 ]*vfnmadd213ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 ac 31[ 	 ]*vfnmadd213ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ac 71 7f[ 	 ]*vfnmadd213ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df ac 72 80[ 	 ]*vfnmadd213ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ad f4[ 	 ]*vfnmadd213sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ad f4[ 	 ]*vfnmadd213sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ad f4[ 	 ]*vfnmadd213sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ad b4 f4 00 00 00 10[ 	 ]*vfnmadd213sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ad 31[ 	 ]*vfnmadd213sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ad 71 7f[ 	 ]*vfnmadd213sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ad 72 80[ 	 ]*vfnmadd213sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 bc f4[ 	 ]*vfnmadd231ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bc f4[ 	 ]*vfnmadd231ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bc f4[ 	 ]*vfnmadd231ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f bc b4 f4 00 00 00 10[ 	 ]*vfnmadd231ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 bc 31[ 	 ]*vfnmadd231ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 bc 71 7f[ 	 ]*vfnmadd231ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df bc 72 80[ 	 ]*vfnmadd231ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bd f4[ 	 ]*vfnmadd231sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bd f4[ 	 ]*vfnmadd231sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bd f4[ 	 ]*vfnmadd231sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bd b4 f4 00 00 00 10[ 	 ]*vfnmadd231sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bd 31[ 	 ]*vfnmadd231sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bd 71 7f[ 	 ]*vfnmadd231sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bd 72 80[ 	 ]*vfnmadd231sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9e f4[ 	 ]*vfnmsub132ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9e f4[ 	 ]*vfnmsub132ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9e f4[ 	 ]*vfnmsub132ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 9e b4 f4 00 00 00 10[ 	 ]*vfnmsub132ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 9e 31[ 	 ]*vfnmsub132ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9e 71 7f[ 	 ]*vfnmsub132ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 9e 72 80[ 	 ]*vfnmsub132ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9f f4[ 	 ]*vfnmsub132sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9f f4[ 	 ]*vfnmsub132sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9f f4[ 	 ]*vfnmsub132sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9f b4 f4 00 00 00 10[ 	 ]*vfnmsub132sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9f 31[ 	 ]*vfnmsub132sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9f 71 7f[ 	 ]*vfnmsub132sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9f 72 80[ 	 ]*vfnmsub132sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ae f4[ 	 ]*vfnmsub213ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ae f4[ 	 ]*vfnmsub213ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ae f4[ 	 ]*vfnmsub213ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f ae b4 f4 00 00 00 10[ 	 ]*vfnmsub213ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 ae 31[ 	 ]*vfnmsub213ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ae 71 7f[ 	 ]*vfnmsub213ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df ae 72 80[ 	 ]*vfnmsub213ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 af f4[ 	 ]*vfnmsub213sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 af f4[ 	 ]*vfnmsub213sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f af f4[ 	 ]*vfnmsub213sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f af b4 f4 00 00 00 10[ 	 ]*vfnmsub213sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 af 31[ 	 ]*vfnmsub213sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 af 71 7f[ 	 ]*vfnmsub213sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f af 72 80[ 	 ]*vfnmsub213sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 be f4[ 	 ]*vfnmsub231ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 be f4[ 	 ]*vfnmsub231ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f be f4[ 	 ]*vfnmsub231ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f be b4 f4 00 00 00 10[ 	 ]*vfnmsub231ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 be 31[ 	 ]*vfnmsub231ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 be 71 7f[ 	 ]*vfnmsub231ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df be 72 80[ 	 ]*vfnmsub231ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bf f4[ 	 ]*vfnmsub231sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bf f4[ 	 ]*vfnmsub231sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bf f4[ 	 ]*vfnmsub231sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bf b4 f4 00 00 00 10[ 	 ]*vfnmsub231sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bf 31[ 	 ]*vfnmsub231sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bf 71 7f[ 	 ]*vfnmsub231sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bf 72 80[ 	 ]*vfnmsub231sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 66 ee 7b[ 	 ]*vfpclassph k5,zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 66 ee 7b[ 	 ]*vfpclassph k5\{k7\},zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 66 ac f4 00 00 00 10 7b[ 	 ]*vfpclassph k5\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 66 29 7b[ 	 ]*vfpclassph k5,WORD BCST \[ecx\]\{1to32\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 66 69 7f 7b[ 	 ]*vfpclassph k5,ZMMWORD PTR \[ecx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 5f 66 6a 80 7b[ 	 ]*vfpclassph k5\{k7\},WORD BCST \[edx-0x100\]\{1to32\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 66 69 01 7b[ 	 ]*vfpclassph k5,WORD BCST \[ecx\+0x2\]\{1to32\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 67 ee 7b[ 	 ]*vfpclasssh k5,xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 67 ee 7b[ 	 ]*vfpclasssh k5\{k7\},xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 67 ac f4 00 00 00 10 7b[ 	 ]*vfpclasssh k5\{k7\},WORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 67 29 7b[ 	 ]*vfpclasssh k5,WORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 67 69 7f 7b[ 	 ]*vfpclasssh k5,WORD PTR \[ecx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 67 6a 80 7b[ 	 ]*vfpclasssh k5\{k7\},WORD PTR \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 42 f5[ 	 ]*vgetexpph zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 42 f5[ 	 ]*vgetexpph zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 42 f5[ 	 ]*vgetexpph zmm6\{k7\}\{z\},zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 42 b4 f4 00 00 00 10[ 	 ]*vgetexpph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 42 31[ 	 ]*vgetexpph zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 42 71 7f[ 	 ]*vgetexpph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 42 72 80[ 	 ]*vgetexpph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 43 f4[ 	 ]*vgetexpsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 43 f4[ 	 ]*vgetexpsh xmm6,xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 43 f4[ 	 ]*vgetexpsh xmm6\{k7\}\{z\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 43 b4 f4 00 00 00 10[ 	 ]*vgetexpsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 43 31[ 	 ]*vgetexpsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 43 71 7f[ 	 ]*vgetexpsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 43 72 80[ 	 ]*vgetexpsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 26 f5 7b[ 	 ]*vgetmantph zmm6,zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 26 f5 7b[ 	 ]*vgetmantph zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 26 f5 7b[ 	 ]*vgetmantph zmm6\{k7\}\{z\},zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 26 b4 f4 00 00 00 10 7b[ 	 ]*vgetmantph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 26 31 7b[ 	 ]*vgetmantph zmm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 26 71 7f 7b[ 	 ]*vgetmantph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c df 26 72 80 7b[ 	 ]*vgetmantph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 27 f4 7b[ 	 ]*vgetmantsh xmm6,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 27 f4 7b[ 	 ]*vgetmantsh xmm6,xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 9f 27 f4 7b[ 	 ]*vgetmantsh xmm6\{k7\}\{z\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f 27 b4 f4 00 00 00 10 7b[ 	 ]*vgetmantsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 27 31 7b[ 	 ]*vgetmantsh xmm6,xmm5,WORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 27 71 7f 7b[ 	 ]*vgetmantsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 8f 27 72 80 7b[ 	 ]*vgetmantsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5f f4[ 	 ]*vmaxph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5f f4[ 	 ]*vmaxph zmm6,zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5f f4[ 	 ]*vmaxph zmm6\{k7\}\{z\},zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5f b4 f4 00 00 00 10[ 	 ]*vmaxph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5f 31[ 	 ]*vmaxph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5f 71 7f[ 	 ]*vmaxph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5f 72 80[ 	 ]*vmaxph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5f f4[ 	 ]*vmaxsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5f f4[ 	 ]*vmaxsh xmm6,xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5f f4[ 	 ]*vmaxsh xmm6\{k7\}\{z\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5f b4 f4 00 00 00 10[ 	 ]*vmaxsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5f 31[ 	 ]*vmaxsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5f 71 7f[ 	 ]*vmaxsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5f 72 80[ 	 ]*vmaxsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5d f4[ 	 ]*vminph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5d f4[ 	 ]*vminph zmm6,zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5d f4[ 	 ]*vminph zmm6\{k7\}\{z\},zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5d b4 f4 00 00 00 10[ 	 ]*vminph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5d 31[ 	 ]*vminph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5d 71 7f[ 	 ]*vminph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5d 72 80[ 	 ]*vminph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5d f4[ 	 ]*vminsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5d f4[ 	 ]*vminsh xmm6,xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5d f4[ 	 ]*vminsh xmm6\{k7\}\{z\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5d b4 f4 00 00 00 10[ 	 ]*vminsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5d 31[ 	 ]*vminsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5d 71 7f[ 	 ]*vminsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5d 72 80[ 	 ]*vminsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 10 f4[ 	 ]*vmovsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 10 f4[ 	 ]*vmovsh xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 10 b4 f4 00 00 00 10[ 	 ]*vmovsh xmm6\{k7\},WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 10 31[ 	 ]*vmovsh xmm6,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 10 71 7f[ 	 ]*vmovsh xmm6,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 8f 10 72 80[ 	 ]*vmovsh xmm6\{k7\}\{z\},WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 11 b4 f4 00 00 00 10[ 	 ]*vmovsh WORD PTR \[esp\+esi\*8\+0x10000000\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 11 31[ 	 ]*vmovsh WORD PTR \[ecx\],xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 11 71 7f[ 	 ]*vmovsh WORD PTR \[ecx\+0xfe\],xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 11 72 80[ 	 ]*vmovsh WORD PTR \[edx-0x100\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e f2[ 	 ]*vmovw  xmm6,edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e f2[ 	 ]*vmovw  edx,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e b4 f4 00 00 00 10[ 	 ]*vmovw  xmm6,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e 31[ 	 ]*vmovw  xmm6,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e 71 7f[ 	 ]*vmovw  xmm6,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e 72 80[ 	 ]*vmovw  xmm6,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e b4 f4 00 00 00 10[ 	 ]*vmovw  WORD PTR \[esp\+esi\*8\+0x10000000\],xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e 31[ 	 ]*vmovw  WORD PTR \[ecx\],xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e 71 7f[ 	 ]*vmovw  WORD PTR \[ecx\+0xfe\],xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e 72 80[ 	 ]*vmovw  WORD PTR \[edx-0x100\],xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 59 f4[ 	 ]*vmulph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 59 f4[ 	 ]*vmulph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 59 f4[ 	 ]*vmulph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 59 b4 f4 00 00 00 10[ 	 ]*vmulph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 59 31[ 	 ]*vmulph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 59 71 7f[ 	 ]*vmulph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 59 72 80[ 	 ]*vmulph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 59 f4[ 	 ]*vmulsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 59 f4[ 	 ]*vmulsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 59 f4[ 	 ]*vmulsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 59 b4 f4 00 00 00 10[ 	 ]*vmulsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 59 31[ 	 ]*vmulsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 59 71 7f[ 	 ]*vmulsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 59 72 80[ 	 ]*vmulsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4c f5[ 	 ]*vrcpph zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d cf 4c f5[ 	 ]*vrcpph zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 4c b4 f4 00 00 00 10[ 	 ]*vrcpph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 4c 31[ 	 ]*vrcpph zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4c 71 7f[ 	 ]*vrcpph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 4c 72 80[ 	 ]*vrcpph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4d f4[ 	 ]*vrcpsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4d f4[ 	 ]*vrcpsh xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 4d b4 f4 00 00 00 10[ 	 ]*vrcpsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4d 31[ 	 ]*vrcpsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4d 71 7f[ 	 ]*vrcpsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4d 72 80[ 	 ]*vrcpsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 56 f5 7b[ 	 ]*vreduceph zmm6,zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 56 f5 7b[ 	 ]*vreduceph zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 56 f5 7b[ 	 ]*vreduceph zmm6\{k7\}\{z\},zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 56 b4 f4 00 00 00 10 7b[ 	 ]*vreduceph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 56 31 7b[ 	 ]*vreduceph zmm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 56 71 7f 7b[ 	 ]*vreduceph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c df 56 72 80 7b[ 	 ]*vreduceph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 57 f4 7b[ 	 ]*vreducesh xmm6,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 57 f4 7b[ 	 ]*vreducesh xmm6,xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 9f 57 f4 7b[ 	 ]*vreducesh xmm6\{k7\}\{z\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f 57 b4 f4 00 00 00 10 7b[ 	 ]*vreducesh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 57 31 7b[ 	 ]*vreducesh xmm6,xmm5,WORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 57 71 7f 7b[ 	 ]*vreducesh xmm6,xmm5,WORD PTR \[ecx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 8f 57 72 80 7b[ 	 ]*vreducesh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 08 f5 7b[ 	 ]*vrndscaleph zmm6,zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 08 f5 7b[ 	 ]*vrndscaleph zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 08 f5 7b[ 	 ]*vrndscaleph zmm6\{k7\}\{z\},zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 08 b4 f4 00 00 00 10 7b[ 	 ]*vrndscaleph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 08 31 7b[ 	 ]*vrndscaleph zmm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 08 71 7f 7b[ 	 ]*vrndscaleph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c df 08 72 80 7b[ 	 ]*vrndscaleph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 0a f4 7b[ 	 ]*vrndscalesh xmm6,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 0a f4 7b[ 	 ]*vrndscalesh xmm6,xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 9f 0a f4 7b[ 	 ]*vrndscalesh xmm6\{k7\}\{z\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f 0a b4 f4 00 00 00 10 7b[ 	 ]*vrndscalesh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 0a 31 7b[ 	 ]*vrndscalesh xmm6,xmm5,WORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 0a 71 7f 7b[ 	 ]*vrndscalesh xmm6,xmm5,WORD PTR \[ecx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 8f 0a 72 80 7b[ 	 ]*vrndscalesh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4e f5[ 	 ]*vrsqrtph zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d cf 4e f5[ 	 ]*vrsqrtph zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 4e b4 f4 00 00 00 10[ 	 ]*vrsqrtph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 4e 31[ 	 ]*vrsqrtph zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4e 71 7f[ 	 ]*vrsqrtph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 4e 72 80[ 	 ]*vrsqrtph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4f f4[ 	 ]*vrsqrtsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4f f4[ 	 ]*vrsqrtsh xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 4f b4 f4 00 00 00 10[ 	 ]*vrsqrtsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4f 31[ 	 ]*vrsqrtsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4f 71 7f[ 	 ]*vrsqrtsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4f 72 80[ 	 ]*vrsqrtsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 2c f4[ 	 ]*vscalefph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 2c f4[ 	 ]*vscalefph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 2c f4[ 	 ]*vscalefph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 2c b4 f4 00 00 00 10[ 	 ]*vscalefph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 2c 31[ 	 ]*vscalefph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 2c 71 7f[ 	 ]*vscalefph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 2c 72 80[ 	 ]*vscalefph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2d f4[ 	 ]*vscalefsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 2d f4[ 	 ]*vscalefsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 2d f4[ 	 ]*vscalefsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 2d b4 f4 00 00 00 10[ 	 ]*vscalefsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2d 31[ 	 ]*vscalefsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2d 71 7f[ 	 ]*vscalefsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 2d 72 80[ 	 ]*vscalefsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 51 f5[ 	 ]*vsqrtph zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 51 f5[ 	 ]*vsqrtph zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 51 f5[ 	 ]*vsqrtph zmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 51 b4 f4 00 00 00 10[ 	 ]*vsqrtph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 51 31[ 	 ]*vsqrtph zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 51 71 7f[ 	 ]*vsqrtph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 51 72 80[ 	 ]*vsqrtph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 51 f4[ 	 ]*vsqrtsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 51 f4[ 	 ]*vsqrtsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 51 f4[ 	 ]*vsqrtsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 51 b4 f4 00 00 00 10[ 	 ]*vsqrtsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 51 31[ 	 ]*vsqrtsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 51 71 7f[ 	 ]*vsqrtsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 51 72 80[ 	 ]*vsqrtsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5c f4[ 	 ]*vsubph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5c f4[ 	 ]*vsubph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5c f4[ 	 ]*vsubph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5c b4 f4 00 00 00 10[ 	 ]*vsubph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5c 31[ 	 ]*vsubph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5c 71 7f[ 	 ]*vsubph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5c 72 80[ 	 ]*vsubph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5c f4[ 	 ]*vsubsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5c f4[ 	 ]*vsubsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5c f4[ 	 ]*vsubsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5c b4 f4 00 00 00 10[ 	 ]*vsubsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5c 31[ 	 ]*vsubsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5c 71 7f[ 	 ]*vsubsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5c 72 80[ 	 ]*vsubsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e f5[ 	 ]*vucomish xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 2e f5[ 	 ]*vucomish xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e b4 f4 00 00 00 10[ 	 ]*vucomish xmm6,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e 31[ 	 ]*vucomish xmm6,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e 71 7f[ 	 ]*vucomish xmm6,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e 72 80[ 	 ]*vucomish xmm6,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 58 f4[ 	 ]*vaddph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 58 f4[ 	 ]*vaddph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 58 f4[ 	 ]*vaddph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 58 b4 f4 00 00 00 10[ 	 ]*vaddph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 58 31[ 	 ]*vaddph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 58 71 7f[ 	 ]*vaddph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 58 72 80[ 	 ]*vaddph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 58 f4[ 	 ]*vaddsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 58 f4[ 	 ]*vaddsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 58 f4[ 	 ]*vaddsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 58 b4 f4 00 00 00 10[ 	 ]*vaddsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 58 31[ 	 ]*vaddsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 58 71 7f[ 	 ]*vaddsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 58 72 80[ 	 ]*vaddsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 48 c2 ec 7b[ 	 ]*vcmpph k5,zmm5,zmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 c2 ec 7b[ 	 ]*vcmpph k5,zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 1f c2 ec 7b[ 	 ]*vcmpph k5\{k7\},zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 4f c2 ac f4 00 00 00 10 7b[ 	 ]*vcmpph k5\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 58 c2 29 7b[ 	 ]*vcmpph k5,zmm5,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 48 c2 69 7f 7b[ 	 ]*vcmpph k5,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 5f c2 6a 80 7b[ 	 ]*vcmpph k5\{k7\},zmm5,WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 08 c2 ec 7b[ 	 ]*vcmpsh k5,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 18 c2 ec 7b[ 	 ]*vcmpsh k5,xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 1f c2 ec 7b[ 	 ]*vcmpsh k5\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 ac f4 00 00 00 10 7b[ 	 ]*vcmpsh k5\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 08 c2 29 7b[ 	 ]*vcmpsh k5,xmm5,WORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 08 c2 69 7f 7b[ 	 ]*vcmpsh k5,xmm5,WORD PTR \[ecx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 56 0f c2 6a 80 7b[ 	 ]*vcmpsh k5\{k7\},xmm5,WORD PTR \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f f5[ 	 ]*vcomish xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 2f f5[ 	 ]*vcomish xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f b4 f4 00 00 00 10[ 	 ]*vcomish xmm6,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f 31[ 	 ]*vcomish xmm6,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f 71 7f[ 	 ]*vcomish xmm6,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2f 72 80[ 	 ]*vcomish xmm6,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5b f5[ 	 ]*vcvtdq2ph ymm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5b f5[ 	 ]*vcvtdq2ph ymm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5b f5[ 	 ]*vcvtdq2ph ymm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 5b b4 f4 00 00 00 10[ 	 ]*vcvtdq2ph ymm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 5b 31[ 	 ]*vcvtdq2ph ymm6,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5b 71 7f[ 	 ]*vcvtdq2ph ymm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 5b 72 80[ 	 ]*vcvtdq2ph ymm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 48 5a f5[ 	 ]*vcvtpd2ph xmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 18 5a f5[ 	 ]*vcvtpd2ph xmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 9f 5a f5[ 	 ]*vcvtpd2ph xmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 4f 5a b4 f4 00 00 00 10[ 	 ]*vcvtpd2ph xmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 58 5a 31[ 	 ]*vcvtpd2ph xmm6,QWORD BCST \[ecx\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 48 5a 71 7f[ 	 ]*vcvtpd2ph xmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd df 5a 72 80[ 	 ]*vcvtpd2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 5b f5[ 	 ]*vcvtph2dq zmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 5b f5[ 	 ]*vcvtph2dq zmm6,ymm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 5b f5[ 	 ]*vcvtph2dq zmm6\{k7\}\{z\},ymm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 5b b4 f4 00 00 00 10[ 	 ]*vcvtph2dq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 5b 31[ 	 ]*vcvtph2dq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 5b 71 7f[ 	 ]*vcvtph2dq zmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 5b 72 80[ 	 ]*vcvtph2dq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5a f5[ 	 ]*vcvtph2pd zmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5a f5[ 	 ]*vcvtph2pd zmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5a f5[ 	 ]*vcvtph2pd zmm6\{k7\}\{z\},xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 5a b4 f4 00 00 00 10[ 	 ]*vcvtph2pd zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 5a 31[ 	 ]*vcvtph2pd zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 5a 71 7f[ 	 ]*vcvtph2pd zmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 5a 72 80[ 	 ]*vcvtph2pd zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 13 f5[ 	 ]*vcvtph2psx zmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 13 f5[ 	 ]*vcvtph2psx zmm6,ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 13 f5[ 	 ]*vcvtph2psx zmm6\{k7\}\{z\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 13 b4 f4 00 00 00 10[ 	 ]*vcvtph2psx zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 13 31[ 	 ]*vcvtph2psx zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 13 71 7f[ 	 ]*vcvtph2psx zmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 13 72 80[ 	 ]*vcvtph2psx zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7b f5[ 	 ]*vcvtph2qq zmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7b f5[ 	 ]*vcvtph2qq zmm6,xmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7b f5[ 	 ]*vcvtph2qq zmm6\{k7\}\{z\},xmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7b b4 f4 00 00 00 10[ 	 ]*vcvtph2qq zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7b 31[ 	 ]*vcvtph2qq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7b 71 7f[ 	 ]*vcvtph2qq zmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7b 72 80[ 	 ]*vcvtph2qq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 79 f5[ 	 ]*vcvtph2udq zmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 79 f5[ 	 ]*vcvtph2udq zmm6,ymm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 79 f5[ 	 ]*vcvtph2udq zmm6\{k7\}\{z\},ymm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 79 b4 f4 00 00 00 10[ 	 ]*vcvtph2udq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 79 31[ 	 ]*vcvtph2udq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 79 71 7f[ 	 ]*vcvtph2udq zmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 79 72 80[ 	 ]*vcvtph2udq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 79 f5[ 	 ]*vcvtph2uqq zmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 79 f5[ 	 ]*vcvtph2uqq zmm6,xmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 79 f5[ 	 ]*vcvtph2uqq zmm6\{k7\}\{z\},xmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 79 b4 f4 00 00 00 10[ 	 ]*vcvtph2uqq zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 79 31[ 	 ]*vcvtph2uqq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 79 71 7f[ 	 ]*vcvtph2uqq zmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 79 72 80[ 	 ]*vcvtph2uqq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7d f5[ 	 ]*vcvtph2uw zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7d f5[ 	 ]*vcvtph2uw zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7d f5[ 	 ]*vcvtph2uw zmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 7d b4 f4 00 00 00 10[ 	 ]*vcvtph2uw zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 7d 31[ 	 ]*vcvtph2uw zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7d 71 7f[ 	 ]*vcvtph2uw zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 7d 72 80[ 	 ]*vcvtph2uw zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7d f5[ 	 ]*vcvtph2w zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7d f5[ 	 ]*vcvtph2w zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7d f5[ 	 ]*vcvtph2w zmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7d b4 f4 00 00 00 10[ 	 ]*vcvtph2w zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7d 31[ 	 ]*vcvtph2w zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7d 71 7f[ 	 ]*vcvtph2w zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7d 72 80[ 	 ]*vcvtph2w zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 1d f5[ 	 ]*vcvtps2phx ymm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 1d f5[ 	 ]*vcvtps2phx ymm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 1d f5[ 	 ]*vcvtps2phx ymm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 1d b4 f4 00 00 00 10[ 	 ]*vcvtps2phx ymm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 1d 31[ 	 ]*vcvtps2phx ymm6,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 1d 71 7f[ 	 ]*vcvtps2phx ymm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 1d 72 80[ 	 ]*vcvtps2phx ymm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 48 5b f5[ 	 ]*vcvtqq2ph xmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 18 5b f5[ 	 ]*vcvtqq2ph xmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 9f 5b f5[ 	 ]*vcvtqq2ph xmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 4f 5b b4 f4 00 00 00 10[ 	 ]*vcvtqq2ph xmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 58 5b 31[ 	 ]*vcvtqq2ph xmm6,QWORD BCST \[ecx\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 48 5b 71 7f[ 	 ]*vcvtqq2ph xmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc df 5b 72 80[ 	 ]*vcvtqq2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 08 5a f4[ 	 ]*vcvtsd2sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 18 5a f4[ 	 ]*vcvtsd2sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 9f 5a f4[ 	 ]*vcvtsd2sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 0f 5a b4 f4 00 00 00 10[ 	 ]*vcvtsd2sh xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 08 5a 31[ 	 ]*vcvtsd2sh xmm6,xmm5,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 08 5a 71 7f[ 	 ]*vcvtsd2sh xmm6,xmm5,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 d7 8f 5a 72 80[ 	 ]*vcvtsd2sh xmm6\{k7\}\{z\},xmm5,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5a f4[ 	 ]*vcvtsh2sd xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5a f4[ 	 ]*vcvtsh2sd xmm6,xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5a f4[ 	 ]*vcvtsh2sd xmm6\{k7\}\{z\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5a b4 f4 00 00 00 10[ 	 ]*vcvtsh2sd xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5a 31[ 	 ]*vcvtsh2sd xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5a 71 7f[ 	 ]*vcvtsh2sd xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5a 72 80[ 	 ]*vcvtsh2sd xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d d6[ 	 ]*vcvtsh2si edx,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 2d d6[ 	 ]*vcvtsh2si edx,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 94 f4 00 00 00 10[ 	 ]*vcvtsh2si edx,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 11[ 	 ]*vcvtsh2si edx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 51 7f[ 	 ]*vcvtsh2si edx,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2d 52 80[ 	 ]*vcvtsh2si edx,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 08 13 f4[ 	 ]*vcvtsh2ss xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 18 13 f4[ 	 ]*vcvtsh2ss xmm6,xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 9f 13 f4[ 	 ]*vcvtsh2ss xmm6\{k7\}\{z\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 0f 13 b4 f4 00 00 00 10[ 	 ]*vcvtsh2ss xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 08 13 31[ 	 ]*vcvtsh2ss xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 08 13 71 7f[ 	 ]*vcvtsh2ss xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 54 8f 13 72 80[ 	 ]*vcvtsh2ss xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 d6[ 	 ]*vcvtsh2usi edx,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 79 d6[ 	 ]*vcvtsh2usi edx,xmm6\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 94 f4 00 00 00 10[ 	 ]*vcvtsh2usi edx,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 11[ 	 ]*vcvtsh2usi edx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 51 7f[ 	 ]*vcvtsh2usi edx,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 79 52 80[ 	 ]*vcvtsh2usi edx,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a f2[ 	 ]*vcvtsi2sh xmm6,xmm5,edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 2a f2[ 	 ]*vcvtsi2sh xmm6,xmm5,edx\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a b4 f4 00 00 00 10[ 	 ]*vcvtsi2sh xmm6,xmm5,DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a 31[ 	 ]*vcvtsi2sh xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a 71 7f[ 	 ]*vcvtsi2sh xmm6,xmm5,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 2a 72 80[ 	 ]*vcvtsi2sh xmm6,xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 1d f4[ 	 ]*vcvtss2sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 1d f4[ 	 ]*vcvtss2sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 1d f4[ 	 ]*vcvtss2sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 1d b4 f4 00 00 00 10[ 	 ]*vcvtss2sh xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 1d 31[ 	 ]*vcvtss2sh xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 1d 71 7f[ 	 ]*vcvtss2sh xmm6,xmm5,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 1d 72 80[ 	 ]*vcvtss2sh xmm6\{k7\}\{z\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 5b f5[ 	 ]*vcvttph2dq zmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 5b f5[ 	 ]*vcvttph2dq zmm6,ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 5b f5[ 	 ]*vcvttph2dq zmm6\{k7\}\{z\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 4f 5b b4 f4 00 00 00 10[ 	 ]*vcvttph2dq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 58 5b 31[ 	 ]*vcvttph2dq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 5b 71 7f[ 	 ]*vcvttph2dq zmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e df 5b 72 80[ 	 ]*vcvttph2dq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7a f5[ 	 ]*vcvttph2qq zmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7a f5[ 	 ]*vcvttph2qq zmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7a f5[ 	 ]*vcvttph2qq zmm6\{k7\}\{z\},xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7a b4 f4 00 00 00 10[ 	 ]*vcvttph2qq zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7a 31[ 	 ]*vcvttph2qq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7a 71 7f[ 	 ]*vcvttph2qq zmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7a 72 80[ 	 ]*vcvttph2qq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 78 f5[ 	 ]*vcvttph2udq zmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 78 f5[ 	 ]*vcvttph2udq zmm6,ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 78 f5[ 	 ]*vcvttph2udq zmm6\{k7\}\{z\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 78 b4 f4 00 00 00 10[ 	 ]*vcvttph2udq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 78 31[ 	 ]*vcvttph2udq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 78 71 7f[ 	 ]*vcvttph2udq zmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 78 72 80[ 	 ]*vcvttph2udq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 78 f5[ 	 ]*vcvttph2uqq zmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 78 f5[ 	 ]*vcvttph2uqq zmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 78 f5[ 	 ]*vcvttph2uqq zmm6\{k7\}\{z\},xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 78 b4 f4 00 00 00 10[ 	 ]*vcvttph2uqq zmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 78 31[ 	 ]*vcvttph2uqq zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 78 71 7f[ 	 ]*vcvttph2uqq zmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 78 72 80[ 	 ]*vcvttph2uqq zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7c f5[ 	 ]*vcvttph2uw zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7c f5[ 	 ]*vcvttph2uw zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7c f5[ 	 ]*vcvttph2uw zmm6\{k7\}\{z\},zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 7c b4 f4 00 00 00 10[ 	 ]*vcvttph2uw zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 7c 31[ 	 ]*vcvttph2uw zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 7c 71 7f[ 	 ]*vcvttph2uw zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 7c 72 80[ 	 ]*vcvttph2uw zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7c f5[ 	 ]*vcvttph2w zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7c f5[ 	 ]*vcvttph2w zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7c f5[ 	 ]*vcvttph2w zmm6\{k7\}\{z\},zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 4f 7c b4 f4 00 00 00 10[ 	 ]*vcvttph2w zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 58 7c 31[ 	 ]*vcvttph2w zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 48 7c 71 7f[ 	 ]*vcvttph2w zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d df 7c 72 80[ 	 ]*vcvttph2w zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c d6[ 	 ]*vcvttsh2si edx,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 2c d6[ 	 ]*vcvttsh2si edx,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 94 f4 00 00 00 10[ 	 ]*vcvttsh2si edx,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 11[ 	 ]*vcvttsh2si edx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 51 7f[ 	 ]*vcvttsh2si edx,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 2c 52 80[ 	 ]*vcvttsh2si edx,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 d6[ 	 ]*vcvttsh2usi edx,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 78 d6[ 	 ]*vcvttsh2usi edx,xmm6\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 94 f4 00 00 00 10[ 	 ]*vcvttsh2usi edx,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 11[ 	 ]*vcvttsh2usi edx,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 51 7f[ 	 ]*vcvttsh2usi edx,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 78 52 80[ 	 ]*vcvttsh2usi edx,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7a f5[ 	 ]*vcvtudq2ph ymm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7a f5[ 	 ]*vcvtudq2ph ymm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7a f5[ 	 ]*vcvtudq2ph ymm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 4f 7a b4 f4 00 00 00 10[ 	 ]*vcvtudq2ph ymm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 58 7a 31[ 	 ]*vcvtudq2ph ymm6,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7a 71 7f[ 	 ]*vcvtudq2ph ymm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f df 7a 72 80[ 	 ]*vcvtudq2ph ymm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 48 7a f5[ 	 ]*vcvtuqq2ph xmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 18 7a f5[ 	 ]*vcvtuqq2ph xmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 9f 7a f5[ 	 ]*vcvtuqq2ph xmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 4f 7a b4 f4 00 00 00 10[ 	 ]*vcvtuqq2ph xmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 58 7a 31[ 	 ]*vcvtuqq2ph xmm6,QWORD BCST \[ecx\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 48 7a 71 7f[ 	 ]*vcvtuqq2ph xmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff df 7a 72 80[ 	 ]*vcvtuqq2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b f2[ 	 ]*vcvtusi2sh xmm6,xmm5,edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 7b f2[ 	 ]*vcvtusi2sh xmm6,xmm5,edx\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b b4 f4 00 00 00 10[ 	 ]*vcvtusi2sh xmm6,xmm5,DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b 31[ 	 ]*vcvtusi2sh xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b 71 7f[ 	 ]*vcvtusi2sh xmm6,xmm5,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 7b 72 80[ 	 ]*vcvtusi2sh xmm6,xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7d f5[ 	 ]*vcvtuw2ph zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7d f5[ 	 ]*vcvtuw2ph zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7d f5[ 	 ]*vcvtuw2ph zmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 4f 7d b4 f4 00 00 00 10[ 	 ]*vcvtuw2ph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 58 7d 31[ 	 ]*vcvtuw2ph zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 48 7d 71 7f[ 	 ]*vcvtuw2ph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f df 7d 72 80[ 	 ]*vcvtuw2ph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 7d f5[ 	 ]*vcvtw2ph zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 7d f5[ 	 ]*vcvtw2ph zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 7d f5[ 	 ]*vcvtw2ph zmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 4f 7d b4 f4 00 00 00 10[ 	 ]*vcvtw2ph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 58 7d 31[ 	 ]*vcvtw2ph zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 48 7d 71 7f[ 	 ]*vcvtw2ph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e df 7d 72 80[ 	 ]*vcvtw2ph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5e f4[ 	 ]*vdivph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5e f4[ 	 ]*vdivph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5e f4[ 	 ]*vdivph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5e b4 f4 00 00 00 10[ 	 ]*vdivph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5e 31[ 	 ]*vdivph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5e 71 7f[ 	 ]*vdivph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5e 72 80[ 	 ]*vdivph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5e f4[ 	 ]*vdivsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5e f4[ 	 ]*vdivsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5e f4[ 	 ]*vdivsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5e b4 f4 00 00 00 10[ 	 ]*vdivsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5e 31[ 	 ]*vdivsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5e 71 7f[ 	 ]*vdivsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5e 72 80[ 	 ]*vdivsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 56 f4[ 	 ]*vfcmaddcph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 56 f4[ 	 ]*vfcmaddcph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f 56 f4[ 	 ]*vfcmaddcph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 4f 56 b4 f4 00 00 00 10[ 	 ]*vfcmaddcph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 58 56 31[ 	 ]*vfcmaddcph zmm6,zmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 56 71 7f[ 	 ]*vfcmaddcph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 df 56 72 80[ 	 ]*vfcmaddcph zmm6\{k7\}\{z\},zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 57 f4[ 	 ]*vfcmaddcsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 57 f4[ 	 ]*vfcmaddcsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f 57 f4[ 	 ]*vfcmaddcsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f 57 b4 f4 00 00 00 10[ 	 ]*vfcmaddcsh xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 57 31[ 	 ]*vfcmaddcsh xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 57 71 7f[ 	 ]*vfcmaddcsh xmm6,xmm5,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f 57 72 80[ 	 ]*vfcmaddcsh xmm6\{k7\}\{z\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 d6 f4[ 	 ]*vfcmulcph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 d6 f4[ 	 ]*vfcmulcph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f d6 f4[ 	 ]*vfcmulcph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 4f d6 b4 f4 00 00 00 10[ 	 ]*vfcmulcph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 58 d6 31[ 	 ]*vfcmulcph zmm6,zmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 48 d6 71 7f[ 	 ]*vfcmulcph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 df d6 72 80[ 	 ]*vfcmulcph zmm6\{k7\}\{z\},zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d7 f4[ 	 ]*vfcmulcsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 d7 f4[ 	 ]*vfcmulcsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f d7 f4[ 	 ]*vfcmulcsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f d7 b4 f4 00 00 00 10[ 	 ]*vfcmulcsh xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d7 31[ 	 ]*vfcmulcsh xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d7 71 7f[ 	 ]*vfcmulcsh xmm6,xmm5,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f d7 72 80[ 	 ]*vfcmulcsh xmm6\{k7\}\{z\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 98 f4[ 	 ]*vfmadd132ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 98 f4[ 	 ]*vfmadd132ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 98 f4[ 	 ]*vfmadd132ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 98 b4 f4 00 00 00 10[ 	 ]*vfmadd132ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 98 31[ 	 ]*vfmadd132ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 98 71 7f[ 	 ]*vfmadd132ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 98 72 80[ 	 ]*vfmadd132ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 99 f4[ 	 ]*vfmadd132sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 99 f4[ 	 ]*vfmadd132sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 99 f4[ 	 ]*vfmadd132sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 99 b4 f4 00 00 00 10[ 	 ]*vfmadd132sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 99 31[ 	 ]*vfmadd132sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 99 71 7f[ 	 ]*vfmadd132sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 99 72 80[ 	 ]*vfmadd132sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a8 f4[ 	 ]*vfmadd213ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a8 f4[ 	 ]*vfmadd213ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a8 f4[ 	 ]*vfmadd213ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f a8 b4 f4 00 00 00 10[ 	 ]*vfmadd213ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 a8 31[ 	 ]*vfmadd213ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a8 71 7f[ 	 ]*vfmadd213ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df a8 72 80[ 	 ]*vfmadd213ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a9 f4[ 	 ]*vfmadd213sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a9 f4[ 	 ]*vfmadd213sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a9 f4[ 	 ]*vfmadd213sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a9 b4 f4 00 00 00 10[ 	 ]*vfmadd213sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a9 31[ 	 ]*vfmadd213sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a9 71 7f[ 	 ]*vfmadd213sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a9 72 80[ 	 ]*vfmadd213sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b8 f4[ 	 ]*vfmadd231ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b8 f4[ 	 ]*vfmadd231ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b8 f4[ 	 ]*vfmadd231ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f b8 b4 f4 00 00 00 10[ 	 ]*vfmadd231ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 b8 31[ 	 ]*vfmadd231ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b8 71 7f[ 	 ]*vfmadd231ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df b8 72 80[ 	 ]*vfmadd231ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b9 f4[ 	 ]*vfmadd231sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b9 f4[ 	 ]*vfmadd231sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b9 f4[ 	 ]*vfmadd231sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b9 b4 f4 00 00 00 10[ 	 ]*vfmadd231sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b9 31[ 	 ]*vfmadd231sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b9 71 7f[ 	 ]*vfmadd231sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b9 72 80[ 	 ]*vfmadd231sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 56 f4[ 	 ]*vfmaddcph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 56 f4[ 	 ]*vfmaddcph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f 56 f4[ 	 ]*vfmaddcph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 4f 56 b4 f4 00 00 00 10[ 	 ]*vfmaddcph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 58 56 31[ 	 ]*vfmaddcph zmm6,zmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 56 71 7f[ 	 ]*vfmaddcph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 df 56 72 80[ 	 ]*vfmaddcph zmm6\{k7\}\{z\},zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 57 f4[ 	 ]*vfmaddcsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 57 f4[ 	 ]*vfmaddcsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f 57 f4[ 	 ]*vfmaddcsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f 57 b4 f4 00 00 00 10[ 	 ]*vfmaddcsh xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 57 31[ 	 ]*vfmaddcsh xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 57 71 7f[ 	 ]*vfmaddcsh xmm6,xmm5,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f 57 72 80[ 	 ]*vfmaddcsh xmm6\{k7\}\{z\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 96 f4[ 	 ]*vfmaddsub132ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 96 f4[ 	 ]*vfmaddsub132ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 96 f4[ 	 ]*vfmaddsub132ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 96 b4 f4 00 00 00 10[ 	 ]*vfmaddsub132ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 96 31[ 	 ]*vfmaddsub132ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 96 71 7f[ 	 ]*vfmaddsub132ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 96 72 80[ 	 ]*vfmaddsub132ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a6 f4[ 	 ]*vfmaddsub213ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a6 f4[ 	 ]*vfmaddsub213ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a6 f4[ 	 ]*vfmaddsub213ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f a6 b4 f4 00 00 00 10[ 	 ]*vfmaddsub213ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 a6 31[ 	 ]*vfmaddsub213ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a6 71 7f[ 	 ]*vfmaddsub213ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df a6 72 80[ 	 ]*vfmaddsub213ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b6 f4[ 	 ]*vfmaddsub231ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b6 f4[ 	 ]*vfmaddsub231ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b6 f4[ 	 ]*vfmaddsub231ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f b6 b4 f4 00 00 00 10[ 	 ]*vfmaddsub231ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 b6 31[ 	 ]*vfmaddsub231ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b6 71 7f[ 	 ]*vfmaddsub231ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df b6 72 80[ 	 ]*vfmaddsub231ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9a f4[ 	 ]*vfmsub132ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9a f4[ 	 ]*vfmsub132ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9a f4[ 	 ]*vfmsub132ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 9a b4 f4 00 00 00 10[ 	 ]*vfmsub132ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 9a 31[ 	 ]*vfmsub132ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9a 71 7f[ 	 ]*vfmsub132ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 9a 72 80[ 	 ]*vfmsub132ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9b f4[ 	 ]*vfmsub132sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9b f4[ 	 ]*vfmsub132sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9b f4[ 	 ]*vfmsub132sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9b b4 f4 00 00 00 10[ 	 ]*vfmsub132sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9b 31[ 	 ]*vfmsub132sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9b 71 7f[ 	 ]*vfmsub132sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9b 72 80[ 	 ]*vfmsub132sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 aa f4[ 	 ]*vfmsub213ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 aa f4[ 	 ]*vfmsub213ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f aa f4[ 	 ]*vfmsub213ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f aa b4 f4 00 00 00 10[ 	 ]*vfmsub213ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 aa 31[ 	 ]*vfmsub213ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 aa 71 7f[ 	 ]*vfmsub213ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df aa 72 80[ 	 ]*vfmsub213ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ab f4[ 	 ]*vfmsub213sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ab f4[ 	 ]*vfmsub213sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ab f4[ 	 ]*vfmsub213sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ab b4 f4 00 00 00 10[ 	 ]*vfmsub213sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ab 31[ 	 ]*vfmsub213sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ab 71 7f[ 	 ]*vfmsub213sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ab 72 80[ 	 ]*vfmsub213sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ba f4[ 	 ]*vfmsub231ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ba f4[ 	 ]*vfmsub231ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ba f4[ 	 ]*vfmsub231ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f ba b4 f4 00 00 00 10[ 	 ]*vfmsub231ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 ba 31[ 	 ]*vfmsub231ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ba 71 7f[ 	 ]*vfmsub231ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df ba 72 80[ 	 ]*vfmsub231ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bb f4[ 	 ]*vfmsub231sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bb f4[ 	 ]*vfmsub231sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bb f4[ 	 ]*vfmsub231sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bb b4 f4 00 00 00 10[ 	 ]*vfmsub231sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bb 31[ 	 ]*vfmsub231sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bb 71 7f[ 	 ]*vfmsub231sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bb 72 80[ 	 ]*vfmsub231sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 97 f4[ 	 ]*vfmsubadd132ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 97 f4[ 	 ]*vfmsubadd132ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 97 f4[ 	 ]*vfmsubadd132ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 97 b4 f4 00 00 00 10[ 	 ]*vfmsubadd132ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 97 31[ 	 ]*vfmsubadd132ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 97 71 7f[ 	 ]*vfmsubadd132ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 97 72 80[ 	 ]*vfmsubadd132ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a7 f4[ 	 ]*vfmsubadd213ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a7 f4[ 	 ]*vfmsubadd213ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a7 f4[ 	 ]*vfmsubadd213ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f a7 b4 f4 00 00 00 10[ 	 ]*vfmsubadd213ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 a7 31[ 	 ]*vfmsubadd213ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 a7 71 7f[ 	 ]*vfmsubadd213ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df a7 72 80[ 	 ]*vfmsubadd213ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b7 f4[ 	 ]*vfmsubadd231ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b7 f4[ 	 ]*vfmsubadd231ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b7 f4[ 	 ]*vfmsubadd231ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f b7 b4 f4 00 00 00 10[ 	 ]*vfmsubadd231ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 b7 31[ 	 ]*vfmsubadd231ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 b7 71 7f[ 	 ]*vfmsubadd231ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df b7 72 80[ 	 ]*vfmsubadd231ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 d6 f4[ 	 ]*vfmulcph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 d6 f4[ 	 ]*vfmulcph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f d6 f4[ 	 ]*vfmulcph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 4f d6 b4 f4 00 00 00 10[ 	 ]*vfmulcph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 58 d6 31[ 	 ]*vfmulcph zmm6,zmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 48 d6 71 7f[ 	 ]*vfmulcph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 df d6 72 80[ 	 ]*vfmulcph zmm6\{k7\}\{z\},zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d7 f4[ 	 ]*vfmulcsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 d7 f4[ 	 ]*vfmulcsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f d7 f4[ 	 ]*vfmulcsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f d7 b4 f4 00 00 00 10[ 	 ]*vfmulcsh xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d7 31[ 	 ]*vfmulcsh xmm6,xmm5,DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d7 71 7f[ 	 ]*vfmulcsh xmm6,xmm5,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f d7 72 80[ 	 ]*vfmulcsh xmm6\{k7\}\{z\},xmm5,DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9c f4[ 	 ]*vfnmadd132ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9c f4[ 	 ]*vfnmadd132ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9c f4[ 	 ]*vfnmadd132ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 9c b4 f4 00 00 00 10[ 	 ]*vfnmadd132ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 9c 31[ 	 ]*vfnmadd132ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9c 71 7f[ 	 ]*vfnmadd132ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 9c 72 80[ 	 ]*vfnmadd132ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9d f4[ 	 ]*vfnmadd132sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9d f4[ 	 ]*vfnmadd132sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9d f4[ 	 ]*vfnmadd132sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9d b4 f4 00 00 00 10[ 	 ]*vfnmadd132sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9d 31[ 	 ]*vfnmadd132sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9d 71 7f[ 	 ]*vfnmadd132sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9d 72 80[ 	 ]*vfnmadd132sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ac f4[ 	 ]*vfnmadd213ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ac f4[ 	 ]*vfnmadd213ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ac f4[ 	 ]*vfnmadd213ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f ac b4 f4 00 00 00 10[ 	 ]*vfnmadd213ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 ac 31[ 	 ]*vfnmadd213ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ac 71 7f[ 	 ]*vfnmadd213ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df ac 72 80[ 	 ]*vfnmadd213ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ad f4[ 	 ]*vfnmadd213sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ad f4[ 	 ]*vfnmadd213sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ad f4[ 	 ]*vfnmadd213sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ad b4 f4 00 00 00 10[ 	 ]*vfnmadd213sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ad 31[ 	 ]*vfnmadd213sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ad 71 7f[ 	 ]*vfnmadd213sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ad 72 80[ 	 ]*vfnmadd213sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 bc f4[ 	 ]*vfnmadd231ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bc f4[ 	 ]*vfnmadd231ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bc f4[ 	 ]*vfnmadd231ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f bc b4 f4 00 00 00 10[ 	 ]*vfnmadd231ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 bc 31[ 	 ]*vfnmadd231ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 bc 71 7f[ 	 ]*vfnmadd231ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df bc 72 80[ 	 ]*vfnmadd231ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bd f4[ 	 ]*vfnmadd231sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bd f4[ 	 ]*vfnmadd231sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bd f4[ 	 ]*vfnmadd231sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bd b4 f4 00 00 00 10[ 	 ]*vfnmadd231sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bd 31[ 	 ]*vfnmadd231sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bd 71 7f[ 	 ]*vfnmadd231sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bd 72 80[ 	 ]*vfnmadd231sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9e f4[ 	 ]*vfnmsub132ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9e f4[ 	 ]*vfnmsub132ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9e f4[ 	 ]*vfnmsub132ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 9e b4 f4 00 00 00 10[ 	 ]*vfnmsub132ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 9e 31[ 	 ]*vfnmsub132ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 9e 71 7f[ 	 ]*vfnmsub132ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 9e 72 80[ 	 ]*vfnmsub132ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9f f4[ 	 ]*vfnmsub132sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9f f4[ 	 ]*vfnmsub132sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9f f4[ 	 ]*vfnmsub132sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9f b4 f4 00 00 00 10[ 	 ]*vfnmsub132sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9f 31[ 	 ]*vfnmsub132sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9f 71 7f[ 	 ]*vfnmsub132sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9f 72 80[ 	 ]*vfnmsub132sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ae f4[ 	 ]*vfnmsub213ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ae f4[ 	 ]*vfnmsub213ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ae f4[ 	 ]*vfnmsub213ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f ae b4 f4 00 00 00 10[ 	 ]*vfnmsub213ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 ae 31[ 	 ]*vfnmsub213ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 ae 71 7f[ 	 ]*vfnmsub213ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df ae 72 80[ 	 ]*vfnmsub213ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 af f4[ 	 ]*vfnmsub213sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 af f4[ 	 ]*vfnmsub213sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f af f4[ 	 ]*vfnmsub213sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f af b4 f4 00 00 00 10[ 	 ]*vfnmsub213sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 af 31[ 	 ]*vfnmsub213sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 af 71 7f[ 	 ]*vfnmsub213sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f af 72 80[ 	 ]*vfnmsub213sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 be f4[ 	 ]*vfnmsub231ph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 be f4[ 	 ]*vfnmsub231ph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f be f4[ 	 ]*vfnmsub231ph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f be b4 f4 00 00 00 10[ 	 ]*vfnmsub231ph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 be 31[ 	 ]*vfnmsub231ph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 be 71 7f[ 	 ]*vfnmsub231ph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df be 72 80[ 	 ]*vfnmsub231ph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bf f4[ 	 ]*vfnmsub231sh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bf f4[ 	 ]*vfnmsub231sh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bf f4[ 	 ]*vfnmsub231sh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bf b4 f4 00 00 00 10[ 	 ]*vfnmsub231sh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bf 31[ 	 ]*vfnmsub231sh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bf 71 7f[ 	 ]*vfnmsub231sh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bf 72 80[ 	 ]*vfnmsub231sh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 66 ee 7b[ 	 ]*vfpclassph k5,zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 66 ee 7b[ 	 ]*vfpclassph k5\{k7\},zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 66 ac f4 00 00 00 10 7b[ 	 ]*vfpclassph k5\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 66 29 7b[ 	 ]*vfpclassph k5,WORD BCST \[ecx\]\{1to32\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 66 69 7f 7b[ 	 ]*vfpclassph k5,ZMMWORD PTR \[ecx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 5f 66 6a 80 7b[ 	 ]*vfpclassph k5\{k7\},WORD BCST \[edx-0x100\]\{1to32\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 67 ee 7b[ 	 ]*vfpclasssh k5,xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 67 ee 7b[ 	 ]*vfpclasssh k5\{k7\},xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 67 ac f4 00 00 00 10 7b[ 	 ]*vfpclasssh k5\{k7\},WORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 67 29 7b[ 	 ]*vfpclasssh k5,WORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 67 69 7f 7b[ 	 ]*vfpclasssh k5,WORD PTR \[ecx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 67 6a 80 7b[ 	 ]*vfpclasssh k5\{k7\},WORD PTR \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 42 f5[ 	 ]*vgetexpph zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 42 f5[ 	 ]*vgetexpph zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 42 f5[ 	 ]*vgetexpph zmm6\{k7\}\{z\},zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 42 b4 f4 00 00 00 10[ 	 ]*vgetexpph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 42 31[ 	 ]*vgetexpph zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 42 71 7f[ 	 ]*vgetexpph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 42 72 80[ 	 ]*vgetexpph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 43 f4[ 	 ]*vgetexpsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 43 f4[ 	 ]*vgetexpsh xmm6,xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 43 f4[ 	 ]*vgetexpsh xmm6\{k7\}\{z\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 43 b4 f4 00 00 00 10[ 	 ]*vgetexpsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 43 31[ 	 ]*vgetexpsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 43 71 7f[ 	 ]*vgetexpsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 43 72 80[ 	 ]*vgetexpsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 26 f5 7b[ 	 ]*vgetmantph zmm6,zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 26 f5 7b[ 	 ]*vgetmantph zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 26 f5 7b[ 	 ]*vgetmantph zmm6\{k7\}\{z\},zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 26 b4 f4 00 00 00 10 7b[ 	 ]*vgetmantph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 26 31 7b[ 	 ]*vgetmantph zmm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 26 71 7f 7b[ 	 ]*vgetmantph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c df 26 72 80 7b[ 	 ]*vgetmantph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 27 f4 7b[ 	 ]*vgetmantsh xmm6,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 27 f4 7b[ 	 ]*vgetmantsh xmm6,xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 9f 27 f4 7b[ 	 ]*vgetmantsh xmm6\{k7\}\{z\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f 27 b4 f4 00 00 00 10 7b[ 	 ]*vgetmantsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 27 31 7b[ 	 ]*vgetmantsh xmm6,xmm5,WORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 27 71 7f 7b[ 	 ]*vgetmantsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 8f 27 72 80 7b[ 	 ]*vgetmantsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5f f4[ 	 ]*vmaxph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5f f4[ 	 ]*vmaxph zmm6,zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5f f4[ 	 ]*vmaxph zmm6\{k7\}\{z\},zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5f b4 f4 00 00 00 10[ 	 ]*vmaxph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5f 31[ 	 ]*vmaxph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5f 71 7f[ 	 ]*vmaxph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5f 72 80[ 	 ]*vmaxph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5f f4[ 	 ]*vmaxsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5f f4[ 	 ]*vmaxsh xmm6,xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5f f4[ 	 ]*vmaxsh xmm6\{k7\}\{z\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5f b4 f4 00 00 00 10[ 	 ]*vmaxsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5f 31[ 	 ]*vmaxsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5f 71 7f[ 	 ]*vmaxsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5f 72 80[ 	 ]*vmaxsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5d f4[ 	 ]*vminph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5d f4[ 	 ]*vminph zmm6,zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5d f4[ 	 ]*vminph zmm6\{k7\}\{z\},zmm5,zmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5d b4 f4 00 00 00 10[ 	 ]*vminph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5d 31[ 	 ]*vminph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5d 71 7f[ 	 ]*vminph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5d 72 80[ 	 ]*vminph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5d f4[ 	 ]*vminsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5d f4[ 	 ]*vminsh xmm6,xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5d f4[ 	 ]*vminsh xmm6\{k7\}\{z\},xmm5,xmm4\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5d b4 f4 00 00 00 10[ 	 ]*vminsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5d 31[ 	 ]*vminsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5d 71 7f[ 	 ]*vminsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5d 72 80[ 	 ]*vminsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 10 f4[ 	 ]*vmovsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 10 f4[ 	 ]*vmovsh xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 10 b4 f4 00 00 00 10[ 	 ]*vmovsh xmm6\{k7\},WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 10 31[ 	 ]*vmovsh xmm6,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 10 71 7f[ 	 ]*vmovsh xmm6,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 8f 10 72 80[ 	 ]*vmovsh xmm6\{k7\}\{z\},WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 11 b4 f4 00 00 00 10[ 	 ]*vmovsh WORD PTR \[esp\+esi\*8\+0x10000000\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 11 31[ 	 ]*vmovsh WORD PTR \[ecx\],xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 11 71 7f[ 	 ]*vmovsh WORD PTR \[ecx\+0xfe\],xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 11 72 80[ 	 ]*vmovsh WORD PTR \[edx-0x100\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e f2[ 	 ]*vmovw  xmm6,edx
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e f2[ 	 ]*vmovw  edx,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e b4 f4 00 00 00 10[ 	 ]*vmovw  xmm6,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e 31[ 	 ]*vmovw  xmm6,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e 71 7f[ 	 ]*vmovw  xmm6,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 6e 72 80[ 	 ]*vmovw  xmm6,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e b4 f4 00 00 00 10[ 	 ]*vmovw  WORD PTR \[esp\+esi\*8\+0x10000000\],xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e 31[ 	 ]*vmovw  WORD PTR \[ecx\],xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e 71 7f[ 	 ]*vmovw  WORD PTR \[ecx\+0xfe\],xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7e 72 80[ 	 ]*vmovw  WORD PTR \[edx-0x100\],xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 59 f4[ 	 ]*vmulph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 59 f4[ 	 ]*vmulph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 59 f4[ 	 ]*vmulph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 59 b4 f4 00 00 00 10[ 	 ]*vmulph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 59 31[ 	 ]*vmulph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 59 71 7f[ 	 ]*vmulph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 59 72 80[ 	 ]*vmulph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 59 f4[ 	 ]*vmulsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 59 f4[ 	 ]*vmulsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 59 f4[ 	 ]*vmulsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 59 b4 f4 00 00 00 10[ 	 ]*vmulsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 59 31[ 	 ]*vmulsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 59 71 7f[ 	 ]*vmulsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 59 72 80[ 	 ]*vmulsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4c f5[ 	 ]*vrcpph zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d cf 4c f5[ 	 ]*vrcpph zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 4c b4 f4 00 00 00 10[ 	 ]*vrcpph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 4c 31[ 	 ]*vrcpph zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4c 71 7f[ 	 ]*vrcpph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 4c 72 80[ 	 ]*vrcpph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4d f4[ 	 ]*vrcpsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4d f4[ 	 ]*vrcpsh xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 4d b4 f4 00 00 00 10[ 	 ]*vrcpsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4d 31[ 	 ]*vrcpsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4d 71 7f[ 	 ]*vrcpsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4d 72 80[ 	 ]*vrcpsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 56 f5 7b[ 	 ]*vreduceph zmm6,zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 56 f5 7b[ 	 ]*vreduceph zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 56 f5 7b[ 	 ]*vreduceph zmm6\{k7\}\{z\},zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 56 b4 f4 00 00 00 10 7b[ 	 ]*vreduceph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 56 31 7b[ 	 ]*vreduceph zmm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 56 71 7f 7b[ 	 ]*vreduceph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c df 56 72 80 7b[ 	 ]*vreduceph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 57 f4 7b[ 	 ]*vreducesh xmm6,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 57 f4 7b[ 	 ]*vreducesh xmm6,xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 9f 57 f4 7b[ 	 ]*vreducesh xmm6\{k7\}\{z\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f 57 b4 f4 00 00 00 10 7b[ 	 ]*vreducesh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 57 31 7b[ 	 ]*vreducesh xmm6,xmm5,WORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 57 71 7f 7b[ 	 ]*vreducesh xmm6,xmm5,WORD PTR \[ecx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 8f 57 72 80 7b[ 	 ]*vreducesh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 08 f5 7b[ 	 ]*vrndscaleph zmm6,zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 08 f5 7b[ 	 ]*vrndscaleph zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 08 f5 7b[ 	 ]*vrndscaleph zmm6\{k7\}\{z\},zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 4f 08 b4 f4 00 00 00 10 7b[ 	 ]*vrndscaleph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 58 08 31 7b[ 	 ]*vrndscaleph zmm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 48 08 71 7f 7b[ 	 ]*vrndscaleph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c df 08 72 80 7b[ 	 ]*vrndscaleph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 0a f4 7b[ 	 ]*vrndscalesh xmm6,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 0a f4 7b[ 	 ]*vrndscalesh xmm6,xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 9f 0a f4 7b[ 	 ]*vrndscalesh xmm6\{k7\}\{z\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f 0a b4 f4 00 00 00 10 7b[ 	 ]*vrndscalesh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 0a 31 7b[ 	 ]*vrndscalesh xmm6,xmm5,WORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 0a 71 7f 7b[ 	 ]*vrndscalesh xmm6,xmm5,WORD PTR \[ecx\+0xfe\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 8f 0a 72 80 7b[ 	 ]*vrndscalesh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4e f5[ 	 ]*vrsqrtph zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d cf 4e f5[ 	 ]*vrsqrtph zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 4f 4e b4 f4 00 00 00 10[ 	 ]*vrsqrtph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 58 4e 31[ 	 ]*vrsqrtph zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 48 4e 71 7f[ 	 ]*vrsqrtph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d df 4e 72 80[ 	 ]*vrsqrtph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4f f4[ 	 ]*vrsqrtsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4f f4[ 	 ]*vrsqrtsh xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 4f b4 f4 00 00 00 10[ 	 ]*vrsqrtsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4f 31[ 	 ]*vrsqrtsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 4f 71 7f[ 	 ]*vrsqrtsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 4f 72 80[ 	 ]*vrsqrtsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 2c f4[ 	 ]*vscalefph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 2c f4[ 	 ]*vscalefph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 2c f4[ 	 ]*vscalefph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 4f 2c b4 f4 00 00 00 10[ 	 ]*vscalefph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 58 2c 31[ 	 ]*vscalefph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 48 2c 71 7f[ 	 ]*vscalefph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 df 2c 72 80[ 	 ]*vscalefph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2d f4[ 	 ]*vscalefsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 2d f4[ 	 ]*vscalefsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 2d f4[ 	 ]*vscalefsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 2d b4 f4 00 00 00 10[ 	 ]*vscalefsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2d 31[ 	 ]*vscalefsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2d 71 7f[ 	 ]*vscalefsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 2d 72 80[ 	 ]*vscalefsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 51 f5[ 	 ]*vsqrtph zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 51 f5[ 	 ]*vsqrtph zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 51 f5[ 	 ]*vsqrtph zmm6\{k7\}\{z\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 4f 51 b4 f4 00 00 00 10[ 	 ]*vsqrtph zmm6\{k7\},ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 58 51 31[ 	 ]*vsqrtph zmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 48 51 71 7f[ 	 ]*vsqrtph zmm6,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c df 51 72 80[ 	 ]*vsqrtph zmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 51 f4[ 	 ]*vsqrtsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 51 f4[ 	 ]*vsqrtsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 51 f4[ 	 ]*vsqrtsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 51 b4 f4 00 00 00 10[ 	 ]*vsqrtsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 51 31[ 	 ]*vsqrtsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 51 71 7f[ 	 ]*vsqrtsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 51 72 80[ 	 ]*vsqrtsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5c f4[ 	 ]*vsubph zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5c f4[ 	 ]*vsubph zmm6,zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5c f4[ 	 ]*vsubph zmm6\{k7\}\{z\},zmm5,zmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 4f 5c b4 f4 00 00 00 10[ 	 ]*vsubph zmm6\{k7\},zmm5,ZMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 58 5c 31[ 	 ]*vsubph zmm6,zmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 48 5c 71 7f[ 	 ]*vsubph zmm6,zmm5,ZMMWORD PTR \[ecx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 df 5c 72 80[ 	 ]*vsubph zmm6\{k7\}\{z\},zmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5c f4[ 	 ]*vsubsh xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 18 5c f4[ 	 ]*vsubsh xmm6,xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 9f 5c f4[ 	 ]*vsubsh xmm6\{k7\}\{z\},xmm5,xmm4\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 0f 5c b4 f4 00 00 00 10[ 	 ]*vsubsh xmm6\{k7\},xmm5,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5c 31[ 	 ]*vsubsh xmm6,xmm5,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 08 5c 71 7f[ 	 ]*vsubsh xmm6,xmm5,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 56 8f 5c 72 80[ 	 ]*vsubsh xmm6\{k7\}\{z\},xmm5,WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e f5[ 	 ]*vucomish xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 2e f5[ 	 ]*vucomish xmm6,xmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e b4 f4 00 00 00 10[ 	 ]*vucomish xmm6,WORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e 31[ 	 ]*vucomish xmm6,WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e 71 7f[ 	 ]*vucomish xmm6,WORD PTR \[ecx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 2e 72 80[ 	 ]*vucomish xmm6,WORD PTR \[edx-0x100\]
#pass
