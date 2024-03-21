#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512DQ/VL insns (Intel disassembly)
#source: x86-64-avx512dq_vl.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a 31[ 	]*vbroadcastf64x2 ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 1a 31[ 	]*vbroadcastf64x2 ymm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd af 1a 31[ 	]*vbroadcastf64x2 ymm30\{k7\}\{z\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 1a b4 f0 23 01 00 00[ 	]*vbroadcastf64x2 ymm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a 72 7f[ 	]*vbroadcastf64x2 ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a 72 80[ 	]*vbroadcastf64x2 ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a 31[ 	]*vbroadcasti64x2 ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 5a 31[ 	]*vbroadcasti64x2 ymm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd af 5a 31[ 	]*vbroadcasti64x2 ymm30\{k7\}\{z\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 5a b4 f0 23 01 00 00[ 	]*vbroadcasti64x2 ymm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a 72 7f[ 	]*vbroadcasti64x2 ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a 72 80[ 	]*vbroadcasti64x2 ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 19 f7[ 	]*vbroadcastf32x2 ymm30,xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 19 f7[ 	]*vbroadcastf32x2 ymm30\{k7\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 19 f7[ 	]*vbroadcastf32x2 ymm30\{k7\}\{z\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 31[ 	]*vbroadcastf32x2 ymm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 19 b4 f0 23 01 00 00[ 	]*vbroadcastf32x2 ymm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 72 7f[ 	]*vbroadcastf32x2 ymm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 ymm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 72 80[ 	]*vbroadcastf32x2 ymm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 ymm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 7b f5[ 	]*vcvtpd2qq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 7b f5[ 	]*vcvtpd2qq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 7b f5[ 	]*vcvtpd2qq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b 31[ 	]*vcvtpd2qq xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 7b b4 f0 23 01 00 00[ 	]*vcvtpd2qq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b 31[ 	]*vcvtpd2qq xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b 72 7f[ 	]*vcvtpd2qq xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b b2 00 08 00 00[ 	]*vcvtpd2qq xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b 72 80[ 	]*vcvtpd2qq xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b b2 f0 f7 ff ff[ 	]*vcvtpd2qq xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b 72 7f[ 	]*vcvtpd2qq xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b b2 00 04 00 00[ 	]*vcvtpd2qq xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b 72 80[ 	]*vcvtpd2qq xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 7b f5[ 	]*vcvtpd2qq ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 7b f5[ 	]*vcvtpd2qq ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 7b f5[ 	]*vcvtpd2qq ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b 31[ 	]*vcvtpd2qq ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 7b b4 f0 23 01 00 00[ 	]*vcvtpd2qq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b 31[ 	]*vcvtpd2qq ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b 72 7f[ 	]*vcvtpd2qq ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b b2 00 10 00 00[ 	]*vcvtpd2qq ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b 72 80[ 	]*vcvtpd2qq ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b b2 e0 ef ff ff[ 	]*vcvtpd2qq ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b 72 7f[ 	]*vcvtpd2qq ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b b2 00 04 00 00[ 	]*vcvtpd2qq ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b 72 80[ 	]*vcvtpd2qq ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 79 f5[ 	]*vcvtpd2uqq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 79 f5[ 	]*vcvtpd2uqq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 79 f5[ 	]*vcvtpd2uqq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 31[ 	]*vcvtpd2uqq xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 79 b4 f0 23 01 00 00[ 	]*vcvtpd2uqq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 31[ 	]*vcvtpd2uqq xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 72 7f[ 	]*vcvtpd2uqq xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 b2 00 08 00 00[ 	]*vcvtpd2uqq xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 72 80[ 	]*vcvtpd2uqq xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 b2 f0 f7 ff ff[ 	]*vcvtpd2uqq xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 72 7f[ 	]*vcvtpd2uqq xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 b2 00 04 00 00[ 	]*vcvtpd2uqq xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 72 80[ 	]*vcvtpd2uqq xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 79 f5[ 	]*vcvtpd2uqq ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 79 f5[ 	]*vcvtpd2uqq ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 79 f5[ 	]*vcvtpd2uqq ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 31[ 	]*vcvtpd2uqq ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 79 b4 f0 23 01 00 00[ 	]*vcvtpd2uqq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 31[ 	]*vcvtpd2uqq ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 72 7f[ 	]*vcvtpd2uqq ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 b2 00 10 00 00[ 	]*vcvtpd2uqq ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 72 80[ 	]*vcvtpd2uqq ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 b2 e0 ef ff ff[ 	]*vcvtpd2uqq ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 72 7f[ 	]*vcvtpd2uqq ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 b2 00 04 00 00[ 	]*vcvtpd2uqq ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 72 80[ 	]*vcvtpd2uqq ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 7b f5[ 	]*vcvtps2qq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 7b f5[ 	]*vcvtps2qq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 7b f5[ 	]*vcvtps2qq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b 31[ 	]*vcvtps2qq xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 7b b4 f0 23 01 00 00[ 	]*vcvtps2qq xmm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b 31[ 	]*vcvtps2qq xmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b 72 7f[ 	]*vcvtps2qq xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b b2 00 04 00 00[ 	]*vcvtps2qq xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b 72 80[ 	]*vcvtps2qq xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b b2 f8 fb ff ff[ 	]*vcvtps2qq xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b 72 7f[ 	]*vcvtps2qq xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b b2 00 02 00 00[ 	]*vcvtps2qq xmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b 72 80[ 	]*vcvtps2qq xmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b b2 fc fd ff ff[ 	]*vcvtps2qq xmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 7b f5[ 	]*vcvtps2qq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 7b f5[ 	]*vcvtps2qq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 7b f5[ 	]*vcvtps2qq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b 31[ 	]*vcvtps2qq ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 7b b4 f0 23 01 00 00[ 	]*vcvtps2qq ymm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b 31[ 	]*vcvtps2qq ymm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b 72 7f[ 	]*vcvtps2qq ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b b2 00 08 00 00[ 	]*vcvtps2qq ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b 72 80[ 	]*vcvtps2qq ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b b2 f0 f7 ff ff[ 	]*vcvtps2qq ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b 72 7f[ 	]*vcvtps2qq ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b b2 00 02 00 00[ 	]*vcvtps2qq ymm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b 72 80[ 	]*vcvtps2qq ymm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b b2 fc fd ff ff[ 	]*vcvtps2qq ymm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 79 f5[ 	]*vcvtps2uqq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 79 f5[ 	]*vcvtps2uqq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 79 f5[ 	]*vcvtps2uqq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 31[ 	]*vcvtps2uqq xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 79 b4 f0 23 01 00 00[ 	]*vcvtps2uqq xmm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 31[ 	]*vcvtps2uqq xmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 72 7f[ 	]*vcvtps2uqq xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 b2 00 04 00 00[ 	]*vcvtps2uqq xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 72 80[ 	]*vcvtps2uqq xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 b2 f8 fb ff ff[ 	]*vcvtps2uqq xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 72 7f[ 	]*vcvtps2uqq xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 b2 00 02 00 00[ 	]*vcvtps2uqq xmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 72 80[ 	]*vcvtps2uqq xmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 b2 fc fd ff ff[ 	]*vcvtps2uqq xmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 79 f5[ 	]*vcvtps2uqq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 79 f5[ 	]*vcvtps2uqq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 79 f5[ 	]*vcvtps2uqq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 31[ 	]*vcvtps2uqq ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 79 b4 f0 23 01 00 00[ 	]*vcvtps2uqq ymm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 31[ 	]*vcvtps2uqq ymm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 72 7f[ 	]*vcvtps2uqq ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 b2 00 08 00 00[ 	]*vcvtps2uqq ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 72 80[ 	]*vcvtps2uqq ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 b2 f0 f7 ff ff[ 	]*vcvtps2uqq ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 72 7f[ 	]*vcvtps2uqq ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 b2 00 02 00 00[ 	]*vcvtps2uqq ymm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 72 80[ 	]*vcvtps2uqq ymm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 b2 fc fd ff ff[ 	]*vcvtps2uqq ymm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 08 e6 f5[ 	]*vcvtqq2pd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 0f e6 f5[ 	]*vcvtqq2pd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 8f e6 f5[ 	]*vcvtqq2pd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 31[ 	]*vcvtqq2pd xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 08 e6 b4 f0 23 01 00 00[ 	]*vcvtqq2pd xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 31[ 	]*vcvtqq2pd xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 72 7f[ 	]*vcvtqq2pd xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 b2 00 08 00 00[ 	]*vcvtqq2pd xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 72 80[ 	]*vcvtqq2pd xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 b2 f0 f7 ff ff[ 	]*vcvtqq2pd xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 72 7f[ 	]*vcvtqq2pd xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 b2 00 04 00 00[ 	]*vcvtqq2pd xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 72 80[ 	]*vcvtqq2pd xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 28 e6 f5[ 	]*vcvtqq2pd ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 2f e6 f5[ 	]*vcvtqq2pd ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe af e6 f5[ 	]*vcvtqq2pd ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 31[ 	]*vcvtqq2pd ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 28 e6 b4 f0 23 01 00 00[ 	]*vcvtqq2pd ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 31[ 	]*vcvtqq2pd ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 72 7f[ 	]*vcvtqq2pd ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 b2 00 10 00 00[ 	]*vcvtqq2pd ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 72 80[ 	]*vcvtqq2pd ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 b2 e0 ef ff ff[ 	]*vcvtqq2pd ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 72 7f[ 	]*vcvtqq2pd ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 b2 00 04 00 00[ 	]*vcvtqq2pd ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 72 80[ 	]*vcvtqq2pd ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 08 5b f5[ 	]*vcvtqq2ps xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 0f 5b f5[ 	]*vcvtqq2ps xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 8f 5b f5[ 	]*vcvtqq2ps xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b 31[ 	]*vcvtqq2ps xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fc 08 5b b4 f0 23 01 00 00[ 	]*vcvtqq2ps xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b 31[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rcx\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b 72 7f[ 	]*vcvtqq2ps xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b b2 00 08 00 00[ 	]*vcvtqq2ps xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b 72 80[ 	]*vcvtqq2ps xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b b2 f0 f7 ff ff[ 	]*vcvtqq2ps xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b 72 7f[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx\+0x3f8\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b b2 00 04 00 00[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx\+0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b 72 80[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx-0x408\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 28 5b f5[ 	]*vcvtqq2ps xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 2f 5b f5[ 	]*vcvtqq2ps xmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fc af 5b f5[ 	]*vcvtqq2ps xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b 31[ 	]*vcvtqq2ps xmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fc 28 5b b4 f0 23 01 00 00[ 	]*vcvtqq2ps xmm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b 31[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rcx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b 72 7f[ 	]*vcvtqq2ps xmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b b2 00 10 00 00[ 	]*vcvtqq2ps xmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b 72 80[ 	]*vcvtqq2ps xmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b b2 e0 ef ff ff[ 	]*vcvtqq2ps xmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b 72 7f[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx\+0x3f8\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b b2 00 04 00 00[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx\+0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b 72 80[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx-0x408\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 08 7a f5[ 	]*vcvtuqq2pd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 0f 7a f5[ 	]*vcvtuqq2pd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 8f 7a f5[ 	]*vcvtuqq2pd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a 31[ 	]*vcvtuqq2pd xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 08 7a b4 f0 23 01 00 00[ 	]*vcvtuqq2pd xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a 31[ 	]*vcvtuqq2pd xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a 72 7f[ 	]*vcvtuqq2pd xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a b2 00 08 00 00[ 	]*vcvtuqq2pd xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a 72 80[ 	]*vcvtuqq2pd xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2pd xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a 72 7f[ 	]*vcvtuqq2pd xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a b2 00 04 00 00[ 	]*vcvtuqq2pd xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a 72 80[ 	]*vcvtuqq2pd xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 28 7a f5[ 	]*vcvtuqq2pd ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 2f 7a f5[ 	]*vcvtuqq2pd ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe af 7a f5[ 	]*vcvtuqq2pd ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a 31[ 	]*vcvtuqq2pd ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 28 7a b4 f0 23 01 00 00[ 	]*vcvtuqq2pd ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a 31[ 	]*vcvtuqq2pd ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a 72 7f[ 	]*vcvtuqq2pd ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a b2 00 10 00 00[ 	]*vcvtuqq2pd ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a 72 80[ 	]*vcvtuqq2pd ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a b2 e0 ef ff ff[ 	]*vcvtuqq2pd ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a 72 7f[ 	]*vcvtuqq2pd ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a b2 00 04 00 00[ 	]*vcvtuqq2pd ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a 72 80[ 	]*vcvtuqq2pd ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 7a f5[ 	]*vcvtuqq2ps xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 7a f5[ 	]*vcvtuqq2ps xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 7a f5[ 	]*vcvtuqq2ps xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a 31[ 	]*vcvtuqq2ps xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 08 7a b4 f0 23 01 00 00[ 	]*vcvtuqq2ps xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a 31[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rcx\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a 72 7f[ 	]*vcvtuqq2ps xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a b2 00 08 00 00[ 	]*vcvtuqq2ps xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a 72 80[ 	]*vcvtuqq2ps xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2ps xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a 72 7f[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx\+0x3f8\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a b2 00 04 00 00[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx\+0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a 72 80[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx-0x408\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 7a f5[ 	]*vcvtuqq2ps xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 7a f5[ 	]*vcvtuqq2ps xmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 7a f5[ 	]*vcvtuqq2ps xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a 31[ 	]*vcvtuqq2ps xmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 28 7a b4 f0 23 01 00 00[ 	]*vcvtuqq2ps xmm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a 31[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rcx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a 72 7f[ 	]*vcvtuqq2ps xmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a b2 00 10 00 00[ 	]*vcvtuqq2ps xmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a 72 80[ 	]*vcvtuqq2ps xmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a b2 e0 ef ff ff[ 	]*vcvtuqq2ps xmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a 72 7f[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx\+0x3f8\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a b2 00 04 00 00[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx\+0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a 72 80[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx-0x408\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 19 ee ab[ 	]*vextractf64x2 xmm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 2f 19 ee ab[ 	]*vextractf64x2 xmm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd af 19 ee ab[ 	]*vextractf64x2 xmm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 19 ee 7b[ 	]*vextractf64x2 xmm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 39 ee ab[ 	]*vextracti64x2 xmm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 2f 39 ee ab[ 	]*vextracti64x2 xmm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd af 39 ee ab[ 	]*vextracti64x2 xmm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 39 ee 7b[ 	]*vextracti64x2 xmm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 66 ee ab[ 	]*vfpclasspd k5,xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 0f 66 ee ab[ 	]*vfpclasspd k5\{k7\},xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 66 ee 7b[ 	]*vfpclasspd k5,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 29 7b[ 	]*vfpclasspd k5,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 08 66 ac f0 23 01 00 00 7b[ 	]*vfpclasspd k5,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 29 7b[ 	]*vfpclasspd k5,QWORD BCST \[rcx\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 6a 7f 7b[ 	]*vfpclasspd k5,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 aa 00 08 00 00 7b[ 	]*vfpclasspd k5,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 6a 80 7b[ 	]*vfpclasspd k5,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 aa f0 f7 ff ff 7b[ 	]*vfpclasspd k5,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 6a 7f 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx\+0x3f8\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 aa 00 04 00 00 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx\+0x400\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 6a 80 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx-0x400\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx-0x408\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 28 66 ee ab[ 	]*vfpclasspd k5,ymm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 2f 66 ee ab[ 	]*vfpclasspd k5\{k7\},ymm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 28 66 ee 7b[ 	]*vfpclasspd k5,ymm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 29 7b[ 	]*vfpclasspd k5,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 28 66 ac f0 23 01 00 00 7b[ 	]*vfpclasspd k5,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 29 7b[ 	]*vfpclasspd k5,QWORD BCST \[rcx\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 6a 7f 7b[ 	]*vfpclasspd k5,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 aa 00 10 00 00 7b[ 	]*vfpclasspd k5,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 6a 80 7b[ 	]*vfpclasspd k5,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 aa e0 ef ff ff 7b[ 	]*vfpclasspd k5,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 6a 7f 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx\+0x3f8\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 aa 00 04 00 00 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx\+0x400\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 6a 80 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx-0x400\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx-0x408\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 66 ee ab[ 	]*vfpclassps k5,xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 0f 66 ee ab[ 	]*vfpclassps k5\{k7\},xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 66 ee 7b[ 	]*vfpclassps k5,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 29 7b[ 	]*vfpclassps k5,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 08 66 ac f0 23 01 00 00 7b[ 	]*vfpclassps k5,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 29 7b[ 	]*vfpclassps k5,DWORD BCST \[rcx\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 6a 7f 7b[ 	]*vfpclassps k5,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 aa 00 08 00 00 7b[ 	]*vfpclassps k5,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 6a 80 7b[ 	]*vfpclassps k5,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 aa f0 f7 ff ff 7b[ 	]*vfpclassps k5,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 6a 7f 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx\+0x1fc\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 aa 00 02 00 00 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx\+0x200\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 6a 80 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx-0x200\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 aa fc fd ff ff 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx-0x204\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 28 66 ee ab[ 	]*vfpclassps k5,ymm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 2f 66 ee ab[ 	]*vfpclassps k5\{k7\},ymm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 28 66 ee 7b[ 	]*vfpclassps k5,ymm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 29 7b[ 	]*vfpclassps k5,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 28 66 ac f0 23 01 00 00 7b[ 	]*vfpclassps k5,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 29 7b[ 	]*vfpclassps k5,DWORD BCST \[rcx\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 6a 7f 7b[ 	]*vfpclassps k5,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 aa 00 10 00 00 7b[ 	]*vfpclassps k5,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 6a 80 7b[ 	]*vfpclassps k5,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 aa e0 ef ff ff 7b[ 	]*vfpclassps k5,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 6a 7f 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx\+0x1fc\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 aa 00 02 00 00 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx\+0x200\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 6a 80 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx-0x200\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 aa fc fd ff ff 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx-0x204\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 18 f4 ab[ 	]*vinsertf64x2 ymm30,ymm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 18 f4 ab[ 	]*vinsertf64x2 ymm30\{k7\},ymm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 18 f4 ab[ 	]*vinsertf64x2 ymm30\{k7\}\{z\},ymm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 18 f4 7b[ 	]*vinsertf64x2 ymm30,ymm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 31 7b[ 	]*vinsertf64x2 ymm30,ymm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 18 b4 f0 23 01 00 00 7b[ 	]*vinsertf64x2 ymm30,ymm29,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 72 7f 7b[ 	]*vinsertf64x2 ymm30,ymm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 ymm30,ymm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 72 80 7b[ 	]*vinsertf64x2 ymm30,ymm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 ymm30,ymm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 38 f4 ab[ 	]*vinserti64x2 ymm30,ymm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 38 f4 ab[ 	]*vinserti64x2 ymm30\{k7\},ymm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 38 f4 ab[ 	]*vinserti64x2 ymm30\{k7\}\{z\},ymm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 38 f4 7b[ 	]*vinserti64x2 ymm30,ymm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 31 7b[ 	]*vinserti64x2 ymm30,ymm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 38 b4 f0 23 01 00 00 7b[ 	]*vinserti64x2 ymm30,ymm29,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 72 7f 7b[ 	]*vinserti64x2 ymm30,ymm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 ymm30,ymm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 72 80 7b[ 	]*vinserti64x2 ymm30,ymm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 ymm30,ymm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 59 f7[ 	]*vbroadcasti32x2 xmm30,xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 59 f7[ 	]*vbroadcasti32x2 xmm30\{k7\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 59 f7[ 	]*vbroadcasti32x2 xmm30\{k7\}\{z\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 31[ 	]*vbroadcasti32x2 xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 59 b4 f0 23 01 00 00[ 	]*vbroadcasti32x2 xmm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 72 7f[ 	]*vbroadcasti32x2 xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 72 80[ 	]*vbroadcasti32x2 xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 59 f7[ 	]*vbroadcasti32x2 ymm30,xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 59 f7[ 	]*vbroadcasti32x2 ymm30\{k7\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 59 f7[ 	]*vbroadcasti32x2 ymm30\{k7\}\{z\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 31[ 	]*vbroadcasti32x2 ymm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 59 b4 f0 23 01 00 00[ 	]*vbroadcasti32x2 ymm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 72 7f[ 	]*vbroadcasti32x2 ymm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 ymm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 72 80[ 	]*vbroadcasti32x2 ymm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 ymm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 40 f4[ 	]*vpmullq xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 40 f4[ 	]*vpmullq xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 40 f4[ 	]*vpmullq xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 31[ 	]*vpmullq xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 40 b4 f0 23 01 00 00[ 	]*vpmullq xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 31[ 	]*vpmullq xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 72 7f[ 	]*vpmullq xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 b2 00 08 00 00[ 	]*vpmullq xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 72 80[ 	]*vpmullq xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 b2 f0 f7 ff ff[ 	]*vpmullq xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 72 7f[ 	]*vpmullq xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 b2 00 04 00 00[ 	]*vpmullq xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 72 80[ 	]*vpmullq xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 b2 f8 fb ff ff[ 	]*vpmullq xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 40 f4[ 	]*vpmullq ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 40 f4[ 	]*vpmullq ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 40 f4[ 	]*vpmullq ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 31[ 	]*vpmullq ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 40 b4 f0 23 01 00 00[ 	]*vpmullq ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 31[ 	]*vpmullq ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 72 7f[ 	]*vpmullq ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 b2 00 10 00 00[ 	]*vpmullq ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 72 80[ 	]*vpmullq ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 b2 e0 ef ff ff[ 	]*vpmullq ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 72 7f[ 	]*vpmullq ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 b2 00 04 00 00[ 	]*vpmullq ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 72 80[ 	]*vpmullq ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 b2 f8 fb ff ff[ 	]*vpmullq ymm30,ymm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 50 f4 ab[ 	]*vrangepd xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 50 f4 ab[ 	]*vrangepd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 50 f4 ab[ 	]*vrangepd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 50 f4 7b[ 	]*vrangepd xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 31 7b[ 	]*vrangepd xmm30,xmm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 50 b4 f0 23 01 00 00 7b[ 	]*vrangepd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 31 7b[ 	]*vrangepd xmm30,xmm29,QWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 72 7f 7b[ 	]*vrangepd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 b2 00 08 00 00 7b[ 	]*vrangepd xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 72 80 7b[ 	]*vrangepd xmm30,xmm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 b2 f0 f7 ff ff 7b[ 	]*vrangepd xmm30,xmm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 72 7f 7b[ 	]*vrangepd xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 b2 00 04 00 00 7b[ 	]*vrangepd xmm30,xmm29,QWORD BCST \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 72 80 7b[ 	]*vrangepd xmm30,xmm29,QWORD BCST \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 b2 f8 fb ff ff 7b[ 	]*vrangepd xmm30,xmm29,QWORD BCST \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 50 f4 ab[ 	]*vrangepd ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 50 f4 ab[ 	]*vrangepd ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 50 f4 ab[ 	]*vrangepd ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 50 f4 7b[ 	]*vrangepd ymm30,ymm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 31 7b[ 	]*vrangepd ymm30,ymm29,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 50 b4 f0 23 01 00 00 7b[ 	]*vrangepd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 31 7b[ 	]*vrangepd ymm30,ymm29,QWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 72 7f 7b[ 	]*vrangepd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 b2 00 10 00 00 7b[ 	]*vrangepd ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 72 80 7b[ 	]*vrangepd ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 b2 e0 ef ff ff 7b[ 	]*vrangepd ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 72 7f 7b[ 	]*vrangepd ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 b2 00 04 00 00 7b[ 	]*vrangepd ymm30,ymm29,QWORD BCST \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 72 80 7b[ 	]*vrangepd ymm30,ymm29,QWORD BCST \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 b2 f8 fb ff ff 7b[ 	]*vrangepd ymm30,ymm29,QWORD BCST \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 50 f4 ab[ 	]*vrangeps xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 50 f4 ab[ 	]*vrangeps xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 50 f4 ab[ 	]*vrangeps xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 50 f4 7b[ 	]*vrangeps xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 31 7b[ 	]*vrangeps xmm30,xmm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 50 b4 f0 23 01 00 00 7b[ 	]*vrangeps xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 31 7b[ 	]*vrangeps xmm30,xmm29,DWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 72 7f 7b[ 	]*vrangeps xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 b2 00 08 00 00 7b[ 	]*vrangeps xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 72 80 7b[ 	]*vrangeps xmm30,xmm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 b2 f0 f7 ff ff 7b[ 	]*vrangeps xmm30,xmm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 72 7f 7b[ 	]*vrangeps xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 b2 00 02 00 00 7b[ 	]*vrangeps xmm30,xmm29,DWORD BCST \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 72 80 7b[ 	]*vrangeps xmm30,xmm29,DWORD BCST \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 b2 fc fd ff ff 7b[ 	]*vrangeps xmm30,xmm29,DWORD BCST \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 50 f4 ab[ 	]*vrangeps ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 50 f4 ab[ 	]*vrangeps ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 50 f4 ab[ 	]*vrangeps ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 50 f4 7b[ 	]*vrangeps ymm30,ymm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 31 7b[ 	]*vrangeps ymm30,ymm29,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 50 b4 f0 23 01 00 00 7b[ 	]*vrangeps ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 31 7b[ 	]*vrangeps ymm30,ymm29,DWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 72 7f 7b[ 	]*vrangeps ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 b2 00 10 00 00 7b[ 	]*vrangeps ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 72 80 7b[ 	]*vrangeps ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 b2 e0 ef ff ff 7b[ 	]*vrangeps ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 72 7f 7b[ 	]*vrangeps ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 b2 00 02 00 00 7b[ 	]*vrangeps ymm30,ymm29,DWORD BCST \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 72 80 7b[ 	]*vrangeps ymm30,ymm29,DWORD BCST \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 b2 fc fd ff ff 7b[ 	]*vrangeps ymm30,ymm29,DWORD BCST \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 54 f4[ 	]*vandpd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 54 f4[ 	]*vandpd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 54 f4[ 	]*vandpd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 31[ 	]*vandpd xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 54 b4 f0 23 01 00 00[ 	]*vandpd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 31[ 	]*vandpd xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 72 7f[ 	]*vandpd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 b2 00 08 00 00[ 	]*vandpd xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 72 80[ 	]*vandpd xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 b2 f0 f7 ff ff[ 	]*vandpd xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 72 7f[ 	]*vandpd xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 b2 00 04 00 00[ 	]*vandpd xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 72 80[ 	]*vandpd xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 b2 f8 fb ff ff[ 	]*vandpd xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 54 f4[ 	]*vandpd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 54 f4[ 	]*vandpd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 54 f4[ 	]*vandpd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 31[ 	]*vandpd ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 54 b4 f0 23 01 00 00[ 	]*vandpd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 31[ 	]*vandpd ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 72 7f[ 	]*vandpd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 b2 00 10 00 00[ 	]*vandpd ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 72 80[ 	]*vandpd ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 b2 e0 ef ff ff[ 	]*vandpd ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 72 7f[ 	]*vandpd ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 b2 00 04 00 00[ 	]*vandpd ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 72 80[ 	]*vandpd ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 b2 f8 fb ff ff[ 	]*vandpd ymm30,ymm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 54 f4[ 	]*vandps xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 54 f4[ 	]*vandps xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 54 f4[ 	]*vandps xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 31[ 	]*vandps xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 54 b4 f0 23 01 00 00[ 	]*vandps xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 31[ 	]*vandps xmm30,xmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 72 7f[ 	]*vandps xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 b2 00 08 00 00[ 	]*vandps xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 72 80[ 	]*vandps xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 b2 f0 f7 ff ff[ 	]*vandps xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 72 7f[ 	]*vandps xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 b2 00 02 00 00[ 	]*vandps xmm30,xmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 72 80[ 	]*vandps xmm30,xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 b2 fc fd ff ff[ 	]*vandps xmm30,xmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 54 f4[ 	]*vandps ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 54 f4[ 	]*vandps ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 54 f4[ 	]*vandps ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 31[ 	]*vandps ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 54 b4 f0 23 01 00 00[ 	]*vandps ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 31[ 	]*vandps ymm30,ymm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 72 7f[ 	]*vandps ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 b2 00 10 00 00[ 	]*vandps ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 72 80[ 	]*vandps ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 b2 e0 ef ff ff[ 	]*vandps ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 72 7f[ 	]*vandps ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 b2 00 02 00 00[ 	]*vandps ymm30,ymm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 72 80[ 	]*vandps ymm30,ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 b2 fc fd ff ff[ 	]*vandps ymm30,ymm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 55 f4[ 	]*vandnpd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 55 f4[ 	]*vandnpd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 55 f4[ 	]*vandnpd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 31[ 	]*vandnpd xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 55 b4 f0 23 01 00 00[ 	]*vandnpd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 31[ 	]*vandnpd xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 72 7f[ 	]*vandnpd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 b2 00 08 00 00[ 	]*vandnpd xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 72 80[ 	]*vandnpd xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 b2 f0 f7 ff ff[ 	]*vandnpd xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 72 7f[ 	]*vandnpd xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 b2 00 04 00 00[ 	]*vandnpd xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 72 80[ 	]*vandnpd xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 b2 f8 fb ff ff[ 	]*vandnpd xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 55 f4[ 	]*vandnpd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 55 f4[ 	]*vandnpd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 55 f4[ 	]*vandnpd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 31[ 	]*vandnpd ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 55 b4 f0 23 01 00 00[ 	]*vandnpd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 31[ 	]*vandnpd ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 72 7f[ 	]*vandnpd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 b2 00 10 00 00[ 	]*vandnpd ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 72 80[ 	]*vandnpd ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 b2 e0 ef ff ff[ 	]*vandnpd ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 72 7f[ 	]*vandnpd ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 b2 00 04 00 00[ 	]*vandnpd ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 72 80[ 	]*vandnpd ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 b2 f8 fb ff ff[ 	]*vandnpd ymm30,ymm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 55 f4[ 	]*vandnps xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 55 f4[ 	]*vandnps xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 55 f4[ 	]*vandnps xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 31[ 	]*vandnps xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 55 b4 f0 23 01 00 00[ 	]*vandnps xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 31[ 	]*vandnps xmm30,xmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 72 7f[ 	]*vandnps xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 b2 00 08 00 00[ 	]*vandnps xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 72 80[ 	]*vandnps xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 b2 f0 f7 ff ff[ 	]*vandnps xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 72 7f[ 	]*vandnps xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 b2 00 02 00 00[ 	]*vandnps xmm30,xmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 72 80[ 	]*vandnps xmm30,xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 b2 fc fd ff ff[ 	]*vandnps xmm30,xmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 55 f4[ 	]*vandnps ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 55 f4[ 	]*vandnps ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 55 f4[ 	]*vandnps ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 31[ 	]*vandnps ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 55 b4 f0 23 01 00 00[ 	]*vandnps ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 31[ 	]*vandnps ymm30,ymm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 72 7f[ 	]*vandnps ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 b2 00 10 00 00[ 	]*vandnps ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 72 80[ 	]*vandnps ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 b2 e0 ef ff ff[ 	]*vandnps ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 72 7f[ 	]*vandnps ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 b2 00 02 00 00[ 	]*vandnps ymm30,ymm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 72 80[ 	]*vandnps ymm30,ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 b2 fc fd ff ff[ 	]*vandnps ymm30,ymm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 56 f4[ 	]*vorpd  xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 56 f4[ 	]*vorpd  xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 56 f4[ 	]*vorpd  xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 31[ 	]*vorpd  xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 56 b4 f0 23 01 00 00[ 	]*vorpd  xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 31[ 	]*vorpd  xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 72 7f[ 	]*vorpd  xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 b2 00 08 00 00[ 	]*vorpd  xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 72 80[ 	]*vorpd  xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 b2 f0 f7 ff ff[ 	]*vorpd  xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 72 7f[ 	]*vorpd  xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 b2 00 04 00 00[ 	]*vorpd  xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 72 80[ 	]*vorpd  xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 b2 f8 fb ff ff[ 	]*vorpd  xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 56 f4[ 	]*vorpd  ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 56 f4[ 	]*vorpd  ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 56 f4[ 	]*vorpd  ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 31[ 	]*vorpd  ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 56 b4 f0 23 01 00 00[ 	]*vorpd  ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 31[ 	]*vorpd  ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 72 7f[ 	]*vorpd  ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 b2 00 10 00 00[ 	]*vorpd  ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 72 80[ 	]*vorpd  ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 b2 e0 ef ff ff[ 	]*vorpd  ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 72 7f[ 	]*vorpd  ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 b2 00 04 00 00[ 	]*vorpd  ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 72 80[ 	]*vorpd  ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 b2 f8 fb ff ff[ 	]*vorpd  ymm30,ymm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 56 f4[ 	]*vorps  xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 56 f4[ 	]*vorps  xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 56 f4[ 	]*vorps  xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 31[ 	]*vorps  xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 56 b4 f0 23 01 00 00[ 	]*vorps  xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 31[ 	]*vorps  xmm30,xmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 72 7f[ 	]*vorps  xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 b2 00 08 00 00[ 	]*vorps  xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 72 80[ 	]*vorps  xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 b2 f0 f7 ff ff[ 	]*vorps  xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 72 7f[ 	]*vorps  xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 b2 00 02 00 00[ 	]*vorps  xmm30,xmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 72 80[ 	]*vorps  xmm30,xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 b2 fc fd ff ff[ 	]*vorps  xmm30,xmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 56 f4[ 	]*vorps  ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 56 f4[ 	]*vorps  ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 56 f4[ 	]*vorps  ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 31[ 	]*vorps  ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 56 b4 f0 23 01 00 00[ 	]*vorps  ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 31[ 	]*vorps  ymm30,ymm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 72 7f[ 	]*vorps  ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 b2 00 10 00 00[ 	]*vorps  ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 72 80[ 	]*vorps  ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 b2 e0 ef ff ff[ 	]*vorps  ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 72 7f[ 	]*vorps  ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 b2 00 02 00 00[ 	]*vorps  ymm30,ymm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 72 80[ 	]*vorps  ymm30,ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 b2 fc fd ff ff[ 	]*vorps  ymm30,ymm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 57 f4[ 	]*vxorpd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 57 f4[ 	]*vxorpd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 57 f4[ 	]*vxorpd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 31[ 	]*vxorpd xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 57 b4 f0 23 01 00 00[ 	]*vxorpd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 31[ 	]*vxorpd xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 72 7f[ 	]*vxorpd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 b2 00 08 00 00[ 	]*vxorpd xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 72 80[ 	]*vxorpd xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 b2 f0 f7 ff ff[ 	]*vxorpd xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 72 7f[ 	]*vxorpd xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 b2 00 04 00 00[ 	]*vxorpd xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 72 80[ 	]*vxorpd xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 b2 f8 fb ff ff[ 	]*vxorpd xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 57 f4[ 	]*vxorpd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 57 f4[ 	]*vxorpd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 57 f4[ 	]*vxorpd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 31[ 	]*vxorpd ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 57 b4 f0 23 01 00 00[ 	]*vxorpd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 31[ 	]*vxorpd ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 72 7f[ 	]*vxorpd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 b2 00 10 00 00[ 	]*vxorpd ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 72 80[ 	]*vxorpd ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 b2 e0 ef ff ff[ 	]*vxorpd ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 72 7f[ 	]*vxorpd ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 b2 00 04 00 00[ 	]*vxorpd ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 72 80[ 	]*vxorpd ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 b2 f8 fb ff ff[ 	]*vxorpd ymm30,ymm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 57 f4[ 	]*vxorps xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 57 f4[ 	]*vxorps xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 57 f4[ 	]*vxorps xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 31[ 	]*vxorps xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 57 b4 f0 23 01 00 00[ 	]*vxorps xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 31[ 	]*vxorps xmm30,xmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 72 7f[ 	]*vxorps xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 b2 00 08 00 00[ 	]*vxorps xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 72 80[ 	]*vxorps xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 b2 f0 f7 ff ff[ 	]*vxorps xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 72 7f[ 	]*vxorps xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 b2 00 02 00 00[ 	]*vxorps xmm30,xmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 72 80[ 	]*vxorps xmm30,xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 b2 fc fd ff ff[ 	]*vxorps xmm30,xmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 57 f4[ 	]*vxorps ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 57 f4[ 	]*vxorps ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 57 f4[ 	]*vxorps ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 31[ 	]*vxorps ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 57 b4 f0 23 01 00 00[ 	]*vxorps ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 31[ 	]*vxorps ymm30,ymm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 72 7f[ 	]*vxorps ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 b2 00 10 00 00[ 	]*vxorps ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 72 80[ 	]*vxorps ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 b2 e0 ef ff ff[ 	]*vxorps ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 72 7f[ 	]*vxorps ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 b2 00 02 00 00[ 	]*vxorps ymm30,ymm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 72 80[ 	]*vxorps ymm30,ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 b2 fc fd ff ff[ 	]*vxorps ymm30,ymm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 08 56 f5 ab[ 	]*vreducepd xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 0f 56 f5 ab[ 	]*vreducepd xmm30\{k7\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 8f 56 f5 ab[ 	]*vreducepd xmm30\{k7\}\{z\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 08 56 f5 7b[ 	]*vreducepd xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 31 7b[ 	]*vreducepd xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 56 b4 f0 23 01 00 00 7b[ 	]*vreducepd xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 31 7b[ 	]*vreducepd xmm30,QWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 72 7f 7b[ 	]*vreducepd xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 b2 00 08 00 00 7b[ 	]*vreducepd xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 72 80 7b[ 	]*vreducepd xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 b2 f0 f7 ff ff 7b[ 	]*vreducepd xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 72 7f 7b[ 	]*vreducepd xmm30,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 b2 00 04 00 00 7b[ 	]*vreducepd xmm30,QWORD BCST \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 72 80 7b[ 	]*vreducepd xmm30,QWORD BCST \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 b2 f8 fb ff ff 7b[ 	]*vreducepd xmm30,QWORD BCST \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 56 f5 ab[ 	]*vreducepd ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 2f 56 f5 ab[ 	]*vreducepd ymm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd af 56 f5 ab[ 	]*vreducepd ymm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 56 f5 7b[ 	]*vreducepd ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 31 7b[ 	]*vreducepd ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 28 56 b4 f0 23 01 00 00 7b[ 	]*vreducepd ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 31 7b[ 	]*vreducepd ymm30,QWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 72 7f 7b[ 	]*vreducepd ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 b2 00 10 00 00 7b[ 	]*vreducepd ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 72 80 7b[ 	]*vreducepd ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 b2 e0 ef ff ff 7b[ 	]*vreducepd ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 72 7f 7b[ 	]*vreducepd ymm30,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 b2 00 04 00 00 7b[ 	]*vreducepd ymm30,QWORD BCST \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 72 80 7b[ 	]*vreducepd ymm30,QWORD BCST \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 b2 f8 fb ff ff 7b[ 	]*vreducepd ymm30,QWORD BCST \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 08 56 f5 ab[ 	]*vreduceps xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 0f 56 f5 ab[ 	]*vreduceps xmm30\{k7\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 8f 56 f5 ab[ 	]*vreduceps xmm30\{k7\}\{z\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 08 56 f5 7b[ 	]*vreduceps xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 31 7b[ 	]*vreduceps xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 08 56 b4 f0 23 01 00 00 7b[ 	]*vreduceps xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 31 7b[ 	]*vreduceps xmm30,DWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 72 7f 7b[ 	]*vreduceps xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 b2 00 08 00 00 7b[ 	]*vreduceps xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 72 80 7b[ 	]*vreduceps xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 b2 f0 f7 ff ff 7b[ 	]*vreduceps xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 72 7f 7b[ 	]*vreduceps xmm30,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 b2 00 02 00 00 7b[ 	]*vreduceps xmm30,DWORD BCST \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 72 80 7b[ 	]*vreduceps xmm30,DWORD BCST \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 b2 fc fd ff ff 7b[ 	]*vreduceps xmm30,DWORD BCST \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 28 56 f5 ab[ 	]*vreduceps ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 2f 56 f5 ab[ 	]*vreduceps ymm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d af 56 f5 ab[ 	]*vreduceps ymm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 28 56 f5 7b[ 	]*vreduceps ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 31 7b[ 	]*vreduceps ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 28 56 b4 f0 23 01 00 00 7b[ 	]*vreduceps ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 31 7b[ 	]*vreduceps ymm30,DWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 72 7f 7b[ 	]*vreduceps ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 b2 00 10 00 00 7b[ 	]*vreduceps ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 72 80 7b[ 	]*vreduceps ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 b2 e0 ef ff ff 7b[ 	]*vreduceps ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 72 7f 7b[ 	]*vreduceps ymm30,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 b2 00 02 00 00 7b[ 	]*vreduceps ymm30,DWORD BCST \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 72 80 7b[ 	]*vreduceps ymm30,DWORD BCST \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 b2 fc fd ff ff 7b[ 	]*vreduceps ymm30,DWORD BCST \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 29 ab[ 	]*vextractf64x2 XMMWORD PTR \[rcx\],ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 2f 19 29 ab[ 	]*vextractf64x2 XMMWORD PTR \[rcx\]\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 29 7b[ 	]*vextractf64x2 XMMWORD PTR \[rcx\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 28 19 ac f0 23 01 00 00 7b[ 	]*vextractf64x2 XMMWORD PTR \[rax\+r14\*8\+0x123\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 6a 7f 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx\+0x7f0\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 aa 00 08 00 00 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx\+0x800\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 6a 80 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx-0x800\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 aa f0 f7 ff ff 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx-0x810\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 29 ab[ 	]*vextracti64x2 XMMWORD PTR \[rcx\],ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 2f 39 29 ab[ 	]*vextracti64x2 XMMWORD PTR \[rcx\]\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 29 7b[ 	]*vextracti64x2 XMMWORD PTR \[rcx\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 28 39 ac f0 23 01 00 00 7b[ 	]*vextracti64x2 XMMWORD PTR \[rax\+r14\*8\+0x123\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 6a 7f 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx\+0x7f0\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 aa 00 08 00 00 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx\+0x800\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 6a 80 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx-0x800\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 aa f0 f7 ff ff 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx-0x810\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 7a f5[ 	]*vcvttpd2qq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 7a f5[ 	]*vcvttpd2qq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 7a f5[ 	]*vcvttpd2qq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a 31[ 	]*vcvttpd2qq xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 7a b4 f0 23 01 00 00[ 	]*vcvttpd2qq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a 31[ 	]*vcvttpd2qq xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a 72 7f[ 	]*vcvttpd2qq xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a b2 00 08 00 00[ 	]*vcvttpd2qq xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a 72 80[ 	]*vcvttpd2qq xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a b2 f0 f7 ff ff[ 	]*vcvttpd2qq xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a 72 7f[ 	]*vcvttpd2qq xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a b2 00 04 00 00[ 	]*vcvttpd2qq xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a 72 80[ 	]*vcvttpd2qq xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 7a f5[ 	]*vcvttpd2qq ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 7a f5[ 	]*vcvttpd2qq ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 7a f5[ 	]*vcvttpd2qq ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a 31[ 	]*vcvttpd2qq ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 7a b4 f0 23 01 00 00[ 	]*vcvttpd2qq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a 31[ 	]*vcvttpd2qq ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a 72 7f[ 	]*vcvttpd2qq ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a b2 00 10 00 00[ 	]*vcvttpd2qq ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a 72 80[ 	]*vcvttpd2qq ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a b2 e0 ef ff ff[ 	]*vcvttpd2qq ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a 72 7f[ 	]*vcvttpd2qq ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a b2 00 04 00 00[ 	]*vcvttpd2qq ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a 72 80[ 	]*vcvttpd2qq ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 78 f5[ 	]*vcvttpd2uqq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 78 f5[ 	]*vcvttpd2uqq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 78 f5[ 	]*vcvttpd2uqq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 31[ 	]*vcvttpd2uqq xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 78 b4 f0 23 01 00 00[ 	]*vcvttpd2uqq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 31[ 	]*vcvttpd2uqq xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 72 7f[ 	]*vcvttpd2uqq xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 b2 00 08 00 00[ 	]*vcvttpd2uqq xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 72 80[ 	]*vcvttpd2uqq xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 b2 f0 f7 ff ff[ 	]*vcvttpd2uqq xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 72 7f[ 	]*vcvttpd2uqq xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 b2 00 04 00 00[ 	]*vcvttpd2uqq xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 72 80[ 	]*vcvttpd2uqq xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 78 f5[ 	]*vcvttpd2uqq ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 78 f5[ 	]*vcvttpd2uqq ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 78 f5[ 	]*vcvttpd2uqq ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 31[ 	]*vcvttpd2uqq ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 78 b4 f0 23 01 00 00[ 	]*vcvttpd2uqq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 31[ 	]*vcvttpd2uqq ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 72 7f[ 	]*vcvttpd2uqq ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 b2 00 10 00 00[ 	]*vcvttpd2uqq ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 72 80[ 	]*vcvttpd2uqq ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 b2 e0 ef ff ff[ 	]*vcvttpd2uqq ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 72 7f[ 	]*vcvttpd2uqq ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 b2 00 04 00 00[ 	]*vcvttpd2uqq ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 72 80[ 	]*vcvttpd2uqq ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 7a f5[ 	]*vcvttps2qq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 7a f5[ 	]*vcvttps2qq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 7a f5[ 	]*vcvttps2qq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a 31[ 	]*vcvttps2qq xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 7a b4 f0 23 01 00 00[ 	]*vcvttps2qq xmm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a 31[ 	]*vcvttps2qq xmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a 72 7f[ 	]*vcvttps2qq xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a b2 00 04 00 00[ 	]*vcvttps2qq xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a 72 80[ 	]*vcvttps2qq xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a b2 f8 fb ff ff[ 	]*vcvttps2qq xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a 72 7f[ 	]*vcvttps2qq xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a b2 00 02 00 00[ 	]*vcvttps2qq xmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a 72 80[ 	]*vcvttps2qq xmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a b2 fc fd ff ff[ 	]*vcvttps2qq xmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 7a f5[ 	]*vcvttps2qq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 7a f5[ 	]*vcvttps2qq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 7a f5[ 	]*vcvttps2qq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a 31[ 	]*vcvttps2qq ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 7a b4 f0 23 01 00 00[ 	]*vcvttps2qq ymm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a 31[ 	]*vcvttps2qq ymm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a 72 7f[ 	]*vcvttps2qq ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a b2 00 08 00 00[ 	]*vcvttps2qq ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a 72 80[ 	]*vcvttps2qq ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a b2 f0 f7 ff ff[ 	]*vcvttps2qq ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a 72 7f[ 	]*vcvttps2qq ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a b2 00 02 00 00[ 	]*vcvttps2qq ymm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a 72 80[ 	]*vcvttps2qq ymm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a b2 fc fd ff ff[ 	]*vcvttps2qq ymm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 78 f5[ 	]*vcvttps2uqq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 78 f5[ 	]*vcvttps2uqq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 78 f5[ 	]*vcvttps2uqq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 31[ 	]*vcvttps2uqq xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 78 b4 f0 23 01 00 00[ 	]*vcvttps2uqq xmm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 31[ 	]*vcvttps2uqq xmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 72 7f[ 	]*vcvttps2uqq xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 b2 00 04 00 00[ 	]*vcvttps2uqq xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 72 80[ 	]*vcvttps2uqq xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 b2 f8 fb ff ff[ 	]*vcvttps2uqq xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 72 7f[ 	]*vcvttps2uqq xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 b2 00 02 00 00[ 	]*vcvttps2uqq xmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 72 80[ 	]*vcvttps2uqq xmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 b2 fc fd ff ff[ 	]*vcvttps2uqq xmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 78 f5[ 	]*vcvttps2uqq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 78 f5[ 	]*vcvttps2uqq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 78 f5[ 	]*vcvttps2uqq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 31[ 	]*vcvttps2uqq ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 78 b4 f0 23 01 00 00[ 	]*vcvttps2uqq ymm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 31[ 	]*vcvttps2uqq ymm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 72 7f[ 	]*vcvttps2uqq ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 b2 00 08 00 00[ 	]*vcvttps2uqq ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 72 80[ 	]*vcvttps2uqq ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 b2 f0 f7 ff ff[ 	]*vcvttps2uqq ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 72 7f[ 	]*vcvttps2uqq ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 b2 00 02 00 00[ 	]*vcvttps2uqq ymm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 72 80[ 	]*vcvttps2uqq ymm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 b2 fc fd ff ff[ 	]*vcvttps2uqq ymm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 08 39 ee[ 	]*vpmovd2m k5,xmm30
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 28 39 ee[ 	]*vpmovd2m k5,ymm30
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 08 39 ee[ 	]*vpmovq2m k5,xmm30
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 28 39 ee[ 	]*vpmovq2m k5,ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 38 f5[ 	]*vpmovm2d xmm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 38 f5[ 	]*vpmovm2d ymm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 08 38 f5[ 	]*vpmovm2q xmm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 28 38 f5[ 	]*vpmovm2q ymm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a 31[ 	]*vbroadcastf64x2 ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 1a 31[ 	]*vbroadcastf64x2 ymm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd af 1a 31[ 	]*vbroadcastf64x2 ymm30\{k7\}\{z\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 1a b4 f0 34 12 00 00[ 	]*vbroadcastf64x2 ymm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a 72 7f[ 	]*vbroadcastf64x2 ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a 72 80[ 	]*vbroadcastf64x2 ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a 31[ 	]*vbroadcasti64x2 ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 2f 5a 31[ 	]*vbroadcasti64x2 ymm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd af 5a 31[ 	]*vbroadcasti64x2 ymm30\{k7\}\{z\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 5a b4 f0 34 12 00 00[ 	]*vbroadcasti64x2 ymm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a 72 7f[ 	]*vbroadcasti64x2 ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a 72 80[ 	]*vbroadcasti64x2 ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 19 f7[ 	]*vbroadcastf32x2 ymm30,xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 19 f7[ 	]*vbroadcastf32x2 ymm30\{k7\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 19 f7[ 	]*vbroadcastf32x2 ymm30\{k7\}\{z\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 31[ 	]*vbroadcastf32x2 ymm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 19 b4 f0 34 12 00 00[ 	]*vbroadcastf32x2 ymm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 72 7f[ 	]*vbroadcastf32x2 ymm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 ymm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 72 80[ 	]*vbroadcastf32x2 ymm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 ymm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 7b f5[ 	]*vcvtpd2qq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 7b f5[ 	]*vcvtpd2qq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 7b f5[ 	]*vcvtpd2qq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b 31[ 	]*vcvtpd2qq xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 7b b4 f0 34 12 00 00[ 	]*vcvtpd2qq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b 31[ 	]*vcvtpd2qq xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b 72 7f[ 	]*vcvtpd2qq xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b b2 00 08 00 00[ 	]*vcvtpd2qq xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b 72 80[ 	]*vcvtpd2qq xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7b b2 f0 f7 ff ff[ 	]*vcvtpd2qq xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b 72 7f[ 	]*vcvtpd2qq xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b b2 00 04 00 00[ 	]*vcvtpd2qq xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b 72 80[ 	]*vcvtpd2qq xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 7b f5[ 	]*vcvtpd2qq ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 7b f5[ 	]*vcvtpd2qq ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 7b f5[ 	]*vcvtpd2qq ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b 31[ 	]*vcvtpd2qq ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 7b b4 f0 34 12 00 00[ 	]*vcvtpd2qq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b 31[ 	]*vcvtpd2qq ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b 72 7f[ 	]*vcvtpd2qq ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b b2 00 10 00 00[ 	]*vcvtpd2qq ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b 72 80[ 	]*vcvtpd2qq ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7b b2 e0 ef ff ff[ 	]*vcvtpd2qq ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b 72 7f[ 	]*vcvtpd2qq ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b b2 00 04 00 00[ 	]*vcvtpd2qq ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b 72 80[ 	]*vcvtpd2qq ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 79 f5[ 	]*vcvtpd2uqq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 79 f5[ 	]*vcvtpd2uqq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 79 f5[ 	]*vcvtpd2uqq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 31[ 	]*vcvtpd2uqq xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 79 b4 f0 34 12 00 00[ 	]*vcvtpd2uqq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 31[ 	]*vcvtpd2uqq xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 72 7f[ 	]*vcvtpd2uqq xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 b2 00 08 00 00[ 	]*vcvtpd2uqq xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 72 80[ 	]*vcvtpd2uqq xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 79 b2 f0 f7 ff ff[ 	]*vcvtpd2uqq xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 72 7f[ 	]*vcvtpd2uqq xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 b2 00 04 00 00[ 	]*vcvtpd2uqq xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 72 80[ 	]*vcvtpd2uqq xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 79 f5[ 	]*vcvtpd2uqq ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 79 f5[ 	]*vcvtpd2uqq ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 79 f5[ 	]*vcvtpd2uqq ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 31[ 	]*vcvtpd2uqq ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 79 b4 f0 34 12 00 00[ 	]*vcvtpd2uqq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 31[ 	]*vcvtpd2uqq ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 72 7f[ 	]*vcvtpd2uqq ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 b2 00 10 00 00[ 	]*vcvtpd2uqq ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 72 80[ 	]*vcvtpd2uqq ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 79 b2 e0 ef ff ff[ 	]*vcvtpd2uqq ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 72 7f[ 	]*vcvtpd2uqq ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 b2 00 04 00 00[ 	]*vcvtpd2uqq ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 72 80[ 	]*vcvtpd2uqq ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 7b f5[ 	]*vcvtps2qq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 7b f5[ 	]*vcvtps2qq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 7b f5[ 	]*vcvtps2qq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b 31[ 	]*vcvtps2qq xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 7b b4 f0 34 12 00 00[ 	]*vcvtps2qq xmm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b 31[ 	]*vcvtps2qq xmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b 72 7f[ 	]*vcvtps2qq xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b b2 00 04 00 00[ 	]*vcvtps2qq xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b 72 80[ 	]*vcvtps2qq xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7b b2 f8 fb ff ff[ 	]*vcvtps2qq xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b 72 7f[ 	]*vcvtps2qq xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b b2 00 02 00 00[ 	]*vcvtps2qq xmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b 72 80[ 	]*vcvtps2qq xmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b b2 fc fd ff ff[ 	]*vcvtps2qq xmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7b 72 7f[ 	]*vcvtps2qq xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 7b f5[ 	]*vcvtps2qq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 7b f5[ 	]*vcvtps2qq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 7b f5[ 	]*vcvtps2qq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b 31[ 	]*vcvtps2qq ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 7b b4 f0 34 12 00 00[ 	]*vcvtps2qq ymm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b 31[ 	]*vcvtps2qq ymm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b 72 7f[ 	]*vcvtps2qq ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b b2 00 08 00 00[ 	]*vcvtps2qq ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b 72 80[ 	]*vcvtps2qq ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7b b2 f0 f7 ff ff[ 	]*vcvtps2qq ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b 72 7f[ 	]*vcvtps2qq ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b b2 00 02 00 00[ 	]*vcvtps2qq ymm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b 72 80[ 	]*vcvtps2qq ymm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b b2 fc fd ff ff[ 	]*vcvtps2qq ymm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7b 72 7f[ 	]*vcvtps2qq ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 79 f5[ 	]*vcvtps2uqq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 79 f5[ 	]*vcvtps2uqq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 79 f5[ 	]*vcvtps2uqq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 31[ 	]*vcvtps2uqq xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 79 b4 f0 34 12 00 00[ 	]*vcvtps2uqq xmm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 31[ 	]*vcvtps2uqq xmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 72 7f[ 	]*vcvtps2uqq xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 b2 00 04 00 00[ 	]*vcvtps2uqq xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 72 80[ 	]*vcvtps2uqq xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 79 b2 f8 fb ff ff[ 	]*vcvtps2uqq xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 72 7f[ 	]*vcvtps2uqq xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 b2 00 02 00 00[ 	]*vcvtps2uqq xmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 72 80[ 	]*vcvtps2uqq xmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 b2 fc fd ff ff[ 	]*vcvtps2uqq xmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 79 72 7f[ 	]*vcvtps2uqq xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 79 f5[ 	]*vcvtps2uqq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 79 f5[ 	]*vcvtps2uqq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 79 f5[ 	]*vcvtps2uqq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 31[ 	]*vcvtps2uqq ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 79 b4 f0 34 12 00 00[ 	]*vcvtps2uqq ymm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 31[ 	]*vcvtps2uqq ymm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 72 7f[ 	]*vcvtps2uqq ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 b2 00 08 00 00[ 	]*vcvtps2uqq ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 72 80[ 	]*vcvtps2uqq ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 79 b2 f0 f7 ff ff[ 	]*vcvtps2uqq ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 72 7f[ 	]*vcvtps2uqq ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 b2 00 02 00 00[ 	]*vcvtps2uqq ymm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 72 80[ 	]*vcvtps2uqq ymm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 b2 fc fd ff ff[ 	]*vcvtps2uqq ymm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 79 72 7f[ 	]*vcvtps2uqq ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 08 e6 f5[ 	]*vcvtqq2pd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 0f e6 f5[ 	]*vcvtqq2pd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 8f e6 f5[ 	]*vcvtqq2pd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 31[ 	]*vcvtqq2pd xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 08 e6 b4 f0 34 12 00 00[ 	]*vcvtqq2pd xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 31[ 	]*vcvtqq2pd xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 72 7f[ 	]*vcvtqq2pd xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 b2 00 08 00 00[ 	]*vcvtqq2pd xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 72 80[ 	]*vcvtqq2pd xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 e6 b2 f0 f7 ff ff[ 	]*vcvtqq2pd xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 72 7f[ 	]*vcvtqq2pd xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 b2 00 04 00 00[ 	]*vcvtqq2pd xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 72 80[ 	]*vcvtqq2pd xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 28 e6 f5[ 	]*vcvtqq2pd ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 2f e6 f5[ 	]*vcvtqq2pd ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe af e6 f5[ 	]*vcvtqq2pd ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 31[ 	]*vcvtqq2pd ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 28 e6 b4 f0 34 12 00 00[ 	]*vcvtqq2pd ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 31[ 	]*vcvtqq2pd ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 72 7f[ 	]*vcvtqq2pd ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 b2 00 10 00 00[ 	]*vcvtqq2pd ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 72 80[ 	]*vcvtqq2pd ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 e6 b2 e0 ef ff ff[ 	]*vcvtqq2pd ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 72 7f[ 	]*vcvtqq2pd ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 b2 00 04 00 00[ 	]*vcvtqq2pd ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 72 80[ 	]*vcvtqq2pd ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 08 5b f5[ 	]*vcvtqq2ps xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 0f 5b f5[ 	]*vcvtqq2ps xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 8f 5b f5[ 	]*vcvtqq2ps xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b 31[ 	]*vcvtqq2ps xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fc 08 5b b4 f0 34 12 00 00[ 	]*vcvtqq2ps xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b 31[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rcx\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b 72 7f[ 	]*vcvtqq2ps xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b b2 00 08 00 00[ 	]*vcvtqq2ps xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b 72 80[ 	]*vcvtqq2ps xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 08 5b b2 f0 f7 ff ff[ 	]*vcvtqq2ps xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b 72 7f[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx\+0x3f8\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b b2 00 04 00 00[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx\+0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b 72 80[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 18 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx-0x408\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 28 5b f5[ 	]*vcvtqq2ps xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 2f 5b f5[ 	]*vcvtqq2ps xmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fc af 5b f5[ 	]*vcvtqq2ps xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b 31[ 	]*vcvtqq2ps xmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fc 28 5b b4 f0 34 12 00 00[ 	]*vcvtqq2ps xmm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b 31[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rcx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b 72 7f[ 	]*vcvtqq2ps xmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b b2 00 10 00 00[ 	]*vcvtqq2ps xmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b 72 80[ 	]*vcvtqq2ps xmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 28 5b b2 e0 ef ff ff[ 	]*vcvtqq2ps xmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b 72 7f[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx\+0x3f8\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b b2 00 04 00 00[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx\+0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b 72 80[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 38 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps xmm30,QWORD BCST \[rdx-0x408\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 08 7a f5[ 	]*vcvtuqq2pd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 0f 7a f5[ 	]*vcvtuqq2pd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 8f 7a f5[ 	]*vcvtuqq2pd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a 31[ 	]*vcvtuqq2pd xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 08 7a b4 f0 34 12 00 00[ 	]*vcvtuqq2pd xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a 31[ 	]*vcvtuqq2pd xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a 72 7f[ 	]*vcvtuqq2pd xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a b2 00 08 00 00[ 	]*vcvtuqq2pd xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a 72 80[ 	]*vcvtuqq2pd xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 08 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2pd xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a 72 7f[ 	]*vcvtuqq2pd xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a b2 00 04 00 00[ 	]*vcvtuqq2pd xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a 72 80[ 	]*vcvtuqq2pd xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 18 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 28 7a f5[ 	]*vcvtuqq2pd ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 2f 7a f5[ 	]*vcvtuqq2pd ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe af 7a f5[ 	]*vcvtuqq2pd ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a 31[ 	]*vcvtuqq2pd ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 28 7a b4 f0 34 12 00 00[ 	]*vcvtuqq2pd ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a 31[ 	]*vcvtuqq2pd ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a 72 7f[ 	]*vcvtuqq2pd ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a b2 00 10 00 00[ 	]*vcvtuqq2pd ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a 72 80[ 	]*vcvtuqq2pd ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 28 7a b2 e0 ef ff ff[ 	]*vcvtuqq2pd ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a 72 7f[ 	]*vcvtuqq2pd ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a b2 00 04 00 00[ 	]*vcvtuqq2pd ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a 72 80[ 	]*vcvtuqq2pd ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 38 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 7a f5[ 	]*vcvtuqq2ps xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 7a f5[ 	]*vcvtuqq2ps xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 7a f5[ 	]*vcvtuqq2ps xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a 31[ 	]*vcvtuqq2ps xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 08 7a b4 f0 34 12 00 00[ 	]*vcvtuqq2ps xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a 31[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rcx\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a 72 7f[ 	]*vcvtuqq2ps xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a b2 00 08 00 00[ 	]*vcvtuqq2ps xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a 72 80[ 	]*vcvtuqq2ps xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2ps xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a 72 7f[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx\+0x3f8\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a b2 00 04 00 00[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx\+0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a 72 80[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 18 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx-0x408\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 7a f5[ 	]*vcvtuqq2ps xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 7a f5[ 	]*vcvtuqq2ps xmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 7a f5[ 	]*vcvtuqq2ps xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a 31[ 	]*vcvtuqq2ps xmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 28 7a b4 f0 34 12 00 00[ 	]*vcvtuqq2ps xmm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a 31[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rcx\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a 72 7f[ 	]*vcvtuqq2ps xmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a b2 00 10 00 00[ 	]*vcvtuqq2ps xmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a 72 80[ 	]*vcvtuqq2ps xmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7a b2 e0 ef ff ff[ 	]*vcvtuqq2ps xmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a 72 7f[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx\+0x3f8\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a b2 00 04 00 00[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx\+0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a 72 80[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 38 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps xmm30,QWORD BCST \[rdx-0x408\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 19 ee ab[ 	]*vextractf64x2 xmm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 2f 19 ee ab[ 	]*vextractf64x2 xmm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd af 19 ee ab[ 	]*vextractf64x2 xmm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 19 ee 7b[ 	]*vextractf64x2 xmm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 39 ee ab[ 	]*vextracti64x2 xmm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 2f 39 ee ab[ 	]*vextracti64x2 xmm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd af 39 ee ab[ 	]*vextracti64x2 xmm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 39 ee 7b[ 	]*vextracti64x2 xmm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 66 ee ab[ 	]*vfpclasspd k5,xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 0f 66 ee ab[ 	]*vfpclasspd k5\{k7\},xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 66 ee 7b[ 	]*vfpclasspd k5,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 29 7b[ 	]*vfpclasspd k5,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 08 66 ac f0 34 12 00 00 7b[ 	]*vfpclasspd k5,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 29 7b[ 	]*vfpclasspd k5,QWORD BCST \[rcx\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 6a 7f 7b[ 	]*vfpclasspd k5,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 aa 00 08 00 00 7b[ 	]*vfpclasspd k5,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 6a 80 7b[ 	]*vfpclasspd k5,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 66 aa f0 f7 ff ff 7b[ 	]*vfpclasspd k5,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 6a 7f 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx\+0x3f8\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 aa 00 04 00 00 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx\+0x400\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 6a 80 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx-0x400\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx-0x408\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 28 66 ee ab[ 	]*vfpclasspd k5,ymm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 2f 66 ee ab[ 	]*vfpclasspd k5\{k7\},ymm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 28 66 ee 7b[ 	]*vfpclasspd k5,ymm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 29 7b[ 	]*vfpclasspd k5,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 28 66 ac f0 34 12 00 00 7b[ 	]*vfpclasspd k5,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 29 7b[ 	]*vfpclasspd k5,QWORD BCST \[rcx\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 6a 7f 7b[ 	]*vfpclasspd k5,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 aa 00 10 00 00 7b[ 	]*vfpclasspd k5,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 6a 80 7b[ 	]*vfpclasspd k5,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 28 66 aa e0 ef ff ff 7b[ 	]*vfpclasspd k5,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 6a 7f 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx\+0x3f8\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 aa 00 04 00 00 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx\+0x400\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 6a 80 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx-0x400\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 38 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx-0x408\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 66 ee ab[ 	]*vfpclassps k5,xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 0f 66 ee ab[ 	]*vfpclassps k5\{k7\},xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 66 ee 7b[ 	]*vfpclassps k5,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 29 7b[ 	]*vfpclassps k5,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 08 66 ac f0 34 12 00 00 7b[ 	]*vfpclassps k5,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 29 7b[ 	]*vfpclassps k5,DWORD BCST \[rcx\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 6a 7f 7b[ 	]*vfpclassps k5,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 aa 00 08 00 00 7b[ 	]*vfpclassps k5,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 6a 80 7b[ 	]*vfpclassps k5,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 66 aa f0 f7 ff ff 7b[ 	]*vfpclassps k5,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 6a 7f 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx\+0x1fc\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 aa 00 02 00 00 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx\+0x200\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 6a 80 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx-0x200\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 66 aa fc fd ff ff 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx-0x204\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 28 66 ee ab[ 	]*vfpclassps k5,ymm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 2f 66 ee ab[ 	]*vfpclassps k5\{k7\},ymm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 28 66 ee 7b[ 	]*vfpclassps k5,ymm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 29 7b[ 	]*vfpclassps k5,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 28 66 ac f0 34 12 00 00 7b[ 	]*vfpclassps k5,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 29 7b[ 	]*vfpclassps k5,DWORD BCST \[rcx\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 6a 7f 7b[ 	]*vfpclassps k5,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 aa 00 10 00 00 7b[ 	]*vfpclassps k5,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 6a 80 7b[ 	]*vfpclassps k5,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 28 66 aa e0 ef ff ff 7b[ 	]*vfpclassps k5,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 6a 7f 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx\+0x1fc\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 aa 00 02 00 00 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx\+0x200\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 6a 80 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx-0x200\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 38 66 aa fc fd ff ff 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx-0x204\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 18 f4 ab[ 	]*vinsertf64x2 ymm30,ymm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 18 f4 ab[ 	]*vinsertf64x2 ymm30\{k7\},ymm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 18 f4 ab[ 	]*vinsertf64x2 ymm30\{k7\}\{z\},ymm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 18 f4 7b[ 	]*vinsertf64x2 ymm30,ymm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 31 7b[ 	]*vinsertf64x2 ymm30,ymm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 18 b4 f0 34 12 00 00 7b[ 	]*vinsertf64x2 ymm30,ymm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 72 7f 7b[ 	]*vinsertf64x2 ymm30,ymm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 ymm30,ymm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 72 80 7b[ 	]*vinsertf64x2 ymm30,ymm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 ymm30,ymm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 38 f4 ab[ 	]*vinserti64x2 ymm30,ymm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 38 f4 ab[ 	]*vinserti64x2 ymm30\{k7\},ymm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 38 f4 ab[ 	]*vinserti64x2 ymm30\{k7\}\{z\},ymm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 38 f4 7b[ 	]*vinserti64x2 ymm30,ymm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 31 7b[ 	]*vinserti64x2 ymm30,ymm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 38 b4 f0 34 12 00 00 7b[ 	]*vinserti64x2 ymm30,ymm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 72 7f 7b[ 	]*vinserti64x2 ymm30,ymm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 ymm30,ymm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 72 80 7b[ 	]*vinserti64x2 ymm30,ymm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 ymm30,ymm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 59 f7[ 	]*vbroadcasti32x2 xmm30,xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 59 f7[ 	]*vbroadcasti32x2 xmm30\{k7\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 59 f7[ 	]*vbroadcasti32x2 xmm30\{k7\}\{z\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 31[ 	]*vbroadcasti32x2 xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 59 b4 f0 34 12 00 00[ 	]*vbroadcasti32x2 xmm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 72 7f[ 	]*vbroadcasti32x2 xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 72 80[ 	]*vbroadcasti32x2 xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 59 f7[ 	]*vbroadcasti32x2 ymm30,xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 59 f7[ 	]*vbroadcasti32x2 ymm30\{k7\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 59 f7[ 	]*vbroadcasti32x2 ymm30\{k7\}\{z\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 31[ 	]*vbroadcasti32x2 ymm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 59 b4 f0 34 12 00 00[ 	]*vbroadcasti32x2 ymm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 72 7f[ 	]*vbroadcasti32x2 ymm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 ymm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 72 80[ 	]*vbroadcasti32x2 ymm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 ymm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 40 f4[ 	]*vpmullq xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 40 f4[ 	]*vpmullq xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 40 f4[ 	]*vpmullq xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 31[ 	]*vpmullq xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 40 b4 f0 34 12 00 00[ 	]*vpmullq xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 31[ 	]*vpmullq xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 72 7f[ 	]*vpmullq xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 b2 00 08 00 00[ 	]*vpmullq xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 72 80[ 	]*vpmullq xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 40 b2 f0 f7 ff ff[ 	]*vpmullq xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 72 7f[ 	]*vpmullq xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 b2 00 04 00 00[ 	]*vpmullq xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 72 80[ 	]*vpmullq xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 40 b2 f8 fb ff ff[ 	]*vpmullq xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 40 f4[ 	]*vpmullq ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 40 f4[ 	]*vpmullq ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 40 f4[ 	]*vpmullq ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 31[ 	]*vpmullq ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 40 b4 f0 34 12 00 00[ 	]*vpmullq ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 31[ 	]*vpmullq ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 72 7f[ 	]*vpmullq ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 b2 00 10 00 00[ 	]*vpmullq ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 72 80[ 	]*vpmullq ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 40 b2 e0 ef ff ff[ 	]*vpmullq ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 72 7f[ 	]*vpmullq ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 b2 00 04 00 00[ 	]*vpmullq ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 72 80[ 	]*vpmullq ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 40 b2 f8 fb ff ff[ 	]*vpmullq ymm30,ymm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 50 f4 ab[ 	]*vrangepd xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 50 f4 ab[ 	]*vrangepd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 50 f4 ab[ 	]*vrangepd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 50 f4 7b[ 	]*vrangepd xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 31 7b[ 	]*vrangepd xmm30,xmm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 50 b4 f0 34 12 00 00 7b[ 	]*vrangepd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 31 7b[ 	]*vrangepd xmm30,xmm29,QWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 72 7f 7b[ 	]*vrangepd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 b2 00 08 00 00 7b[ 	]*vrangepd xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 72 80 7b[ 	]*vrangepd xmm30,xmm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 50 b2 f0 f7 ff ff 7b[ 	]*vrangepd xmm30,xmm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 72 7f 7b[ 	]*vrangepd xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 b2 00 04 00 00 7b[ 	]*vrangepd xmm30,xmm29,QWORD BCST \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 72 80 7b[ 	]*vrangepd xmm30,xmm29,QWORD BCST \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 10 50 b2 f8 fb ff ff 7b[ 	]*vrangepd xmm30,xmm29,QWORD BCST \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 50 f4 ab[ 	]*vrangepd ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 27 50 f4 ab[ 	]*vrangepd ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 a7 50 f4 ab[ 	]*vrangepd ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 20 50 f4 7b[ 	]*vrangepd ymm30,ymm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 31 7b[ 	]*vrangepd ymm30,ymm29,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 20 50 b4 f0 34 12 00 00 7b[ 	]*vrangepd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 31 7b[ 	]*vrangepd ymm30,ymm29,QWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 72 7f 7b[ 	]*vrangepd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 b2 00 10 00 00 7b[ 	]*vrangepd ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 72 80 7b[ 	]*vrangepd ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 20 50 b2 e0 ef ff ff 7b[ 	]*vrangepd ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 72 7f 7b[ 	]*vrangepd ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 b2 00 04 00 00 7b[ 	]*vrangepd ymm30,ymm29,QWORD BCST \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 72 80 7b[ 	]*vrangepd ymm30,ymm29,QWORD BCST \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 30 50 b2 f8 fb ff ff 7b[ 	]*vrangepd ymm30,ymm29,QWORD BCST \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 50 f4 ab[ 	]*vrangeps xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 50 f4 ab[ 	]*vrangeps xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 50 f4 ab[ 	]*vrangeps xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 50 f4 7b[ 	]*vrangeps xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 31 7b[ 	]*vrangeps xmm30,xmm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 50 b4 f0 34 12 00 00 7b[ 	]*vrangeps xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 31 7b[ 	]*vrangeps xmm30,xmm29,DWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 72 7f 7b[ 	]*vrangeps xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 b2 00 08 00 00 7b[ 	]*vrangeps xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 72 80 7b[ 	]*vrangeps xmm30,xmm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 50 b2 f0 f7 ff ff 7b[ 	]*vrangeps xmm30,xmm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 72 7f 7b[ 	]*vrangeps xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 b2 00 02 00 00 7b[ 	]*vrangeps xmm30,xmm29,DWORD BCST \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 72 80 7b[ 	]*vrangeps xmm30,xmm29,DWORD BCST \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 10 50 b2 fc fd ff ff 7b[ 	]*vrangeps xmm30,xmm29,DWORD BCST \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 50 f4 ab[ 	]*vrangeps ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 50 f4 ab[ 	]*vrangeps ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 50 f4 ab[ 	]*vrangeps ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 50 f4 7b[ 	]*vrangeps ymm30,ymm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 31 7b[ 	]*vrangeps ymm30,ymm29,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 50 b4 f0 34 12 00 00 7b[ 	]*vrangeps ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 31 7b[ 	]*vrangeps ymm30,ymm29,DWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 72 7f 7b[ 	]*vrangeps ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 b2 00 10 00 00 7b[ 	]*vrangeps ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 72 80 7b[ 	]*vrangeps ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 50 b2 e0 ef ff ff 7b[ 	]*vrangeps ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 72 7f 7b[ 	]*vrangeps ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 b2 00 02 00 00 7b[ 	]*vrangeps ymm30,ymm29,DWORD BCST \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 72 80 7b[ 	]*vrangeps ymm30,ymm29,DWORD BCST \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 30 50 b2 fc fd ff ff 7b[ 	]*vrangeps ymm30,ymm29,DWORD BCST \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 54 f4[ 	]*vandpd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 54 f4[ 	]*vandpd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 54 f4[ 	]*vandpd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 31[ 	]*vandpd xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 54 b4 f0 34 12 00 00[ 	]*vandpd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 31[ 	]*vandpd xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 72 7f[ 	]*vandpd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 b2 00 08 00 00[ 	]*vandpd xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 72 80[ 	]*vandpd xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 54 b2 f0 f7 ff ff[ 	]*vandpd xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 72 7f[ 	]*vandpd xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 b2 00 04 00 00[ 	]*vandpd xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 72 80[ 	]*vandpd xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 54 b2 f8 fb ff ff[ 	]*vandpd xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 54 f4[ 	]*vandpd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 54 f4[ 	]*vandpd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 54 f4[ 	]*vandpd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 31[ 	]*vandpd ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 54 b4 f0 34 12 00 00[ 	]*vandpd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 31[ 	]*vandpd ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 72 7f[ 	]*vandpd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 b2 00 10 00 00[ 	]*vandpd ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 72 80[ 	]*vandpd ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 54 b2 e0 ef ff ff[ 	]*vandpd ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 72 7f[ 	]*vandpd ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 b2 00 04 00 00[ 	]*vandpd ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 72 80[ 	]*vandpd ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 54 b2 f8 fb ff ff[ 	]*vandpd ymm30,ymm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 54 f4[ 	]*vandps xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 54 f4[ 	]*vandps xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 54 f4[ 	]*vandps xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 31[ 	]*vandps xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 54 b4 f0 34 12 00 00[ 	]*vandps xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 31[ 	]*vandps xmm30,xmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 72 7f[ 	]*vandps xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 b2 00 08 00 00[ 	]*vandps xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 72 80[ 	]*vandps xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 54 b2 f0 f7 ff ff[ 	]*vandps xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 72 7f[ 	]*vandps xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 b2 00 02 00 00[ 	]*vandps xmm30,xmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 72 80[ 	]*vandps xmm30,xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 54 b2 fc fd ff ff[ 	]*vandps xmm30,xmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 54 f4[ 	]*vandps ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 54 f4[ 	]*vandps ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 54 f4[ 	]*vandps ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 31[ 	]*vandps ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 54 b4 f0 34 12 00 00[ 	]*vandps ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 31[ 	]*vandps ymm30,ymm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 72 7f[ 	]*vandps ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 b2 00 10 00 00[ 	]*vandps ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 72 80[ 	]*vandps ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 54 b2 e0 ef ff ff[ 	]*vandps ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 72 7f[ 	]*vandps ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 b2 00 02 00 00[ 	]*vandps ymm30,ymm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 72 80[ 	]*vandps ymm30,ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 54 b2 fc fd ff ff[ 	]*vandps ymm30,ymm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 55 f4[ 	]*vandnpd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 55 f4[ 	]*vandnpd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 55 f4[ 	]*vandnpd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 31[ 	]*vandnpd xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 55 b4 f0 34 12 00 00[ 	]*vandnpd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 31[ 	]*vandnpd xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 72 7f[ 	]*vandnpd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 b2 00 08 00 00[ 	]*vandnpd xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 72 80[ 	]*vandnpd xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 55 b2 f0 f7 ff ff[ 	]*vandnpd xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 72 7f[ 	]*vandnpd xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 b2 00 04 00 00[ 	]*vandnpd xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 72 80[ 	]*vandnpd xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 55 b2 f8 fb ff ff[ 	]*vandnpd xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 55 f4[ 	]*vandnpd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 55 f4[ 	]*vandnpd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 55 f4[ 	]*vandnpd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 31[ 	]*vandnpd ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 55 b4 f0 34 12 00 00[ 	]*vandnpd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 31[ 	]*vandnpd ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 72 7f[ 	]*vandnpd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 b2 00 10 00 00[ 	]*vandnpd ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 72 80[ 	]*vandnpd ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 55 b2 e0 ef ff ff[ 	]*vandnpd ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 72 7f[ 	]*vandnpd ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 b2 00 04 00 00[ 	]*vandnpd ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 72 80[ 	]*vandnpd ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 55 b2 f8 fb ff ff[ 	]*vandnpd ymm30,ymm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 55 f4[ 	]*vandnps xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 55 f4[ 	]*vandnps xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 55 f4[ 	]*vandnps xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 31[ 	]*vandnps xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 55 b4 f0 34 12 00 00[ 	]*vandnps xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 31[ 	]*vandnps xmm30,xmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 72 7f[ 	]*vandnps xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 b2 00 08 00 00[ 	]*vandnps xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 72 80[ 	]*vandnps xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 55 b2 f0 f7 ff ff[ 	]*vandnps xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 72 7f[ 	]*vandnps xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 b2 00 02 00 00[ 	]*vandnps xmm30,xmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 72 80[ 	]*vandnps xmm30,xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 55 b2 fc fd ff ff[ 	]*vandnps xmm30,xmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 55 f4[ 	]*vandnps ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 55 f4[ 	]*vandnps ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 55 f4[ 	]*vandnps ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 31[ 	]*vandnps ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 55 b4 f0 34 12 00 00[ 	]*vandnps ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 31[ 	]*vandnps ymm30,ymm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 72 7f[ 	]*vandnps ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 b2 00 10 00 00[ 	]*vandnps ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 72 80[ 	]*vandnps ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 55 b2 e0 ef ff ff[ 	]*vandnps ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 72 7f[ 	]*vandnps ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 b2 00 02 00 00[ 	]*vandnps ymm30,ymm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 72 80[ 	]*vandnps ymm30,ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 55 b2 fc fd ff ff[ 	]*vandnps ymm30,ymm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 56 f4[ 	]*vorpd  xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 56 f4[ 	]*vorpd  xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 56 f4[ 	]*vorpd  xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 31[ 	]*vorpd  xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 56 b4 f0 34 12 00 00[ 	]*vorpd  xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 31[ 	]*vorpd  xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 72 7f[ 	]*vorpd  xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 b2 00 08 00 00[ 	]*vorpd  xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 72 80[ 	]*vorpd  xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 56 b2 f0 f7 ff ff[ 	]*vorpd  xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 72 7f[ 	]*vorpd  xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 b2 00 04 00 00[ 	]*vorpd  xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 72 80[ 	]*vorpd  xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 56 b2 f8 fb ff ff[ 	]*vorpd  xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 56 f4[ 	]*vorpd  ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 56 f4[ 	]*vorpd  ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 56 f4[ 	]*vorpd  ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 31[ 	]*vorpd  ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 56 b4 f0 34 12 00 00[ 	]*vorpd  ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 31[ 	]*vorpd  ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 72 7f[ 	]*vorpd  ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 b2 00 10 00 00[ 	]*vorpd  ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 72 80[ 	]*vorpd  ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 56 b2 e0 ef ff ff[ 	]*vorpd  ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 72 7f[ 	]*vorpd  ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 b2 00 04 00 00[ 	]*vorpd  ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 72 80[ 	]*vorpd  ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 56 b2 f8 fb ff ff[ 	]*vorpd  ymm30,ymm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 56 f4[ 	]*vorps  xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 56 f4[ 	]*vorps  xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 56 f4[ 	]*vorps  xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 31[ 	]*vorps  xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 56 b4 f0 34 12 00 00[ 	]*vorps  xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 31[ 	]*vorps  xmm30,xmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 72 7f[ 	]*vorps  xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 b2 00 08 00 00[ 	]*vorps  xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 72 80[ 	]*vorps  xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 56 b2 f0 f7 ff ff[ 	]*vorps  xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 72 7f[ 	]*vorps  xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 b2 00 02 00 00[ 	]*vorps  xmm30,xmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 72 80[ 	]*vorps  xmm30,xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 56 b2 fc fd ff ff[ 	]*vorps  xmm30,xmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 56 f4[ 	]*vorps  ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 56 f4[ 	]*vorps  ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 56 f4[ 	]*vorps  ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 31[ 	]*vorps  ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 56 b4 f0 34 12 00 00[ 	]*vorps  ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 31[ 	]*vorps  ymm30,ymm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 72 7f[ 	]*vorps  ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 b2 00 10 00 00[ 	]*vorps  ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 72 80[ 	]*vorps  ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 56 b2 e0 ef ff ff[ 	]*vorps  ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 72 7f[ 	]*vorps  ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 b2 00 02 00 00[ 	]*vorps  ymm30,ymm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 72 80[ 	]*vorps  ymm30,ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 56 b2 fc fd ff ff[ 	]*vorps  ymm30,ymm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 00 57 f4[ 	]*vxorpd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 07 57 f4[ 	]*vxorpd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 87 57 f4[ 	]*vxorpd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 31[ 	]*vxorpd xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 00 57 b4 f0 34 12 00 00[ 	]*vxorpd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 31[ 	]*vxorpd xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 72 7f[ 	]*vxorpd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 b2 00 08 00 00[ 	]*vxorpd xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 72 80[ 	]*vxorpd xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 00 57 b2 f0 f7 ff ff[ 	]*vxorpd xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 72 7f[ 	]*vxorpd xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 b2 00 04 00 00[ 	]*vxorpd xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 72 80[ 	]*vxorpd xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 10 57 b2 f8 fb ff ff[ 	]*vxorpd xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 20 57 f4[ 	]*vxorpd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 27 57 f4[ 	]*vxorpd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 a7 57 f4[ 	]*vxorpd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 31[ 	]*vxorpd ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 20 57 b4 f0 34 12 00 00[ 	]*vxorpd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 31[ 	]*vxorpd ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 72 7f[ 	]*vxorpd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 b2 00 10 00 00[ 	]*vxorpd ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 72 80[ 	]*vxorpd ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 20 57 b2 e0 ef ff ff[ 	]*vxorpd ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 72 7f[ 	]*vxorpd ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 b2 00 04 00 00[ 	]*vxorpd ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 72 80[ 	]*vxorpd ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 30 57 b2 f8 fb ff ff[ 	]*vxorpd ymm30,ymm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 00 57 f4[ 	]*vxorps xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 07 57 f4[ 	]*vxorps xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 87 57 f4[ 	]*vxorps xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 31[ 	]*vxorps xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 00 57 b4 f0 34 12 00 00[ 	]*vxorps xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 31[ 	]*vxorps xmm30,xmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 72 7f[ 	]*vxorps xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 b2 00 08 00 00[ 	]*vxorps xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 72 80[ 	]*vxorps xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 00 57 b2 f0 f7 ff ff[ 	]*vxorps xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 72 7f[ 	]*vxorps xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 b2 00 02 00 00[ 	]*vxorps xmm30,xmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 72 80[ 	]*vxorps xmm30,xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 10 57 b2 fc fd ff ff[ 	]*vxorps xmm30,xmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 20 57 f4[ 	]*vxorps ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 27 57 f4[ 	]*vxorps ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 a7 57 f4[ 	]*vxorps ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 31[ 	]*vxorps ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 20 57 b4 f0 34 12 00 00[ 	]*vxorps ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 31[ 	]*vxorps ymm30,ymm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 72 7f[ 	]*vxorps ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 b2 00 10 00 00[ 	]*vxorps ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 72 80[ 	]*vxorps ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 20 57 b2 e0 ef ff ff[ 	]*vxorps ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 72 7f[ 	]*vxorps ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 b2 00 02 00 00[ 	]*vxorps ymm30,ymm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 72 80[ 	]*vxorps ymm30,ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 30 57 b2 fc fd ff ff[ 	]*vxorps ymm30,ymm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 08 56 f5 ab[ 	]*vreducepd xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 0f 56 f5 ab[ 	]*vreducepd xmm30\{k7\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 8f 56 f5 ab[ 	]*vreducepd xmm30\{k7\}\{z\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 08 56 f5 7b[ 	]*vreducepd xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 31 7b[ 	]*vreducepd xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 56 b4 f0 34 12 00 00 7b[ 	]*vreducepd xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 31 7b[ 	]*vreducepd xmm30,QWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 72 7f 7b[ 	]*vreducepd xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 b2 00 08 00 00 7b[ 	]*vreducepd xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 72 80 7b[ 	]*vreducepd xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 56 b2 f0 f7 ff ff 7b[ 	]*vreducepd xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 72 7f 7b[ 	]*vreducepd xmm30,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 b2 00 04 00 00 7b[ 	]*vreducepd xmm30,QWORD BCST \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 72 80 7b[ 	]*vreducepd xmm30,QWORD BCST \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 18 56 b2 f8 fb ff ff 7b[ 	]*vreducepd xmm30,QWORD BCST \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 56 f5 ab[ 	]*vreducepd ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 2f 56 f5 ab[ 	]*vreducepd ymm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd af 56 f5 ab[ 	]*vreducepd ymm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 28 56 f5 7b[ 	]*vreducepd ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 31 7b[ 	]*vreducepd ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 28 56 b4 f0 34 12 00 00 7b[ 	]*vreducepd ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 31 7b[ 	]*vreducepd ymm30,QWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 72 7f 7b[ 	]*vreducepd ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 b2 00 10 00 00 7b[ 	]*vreducepd ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 72 80 7b[ 	]*vreducepd ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 56 b2 e0 ef ff ff 7b[ 	]*vreducepd ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 72 7f 7b[ 	]*vreducepd ymm30,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 b2 00 04 00 00 7b[ 	]*vreducepd ymm30,QWORD BCST \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 72 80 7b[ 	]*vreducepd ymm30,QWORD BCST \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 38 56 b2 f8 fb ff ff 7b[ 	]*vreducepd ymm30,QWORD BCST \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 08 56 f5 ab[ 	]*vreduceps xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 0f 56 f5 ab[ 	]*vreduceps xmm30\{k7\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 8f 56 f5 ab[ 	]*vreduceps xmm30\{k7\}\{z\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 08 56 f5 7b[ 	]*vreduceps xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 31 7b[ 	]*vreduceps xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 08 56 b4 f0 34 12 00 00 7b[ 	]*vreduceps xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 31 7b[ 	]*vreduceps xmm30,DWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 72 7f 7b[ 	]*vreduceps xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 b2 00 08 00 00 7b[ 	]*vreduceps xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 72 80 7b[ 	]*vreduceps xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 56 b2 f0 f7 ff ff 7b[ 	]*vreduceps xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 72 7f 7b[ 	]*vreduceps xmm30,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 b2 00 02 00 00 7b[ 	]*vreduceps xmm30,DWORD BCST \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 72 80 7b[ 	]*vreduceps xmm30,DWORD BCST \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 18 56 b2 fc fd ff ff 7b[ 	]*vreduceps xmm30,DWORD BCST \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 28 56 f5 ab[ 	]*vreduceps ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 2f 56 f5 ab[ 	]*vreduceps ymm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d af 56 f5 ab[ 	]*vreduceps ymm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 28 56 f5 7b[ 	]*vreduceps ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 31 7b[ 	]*vreduceps ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 28 56 b4 f0 34 12 00 00 7b[ 	]*vreduceps ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 31 7b[ 	]*vreduceps ymm30,DWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 72 7f 7b[ 	]*vreduceps ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 b2 00 10 00 00 7b[ 	]*vreduceps ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 72 80 7b[ 	]*vreduceps ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 28 56 b2 e0 ef ff ff 7b[ 	]*vreduceps ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 72 7f 7b[ 	]*vreduceps ymm30,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 b2 00 02 00 00 7b[ 	]*vreduceps ymm30,DWORD BCST \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 72 80 7b[ 	]*vreduceps ymm30,DWORD BCST \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 38 56 b2 fc fd ff ff 7b[ 	]*vreduceps ymm30,DWORD BCST \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 29 ab[ 	]*vextractf64x2 XMMWORD PTR \[rcx\],ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 2f 19 29 ab[ 	]*vextractf64x2 XMMWORD PTR \[rcx\]\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 29 7b[ 	]*vextractf64x2 XMMWORD PTR \[rcx\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 28 19 ac f0 34 12 00 00 7b[ 	]*vextractf64x2 XMMWORD PTR \[rax\+r14\*8\+0x1234\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 6a 7f 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx\+0x7f0\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 aa 00 08 00 00 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx\+0x800\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 6a 80 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx-0x800\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 19 aa f0 f7 ff ff 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx-0x810\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 29 ab[ 	]*vextracti64x2 XMMWORD PTR \[rcx\],ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 2f 39 29 ab[ 	]*vextracti64x2 XMMWORD PTR \[rcx\]\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 29 7b[ 	]*vextracti64x2 XMMWORD PTR \[rcx\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 28 39 ac f0 34 12 00 00 7b[ 	]*vextracti64x2 XMMWORD PTR \[rax\+r14\*8\+0x1234\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 6a 7f 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx\+0x7f0\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 aa 00 08 00 00 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx\+0x800\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 6a 80 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx-0x800\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 28 39 aa f0 f7 ff ff 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx-0x810\],ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 7a f5[ 	]*vcvttpd2qq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 7a f5[ 	]*vcvttpd2qq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 7a f5[ 	]*vcvttpd2qq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a 31[ 	]*vcvttpd2qq xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 7a b4 f0 34 12 00 00[ 	]*vcvttpd2qq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a 31[ 	]*vcvttpd2qq xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a 72 7f[ 	]*vcvttpd2qq xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a b2 00 08 00 00[ 	]*vcvttpd2qq xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a 72 80[ 	]*vcvttpd2qq xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 7a b2 f0 f7 ff ff[ 	]*vcvttpd2qq xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a 72 7f[ 	]*vcvttpd2qq xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a b2 00 04 00 00[ 	]*vcvttpd2qq xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a 72 80[ 	]*vcvttpd2qq xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 7a f5[ 	]*vcvttpd2qq ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 7a f5[ 	]*vcvttpd2qq ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 7a f5[ 	]*vcvttpd2qq ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a 31[ 	]*vcvttpd2qq ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 7a b4 f0 34 12 00 00[ 	]*vcvttpd2qq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a 31[ 	]*vcvttpd2qq ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a 72 7f[ 	]*vcvttpd2qq ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a b2 00 10 00 00[ 	]*vcvttpd2qq ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a 72 80[ 	]*vcvttpd2qq ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 7a b2 e0 ef ff ff[ 	]*vcvttpd2qq ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a 72 7f[ 	]*vcvttpd2qq ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a b2 00 04 00 00[ 	]*vcvttpd2qq ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a 72 80[ 	]*vcvttpd2qq ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 08 78 f5[ 	]*vcvttpd2uqq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 0f 78 f5[ 	]*vcvttpd2uqq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 8f 78 f5[ 	]*vcvttpd2uqq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 31[ 	]*vcvttpd2uqq xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 08 78 b4 f0 34 12 00 00[ 	]*vcvttpd2uqq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 31[ 	]*vcvttpd2uqq xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 72 7f[ 	]*vcvttpd2uqq xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 b2 00 08 00 00[ 	]*vcvttpd2uqq xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 72 80[ 	]*vcvttpd2uqq xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 08 78 b2 f0 f7 ff ff[ 	]*vcvttpd2uqq xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 72 7f[ 	]*vcvttpd2uqq xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 b2 00 04 00 00[ 	]*vcvttpd2uqq xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 72 80[ 	]*vcvttpd2uqq xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 18 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 28 78 f5[ 	]*vcvttpd2uqq ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 2f 78 f5[ 	]*vcvttpd2uqq ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd af 78 f5[ 	]*vcvttpd2uqq ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 31[ 	]*vcvttpd2uqq ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 28 78 b4 f0 34 12 00 00[ 	]*vcvttpd2uqq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 31[ 	]*vcvttpd2uqq ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 72 7f[ 	]*vcvttpd2uqq ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 b2 00 10 00 00[ 	]*vcvttpd2uqq ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 72 80[ 	]*vcvttpd2uqq ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 28 78 b2 e0 ef ff ff[ 	]*vcvttpd2uqq ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 72 7f[ 	]*vcvttpd2uqq ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 b2 00 04 00 00[ 	]*vcvttpd2uqq ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 72 80[ 	]*vcvttpd2uqq ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 38 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 7a f5[ 	]*vcvttps2qq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 7a f5[ 	]*vcvttps2qq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 7a f5[ 	]*vcvttps2qq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a 31[ 	]*vcvttps2qq xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 7a b4 f0 34 12 00 00[ 	]*vcvttps2qq xmm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a 31[ 	]*vcvttps2qq xmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a 72 7f[ 	]*vcvttps2qq xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a b2 00 04 00 00[ 	]*vcvttps2qq xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a 72 80[ 	]*vcvttps2qq xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 7a b2 f8 fb ff ff[ 	]*vcvttps2qq xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a 72 7f[ 	]*vcvttps2qq xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a b2 00 02 00 00[ 	]*vcvttps2qq xmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a 72 80[ 	]*vcvttps2qq xmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a b2 fc fd ff ff[ 	]*vcvttps2qq xmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 7a 72 7f[ 	]*vcvttps2qq xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 7a f5[ 	]*vcvttps2qq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 7a f5[ 	]*vcvttps2qq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 7a f5[ 	]*vcvttps2qq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a 31[ 	]*vcvttps2qq ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 7a b4 f0 34 12 00 00[ 	]*vcvttps2qq ymm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a 31[ 	]*vcvttps2qq ymm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a 72 7f[ 	]*vcvttps2qq ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a b2 00 08 00 00[ 	]*vcvttps2qq ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a 72 80[ 	]*vcvttps2qq ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 7a b2 f0 f7 ff ff[ 	]*vcvttps2qq ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a 72 7f[ 	]*vcvttps2qq ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a b2 00 02 00 00[ 	]*vcvttps2qq ymm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a 72 80[ 	]*vcvttps2qq ymm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a b2 fc fd ff ff[ 	]*vcvttps2qq ymm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 7a 72 7f[ 	]*vcvttps2qq ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 08 78 f5[ 	]*vcvttps2uqq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 0f 78 f5[ 	]*vcvttps2uqq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 8f 78 f5[ 	]*vcvttps2uqq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 31[ 	]*vcvttps2uqq xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 08 78 b4 f0 34 12 00 00[ 	]*vcvttps2uqq xmm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 31[ 	]*vcvttps2uqq xmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 72 7f[ 	]*vcvttps2uqq xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 b2 00 04 00 00[ 	]*vcvttps2uqq xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 72 80[ 	]*vcvttps2uqq xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 08 78 b2 f8 fb ff ff[ 	]*vcvttps2uqq xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 72 7f[ 	]*vcvttps2uqq xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 b2 00 02 00 00[ 	]*vcvttps2uqq xmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 72 80[ 	]*vcvttps2uqq xmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 b2 fc fd ff ff[ 	]*vcvttps2uqq xmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 18 78 72 7f[ 	]*vcvttps2uqq xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 28 78 f5[ 	]*vcvttps2uqq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 2f 78 f5[ 	]*vcvttps2uqq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d af 78 f5[ 	]*vcvttps2uqq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 31[ 	]*vcvttps2uqq ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 28 78 b4 f0 34 12 00 00[ 	]*vcvttps2uqq ymm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 31[ 	]*vcvttps2uqq ymm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 72 7f[ 	]*vcvttps2uqq ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 b2 00 08 00 00[ 	]*vcvttps2uqq ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 72 80[ 	]*vcvttps2uqq ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 28 78 b2 f0 f7 ff ff[ 	]*vcvttps2uqq ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 72 7f[ 	]*vcvttps2uqq ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 b2 00 02 00 00[ 	]*vcvttps2uqq ymm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 72 80[ 	]*vcvttps2uqq ymm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 b2 fc fd ff ff[ 	]*vcvttps2uqq ymm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 38 78 72 7f[ 	]*vcvttps2uqq ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 08 39 ee[ 	]*vpmovd2m k5,xmm30
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 28 39 ee[ 	]*vpmovd2m k5,ymm30
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 08 39 ee[ 	]*vpmovq2m k5,xmm30
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 28 39 ee[ 	]*vpmovq2m k5,ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 38 f5[ 	]*vpmovm2d xmm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 38 f5[ 	]*vpmovm2d ymm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 08 38 f5[ 	]*vpmovm2q xmm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 28 38 f5[ 	]*vpmovm2q ymm30,k5
#pass
