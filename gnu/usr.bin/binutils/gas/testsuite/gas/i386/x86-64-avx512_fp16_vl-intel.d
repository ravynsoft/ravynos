#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512-FP16,AVX512VL insns (Intel disassembly)
#source: x86-64-avx512_fp16_vl.s

.*: +file format .*

Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 05 14 20 58 f4[ 	 ]*vaddph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 a7 58 f4[ 	 ]*vaddph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 00 58 f4[ 	 ]*vaddph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 87 58 f4[ 	 ]*vaddph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 25 14 27 58 b4 f5 00 00 00 10[ 	 ]*vaddph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 30 58 31[ 	 ]*vaddph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 20 58 71 7f[ 	 ]*vaddph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 b7 58 72 80[ 	 ]*vaddph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 14 07 58 b4 f5 00 00 00 10[ 	 ]*vaddph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 10 58 31[ 	 ]*vaddph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 00 58 71 7f[ 	 ]*vaddph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 97 58 72 80[ 	 ]*vaddph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 93 14 20 c2 ec 7b[ 	 ]*vcmpph k5,ymm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 14 27 c2 ec 7b[ 	 ]*vcmpph k5\{k7\},ymm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 14 00 c2 ec 7b[ 	 ]*vcmpph k5,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 14 07 c2 ec 7b[ 	 ]*vcmpph k5\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 14 07 c2 ac f5 00 00 00 10 7b[ 	 ]*vcmpph k5\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 d3 14 10 c2 29 7b[ 	 ]*vcmpph k5,xmm29,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 14 00 c2 69 7f 7b[ 	 ]*vcmpph k5,xmm29,XMMWORD PTR \[rcx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 14 17 c2 6a 80 7b[ 	 ]*vcmpph k5\{k7\},xmm29,WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 14 27 c2 ac f5 00 00 00 10 7b[ 	 ]*vcmpph k5\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 d3 14 30 c2 29 7b[ 	 ]*vcmpph k5,ymm29,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 14 20 c2 69 7f 7b[ 	 ]*vcmpph k5,ymm29,YMMWORD PTR \[rcx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 14 37 c2 6a 80 7b[ 	 ]*vcmpph k5\{k7\},ymm29,WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 5b f5[ 	 ]*vcvtdq2ph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 8f 5b f5[ 	 ]*vcvtdq2ph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 28 5b f5[ 	 ]*vcvtdq2ph xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c af 5b f5[ 	 ]*vcvtdq2ph xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 0f 5b b4 f5 00 00 00 10[ 	 ]*vcvtdq2ph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 18 5b 31[ 	 ]*vcvtdq2ph xmm30,DWORD BCST \[r9\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 5b 71 7f[ 	 ]*vcvtdq2ph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 9f 5b 72 80[ 	 ]*vcvtdq2ph xmm30\{k7\}\{z\},DWORD BCST \[rdx-0x200\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 38 5b 31[ 	 ]*vcvtdq2ph xmm30,DWORD BCST \[r9\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 28 5b 71 7f[ 	 ]*vcvtdq2ph xmm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c bf 5b 72 80[ 	 ]*vcvtdq2ph xmm30\{k7\}\{z\},DWORD BCST \[rdx-0x200\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 05 fd 08 5a f5[ 	 ]*vcvtpd2ph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 fd 8f 5a f5[ 	 ]*vcvtpd2ph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 fd 28 5a f5[ 	 ]*vcvtpd2ph xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 fd af 5a f5[ 	 ]*vcvtpd2ph xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 fd 0f 5a b4 f5 00 00 00 10[ 	 ]*vcvtpd2ph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 fd 18 5a 31[ 	 ]*vcvtpd2ph xmm30,QWORD BCST \[r9\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 65 fd 08 5a 71 7f[ 	 ]*vcvtpd2ph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 fd 9f 5a 72 80[ 	 ]*vcvtpd2ph xmm30\{k7\}\{z\},QWORD BCST \[rdx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 45 fd 38 5a 31[ 	 ]*vcvtpd2ph xmm30,QWORD BCST \[r9\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 65 fd 28 5a 71 7f[ 	 ]*vcvtpd2ph xmm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 fd bf 5a 72 80[ 	 ]*vcvtpd2ph xmm30\{k7\}\{z\},QWORD BCST \[rdx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 5b f5[ 	 ]*vcvtph2dq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 5b f5[ 	 ]*vcvtph2dq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 5b f5[ 	 ]*vcvtph2dq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 5b f5[ 	 ]*vcvtph2dq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 5b b4 f5 00 00 00 10[ 	 ]*vcvtph2dq xmm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 5b 31[ 	 ]*vcvtph2dq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 5b 71 7f[ 	 ]*vcvtph2dq xmm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 5b 72 80[ 	 ]*vcvtph2dq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 2f 5b b4 f5 00 00 00 10[ 	 ]*vcvtph2dq ymm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 5b 31[ 	 ]*vcvtph2dq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 5b 71 7f[ 	 ]*vcvtph2dq ymm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 5b 72 80[ 	 ]*vcvtph2dq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 5a f5[ 	 ]*vcvtph2pd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 8f 5a f5[ 	 ]*vcvtph2pd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 28 5a f5[ 	 ]*vcvtph2pd ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c af 5a f5[ 	 ]*vcvtph2pd ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 0f 5a b4 f5 00 00 00 10[ 	 ]*vcvtph2pd xmm30\{k7\},DWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 18 5a 31[ 	 ]*vcvtph2pd xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 5a 71 7f[ 	 ]*vcvtph2pd xmm30,DWORD PTR \[rcx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 9f 5a 72 80[ 	 ]*vcvtph2pd xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 2f 5a b4 f5 00 00 00 10[ 	 ]*vcvtph2pd ymm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 38 5a 31[ 	 ]*vcvtph2pd ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 28 5a 71 7f[ 	 ]*vcvtph2pd ymm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c bf 5a 72 80[ 	 ]*vcvtph2pd ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 08 13 f5[ 	 ]*vcvtph2psx xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 8f 13 f5[ 	 ]*vcvtph2psx xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 28 13 f5[ 	 ]*vcvtph2psx ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d af 13 f5[ 	 ]*vcvtph2psx ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 0f 13 b4 f5 00 00 00 10[ 	 ]*vcvtph2psx xmm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 18 13 31[ 	 ]*vcvtph2psx xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 08 13 71 7f[ 	 ]*vcvtph2psx xmm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 9f 13 72 80[ 	 ]*vcvtph2psx xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 2f 13 b4 f5 00 00 00 10[ 	 ]*vcvtph2psx ymm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 38 13 31[ 	 ]*vcvtph2psx ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 28 13 71 7f[ 	 ]*vcvtph2psx ymm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d bf 13 72 80[ 	 ]*vcvtph2psx ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 7b f5[ 	 ]*vcvtph2qq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 7b f5[ 	 ]*vcvtph2qq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 7b f5[ 	 ]*vcvtph2qq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 7b f5[ 	 ]*vcvtph2qq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 7b b4 f5 00 00 00 10[ 	 ]*vcvtph2qq xmm30\{k7\},DWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 7b 31[ 	 ]*vcvtph2qq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 7b 71 7f[ 	 ]*vcvtph2qq xmm30,DWORD PTR \[rcx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 7b 72 80[ 	 ]*vcvtph2qq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 2f 7b b4 f5 00 00 00 10[ 	 ]*vcvtph2qq ymm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 7b 31[ 	 ]*vcvtph2qq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 7b 71 7f[ 	 ]*vcvtph2qq ymm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 7b 72 80[ 	 ]*vcvtph2qq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 79 f5[ 	 ]*vcvtph2udq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 8f 79 f5[ 	 ]*vcvtph2udq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 28 79 f5[ 	 ]*vcvtph2udq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c af 79 f5[ 	 ]*vcvtph2udq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 0f 79 b4 f5 00 00 00 10[ 	 ]*vcvtph2udq xmm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 18 79 31[ 	 ]*vcvtph2udq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 79 71 7f[ 	 ]*vcvtph2udq xmm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 9f 79 72 80[ 	 ]*vcvtph2udq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 2f 79 b4 f5 00 00 00 10[ 	 ]*vcvtph2udq ymm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 38 79 31[ 	 ]*vcvtph2udq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 28 79 71 7f[ 	 ]*vcvtph2udq ymm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c bf 79 72 80[ 	 ]*vcvtph2udq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 79 f5[ 	 ]*vcvtph2uqq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 79 f5[ 	 ]*vcvtph2uqq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 79 f5[ 	 ]*vcvtph2uqq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 79 f5[ 	 ]*vcvtph2uqq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 79 b4 f5 00 00 00 10[ 	 ]*vcvtph2uqq xmm30\{k7\},DWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 79 31[ 	 ]*vcvtph2uqq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 79 71 7f[ 	 ]*vcvtph2uqq xmm30,DWORD PTR \[rcx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 79 72 80[ 	 ]*vcvtph2uqq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 2f 79 b4 f5 00 00 00 10[ 	 ]*vcvtph2uqq ymm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 79 31[ 	 ]*vcvtph2uqq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 79 71 7f[ 	 ]*vcvtph2uqq ymm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 79 72 80[ 	 ]*vcvtph2uqq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 7d f5[ 	 ]*vcvtph2uw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 8f 7d f5[ 	 ]*vcvtph2uw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 28 7d f5[ 	 ]*vcvtph2uw ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c af 7d f5[ 	 ]*vcvtph2uw ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 0f 7d b4 f5 00 00 00 10[ 	 ]*vcvtph2uw xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 18 7d 31[ 	 ]*vcvtph2uw xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 7d 71 7f[ 	 ]*vcvtph2uw xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 9f 7d 72 80[ 	 ]*vcvtph2uw xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 2f 7d b4 f5 00 00 00 10[ 	 ]*vcvtph2uw ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 38 7d 31[ 	 ]*vcvtph2uw ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 28 7d 71 7f[ 	 ]*vcvtph2uw ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c bf 7d 72 80[ 	 ]*vcvtph2uw ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 7d f5[ 	 ]*vcvtph2w xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 7d f5[ 	 ]*vcvtph2w xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 7d f5[ 	 ]*vcvtph2w ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 7d f5[ 	 ]*vcvtph2w ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 7d b4 f5 00 00 00 10[ 	 ]*vcvtph2w xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 7d 31[ 	 ]*vcvtph2w xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 7d 71 7f[ 	 ]*vcvtph2w xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 7d 72 80[ 	 ]*vcvtph2w xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 2f 7d b4 f5 00 00 00 10[ 	 ]*vcvtph2w ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 7d 31[ 	 ]*vcvtph2w ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 7d 71 7f[ 	 ]*vcvtph2w ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 7d 72 80[ 	 ]*vcvtph2w ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 1d f5[ 	 ]*vcvtps2phx xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 1d f5[ 	 ]*vcvtps2phx xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 1d f5[ 	 ]*vcvtps2phx xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 1d f5[ 	 ]*vcvtps2phx xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 1d b4 f5 00 00 00 10[ 	 ]*vcvtps2phx xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 1d 31[ 	 ]*vcvtps2phx xmm30,DWORD BCST \[r9\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 1d 71 7f[ 	 ]*vcvtps2phx xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 1d 72 80[ 	 ]*vcvtps2phx xmm30\{k7\}\{z\},DWORD BCST \[rdx-0x200\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 1d 31[ 	 ]*vcvtps2phx xmm30,DWORD BCST \[r9\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 1d 71 7f[ 	 ]*vcvtps2phx xmm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 1d 72 80[ 	 ]*vcvtps2phx xmm30\{k7\}\{z\},DWORD BCST \[rdx-0x200\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 05 fc 08 5b f5[ 	 ]*vcvtqq2ph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 fc 8f 5b f5[ 	 ]*vcvtqq2ph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 fc 28 5b f5[ 	 ]*vcvtqq2ph xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 fc af 5b f5[ 	 ]*vcvtqq2ph xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 fc 0f 5b b4 f5 00 00 00 10[ 	 ]*vcvtqq2ph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 fc 18 5b 31[ 	 ]*vcvtqq2ph xmm30,QWORD BCST \[r9\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 65 fc 08 5b 71 7f[ 	 ]*vcvtqq2ph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 fc 9f 5b 72 80[ 	 ]*vcvtqq2ph xmm30\{k7\}\{z\},QWORD BCST \[rdx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 45 fc 38 5b 31[ 	 ]*vcvtqq2ph xmm30,QWORD BCST \[r9\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 65 fc 28 5b 71 7f[ 	 ]*vcvtqq2ph xmm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 fc bf 5b 72 80[ 	 ]*vcvtqq2ph xmm30\{k7\}\{z\},QWORD BCST \[rdx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 08 5b f5[ 	 ]*vcvttph2dq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 8f 5b f5[ 	 ]*vcvttph2dq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 28 5b f5[ 	 ]*vcvttph2dq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7e af 5b f5[ 	 ]*vcvttph2dq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 0f 5b b4 f5 00 00 00 10[ 	 ]*vcvttph2dq xmm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 18 5b 31[ 	 ]*vcvttph2dq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 08 5b 71 7f[ 	 ]*vcvttph2dq xmm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 9f 5b 72 80[ 	 ]*vcvttph2dq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 2f 5b b4 f5 00 00 00 10[ 	 ]*vcvttph2dq ymm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 38 5b 31[ 	 ]*vcvttph2dq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 28 5b 71 7f[ 	 ]*vcvttph2dq ymm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e bf 5b 72 80[ 	 ]*vcvttph2dq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 7a f5[ 	 ]*vcvttph2qq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 7a f5[ 	 ]*vcvttph2qq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 7a f5[ 	 ]*vcvttph2qq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 7a f5[ 	 ]*vcvttph2qq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 7a b4 f5 00 00 00 10[ 	 ]*vcvttph2qq xmm30\{k7\},DWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 7a 31[ 	 ]*vcvttph2qq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 7a 71 7f[ 	 ]*vcvttph2qq xmm30,DWORD PTR \[rcx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 7a 72 80[ 	 ]*vcvttph2qq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 2f 7a b4 f5 00 00 00 10[ 	 ]*vcvttph2qq ymm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 7a 31[ 	 ]*vcvttph2qq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 7a 71 7f[ 	 ]*vcvttph2qq ymm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 7a 72 80[ 	 ]*vcvttph2qq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 78 f5[ 	 ]*vcvttph2udq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 8f 78 f5[ 	 ]*vcvttph2udq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 28 78 f5[ 	 ]*vcvttph2udq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c af 78 f5[ 	 ]*vcvttph2udq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 0f 78 b4 f5 00 00 00 10[ 	 ]*vcvttph2udq xmm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 18 78 31[ 	 ]*vcvttph2udq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 78 71 7f[ 	 ]*vcvttph2udq xmm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 9f 78 72 80[ 	 ]*vcvttph2udq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 2f 78 b4 f5 00 00 00 10[ 	 ]*vcvttph2udq ymm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 38 78 31[ 	 ]*vcvttph2udq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 28 78 71 7f[ 	 ]*vcvttph2udq ymm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c bf 78 72 80[ 	 ]*vcvttph2udq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 78 f5[ 	 ]*vcvttph2uqq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 78 f5[ 	 ]*vcvttph2uqq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 78 f5[ 	 ]*vcvttph2uqq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 78 f5[ 	 ]*vcvttph2uqq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 78 b4 f5 00 00 00 10[ 	 ]*vcvttph2uqq xmm30\{k7\},DWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 78 31[ 	 ]*vcvttph2uqq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 78 71 7f[ 	 ]*vcvttph2uqq xmm30,DWORD PTR \[rcx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 78 72 80[ 	 ]*vcvttph2uqq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 2f 78 b4 f5 00 00 00 10[ 	 ]*vcvttph2uqq ymm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 78 31[ 	 ]*vcvttph2uqq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 78 71 7f[ 	 ]*vcvttph2uqq ymm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 78 72 80[ 	 ]*vcvttph2uqq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 7c f5[ 	 ]*vcvttph2uw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 8f 7c f5[ 	 ]*vcvttph2uw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 28 7c f5[ 	 ]*vcvttph2uw ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c af 7c f5[ 	 ]*vcvttph2uw ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 0f 7c b4 f5 00 00 00 10[ 	 ]*vcvttph2uw xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 18 7c 31[ 	 ]*vcvttph2uw xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 7c 71 7f[ 	 ]*vcvttph2uw xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 9f 7c 72 80[ 	 ]*vcvttph2uw xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 2f 7c b4 f5 00 00 00 10[ 	 ]*vcvttph2uw ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 38 7c 31[ 	 ]*vcvttph2uw ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 28 7c 71 7f[ 	 ]*vcvttph2uw ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c bf 7c 72 80[ 	 ]*vcvttph2uw ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 7c f5[ 	 ]*vcvttph2w xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 7c f5[ 	 ]*vcvttph2w xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 7c f5[ 	 ]*vcvttph2w ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 7c f5[ 	 ]*vcvttph2w ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 7c b4 f5 00 00 00 10[ 	 ]*vcvttph2w xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 7c 31[ 	 ]*vcvttph2w xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 7c 71 7f[ 	 ]*vcvttph2w xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 7c 72 80[ 	 ]*vcvttph2w xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 2f 7c b4 f5 00 00 00 10[ 	 ]*vcvttph2w ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 7c 31[ 	 ]*vcvttph2w ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 7c 71 7f[ 	 ]*vcvttph2w ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 7c 72 80[ 	 ]*vcvttph2w ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 08 7a f5[ 	 ]*vcvtudq2ph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 8f 7a f5[ 	 ]*vcvtudq2ph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 28 7a f5[ 	 ]*vcvtudq2ph xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7f af 7a f5[ 	 ]*vcvtudq2ph xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7f 0f 7a b4 f5 00 00 00 10[ 	 ]*vcvtudq2ph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7f 18 7a 31[ 	 ]*vcvtudq2ph xmm30,DWORD BCST \[r9\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 08 7a 71 7f[ 	 ]*vcvtudq2ph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 9f 7a 72 80[ 	 ]*vcvtudq2ph xmm30\{k7\}\{z\},DWORD BCST \[rdx-0x200\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7f 38 7a 31[ 	 ]*vcvtudq2ph xmm30,DWORD BCST \[r9\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 28 7a 71 7f[ 	 ]*vcvtudq2ph xmm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7f bf 7a 72 80[ 	 ]*vcvtudq2ph xmm30\{k7\}\{z\},DWORD BCST \[rdx-0x200\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 05 ff 08 7a f5[ 	 ]*vcvtuqq2ph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 ff 8f 7a f5[ 	 ]*vcvtuqq2ph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 ff 28 7a f5[ 	 ]*vcvtuqq2ph xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 ff af 7a f5[ 	 ]*vcvtuqq2ph xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 ff 0f 7a b4 f5 00 00 00 10[ 	 ]*vcvtuqq2ph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 ff 18 7a 31[ 	 ]*vcvtuqq2ph xmm30,QWORD BCST \[r9\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 65 ff 08 7a 71 7f[ 	 ]*vcvtuqq2ph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 ff 9f 7a 72 80[ 	 ]*vcvtuqq2ph xmm30\{k7\}\{z\},QWORD BCST \[rdx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 45 ff 38 7a 31[ 	 ]*vcvtuqq2ph xmm30,QWORD BCST \[r9\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 65 ff 28 7a 71 7f[ 	 ]*vcvtuqq2ph xmm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 ff bf 7a 72 80[ 	 ]*vcvtuqq2ph xmm30\{k7\}\{z\},QWORD BCST \[rdx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 08 7d f5[ 	 ]*vcvtuw2ph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 8f 7d f5[ 	 ]*vcvtuw2ph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 28 7d f5[ 	 ]*vcvtuw2ph ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7f af 7d f5[ 	 ]*vcvtuw2ph ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7f 0f 7d b4 f5 00 00 00 10[ 	 ]*vcvtuw2ph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7f 18 7d 31[ 	 ]*vcvtuw2ph xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 08 7d 71 7f[ 	 ]*vcvtuw2ph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 9f 7d 72 80[ 	 ]*vcvtuw2ph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7f 2f 7d b4 f5 00 00 00 10[ 	 ]*vcvtuw2ph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7f 38 7d 31[ 	 ]*vcvtuw2ph ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 28 7d 71 7f[ 	 ]*vcvtuw2ph ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7f bf 7d 72 80[ 	 ]*vcvtuw2ph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 08 7d f5[ 	 ]*vcvtw2ph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 8f 7d f5[ 	 ]*vcvtw2ph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 28 7d f5[ 	 ]*vcvtw2ph ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7e af 7d f5[ 	 ]*vcvtw2ph ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 0f 7d b4 f5 00 00 00 10[ 	 ]*vcvtw2ph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 18 7d 31[ 	 ]*vcvtw2ph xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 08 7d 71 7f[ 	 ]*vcvtw2ph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 9f 7d 72 80[ 	 ]*vcvtw2ph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 2f 7d b4 f5 00 00 00 10[ 	 ]*vcvtw2ph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 38 7d 31[ 	 ]*vcvtw2ph ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 28 7d 71 7f[ 	 ]*vcvtw2ph ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e bf 7d 72 80[ 	 ]*vcvtw2ph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 14 20 5e f4[ 	 ]*vdivph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 a7 5e f4[ 	 ]*vdivph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 00 5e f4[ 	 ]*vdivph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 87 5e f4[ 	 ]*vdivph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 25 14 27 5e b4 f5 00 00 00 10[ 	 ]*vdivph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 30 5e 31[ 	 ]*vdivph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 20 5e 71 7f[ 	 ]*vdivph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 b7 5e 72 80[ 	 ]*vdivph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 14 07 5e b4 f5 00 00 00 10[ 	 ]*vdivph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 10 5e 31[ 	 ]*vdivph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 00 5e 71 7f[ 	 ]*vdivph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 97 5e 72 80[ 	 ]*vdivph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 17 20 56 f4[ 	 ]*vfcmaddcph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 17 a7 56 f4[ 	 ]*vfcmaddcph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 17 00 56 f4[ 	 ]*vfcmaddcph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 17 87 56 f4[ 	 ]*vfcmaddcph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 17 27 56 b4 f5 00 00 00 10[ 	 ]*vfcmaddcph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 17 30 56 31[ 	 ]*vfcmaddcph ymm30,ymm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 20 56 71 7f[ 	 ]*vfcmaddcph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 b7 56 72 80[ 	 ]*vfcmaddcph ymm30\{k7\}\{z\},ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 26 17 07 56 b4 f5 00 00 00 10[ 	 ]*vfcmaddcph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 17 10 56 31[ 	 ]*vfcmaddcph xmm30,xmm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 00 56 71 7f[ 	 ]*vfcmaddcph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 97 56 72 80[ 	 ]*vfcmaddcph xmm30\{k7\}\{z\},xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 06 17 20 d6 f4[ 	 ]*vfcmulcph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 17 a7 d6 f4[ 	 ]*vfcmulcph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 17 00 d6 f4[ 	 ]*vfcmulcph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 17 87 d6 f4[ 	 ]*vfcmulcph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 17 27 d6 b4 f5 00 00 00 10[ 	 ]*vfcmulcph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 17 30 d6 31[ 	 ]*vfcmulcph ymm30,ymm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 20 d6 71 7f[ 	 ]*vfcmulcph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 b7 d6 72 80[ 	 ]*vfcmulcph ymm30\{k7\}\{z\},ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 26 17 07 d6 b4 f5 00 00 00 10[ 	 ]*vfcmulcph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 17 10 d6 31[ 	 ]*vfcmulcph xmm30,xmm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 00 d6 71 7f[ 	 ]*vfcmulcph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 97 d6 72 80[ 	 ]*vfcmulcph xmm30\{k7\}\{z\},xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 98 f4[ 	 ]*vfmadd132ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 98 f4[ 	 ]*vfmadd132ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 98 f4[ 	 ]*vfmadd132ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 98 f4[ 	 ]*vfmadd132ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 98 b4 f5 00 00 00 10[ 	 ]*vfmadd132ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 98 31[ 	 ]*vfmadd132ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 98 71 7f[ 	 ]*vfmadd132ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 98 72 80[ 	 ]*vfmadd132ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 98 b4 f5 00 00 00 10[ 	 ]*vfmadd132ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 98 31[ 	 ]*vfmadd132ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 98 71 7f[ 	 ]*vfmadd132ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 98 72 80[ 	 ]*vfmadd132ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 a8 f4[ 	 ]*vfmadd213ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 a8 f4[ 	 ]*vfmadd213ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 a8 f4[ 	 ]*vfmadd213ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 a8 f4[ 	 ]*vfmadd213ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 a8 b4 f5 00 00 00 10[ 	 ]*vfmadd213ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 a8 31[ 	 ]*vfmadd213ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 a8 71 7f[ 	 ]*vfmadd213ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 a8 72 80[ 	 ]*vfmadd213ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 a8 b4 f5 00 00 00 10[ 	 ]*vfmadd213ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 a8 31[ 	 ]*vfmadd213ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 a8 71 7f[ 	 ]*vfmadd213ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 a8 72 80[ 	 ]*vfmadd213ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 b8 f4[ 	 ]*vfmadd231ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 b8 f4[ 	 ]*vfmadd231ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 b8 f4[ 	 ]*vfmadd231ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 b8 f4[ 	 ]*vfmadd231ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 b8 b4 f5 00 00 00 10[ 	 ]*vfmadd231ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 b8 31[ 	 ]*vfmadd231ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 b8 71 7f[ 	 ]*vfmadd231ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 b8 72 80[ 	 ]*vfmadd231ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 b8 b4 f5 00 00 00 10[ 	 ]*vfmadd231ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 b8 31[ 	 ]*vfmadd231ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 b8 71 7f[ 	 ]*vfmadd231ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 b8 72 80[ 	 ]*vfmadd231ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 16 20 56 f4[ 	 ]*vfmaddcph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 16 a7 56 f4[ 	 ]*vfmaddcph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 16 00 56 f4[ 	 ]*vfmaddcph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 16 87 56 f4[ 	 ]*vfmaddcph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 16 27 56 b4 f5 00 00 00 10[ 	 ]*vfmaddcph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 16 30 56 31[ 	 ]*vfmaddcph ymm30,ymm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 20 56 71 7f[ 	 ]*vfmaddcph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 b7 56 72 80[ 	 ]*vfmaddcph ymm30\{k7\}\{z\},ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 26 16 07 56 b4 f5 00 00 00 10[ 	 ]*vfmaddcph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 16 10 56 31[ 	 ]*vfmaddcph xmm30,xmm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 00 56 71 7f[ 	 ]*vfmaddcph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 97 56 72 80[ 	 ]*vfmaddcph xmm30\{k7\}\{z\},xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 96 f4[ 	 ]*vfmaddsub132ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 96 f4[ 	 ]*vfmaddsub132ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 96 f4[ 	 ]*vfmaddsub132ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 96 f4[ 	 ]*vfmaddsub132ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 96 b4 f5 00 00 00 10[ 	 ]*vfmaddsub132ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 96 31[ 	 ]*vfmaddsub132ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 96 71 7f[ 	 ]*vfmaddsub132ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 96 72 80[ 	 ]*vfmaddsub132ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 96 b4 f5 00 00 00 10[ 	 ]*vfmaddsub132ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 96 31[ 	 ]*vfmaddsub132ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 96 71 7f[ 	 ]*vfmaddsub132ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 96 72 80[ 	 ]*vfmaddsub132ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 a6 f4[ 	 ]*vfmaddsub213ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 a6 f4[ 	 ]*vfmaddsub213ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 a6 f4[ 	 ]*vfmaddsub213ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 a6 f4[ 	 ]*vfmaddsub213ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 a6 b4 f5 00 00 00 10[ 	 ]*vfmaddsub213ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 a6 31[ 	 ]*vfmaddsub213ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 a6 71 7f[ 	 ]*vfmaddsub213ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 a6 72 80[ 	 ]*vfmaddsub213ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 a6 b4 f5 00 00 00 10[ 	 ]*vfmaddsub213ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 a6 31[ 	 ]*vfmaddsub213ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 a6 71 7f[ 	 ]*vfmaddsub213ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 a6 72 80[ 	 ]*vfmaddsub213ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 b6 f4[ 	 ]*vfmaddsub231ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 b6 f4[ 	 ]*vfmaddsub231ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 b6 f4[ 	 ]*vfmaddsub231ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 b6 f4[ 	 ]*vfmaddsub231ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 b6 b4 f5 00 00 00 10[ 	 ]*vfmaddsub231ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 b6 31[ 	 ]*vfmaddsub231ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 b6 71 7f[ 	 ]*vfmaddsub231ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 b6 72 80[ 	 ]*vfmaddsub231ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 b6 b4 f5 00 00 00 10[ 	 ]*vfmaddsub231ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 b6 31[ 	 ]*vfmaddsub231ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 b6 71 7f[ 	 ]*vfmaddsub231ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 b6 72 80[ 	 ]*vfmaddsub231ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 9a f4[ 	 ]*vfmsub132ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 9a f4[ 	 ]*vfmsub132ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 9a f4[ 	 ]*vfmsub132ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 9a f4[ 	 ]*vfmsub132ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 9a b4 f5 00 00 00 10[ 	 ]*vfmsub132ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 9a 31[ 	 ]*vfmsub132ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 9a 71 7f[ 	 ]*vfmsub132ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 9a 72 80[ 	 ]*vfmsub132ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 9a b4 f5 00 00 00 10[ 	 ]*vfmsub132ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 9a 31[ 	 ]*vfmsub132ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 9a 71 7f[ 	 ]*vfmsub132ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 9a 72 80[ 	 ]*vfmsub132ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 aa f4[ 	 ]*vfmsub213ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 aa f4[ 	 ]*vfmsub213ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 aa f4[ 	 ]*vfmsub213ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 aa f4[ 	 ]*vfmsub213ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 aa b4 f5 00 00 00 10[ 	 ]*vfmsub213ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 aa 31[ 	 ]*vfmsub213ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 aa 71 7f[ 	 ]*vfmsub213ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 aa 72 80[ 	 ]*vfmsub213ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 aa b4 f5 00 00 00 10[ 	 ]*vfmsub213ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 aa 31[ 	 ]*vfmsub213ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 aa 71 7f[ 	 ]*vfmsub213ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 aa 72 80[ 	 ]*vfmsub213ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 ba f4[ 	 ]*vfmsub231ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 ba f4[ 	 ]*vfmsub231ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 ba f4[ 	 ]*vfmsub231ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 ba f4[ 	 ]*vfmsub231ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 ba b4 f5 00 00 00 10[ 	 ]*vfmsub231ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 ba 31[ 	 ]*vfmsub231ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 ba 71 7f[ 	 ]*vfmsub231ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 ba 72 80[ 	 ]*vfmsub231ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 ba b4 f5 00 00 00 10[ 	 ]*vfmsub231ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 ba 31[ 	 ]*vfmsub231ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 ba 71 7f[ 	 ]*vfmsub231ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 ba 72 80[ 	 ]*vfmsub231ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 97 f4[ 	 ]*vfmsubadd132ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 97 f4[ 	 ]*vfmsubadd132ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 97 f4[ 	 ]*vfmsubadd132ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 97 f4[ 	 ]*vfmsubadd132ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 97 b4 f5 00 00 00 10[ 	 ]*vfmsubadd132ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 97 31[ 	 ]*vfmsubadd132ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 97 71 7f[ 	 ]*vfmsubadd132ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 97 72 80[ 	 ]*vfmsubadd132ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 97 b4 f5 00 00 00 10[ 	 ]*vfmsubadd132ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 97 31[ 	 ]*vfmsubadd132ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 97 71 7f[ 	 ]*vfmsubadd132ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 97 72 80[ 	 ]*vfmsubadd132ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 a7 f4[ 	 ]*vfmsubadd213ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 a7 f4[ 	 ]*vfmsubadd213ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 a7 f4[ 	 ]*vfmsubadd213ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 a7 f4[ 	 ]*vfmsubadd213ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 a7 b4 f5 00 00 00 10[ 	 ]*vfmsubadd213ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 a7 31[ 	 ]*vfmsubadd213ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 a7 71 7f[ 	 ]*vfmsubadd213ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 a7 72 80[ 	 ]*vfmsubadd213ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 a7 b4 f5 00 00 00 10[ 	 ]*vfmsubadd213ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 a7 31[ 	 ]*vfmsubadd213ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 a7 71 7f[ 	 ]*vfmsubadd213ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 a7 72 80[ 	 ]*vfmsubadd213ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 b7 f4[ 	 ]*vfmsubadd231ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 b7 f4[ 	 ]*vfmsubadd231ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 b7 f4[ 	 ]*vfmsubadd231ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 b7 f4[ 	 ]*vfmsubadd231ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 b7 b4 f5 00 00 00 10[ 	 ]*vfmsubadd231ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 b7 31[ 	 ]*vfmsubadd231ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 b7 71 7f[ 	 ]*vfmsubadd231ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 b7 72 80[ 	 ]*vfmsubadd231ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 b7 b4 f5 00 00 00 10[ 	 ]*vfmsubadd231ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 b7 31[ 	 ]*vfmsubadd231ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 b7 71 7f[ 	 ]*vfmsubadd231ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 b7 72 80[ 	 ]*vfmsubadd231ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 16 20 d6 f4[ 	 ]*vfmulcph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 16 a7 d6 f4[ 	 ]*vfmulcph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 16 00 d6 f4[ 	 ]*vfmulcph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 16 87 d6 f4[ 	 ]*vfmulcph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 16 27 d6 b4 f5 00 00 00 10[ 	 ]*vfmulcph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 16 30 d6 31[ 	 ]*vfmulcph ymm30,ymm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 20 d6 71 7f[ 	 ]*vfmulcph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 b7 d6 72 80[ 	 ]*vfmulcph ymm30\{k7\}\{z\},ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 26 16 07 d6 b4 f5 00 00 00 10[ 	 ]*vfmulcph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 16 10 d6 31[ 	 ]*vfmulcph xmm30,xmm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 00 d6 71 7f[ 	 ]*vfmulcph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 97 d6 72 80[ 	 ]*vfmulcph xmm30\{k7\}\{z\},xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 9c f4[ 	 ]*vfnmadd132ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 9c f4[ 	 ]*vfnmadd132ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 9c f4[ 	 ]*vfnmadd132ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 9c f4[ 	 ]*vfnmadd132ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 9c b4 f5 00 00 00 10[ 	 ]*vfnmadd132ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 9c 31[ 	 ]*vfnmadd132ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 9c 71 7f[ 	 ]*vfnmadd132ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 9c 72 80[ 	 ]*vfnmadd132ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 9c b4 f5 00 00 00 10[ 	 ]*vfnmadd132ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 9c 31[ 	 ]*vfnmadd132ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 9c 71 7f[ 	 ]*vfnmadd132ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 9c 72 80[ 	 ]*vfnmadd132ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 ac f4[ 	 ]*vfnmadd213ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 ac f4[ 	 ]*vfnmadd213ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 ac f4[ 	 ]*vfnmadd213ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 ac f4[ 	 ]*vfnmadd213ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 ac b4 f5 00 00 00 10[ 	 ]*vfnmadd213ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 ac 31[ 	 ]*vfnmadd213ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 ac 71 7f[ 	 ]*vfnmadd213ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 ac 72 80[ 	 ]*vfnmadd213ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 ac b4 f5 00 00 00 10[ 	 ]*vfnmadd213ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 ac 31[ 	 ]*vfnmadd213ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 ac 71 7f[ 	 ]*vfnmadd213ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 ac 72 80[ 	 ]*vfnmadd213ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 bc f4[ 	 ]*vfnmadd231ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 bc f4[ 	 ]*vfnmadd231ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 bc f4[ 	 ]*vfnmadd231ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 bc f4[ 	 ]*vfnmadd231ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 bc b4 f5 00 00 00 10[ 	 ]*vfnmadd231ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 bc 31[ 	 ]*vfnmadd231ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 bc 71 7f[ 	 ]*vfnmadd231ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 bc 72 80[ 	 ]*vfnmadd231ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 bc b4 f5 00 00 00 10[ 	 ]*vfnmadd231ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 bc 31[ 	 ]*vfnmadd231ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 bc 71 7f[ 	 ]*vfnmadd231ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 bc 72 80[ 	 ]*vfnmadd231ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 9e f4[ 	 ]*vfnmsub132ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 9e f4[ 	 ]*vfnmsub132ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 9e f4[ 	 ]*vfnmsub132ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 9e f4[ 	 ]*vfnmsub132ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 9e b4 f5 00 00 00 10[ 	 ]*vfnmsub132ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 9e 31[ 	 ]*vfnmsub132ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 9e 71 7f[ 	 ]*vfnmsub132ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 9e 72 80[ 	 ]*vfnmsub132ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 9e b4 f5 00 00 00 10[ 	 ]*vfnmsub132ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 9e 31[ 	 ]*vfnmsub132ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 9e 71 7f[ 	 ]*vfnmsub132ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 9e 72 80[ 	 ]*vfnmsub132ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 ae f4[ 	 ]*vfnmsub213ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 ae f4[ 	 ]*vfnmsub213ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 ae f4[ 	 ]*vfnmsub213ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 ae f4[ 	 ]*vfnmsub213ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 ae b4 f5 00 00 00 10[ 	 ]*vfnmsub213ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 ae 31[ 	 ]*vfnmsub213ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 ae 71 7f[ 	 ]*vfnmsub213ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 ae 72 80[ 	 ]*vfnmsub213ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 ae b4 f5 00 00 00 10[ 	 ]*vfnmsub213ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 ae 31[ 	 ]*vfnmsub213ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 ae 71 7f[ 	 ]*vfnmsub213ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 ae 72 80[ 	 ]*vfnmsub213ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 be f4[ 	 ]*vfnmsub231ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 be f4[ 	 ]*vfnmsub231ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 be f4[ 	 ]*vfnmsub231ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 be f4[ 	 ]*vfnmsub231ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 be b4 f5 00 00 00 10[ 	 ]*vfnmsub231ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 be 31[ 	 ]*vfnmsub231ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 be 71 7f[ 	 ]*vfnmsub231ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 be 72 80[ 	 ]*vfnmsub231ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 be b4 f5 00 00 00 10[ 	 ]*vfnmsub231ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 be 31[ 	 ]*vfnmsub231ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 be 71 7f[ 	 ]*vfnmsub231ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 be 72 80[ 	 ]*vfnmsub231ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 08 66 ee 7b[ 	 ]*vfpclassph k5,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 0f 66 ee 7b[ 	 ]*vfpclassph k5\{k7\},xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 28 66 ee 7b[ 	 ]*vfpclassph k5,ymm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 2f 66 ee 7b[ 	 ]*vfpclassph k5\{k7\},ymm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 7c 0f 66 ac f5 00 00 00 10 7b[ 	 ]*vfpclassph k5\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 d3 7c 18 66 29 7b[ 	 ]*vfpclassph k5,WORD BCST \[r9\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 66 69 7f 7b[ 	 ]*vfpclassph k5,XMMWORD PTR \[rcx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 1f 66 6a 80 7b[ 	 ]*vfpclassph k5\{k7\},WORD BCST \[rdx-0x100\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 d3 7c 38 66 29 7b[ 	 ]*vfpclassph k5,WORD BCST \[r9\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 66 69 7f 7b[ 	 ]*vfpclassph k5,YMMWORD PTR \[rcx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 3f 66 6a 80 7b[ 	 ]*vfpclassph k5\{k7\},WORD BCST \[rdx-0x100\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 08 42 f5[ 	 ]*vgetexpph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 8f 42 f5[ 	 ]*vgetexpph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 28 42 f5[ 	 ]*vgetexpph ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d af 42 f5[ 	 ]*vgetexpph ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 0f 42 b4 f5 00 00 00 10[ 	 ]*vgetexpph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 18 42 31[ 	 ]*vgetexpph xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 08 42 71 7f[ 	 ]*vgetexpph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 9f 42 72 80[ 	 ]*vgetexpph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 2f 42 b4 f5 00 00 00 10[ 	 ]*vgetexpph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 38 42 31[ 	 ]*vgetexpph ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 28 42 71 7f[ 	 ]*vgetexpph ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d bf 42 72 80[ 	 ]*vgetexpph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 28 26 f5 7b[ 	 ]*vgetmantph ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c af 26 f5 7b[ 	 ]*vgetmantph ymm30\{k7\}\{z\},ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 08 26 f5 7b[ 	 ]*vgetmantph xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 8f 26 f5 7b[ 	 ]*vgetmantph xmm30\{k7\}\{z\},xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 0f 26 b4 f5 00 00 00 10 7b[ 	 ]*vgetmantph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 18 26 31 7b[ 	 ]*vgetmantph xmm30,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 08 26 71 7f 7b[ 	 ]*vgetmantph xmm30,XMMWORD PTR \[rcx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 9f 26 72 80 7b[ 	 ]*vgetmantph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 2f 26 b4 f5 00 00 00 10 7b[ 	 ]*vgetmantph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 38 26 31 7b[ 	 ]*vgetmantph ymm30,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 28 26 71 7f 7b[ 	 ]*vgetmantph ymm30,YMMWORD PTR \[rcx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c bf 26 72 80 7b[ 	 ]*vgetmantph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 05 14 20 5f f4[ 	 ]*vmaxph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 a7 5f f4[ 	 ]*vmaxph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 00 5f f4[ 	 ]*vmaxph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 87 5f f4[ 	 ]*vmaxph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 25 14 27 5f b4 f5 00 00 00 10[ 	 ]*vmaxph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 30 5f 31[ 	 ]*vmaxph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 20 5f 71 7f[ 	 ]*vmaxph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 b7 5f 72 80[ 	 ]*vmaxph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 14 07 5f b4 f5 00 00 00 10[ 	 ]*vmaxph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 10 5f 31[ 	 ]*vmaxph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 00 5f 71 7f[ 	 ]*vmaxph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 97 5f 72 80[ 	 ]*vmaxph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 14 20 5d f4[ 	 ]*vminph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 a7 5d f4[ 	 ]*vminph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 00 5d f4[ 	 ]*vminph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 87 5d f4[ 	 ]*vminph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 25 14 27 5d b4 f5 00 00 00 10[ 	 ]*vminph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 30 5d 31[ 	 ]*vminph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 20 5d 71 7f[ 	 ]*vminph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 b7 5d 72 80[ 	 ]*vminph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 14 07 5d b4 f5 00 00 00 10[ 	 ]*vminph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 10 5d 31[ 	 ]*vminph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 00 5d 71 7f[ 	 ]*vminph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 97 5d 72 80[ 	 ]*vminph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 14 20 59 f4[ 	 ]*vmulph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 a7 59 f4[ 	 ]*vmulph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 00 59 f4[ 	 ]*vmulph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 87 59 f4[ 	 ]*vmulph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 25 14 27 59 b4 f5 00 00 00 10[ 	 ]*vmulph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 30 59 31[ 	 ]*vmulph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 20 59 71 7f[ 	 ]*vmulph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 b7 59 72 80[ 	 ]*vmulph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 14 07 59 b4 f5 00 00 00 10[ 	 ]*vmulph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 10 59 31[ 	 ]*vmulph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 00 59 71 7f[ 	 ]*vmulph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 97 59 72 80[ 	 ]*vmulph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 08 4c f5[ 	 ]*vrcpph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 8f 4c f5[ 	 ]*vrcpph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 28 4c f5[ 	 ]*vrcpph ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d af 4c f5[ 	 ]*vrcpph ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 0f 4c b4 f5 00 00 00 10[ 	 ]*vrcpph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 18 4c 31[ 	 ]*vrcpph xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 08 4c 71 7f[ 	 ]*vrcpph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 9f 4c 72 80[ 	 ]*vrcpph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 2f 4c b4 f5 00 00 00 10[ 	 ]*vrcpph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 38 4c 31[ 	 ]*vrcpph ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 28 4c 71 7f[ 	 ]*vrcpph ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d bf 4c 72 80[ 	 ]*vrcpph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 28 56 f5 7b[ 	 ]*vreduceph ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c af 56 f5 7b[ 	 ]*vreduceph ymm30\{k7\}\{z\},ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 08 56 f5 7b[ 	 ]*vreduceph xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 8f 56 f5 7b[ 	 ]*vreduceph xmm30\{k7\}\{z\},xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 0f 56 b4 f5 00 00 00 10 7b[ 	 ]*vreduceph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 18 56 31 7b[ 	 ]*vreduceph xmm30,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 08 56 71 7f 7b[ 	 ]*vreduceph xmm30,XMMWORD PTR \[rcx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 9f 56 72 80 7b[ 	 ]*vreduceph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 2f 56 b4 f5 00 00 00 10 7b[ 	 ]*vreduceph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 38 56 31 7b[ 	 ]*vreduceph ymm30,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 28 56 71 7f 7b[ 	 ]*vreduceph ymm30,YMMWORD PTR \[rcx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c bf 56 72 80 7b[ 	 ]*vreduceph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 28 08 f5 7b[ 	 ]*vrndscaleph ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c af 08 f5 7b[ 	 ]*vrndscaleph ymm30\{k7\}\{z\},ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 08 08 f5 7b[ 	 ]*vrndscaleph xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 8f 08 f5 7b[ 	 ]*vrndscaleph xmm30\{k7\}\{z\},xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 0f 08 b4 f5 00 00 00 10 7b[ 	 ]*vrndscaleph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 18 08 31 7b[ 	 ]*vrndscaleph xmm30,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 08 08 71 7f 7b[ 	 ]*vrndscaleph xmm30,XMMWORD PTR \[rcx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 9f 08 72 80 7b[ 	 ]*vrndscaleph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 2f 08 b4 f5 00 00 00 10 7b[ 	 ]*vrndscaleph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 38 08 31 7b[ 	 ]*vrndscaleph ymm30,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 28 08 71 7f 7b[ 	 ]*vrndscaleph ymm30,YMMWORD PTR \[rcx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c bf 08 72 80 7b[ 	 ]*vrndscaleph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 08 4e f5[ 	 ]*vrsqrtph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 8f 4e f5[ 	 ]*vrsqrtph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 28 4e f5[ 	 ]*vrsqrtph ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d af 4e f5[ 	 ]*vrsqrtph ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 0f 4e b4 f5 00 00 00 10[ 	 ]*vrsqrtph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 18 4e 31[ 	 ]*vrsqrtph xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 08 4e 71 7f[ 	 ]*vrsqrtph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 9f 4e 72 80[ 	 ]*vrsqrtph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 2f 4e b4 f5 00 00 00 10[ 	 ]*vrsqrtph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 38 4e 31[ 	 ]*vrsqrtph ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 28 4e 71 7f[ 	 ]*vrsqrtph ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d bf 4e 72 80[ 	 ]*vrsqrtph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 2c f4[ 	 ]*vscalefph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 2c f4[ 	 ]*vscalefph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 2c f4[ 	 ]*vscalefph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 2c f4[ 	 ]*vscalefph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 2c b4 f5 00 00 00 10[ 	 ]*vscalefph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 2c 31[ 	 ]*vscalefph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 2c 71 7f[ 	 ]*vscalefph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 2c 72 80[ 	 ]*vscalefph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 2c b4 f5 00 00 00 10[ 	 ]*vscalefph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 2c 31[ 	 ]*vscalefph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 2c 71 7f[ 	 ]*vscalefph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 2c 72 80[ 	 ]*vscalefph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 51 f5[ 	 ]*vsqrtph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 8f 51 f5[ 	 ]*vsqrtph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 28 51 f5[ 	 ]*vsqrtph ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c af 51 f5[ 	 ]*vsqrtph ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 0f 51 b4 f5 00 00 00 10[ 	 ]*vsqrtph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 18 51 31[ 	 ]*vsqrtph xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 51 71 7f[ 	 ]*vsqrtph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 9f 51 72 80[ 	 ]*vsqrtph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 2f 51 b4 f5 00 00 00 10[ 	 ]*vsqrtph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 38 51 31[ 	 ]*vsqrtph ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 28 51 71 7f[ 	 ]*vsqrtph ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c bf 51 72 80[ 	 ]*vsqrtph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 14 20 5c f4[ 	 ]*vsubph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 a7 5c f4[ 	 ]*vsubph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 00 5c f4[ 	 ]*vsubph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 87 5c f4[ 	 ]*vsubph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 25 14 27 5c b4 f5 00 00 00 10[ 	 ]*vsubph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 30 5c 31[ 	 ]*vsubph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 20 5c 71 7f[ 	 ]*vsubph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 b7 5c 72 80[ 	 ]*vsubph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 14 07 5c b4 f5 00 00 00 10[ 	 ]*vsubph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 10 5c 31[ 	 ]*vsubph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 00 5c 71 7f[ 	 ]*vsubph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 97 5c 72 80[ 	 ]*vsubph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 14 20 58 f4[ 	 ]*vaddph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 a7 58 f4[ 	 ]*vaddph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 00 58 f4[ 	 ]*vaddph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 87 58 f4[ 	 ]*vaddph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 25 14 27 58 b4 f5 00 00 00 10[ 	 ]*vaddph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 30 58 31[ 	 ]*vaddph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 20 58 71 7f[ 	 ]*vaddph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 b7 58 72 80[ 	 ]*vaddph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 14 07 58 b4 f5 00 00 00 10[ 	 ]*vaddph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 10 58 31[ 	 ]*vaddph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 00 58 71 7f[ 	 ]*vaddph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 97 58 72 80[ 	 ]*vaddph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 93 14 20 c2 ec 7b[ 	 ]*vcmpph k5,ymm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 14 27 c2 ec 7b[ 	 ]*vcmpph k5\{k7\},ymm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 14 00 c2 ec 7b[ 	 ]*vcmpph k5,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 14 07 c2 ec 7b[ 	 ]*vcmpph k5\{k7\},xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 14 07 c2 ac f5 00 00 00 10 7b[ 	 ]*vcmpph k5\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 d3 14 10 c2 29 7b[ 	 ]*vcmpph k5,xmm29,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 14 00 c2 69 7f 7b[ 	 ]*vcmpph k5,xmm29,XMMWORD PTR \[rcx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 14 17 c2 6a 80 7b[ 	 ]*vcmpph k5\{k7\},xmm29,WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 14 27 c2 ac f5 00 00 00 10 7b[ 	 ]*vcmpph k5\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 d3 14 30 c2 29 7b[ 	 ]*vcmpph k5,ymm29,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 14 20 c2 69 7f 7b[ 	 ]*vcmpph k5,ymm29,YMMWORD PTR \[rcx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 14 37 c2 6a 80 7b[ 	 ]*vcmpph k5\{k7\},ymm29,WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 5b f5[ 	 ]*vcvtdq2ph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 8f 5b f5[ 	 ]*vcvtdq2ph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 28 5b f5[ 	 ]*vcvtdq2ph xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c af 5b f5[ 	 ]*vcvtdq2ph xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 0f 5b b4 f5 00 00 00 10[ 	 ]*vcvtdq2ph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 18 5b 31[ 	 ]*vcvtdq2ph xmm30,DWORD BCST \[r9\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 5b 71 7f[ 	 ]*vcvtdq2ph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 9f 5b 72 80[ 	 ]*vcvtdq2ph xmm30\{k7\}\{z\},DWORD BCST \[rdx-0x200\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 38 5b 31[ 	 ]*vcvtdq2ph xmm30,DWORD BCST \[r9\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 28 5b 71 7f[ 	 ]*vcvtdq2ph xmm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c bf 5b 72 80[ 	 ]*vcvtdq2ph xmm30\{k7\}\{z\},DWORD BCST \[rdx-0x200\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 05 fd 08 5a f5[ 	 ]*vcvtpd2ph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 fd 8f 5a f5[ 	 ]*vcvtpd2ph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 fd 28 5a f5[ 	 ]*vcvtpd2ph xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 fd af 5a f5[ 	 ]*vcvtpd2ph xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 fd 0f 5a b4 f5 00 00 00 10[ 	 ]*vcvtpd2ph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 fd 18 5a 31[ 	 ]*vcvtpd2ph xmm30,QWORD BCST \[r9\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 65 fd 08 5a 71 7f[ 	 ]*vcvtpd2ph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 fd 9f 5a 72 80[ 	 ]*vcvtpd2ph xmm30\{k7\}\{z\},QWORD BCST \[rdx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 45 fd 38 5a 31[ 	 ]*vcvtpd2ph xmm30,QWORD BCST \[r9\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 65 fd 28 5a 71 7f[ 	 ]*vcvtpd2ph xmm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 fd bf 5a 72 80[ 	 ]*vcvtpd2ph xmm30\{k7\}\{z\},QWORD BCST \[rdx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 5b f5[ 	 ]*vcvtph2dq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 5b f5[ 	 ]*vcvtph2dq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 5b f5[ 	 ]*vcvtph2dq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 5b f5[ 	 ]*vcvtph2dq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 5b b4 f5 00 00 00 10[ 	 ]*vcvtph2dq xmm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 5b 31[ 	 ]*vcvtph2dq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 5b 71 7f[ 	 ]*vcvtph2dq xmm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 5b 72 80[ 	 ]*vcvtph2dq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 2f 5b b4 f5 00 00 00 10[ 	 ]*vcvtph2dq ymm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 5b 31[ 	 ]*vcvtph2dq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 5b 71 7f[ 	 ]*vcvtph2dq ymm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 5b 72 80[ 	 ]*vcvtph2dq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 5a f5[ 	 ]*vcvtph2pd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 8f 5a f5[ 	 ]*vcvtph2pd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 28 5a f5[ 	 ]*vcvtph2pd ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c af 5a f5[ 	 ]*vcvtph2pd ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 0f 5a b4 f5 00 00 00 10[ 	 ]*vcvtph2pd xmm30\{k7\},DWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 18 5a 31[ 	 ]*vcvtph2pd xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 5a 71 7f[ 	 ]*vcvtph2pd xmm30,DWORD PTR \[rcx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 9f 5a 72 80[ 	 ]*vcvtph2pd xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 2f 5a b4 f5 00 00 00 10[ 	 ]*vcvtph2pd ymm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 38 5a 31[ 	 ]*vcvtph2pd ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 28 5a 71 7f[ 	 ]*vcvtph2pd ymm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c bf 5a 72 80[ 	 ]*vcvtph2pd ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 08 13 f5[ 	 ]*vcvtph2psx xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 8f 13 f5[ 	 ]*vcvtph2psx xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 28 13 f5[ 	 ]*vcvtph2psx ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d af 13 f5[ 	 ]*vcvtph2psx ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 0f 13 b4 f5 00 00 00 10[ 	 ]*vcvtph2psx xmm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 18 13 31[ 	 ]*vcvtph2psx xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 08 13 71 7f[ 	 ]*vcvtph2psx xmm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 9f 13 72 80[ 	 ]*vcvtph2psx xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 2f 13 b4 f5 00 00 00 10[ 	 ]*vcvtph2psx ymm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 38 13 31[ 	 ]*vcvtph2psx ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 28 13 71 7f[ 	 ]*vcvtph2psx ymm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d bf 13 72 80[ 	 ]*vcvtph2psx ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 7b f5[ 	 ]*vcvtph2qq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 7b f5[ 	 ]*vcvtph2qq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 7b f5[ 	 ]*vcvtph2qq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 7b f5[ 	 ]*vcvtph2qq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 7b b4 f5 00 00 00 10[ 	 ]*vcvtph2qq xmm30\{k7\},DWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 7b 31[ 	 ]*vcvtph2qq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 7b 71 7f[ 	 ]*vcvtph2qq xmm30,DWORD PTR \[rcx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 7b 72 80[ 	 ]*vcvtph2qq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 2f 7b b4 f5 00 00 00 10[ 	 ]*vcvtph2qq ymm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 7b 31[ 	 ]*vcvtph2qq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 7b 71 7f[ 	 ]*vcvtph2qq ymm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 7b 72 80[ 	 ]*vcvtph2qq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 79 f5[ 	 ]*vcvtph2udq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 8f 79 f5[ 	 ]*vcvtph2udq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 28 79 f5[ 	 ]*vcvtph2udq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c af 79 f5[ 	 ]*vcvtph2udq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 0f 79 b4 f5 00 00 00 10[ 	 ]*vcvtph2udq xmm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 18 79 31[ 	 ]*vcvtph2udq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 79 71 7f[ 	 ]*vcvtph2udq xmm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 9f 79 72 80[ 	 ]*vcvtph2udq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 2f 79 b4 f5 00 00 00 10[ 	 ]*vcvtph2udq ymm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 38 79 31[ 	 ]*vcvtph2udq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 28 79 71 7f[ 	 ]*vcvtph2udq ymm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c bf 79 72 80[ 	 ]*vcvtph2udq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 79 f5[ 	 ]*vcvtph2uqq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 79 f5[ 	 ]*vcvtph2uqq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 79 f5[ 	 ]*vcvtph2uqq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 79 f5[ 	 ]*vcvtph2uqq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 79 b4 f5 00 00 00 10[ 	 ]*vcvtph2uqq xmm30\{k7\},DWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 79 31[ 	 ]*vcvtph2uqq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 79 71 7f[ 	 ]*vcvtph2uqq xmm30,DWORD PTR \[rcx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 79 72 80[ 	 ]*vcvtph2uqq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 2f 79 b4 f5 00 00 00 10[ 	 ]*vcvtph2uqq ymm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 79 31[ 	 ]*vcvtph2uqq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 79 71 7f[ 	 ]*vcvtph2uqq ymm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 79 72 80[ 	 ]*vcvtph2uqq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 7d f5[ 	 ]*vcvtph2uw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 8f 7d f5[ 	 ]*vcvtph2uw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 28 7d f5[ 	 ]*vcvtph2uw ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c af 7d f5[ 	 ]*vcvtph2uw ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 0f 7d b4 f5 00 00 00 10[ 	 ]*vcvtph2uw xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 18 7d 31[ 	 ]*vcvtph2uw xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 7d 71 7f[ 	 ]*vcvtph2uw xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 9f 7d 72 80[ 	 ]*vcvtph2uw xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 2f 7d b4 f5 00 00 00 10[ 	 ]*vcvtph2uw ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 38 7d 31[ 	 ]*vcvtph2uw ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 28 7d 71 7f[ 	 ]*vcvtph2uw ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c bf 7d 72 80[ 	 ]*vcvtph2uw ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 7d f5[ 	 ]*vcvtph2w xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 7d f5[ 	 ]*vcvtph2w xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 7d f5[ 	 ]*vcvtph2w ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 7d f5[ 	 ]*vcvtph2w ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 7d b4 f5 00 00 00 10[ 	 ]*vcvtph2w xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 7d 31[ 	 ]*vcvtph2w xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 7d 71 7f[ 	 ]*vcvtph2w xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 7d 72 80[ 	 ]*vcvtph2w xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 2f 7d b4 f5 00 00 00 10[ 	 ]*vcvtph2w ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 7d 31[ 	 ]*vcvtph2w ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 7d 71 7f[ 	 ]*vcvtph2w ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 7d 72 80[ 	 ]*vcvtph2w ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 1d f5[ 	 ]*vcvtps2phx xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 1d f5[ 	 ]*vcvtps2phx xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 1d f5[ 	 ]*vcvtps2phx xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 1d f5[ 	 ]*vcvtps2phx xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 1d b4 f5 00 00 00 10[ 	 ]*vcvtps2phx xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 1d 31[ 	 ]*vcvtps2phx xmm30,DWORD BCST \[r9\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 1d 71 7f[ 	 ]*vcvtps2phx xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 1d 72 80[ 	 ]*vcvtps2phx xmm30\{k7\}\{z\},DWORD BCST \[rdx-0x200\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 1d 31[ 	 ]*vcvtps2phx xmm30,DWORD BCST \[r9\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 1d 71 7f[ 	 ]*vcvtps2phx xmm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 1d 72 80[ 	 ]*vcvtps2phx xmm30\{k7\}\{z\},DWORD BCST \[rdx-0x200\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 05 fc 08 5b f5[ 	 ]*vcvtqq2ph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 fc 8f 5b f5[ 	 ]*vcvtqq2ph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 fc 28 5b f5[ 	 ]*vcvtqq2ph xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 fc af 5b f5[ 	 ]*vcvtqq2ph xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 fc 0f 5b b4 f5 00 00 00 10[ 	 ]*vcvtqq2ph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 fc 18 5b 31[ 	 ]*vcvtqq2ph xmm30,QWORD BCST \[r9\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 65 fc 08 5b 71 7f[ 	 ]*vcvtqq2ph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 fc 9f 5b 72 80[ 	 ]*vcvtqq2ph xmm30\{k7\}\{z\},QWORD BCST \[rdx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 45 fc 38 5b 31[ 	 ]*vcvtqq2ph xmm30,QWORD BCST \[r9\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 65 fc 28 5b 71 7f[ 	 ]*vcvtqq2ph xmm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 fc bf 5b 72 80[ 	 ]*vcvtqq2ph xmm30\{k7\}\{z\},QWORD BCST \[rdx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 08 5b f5[ 	 ]*vcvttph2dq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 8f 5b f5[ 	 ]*vcvttph2dq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 28 5b f5[ 	 ]*vcvttph2dq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7e af 5b f5[ 	 ]*vcvttph2dq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 0f 5b b4 f5 00 00 00 10[ 	 ]*vcvttph2dq xmm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 18 5b 31[ 	 ]*vcvttph2dq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 08 5b 71 7f[ 	 ]*vcvttph2dq xmm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 9f 5b 72 80[ 	 ]*vcvttph2dq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 2f 5b b4 f5 00 00 00 10[ 	 ]*vcvttph2dq ymm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 38 5b 31[ 	 ]*vcvttph2dq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 28 5b 71 7f[ 	 ]*vcvttph2dq ymm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e bf 5b 72 80[ 	 ]*vcvttph2dq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 7a f5[ 	 ]*vcvttph2qq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 7a f5[ 	 ]*vcvttph2qq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 7a f5[ 	 ]*vcvttph2qq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 7a f5[ 	 ]*vcvttph2qq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 7a b4 f5 00 00 00 10[ 	 ]*vcvttph2qq xmm30\{k7\},DWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 7a 31[ 	 ]*vcvttph2qq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 7a 71 7f[ 	 ]*vcvttph2qq xmm30,DWORD PTR \[rcx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 7a 72 80[ 	 ]*vcvttph2qq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 2f 7a b4 f5 00 00 00 10[ 	 ]*vcvttph2qq ymm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 7a 31[ 	 ]*vcvttph2qq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 7a 71 7f[ 	 ]*vcvttph2qq ymm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 7a 72 80[ 	 ]*vcvttph2qq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 78 f5[ 	 ]*vcvttph2udq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 8f 78 f5[ 	 ]*vcvttph2udq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 28 78 f5[ 	 ]*vcvttph2udq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c af 78 f5[ 	 ]*vcvttph2udq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 0f 78 b4 f5 00 00 00 10[ 	 ]*vcvttph2udq xmm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 18 78 31[ 	 ]*vcvttph2udq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 78 71 7f[ 	 ]*vcvttph2udq xmm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 9f 78 72 80[ 	 ]*vcvttph2udq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 2f 78 b4 f5 00 00 00 10[ 	 ]*vcvttph2udq ymm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 38 78 31[ 	 ]*vcvttph2udq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 28 78 71 7f[ 	 ]*vcvttph2udq ymm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c bf 78 72 80[ 	 ]*vcvttph2udq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 78 f5[ 	 ]*vcvttph2uqq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 78 f5[ 	 ]*vcvttph2uqq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 78 f5[ 	 ]*vcvttph2uqq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 78 f5[ 	 ]*vcvttph2uqq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 78 b4 f5 00 00 00 10[ 	 ]*vcvttph2uqq xmm30\{k7\},DWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 78 31[ 	 ]*vcvttph2uqq xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 78 71 7f[ 	 ]*vcvttph2uqq xmm30,DWORD PTR \[rcx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 78 72 80[ 	 ]*vcvttph2uqq xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 2f 78 b4 f5 00 00 00 10[ 	 ]*vcvttph2uqq ymm30\{k7\},QWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 78 31[ 	 ]*vcvttph2uqq ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 78 71 7f[ 	 ]*vcvttph2uqq ymm30,QWORD PTR \[rcx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 78 72 80[ 	 ]*vcvttph2uqq ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 7c f5[ 	 ]*vcvttph2uw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 8f 7c f5[ 	 ]*vcvttph2uw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 28 7c f5[ 	 ]*vcvttph2uw ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c af 7c f5[ 	 ]*vcvttph2uw ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 0f 7c b4 f5 00 00 00 10[ 	 ]*vcvttph2uw xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 18 7c 31[ 	 ]*vcvttph2uw xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 7c 71 7f[ 	 ]*vcvttph2uw xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 9f 7c 72 80[ 	 ]*vcvttph2uw xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 2f 7c b4 f5 00 00 00 10[ 	 ]*vcvttph2uw ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 38 7c 31[ 	 ]*vcvttph2uw ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 28 7c 71 7f[ 	 ]*vcvttph2uw ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c bf 7c 72 80[ 	 ]*vcvttph2uw ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 08 7c f5[ 	 ]*vcvttph2w xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 8f 7c f5[ 	 ]*vcvttph2w xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d 28 7c f5[ 	 ]*vcvttph2w ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7d af 7c f5[ 	 ]*vcvttph2w ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 0f 7c b4 f5 00 00 00 10[ 	 ]*vcvttph2w xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 18 7c 31[ 	 ]*vcvttph2w xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 08 7c 71 7f[ 	 ]*vcvttph2w xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 9f 7c 72 80[ 	 ]*vcvttph2w xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7d 2f 7c b4 f5 00 00 00 10[ 	 ]*vcvttph2w ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7d 38 7c 31[ 	 ]*vcvttph2w ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d 28 7c 71 7f[ 	 ]*vcvttph2w ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7d bf 7c 72 80[ 	 ]*vcvttph2w ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 08 7a f5[ 	 ]*vcvtudq2ph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 8f 7a f5[ 	 ]*vcvtudq2ph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 28 7a f5[ 	 ]*vcvtudq2ph xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7f af 7a f5[ 	 ]*vcvtudq2ph xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7f 0f 7a b4 f5 00 00 00 10[ 	 ]*vcvtudq2ph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7f 18 7a 31[ 	 ]*vcvtudq2ph xmm30,DWORD BCST \[r9\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 08 7a 71 7f[ 	 ]*vcvtudq2ph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 9f 7a 72 80[ 	 ]*vcvtudq2ph xmm30\{k7\}\{z\},DWORD BCST \[rdx-0x200\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 45 7f 38 7a 31[ 	 ]*vcvtudq2ph xmm30,DWORD BCST \[r9\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 28 7a 71 7f[ 	 ]*vcvtudq2ph xmm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7f bf 7a 72 80[ 	 ]*vcvtudq2ph xmm30\{k7\}\{z\},DWORD BCST \[rdx-0x200\]\{1to8\}
[ 	]*[a-f0-9]+:[ 	]*62 05 ff 08 7a f5[ 	 ]*vcvtuqq2ph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 ff 8f 7a f5[ 	 ]*vcvtuqq2ph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 ff 28 7a f5[ 	 ]*vcvtuqq2ph xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 ff af 7a f5[ 	 ]*vcvtuqq2ph xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 ff 0f 7a b4 f5 00 00 00 10[ 	 ]*vcvtuqq2ph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 ff 18 7a 31[ 	 ]*vcvtuqq2ph xmm30,QWORD BCST \[r9\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 65 ff 08 7a 71 7f[ 	 ]*vcvtuqq2ph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 ff 9f 7a 72 80[ 	 ]*vcvtuqq2ph xmm30\{k7\}\{z\},QWORD BCST \[rdx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 45 ff 38 7a 31[ 	 ]*vcvtuqq2ph xmm30,QWORD BCST \[r9\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 65 ff 28 7a 71 7f[ 	 ]*vcvtuqq2ph xmm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 ff bf 7a 72 80[ 	 ]*vcvtuqq2ph xmm30\{k7\}\{z\},QWORD BCST \[rdx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 08 7d f5[ 	 ]*vcvtuw2ph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 8f 7d f5[ 	 ]*vcvtuw2ph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7f 28 7d f5[ 	 ]*vcvtuw2ph ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7f af 7d f5[ 	 ]*vcvtuw2ph ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7f 0f 7d b4 f5 00 00 00 10[ 	 ]*vcvtuw2ph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7f 18 7d 31[ 	 ]*vcvtuw2ph xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 08 7d 71 7f[ 	 ]*vcvtuw2ph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 9f 7d 72 80[ 	 ]*vcvtuw2ph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7f 2f 7d b4 f5 00 00 00 10[ 	 ]*vcvtuw2ph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7f 38 7d 31[ 	 ]*vcvtuw2ph ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7f 28 7d 71 7f[ 	 ]*vcvtuw2ph ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7f bf 7d 72 80[ 	 ]*vcvtuw2ph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 08 7d f5[ 	 ]*vcvtw2ph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 8f 7d f5[ 	 ]*vcvtw2ph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7e 28 7d f5[ 	 ]*vcvtw2ph ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7e af 7d f5[ 	 ]*vcvtw2ph ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 0f 7d b4 f5 00 00 00 10[ 	 ]*vcvtw2ph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 18 7d 31[ 	 ]*vcvtw2ph xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 08 7d 71 7f[ 	 ]*vcvtw2ph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 9f 7d 72 80[ 	 ]*vcvtw2ph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7e 2f 7d b4 f5 00 00 00 10[ 	 ]*vcvtw2ph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7e 38 7d 31[ 	 ]*vcvtw2ph ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e 28 7d 71 7f[ 	 ]*vcvtw2ph ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7e bf 7d 72 80[ 	 ]*vcvtw2ph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 14 20 5e f4[ 	 ]*vdivph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 a7 5e f4[ 	 ]*vdivph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 00 5e f4[ 	 ]*vdivph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 87 5e f4[ 	 ]*vdivph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 25 14 27 5e b4 f5 00 00 00 10[ 	 ]*vdivph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 30 5e 31[ 	 ]*vdivph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 20 5e 71 7f[ 	 ]*vdivph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 b7 5e 72 80[ 	 ]*vdivph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 14 07 5e b4 f5 00 00 00 10[ 	 ]*vdivph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 10 5e 31[ 	 ]*vdivph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 00 5e 71 7f[ 	 ]*vdivph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 97 5e 72 80[ 	 ]*vdivph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 17 20 56 f4[ 	 ]*vfcmaddcph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 17 a7 56 f4[ 	 ]*vfcmaddcph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 17 00 56 f4[ 	 ]*vfcmaddcph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 17 87 56 f4[ 	 ]*vfcmaddcph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 17 27 56 b4 f5 00 00 00 10[ 	 ]*vfcmaddcph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 17 30 56 31[ 	 ]*vfcmaddcph ymm30,ymm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 20 56 71 7f[ 	 ]*vfcmaddcph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 b7 56 72 80[ 	 ]*vfcmaddcph ymm30\{k7\}\{z\},ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 26 17 07 56 b4 f5 00 00 00 10[ 	 ]*vfcmaddcph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 17 10 56 31[ 	 ]*vfcmaddcph xmm30,xmm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 00 56 71 7f[ 	 ]*vfcmaddcph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 97 56 72 80[ 	 ]*vfcmaddcph xmm30\{k7\}\{z\},xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 06 17 20 d6 f4[ 	 ]*vfcmulcph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 17 a7 d6 f4[ 	 ]*vfcmulcph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 17 00 d6 f4[ 	 ]*vfcmulcph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 17 87 d6 f4[ 	 ]*vfcmulcph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 17 27 d6 b4 f5 00 00 00 10[ 	 ]*vfcmulcph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 17 30 d6 31[ 	 ]*vfcmulcph ymm30,ymm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 20 d6 71 7f[ 	 ]*vfcmulcph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 b7 d6 72 80[ 	 ]*vfcmulcph ymm30\{k7\}\{z\},ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 26 17 07 d6 b4 f5 00 00 00 10[ 	 ]*vfcmulcph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 17 10 d6 31[ 	 ]*vfcmulcph xmm30,xmm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 00 d6 71 7f[ 	 ]*vfcmulcph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 17 97 d6 72 80[ 	 ]*vfcmulcph xmm30\{k7\}\{z\},xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 98 f4[ 	 ]*vfmadd132ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 98 f4[ 	 ]*vfmadd132ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 98 f4[ 	 ]*vfmadd132ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 98 f4[ 	 ]*vfmadd132ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 98 b4 f5 00 00 00 10[ 	 ]*vfmadd132ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 98 31[ 	 ]*vfmadd132ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 98 71 7f[ 	 ]*vfmadd132ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 98 72 80[ 	 ]*vfmadd132ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 98 b4 f5 00 00 00 10[ 	 ]*vfmadd132ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 98 31[ 	 ]*vfmadd132ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 98 71 7f[ 	 ]*vfmadd132ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 98 72 80[ 	 ]*vfmadd132ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 a8 f4[ 	 ]*vfmadd213ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 a8 f4[ 	 ]*vfmadd213ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 a8 f4[ 	 ]*vfmadd213ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 a8 f4[ 	 ]*vfmadd213ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 a8 b4 f5 00 00 00 10[ 	 ]*vfmadd213ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 a8 31[ 	 ]*vfmadd213ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 a8 71 7f[ 	 ]*vfmadd213ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 a8 72 80[ 	 ]*vfmadd213ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 a8 b4 f5 00 00 00 10[ 	 ]*vfmadd213ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 a8 31[ 	 ]*vfmadd213ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 a8 71 7f[ 	 ]*vfmadd213ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 a8 72 80[ 	 ]*vfmadd213ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 b8 f4[ 	 ]*vfmadd231ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 b8 f4[ 	 ]*vfmadd231ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 b8 f4[ 	 ]*vfmadd231ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 b8 f4[ 	 ]*vfmadd231ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 b8 b4 f5 00 00 00 10[ 	 ]*vfmadd231ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 b8 31[ 	 ]*vfmadd231ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 b8 71 7f[ 	 ]*vfmadd231ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 b8 72 80[ 	 ]*vfmadd231ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 b8 b4 f5 00 00 00 10[ 	 ]*vfmadd231ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 b8 31[ 	 ]*vfmadd231ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 b8 71 7f[ 	 ]*vfmadd231ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 b8 72 80[ 	 ]*vfmadd231ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 16 20 56 f4[ 	 ]*vfmaddcph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 16 a7 56 f4[ 	 ]*vfmaddcph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 16 00 56 f4[ 	 ]*vfmaddcph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 16 87 56 f4[ 	 ]*vfmaddcph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 16 27 56 b4 f5 00 00 00 10[ 	 ]*vfmaddcph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 16 30 56 31[ 	 ]*vfmaddcph ymm30,ymm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 20 56 71 7f[ 	 ]*vfmaddcph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 b7 56 72 80[ 	 ]*vfmaddcph ymm30\{k7\}\{z\},ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 26 16 07 56 b4 f5 00 00 00 10[ 	 ]*vfmaddcph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 16 10 56 31[ 	 ]*vfmaddcph xmm30,xmm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 00 56 71 7f[ 	 ]*vfmaddcph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 97 56 72 80[ 	 ]*vfmaddcph xmm30\{k7\}\{z\},xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 96 f4[ 	 ]*vfmaddsub132ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 96 f4[ 	 ]*vfmaddsub132ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 96 f4[ 	 ]*vfmaddsub132ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 96 f4[ 	 ]*vfmaddsub132ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 96 b4 f5 00 00 00 10[ 	 ]*vfmaddsub132ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 96 31[ 	 ]*vfmaddsub132ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 96 71 7f[ 	 ]*vfmaddsub132ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 96 72 80[ 	 ]*vfmaddsub132ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 96 b4 f5 00 00 00 10[ 	 ]*vfmaddsub132ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 96 31[ 	 ]*vfmaddsub132ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 96 71 7f[ 	 ]*vfmaddsub132ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 96 72 80[ 	 ]*vfmaddsub132ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 a6 f4[ 	 ]*vfmaddsub213ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 a6 f4[ 	 ]*vfmaddsub213ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 a6 f4[ 	 ]*vfmaddsub213ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 a6 f4[ 	 ]*vfmaddsub213ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 a6 b4 f5 00 00 00 10[ 	 ]*vfmaddsub213ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 a6 31[ 	 ]*vfmaddsub213ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 a6 71 7f[ 	 ]*vfmaddsub213ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 a6 72 80[ 	 ]*vfmaddsub213ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 a6 b4 f5 00 00 00 10[ 	 ]*vfmaddsub213ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 a6 31[ 	 ]*vfmaddsub213ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 a6 71 7f[ 	 ]*vfmaddsub213ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 a6 72 80[ 	 ]*vfmaddsub213ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 b6 f4[ 	 ]*vfmaddsub231ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 b6 f4[ 	 ]*vfmaddsub231ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 b6 f4[ 	 ]*vfmaddsub231ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 b6 f4[ 	 ]*vfmaddsub231ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 b6 b4 f5 00 00 00 10[ 	 ]*vfmaddsub231ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 b6 31[ 	 ]*vfmaddsub231ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 b6 71 7f[ 	 ]*vfmaddsub231ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 b6 72 80[ 	 ]*vfmaddsub231ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 b6 b4 f5 00 00 00 10[ 	 ]*vfmaddsub231ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 b6 31[ 	 ]*vfmaddsub231ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 b6 71 7f[ 	 ]*vfmaddsub231ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 b6 72 80[ 	 ]*vfmaddsub231ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 9a f4[ 	 ]*vfmsub132ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 9a f4[ 	 ]*vfmsub132ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 9a f4[ 	 ]*vfmsub132ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 9a f4[ 	 ]*vfmsub132ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 9a b4 f5 00 00 00 10[ 	 ]*vfmsub132ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 9a 31[ 	 ]*vfmsub132ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 9a 71 7f[ 	 ]*vfmsub132ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 9a 72 80[ 	 ]*vfmsub132ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 9a b4 f5 00 00 00 10[ 	 ]*vfmsub132ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 9a 31[ 	 ]*vfmsub132ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 9a 71 7f[ 	 ]*vfmsub132ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 9a 72 80[ 	 ]*vfmsub132ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 aa f4[ 	 ]*vfmsub213ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 aa f4[ 	 ]*vfmsub213ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 aa f4[ 	 ]*vfmsub213ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 aa f4[ 	 ]*vfmsub213ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 aa b4 f5 00 00 00 10[ 	 ]*vfmsub213ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 aa 31[ 	 ]*vfmsub213ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 aa 71 7f[ 	 ]*vfmsub213ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 aa 72 80[ 	 ]*vfmsub213ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 aa b4 f5 00 00 00 10[ 	 ]*vfmsub213ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 aa 31[ 	 ]*vfmsub213ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 aa 71 7f[ 	 ]*vfmsub213ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 aa 72 80[ 	 ]*vfmsub213ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 ba f4[ 	 ]*vfmsub231ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 ba f4[ 	 ]*vfmsub231ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 ba f4[ 	 ]*vfmsub231ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 ba f4[ 	 ]*vfmsub231ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 ba b4 f5 00 00 00 10[ 	 ]*vfmsub231ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 ba 31[ 	 ]*vfmsub231ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 ba 71 7f[ 	 ]*vfmsub231ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 ba 72 80[ 	 ]*vfmsub231ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 ba b4 f5 00 00 00 10[ 	 ]*vfmsub231ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 ba 31[ 	 ]*vfmsub231ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 ba 71 7f[ 	 ]*vfmsub231ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 ba 72 80[ 	 ]*vfmsub231ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 97 f4[ 	 ]*vfmsubadd132ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 97 f4[ 	 ]*vfmsubadd132ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 97 f4[ 	 ]*vfmsubadd132ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 97 f4[ 	 ]*vfmsubadd132ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 97 b4 f5 00 00 00 10[ 	 ]*vfmsubadd132ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 97 31[ 	 ]*vfmsubadd132ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 97 71 7f[ 	 ]*vfmsubadd132ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 97 72 80[ 	 ]*vfmsubadd132ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 97 b4 f5 00 00 00 10[ 	 ]*vfmsubadd132ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 97 31[ 	 ]*vfmsubadd132ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 97 71 7f[ 	 ]*vfmsubadd132ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 97 72 80[ 	 ]*vfmsubadd132ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 a7 f4[ 	 ]*vfmsubadd213ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 a7 f4[ 	 ]*vfmsubadd213ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 a7 f4[ 	 ]*vfmsubadd213ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 a7 f4[ 	 ]*vfmsubadd213ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 a7 b4 f5 00 00 00 10[ 	 ]*vfmsubadd213ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 a7 31[ 	 ]*vfmsubadd213ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 a7 71 7f[ 	 ]*vfmsubadd213ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 a7 72 80[ 	 ]*vfmsubadd213ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 a7 b4 f5 00 00 00 10[ 	 ]*vfmsubadd213ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 a7 31[ 	 ]*vfmsubadd213ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 a7 71 7f[ 	 ]*vfmsubadd213ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 a7 72 80[ 	 ]*vfmsubadd213ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 b7 f4[ 	 ]*vfmsubadd231ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 b7 f4[ 	 ]*vfmsubadd231ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 b7 f4[ 	 ]*vfmsubadd231ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 b7 f4[ 	 ]*vfmsubadd231ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 b7 b4 f5 00 00 00 10[ 	 ]*vfmsubadd231ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 b7 31[ 	 ]*vfmsubadd231ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 b7 71 7f[ 	 ]*vfmsubadd231ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 b7 72 80[ 	 ]*vfmsubadd231ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 b7 b4 f5 00 00 00 10[ 	 ]*vfmsubadd231ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 b7 31[ 	 ]*vfmsubadd231ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 b7 71 7f[ 	 ]*vfmsubadd231ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 b7 72 80[ 	 ]*vfmsubadd231ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 16 20 d6 f4[ 	 ]*vfmulcph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 16 a7 d6 f4[ 	 ]*vfmulcph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 16 00 d6 f4[ 	 ]*vfmulcph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 16 87 d6 f4[ 	 ]*vfmulcph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 16 27 d6 b4 f5 00 00 00 10[ 	 ]*vfmulcph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 16 30 d6 31[ 	 ]*vfmulcph ymm30,ymm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 20 d6 71 7f[ 	 ]*vfmulcph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 b7 d6 72 80[ 	 ]*vfmulcph ymm30\{k7\}\{z\},ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 26 16 07 d6 b4 f5 00 00 00 10[ 	 ]*vfmulcph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 16 10 d6 31[ 	 ]*vfmulcph xmm30,xmm29,DWORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 00 d6 71 7f[ 	 ]*vfmulcph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 16 97 d6 72 80[ 	 ]*vfmulcph xmm30\{k7\}\{z\},xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 9c f4[ 	 ]*vfnmadd132ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 9c f4[ 	 ]*vfnmadd132ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 9c f4[ 	 ]*vfnmadd132ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 9c f4[ 	 ]*vfnmadd132ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 9c b4 f5 00 00 00 10[ 	 ]*vfnmadd132ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 9c 31[ 	 ]*vfnmadd132ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 9c 71 7f[ 	 ]*vfnmadd132ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 9c 72 80[ 	 ]*vfnmadd132ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 9c b4 f5 00 00 00 10[ 	 ]*vfnmadd132ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 9c 31[ 	 ]*vfnmadd132ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 9c 71 7f[ 	 ]*vfnmadd132ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 9c 72 80[ 	 ]*vfnmadd132ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 ac f4[ 	 ]*vfnmadd213ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 ac f4[ 	 ]*vfnmadd213ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 ac f4[ 	 ]*vfnmadd213ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 ac f4[ 	 ]*vfnmadd213ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 ac b4 f5 00 00 00 10[ 	 ]*vfnmadd213ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 ac 31[ 	 ]*vfnmadd213ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 ac 71 7f[ 	 ]*vfnmadd213ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 ac 72 80[ 	 ]*vfnmadd213ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 ac b4 f5 00 00 00 10[ 	 ]*vfnmadd213ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 ac 31[ 	 ]*vfnmadd213ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 ac 71 7f[ 	 ]*vfnmadd213ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 ac 72 80[ 	 ]*vfnmadd213ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 bc f4[ 	 ]*vfnmadd231ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 bc f4[ 	 ]*vfnmadd231ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 bc f4[ 	 ]*vfnmadd231ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 bc f4[ 	 ]*vfnmadd231ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 bc b4 f5 00 00 00 10[ 	 ]*vfnmadd231ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 bc 31[ 	 ]*vfnmadd231ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 bc 71 7f[ 	 ]*vfnmadd231ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 bc 72 80[ 	 ]*vfnmadd231ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 bc b4 f5 00 00 00 10[ 	 ]*vfnmadd231ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 bc 31[ 	 ]*vfnmadd231ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 bc 71 7f[ 	 ]*vfnmadd231ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 bc 72 80[ 	 ]*vfnmadd231ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 9e f4[ 	 ]*vfnmsub132ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 9e f4[ 	 ]*vfnmsub132ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 9e f4[ 	 ]*vfnmsub132ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 9e f4[ 	 ]*vfnmsub132ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 9e b4 f5 00 00 00 10[ 	 ]*vfnmsub132ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 9e 31[ 	 ]*vfnmsub132ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 9e 71 7f[ 	 ]*vfnmsub132ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 9e 72 80[ 	 ]*vfnmsub132ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 9e b4 f5 00 00 00 10[ 	 ]*vfnmsub132ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 9e 31[ 	 ]*vfnmsub132ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 9e 71 7f[ 	 ]*vfnmsub132ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 9e 72 80[ 	 ]*vfnmsub132ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 ae f4[ 	 ]*vfnmsub213ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 ae f4[ 	 ]*vfnmsub213ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 ae f4[ 	 ]*vfnmsub213ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 ae f4[ 	 ]*vfnmsub213ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 ae b4 f5 00 00 00 10[ 	 ]*vfnmsub213ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 ae 31[ 	 ]*vfnmsub213ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 ae 71 7f[ 	 ]*vfnmsub213ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 ae 72 80[ 	 ]*vfnmsub213ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 ae b4 f5 00 00 00 10[ 	 ]*vfnmsub213ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 ae 31[ 	 ]*vfnmsub213ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 ae 71 7f[ 	 ]*vfnmsub213ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 ae 72 80[ 	 ]*vfnmsub213ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 be f4[ 	 ]*vfnmsub231ph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 be f4[ 	 ]*vfnmsub231ph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 be f4[ 	 ]*vfnmsub231ph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 be f4[ 	 ]*vfnmsub231ph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 be b4 f5 00 00 00 10[ 	 ]*vfnmsub231ph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 be 31[ 	 ]*vfnmsub231ph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 be 71 7f[ 	 ]*vfnmsub231ph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 be 72 80[ 	 ]*vfnmsub231ph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 be b4 f5 00 00 00 10[ 	 ]*vfnmsub231ph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 be 31[ 	 ]*vfnmsub231ph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 be 71 7f[ 	 ]*vfnmsub231ph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 be 72 80[ 	 ]*vfnmsub231ph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 08 66 ee 7b[ 	 ]*vfpclassph k5,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 0f 66 ee 7b[ 	 ]*vfpclassph k5\{k7\},xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 28 66 ee 7b[ 	 ]*vfpclassph k5,ymm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 7c 2f 66 ee 7b[ 	 ]*vfpclassph k5\{k7\},ymm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 7c 0f 66 ac f5 00 00 00 10 7b[ 	 ]*vfpclassph k5\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 d3 7c 18 66 29 7b[ 	 ]*vfpclassph k5,WORD BCST \[r9\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 08 66 69 7f 7b[ 	 ]*vfpclassph k5,XMMWORD PTR \[rcx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 1f 66 6a 80 7b[ 	 ]*vfpclassph k5\{k7\},WORD BCST \[rdx-0x100\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 d3 7c 38 66 29 7b[ 	 ]*vfpclassph k5,WORD BCST \[r9\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 28 66 69 7f 7b[ 	 ]*vfpclassph k5,YMMWORD PTR \[rcx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7c 3f 66 6a 80 7b[ 	 ]*vfpclassph k5\{k7\},WORD BCST \[rdx-0x100\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 08 42 f5[ 	 ]*vgetexpph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 8f 42 f5[ 	 ]*vgetexpph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 28 42 f5[ 	 ]*vgetexpph ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d af 42 f5[ 	 ]*vgetexpph ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 0f 42 b4 f5 00 00 00 10[ 	 ]*vgetexpph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 18 42 31[ 	 ]*vgetexpph xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 08 42 71 7f[ 	 ]*vgetexpph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 9f 42 72 80[ 	 ]*vgetexpph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 2f 42 b4 f5 00 00 00 10[ 	 ]*vgetexpph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 38 42 31[ 	 ]*vgetexpph ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 28 42 71 7f[ 	 ]*vgetexpph ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d bf 42 72 80[ 	 ]*vgetexpph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 28 26 f5 7b[ 	 ]*vgetmantph ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c af 26 f5 7b[ 	 ]*vgetmantph ymm30\{k7\}\{z\},ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 08 26 f5 7b[ 	 ]*vgetmantph xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 8f 26 f5 7b[ 	 ]*vgetmantph xmm30\{k7\}\{z\},xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 0f 26 b4 f5 00 00 00 10 7b[ 	 ]*vgetmantph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 18 26 31 7b[ 	 ]*vgetmantph xmm30,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 08 26 71 7f 7b[ 	 ]*vgetmantph xmm30,XMMWORD PTR \[rcx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 9f 26 72 80 7b[ 	 ]*vgetmantph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 2f 26 b4 f5 00 00 00 10 7b[ 	 ]*vgetmantph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 38 26 31 7b[ 	 ]*vgetmantph ymm30,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 28 26 71 7f 7b[ 	 ]*vgetmantph ymm30,YMMWORD PTR \[rcx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c bf 26 72 80 7b[ 	 ]*vgetmantph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 05 14 20 5f f4[ 	 ]*vmaxph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 a7 5f f4[ 	 ]*vmaxph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 00 5f f4[ 	 ]*vmaxph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 87 5f f4[ 	 ]*vmaxph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 25 14 27 5f b4 f5 00 00 00 10[ 	 ]*vmaxph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 30 5f 31[ 	 ]*vmaxph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 20 5f 71 7f[ 	 ]*vmaxph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 b7 5f 72 80[ 	 ]*vmaxph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 14 07 5f b4 f5 00 00 00 10[ 	 ]*vmaxph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 10 5f 31[ 	 ]*vmaxph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 00 5f 71 7f[ 	 ]*vmaxph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 97 5f 72 80[ 	 ]*vmaxph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 14 20 5d f4[ 	 ]*vminph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 a7 5d f4[ 	 ]*vminph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 00 5d f4[ 	 ]*vminph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 87 5d f4[ 	 ]*vminph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 25 14 27 5d b4 f5 00 00 00 10[ 	 ]*vminph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 30 5d 31[ 	 ]*vminph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 20 5d 71 7f[ 	 ]*vminph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 b7 5d 72 80[ 	 ]*vminph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 14 07 5d b4 f5 00 00 00 10[ 	 ]*vminph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 10 5d 31[ 	 ]*vminph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 00 5d 71 7f[ 	 ]*vminph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 97 5d 72 80[ 	 ]*vminph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 14 20 59 f4[ 	 ]*vmulph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 a7 59 f4[ 	 ]*vmulph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 00 59 f4[ 	 ]*vmulph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 87 59 f4[ 	 ]*vmulph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 25 14 27 59 b4 f5 00 00 00 10[ 	 ]*vmulph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 30 59 31[ 	 ]*vmulph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 20 59 71 7f[ 	 ]*vmulph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 b7 59 72 80[ 	 ]*vmulph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 14 07 59 b4 f5 00 00 00 10[ 	 ]*vmulph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 10 59 31[ 	 ]*vmulph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 00 59 71 7f[ 	 ]*vmulph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 97 59 72 80[ 	 ]*vmulph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 08 4c f5[ 	 ]*vrcpph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 8f 4c f5[ 	 ]*vrcpph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 28 4c f5[ 	 ]*vrcpph ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d af 4c f5[ 	 ]*vrcpph ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 0f 4c b4 f5 00 00 00 10[ 	 ]*vrcpph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 18 4c 31[ 	 ]*vrcpph xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 08 4c 71 7f[ 	 ]*vrcpph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 9f 4c 72 80[ 	 ]*vrcpph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 2f 4c b4 f5 00 00 00 10[ 	 ]*vrcpph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 38 4c 31[ 	 ]*vrcpph ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 28 4c 71 7f[ 	 ]*vrcpph ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d bf 4c 72 80[ 	 ]*vrcpph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 28 56 f5 7b[ 	 ]*vreduceph ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c af 56 f5 7b[ 	 ]*vreduceph ymm30\{k7\}\{z\},ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 08 56 f5 7b[ 	 ]*vreduceph xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 8f 56 f5 7b[ 	 ]*vreduceph xmm30\{k7\}\{z\},xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 0f 56 b4 f5 00 00 00 10 7b[ 	 ]*vreduceph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 18 56 31 7b[ 	 ]*vreduceph xmm30,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 08 56 71 7f 7b[ 	 ]*vreduceph xmm30,XMMWORD PTR \[rcx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 9f 56 72 80 7b[ 	 ]*vreduceph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 2f 56 b4 f5 00 00 00 10 7b[ 	 ]*vreduceph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 38 56 31 7b[ 	 ]*vreduceph ymm30,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 28 56 71 7f 7b[ 	 ]*vreduceph ymm30,YMMWORD PTR \[rcx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c bf 56 72 80 7b[ 	 ]*vreduceph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 28 08 f5 7b[ 	 ]*vrndscaleph ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c af 08 f5 7b[ 	 ]*vrndscaleph ymm30\{k7\}\{z\},ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 08 08 f5 7b[ 	 ]*vrndscaleph xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7c 8f 08 f5 7b[ 	 ]*vrndscaleph xmm30\{k7\}\{z\},xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 0f 08 b4 f5 00 00 00 10 7b[ 	 ]*vrndscaleph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 18 08 31 7b[ 	 ]*vrndscaleph xmm30,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 08 08 71 7f 7b[ 	 ]*vrndscaleph xmm30,XMMWORD PTR \[rcx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 9f 08 72 80 7b[ 	 ]*vrndscaleph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7c 2f 08 b4 f5 00 00 00 10 7b[ 	 ]*vrndscaleph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7c 38 08 31 7b[ 	 ]*vrndscaleph ymm30,WORD BCST \[r9\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c 28 08 71 7f 7b[ 	 ]*vrndscaleph ymm30,YMMWORD PTR \[rcx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7c bf 08 72 80 7b[ 	 ]*vrndscaleph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 08 4e f5[ 	 ]*vrsqrtph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 8f 4e f5[ 	 ]*vrsqrtph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d 28 4e f5[ 	 ]*vrsqrtph ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 06 7d af 4e f5[ 	 ]*vrsqrtph ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 0f 4e b4 f5 00 00 00 10[ 	 ]*vrsqrtph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 18 4e 31[ 	 ]*vrsqrtph xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 08 4e 71 7f[ 	 ]*vrsqrtph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 9f 4e 72 80[ 	 ]*vrsqrtph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 7d 2f 4e b4 f5 00 00 00 10[ 	 ]*vrsqrtph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 7d 38 4e 31[ 	 ]*vrsqrtph ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d 28 4e 71 7f[ 	 ]*vrsqrtph ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 7d bf 4e 72 80[ 	 ]*vrsqrtph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 06 15 20 2c f4[ 	 ]*vscalefph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 a7 2c f4[ 	 ]*vscalefph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 00 2c f4[ 	 ]*vscalefph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 06 15 87 2c f4[ 	 ]*vscalefph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 26 15 27 2c b4 f5 00 00 00 10[ 	 ]*vscalefph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 30 2c 31[ 	 ]*vscalefph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 20 2c 71 7f[ 	 ]*vscalefph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 b7 2c 72 80[ 	 ]*vscalefph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 26 15 07 2c b4 f5 00 00 00 10[ 	 ]*vscalefph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 46 15 10 2c 31[ 	 ]*vscalefph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 00 2c 71 7f[ 	 ]*vscalefph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 66 15 97 2c 72 80[ 	 ]*vscalefph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 08 51 f5[ 	 ]*vsqrtph xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 8f 51 f5[ 	 ]*vsqrtph xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c 28 51 f5[ 	 ]*vsqrtph ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 05 7c af 51 f5[ 	 ]*vsqrtph ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 0f 51 b4 f5 00 00 00 10[ 	 ]*vsqrtph xmm30\{k7\},XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 18 51 31[ 	 ]*vsqrtph xmm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 08 51 71 7f[ 	 ]*vsqrtph xmm30,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 9f 51 72 80[ 	 ]*vsqrtph xmm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 7c 2f 51 b4 f5 00 00 00 10[ 	 ]*vsqrtph ymm30\{k7\},YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 7c 38 51 31[ 	 ]*vsqrtph ymm30,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c 28 51 71 7f[ 	 ]*vsqrtph ymm30,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 7c bf 51 72 80[ 	 ]*vsqrtph ymm30\{k7\}\{z\},WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 05 14 20 5c f4[ 	 ]*vsubph ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 a7 5c f4[ 	 ]*vsubph ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 00 5c f4[ 	 ]*vsubph xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 05 14 87 5c f4[ 	 ]*vsubph xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 25 14 27 5c b4 f5 00 00 00 10[ 	 ]*vsubph ymm30\{k7\},ymm29,YMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 30 5c 31[ 	 ]*vsubph ymm30,ymm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 20 5c 71 7f[ 	 ]*vsubph ymm30,ymm29,YMMWORD PTR \[rcx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 b7 5c 72 80[ 	 ]*vsubph ymm30\{k7\}\{z\},ymm29,WORD BCST \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 25 14 07 5c b4 f5 00 00 00 10[ 	 ]*vsubph xmm30\{k7\},xmm29,XMMWORD PTR \[rbp\+r14\*8\+0x10000000\]
[ 	]*[a-f0-9]+:[ 	]*62 45 14 10 5c 31[ 	 ]*vsubph xmm30,xmm29,WORD BCST \[r9\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 00 5c 71 7f[ 	 ]*vsubph xmm30,xmm29,XMMWORD PTR \[rcx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 65 14 97 5c 72 80[ 	 ]*vsubph xmm30\{k7\}\{z\},xmm29,WORD BCST \[rdx-0x100\]
#pass
