#as:
#objdump: -dw -Mintel
#name: i386 AVX512-FP16,AVX512VL insns (Intel disassembly)
#source: avx512_fp16_vl.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 58 f4[ 	 ]*vaddph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 58 f4[ 	 ]*vaddph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 58 f4[ 	 ]*vaddph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 58 f4[ 	 ]*vaddph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 58 b4 f4 00 00 00 10[ 	 ]*vaddph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 58 31[ 	 ]*vaddph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 58 71 7f[ 	 ]*vaddph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 58 72 80[ 	 ]*vaddph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 58 b4 f4 00 00 00 10[ 	 ]*vaddph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 58 31[ 	 ]*vaddph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 58 71 7f[ 	 ]*vaddph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 58 72 80[ 	 ]*vaddph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 28 c2 ec 7b[ 	 ]*vcmpph k5,ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 2f c2 ec 7b[ 	 ]*vcmpph k5\{k7\},ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 c2 ec 7b[ 	 ]*vcmpph k5,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f c2 ec 7b[ 	 ]*vcmpph k5\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f c2 ac f4 00 00 00 10 7b[ 	 ]*vcmpph k5\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 c2 29 7b[ 	 ]*vcmpph k5,xmm5,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 c2 69 7f 7b[ 	 ]*vcmpph k5,xmm5,XMMWORD PTR \[ecx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 1f c2 6a 80 7b[ 	 ]*vcmpph k5\{k7\},xmm5,WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 2f c2 ac f4 00 00 00 10 7b[ 	 ]*vcmpph k5\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 38 c2 29 7b[ 	 ]*vcmpph k5,ymm5,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 28 c2 69 7f 7b[ 	 ]*vcmpph k5,ymm5,YMMWORD PTR \[ecx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 3f c2 6a 80 7b[ 	 ]*vcmpph k5\{k7\},ymm5,WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5b f5[ 	 ]*vcvtdq2ph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 5b f5[ 	 ]*vcvtdq2ph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5b f5[ 	 ]*vcvtdq2ph xmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 5b f5[ 	 ]*vcvtdq2ph xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 5b b4 f4 00 00 00 10[ 	 ]*vcvtdq2ph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5b 31[ 	 ]*vcvtdq2ph xmm6,DWORD BCST \[ecx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5b 71 7f[ 	 ]*vcvtdq2ph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5b 72 80[ 	 ]*vcvtdq2ph xmm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 5b 31[ 	 ]*vcvtdq2ph xmm6,DWORD BCST \[ecx\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5b 71 7f[ 	 ]*vcvtdq2ph xmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 5b 72 80[ 	 ]*vcvtdq2ph xmm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 08 5a f5[ 	 ]*vcvtpd2ph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 8f 5a f5[ 	 ]*vcvtpd2ph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 28 5a f5[ 	 ]*vcvtpd2ph xmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd af 5a f5[ 	 ]*vcvtpd2ph xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 0f 5a b4 f4 00 00 00 10[ 	 ]*vcvtpd2ph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 18 5a 31[ 	 ]*vcvtpd2ph xmm6,QWORD BCST \[ecx\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 08 5a 71 7f[ 	 ]*vcvtpd2ph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 9f 5a 72 80[ 	 ]*vcvtpd2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 38 5a 31[ 	 ]*vcvtpd2ph xmm6,QWORD BCST \[ecx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 28 5a 71 7f[ 	 ]*vcvtpd2ph xmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd bf 5a 72 80[ 	 ]*vcvtpd2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 5b f5[ 	 ]*vcvtph2dq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 5b f5[ 	 ]*vcvtph2dq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 5b f5[ 	 ]*vcvtph2dq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 5b f5[ 	 ]*vcvtph2dq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 5b b4 f4 00 00 00 10[ 	 ]*vcvtph2dq xmm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 5b 31[ 	 ]*vcvtph2dq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 5b 71 7f[ 	 ]*vcvtph2dq xmm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 5b 72 80[ 	 ]*vcvtph2dq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 5b b4 f4 00 00 00 10[ 	 ]*vcvtph2dq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 5b 31[ 	 ]*vcvtph2dq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 5b 71 7f[ 	 ]*vcvtph2dq ymm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 5b 72 80[ 	 ]*vcvtph2dq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5a f5[ 	 ]*vcvtph2pd xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 5a f5[ 	 ]*vcvtph2pd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5a f5[ 	 ]*vcvtph2pd ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 5a f5[ 	 ]*vcvtph2pd ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 5a b4 f4 00 00 00 10[ 	 ]*vcvtph2pd xmm6\{k7\},DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5a 31[ 	 ]*vcvtph2pd xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5a 71 7f[ 	 ]*vcvtph2pd xmm6,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5a 72 80[ 	 ]*vcvtph2pd xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 5a b4 f4 00 00 00 10[ 	 ]*vcvtph2pd ymm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 5a 31[ 	 ]*vcvtph2pd ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5a 71 7f[ 	 ]*vcvtph2pd ymm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 5a 72 80[ 	 ]*vcvtph2pd ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 13 f5[ 	 ]*vcvtph2psx xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 13 f5[ 	 ]*vcvtph2psx xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 13 f5[ 	 ]*vcvtph2psx ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 13 f5[ 	 ]*vcvtph2psx ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 13 b4 f4 00 00 00 10[ 	 ]*vcvtph2psx xmm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 13 31[ 	 ]*vcvtph2psx xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 13 71 7f[ 	 ]*vcvtph2psx xmm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 13 72 80[ 	 ]*vcvtph2psx xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 13 b4 f4 00 00 00 10[ 	 ]*vcvtph2psx ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 13 31[ 	 ]*vcvtph2psx ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 13 71 7f[ 	 ]*vcvtph2psx ymm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 13 72 80[ 	 ]*vcvtph2psx ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7b f5[ 	 ]*vcvtph2qq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7b f5[ 	 ]*vcvtph2qq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7b f5[ 	 ]*vcvtph2qq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7b f5[ 	 ]*vcvtph2qq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7b b4 f4 00 00 00 10[ 	 ]*vcvtph2qq xmm6\{k7\},DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7b 31[ 	 ]*vcvtph2qq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7b 71 7f[ 	 ]*vcvtph2qq xmm6,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7b 72 80[ 	 ]*vcvtph2qq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7b b4 f4 00 00 00 10[ 	 ]*vcvtph2qq ymm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7b 31[ 	 ]*vcvtph2qq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7b 71 7f[ 	 ]*vcvtph2qq ymm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7b 72 80[ 	 ]*vcvtph2qq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 79 f5[ 	 ]*vcvtph2udq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 79 f5[ 	 ]*vcvtph2udq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 79 f5[ 	 ]*vcvtph2udq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 79 f5[ 	 ]*vcvtph2udq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 79 b4 f4 00 00 00 10[ 	 ]*vcvtph2udq xmm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 79 31[ 	 ]*vcvtph2udq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 79 71 7f[ 	 ]*vcvtph2udq xmm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 79 72 80[ 	 ]*vcvtph2udq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 79 b4 f4 00 00 00 10[ 	 ]*vcvtph2udq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 79 31[ 	 ]*vcvtph2udq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 79 71 7f[ 	 ]*vcvtph2udq ymm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 79 72 80[ 	 ]*vcvtph2udq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 79 f5[ 	 ]*vcvtph2uqq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 79 f5[ 	 ]*vcvtph2uqq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 79 f5[ 	 ]*vcvtph2uqq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 79 f5[ 	 ]*vcvtph2uqq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 79 b4 f4 00 00 00 10[ 	 ]*vcvtph2uqq xmm6\{k7\},DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 79 31[ 	 ]*vcvtph2uqq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 79 71 7f[ 	 ]*vcvtph2uqq xmm6,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 79 72 80[ 	 ]*vcvtph2uqq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 79 b4 f4 00 00 00 10[ 	 ]*vcvtph2uqq ymm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 79 31[ 	 ]*vcvtph2uqq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 79 71 7f[ 	 ]*vcvtph2uqq ymm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 79 72 80[ 	 ]*vcvtph2uqq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7d f5[ 	 ]*vcvtph2uw xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 7d f5[ 	 ]*vcvtph2uw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7d f5[ 	 ]*vcvtph2uw ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 7d f5[ 	 ]*vcvtph2uw ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 7d b4 f4 00 00 00 10[ 	 ]*vcvtph2uw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7d 31[ 	 ]*vcvtph2uw xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7d 71 7f[ 	 ]*vcvtph2uw xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7d 72 80[ 	 ]*vcvtph2uw xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 7d b4 f4 00 00 00 10[ 	 ]*vcvtph2uw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 7d 31[ 	 ]*vcvtph2uw ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7d 71 7f[ 	 ]*vcvtph2uw ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 7d 72 80[ 	 ]*vcvtph2uw ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7d f5[ 	 ]*vcvtph2w xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7d f5[ 	 ]*vcvtph2w xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7d f5[ 	 ]*vcvtph2w ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7d f5[ 	 ]*vcvtph2w ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7d b4 f4 00 00 00 10[ 	 ]*vcvtph2w xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7d 31[ 	 ]*vcvtph2w xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7d 71 7f[ 	 ]*vcvtph2w xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7d 72 80[ 	 ]*vcvtph2w xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7d b4 f4 00 00 00 10[ 	 ]*vcvtph2w ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7d 31[ 	 ]*vcvtph2w ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7d 71 7f[ 	 ]*vcvtph2w ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7d 72 80[ 	 ]*vcvtph2w ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 1d f5[ 	 ]*vcvtps2phx xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 1d f5[ 	 ]*vcvtps2phx xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 1d f5[ 	 ]*vcvtps2phx xmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 1d f5[ 	 ]*vcvtps2phx xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 1d b4 f4 00 00 00 10[ 	 ]*vcvtps2phx xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 1d 31[ 	 ]*vcvtps2phx xmm6,DWORD BCST \[ecx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 1d 71 7f[ 	 ]*vcvtps2phx xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 1d 72 80[ 	 ]*vcvtps2phx xmm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 1d 31[ 	 ]*vcvtps2phx xmm6,DWORD BCST \[ecx\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 1d 71 7f[ 	 ]*vcvtps2phx xmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 1d 72 80[ 	 ]*vcvtps2phx xmm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 08 5b f5[ 	 ]*vcvtqq2ph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 8f 5b f5[ 	 ]*vcvtqq2ph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 28 5b f5[ 	 ]*vcvtqq2ph xmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc af 5b f5[ 	 ]*vcvtqq2ph xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 0f 5b b4 f4 00 00 00 10[ 	 ]*vcvtqq2ph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 18 5b 31[ 	 ]*vcvtqq2ph xmm6,QWORD BCST \[ecx\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 08 5b 71 7f[ 	 ]*vcvtqq2ph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 9f 5b 72 80[ 	 ]*vcvtqq2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 38 5b 31[ 	 ]*vcvtqq2ph xmm6,QWORD BCST \[ecx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 28 5b 71 7f[ 	 ]*vcvtqq2ph xmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc bf 5b 72 80[ 	 ]*vcvtqq2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 5b f5[ 	 ]*vcvttph2dq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 8f 5b f5[ 	 ]*vcvttph2dq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 5b f5[ 	 ]*vcvttph2dq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e af 5b f5[ 	 ]*vcvttph2dq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 5b b4 f4 00 00 00 10[ 	 ]*vcvttph2dq xmm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 5b 31[ 	 ]*vcvttph2dq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 5b 71 7f[ 	 ]*vcvttph2dq xmm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 5b 72 80[ 	 ]*vcvttph2dq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 2f 5b b4 f4 00 00 00 10[ 	 ]*vcvttph2dq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 38 5b 31[ 	 ]*vcvttph2dq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 5b 71 7f[ 	 ]*vcvttph2dq ymm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e bf 5b 72 80[ 	 ]*vcvttph2dq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7a f5[ 	 ]*vcvttph2qq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7a f5[ 	 ]*vcvttph2qq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7a f5[ 	 ]*vcvttph2qq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7a f5[ 	 ]*vcvttph2qq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7a b4 f4 00 00 00 10[ 	 ]*vcvttph2qq xmm6\{k7\},DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7a 31[ 	 ]*vcvttph2qq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7a 71 7f[ 	 ]*vcvttph2qq xmm6,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7a 72 80[ 	 ]*vcvttph2qq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7a b4 f4 00 00 00 10[ 	 ]*vcvttph2qq ymm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7a 31[ 	 ]*vcvttph2qq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7a 71 7f[ 	 ]*vcvttph2qq ymm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7a 72 80[ 	 ]*vcvttph2qq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 78 f5[ 	 ]*vcvttph2udq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 78 f5[ 	 ]*vcvttph2udq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 78 f5[ 	 ]*vcvttph2udq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 78 f5[ 	 ]*vcvttph2udq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 78 b4 f4 00 00 00 10[ 	 ]*vcvttph2udq xmm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 78 31[ 	 ]*vcvttph2udq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 78 71 7f[ 	 ]*vcvttph2udq xmm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 78 72 80[ 	 ]*vcvttph2udq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 78 b4 f4 00 00 00 10[ 	 ]*vcvttph2udq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 78 31[ 	 ]*vcvttph2udq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 78 71 7f[ 	 ]*vcvttph2udq ymm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 78 72 80[ 	 ]*vcvttph2udq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 78 f5[ 	 ]*vcvttph2uqq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 78 f5[ 	 ]*vcvttph2uqq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 78 f5[ 	 ]*vcvttph2uqq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 78 f5[ 	 ]*vcvttph2uqq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 78 b4 f4 00 00 00 10[ 	 ]*vcvttph2uqq xmm6\{k7\},DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 78 31[ 	 ]*vcvttph2uqq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 78 71 7f[ 	 ]*vcvttph2uqq xmm6,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 78 72 80[ 	 ]*vcvttph2uqq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 78 b4 f4 00 00 00 10[ 	 ]*vcvttph2uqq ymm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 78 31[ 	 ]*vcvttph2uqq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 78 71 7f[ 	 ]*vcvttph2uqq ymm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 78 72 80[ 	 ]*vcvttph2uqq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7c f5[ 	 ]*vcvttph2uw xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 7c f5[ 	 ]*vcvttph2uw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7c f5[ 	 ]*vcvttph2uw ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 7c f5[ 	 ]*vcvttph2uw ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 7c b4 f4 00 00 00 10[ 	 ]*vcvttph2uw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7c 31[ 	 ]*vcvttph2uw xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7c 71 7f[ 	 ]*vcvttph2uw xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7c 72 80[ 	 ]*vcvttph2uw xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 7c b4 f4 00 00 00 10[ 	 ]*vcvttph2uw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 7c 31[ 	 ]*vcvttph2uw ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7c 71 7f[ 	 ]*vcvttph2uw ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 7c 72 80[ 	 ]*vcvttph2uw ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7c f5[ 	 ]*vcvttph2w xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7c f5[ 	 ]*vcvttph2w xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7c f5[ 	 ]*vcvttph2w ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7c f5[ 	 ]*vcvttph2w ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7c b4 f4 00 00 00 10[ 	 ]*vcvttph2w xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7c 31[ 	 ]*vcvttph2w xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7c 71 7f[ 	 ]*vcvttph2w xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7c 72 80[ 	 ]*vcvttph2w xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7c b4 f4 00 00 00 10[ 	 ]*vcvttph2w ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7c 31[ 	 ]*vcvttph2w ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7c 71 7f[ 	 ]*vcvttph2w ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7c 72 80[ 	 ]*vcvttph2w ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7a f5[ 	 ]*vcvtudq2ph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 8f 7a f5[ 	 ]*vcvtudq2ph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7a f5[ 	 ]*vcvtudq2ph xmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f af 7a f5[ 	 ]*vcvtudq2ph xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 0f 7a b4 f4 00 00 00 10[ 	 ]*vcvtudq2ph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7a 31[ 	 ]*vcvtudq2ph xmm6,DWORD BCST \[ecx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7a 71 7f[ 	 ]*vcvtudq2ph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7a 72 80[ 	 ]*vcvtudq2ph xmm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 38 7a 31[ 	 ]*vcvtudq2ph xmm6,DWORD BCST \[ecx\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7a 71 7f[ 	 ]*vcvtudq2ph xmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f bf 7a 72 80[ 	 ]*vcvtudq2ph xmm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 08 7a f5[ 	 ]*vcvtuqq2ph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 8f 7a f5[ 	 ]*vcvtuqq2ph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 28 7a f5[ 	 ]*vcvtuqq2ph xmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff af 7a f5[ 	 ]*vcvtuqq2ph xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 0f 7a b4 f4 00 00 00 10[ 	 ]*vcvtuqq2ph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 18 7a 31[ 	 ]*vcvtuqq2ph xmm6,QWORD BCST \[ecx\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 08 7a 71 7f[ 	 ]*vcvtuqq2ph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 9f 7a 72 80[ 	 ]*vcvtuqq2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 38 7a 31[ 	 ]*vcvtuqq2ph xmm6,QWORD BCST \[ecx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 28 7a 71 7f[ 	 ]*vcvtuqq2ph xmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff bf 7a 72 80[ 	 ]*vcvtuqq2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7d f5[ 	 ]*vcvtuw2ph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 8f 7d f5[ 	 ]*vcvtuw2ph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7d f5[ 	 ]*vcvtuw2ph ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f af 7d f5[ 	 ]*vcvtuw2ph ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 0f 7d b4 f4 00 00 00 10[ 	 ]*vcvtuw2ph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7d 31[ 	 ]*vcvtuw2ph xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7d 71 7f[ 	 ]*vcvtuw2ph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7d 72 80[ 	 ]*vcvtuw2ph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 2f 7d b4 f4 00 00 00 10[ 	 ]*vcvtuw2ph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 38 7d 31[ 	 ]*vcvtuw2ph ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7d 71 7f[ 	 ]*vcvtuw2ph ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f bf 7d 72 80[ 	 ]*vcvtuw2ph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 7d f5[ 	 ]*vcvtw2ph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 8f 7d f5[ 	 ]*vcvtw2ph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 7d f5[ 	 ]*vcvtw2ph ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e af 7d f5[ 	 ]*vcvtw2ph ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 7d b4 f4 00 00 00 10[ 	 ]*vcvtw2ph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 7d 31[ 	 ]*vcvtw2ph xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 7d 71 7f[ 	 ]*vcvtw2ph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 7d 72 80[ 	 ]*vcvtw2ph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 2f 7d b4 f4 00 00 00 10[ 	 ]*vcvtw2ph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 38 7d 31[ 	 ]*vcvtw2ph ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 7d 71 7f[ 	 ]*vcvtw2ph ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e bf 7d 72 80[ 	 ]*vcvtw2ph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5e f4[ 	 ]*vdivph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5e f4[ 	 ]*vdivph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5e f4[ 	 ]*vdivph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5e f4[ 	 ]*vdivph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5e b4 f4 00 00 00 10[ 	 ]*vdivph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5e 31[ 	 ]*vdivph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5e 71 7f[ 	 ]*vdivph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5e 72 80[ 	 ]*vdivph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5e b4 f4 00 00 00 10[ 	 ]*vdivph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5e 31[ 	 ]*vdivph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5e 71 7f[ 	 ]*vdivph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5e 72 80[ 	 ]*vdivph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 56 f4[ 	 ]*vfcmaddcph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 af 56 f4[ 	 ]*vfcmaddcph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 56 f4[ 	 ]*vfcmaddcph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f 56 f4[ 	 ]*vfcmaddcph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 2f 56 b4 f4 00 00 00 10[ 	 ]*vfcmaddcph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 38 56 31[ 	 ]*vfcmaddcph ymm6,ymm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 56 71 7f[ 	 ]*vfcmaddcph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 bf 56 72 80[ 	 ]*vfcmaddcph ymm6\{k7\}\{z\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f 56 b4 f4 00 00 00 10[ 	 ]*vfcmaddcph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 56 31[ 	 ]*vfcmaddcph xmm6,xmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 56 71 7f[ 	 ]*vfcmaddcph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f 56 72 80[ 	 ]*vfcmaddcph xmm6\{k7\}\{z\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 d6 f4[ 	 ]*vfcmulcph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 af d6 f4[ 	 ]*vfcmulcph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d6 f4[ 	 ]*vfcmulcph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f d6 f4[ 	 ]*vfcmulcph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 2f d6 b4 f4 00 00 00 10[ 	 ]*vfcmulcph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 38 d6 31[ 	 ]*vfcmulcph ymm6,ymm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 d6 71 7f[ 	 ]*vfcmulcph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 bf d6 72 80[ 	 ]*vfcmulcph ymm6\{k7\}\{z\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f d6 b4 f4 00 00 00 10[ 	 ]*vfcmulcph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 d6 31[ 	 ]*vfcmulcph xmm6,xmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d6 71 7f[ 	 ]*vfcmulcph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f d6 72 80[ 	 ]*vfcmulcph xmm6\{k7\}\{z\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 98 f4[ 	 ]*vfmadd132ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 98 f4[ 	 ]*vfmadd132ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 98 f4[ 	 ]*vfmadd132ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 98 f4[ 	 ]*vfmadd132ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 98 b4 f4 00 00 00 10[ 	 ]*vfmadd132ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 98 31[ 	 ]*vfmadd132ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 98 71 7f[ 	 ]*vfmadd132ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 98 72 80[ 	 ]*vfmadd132ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 98 b4 f4 00 00 00 10[ 	 ]*vfmadd132ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 98 31[ 	 ]*vfmadd132ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 98 71 7f[ 	 ]*vfmadd132ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 98 72 80[ 	 ]*vfmadd132ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a8 f4[ 	 ]*vfmadd213ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af a8 f4[ 	 ]*vfmadd213ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a8 f4[ 	 ]*vfmadd213ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a8 f4[ 	 ]*vfmadd213ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f a8 b4 f4 00 00 00 10[ 	 ]*vfmadd213ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 a8 31[ 	 ]*vfmadd213ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a8 71 7f[ 	 ]*vfmadd213ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf a8 72 80[ 	 ]*vfmadd213ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a8 b4 f4 00 00 00 10[ 	 ]*vfmadd213ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a8 31[ 	 ]*vfmadd213ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a8 71 7f[ 	 ]*vfmadd213ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a8 72 80[ 	 ]*vfmadd213ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b8 f4[ 	 ]*vfmadd231ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af b8 f4[ 	 ]*vfmadd231ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b8 f4[ 	 ]*vfmadd231ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b8 f4[ 	 ]*vfmadd231ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f b8 b4 f4 00 00 00 10[ 	 ]*vfmadd231ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 b8 31[ 	 ]*vfmadd231ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b8 71 7f[ 	 ]*vfmadd231ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf b8 72 80[ 	 ]*vfmadd231ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b8 b4 f4 00 00 00 10[ 	 ]*vfmadd231ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b8 31[ 	 ]*vfmadd231ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b8 71 7f[ 	 ]*vfmadd231ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b8 72 80[ 	 ]*vfmadd231ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 56 f4[ 	 ]*vfmaddcph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 af 56 f4[ 	 ]*vfmaddcph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 56 f4[ 	 ]*vfmaddcph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f 56 f4[ 	 ]*vfmaddcph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 2f 56 b4 f4 00 00 00 10[ 	 ]*vfmaddcph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 38 56 31[ 	 ]*vfmaddcph ymm6,ymm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 56 71 7f[ 	 ]*vfmaddcph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 bf 56 72 80[ 	 ]*vfmaddcph ymm6\{k7\}\{z\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f 56 b4 f4 00 00 00 10[ 	 ]*vfmaddcph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 56 31[ 	 ]*vfmaddcph xmm6,xmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 56 71 7f[ 	 ]*vfmaddcph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f 56 72 80[ 	 ]*vfmaddcph xmm6\{k7\}\{z\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 96 f4[ 	 ]*vfmaddsub132ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 96 f4[ 	 ]*vfmaddsub132ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 96 f4[ 	 ]*vfmaddsub132ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 96 f4[ 	 ]*vfmaddsub132ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 96 b4 f4 00 00 00 10[ 	 ]*vfmaddsub132ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 96 31[ 	 ]*vfmaddsub132ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 96 71 7f[ 	 ]*vfmaddsub132ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 96 72 80[ 	 ]*vfmaddsub132ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 96 b4 f4 00 00 00 10[ 	 ]*vfmaddsub132ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 96 31[ 	 ]*vfmaddsub132ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 96 71 7f[ 	 ]*vfmaddsub132ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 96 72 80[ 	 ]*vfmaddsub132ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a6 f4[ 	 ]*vfmaddsub213ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af a6 f4[ 	 ]*vfmaddsub213ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a6 f4[ 	 ]*vfmaddsub213ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a6 f4[ 	 ]*vfmaddsub213ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f a6 b4 f4 00 00 00 10[ 	 ]*vfmaddsub213ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 a6 31[ 	 ]*vfmaddsub213ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a6 71 7f[ 	 ]*vfmaddsub213ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf a6 72 80[ 	 ]*vfmaddsub213ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a6 b4 f4 00 00 00 10[ 	 ]*vfmaddsub213ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a6 31[ 	 ]*vfmaddsub213ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a6 71 7f[ 	 ]*vfmaddsub213ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a6 72 80[ 	 ]*vfmaddsub213ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b6 f4[ 	 ]*vfmaddsub231ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af b6 f4[ 	 ]*vfmaddsub231ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b6 f4[ 	 ]*vfmaddsub231ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b6 f4[ 	 ]*vfmaddsub231ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f b6 b4 f4 00 00 00 10[ 	 ]*vfmaddsub231ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 b6 31[ 	 ]*vfmaddsub231ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b6 71 7f[ 	 ]*vfmaddsub231ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf b6 72 80[ 	 ]*vfmaddsub231ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b6 b4 f4 00 00 00 10[ 	 ]*vfmaddsub231ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b6 31[ 	 ]*vfmaddsub231ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b6 71 7f[ 	 ]*vfmaddsub231ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b6 72 80[ 	 ]*vfmaddsub231ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9a f4[ 	 ]*vfmsub132ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 9a f4[ 	 ]*vfmsub132ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9a f4[ 	 ]*vfmsub132ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9a f4[ 	 ]*vfmsub132ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 9a b4 f4 00 00 00 10[ 	 ]*vfmsub132ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 9a 31[ 	 ]*vfmsub132ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9a 71 7f[ 	 ]*vfmsub132ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 9a 72 80[ 	 ]*vfmsub132ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9a b4 f4 00 00 00 10[ 	 ]*vfmsub132ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9a 31[ 	 ]*vfmsub132ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9a 71 7f[ 	 ]*vfmsub132ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9a 72 80[ 	 ]*vfmsub132ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 aa f4[ 	 ]*vfmsub213ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af aa f4[ 	 ]*vfmsub213ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 aa f4[ 	 ]*vfmsub213ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f aa f4[ 	 ]*vfmsub213ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f aa b4 f4 00 00 00 10[ 	 ]*vfmsub213ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 aa 31[ 	 ]*vfmsub213ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 aa 71 7f[ 	 ]*vfmsub213ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf aa 72 80[ 	 ]*vfmsub213ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f aa b4 f4 00 00 00 10[ 	 ]*vfmsub213ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 aa 31[ 	 ]*vfmsub213ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 aa 71 7f[ 	 ]*vfmsub213ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f aa 72 80[ 	 ]*vfmsub213ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ba f4[ 	 ]*vfmsub231ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af ba f4[ 	 ]*vfmsub231ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ba f4[ 	 ]*vfmsub231ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ba f4[ 	 ]*vfmsub231ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f ba b4 f4 00 00 00 10[ 	 ]*vfmsub231ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 ba 31[ 	 ]*vfmsub231ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ba 71 7f[ 	 ]*vfmsub231ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf ba 72 80[ 	 ]*vfmsub231ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ba b4 f4 00 00 00 10[ 	 ]*vfmsub231ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ba 31[ 	 ]*vfmsub231ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ba 71 7f[ 	 ]*vfmsub231ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ba 72 80[ 	 ]*vfmsub231ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 97 f4[ 	 ]*vfmsubadd132ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 97 f4[ 	 ]*vfmsubadd132ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 97 f4[ 	 ]*vfmsubadd132ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 97 f4[ 	 ]*vfmsubadd132ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 97 b4 f4 00 00 00 10[ 	 ]*vfmsubadd132ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 97 31[ 	 ]*vfmsubadd132ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 97 71 7f[ 	 ]*vfmsubadd132ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 97 72 80[ 	 ]*vfmsubadd132ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 97 b4 f4 00 00 00 10[ 	 ]*vfmsubadd132ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 97 31[ 	 ]*vfmsubadd132ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 97 71 7f[ 	 ]*vfmsubadd132ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 97 72 80[ 	 ]*vfmsubadd132ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a7 f4[ 	 ]*vfmsubadd213ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af a7 f4[ 	 ]*vfmsubadd213ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a7 f4[ 	 ]*vfmsubadd213ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a7 f4[ 	 ]*vfmsubadd213ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f a7 b4 f4 00 00 00 10[ 	 ]*vfmsubadd213ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 a7 31[ 	 ]*vfmsubadd213ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a7 71 7f[ 	 ]*vfmsubadd213ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf a7 72 80[ 	 ]*vfmsubadd213ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a7 b4 f4 00 00 00 10[ 	 ]*vfmsubadd213ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a7 31[ 	 ]*vfmsubadd213ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a7 71 7f[ 	 ]*vfmsubadd213ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a7 72 80[ 	 ]*vfmsubadd213ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b7 f4[ 	 ]*vfmsubadd231ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af b7 f4[ 	 ]*vfmsubadd231ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b7 f4[ 	 ]*vfmsubadd231ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b7 f4[ 	 ]*vfmsubadd231ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f b7 b4 f4 00 00 00 10[ 	 ]*vfmsubadd231ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 b7 31[ 	 ]*vfmsubadd231ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b7 71 7f[ 	 ]*vfmsubadd231ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf b7 72 80[ 	 ]*vfmsubadd231ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b7 b4 f4 00 00 00 10[ 	 ]*vfmsubadd231ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b7 31[ 	 ]*vfmsubadd231ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b7 71 7f[ 	 ]*vfmsubadd231ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b7 72 80[ 	 ]*vfmsubadd231ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 d6 f4[ 	 ]*vfmulcph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 af d6 f4[ 	 ]*vfmulcph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d6 f4[ 	 ]*vfmulcph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f d6 f4[ 	 ]*vfmulcph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 2f d6 b4 f4 00 00 00 10[ 	 ]*vfmulcph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 38 d6 31[ 	 ]*vfmulcph ymm6,ymm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 d6 71 7f[ 	 ]*vfmulcph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 bf d6 72 80[ 	 ]*vfmulcph ymm6\{k7\}\{z\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f d6 b4 f4 00 00 00 10[ 	 ]*vfmulcph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 d6 31[ 	 ]*vfmulcph xmm6,xmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d6 71 7f[ 	 ]*vfmulcph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f d6 72 80[ 	 ]*vfmulcph xmm6\{k7\}\{z\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9c f4[ 	 ]*vfnmadd132ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 9c f4[ 	 ]*vfnmadd132ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9c f4[ 	 ]*vfnmadd132ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9c f4[ 	 ]*vfnmadd132ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 9c b4 f4 00 00 00 10[ 	 ]*vfnmadd132ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 9c 31[ 	 ]*vfnmadd132ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9c 71 7f[ 	 ]*vfnmadd132ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 9c 72 80[ 	 ]*vfnmadd132ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9c b4 f4 00 00 00 10[ 	 ]*vfnmadd132ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9c 31[ 	 ]*vfnmadd132ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9c 71 7f[ 	 ]*vfnmadd132ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9c 72 80[ 	 ]*vfnmadd132ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ac f4[ 	 ]*vfnmadd213ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af ac f4[ 	 ]*vfnmadd213ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ac f4[ 	 ]*vfnmadd213ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ac f4[ 	 ]*vfnmadd213ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f ac b4 f4 00 00 00 10[ 	 ]*vfnmadd213ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 ac 31[ 	 ]*vfnmadd213ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ac 71 7f[ 	 ]*vfnmadd213ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf ac 72 80[ 	 ]*vfnmadd213ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ac b4 f4 00 00 00 10[ 	 ]*vfnmadd213ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ac 31[ 	 ]*vfnmadd213ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ac 71 7f[ 	 ]*vfnmadd213ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ac 72 80[ 	 ]*vfnmadd213ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 bc f4[ 	 ]*vfnmadd231ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af bc f4[ 	 ]*vfnmadd231ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bc f4[ 	 ]*vfnmadd231ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bc f4[ 	 ]*vfnmadd231ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f bc b4 f4 00 00 00 10[ 	 ]*vfnmadd231ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 bc 31[ 	 ]*vfnmadd231ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 bc 71 7f[ 	 ]*vfnmadd231ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf bc 72 80[ 	 ]*vfnmadd231ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bc b4 f4 00 00 00 10[ 	 ]*vfnmadd231ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bc 31[ 	 ]*vfnmadd231ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bc 71 7f[ 	 ]*vfnmadd231ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bc 72 80[ 	 ]*vfnmadd231ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9e f4[ 	 ]*vfnmsub132ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 9e f4[ 	 ]*vfnmsub132ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9e f4[ 	 ]*vfnmsub132ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9e f4[ 	 ]*vfnmsub132ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 9e b4 f4 00 00 00 10[ 	 ]*vfnmsub132ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 9e 31[ 	 ]*vfnmsub132ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9e 71 7f[ 	 ]*vfnmsub132ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 9e 72 80[ 	 ]*vfnmsub132ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9e b4 f4 00 00 00 10[ 	 ]*vfnmsub132ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9e 31[ 	 ]*vfnmsub132ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9e 71 7f[ 	 ]*vfnmsub132ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9e 72 80[ 	 ]*vfnmsub132ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ae f4[ 	 ]*vfnmsub213ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af ae f4[ 	 ]*vfnmsub213ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ae f4[ 	 ]*vfnmsub213ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ae f4[ 	 ]*vfnmsub213ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f ae b4 f4 00 00 00 10[ 	 ]*vfnmsub213ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 ae 31[ 	 ]*vfnmsub213ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ae 71 7f[ 	 ]*vfnmsub213ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf ae 72 80[ 	 ]*vfnmsub213ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ae b4 f4 00 00 00 10[ 	 ]*vfnmsub213ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ae 31[ 	 ]*vfnmsub213ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ae 71 7f[ 	 ]*vfnmsub213ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ae 72 80[ 	 ]*vfnmsub213ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 be f4[ 	 ]*vfnmsub231ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af be f4[ 	 ]*vfnmsub231ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 be f4[ 	 ]*vfnmsub231ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f be f4[ 	 ]*vfnmsub231ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f be b4 f4 00 00 00 10[ 	 ]*vfnmsub231ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 be 31[ 	 ]*vfnmsub231ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 be 71 7f[ 	 ]*vfnmsub231ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf be 72 80[ 	 ]*vfnmsub231ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f be b4 f4 00 00 00 10[ 	 ]*vfnmsub231ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 be 31[ 	 ]*vfnmsub231ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 be 71 7f[ 	 ]*vfnmsub231ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f be 72 80[ 	 ]*vfnmsub231ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 66 ee 7b[ 	 ]*vfpclassph k5,xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 66 ee 7b[ 	 ]*vfpclassph k5\{k7\},xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 66 ee 7b[ 	 ]*vfpclassph k5,ymm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 66 ee 7b[ 	 ]*vfpclassph k5\{k7\},ymm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 66 ac f4 00 00 00 10 7b[ 	 ]*vfpclassph k5\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 66 29 7b[ 	 ]*vfpclassph k5,WORD BCST \[ecx\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 66 69 7f 7b[ 	 ]*vfpclassph k5,XMMWORD PTR \[ecx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 1f 66 6a 80 7b[ 	 ]*vfpclassph k5\{k7\},WORD BCST \[edx-0x100\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 66 69 01 7b[ 	 ]*vfpclassph k5,WORD BCST \[ecx\+0x2\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 66 29 7b[ 	 ]*vfpclassph k5,WORD BCST \[ecx\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 66 69 7f 7b[ 	 ]*vfpclassph k5,YMMWORD PTR \[ecx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 3f 66 6a 80 7b[ 	 ]*vfpclassph k5\{k7\},WORD BCST \[edx-0x100\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 66 69 01 7b[ 	 ]*vfpclassph k5,WORD BCST \[ecx\+0x2\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 42 f5[ 	 ]*vgetexpph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 42 f5[ 	 ]*vgetexpph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 42 f5[ 	 ]*vgetexpph ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 42 f5[ 	 ]*vgetexpph ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 42 b4 f4 00 00 00 10[ 	 ]*vgetexpph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 42 31[ 	 ]*vgetexpph xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 42 71 7f[ 	 ]*vgetexpph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 42 72 80[ 	 ]*vgetexpph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 42 b4 f4 00 00 00 10[ 	 ]*vgetexpph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 42 31[ 	 ]*vgetexpph ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 42 71 7f[ 	 ]*vgetexpph ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 42 72 80[ 	 ]*vgetexpph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 26 f5 7b[ 	 ]*vgetmantph ymm6,ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c af 26 f5 7b[ 	 ]*vgetmantph ymm6\{k7\}\{z\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 26 f5 7b[ 	 ]*vgetmantph xmm6,xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 8f 26 f5 7b[ 	 ]*vgetmantph xmm6\{k7\}\{z\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 26 b4 f4 00 00 00 10 7b[ 	 ]*vgetmantph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 26 31 7b[ 	 ]*vgetmantph xmm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 26 71 7f 7b[ 	 ]*vgetmantph xmm6,XMMWORD PTR \[ecx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 26 72 80 7b[ 	 ]*vgetmantph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 26 b4 f4 00 00 00 10 7b[ 	 ]*vgetmantph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 26 31 7b[ 	 ]*vgetmantph ymm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 26 71 7f 7b[ 	 ]*vgetmantph ymm6,YMMWORD PTR \[ecx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c bf 26 72 80 7b[ 	 ]*vgetmantph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5f f4[ 	 ]*vmaxph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5f f4[ 	 ]*vmaxph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5f f4[ 	 ]*vmaxph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5f f4[ 	 ]*vmaxph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5f b4 f4 00 00 00 10[ 	 ]*vmaxph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5f 31[ 	 ]*vmaxph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5f 71 7f[ 	 ]*vmaxph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5f 72 80[ 	 ]*vmaxph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5f b4 f4 00 00 00 10[ 	 ]*vmaxph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5f 31[ 	 ]*vmaxph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5f 71 7f[ 	 ]*vmaxph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5f 72 80[ 	 ]*vmaxph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5d f4[ 	 ]*vminph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5d f4[ 	 ]*vminph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5d f4[ 	 ]*vminph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5d f4[ 	 ]*vminph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5d b4 f4 00 00 00 10[ 	 ]*vminph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5d 31[ 	 ]*vminph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5d 71 7f[ 	 ]*vminph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5d 72 80[ 	 ]*vminph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5d b4 f4 00 00 00 10[ 	 ]*vminph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5d 31[ 	 ]*vminph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5d 71 7f[ 	 ]*vminph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5d 72 80[ 	 ]*vminph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 59 f4[ 	 ]*vmulph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 59 f4[ 	 ]*vmulph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 59 f4[ 	 ]*vmulph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 59 f4[ 	 ]*vmulph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 59 b4 f4 00 00 00 10[ 	 ]*vmulph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 59 31[ 	 ]*vmulph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 59 71 7f[ 	 ]*vmulph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 59 72 80[ 	 ]*vmulph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 59 b4 f4 00 00 00 10[ 	 ]*vmulph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 59 31[ 	 ]*vmulph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 59 71 7f[ 	 ]*vmulph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 59 72 80[ 	 ]*vmulph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4c f5[ 	 ]*vrcpph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 4c f5[ 	 ]*vrcpph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4c f5[ 	 ]*vrcpph ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 4c f5[ 	 ]*vrcpph ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 4c b4 f4 00 00 00 10[ 	 ]*vrcpph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 4c 31[ 	 ]*vrcpph xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4c 71 7f[ 	 ]*vrcpph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 4c 72 80[ 	 ]*vrcpph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 4c b4 f4 00 00 00 10[ 	 ]*vrcpph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 4c 31[ 	 ]*vrcpph ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4c 71 7f[ 	 ]*vrcpph ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 4c 72 80[ 	 ]*vrcpph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 56 f5 7b[ 	 ]*vreduceph ymm6,ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c af 56 f5 7b[ 	 ]*vreduceph ymm6\{k7\}\{z\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 56 f5 7b[ 	 ]*vreduceph xmm6,xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 8f 56 f5 7b[ 	 ]*vreduceph xmm6\{k7\}\{z\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 56 b4 f4 00 00 00 10 7b[ 	 ]*vreduceph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 56 31 7b[ 	 ]*vreduceph xmm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 56 71 7f 7b[ 	 ]*vreduceph xmm6,XMMWORD PTR \[ecx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 56 72 80 7b[ 	 ]*vreduceph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 56 b4 f4 00 00 00 10 7b[ 	 ]*vreduceph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 56 31 7b[ 	 ]*vreduceph ymm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 56 71 7f 7b[ 	 ]*vreduceph ymm6,YMMWORD PTR \[ecx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c bf 56 72 80 7b[ 	 ]*vreduceph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 08 f5 7b[ 	 ]*vrndscaleph ymm6,ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c af 08 f5 7b[ 	 ]*vrndscaleph ymm6\{k7\}\{z\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 08 f5 7b[ 	 ]*vrndscaleph xmm6,xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 8f 08 f5 7b[ 	 ]*vrndscaleph xmm6\{k7\}\{z\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 08 b4 f4 00 00 00 10 7b[ 	 ]*vrndscaleph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 08 31 7b[ 	 ]*vrndscaleph xmm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 08 71 7f 7b[ 	 ]*vrndscaleph xmm6,XMMWORD PTR \[ecx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 08 72 80 7b[ 	 ]*vrndscaleph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 08 b4 f4 00 00 00 10 7b[ 	 ]*vrndscaleph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 08 31 7b[ 	 ]*vrndscaleph ymm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 08 71 7f 7b[ 	 ]*vrndscaleph ymm6,YMMWORD PTR \[ecx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c bf 08 72 80 7b[ 	 ]*vrndscaleph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4e f5[ 	 ]*vrsqrtph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 4e f5[ 	 ]*vrsqrtph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4e f5[ 	 ]*vrsqrtph ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 4e f5[ 	 ]*vrsqrtph ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 4e b4 f4 00 00 00 10[ 	 ]*vrsqrtph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 4e 31[ 	 ]*vrsqrtph xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4e 71 7f[ 	 ]*vrsqrtph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 4e 72 80[ 	 ]*vrsqrtph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 4e b4 f4 00 00 00 10[ 	 ]*vrsqrtph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 4e 31[ 	 ]*vrsqrtph ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4e 71 7f[ 	 ]*vrsqrtph ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 4e 72 80[ 	 ]*vrsqrtph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 2c f4[ 	 ]*vscalefph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 2c f4[ 	 ]*vscalefph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2c f4[ 	 ]*vscalefph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 2c f4[ 	 ]*vscalefph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 2c b4 f4 00 00 00 10[ 	 ]*vscalefph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 2c 31[ 	 ]*vscalefph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 2c 71 7f[ 	 ]*vscalefph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 2c 72 80[ 	 ]*vscalefph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 2c b4 f4 00 00 00 10[ 	 ]*vscalefph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 2c 31[ 	 ]*vscalefph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2c 71 7f[ 	 ]*vscalefph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 2c 72 80[ 	 ]*vscalefph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 51 f5[ 	 ]*vsqrtph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 51 f5[ 	 ]*vsqrtph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 51 f5[ 	 ]*vsqrtph ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 51 f5[ 	 ]*vsqrtph ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 51 b4 f4 00 00 00 10[ 	 ]*vsqrtph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 51 31[ 	 ]*vsqrtph xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 51 71 7f[ 	 ]*vsqrtph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 51 72 80[ 	 ]*vsqrtph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 51 b4 f4 00 00 00 10[ 	 ]*vsqrtph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 51 31[ 	 ]*vsqrtph ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 51 71 7f[ 	 ]*vsqrtph ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 51 72 80[ 	 ]*vsqrtph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5c f4[ 	 ]*vsubph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5c f4[ 	 ]*vsubph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5c f4[ 	 ]*vsubph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5c f4[ 	 ]*vsubph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5c b4 f4 00 00 00 10[ 	 ]*vsubph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5c 31[ 	 ]*vsubph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5c 71 7f[ 	 ]*vsubph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5c 72 80[ 	 ]*vsubph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5c b4 f4 00 00 00 10[ 	 ]*vsubph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5c 31[ 	 ]*vsubph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5c 71 7f[ 	 ]*vsubph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5c 72 80[ 	 ]*vsubph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 58 f4[ 	 ]*vaddph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 58 f4[ 	 ]*vaddph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 58 f4[ 	 ]*vaddph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 58 f4[ 	 ]*vaddph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 58 b4 f4 00 00 00 10[ 	 ]*vaddph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 58 31[ 	 ]*vaddph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 58 71 7f[ 	 ]*vaddph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 58 72 80[ 	 ]*vaddph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 58 b4 f4 00 00 00 10[ 	 ]*vaddph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 58 31[ 	 ]*vaddph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 58 71 7f[ 	 ]*vaddph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 58 72 80[ 	 ]*vaddph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 28 c2 ec 7b[ 	 ]*vcmpph k5,ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 2f c2 ec 7b[ 	 ]*vcmpph k5\{k7\},ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 c2 ec 7b[ 	 ]*vcmpph k5,xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f c2 ec 7b[ 	 ]*vcmpph k5\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 0f c2 ac f4 00 00 00 10 7b[ 	 ]*vcmpph k5\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 18 c2 29 7b[ 	 ]*vcmpph k5,xmm5,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 08 c2 69 7f 7b[ 	 ]*vcmpph k5,xmm5,XMMWORD PTR \[ecx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 1f c2 6a 80 7b[ 	 ]*vcmpph k5\{k7\},xmm5,WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 2f c2 ac f4 00 00 00 10 7b[ 	 ]*vcmpph k5\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 38 c2 29 7b[ 	 ]*vcmpph k5,ymm5,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 28 c2 69 7f 7b[ 	 ]*vcmpph k5,ymm5,YMMWORD PTR \[ecx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 54 3f c2 6a 80 7b[ 	 ]*vcmpph k5\{k7\},ymm5,WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5b f5[ 	 ]*vcvtdq2ph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 5b f5[ 	 ]*vcvtdq2ph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5b f5[ 	 ]*vcvtdq2ph xmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 5b f5[ 	 ]*vcvtdq2ph xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 5b b4 f4 00 00 00 10[ 	 ]*vcvtdq2ph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5b 31[ 	 ]*vcvtdq2ph xmm6,DWORD BCST \[ecx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5b 71 7f[ 	 ]*vcvtdq2ph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5b 72 80[ 	 ]*vcvtdq2ph xmm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 5b 31[ 	 ]*vcvtdq2ph xmm6,DWORD BCST \[ecx\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5b 71 7f[ 	 ]*vcvtdq2ph xmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 5b 72 80[ 	 ]*vcvtdq2ph xmm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 08 5a f5[ 	 ]*vcvtpd2ph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 8f 5a f5[ 	 ]*vcvtpd2ph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 28 5a f5[ 	 ]*vcvtpd2ph xmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd af 5a f5[ 	 ]*vcvtpd2ph xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 0f 5a b4 f4 00 00 00 10[ 	 ]*vcvtpd2ph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 18 5a 31[ 	 ]*vcvtpd2ph xmm6,QWORD BCST \[ecx\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 08 5a 71 7f[ 	 ]*vcvtpd2ph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 9f 5a 72 80[ 	 ]*vcvtpd2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 38 5a 31[ 	 ]*vcvtpd2ph xmm6,QWORD BCST \[ecx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd 28 5a 71 7f[ 	 ]*vcvtpd2ph xmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fd bf 5a 72 80[ 	 ]*vcvtpd2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 5b f5[ 	 ]*vcvtph2dq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 5b f5[ 	 ]*vcvtph2dq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 5b f5[ 	 ]*vcvtph2dq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 5b f5[ 	 ]*vcvtph2dq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 5b b4 f4 00 00 00 10[ 	 ]*vcvtph2dq xmm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 5b 31[ 	 ]*vcvtph2dq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 5b 71 7f[ 	 ]*vcvtph2dq xmm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 5b 72 80[ 	 ]*vcvtph2dq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 5b b4 f4 00 00 00 10[ 	 ]*vcvtph2dq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 5b 31[ 	 ]*vcvtph2dq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 5b 71 7f[ 	 ]*vcvtph2dq ymm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 5b 72 80[ 	 ]*vcvtph2dq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5a f5[ 	 ]*vcvtph2pd xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 5a f5[ 	 ]*vcvtph2pd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5a f5[ 	 ]*vcvtph2pd ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 5a f5[ 	 ]*vcvtph2pd ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 5a b4 f4 00 00 00 10[ 	 ]*vcvtph2pd xmm6\{k7\},DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 5a 31[ 	 ]*vcvtph2pd xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 5a 71 7f[ 	 ]*vcvtph2pd xmm6,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 5a 72 80[ 	 ]*vcvtph2pd xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 5a b4 f4 00 00 00 10[ 	 ]*vcvtph2pd ymm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 5a 31[ 	 ]*vcvtph2pd ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 5a 71 7f[ 	 ]*vcvtph2pd ymm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 5a 72 80[ 	 ]*vcvtph2pd ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 13 f5[ 	 ]*vcvtph2psx xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 13 f5[ 	 ]*vcvtph2psx xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 13 f5[ 	 ]*vcvtph2psx ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 13 f5[ 	 ]*vcvtph2psx ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 13 b4 f4 00 00 00 10[ 	 ]*vcvtph2psx xmm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 13 31[ 	 ]*vcvtph2psx xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 13 71 7f[ 	 ]*vcvtph2psx xmm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 13 72 80[ 	 ]*vcvtph2psx xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 13 b4 f4 00 00 00 10[ 	 ]*vcvtph2psx ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 13 31[ 	 ]*vcvtph2psx ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 13 71 7f[ 	 ]*vcvtph2psx ymm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 13 72 80[ 	 ]*vcvtph2psx ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7b f5[ 	 ]*vcvtph2qq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7b f5[ 	 ]*vcvtph2qq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7b f5[ 	 ]*vcvtph2qq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7b f5[ 	 ]*vcvtph2qq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7b b4 f4 00 00 00 10[ 	 ]*vcvtph2qq xmm6\{k7\},DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7b 31[ 	 ]*vcvtph2qq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7b 71 7f[ 	 ]*vcvtph2qq xmm6,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7b 72 80[ 	 ]*vcvtph2qq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7b b4 f4 00 00 00 10[ 	 ]*vcvtph2qq ymm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7b 31[ 	 ]*vcvtph2qq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7b 71 7f[ 	 ]*vcvtph2qq ymm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7b 72 80[ 	 ]*vcvtph2qq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 79 f5[ 	 ]*vcvtph2udq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 79 f5[ 	 ]*vcvtph2udq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 79 f5[ 	 ]*vcvtph2udq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 79 f5[ 	 ]*vcvtph2udq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 79 b4 f4 00 00 00 10[ 	 ]*vcvtph2udq xmm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 79 31[ 	 ]*vcvtph2udq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 79 71 7f[ 	 ]*vcvtph2udq xmm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 79 72 80[ 	 ]*vcvtph2udq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 79 b4 f4 00 00 00 10[ 	 ]*vcvtph2udq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 79 31[ 	 ]*vcvtph2udq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 79 71 7f[ 	 ]*vcvtph2udq ymm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 79 72 80[ 	 ]*vcvtph2udq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 79 f5[ 	 ]*vcvtph2uqq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 79 f5[ 	 ]*vcvtph2uqq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 79 f5[ 	 ]*vcvtph2uqq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 79 f5[ 	 ]*vcvtph2uqq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 79 b4 f4 00 00 00 10[ 	 ]*vcvtph2uqq xmm6\{k7\},DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 79 31[ 	 ]*vcvtph2uqq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 79 71 7f[ 	 ]*vcvtph2uqq xmm6,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 79 72 80[ 	 ]*vcvtph2uqq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 79 b4 f4 00 00 00 10[ 	 ]*vcvtph2uqq ymm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 79 31[ 	 ]*vcvtph2uqq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 79 71 7f[ 	 ]*vcvtph2uqq ymm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 79 72 80[ 	 ]*vcvtph2uqq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7d f5[ 	 ]*vcvtph2uw xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 7d f5[ 	 ]*vcvtph2uw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7d f5[ 	 ]*vcvtph2uw ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 7d f5[ 	 ]*vcvtph2uw ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 7d b4 f4 00 00 00 10[ 	 ]*vcvtph2uw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7d 31[ 	 ]*vcvtph2uw xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7d 71 7f[ 	 ]*vcvtph2uw xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7d 72 80[ 	 ]*vcvtph2uw xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 7d b4 f4 00 00 00 10[ 	 ]*vcvtph2uw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 7d 31[ 	 ]*vcvtph2uw ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7d 71 7f[ 	 ]*vcvtph2uw ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 7d 72 80[ 	 ]*vcvtph2uw ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7d f5[ 	 ]*vcvtph2w xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7d f5[ 	 ]*vcvtph2w xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7d f5[ 	 ]*vcvtph2w ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7d f5[ 	 ]*vcvtph2w ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7d b4 f4 00 00 00 10[ 	 ]*vcvtph2w xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7d 31[ 	 ]*vcvtph2w xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7d 71 7f[ 	 ]*vcvtph2w xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7d 72 80[ 	 ]*vcvtph2w xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7d b4 f4 00 00 00 10[ 	 ]*vcvtph2w ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7d 31[ 	 ]*vcvtph2w ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7d 71 7f[ 	 ]*vcvtph2w ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7d 72 80[ 	 ]*vcvtph2w ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 1d f5[ 	 ]*vcvtps2phx xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 1d f5[ 	 ]*vcvtps2phx xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 1d f5[ 	 ]*vcvtps2phx xmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 1d f5[ 	 ]*vcvtps2phx xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 1d b4 f4 00 00 00 10[ 	 ]*vcvtps2phx xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 1d 31[ 	 ]*vcvtps2phx xmm6,DWORD BCST \[ecx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 1d 71 7f[ 	 ]*vcvtps2phx xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 1d 72 80[ 	 ]*vcvtps2phx xmm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 1d 31[ 	 ]*vcvtps2phx xmm6,DWORD BCST \[ecx\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 1d 71 7f[ 	 ]*vcvtps2phx xmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 1d 72 80[ 	 ]*vcvtps2phx xmm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 08 5b f5[ 	 ]*vcvtqq2ph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 8f 5b f5[ 	 ]*vcvtqq2ph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 28 5b f5[ 	 ]*vcvtqq2ph xmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc af 5b f5[ 	 ]*vcvtqq2ph xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 0f 5b b4 f4 00 00 00 10[ 	 ]*vcvtqq2ph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 18 5b 31[ 	 ]*vcvtqq2ph xmm6,QWORD BCST \[ecx\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 08 5b 71 7f[ 	 ]*vcvtqq2ph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 9f 5b 72 80[ 	 ]*vcvtqq2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 38 5b 31[ 	 ]*vcvtqq2ph xmm6,QWORD BCST \[ecx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc 28 5b 71 7f[ 	 ]*vcvtqq2ph xmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 fc bf 5b 72 80[ 	 ]*vcvtqq2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 5b f5[ 	 ]*vcvttph2dq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 8f 5b f5[ 	 ]*vcvttph2dq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 5b f5[ 	 ]*vcvttph2dq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e af 5b f5[ 	 ]*vcvttph2dq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 5b b4 f4 00 00 00 10[ 	 ]*vcvttph2dq xmm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 5b 31[ 	 ]*vcvttph2dq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 5b 71 7f[ 	 ]*vcvttph2dq xmm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 5b 72 80[ 	 ]*vcvttph2dq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 2f 5b b4 f4 00 00 00 10[ 	 ]*vcvttph2dq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 38 5b 31[ 	 ]*vcvttph2dq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 5b 71 7f[ 	 ]*vcvttph2dq ymm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e bf 5b 72 80[ 	 ]*vcvttph2dq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7a f5[ 	 ]*vcvttph2qq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7a f5[ 	 ]*vcvttph2qq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7a f5[ 	 ]*vcvttph2qq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7a f5[ 	 ]*vcvttph2qq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7a b4 f4 00 00 00 10[ 	 ]*vcvttph2qq xmm6\{k7\},DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7a 31[ 	 ]*vcvttph2qq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7a 71 7f[ 	 ]*vcvttph2qq xmm6,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7a 72 80[ 	 ]*vcvttph2qq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7a b4 f4 00 00 00 10[ 	 ]*vcvttph2qq ymm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7a 31[ 	 ]*vcvttph2qq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7a 71 7f[ 	 ]*vcvttph2qq ymm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7a 72 80[ 	 ]*vcvttph2qq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 78 f5[ 	 ]*vcvttph2udq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 78 f5[ 	 ]*vcvttph2udq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 78 f5[ 	 ]*vcvttph2udq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 78 f5[ 	 ]*vcvttph2udq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 78 b4 f4 00 00 00 10[ 	 ]*vcvttph2udq xmm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 78 31[ 	 ]*vcvttph2udq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 78 71 7f[ 	 ]*vcvttph2udq xmm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 78 72 80[ 	 ]*vcvttph2udq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 78 b4 f4 00 00 00 10[ 	 ]*vcvttph2udq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 78 31[ 	 ]*vcvttph2udq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 78 71 7f[ 	 ]*vcvttph2udq ymm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 78 72 80[ 	 ]*vcvttph2udq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 78 f5[ 	 ]*vcvttph2uqq xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 78 f5[ 	 ]*vcvttph2uqq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 78 f5[ 	 ]*vcvttph2uqq ymm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 78 f5[ 	 ]*vcvttph2uqq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 78 b4 f4 00 00 00 10[ 	 ]*vcvttph2uqq xmm6\{k7\},DWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 78 31[ 	 ]*vcvttph2uqq xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 78 71 7f[ 	 ]*vcvttph2uqq xmm6,DWORD PTR \[ecx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 78 72 80[ 	 ]*vcvttph2uqq xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 78 b4 f4 00 00 00 10[ 	 ]*vcvttph2uqq ymm6\{k7\},QWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 78 31[ 	 ]*vcvttph2uqq ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 78 71 7f[ 	 ]*vcvttph2uqq ymm6,QWORD PTR \[ecx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 78 72 80[ 	 ]*vcvttph2uqq ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7c f5[ 	 ]*vcvttph2uw xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 7c f5[ 	 ]*vcvttph2uw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7c f5[ 	 ]*vcvttph2uw ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 7c f5[ 	 ]*vcvttph2uw ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 7c b4 f4 00 00 00 10[ 	 ]*vcvttph2uw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 7c 31[ 	 ]*vcvttph2uw xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 7c 71 7f[ 	 ]*vcvttph2uw xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 7c 72 80[ 	 ]*vcvttph2uw xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 7c b4 f4 00 00 00 10[ 	 ]*vcvttph2uw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 7c 31[ 	 ]*vcvttph2uw ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 7c 71 7f[ 	 ]*vcvttph2uw ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 7c 72 80[ 	 ]*vcvttph2uw ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7c f5[ 	 ]*vcvttph2w xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 8f 7c f5[ 	 ]*vcvttph2w xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7c f5[ 	 ]*vcvttph2w ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d af 7c f5[ 	 ]*vcvttph2w ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 0f 7c b4 f4 00 00 00 10[ 	 ]*vcvttph2w xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 18 7c 31[ 	 ]*vcvttph2w xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 08 7c 71 7f[ 	 ]*vcvttph2w xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 9f 7c 72 80[ 	 ]*vcvttph2w xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 2f 7c b4 f4 00 00 00 10[ 	 ]*vcvttph2w ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 38 7c 31[ 	 ]*vcvttph2w ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d 28 7c 71 7f[ 	 ]*vcvttph2w ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7d bf 7c 72 80[ 	 ]*vcvttph2w ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7a f5[ 	 ]*vcvtudq2ph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 8f 7a f5[ 	 ]*vcvtudq2ph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7a f5[ 	 ]*vcvtudq2ph xmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f af 7a f5[ 	 ]*vcvtudq2ph xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 0f 7a b4 f4 00 00 00 10[ 	 ]*vcvtudq2ph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7a 31[ 	 ]*vcvtudq2ph xmm6,DWORD BCST \[ecx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7a 71 7f[ 	 ]*vcvtudq2ph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7a 72 80[ 	 ]*vcvtudq2ph xmm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 38 7a 31[ 	 ]*vcvtudq2ph xmm6,DWORD BCST \[ecx\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7a 71 7f[ 	 ]*vcvtudq2ph xmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f bf 7a 72 80[ 	 ]*vcvtudq2ph xmm6\{k7\}\{z\},DWORD BCST \[edx-0x200\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 08 7a f5[ 	 ]*vcvtuqq2ph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 8f 7a f5[ 	 ]*vcvtuqq2ph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 28 7a f5[ 	 ]*vcvtuqq2ph xmm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff af 7a f5[ 	 ]*vcvtuqq2ph xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 0f 7a b4 f4 00 00 00 10[ 	 ]*vcvtuqq2ph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 18 7a 31[ 	 ]*vcvtuqq2ph xmm6,QWORD BCST \[ecx\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 08 7a 71 7f[ 	 ]*vcvtuqq2ph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 9f 7a 72 80[ 	 ]*vcvtuqq2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 38 7a 31[ 	 ]*vcvtuqq2ph xmm6,QWORD BCST \[ecx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff 28 7a 71 7f[ 	 ]*vcvtuqq2ph xmm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 ff bf 7a 72 80[ 	 ]*vcvtuqq2ph xmm6\{k7\}\{z\},QWORD BCST \[edx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7d f5[ 	 ]*vcvtuw2ph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 8f 7d f5[ 	 ]*vcvtuw2ph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7d f5[ 	 ]*vcvtuw2ph ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f af 7d f5[ 	 ]*vcvtuw2ph ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 0f 7d b4 f4 00 00 00 10[ 	 ]*vcvtuw2ph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 18 7d 31[ 	 ]*vcvtuw2ph xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 08 7d 71 7f[ 	 ]*vcvtuw2ph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 9f 7d 72 80[ 	 ]*vcvtuw2ph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 2f 7d b4 f4 00 00 00 10[ 	 ]*vcvtuw2ph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 38 7d 31[ 	 ]*vcvtuw2ph ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f 28 7d 71 7f[ 	 ]*vcvtuw2ph ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7f bf 7d 72 80[ 	 ]*vcvtuw2ph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 7d f5[ 	 ]*vcvtw2ph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 8f 7d f5[ 	 ]*vcvtw2ph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 7d f5[ 	 ]*vcvtw2ph ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e af 7d f5[ 	 ]*vcvtw2ph ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 0f 7d b4 f4 00 00 00 10[ 	 ]*vcvtw2ph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 18 7d 31[ 	 ]*vcvtw2ph xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 08 7d 71 7f[ 	 ]*vcvtw2ph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 9f 7d 72 80[ 	 ]*vcvtw2ph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 2f 7d b4 f4 00 00 00 10[ 	 ]*vcvtw2ph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 38 7d 31[ 	 ]*vcvtw2ph ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e 28 7d 71 7f[ 	 ]*vcvtw2ph ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7e bf 7d 72 80[ 	 ]*vcvtw2ph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5e f4[ 	 ]*vdivph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5e f4[ 	 ]*vdivph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5e f4[ 	 ]*vdivph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5e f4[ 	 ]*vdivph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5e b4 f4 00 00 00 10[ 	 ]*vdivph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5e 31[ 	 ]*vdivph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5e 71 7f[ 	 ]*vdivph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5e 72 80[ 	 ]*vdivph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5e b4 f4 00 00 00 10[ 	 ]*vdivph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5e 31[ 	 ]*vdivph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5e 71 7f[ 	 ]*vdivph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5e 72 80[ 	 ]*vdivph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 56 f4[ 	 ]*vfcmaddcph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 af 56 f4[ 	 ]*vfcmaddcph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 56 f4[ 	 ]*vfcmaddcph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f 56 f4[ 	 ]*vfcmaddcph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 2f 56 b4 f4 00 00 00 10[ 	 ]*vfcmaddcph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 38 56 31[ 	 ]*vfcmaddcph ymm6,ymm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 56 71 7f[ 	 ]*vfcmaddcph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 bf 56 72 80[ 	 ]*vfcmaddcph ymm6\{k7\}\{z\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f 56 b4 f4 00 00 00 10[ 	 ]*vfcmaddcph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 56 31[ 	 ]*vfcmaddcph xmm6,xmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 56 71 7f[ 	 ]*vfcmaddcph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f 56 72 80[ 	 ]*vfcmaddcph xmm6\{k7\}\{z\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 d6 f4[ 	 ]*vfcmulcph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 af d6 f4[ 	 ]*vfcmulcph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d6 f4[ 	 ]*vfcmulcph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 8f d6 f4[ 	 ]*vfcmulcph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 2f d6 b4 f4 00 00 00 10[ 	 ]*vfcmulcph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 38 d6 31[ 	 ]*vfcmulcph ymm6,ymm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 28 d6 71 7f[ 	 ]*vfcmulcph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 bf d6 72 80[ 	 ]*vfcmulcph ymm6\{k7\}\{z\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 0f d6 b4 f4 00 00 00 10[ 	 ]*vfcmulcph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 18 d6 31[ 	 ]*vfcmulcph xmm6,xmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 08 d6 71 7f[ 	 ]*vfcmulcph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 57 9f d6 72 80[ 	 ]*vfcmulcph xmm6\{k7\}\{z\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 98 f4[ 	 ]*vfmadd132ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 98 f4[ 	 ]*vfmadd132ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 98 f4[ 	 ]*vfmadd132ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 98 f4[ 	 ]*vfmadd132ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 98 b4 f4 00 00 00 10[ 	 ]*vfmadd132ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 98 31[ 	 ]*vfmadd132ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 98 71 7f[ 	 ]*vfmadd132ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 98 72 80[ 	 ]*vfmadd132ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 98 b4 f4 00 00 00 10[ 	 ]*vfmadd132ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 98 31[ 	 ]*vfmadd132ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 98 71 7f[ 	 ]*vfmadd132ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 98 72 80[ 	 ]*vfmadd132ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a8 f4[ 	 ]*vfmadd213ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af a8 f4[ 	 ]*vfmadd213ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a8 f4[ 	 ]*vfmadd213ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a8 f4[ 	 ]*vfmadd213ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f a8 b4 f4 00 00 00 10[ 	 ]*vfmadd213ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 a8 31[ 	 ]*vfmadd213ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a8 71 7f[ 	 ]*vfmadd213ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf a8 72 80[ 	 ]*vfmadd213ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a8 b4 f4 00 00 00 10[ 	 ]*vfmadd213ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a8 31[ 	 ]*vfmadd213ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a8 71 7f[ 	 ]*vfmadd213ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a8 72 80[ 	 ]*vfmadd213ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b8 f4[ 	 ]*vfmadd231ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af b8 f4[ 	 ]*vfmadd231ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b8 f4[ 	 ]*vfmadd231ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b8 f4[ 	 ]*vfmadd231ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f b8 b4 f4 00 00 00 10[ 	 ]*vfmadd231ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 b8 31[ 	 ]*vfmadd231ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b8 71 7f[ 	 ]*vfmadd231ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf b8 72 80[ 	 ]*vfmadd231ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b8 b4 f4 00 00 00 10[ 	 ]*vfmadd231ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b8 31[ 	 ]*vfmadd231ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b8 71 7f[ 	 ]*vfmadd231ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b8 72 80[ 	 ]*vfmadd231ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 56 f4[ 	 ]*vfmaddcph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 af 56 f4[ 	 ]*vfmaddcph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 56 f4[ 	 ]*vfmaddcph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f 56 f4[ 	 ]*vfmaddcph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 2f 56 b4 f4 00 00 00 10[ 	 ]*vfmaddcph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 38 56 31[ 	 ]*vfmaddcph ymm6,ymm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 56 71 7f[ 	 ]*vfmaddcph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 bf 56 72 80[ 	 ]*vfmaddcph ymm6\{k7\}\{z\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f 56 b4 f4 00 00 00 10[ 	 ]*vfmaddcph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 56 31[ 	 ]*vfmaddcph xmm6,xmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 56 71 7f[ 	 ]*vfmaddcph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f 56 72 80[ 	 ]*vfmaddcph xmm6\{k7\}\{z\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 96 f4[ 	 ]*vfmaddsub132ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 96 f4[ 	 ]*vfmaddsub132ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 96 f4[ 	 ]*vfmaddsub132ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 96 f4[ 	 ]*vfmaddsub132ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 96 b4 f4 00 00 00 10[ 	 ]*vfmaddsub132ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 96 31[ 	 ]*vfmaddsub132ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 96 71 7f[ 	 ]*vfmaddsub132ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 96 72 80[ 	 ]*vfmaddsub132ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 96 b4 f4 00 00 00 10[ 	 ]*vfmaddsub132ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 96 31[ 	 ]*vfmaddsub132ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 96 71 7f[ 	 ]*vfmaddsub132ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 96 72 80[ 	 ]*vfmaddsub132ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a6 f4[ 	 ]*vfmaddsub213ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af a6 f4[ 	 ]*vfmaddsub213ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a6 f4[ 	 ]*vfmaddsub213ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a6 f4[ 	 ]*vfmaddsub213ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f a6 b4 f4 00 00 00 10[ 	 ]*vfmaddsub213ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 a6 31[ 	 ]*vfmaddsub213ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a6 71 7f[ 	 ]*vfmaddsub213ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf a6 72 80[ 	 ]*vfmaddsub213ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a6 b4 f4 00 00 00 10[ 	 ]*vfmaddsub213ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a6 31[ 	 ]*vfmaddsub213ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a6 71 7f[ 	 ]*vfmaddsub213ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a6 72 80[ 	 ]*vfmaddsub213ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b6 f4[ 	 ]*vfmaddsub231ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af b6 f4[ 	 ]*vfmaddsub231ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b6 f4[ 	 ]*vfmaddsub231ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b6 f4[ 	 ]*vfmaddsub231ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f b6 b4 f4 00 00 00 10[ 	 ]*vfmaddsub231ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 b6 31[ 	 ]*vfmaddsub231ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b6 71 7f[ 	 ]*vfmaddsub231ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf b6 72 80[ 	 ]*vfmaddsub231ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b6 b4 f4 00 00 00 10[ 	 ]*vfmaddsub231ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b6 31[ 	 ]*vfmaddsub231ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b6 71 7f[ 	 ]*vfmaddsub231ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b6 72 80[ 	 ]*vfmaddsub231ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9a f4[ 	 ]*vfmsub132ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 9a f4[ 	 ]*vfmsub132ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9a f4[ 	 ]*vfmsub132ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9a f4[ 	 ]*vfmsub132ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 9a b4 f4 00 00 00 10[ 	 ]*vfmsub132ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 9a 31[ 	 ]*vfmsub132ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9a 71 7f[ 	 ]*vfmsub132ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 9a 72 80[ 	 ]*vfmsub132ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9a b4 f4 00 00 00 10[ 	 ]*vfmsub132ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9a 31[ 	 ]*vfmsub132ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9a 71 7f[ 	 ]*vfmsub132ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9a 72 80[ 	 ]*vfmsub132ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 aa f4[ 	 ]*vfmsub213ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af aa f4[ 	 ]*vfmsub213ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 aa f4[ 	 ]*vfmsub213ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f aa f4[ 	 ]*vfmsub213ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f aa b4 f4 00 00 00 10[ 	 ]*vfmsub213ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 aa 31[ 	 ]*vfmsub213ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 aa 71 7f[ 	 ]*vfmsub213ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf aa 72 80[ 	 ]*vfmsub213ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f aa b4 f4 00 00 00 10[ 	 ]*vfmsub213ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 aa 31[ 	 ]*vfmsub213ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 aa 71 7f[ 	 ]*vfmsub213ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f aa 72 80[ 	 ]*vfmsub213ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ba f4[ 	 ]*vfmsub231ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af ba f4[ 	 ]*vfmsub231ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ba f4[ 	 ]*vfmsub231ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ba f4[ 	 ]*vfmsub231ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f ba b4 f4 00 00 00 10[ 	 ]*vfmsub231ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 ba 31[ 	 ]*vfmsub231ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ba 71 7f[ 	 ]*vfmsub231ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf ba 72 80[ 	 ]*vfmsub231ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ba b4 f4 00 00 00 10[ 	 ]*vfmsub231ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ba 31[ 	 ]*vfmsub231ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ba 71 7f[ 	 ]*vfmsub231ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ba 72 80[ 	 ]*vfmsub231ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 97 f4[ 	 ]*vfmsubadd132ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 97 f4[ 	 ]*vfmsubadd132ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 97 f4[ 	 ]*vfmsubadd132ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 97 f4[ 	 ]*vfmsubadd132ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 97 b4 f4 00 00 00 10[ 	 ]*vfmsubadd132ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 97 31[ 	 ]*vfmsubadd132ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 97 71 7f[ 	 ]*vfmsubadd132ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 97 72 80[ 	 ]*vfmsubadd132ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 97 b4 f4 00 00 00 10[ 	 ]*vfmsubadd132ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 97 31[ 	 ]*vfmsubadd132ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 97 71 7f[ 	 ]*vfmsubadd132ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 97 72 80[ 	 ]*vfmsubadd132ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a7 f4[ 	 ]*vfmsubadd213ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af a7 f4[ 	 ]*vfmsubadd213ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a7 f4[ 	 ]*vfmsubadd213ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f a7 f4[ 	 ]*vfmsubadd213ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f a7 b4 f4 00 00 00 10[ 	 ]*vfmsubadd213ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 a7 31[ 	 ]*vfmsubadd213ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 a7 71 7f[ 	 ]*vfmsubadd213ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf a7 72 80[ 	 ]*vfmsubadd213ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f a7 b4 f4 00 00 00 10[ 	 ]*vfmsubadd213ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 a7 31[ 	 ]*vfmsubadd213ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 a7 71 7f[ 	 ]*vfmsubadd213ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f a7 72 80[ 	 ]*vfmsubadd213ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b7 f4[ 	 ]*vfmsubadd231ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af b7 f4[ 	 ]*vfmsubadd231ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b7 f4[ 	 ]*vfmsubadd231ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f b7 f4[ 	 ]*vfmsubadd231ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f b7 b4 f4 00 00 00 10[ 	 ]*vfmsubadd231ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 b7 31[ 	 ]*vfmsubadd231ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 b7 71 7f[ 	 ]*vfmsubadd231ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf b7 72 80[ 	 ]*vfmsubadd231ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f b7 b4 f4 00 00 00 10[ 	 ]*vfmsubadd231ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 b7 31[ 	 ]*vfmsubadd231ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 b7 71 7f[ 	 ]*vfmsubadd231ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f b7 72 80[ 	 ]*vfmsubadd231ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 d6 f4[ 	 ]*vfmulcph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 af d6 f4[ 	 ]*vfmulcph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d6 f4[ 	 ]*vfmulcph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 8f d6 f4[ 	 ]*vfmulcph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 2f d6 b4 f4 00 00 00 10[ 	 ]*vfmulcph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 38 d6 31[ 	 ]*vfmulcph ymm6,ymm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 28 d6 71 7f[ 	 ]*vfmulcph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 bf d6 72 80[ 	 ]*vfmulcph ymm6\{k7\}\{z\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 0f d6 b4 f4 00 00 00 10[ 	 ]*vfmulcph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 18 d6 31[ 	 ]*vfmulcph xmm6,xmm5,DWORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 08 d6 71 7f[ 	 ]*vfmulcph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 56 9f d6 72 80[ 	 ]*vfmulcph xmm6\{k7\}\{z\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9c f4[ 	 ]*vfnmadd132ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 9c f4[ 	 ]*vfnmadd132ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9c f4[ 	 ]*vfnmadd132ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9c f4[ 	 ]*vfnmadd132ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 9c b4 f4 00 00 00 10[ 	 ]*vfnmadd132ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 9c 31[ 	 ]*vfnmadd132ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9c 71 7f[ 	 ]*vfnmadd132ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 9c 72 80[ 	 ]*vfnmadd132ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9c b4 f4 00 00 00 10[ 	 ]*vfnmadd132ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9c 31[ 	 ]*vfnmadd132ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9c 71 7f[ 	 ]*vfnmadd132ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9c 72 80[ 	 ]*vfnmadd132ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ac f4[ 	 ]*vfnmadd213ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af ac f4[ 	 ]*vfnmadd213ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ac f4[ 	 ]*vfnmadd213ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ac f4[ 	 ]*vfnmadd213ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f ac b4 f4 00 00 00 10[ 	 ]*vfnmadd213ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 ac 31[ 	 ]*vfnmadd213ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ac 71 7f[ 	 ]*vfnmadd213ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf ac 72 80[ 	 ]*vfnmadd213ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ac b4 f4 00 00 00 10[ 	 ]*vfnmadd213ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ac 31[ 	 ]*vfnmadd213ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ac 71 7f[ 	 ]*vfnmadd213ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ac 72 80[ 	 ]*vfnmadd213ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 bc f4[ 	 ]*vfnmadd231ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af bc f4[ 	 ]*vfnmadd231ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bc f4[ 	 ]*vfnmadd231ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f bc f4[ 	 ]*vfnmadd231ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f bc b4 f4 00 00 00 10[ 	 ]*vfnmadd231ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 bc 31[ 	 ]*vfnmadd231ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 bc 71 7f[ 	 ]*vfnmadd231ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf bc 72 80[ 	 ]*vfnmadd231ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f bc b4 f4 00 00 00 10[ 	 ]*vfnmadd231ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 bc 31[ 	 ]*vfnmadd231ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 bc 71 7f[ 	 ]*vfnmadd231ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f bc 72 80[ 	 ]*vfnmadd231ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9e f4[ 	 ]*vfnmsub132ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 9e f4[ 	 ]*vfnmsub132ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9e f4[ 	 ]*vfnmsub132ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 9e f4[ 	 ]*vfnmsub132ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 9e b4 f4 00 00 00 10[ 	 ]*vfnmsub132ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 9e 31[ 	 ]*vfnmsub132ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 9e 71 7f[ 	 ]*vfnmsub132ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 9e 72 80[ 	 ]*vfnmsub132ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 9e b4 f4 00 00 00 10[ 	 ]*vfnmsub132ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 9e 31[ 	 ]*vfnmsub132ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 9e 71 7f[ 	 ]*vfnmsub132ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 9e 72 80[ 	 ]*vfnmsub132ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ae f4[ 	 ]*vfnmsub213ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af ae f4[ 	 ]*vfnmsub213ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ae f4[ 	 ]*vfnmsub213ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f ae f4[ 	 ]*vfnmsub213ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f ae b4 f4 00 00 00 10[ 	 ]*vfnmsub213ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 ae 31[ 	 ]*vfnmsub213ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 ae 71 7f[ 	 ]*vfnmsub213ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf ae 72 80[ 	 ]*vfnmsub213ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f ae b4 f4 00 00 00 10[ 	 ]*vfnmsub213ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 ae 31[ 	 ]*vfnmsub213ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 ae 71 7f[ 	 ]*vfnmsub213ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f ae 72 80[ 	 ]*vfnmsub213ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 be f4[ 	 ]*vfnmsub231ph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af be f4[ 	 ]*vfnmsub231ph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 be f4[ 	 ]*vfnmsub231ph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f be f4[ 	 ]*vfnmsub231ph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f be b4 f4 00 00 00 10[ 	 ]*vfnmsub231ph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 be 31[ 	 ]*vfnmsub231ph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 be 71 7f[ 	 ]*vfnmsub231ph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf be 72 80[ 	 ]*vfnmsub231ph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f be b4 f4 00 00 00 10[ 	 ]*vfnmsub231ph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 be 31[ 	 ]*vfnmsub231ph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 be 71 7f[ 	 ]*vfnmsub231ph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f be 72 80[ 	 ]*vfnmsub231ph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 66 ee 7b[ 	 ]*vfpclassph k5,xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 66 ee 7b[ 	 ]*vfpclassph k5\{k7\},xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 66 ee 7b[ 	 ]*vfpclassph k5,ymm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 66 ee 7b[ 	 ]*vfpclassph k5\{k7\},ymm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 66 ac f4 00 00 00 10 7b[ 	 ]*vfpclassph k5\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 66 29 7b[ 	 ]*vfpclassph k5,WORD BCST \[ecx\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 66 69 7f 7b[ 	 ]*vfpclassph k5,XMMWORD PTR \[ecx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 1f 66 6a 80 7b[ 	 ]*vfpclassph k5\{k7\},WORD BCST \[edx-0x100\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 66 29 7b[ 	 ]*vfpclassph k5,WORD BCST \[ecx\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 66 69 7f 7b[ 	 ]*vfpclassph k5,YMMWORD PTR \[ecx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 3f 66 6a 80 7b[ 	 ]*vfpclassph k5\{k7\},WORD BCST \[edx-0x100\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 42 f5[ 	 ]*vgetexpph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 42 f5[ 	 ]*vgetexpph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 42 f5[ 	 ]*vgetexpph ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 42 f5[ 	 ]*vgetexpph ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 42 b4 f4 00 00 00 10[ 	 ]*vgetexpph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 42 31[ 	 ]*vgetexpph xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 42 71 7f[ 	 ]*vgetexpph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 42 72 80[ 	 ]*vgetexpph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 42 b4 f4 00 00 00 10[ 	 ]*vgetexpph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 42 31[ 	 ]*vgetexpph ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 42 71 7f[ 	 ]*vgetexpph ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 42 72 80[ 	 ]*vgetexpph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 26 f5 7b[ 	 ]*vgetmantph ymm6,ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c af 26 f5 7b[ 	 ]*vgetmantph ymm6\{k7\}\{z\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 26 f5 7b[ 	 ]*vgetmantph xmm6,xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 8f 26 f5 7b[ 	 ]*vgetmantph xmm6\{k7\}\{z\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 26 b4 f4 00 00 00 10 7b[ 	 ]*vgetmantph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 26 31 7b[ 	 ]*vgetmantph xmm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 26 71 7f 7b[ 	 ]*vgetmantph xmm6,XMMWORD PTR \[ecx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 26 72 80 7b[ 	 ]*vgetmantph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 26 b4 f4 00 00 00 10 7b[ 	 ]*vgetmantph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 26 31 7b[ 	 ]*vgetmantph ymm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 26 71 7f 7b[ 	 ]*vgetmantph ymm6,YMMWORD PTR \[ecx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c bf 26 72 80 7b[ 	 ]*vgetmantph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5f f4[ 	 ]*vmaxph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5f f4[ 	 ]*vmaxph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5f f4[ 	 ]*vmaxph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5f f4[ 	 ]*vmaxph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5f b4 f4 00 00 00 10[ 	 ]*vmaxph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5f 31[ 	 ]*vmaxph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5f 71 7f[ 	 ]*vmaxph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5f 72 80[ 	 ]*vmaxph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5f b4 f4 00 00 00 10[ 	 ]*vmaxph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5f 31[ 	 ]*vmaxph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5f 71 7f[ 	 ]*vmaxph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5f 72 80[ 	 ]*vmaxph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5d f4[ 	 ]*vminph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5d f4[ 	 ]*vminph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5d f4[ 	 ]*vminph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5d f4[ 	 ]*vminph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5d b4 f4 00 00 00 10[ 	 ]*vminph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5d 31[ 	 ]*vminph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5d 71 7f[ 	 ]*vminph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5d 72 80[ 	 ]*vminph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5d b4 f4 00 00 00 10[ 	 ]*vminph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5d 31[ 	 ]*vminph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5d 71 7f[ 	 ]*vminph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5d 72 80[ 	 ]*vminph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 59 f4[ 	 ]*vmulph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 59 f4[ 	 ]*vmulph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 59 f4[ 	 ]*vmulph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 59 f4[ 	 ]*vmulph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 59 b4 f4 00 00 00 10[ 	 ]*vmulph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 59 31[ 	 ]*vmulph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 59 71 7f[ 	 ]*vmulph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 59 72 80[ 	 ]*vmulph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 59 b4 f4 00 00 00 10[ 	 ]*vmulph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 59 31[ 	 ]*vmulph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 59 71 7f[ 	 ]*vmulph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 59 72 80[ 	 ]*vmulph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4c f5[ 	 ]*vrcpph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 4c f5[ 	 ]*vrcpph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4c f5[ 	 ]*vrcpph ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 4c f5[ 	 ]*vrcpph ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 4c b4 f4 00 00 00 10[ 	 ]*vrcpph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 4c 31[ 	 ]*vrcpph xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4c 71 7f[ 	 ]*vrcpph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 4c 72 80[ 	 ]*vrcpph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 4c b4 f4 00 00 00 10[ 	 ]*vrcpph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 4c 31[ 	 ]*vrcpph ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4c 71 7f[ 	 ]*vrcpph ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 4c 72 80[ 	 ]*vrcpph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 56 f5 7b[ 	 ]*vreduceph ymm6,ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c af 56 f5 7b[ 	 ]*vreduceph ymm6\{k7\}\{z\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 56 f5 7b[ 	 ]*vreduceph xmm6,xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 8f 56 f5 7b[ 	 ]*vreduceph xmm6\{k7\}\{z\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 56 b4 f4 00 00 00 10 7b[ 	 ]*vreduceph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 56 31 7b[ 	 ]*vreduceph xmm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 56 71 7f 7b[ 	 ]*vreduceph xmm6,XMMWORD PTR \[ecx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 56 72 80 7b[ 	 ]*vreduceph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 56 b4 f4 00 00 00 10 7b[ 	 ]*vreduceph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 56 31 7b[ 	 ]*vreduceph ymm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 56 71 7f 7b[ 	 ]*vreduceph ymm6,YMMWORD PTR \[ecx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c bf 56 72 80 7b[ 	 ]*vreduceph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 08 f5 7b[ 	 ]*vrndscaleph ymm6,ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c af 08 f5 7b[ 	 ]*vrndscaleph ymm6\{k7\}\{z\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 08 f5 7b[ 	 ]*vrndscaleph xmm6,xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 8f 08 f5 7b[ 	 ]*vrndscaleph xmm6\{k7\}\{z\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 0f 08 b4 f4 00 00 00 10 7b[ 	 ]*vrndscaleph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 18 08 31 7b[ 	 ]*vrndscaleph xmm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 08 71 7f 7b[ 	 ]*vrndscaleph xmm6,XMMWORD PTR \[ecx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 9f 08 72 80 7b[ 	 ]*vrndscaleph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 2f 08 b4 f4 00 00 00 10 7b[ 	 ]*vrndscaleph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 38 08 31 7b[ 	 ]*vrndscaleph ymm6,WORD BCST \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 08 71 7f 7b[ 	 ]*vrndscaleph ymm6,YMMWORD PTR \[ecx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c bf 08 72 80 7b[ 	 ]*vrndscaleph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4e f5[ 	 ]*vrsqrtph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 8f 4e f5[ 	 ]*vrsqrtph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4e f5[ 	 ]*vrsqrtph ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d af 4e f5[ 	 ]*vrsqrtph ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 0f 4e b4 f4 00 00 00 10[ 	 ]*vrsqrtph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 18 4e 31[ 	 ]*vrsqrtph xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 08 4e 71 7f[ 	 ]*vrsqrtph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 9f 4e 72 80[ 	 ]*vrsqrtph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 2f 4e b4 f4 00 00 00 10[ 	 ]*vrsqrtph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 38 4e 31[ 	 ]*vrsqrtph ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d 28 4e 71 7f[ 	 ]*vrsqrtph ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 7d bf 4e 72 80[ 	 ]*vrsqrtph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 2c f4[ 	 ]*vscalefph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 af 2c f4[ 	 ]*vscalefph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2c f4[ 	 ]*vscalefph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 8f 2c f4[ 	 ]*vscalefph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 2f 2c b4 f4 00 00 00 10[ 	 ]*vscalefph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 38 2c 31[ 	 ]*vscalefph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 28 2c 71 7f[ 	 ]*vscalefph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 bf 2c 72 80[ 	 ]*vscalefph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 0f 2c b4 f4 00 00 00 10[ 	 ]*vscalefph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 18 2c 31[ 	 ]*vscalefph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 08 2c 71 7f[ 	 ]*vscalefph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f6 55 9f 2c 72 80[ 	 ]*vscalefph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 51 f5[ 	 ]*vsqrtph xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 8f 51 f5[ 	 ]*vsqrtph xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 51 f5[ 	 ]*vsqrtph ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c af 51 f5[ 	 ]*vsqrtph ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 0f 51 b4 f4 00 00 00 10[ 	 ]*vsqrtph xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 18 51 31[ 	 ]*vsqrtph xmm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 08 51 71 7f[ 	 ]*vsqrtph xmm6,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 9f 51 72 80[ 	 ]*vsqrtph xmm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 2f 51 b4 f4 00 00 00 10[ 	 ]*vsqrtph ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 38 51 31[ 	 ]*vsqrtph ymm6,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c 28 51 71 7f[ 	 ]*vsqrtph ymm6,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 7c bf 51 72 80[ 	 ]*vsqrtph ymm6\{k7\}\{z\},WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5c f4[ 	 ]*vsubph ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 af 5c f4[ 	 ]*vsubph ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5c f4[ 	 ]*vsubph xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 8f 5c f4[ 	 ]*vsubph xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 2f 5c b4 f4 00 00 00 10[ 	 ]*vsubph ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 38 5c 31[ 	 ]*vsubph ymm6,ymm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 28 5c 71 7f[ 	 ]*vsubph ymm6,ymm5,YMMWORD PTR \[ecx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 bf 5c 72 80[ 	 ]*vsubph ymm6\{k7\}\{z\},ymm5,WORD BCST \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 0f 5c b4 f4 00 00 00 10[ 	 ]*vsubph xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 18 5c 31[ 	 ]*vsubph xmm6,xmm5,WORD BCST \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 08 5c 71 7f[ 	 ]*vsubph xmm6,xmm5,XMMWORD PTR \[ecx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f5 54 9f 5c 72 80[ 	 ]*vsubph xmm6\{k7\}\{z\},xmm5,WORD BCST \[edx-0x100\]
#pass
