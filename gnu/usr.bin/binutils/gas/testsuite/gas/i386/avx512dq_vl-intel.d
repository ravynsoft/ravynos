#as:
#objdump: -dw -Mintel
#name: i386 AVX512DQ/VL insns (Intel disassembly)
#source: avx512dq_vl.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a 31[ 	]*vbroadcastf64x2 ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 1a 31[ 	]*vbroadcastf64x2 ymm6\{k7\}\{z\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a b4 f4 c0 1d fe ff[ 	]*vbroadcastf64x2 ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a 72 7f[ 	]*vbroadcastf64x2 ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a 72 80[ 	]*vbroadcastf64x2 ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a 31[ 	]*vbroadcasti64x2 ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 5a 31[ 	]*vbroadcasti64x2 ymm6\{k7\}\{z\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a b4 f4 c0 1d fe ff[ 	]*vbroadcasti64x2 ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a 72 7f[ 	]*vbroadcasti64x2 ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a 72 80[ 	]*vbroadcasti64x2 ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 f7[ 	]*vbroadcastf32x2 ymm6\{k7\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 19 f7[ 	]*vbroadcastf32x2 ymm6\{k7\}\{z\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 31[ 	]*vbroadcastf32x2 ymm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 b4 f4 c0 1d fe ff[ 	]*vbroadcastf32x2 ymm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 72 7f[ 	]*vbroadcastf32x2 ymm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 ymm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 72 80[ 	]*vbroadcastf32x2 ymm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 ymm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b f5[ 	]*vcvtpd2qq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 7b f5[ 	]*vcvtpd2qq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b 31[ 	]*vcvtpd2qq xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b b4 f4 c0 1d fe ff[ 	]*vcvtpd2qq xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b 30[ 	]*vcvtpd2qq xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b 72 7f[ 	]*vcvtpd2qq xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b b2 00 08 00 00[ 	]*vcvtpd2qq xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b 72 80[ 	]*vcvtpd2qq xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b b2 f0 f7 ff ff[ 	]*vcvtpd2qq xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b 72 7f[ 	]*vcvtpd2qq xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b b2 00 04 00 00[ 	]*vcvtpd2qq xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b 72 80[ 	]*vcvtpd2qq xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b f5[ 	]*vcvtpd2qq ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 7b f5[ 	]*vcvtpd2qq ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b 31[ 	]*vcvtpd2qq ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b b4 f4 c0 1d fe ff[ 	]*vcvtpd2qq ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b 30[ 	]*vcvtpd2qq ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b 72 7f[ 	]*vcvtpd2qq ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b b2 00 10 00 00[ 	]*vcvtpd2qq ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b 72 80[ 	]*vcvtpd2qq ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b b2 e0 ef ff ff[ 	]*vcvtpd2qq ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b 72 7f[ 	]*vcvtpd2qq ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b b2 00 04 00 00[ 	]*vcvtpd2qq ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b 72 80[ 	]*vcvtpd2qq ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 f5[ 	]*vcvtpd2uqq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 79 f5[ 	]*vcvtpd2uqq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 31[ 	]*vcvtpd2uqq xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 b4 f4 c0 1d fe ff[ 	]*vcvtpd2uqq xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 30[ 	]*vcvtpd2uqq xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 72 7f[ 	]*vcvtpd2uqq xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 b2 00 08 00 00[ 	]*vcvtpd2uqq xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 72 80[ 	]*vcvtpd2uqq xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 b2 f0 f7 ff ff[ 	]*vcvtpd2uqq xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 72 7f[ 	]*vcvtpd2uqq xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 b2 00 04 00 00[ 	]*vcvtpd2uqq xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 72 80[ 	]*vcvtpd2uqq xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 f5[ 	]*vcvtpd2uqq ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 79 f5[ 	]*vcvtpd2uqq ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 31[ 	]*vcvtpd2uqq ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 b4 f4 c0 1d fe ff[ 	]*vcvtpd2uqq ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 30[ 	]*vcvtpd2uqq ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 72 7f[ 	]*vcvtpd2uqq ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 b2 00 10 00 00[ 	]*vcvtpd2uqq ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 72 80[ 	]*vcvtpd2uqq ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 b2 e0 ef ff ff[ 	]*vcvtpd2uqq ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 72 7f[ 	]*vcvtpd2uqq ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 b2 00 04 00 00[ 	]*vcvtpd2uqq ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 72 80[ 	]*vcvtpd2uqq ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b f5[ 	]*vcvtps2qq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 7b f5[ 	]*vcvtps2qq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b 31[ 	]*vcvtps2qq xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b b4 f4 c0 1d fe ff[ 	]*vcvtps2qq xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b 30[ 	]*vcvtps2qq xmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b 72 7f[ 	]*vcvtps2qq xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b b2 00 04 00 00[ 	]*vcvtps2qq xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b 72 80[ 	]*vcvtps2qq xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b b2 f8 fb ff ff[ 	]*vcvtps2qq xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b 72 7f[ 	]*vcvtps2qq xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b b2 00 02 00 00[ 	]*vcvtps2qq xmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b 72 80[ 	]*vcvtps2qq xmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b b2 fc fd ff ff[ 	]*vcvtps2qq xmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b f5[ 	]*vcvtps2qq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 7b f5[ 	]*vcvtps2qq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b 31[ 	]*vcvtps2qq ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b b4 f4 c0 1d fe ff[ 	]*vcvtps2qq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b 30[ 	]*vcvtps2qq ymm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b 72 7f[ 	]*vcvtps2qq ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b b2 00 08 00 00[ 	]*vcvtps2qq ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b 72 80[ 	]*vcvtps2qq ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b b2 f0 f7 ff ff[ 	]*vcvtps2qq ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b 72 7f[ 	]*vcvtps2qq ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b b2 00 02 00 00[ 	]*vcvtps2qq ymm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b 72 80[ 	]*vcvtps2qq ymm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b b2 fc fd ff ff[ 	]*vcvtps2qq ymm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 f5[ 	]*vcvtps2uqq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 79 f5[ 	]*vcvtps2uqq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 31[ 	]*vcvtps2uqq xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 b4 f4 c0 1d fe ff[ 	]*vcvtps2uqq xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 30[ 	]*vcvtps2uqq xmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 72 7f[ 	]*vcvtps2uqq xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 b2 00 04 00 00[ 	]*vcvtps2uqq xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 72 80[ 	]*vcvtps2uqq xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 b2 f8 fb ff ff[ 	]*vcvtps2uqq xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 72 7f[ 	]*vcvtps2uqq xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 b2 00 02 00 00[ 	]*vcvtps2uqq xmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 72 80[ 	]*vcvtps2uqq xmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 b2 fc fd ff ff[ 	]*vcvtps2uqq xmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 f5[ 	]*vcvtps2uqq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 79 f5[ 	]*vcvtps2uqq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 31[ 	]*vcvtps2uqq ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 b4 f4 c0 1d fe ff[ 	]*vcvtps2uqq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 30[ 	]*vcvtps2uqq ymm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 72 7f[ 	]*vcvtps2uqq ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 b2 00 08 00 00[ 	]*vcvtps2uqq ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 72 80[ 	]*vcvtps2uqq ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 b2 f0 f7 ff ff[ 	]*vcvtps2uqq ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 72 7f[ 	]*vcvtps2uqq ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 b2 00 02 00 00[ 	]*vcvtps2uqq ymm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 72 80[ 	]*vcvtps2uqq ymm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 b2 fc fd ff ff[ 	]*vcvtps2uqq ymm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 f5[ 	]*vcvtqq2pd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 8f e6 f5[ 	]*vcvtqq2pd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 31[ 	]*vcvtqq2pd xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 b4 f4 c0 1d fe ff[ 	]*vcvtqq2pd xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 30[ 	]*vcvtqq2pd xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 72 7f[ 	]*vcvtqq2pd xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 b2 00 08 00 00[ 	]*vcvtqq2pd xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 72 80[ 	]*vcvtqq2pd xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 b2 f0 f7 ff ff[ 	]*vcvtqq2pd xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 72 7f[ 	]*vcvtqq2pd xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 b2 00 04 00 00[ 	]*vcvtqq2pd xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 72 80[ 	]*vcvtqq2pd xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 f5[ 	]*vcvtqq2pd ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe af e6 f5[ 	]*vcvtqq2pd ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 31[ 	]*vcvtqq2pd ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 b4 f4 c0 1d fe ff[ 	]*vcvtqq2pd ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 30[ 	]*vcvtqq2pd ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 72 7f[ 	]*vcvtqq2pd ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 b2 00 10 00 00[ 	]*vcvtqq2pd ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 72 80[ 	]*vcvtqq2pd ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 b2 e0 ef ff ff[ 	]*vcvtqq2pd ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 72 7f[ 	]*vcvtqq2pd ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 b2 00 04 00 00[ 	]*vcvtqq2pd ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 72 80[ 	]*vcvtqq2pd ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b f5[ 	]*vcvtqq2ps xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 8f 5b f5[ 	]*vcvtqq2ps xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b 31[ 	]*vcvtqq2ps xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b b4 f4 c0 1d fe ff[ 	]*vcvtqq2ps xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b 30[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[eax\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b 72 7f[ 	]*vcvtqq2ps xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b b2 00 08 00 00[ 	]*vcvtqq2ps xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b 72 80[ 	]*vcvtqq2ps xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b b2 f0 f7 ff ff[ 	]*vcvtqq2ps xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b 72 7f[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b b2 00 04 00 00[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b 72 80[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx-0x408\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b f5[ 	]*vcvtqq2ps xmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc af 5b f5[ 	]*vcvtqq2ps xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b 31[ 	]*vcvtqq2ps xmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b b4 f4 c0 1d fe ff[ 	]*vcvtqq2ps xmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b 30[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[eax\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b 72 7f[ 	]*vcvtqq2ps xmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b b2 00 10 00 00[ 	]*vcvtqq2ps xmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b 72 80[ 	]*vcvtqq2ps xmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b b2 e0 ef ff ff[ 	]*vcvtqq2ps xmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b 72 7f[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b b2 00 04 00 00[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b 72 80[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx-0x408\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a f5[ 	]*vcvtuqq2pd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 8f 7a f5[ 	]*vcvtuqq2pd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a 31[ 	]*vcvtuqq2pd xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2pd xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a 30[ 	]*vcvtuqq2pd xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a 72 7f[ 	]*vcvtuqq2pd xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a b2 00 08 00 00[ 	]*vcvtuqq2pd xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a 72 80[ 	]*vcvtuqq2pd xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2pd xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a 72 7f[ 	]*vcvtuqq2pd xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a b2 00 04 00 00[ 	]*vcvtuqq2pd xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a 72 80[ 	]*vcvtuqq2pd xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a f5[ 	]*vcvtuqq2pd ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe af 7a f5[ 	]*vcvtuqq2pd ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a 31[ 	]*vcvtuqq2pd ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2pd ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a 30[ 	]*vcvtuqq2pd ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a 72 7f[ 	]*vcvtuqq2pd ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a b2 00 10 00 00[ 	]*vcvtuqq2pd ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a 72 80[ 	]*vcvtuqq2pd ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a b2 e0 ef ff ff[ 	]*vcvtuqq2pd ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a 72 7f[ 	]*vcvtuqq2pd ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a b2 00 04 00 00[ 	]*vcvtuqq2pd ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a 72 80[ 	]*vcvtuqq2pd ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a f5[ 	]*vcvtuqq2ps xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 7a f5[ 	]*vcvtuqq2ps xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a 31[ 	]*vcvtuqq2ps xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2ps xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a 30[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[eax\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a 72 7f[ 	]*vcvtuqq2ps xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a b2 00 08 00 00[ 	]*vcvtuqq2ps xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a 72 80[ 	]*vcvtuqq2ps xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2ps xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a 72 7f[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a b2 00 04 00 00[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a 72 80[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx-0x408\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a f5[ 	]*vcvtuqq2ps xmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 7a f5[ 	]*vcvtuqq2ps xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a 31[ 	]*vcvtuqq2ps xmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2ps xmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a 30[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[eax\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a 72 7f[ 	]*vcvtuqq2ps xmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a b2 00 10 00 00[ 	]*vcvtuqq2ps xmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a 72 80[ 	]*vcvtuqq2ps xmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a b2 e0 ef ff ff[ 	]*vcvtuqq2ps xmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a 72 7f[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a b2 00 04 00 00[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a 72 80[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx-0x408\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 ee ab[ 	]*vextractf64x2 xmm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd af 19 ee ab[ 	]*vextractf64x2 xmm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 ee 7b[ 	]*vextractf64x2 xmm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 ee ab[ 	]*vextracti64x2 xmm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd af 39 ee ab[ 	]*vextracti64x2 xmm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 ee 7b[ 	]*vextracti64x2 xmm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 ee ab[ 	]*vfpclasspd k5\{k7\},xmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 ee 7b[ 	]*vfpclasspd k5\{k7\},xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 29 7b[ 	]*vfpclasspd k5\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspd k5\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 28 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[eax\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 6a 7f 7b[ 	]*vfpclasspd k5\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 aa 00 08 00 00 7b[ 	]*vfpclasspd k5\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 6a 80 7b[ 	]*vfpclasspd k5\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 aa f0 f7 ff ff 7b[ 	]*vfpclasspd k5\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 6a 7f 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx\+0x3f8\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 aa 00 04 00 00 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx\+0x400\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 6a 80 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx-0x400\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx-0x408\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 ee ab[ 	]*vfpclasspd k5\{k7\},ymm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 ee 7b[ 	]*vfpclasspd k5\{k7\},ymm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 29 7b[ 	]*vfpclasspd k5\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspd k5\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 28 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[eax\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 6a 7f 7b[ 	]*vfpclasspd k5\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 aa 00 10 00 00 7b[ 	]*vfpclasspd k5\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 6a 80 7b[ 	]*vfpclasspd k5\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 aa e0 ef ff ff 7b[ 	]*vfpclasspd k5\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 6a 7f 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx\+0x3f8\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 aa 00 04 00 00 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx\+0x400\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 6a 80 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx-0x400\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx-0x408\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 ee ab[ 	]*vfpclassps k5\{k7\},xmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 ee 7b[ 	]*vfpclassps k5\{k7\},xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 29 7b[ 	]*vfpclassps k5\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclassps k5\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 28 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[eax\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 6a 7f 7b[ 	]*vfpclassps k5\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 aa 00 08 00 00 7b[ 	]*vfpclassps k5\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 6a 80 7b[ 	]*vfpclassps k5\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 aa f0 f7 ff ff 7b[ 	]*vfpclassps k5\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 6a 7f 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx\+0x1fc\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 aa 00 02 00 00 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx\+0x200\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 6a 80 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx-0x200\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 aa fc fd ff ff 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx-0x204\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 ee ab[ 	]*vfpclassps k5\{k7\},ymm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 ee 7b[ 	]*vfpclassps k5\{k7\},ymm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 29 7b[ 	]*vfpclassps k5\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclassps k5\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 28 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[eax\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 6a 7f 7b[ 	]*vfpclassps k5\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 aa 00 10 00 00 7b[ 	]*vfpclassps k5\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 6a 80 7b[ 	]*vfpclassps k5\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 aa e0 ef ff ff 7b[ 	]*vfpclassps k5\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 6a 7f 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx\+0x1fc\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 aa 00 02 00 00 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx\+0x200\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 6a 80 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx-0x200\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 aa fc fd ff ff 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx-0x204\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 f4 ab[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 18 f4 ab[ 	]*vinsertf64x2 ymm6\{k7\}\{z\},ymm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 f4 7b[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 31 7b[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 b4 f4 c0 1d fe ff 7b[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 72 7f 7b[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 72 80 7b[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 f4 ab[ 	]*vinserti64x2 ymm6\{k7\},ymm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 38 f4 ab[ 	]*vinserti64x2 ymm6\{k7\}\{z\},ymm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 f4 7b[ 	]*vinserti64x2 ymm6\{k7\},ymm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 31 7b[ 	]*vinserti64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 b4 f4 c0 1d fe ff 7b[ 	]*vinserti64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 72 7f 7b[ 	]*vinserti64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 72 80 7b[ 	]*vinserti64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 f7[ 	]*vbroadcasti32x2 xmm6\{k7\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 59 f7[ 	]*vbroadcasti32x2 xmm6\{k7\}\{z\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 31[ 	]*vbroadcasti32x2 xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x2 xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 72 7f[ 	]*vbroadcasti32x2 xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 72 80[ 	]*vbroadcasti32x2 xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 f7[ 	]*vbroadcasti32x2 ymm6\{k7\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 59 f7[ 	]*vbroadcasti32x2 ymm6\{k7\}\{z\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 31[ 	]*vbroadcasti32x2 ymm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x2 ymm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 72 7f[ 	]*vbroadcasti32x2 ymm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 ymm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 72 80[ 	]*vbroadcasti32x2 ymm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 ymm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 f4[ 	]*vpmullq xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 40 f4[ 	]*vpmullq xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 31[ 	]*vpmullq xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 b4 f4 c0 1d fe ff[ 	]*vpmullq xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 30[ 	]*vpmullq xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 72 7f[ 	]*vpmullq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 b2 00 08 00 00[ 	]*vpmullq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 72 80[ 	]*vpmullq xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 b2 f0 f7 ff ff[ 	]*vpmullq xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 72 7f[ 	]*vpmullq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 b2 00 04 00 00[ 	]*vpmullq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 72 80[ 	]*vpmullq xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 b2 f8 fb ff ff[ 	]*vpmullq xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 f4[ 	]*vpmullq ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 40 f4[ 	]*vpmullq ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 31[ 	]*vpmullq ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 b4 f4 c0 1d fe ff[ 	]*vpmullq ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 30[ 	]*vpmullq ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 72 7f[ 	]*vpmullq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 b2 00 10 00 00[ 	]*vpmullq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 72 80[ 	]*vpmullq ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 b2 e0 ef ff ff[ 	]*vpmullq ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 72 7f[ 	]*vpmullq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 b2 00 04 00 00[ 	]*vpmullq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 72 80[ 	]*vpmullq ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 b2 f8 fb ff ff[ 	]*vpmullq ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 f4 ab[ 	]*vrangepd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 50 f4 ab[ 	]*vrangepd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 f4 7b[ 	]*vrangepd xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 31 7b[ 	]*vrangepd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangepd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 30 7b[ 	]*vrangepd xmm6\{k7\},xmm5,QWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 72 7f 7b[ 	]*vrangepd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 b2 00 08 00 00 7b[ 	]*vrangepd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 72 80 7b[ 	]*vrangepd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 b2 f0 f7 ff ff 7b[ 	]*vrangepd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 72 7f 7b[ 	]*vrangepd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 b2 00 04 00 00 7b[ 	]*vrangepd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 72 80 7b[ 	]*vrangepd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 b2 f8 fb ff ff 7b[ 	]*vrangepd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 f4 ab[ 	]*vrangepd ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 50 f4 ab[ 	]*vrangepd ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 f4 7b[ 	]*vrangepd ymm6\{k7\},ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 31 7b[ 	]*vrangepd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangepd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 30 7b[ 	]*vrangepd ymm6\{k7\},ymm5,QWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 72 7f 7b[ 	]*vrangepd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 b2 00 10 00 00 7b[ 	]*vrangepd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 72 80 7b[ 	]*vrangepd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 b2 e0 ef ff ff 7b[ 	]*vrangepd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 72 7f 7b[ 	]*vrangepd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 b2 00 04 00 00 7b[ 	]*vrangepd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 72 80 7b[ 	]*vrangepd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 b2 f8 fb ff ff 7b[ 	]*vrangepd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 f4 ab[ 	]*vrangeps xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 50 f4 ab[ 	]*vrangeps xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 f4 7b[ 	]*vrangeps xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 31 7b[ 	]*vrangeps xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangeps xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 30 7b[ 	]*vrangeps xmm6\{k7\},xmm5,DWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 72 7f 7b[ 	]*vrangeps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 b2 00 08 00 00 7b[ 	]*vrangeps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 72 80 7b[ 	]*vrangeps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 b2 f0 f7 ff ff 7b[ 	]*vrangeps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 72 7f 7b[ 	]*vrangeps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 b2 00 02 00 00 7b[ 	]*vrangeps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 72 80 7b[ 	]*vrangeps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 b2 fc fd ff ff 7b[ 	]*vrangeps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 f4 ab[ 	]*vrangeps ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 50 f4 ab[ 	]*vrangeps ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 f4 7b[ 	]*vrangeps ymm6\{k7\},ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 31 7b[ 	]*vrangeps ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangeps ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 30 7b[ 	]*vrangeps ymm6\{k7\},ymm5,DWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 72 7f 7b[ 	]*vrangeps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 b2 00 10 00 00 7b[ 	]*vrangeps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 72 80 7b[ 	]*vrangeps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 b2 e0 ef ff ff 7b[ 	]*vrangeps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 72 7f 7b[ 	]*vrangeps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 b2 00 02 00 00 7b[ 	]*vrangeps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 72 80 7b[ 	]*vrangeps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 b2 fc fd ff ff 7b[ 	]*vrangeps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 f4[ 	]*vandpd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 54 f4[ 	]*vandpd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 31[ 	]*vandpd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 b4 f4 c0 1d fe ff[ 	]*vandpd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 30[ 	]*vandpd xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 72 7f[ 	]*vandpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 b2 00 08 00 00[ 	]*vandpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 72 80[ 	]*vandpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 b2 f0 f7 ff ff[ 	]*vandpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 72 7f[ 	]*vandpd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 b2 00 04 00 00[ 	]*vandpd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 72 80[ 	]*vandpd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 b2 f8 fb ff ff[ 	]*vandpd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 f4[ 	]*vandpd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 54 f4[ 	]*vandpd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 31[ 	]*vandpd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 b4 f4 c0 1d fe ff[ 	]*vandpd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 30[ 	]*vandpd ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 72 7f[ 	]*vandpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 b2 00 10 00 00[ 	]*vandpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 72 80[ 	]*vandpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 b2 e0 ef ff ff[ 	]*vandpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 72 7f[ 	]*vandpd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 b2 00 04 00 00[ 	]*vandpd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 72 80[ 	]*vandpd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 b2 f8 fb ff ff[ 	]*vandpd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 f4[ 	]*vandps xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 54 f4[ 	]*vandps xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 31[ 	]*vandps xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 b4 f4 c0 1d fe ff[ 	]*vandps xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 30[ 	]*vandps xmm6\{k7\},xmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 72 7f[ 	]*vandps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 b2 00 08 00 00[ 	]*vandps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 72 80[ 	]*vandps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 b2 f0 f7 ff ff[ 	]*vandps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 72 7f[ 	]*vandps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 b2 00 02 00 00[ 	]*vandps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 72 80[ 	]*vandps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 b2 fc fd ff ff[ 	]*vandps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 f4[ 	]*vandps ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 54 f4[ 	]*vandps ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 31[ 	]*vandps ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 b4 f4 c0 1d fe ff[ 	]*vandps ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 30[ 	]*vandps ymm6\{k7\},ymm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 72 7f[ 	]*vandps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 b2 00 10 00 00[ 	]*vandps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 72 80[ 	]*vandps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 b2 e0 ef ff ff[ 	]*vandps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 72 7f[ 	]*vandps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 b2 00 02 00 00[ 	]*vandps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 72 80[ 	]*vandps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 b2 fc fd ff ff[ 	]*vandps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 f4[ 	]*vandnpd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 55 f4[ 	]*vandnpd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 31[ 	]*vandnpd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 b4 f4 c0 1d fe ff[ 	]*vandnpd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 30[ 	]*vandnpd xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 72 7f[ 	]*vandnpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 b2 00 08 00 00[ 	]*vandnpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 72 80[ 	]*vandnpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 b2 f0 f7 ff ff[ 	]*vandnpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 72 7f[ 	]*vandnpd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 b2 00 04 00 00[ 	]*vandnpd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 72 80[ 	]*vandnpd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 b2 f8 fb ff ff[ 	]*vandnpd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 f4[ 	]*vandnpd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 55 f4[ 	]*vandnpd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 31[ 	]*vandnpd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 b4 f4 c0 1d fe ff[ 	]*vandnpd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 30[ 	]*vandnpd ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 72 7f[ 	]*vandnpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 b2 00 10 00 00[ 	]*vandnpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 72 80[ 	]*vandnpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 b2 e0 ef ff ff[ 	]*vandnpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 72 7f[ 	]*vandnpd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 b2 00 04 00 00[ 	]*vandnpd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 72 80[ 	]*vandnpd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 b2 f8 fb ff ff[ 	]*vandnpd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 f4[ 	]*vandnps xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 55 f4[ 	]*vandnps xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 31[ 	]*vandnps xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 b4 f4 c0 1d fe ff[ 	]*vandnps xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 30[ 	]*vandnps xmm6\{k7\},xmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 72 7f[ 	]*vandnps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 b2 00 08 00 00[ 	]*vandnps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 72 80[ 	]*vandnps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 b2 f0 f7 ff ff[ 	]*vandnps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 72 7f[ 	]*vandnps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 b2 00 02 00 00[ 	]*vandnps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 72 80[ 	]*vandnps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 b2 fc fd ff ff[ 	]*vandnps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 f4[ 	]*vandnps ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 55 f4[ 	]*vandnps ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 31[ 	]*vandnps ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 b4 f4 c0 1d fe ff[ 	]*vandnps ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 30[ 	]*vandnps ymm6\{k7\},ymm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 72 7f[ 	]*vandnps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 b2 00 10 00 00[ 	]*vandnps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 72 80[ 	]*vandnps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 b2 e0 ef ff ff[ 	]*vandnps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 72 7f[ 	]*vandnps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 b2 00 02 00 00[ 	]*vandnps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 72 80[ 	]*vandnps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 b2 fc fd ff ff[ 	]*vandnps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 f4[ 	]*vorpd  xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 56 f4[ 	]*vorpd  xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 31[ 	]*vorpd  xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 b4 f4 c0 1d fe ff[ 	]*vorpd  xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 30[ 	]*vorpd  xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 72 7f[ 	]*vorpd  xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 b2 00 08 00 00[ 	]*vorpd  xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 72 80[ 	]*vorpd  xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 b2 f0 f7 ff ff[ 	]*vorpd  xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 72 7f[ 	]*vorpd  xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 b2 00 04 00 00[ 	]*vorpd  xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 72 80[ 	]*vorpd  xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 b2 f8 fb ff ff[ 	]*vorpd  xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 f4[ 	]*vorpd  ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 56 f4[ 	]*vorpd  ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 31[ 	]*vorpd  ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 b4 f4 c0 1d fe ff[ 	]*vorpd  ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 30[ 	]*vorpd  ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 72 7f[ 	]*vorpd  ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 b2 00 10 00 00[ 	]*vorpd  ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 72 80[ 	]*vorpd  ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 b2 e0 ef ff ff[ 	]*vorpd  ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 72 7f[ 	]*vorpd  ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 b2 00 04 00 00[ 	]*vorpd  ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 72 80[ 	]*vorpd  ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 b2 f8 fb ff ff[ 	]*vorpd  ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 f4[ 	]*vorps  xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 56 f4[ 	]*vorps  xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 31[ 	]*vorps  xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 b4 f4 c0 1d fe ff[ 	]*vorps  xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 30[ 	]*vorps  xmm6\{k7\},xmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 72 7f[ 	]*vorps  xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 b2 00 08 00 00[ 	]*vorps  xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 72 80[ 	]*vorps  xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 b2 f0 f7 ff ff[ 	]*vorps  xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 72 7f[ 	]*vorps  xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 b2 00 02 00 00[ 	]*vorps  xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 72 80[ 	]*vorps  xmm6\{k7\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 b2 fc fd ff ff[ 	]*vorps  xmm6\{k7\},xmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 f4[ 	]*vorps  ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 56 f4[ 	]*vorps  ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 31[ 	]*vorps  ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 b4 f4 c0 1d fe ff[ 	]*vorps  ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 30[ 	]*vorps  ymm6\{k7\},ymm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 72 7f[ 	]*vorps  ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 b2 00 10 00 00[ 	]*vorps  ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 72 80[ 	]*vorps  ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 b2 e0 ef ff ff[ 	]*vorps  ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 72 7f[ 	]*vorps  ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 b2 00 02 00 00[ 	]*vorps  ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 72 80[ 	]*vorps  ymm6\{k7\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 b2 fc fd ff ff[ 	]*vorps  ymm6\{k7\},ymm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 f4[ 	]*vxorpd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 57 f4[ 	]*vxorpd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 31[ 	]*vxorpd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 b4 f4 c0 1d fe ff[ 	]*vxorpd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 30[ 	]*vxorpd xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 72 7f[ 	]*vxorpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 b2 00 08 00 00[ 	]*vxorpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 72 80[ 	]*vxorpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 b2 f0 f7 ff ff[ 	]*vxorpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 72 7f[ 	]*vxorpd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 b2 00 04 00 00[ 	]*vxorpd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 72 80[ 	]*vxorpd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 b2 f8 fb ff ff[ 	]*vxorpd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 f4[ 	]*vxorpd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 57 f4[ 	]*vxorpd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 31[ 	]*vxorpd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 b4 f4 c0 1d fe ff[ 	]*vxorpd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 30[ 	]*vxorpd ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 72 7f[ 	]*vxorpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 b2 00 10 00 00[ 	]*vxorpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 72 80[ 	]*vxorpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 b2 e0 ef ff ff[ 	]*vxorpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 72 7f[ 	]*vxorpd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 b2 00 04 00 00[ 	]*vxorpd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 72 80[ 	]*vxorpd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 b2 f8 fb ff ff[ 	]*vxorpd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 f4[ 	]*vxorps xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 57 f4[ 	]*vxorps xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 31[ 	]*vxorps xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 b4 f4 c0 1d fe ff[ 	]*vxorps xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 30[ 	]*vxorps xmm6\{k7\},xmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 72 7f[ 	]*vxorps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 b2 00 08 00 00[ 	]*vxorps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 72 80[ 	]*vxorps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 b2 f0 f7 ff ff[ 	]*vxorps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 72 7f[ 	]*vxorps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 b2 00 02 00 00[ 	]*vxorps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 72 80[ 	]*vxorps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 b2 fc fd ff ff[ 	]*vxorps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 f4[ 	]*vxorps ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 57 f4[ 	]*vxorps ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 31[ 	]*vxorps ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 b4 f4 c0 1d fe ff[ 	]*vxorps ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 30[ 	]*vxorps ymm6\{k7\},ymm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 72 7f[ 	]*vxorps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 b2 00 10 00 00[ 	]*vxorps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 72 80[ 	]*vxorps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 b2 e0 ef ff ff[ 	]*vxorps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 72 7f[ 	]*vxorps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 b2 00 02 00 00[ 	]*vxorps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 72 80[ 	]*vxorps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 b2 fc fd ff ff[ 	]*vxorps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 f5 ab[ 	]*vreducepd xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 8f 56 f5 ab[ 	]*vreducepd xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 f5 7b[ 	]*vreducepd xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 31 7b[ 	]*vreducepd xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreducepd xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 30 7b[ 	]*vreducepd xmm6\{k7\},QWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 72 7f 7b[ 	]*vreducepd xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 b2 00 08 00 00 7b[ 	]*vreducepd xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 72 80 7b[ 	]*vreducepd xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 b2 f0 f7 ff ff 7b[ 	]*vreducepd xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 72 7f 7b[ 	]*vreducepd xmm6\{k7\},QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 b2 00 04 00 00 7b[ 	]*vreducepd xmm6\{k7\},QWORD BCST \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 72 80 7b[ 	]*vreducepd xmm6\{k7\},QWORD BCST \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 b2 f8 fb ff ff 7b[ 	]*vreducepd xmm6\{k7\},QWORD BCST \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 f5 ab[ 	]*vreducepd ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd af 56 f5 ab[ 	]*vreducepd ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 f5 7b[ 	]*vreducepd ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 31 7b[ 	]*vreducepd ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreducepd ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 30 7b[ 	]*vreducepd ymm6\{k7\},QWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 72 7f 7b[ 	]*vreducepd ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 b2 00 10 00 00 7b[ 	]*vreducepd ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 72 80 7b[ 	]*vreducepd ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 b2 e0 ef ff ff 7b[ 	]*vreducepd ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 72 7f 7b[ 	]*vreducepd ymm6\{k7\},QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 b2 00 04 00 00 7b[ 	]*vreducepd ymm6\{k7\},QWORD BCST \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 72 80 7b[ 	]*vreducepd ymm6\{k7\},QWORD BCST \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 b2 f8 fb ff ff 7b[ 	]*vreducepd ymm6\{k7\},QWORD BCST \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 f5 ab[ 	]*vreduceps xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 8f 56 f5 ab[ 	]*vreduceps xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 f5 7b[ 	]*vreduceps xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 31 7b[ 	]*vreduceps xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreduceps xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 30 7b[ 	]*vreduceps xmm6\{k7\},DWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 72 7f 7b[ 	]*vreduceps xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 b2 00 08 00 00 7b[ 	]*vreduceps xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 72 80 7b[ 	]*vreduceps xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 b2 f0 f7 ff ff 7b[ 	]*vreduceps xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 72 7f 7b[ 	]*vreduceps xmm6\{k7\},DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 b2 00 02 00 00 7b[ 	]*vreduceps xmm6\{k7\},DWORD BCST \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 72 80 7b[ 	]*vreduceps xmm6\{k7\},DWORD BCST \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 b2 fc fd ff ff 7b[ 	]*vreduceps xmm6\{k7\},DWORD BCST \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 f5 ab[ 	]*vreduceps ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d af 56 f5 ab[ 	]*vreduceps ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 f5 7b[ 	]*vreduceps ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 31 7b[ 	]*vreduceps ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreduceps ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 30 7b[ 	]*vreduceps ymm6\{k7\},DWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 72 7f 7b[ 	]*vreduceps ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 b2 00 10 00 00 7b[ 	]*vreduceps ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 72 80 7b[ 	]*vreduceps ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 b2 e0 ef ff ff 7b[ 	]*vreduceps ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 72 7f 7b[ 	]*vreduceps ymm6\{k7\},DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 b2 00 02 00 00 7b[ 	]*vreduceps ymm6\{k7\},DWORD BCST \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 72 80 7b[ 	]*vreduceps ymm6\{k7\},DWORD BCST \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 b2 fc fd ff ff 7b[ 	]*vreduceps ymm6\{k7\},DWORD BCST \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 29 ab[ 	]*vextractf64x2 XMMWORD PTR \[ecx\]\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 29 7b[ 	]*vextractf64x2 XMMWORD PTR \[ecx\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 ac f4 c0 1d fe ff 7b[ 	]*vextractf64x2 XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 6a 7f 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx\+0x7f0\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 aa 00 08 00 00 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx\+0x800\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 6a 80 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx-0x800\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 aa f0 f7 ff ff 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx-0x810\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 29 ab[ 	]*vextracti64x2 XMMWORD PTR \[ecx\]\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 29 7b[ 	]*vextracti64x2 XMMWORD PTR \[ecx\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 ac f4 c0 1d fe ff 7b[ 	]*vextracti64x2 XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 6a 7f 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx\+0x7f0\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 aa 00 08 00 00 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx\+0x800\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 6a 80 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx-0x800\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 aa f0 f7 ff ff 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx-0x810\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a f5[ 	]*vcvttpd2qq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 7a f5[ 	]*vcvttpd2qq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a 31[ 	]*vcvttpd2qq xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvttpd2qq xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a 30[ 	]*vcvttpd2qq xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a 72 7f[ 	]*vcvttpd2qq xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a b2 00 08 00 00[ 	]*vcvttpd2qq xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a 72 80[ 	]*vcvttpd2qq xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a b2 f0 f7 ff ff[ 	]*vcvttpd2qq xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a 72 7f[ 	]*vcvttpd2qq xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a b2 00 04 00 00[ 	]*vcvttpd2qq xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a 72 80[ 	]*vcvttpd2qq xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a f5[ 	]*vcvttpd2qq ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 7a f5[ 	]*vcvttpd2qq ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a 31[ 	]*vcvttpd2qq ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvttpd2qq ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a 30[ 	]*vcvttpd2qq ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a 72 7f[ 	]*vcvttpd2qq ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a b2 00 10 00 00[ 	]*vcvttpd2qq ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a 72 80[ 	]*vcvttpd2qq ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a b2 e0 ef ff ff[ 	]*vcvttpd2qq ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a 72 7f[ 	]*vcvttpd2qq ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a b2 00 04 00 00[ 	]*vcvttpd2qq ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a 72 80[ 	]*vcvttpd2qq ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 f5[ 	]*vcvttpd2uqq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 78 f5[ 	]*vcvttpd2uqq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 31[ 	]*vcvttpd2uqq xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 b4 f4 c0 1d fe ff[ 	]*vcvttpd2uqq xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 30[ 	]*vcvttpd2uqq xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 72 7f[ 	]*vcvttpd2uqq xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 b2 00 08 00 00[ 	]*vcvttpd2uqq xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 72 80[ 	]*vcvttpd2uqq xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 b2 f0 f7 ff ff[ 	]*vcvttpd2uqq xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 72 7f[ 	]*vcvttpd2uqq xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 b2 00 04 00 00[ 	]*vcvttpd2uqq xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 72 80[ 	]*vcvttpd2uqq xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 f5[ 	]*vcvttpd2uqq ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 78 f5[ 	]*vcvttpd2uqq ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 31[ 	]*vcvttpd2uqq ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 b4 f4 c0 1d fe ff[ 	]*vcvttpd2uqq ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 30[ 	]*vcvttpd2uqq ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 72 7f[ 	]*vcvttpd2uqq ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 b2 00 10 00 00[ 	]*vcvttpd2uqq ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 72 80[ 	]*vcvttpd2uqq ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 b2 e0 ef ff ff[ 	]*vcvttpd2uqq ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 72 7f[ 	]*vcvttpd2uqq ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 b2 00 04 00 00[ 	]*vcvttpd2uqq ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 72 80[ 	]*vcvttpd2uqq ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a f5[ 	]*vcvttps2qq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 7a f5[ 	]*vcvttps2qq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a 31[ 	]*vcvttps2qq xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvttps2qq xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a 30[ 	]*vcvttps2qq xmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a 72 7f[ 	]*vcvttps2qq xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a b2 00 04 00 00[ 	]*vcvttps2qq xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a 72 80[ 	]*vcvttps2qq xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a b2 f8 fb ff ff[ 	]*vcvttps2qq xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a 72 7f[ 	]*vcvttps2qq xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a b2 00 02 00 00[ 	]*vcvttps2qq xmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a 72 80[ 	]*vcvttps2qq xmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a b2 fc fd ff ff[ 	]*vcvttps2qq xmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a f5[ 	]*vcvttps2qq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 7a f5[ 	]*vcvttps2qq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a 31[ 	]*vcvttps2qq ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvttps2qq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a 30[ 	]*vcvttps2qq ymm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a 72 7f[ 	]*vcvttps2qq ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a b2 00 08 00 00[ 	]*vcvttps2qq ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a 72 80[ 	]*vcvttps2qq ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a b2 f0 f7 ff ff[ 	]*vcvttps2qq ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a 72 7f[ 	]*vcvttps2qq ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a b2 00 02 00 00[ 	]*vcvttps2qq ymm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a 72 80[ 	]*vcvttps2qq ymm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a b2 fc fd ff ff[ 	]*vcvttps2qq ymm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 f5[ 	]*vcvttps2uqq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 78 f5[ 	]*vcvttps2uqq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 31[ 	]*vcvttps2uqq xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 b4 f4 c0 1d fe ff[ 	]*vcvttps2uqq xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 30[ 	]*vcvttps2uqq xmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 72 7f[ 	]*vcvttps2uqq xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 b2 00 04 00 00[ 	]*vcvttps2uqq xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 72 80[ 	]*vcvttps2uqq xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 b2 f8 fb ff ff[ 	]*vcvttps2uqq xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 72 7f[ 	]*vcvttps2uqq xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 b2 00 02 00 00[ 	]*vcvttps2uqq xmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 72 80[ 	]*vcvttps2uqq xmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 b2 fc fd ff ff[ 	]*vcvttps2uqq xmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 f5[ 	]*vcvttps2uqq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 78 f5[ 	]*vcvttps2uqq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 31[ 	]*vcvttps2uqq ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 b4 f4 c0 1d fe ff[ 	]*vcvttps2uqq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 30[ 	]*vcvttps2uqq ymm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 72 7f[ 	]*vcvttps2uqq ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 b2 00 08 00 00[ 	]*vcvttps2uqq ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 72 80[ 	]*vcvttps2uqq ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 b2 f0 f7 ff ff[ 	]*vcvttps2uqq ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 72 7f[ 	]*vcvttps2uqq ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 b2 00 02 00 00[ 	]*vcvttps2uqq ymm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 72 80[ 	]*vcvttps2uqq ymm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 b2 fc fd ff ff[ 	]*vcvttps2uqq ymm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 39 ee[ 	]*vpmovd2m k5,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 39 ee[ 	]*vpmovd2m k5,ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 39 ee[ 	]*vpmovq2m k5,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 39 ee[ 	]*vpmovq2m k5,ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 38 f5[ 	]*vpmovm2d xmm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 38 f5[ 	]*vpmovm2d ymm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 38 f5[ 	]*vpmovm2q xmm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 38 f5[ 	]*vpmovm2q ymm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a 31[ 	]*vbroadcastf64x2 ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 1a 31[ 	]*vbroadcastf64x2 ymm6\{k7\}\{z\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a b4 f4 c0 1d fe ff[ 	]*vbroadcastf64x2 ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a 72 7f[ 	]*vbroadcastf64x2 ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a 72 80[ 	]*vbroadcastf64x2 ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a 31[ 	]*vbroadcasti64x2 ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 5a 31[ 	]*vbroadcasti64x2 ymm6\{k7\}\{z\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a b4 f4 c0 1d fe ff[ 	]*vbroadcasti64x2 ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a 72 7f[ 	]*vbroadcasti64x2 ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a 72 80[ 	]*vbroadcasti64x2 ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 f7[ 	]*vbroadcastf32x2 ymm6\{k7\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 19 f7[ 	]*vbroadcastf32x2 ymm6\{k7\}\{z\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 31[ 	]*vbroadcastf32x2 ymm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 b4 f4 c0 1d fe ff[ 	]*vbroadcastf32x2 ymm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 72 7f[ 	]*vbroadcastf32x2 ymm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 ymm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 72 80[ 	]*vbroadcastf32x2 ymm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 ymm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b f5[ 	]*vcvtpd2qq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 7b f5[ 	]*vcvtpd2qq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b 31[ 	]*vcvtpd2qq xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b b4 f4 c0 1d fe ff[ 	]*vcvtpd2qq xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b 30[ 	]*vcvtpd2qq xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b 72 7f[ 	]*vcvtpd2qq xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b b2 00 08 00 00[ 	]*vcvtpd2qq xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b 72 80[ 	]*vcvtpd2qq xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7b b2 f0 f7 ff ff[ 	]*vcvtpd2qq xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b 72 7f[ 	]*vcvtpd2qq xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b b2 00 04 00 00[ 	]*vcvtpd2qq xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b 72 80[ 	]*vcvtpd2qq xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b f5[ 	]*vcvtpd2qq ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 7b f5[ 	]*vcvtpd2qq ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b 31[ 	]*vcvtpd2qq ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b b4 f4 c0 1d fe ff[ 	]*vcvtpd2qq ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b 30[ 	]*vcvtpd2qq ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b 72 7f[ 	]*vcvtpd2qq ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b b2 00 10 00 00[ 	]*vcvtpd2qq ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b 72 80[ 	]*vcvtpd2qq ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7b b2 e0 ef ff ff[ 	]*vcvtpd2qq ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b 72 7f[ 	]*vcvtpd2qq ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b b2 00 04 00 00[ 	]*vcvtpd2qq ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b 72 80[ 	]*vcvtpd2qq ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 f5[ 	]*vcvtpd2uqq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 79 f5[ 	]*vcvtpd2uqq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 31[ 	]*vcvtpd2uqq xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 b4 f4 c0 1d fe ff[ 	]*vcvtpd2uqq xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 30[ 	]*vcvtpd2uqq xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 72 7f[ 	]*vcvtpd2uqq xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 b2 00 08 00 00[ 	]*vcvtpd2uqq xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 72 80[ 	]*vcvtpd2uqq xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 79 b2 f0 f7 ff ff[ 	]*vcvtpd2uqq xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 72 7f[ 	]*vcvtpd2uqq xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 b2 00 04 00 00[ 	]*vcvtpd2uqq xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 72 80[ 	]*vcvtpd2uqq xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 f5[ 	]*vcvtpd2uqq ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 79 f5[ 	]*vcvtpd2uqq ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 31[ 	]*vcvtpd2uqq ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 b4 f4 c0 1d fe ff[ 	]*vcvtpd2uqq ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 30[ 	]*vcvtpd2uqq ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 72 7f[ 	]*vcvtpd2uqq ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 b2 00 10 00 00[ 	]*vcvtpd2uqq ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 72 80[ 	]*vcvtpd2uqq ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 79 b2 e0 ef ff ff[ 	]*vcvtpd2uqq ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 72 7f[ 	]*vcvtpd2uqq ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 b2 00 04 00 00[ 	]*vcvtpd2uqq ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 72 80[ 	]*vcvtpd2uqq ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b f5[ 	]*vcvtps2qq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 7b f5[ 	]*vcvtps2qq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b 31[ 	]*vcvtps2qq xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b b4 f4 c0 1d fe ff[ 	]*vcvtps2qq xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b 30[ 	]*vcvtps2qq xmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b 72 7f[ 	]*vcvtps2qq xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b b2 00 04 00 00[ 	]*vcvtps2qq xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b 72 80[ 	]*vcvtps2qq xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7b b2 f8 fb ff ff[ 	]*vcvtps2qq xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b 72 7f[ 	]*vcvtps2qq xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b b2 00 02 00 00[ 	]*vcvtps2qq xmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b 72 80[ 	]*vcvtps2qq xmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b b2 fc fd ff ff[ 	]*vcvtps2qq xmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b 72 7f[ 	]*vcvtps2qq xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b f5[ 	]*vcvtps2qq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 7b f5[ 	]*vcvtps2qq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b 31[ 	]*vcvtps2qq ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b b4 f4 c0 1d fe ff[ 	]*vcvtps2qq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b 30[ 	]*vcvtps2qq ymm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b 72 7f[ 	]*vcvtps2qq ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b b2 00 08 00 00[ 	]*vcvtps2qq ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b 72 80[ 	]*vcvtps2qq ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7b b2 f0 f7 ff ff[ 	]*vcvtps2qq ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b 72 7f[ 	]*vcvtps2qq ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b b2 00 02 00 00[ 	]*vcvtps2qq ymm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b 72 80[ 	]*vcvtps2qq ymm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b b2 fc fd ff ff[ 	]*vcvtps2qq ymm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b 72 7f[ 	]*vcvtps2qq ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 f5[ 	]*vcvtps2uqq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 79 f5[ 	]*vcvtps2uqq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 31[ 	]*vcvtps2uqq xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 b4 f4 c0 1d fe ff[ 	]*vcvtps2uqq xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 30[ 	]*vcvtps2uqq xmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 72 7f[ 	]*vcvtps2uqq xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 b2 00 04 00 00[ 	]*vcvtps2uqq xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 72 80[ 	]*vcvtps2uqq xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 79 b2 f8 fb ff ff[ 	]*vcvtps2uqq xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 72 7f[ 	]*vcvtps2uqq xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 b2 00 02 00 00[ 	]*vcvtps2uqq xmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 72 80[ 	]*vcvtps2uqq xmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 b2 fc fd ff ff[ 	]*vcvtps2uqq xmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 72 7f[ 	]*vcvtps2uqq xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 f5[ 	]*vcvtps2uqq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 79 f5[ 	]*vcvtps2uqq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 31[ 	]*vcvtps2uqq ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 b4 f4 c0 1d fe ff[ 	]*vcvtps2uqq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 30[ 	]*vcvtps2uqq ymm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 72 7f[ 	]*vcvtps2uqq ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 b2 00 08 00 00[ 	]*vcvtps2uqq ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 72 80[ 	]*vcvtps2uqq ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 79 b2 f0 f7 ff ff[ 	]*vcvtps2uqq ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 72 7f[ 	]*vcvtps2uqq ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 b2 00 02 00 00[ 	]*vcvtps2uqq ymm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 72 80[ 	]*vcvtps2uqq ymm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 b2 fc fd ff ff[ 	]*vcvtps2uqq ymm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 72 7f[ 	]*vcvtps2uqq ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 f5[ 	]*vcvtqq2pd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 8f e6 f5[ 	]*vcvtqq2pd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 31[ 	]*vcvtqq2pd xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 b4 f4 c0 1d fe ff[ 	]*vcvtqq2pd xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 30[ 	]*vcvtqq2pd xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 72 7f[ 	]*vcvtqq2pd xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 b2 00 08 00 00[ 	]*vcvtqq2pd xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 72 80[ 	]*vcvtqq2pd xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f e6 b2 f0 f7 ff ff[ 	]*vcvtqq2pd xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 72 7f[ 	]*vcvtqq2pd xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 b2 00 04 00 00[ 	]*vcvtqq2pd xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 72 80[ 	]*vcvtqq2pd xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 f5[ 	]*vcvtqq2pd ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe af e6 f5[ 	]*vcvtqq2pd ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 31[ 	]*vcvtqq2pd ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 b4 f4 c0 1d fe ff[ 	]*vcvtqq2pd ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 30[ 	]*vcvtqq2pd ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 72 7f[ 	]*vcvtqq2pd ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 b2 00 10 00 00[ 	]*vcvtqq2pd ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 72 80[ 	]*vcvtqq2pd ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f e6 b2 e0 ef ff ff[ 	]*vcvtqq2pd ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 72 7f[ 	]*vcvtqq2pd ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 b2 00 04 00 00[ 	]*vcvtqq2pd ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 72 80[ 	]*vcvtqq2pd ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b f5[ 	]*vcvtqq2ps xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 8f 5b f5[ 	]*vcvtqq2ps xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b 31[ 	]*vcvtqq2ps xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b b4 f4 c0 1d fe ff[ 	]*vcvtqq2ps xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b 30[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[eax\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b 72 7f[ 	]*vcvtqq2ps xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b b2 00 08 00 00[ 	]*vcvtqq2ps xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b 72 80[ 	]*vcvtqq2ps xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 0f 5b b2 f0 f7 ff ff[ 	]*vcvtqq2ps xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b 72 7f[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b b2 00 04 00 00[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b 72 80[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx-0x408\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b f5[ 	]*vcvtqq2ps xmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc af 5b f5[ 	]*vcvtqq2ps xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b 31[ 	]*vcvtqq2ps xmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b b4 f4 c0 1d fe ff[ 	]*vcvtqq2ps xmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b 30[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[eax\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b 72 7f[ 	]*vcvtqq2ps xmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b b2 00 10 00 00[ 	]*vcvtqq2ps xmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b 72 80[ 	]*vcvtqq2ps xmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 2f 5b b2 e0 ef ff ff[ 	]*vcvtqq2ps xmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b 72 7f[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b b2 00 04 00 00[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b 72 80[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps xmm6\{k7\},QWORD BCST \[edx-0x408\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a f5[ 	]*vcvtuqq2pd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 8f 7a f5[ 	]*vcvtuqq2pd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a 31[ 	]*vcvtuqq2pd xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2pd xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a 30[ 	]*vcvtuqq2pd xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a 72 7f[ 	]*vcvtuqq2pd xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a b2 00 08 00 00[ 	]*vcvtuqq2pd xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a 72 80[ 	]*vcvtuqq2pd xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2pd xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a 72 7f[ 	]*vcvtuqq2pd xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a b2 00 04 00 00[ 	]*vcvtuqq2pd xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a 72 80[ 	]*vcvtuqq2pd xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 1f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a f5[ 	]*vcvtuqq2pd ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe af 7a f5[ 	]*vcvtuqq2pd ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a 31[ 	]*vcvtuqq2pd ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2pd ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a 30[ 	]*vcvtuqq2pd ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a 72 7f[ 	]*vcvtuqq2pd ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a b2 00 10 00 00[ 	]*vcvtuqq2pd ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a 72 80[ 	]*vcvtuqq2pd ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 7a b2 e0 ef ff ff[ 	]*vcvtuqq2pd ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a 72 7f[ 	]*vcvtuqq2pd ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a b2 00 04 00 00[ 	]*vcvtuqq2pd ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a 72 80[ 	]*vcvtuqq2pd ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 3f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a f5[ 	]*vcvtuqq2ps xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 7a f5[ 	]*vcvtuqq2ps xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a 31[ 	]*vcvtuqq2ps xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2ps xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a 30[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[eax\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a 72 7f[ 	]*vcvtuqq2ps xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a b2 00 08 00 00[ 	]*vcvtuqq2ps xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a 72 80[ 	]*vcvtuqq2ps xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7a b2 f0 f7 ff ff[ 	]*vcvtuqq2ps xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a 72 7f[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a b2 00 04 00 00[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a 72 80[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx-0x400\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx-0x408\]\{1to2\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a f5[ 	]*vcvtuqq2ps xmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 7a f5[ 	]*vcvtuqq2ps xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a 31[ 	]*vcvtuqq2ps xmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2ps xmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a 30[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[eax\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a 72 7f[ 	]*vcvtuqq2ps xmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a b2 00 10 00 00[ 	]*vcvtuqq2ps xmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a 72 80[ 	]*vcvtuqq2ps xmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7a b2 e0 ef ff ff[ 	]*vcvtuqq2ps xmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a 72 7f[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a b2 00 04 00 00[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx\+0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a 72 80[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx-0x400\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps xmm6\{k7\},QWORD BCST \[edx-0x408\]\{1to4\}
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 ee ab[ 	]*vextractf64x2 xmm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd af 19 ee ab[ 	]*vextractf64x2 xmm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 ee 7b[ 	]*vextractf64x2 xmm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 ee ab[ 	]*vextracti64x2 xmm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd af 39 ee ab[ 	]*vextracti64x2 xmm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 ee 7b[ 	]*vextracti64x2 xmm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 ee ab[ 	]*vfpclasspd k5\{k7\},xmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 ee 7b[ 	]*vfpclasspd k5\{k7\},xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 29 7b[ 	]*vfpclasspd k5\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspd k5\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 28 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[eax\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 6a 7f 7b[ 	]*vfpclasspd k5\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 aa 00 08 00 00 7b[ 	]*vfpclasspd k5\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 6a 80 7b[ 	]*vfpclasspd k5\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 66 aa f0 f7 ff ff 7b[ 	]*vfpclasspd k5\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 6a 7f 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx\+0x3f8\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 aa 00 04 00 00 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx\+0x400\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 6a 80 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx-0x400\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx-0x408\]\{1to2\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 ee ab[ 	]*vfpclasspd k5\{k7\},ymm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 ee 7b[ 	]*vfpclasspd k5\{k7\},ymm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 29 7b[ 	]*vfpclasspd k5\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspd k5\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 28 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[eax\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 6a 7f 7b[ 	]*vfpclasspd k5\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 aa 00 10 00 00 7b[ 	]*vfpclasspd k5\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 6a 80 7b[ 	]*vfpclasspd k5\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 66 aa e0 ef ff ff 7b[ 	]*vfpclasspd k5\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 6a 7f 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx\+0x3f8\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 aa 00 04 00 00 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx\+0x400\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 6a 80 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx-0x400\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd k5\{k7\},QWORD BCST \[edx-0x408\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 ee ab[ 	]*vfpclassps k5\{k7\},xmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 ee 7b[ 	]*vfpclassps k5\{k7\},xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 29 7b[ 	]*vfpclassps k5\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclassps k5\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 28 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[eax\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 6a 7f 7b[ 	]*vfpclassps k5\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 aa 00 08 00 00 7b[ 	]*vfpclassps k5\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 6a 80 7b[ 	]*vfpclassps k5\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 66 aa f0 f7 ff ff 7b[ 	]*vfpclassps k5\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 6a 7f 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx\+0x1fc\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 aa 00 02 00 00 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx\+0x200\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 6a 80 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx-0x200\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 66 aa fc fd ff ff 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx-0x204\]\{1to4\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 ee ab[ 	]*vfpclassps k5\{k7\},ymm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 ee 7b[ 	]*vfpclassps k5\{k7\},ymm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 29 7b[ 	]*vfpclassps k5\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclassps k5\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 28 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[eax\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 6a 7f 7b[ 	]*vfpclassps k5\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 aa 00 10 00 00 7b[ 	]*vfpclassps k5\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 6a 80 7b[ 	]*vfpclassps k5\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 66 aa e0 ef ff ff 7b[ 	]*vfpclassps k5\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 6a 7f 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx\+0x1fc\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 aa 00 02 00 00 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx\+0x200\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 6a 80 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx-0x200\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 66 aa fc fd ff ff 7b[ 	]*vfpclassps k5\{k7\},DWORD BCST \[edx-0x204\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 f4 ab[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 18 f4 ab[ 	]*vinsertf64x2 ymm6\{k7\}\{z\},ymm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 f4 7b[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 31 7b[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 b4 f4 c0 1d fe ff 7b[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 72 7f 7b[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 72 80 7b[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 f4 ab[ 	]*vinserti64x2 ymm6\{k7\},ymm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 38 f4 ab[ 	]*vinserti64x2 ymm6\{k7\}\{z\},ymm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 f4 7b[ 	]*vinserti64x2 ymm6\{k7\},ymm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 31 7b[ 	]*vinserti64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 b4 f4 c0 1d fe ff 7b[ 	]*vinserti64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 72 7f 7b[ 	]*vinserti64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 72 80 7b[ 	]*vinserti64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 f7[ 	]*vbroadcasti32x2 xmm6\{k7\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 59 f7[ 	]*vbroadcasti32x2 xmm6\{k7\}\{z\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 31[ 	]*vbroadcasti32x2 xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x2 xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 72 7f[ 	]*vbroadcasti32x2 xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 72 80[ 	]*vbroadcasti32x2 xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 f7[ 	]*vbroadcasti32x2 ymm6\{k7\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 59 f7[ 	]*vbroadcasti32x2 ymm6\{k7\}\{z\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 31[ 	]*vbroadcasti32x2 ymm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x2 ymm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 72 7f[ 	]*vbroadcasti32x2 ymm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 ymm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 72 80[ 	]*vbroadcasti32x2 ymm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 ymm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 f4[ 	]*vpmullq xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 40 f4[ 	]*vpmullq xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 31[ 	]*vpmullq xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 b4 f4 c0 1d fe ff[ 	]*vpmullq xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 30[ 	]*vpmullq xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 72 7f[ 	]*vpmullq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 b2 00 08 00 00[ 	]*vpmullq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 72 80[ 	]*vpmullq xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 40 b2 f0 f7 ff ff[ 	]*vpmullq xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 72 7f[ 	]*vpmullq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 b2 00 04 00 00[ 	]*vpmullq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 72 80[ 	]*vpmullq xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 40 b2 f8 fb ff ff[ 	]*vpmullq xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 f4[ 	]*vpmullq ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 40 f4[ 	]*vpmullq ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 31[ 	]*vpmullq ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 b4 f4 c0 1d fe ff[ 	]*vpmullq ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 30[ 	]*vpmullq ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 72 7f[ 	]*vpmullq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 b2 00 10 00 00[ 	]*vpmullq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 72 80[ 	]*vpmullq ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 40 b2 e0 ef ff ff[ 	]*vpmullq ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 72 7f[ 	]*vpmullq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 b2 00 04 00 00[ 	]*vpmullq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 72 80[ 	]*vpmullq ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 40 b2 f8 fb ff ff[ 	]*vpmullq ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 f4 ab[ 	]*vrangepd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 50 f4 ab[ 	]*vrangepd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 f4 7b[ 	]*vrangepd xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 31 7b[ 	]*vrangepd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangepd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 30 7b[ 	]*vrangepd xmm6\{k7\},xmm5,QWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 72 7f 7b[ 	]*vrangepd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 b2 00 08 00 00 7b[ 	]*vrangepd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 72 80 7b[ 	]*vrangepd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 50 b2 f0 f7 ff ff 7b[ 	]*vrangepd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 72 7f 7b[ 	]*vrangepd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 b2 00 04 00 00 7b[ 	]*vrangepd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 72 80 7b[ 	]*vrangepd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 50 b2 f8 fb ff ff 7b[ 	]*vrangepd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 f4 ab[ 	]*vrangepd ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 50 f4 ab[ 	]*vrangepd ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 f4 7b[ 	]*vrangepd ymm6\{k7\},ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 31 7b[ 	]*vrangepd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangepd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 30 7b[ 	]*vrangepd ymm6\{k7\},ymm5,QWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 72 7f 7b[ 	]*vrangepd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 b2 00 10 00 00 7b[ 	]*vrangepd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 72 80 7b[ 	]*vrangepd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 50 b2 e0 ef ff ff 7b[ 	]*vrangepd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 72 7f 7b[ 	]*vrangepd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 b2 00 04 00 00 7b[ 	]*vrangepd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 72 80 7b[ 	]*vrangepd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f 50 b2 f8 fb ff ff 7b[ 	]*vrangepd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 f4 ab[ 	]*vrangeps xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 50 f4 ab[ 	]*vrangeps xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 f4 7b[ 	]*vrangeps xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 31 7b[ 	]*vrangeps xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangeps xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 30 7b[ 	]*vrangeps xmm6\{k7\},xmm5,DWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 72 7f 7b[ 	]*vrangeps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 b2 00 08 00 00 7b[ 	]*vrangeps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 72 80 7b[ 	]*vrangeps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 50 b2 f0 f7 ff ff 7b[ 	]*vrangeps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 72 7f 7b[ 	]*vrangeps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 b2 00 02 00 00 7b[ 	]*vrangeps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 72 80 7b[ 	]*vrangeps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 50 b2 fc fd ff ff 7b[ 	]*vrangeps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 f4 ab[ 	]*vrangeps ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 50 f4 ab[ 	]*vrangeps ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 f4 7b[ 	]*vrangeps ymm6\{k7\},ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 31 7b[ 	]*vrangeps ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangeps ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 30 7b[ 	]*vrangeps ymm6\{k7\},ymm5,DWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 72 7f 7b[ 	]*vrangeps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 b2 00 10 00 00 7b[ 	]*vrangeps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 72 80 7b[ 	]*vrangeps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 50 b2 e0 ef ff ff 7b[ 	]*vrangeps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 72 7f 7b[ 	]*vrangeps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 b2 00 02 00 00 7b[ 	]*vrangeps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 72 80 7b[ 	]*vrangeps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 3f 50 b2 fc fd ff ff 7b[ 	]*vrangeps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 f4[ 	]*vandpd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 54 f4[ 	]*vandpd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 31[ 	]*vandpd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 b4 f4 c0 1d fe ff[ 	]*vandpd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 30[ 	]*vandpd xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 72 7f[ 	]*vandpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 b2 00 08 00 00[ 	]*vandpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 72 80[ 	]*vandpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 54 b2 f0 f7 ff ff[ 	]*vandpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 72 7f[ 	]*vandpd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 b2 00 04 00 00[ 	]*vandpd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 72 80[ 	]*vandpd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 54 b2 f8 fb ff ff[ 	]*vandpd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 f4[ 	]*vandpd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 54 f4[ 	]*vandpd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 31[ 	]*vandpd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 b4 f4 c0 1d fe ff[ 	]*vandpd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 30[ 	]*vandpd ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 72 7f[ 	]*vandpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 b2 00 10 00 00[ 	]*vandpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 72 80[ 	]*vandpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 54 b2 e0 ef ff ff[ 	]*vandpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 72 7f[ 	]*vandpd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 b2 00 04 00 00[ 	]*vandpd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 72 80[ 	]*vandpd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 54 b2 f8 fb ff ff[ 	]*vandpd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 f4[ 	]*vandps xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 54 f4[ 	]*vandps xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 31[ 	]*vandps xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 b4 f4 c0 1d fe ff[ 	]*vandps xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 30[ 	]*vandps xmm6\{k7\},xmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 72 7f[ 	]*vandps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 b2 00 08 00 00[ 	]*vandps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 72 80[ 	]*vandps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 54 b2 f0 f7 ff ff[ 	]*vandps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 72 7f[ 	]*vandps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 b2 00 02 00 00[ 	]*vandps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 72 80[ 	]*vandps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 54 b2 fc fd ff ff[ 	]*vandps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 f4[ 	]*vandps ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 54 f4[ 	]*vandps ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 31[ 	]*vandps ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 b4 f4 c0 1d fe ff[ 	]*vandps ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 30[ 	]*vandps ymm6\{k7\},ymm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 72 7f[ 	]*vandps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 b2 00 10 00 00[ 	]*vandps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 72 80[ 	]*vandps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 54 b2 e0 ef ff ff[ 	]*vandps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 72 7f[ 	]*vandps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 b2 00 02 00 00[ 	]*vandps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 72 80[ 	]*vandps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 54 b2 fc fd ff ff[ 	]*vandps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 f4[ 	]*vandnpd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 55 f4[ 	]*vandnpd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 31[ 	]*vandnpd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 b4 f4 c0 1d fe ff[ 	]*vandnpd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 30[ 	]*vandnpd xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 72 7f[ 	]*vandnpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 b2 00 08 00 00[ 	]*vandnpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 72 80[ 	]*vandnpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 55 b2 f0 f7 ff ff[ 	]*vandnpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 72 7f[ 	]*vandnpd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 b2 00 04 00 00[ 	]*vandnpd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 72 80[ 	]*vandnpd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 55 b2 f8 fb ff ff[ 	]*vandnpd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 f4[ 	]*vandnpd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 55 f4[ 	]*vandnpd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 31[ 	]*vandnpd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 b4 f4 c0 1d fe ff[ 	]*vandnpd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 30[ 	]*vandnpd ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 72 7f[ 	]*vandnpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 b2 00 10 00 00[ 	]*vandnpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 72 80[ 	]*vandnpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 55 b2 e0 ef ff ff[ 	]*vandnpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 72 7f[ 	]*vandnpd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 b2 00 04 00 00[ 	]*vandnpd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 72 80[ 	]*vandnpd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 55 b2 f8 fb ff ff[ 	]*vandnpd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 f4[ 	]*vandnps xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 55 f4[ 	]*vandnps xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 31[ 	]*vandnps xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 b4 f4 c0 1d fe ff[ 	]*vandnps xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 30[ 	]*vandnps xmm6\{k7\},xmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 72 7f[ 	]*vandnps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 b2 00 08 00 00[ 	]*vandnps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 72 80[ 	]*vandnps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 55 b2 f0 f7 ff ff[ 	]*vandnps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 72 7f[ 	]*vandnps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 b2 00 02 00 00[ 	]*vandnps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 72 80[ 	]*vandnps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 55 b2 fc fd ff ff[ 	]*vandnps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 f4[ 	]*vandnps ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 55 f4[ 	]*vandnps ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 31[ 	]*vandnps ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 b4 f4 c0 1d fe ff[ 	]*vandnps ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 30[ 	]*vandnps ymm6\{k7\},ymm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 72 7f[ 	]*vandnps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 b2 00 10 00 00[ 	]*vandnps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 72 80[ 	]*vandnps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 55 b2 e0 ef ff ff[ 	]*vandnps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 72 7f[ 	]*vandnps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 b2 00 02 00 00[ 	]*vandnps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 72 80[ 	]*vandnps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 55 b2 fc fd ff ff[ 	]*vandnps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 f4[ 	]*vorpd  xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 56 f4[ 	]*vorpd  xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 31[ 	]*vorpd  xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 b4 f4 c0 1d fe ff[ 	]*vorpd  xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 30[ 	]*vorpd  xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 72 7f[ 	]*vorpd  xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 b2 00 08 00 00[ 	]*vorpd  xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 72 80[ 	]*vorpd  xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 56 b2 f0 f7 ff ff[ 	]*vorpd  xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 72 7f[ 	]*vorpd  xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 b2 00 04 00 00[ 	]*vorpd  xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 72 80[ 	]*vorpd  xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 56 b2 f8 fb ff ff[ 	]*vorpd  xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 f4[ 	]*vorpd  ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 56 f4[ 	]*vorpd  ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 31[ 	]*vorpd  ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 b4 f4 c0 1d fe ff[ 	]*vorpd  ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 30[ 	]*vorpd  ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 72 7f[ 	]*vorpd  ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 b2 00 10 00 00[ 	]*vorpd  ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 72 80[ 	]*vorpd  ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 56 b2 e0 ef ff ff[ 	]*vorpd  ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 72 7f[ 	]*vorpd  ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 b2 00 04 00 00[ 	]*vorpd  ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 72 80[ 	]*vorpd  ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 56 b2 f8 fb ff ff[ 	]*vorpd  ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 f4[ 	]*vorps  xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 56 f4[ 	]*vorps  xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 31[ 	]*vorps  xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 b4 f4 c0 1d fe ff[ 	]*vorps  xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 30[ 	]*vorps  xmm6\{k7\},xmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 72 7f[ 	]*vorps  xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 b2 00 08 00 00[ 	]*vorps  xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 72 80[ 	]*vorps  xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 56 b2 f0 f7 ff ff[ 	]*vorps  xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 72 7f[ 	]*vorps  xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 b2 00 02 00 00[ 	]*vorps  xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 72 80[ 	]*vorps  xmm6\{k7\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 56 b2 fc fd ff ff[ 	]*vorps  xmm6\{k7\},xmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 f4[ 	]*vorps  ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 56 f4[ 	]*vorps  ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 31[ 	]*vorps  ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 b4 f4 c0 1d fe ff[ 	]*vorps  ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 30[ 	]*vorps  ymm6\{k7\},ymm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 72 7f[ 	]*vorps  ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 b2 00 10 00 00[ 	]*vorps  ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 72 80[ 	]*vorps  ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 56 b2 e0 ef ff ff[ 	]*vorps  ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 72 7f[ 	]*vorps  ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 b2 00 02 00 00[ 	]*vorps  ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 72 80[ 	]*vorps  ymm6\{k7\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 56 b2 fc fd ff ff[ 	]*vorps  ymm6\{k7\},ymm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 f4[ 	]*vxorpd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 57 f4[ 	]*vxorpd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 31[ 	]*vxorpd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 b4 f4 c0 1d fe ff[ 	]*vxorpd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 30[ 	]*vxorpd xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 72 7f[ 	]*vxorpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 b2 00 08 00 00[ 	]*vxorpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 72 80[ 	]*vxorpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 57 b2 f0 f7 ff ff[ 	]*vxorpd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 72 7f[ 	]*vxorpd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 b2 00 04 00 00[ 	]*vxorpd xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 72 80[ 	]*vxorpd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 1f 57 b2 f8 fb ff ff[ 	]*vxorpd xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 f4[ 	]*vxorpd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 57 f4[ 	]*vxorpd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 31[ 	]*vxorpd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 b4 f4 c0 1d fe ff[ 	]*vxorpd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 30[ 	]*vxorpd ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 72 7f[ 	]*vxorpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 b2 00 10 00 00[ 	]*vxorpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 72 80[ 	]*vxorpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 57 b2 e0 ef ff ff[ 	]*vxorpd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 72 7f[ 	]*vxorpd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 b2 00 04 00 00[ 	]*vxorpd ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 72 80[ 	]*vxorpd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 3f 57 b2 f8 fb ff ff[ 	]*vxorpd ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 f4[ 	]*vxorps xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 8f 57 f4[ 	]*vxorps xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 31[ 	]*vxorps xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 b4 f4 c0 1d fe ff[ 	]*vxorps xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 30[ 	]*vxorps xmm6\{k7\},xmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 72 7f[ 	]*vxorps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 b2 00 08 00 00[ 	]*vxorps xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 72 80[ 	]*vxorps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 0f 57 b2 f0 f7 ff ff[ 	]*vxorps xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 72 7f[ 	]*vxorps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 b2 00 02 00 00[ 	]*vxorps xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 72 80[ 	]*vxorps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 1f 57 b2 fc fd ff ff[ 	]*vxorps xmm6\{k7\},xmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 f4[ 	]*vxorps ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 af 57 f4[ 	]*vxorps ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 31[ 	]*vxorps ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 b4 f4 c0 1d fe ff[ 	]*vxorps ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 30[ 	]*vxorps ymm6\{k7\},ymm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 72 7f[ 	]*vxorps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 b2 00 10 00 00[ 	]*vxorps ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 72 80[ 	]*vxorps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 2f 57 b2 e0 ef ff ff[ 	]*vxorps ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 72 7f[ 	]*vxorps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 b2 00 02 00 00[ 	]*vxorps ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 72 80[ 	]*vxorps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 3f 57 b2 fc fd ff ff[ 	]*vxorps ymm6\{k7\},ymm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 f5 ab[ 	]*vreducepd xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 8f 56 f5 ab[ 	]*vreducepd xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 f5 7b[ 	]*vreducepd xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 31 7b[ 	]*vreducepd xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreducepd xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 30 7b[ 	]*vreducepd xmm6\{k7\},QWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 72 7f 7b[ 	]*vreducepd xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 b2 00 08 00 00 7b[ 	]*vreducepd xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 72 80 7b[ 	]*vreducepd xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 56 b2 f0 f7 ff ff 7b[ 	]*vreducepd xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 72 7f 7b[ 	]*vreducepd xmm6\{k7\},QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 b2 00 04 00 00 7b[ 	]*vreducepd xmm6\{k7\},QWORD BCST \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 72 80 7b[ 	]*vreducepd xmm6\{k7\},QWORD BCST \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 1f 56 b2 f8 fb ff ff 7b[ 	]*vreducepd xmm6\{k7\},QWORD BCST \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 f5 ab[ 	]*vreducepd ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd af 56 f5 ab[ 	]*vreducepd ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 f5 7b[ 	]*vreducepd ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 31 7b[ 	]*vreducepd ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreducepd ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 30 7b[ 	]*vreducepd ymm6\{k7\},QWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 72 7f 7b[ 	]*vreducepd ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 b2 00 10 00 00 7b[ 	]*vreducepd ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 72 80 7b[ 	]*vreducepd ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 56 b2 e0 ef ff ff 7b[ 	]*vreducepd ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 72 7f 7b[ 	]*vreducepd ymm6\{k7\},QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 b2 00 04 00 00 7b[ 	]*vreducepd ymm6\{k7\},QWORD BCST \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 72 80 7b[ 	]*vreducepd ymm6\{k7\},QWORD BCST \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 3f 56 b2 f8 fb ff ff 7b[ 	]*vreducepd ymm6\{k7\},QWORD BCST \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 f5 ab[ 	]*vreduceps xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 8f 56 f5 ab[ 	]*vreduceps xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 f5 7b[ 	]*vreduceps xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 31 7b[ 	]*vreduceps xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreduceps xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 30 7b[ 	]*vreduceps xmm6\{k7\},DWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 72 7f 7b[ 	]*vreduceps xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 b2 00 08 00 00 7b[ 	]*vreduceps xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 72 80 7b[ 	]*vreduceps xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 56 b2 f0 f7 ff ff 7b[ 	]*vreduceps xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 72 7f 7b[ 	]*vreduceps xmm6\{k7\},DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 b2 00 02 00 00 7b[ 	]*vreduceps xmm6\{k7\},DWORD BCST \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 72 80 7b[ 	]*vreduceps xmm6\{k7\},DWORD BCST \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 1f 56 b2 fc fd ff ff 7b[ 	]*vreduceps xmm6\{k7\},DWORD BCST \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 f5 ab[ 	]*vreduceps ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d af 56 f5 ab[ 	]*vreduceps ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 f5 7b[ 	]*vreduceps ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 31 7b[ 	]*vreduceps ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 b4 f4 c0 1d fe ff 7b[ 	]*vreduceps ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 30 7b[ 	]*vreduceps ymm6\{k7\},DWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 72 7f 7b[ 	]*vreduceps ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 b2 00 10 00 00 7b[ 	]*vreduceps ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 72 80 7b[ 	]*vreduceps ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 2f 56 b2 e0 ef ff ff 7b[ 	]*vreduceps ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 72 7f 7b[ 	]*vreduceps ymm6\{k7\},DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 b2 00 02 00 00 7b[ 	]*vreduceps ymm6\{k7\},DWORD BCST \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 72 80 7b[ 	]*vreduceps ymm6\{k7\},DWORD BCST \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 3f 56 b2 fc fd ff ff 7b[ 	]*vreduceps ymm6\{k7\},DWORD BCST \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 29 ab[ 	]*vextractf64x2 XMMWORD PTR \[ecx\]\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 29 7b[ 	]*vextractf64x2 XMMWORD PTR \[ecx\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 ac f4 c0 1d fe ff 7b[ 	]*vextractf64x2 XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 6a 7f 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx\+0x7f0\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 aa 00 08 00 00 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx\+0x800\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 6a 80 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx-0x800\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 19 aa f0 f7 ff ff 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx-0x810\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 29 ab[ 	]*vextracti64x2 XMMWORD PTR \[ecx\]\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 29 7b[ 	]*vextracti64x2 XMMWORD PTR \[ecx\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 ac f4 c0 1d fe ff 7b[ 	]*vextracti64x2 XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 6a 7f 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx\+0x7f0\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 aa 00 08 00 00 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx\+0x800\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 6a 80 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx-0x800\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 2f 39 aa f0 f7 ff ff 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx-0x810\]\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a f5[ 	]*vcvttpd2qq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 7a f5[ 	]*vcvttpd2qq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a 31[ 	]*vcvttpd2qq xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvttpd2qq xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a 30[ 	]*vcvttpd2qq xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a 72 7f[ 	]*vcvttpd2qq xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a b2 00 08 00 00[ 	]*vcvttpd2qq xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a 72 80[ 	]*vcvttpd2qq xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 7a b2 f0 f7 ff ff[ 	]*vcvttpd2qq xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a 72 7f[ 	]*vcvttpd2qq xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a b2 00 04 00 00[ 	]*vcvttpd2qq xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a 72 80[ 	]*vcvttpd2qq xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a f5[ 	]*vcvttpd2qq ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 7a f5[ 	]*vcvttpd2qq ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a 31[ 	]*vcvttpd2qq ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvttpd2qq ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a 30[ 	]*vcvttpd2qq ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a 72 7f[ 	]*vcvttpd2qq ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a b2 00 10 00 00[ 	]*vcvttpd2qq ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a 72 80[ 	]*vcvttpd2qq ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 7a b2 e0 ef ff ff[ 	]*vcvttpd2qq ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a 72 7f[ 	]*vcvttpd2qq ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a b2 00 04 00 00[ 	]*vcvttpd2qq ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a 72 80[ 	]*vcvttpd2qq ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 f5[ 	]*vcvttpd2uqq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 8f 78 f5[ 	]*vcvttpd2uqq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 31[ 	]*vcvttpd2uqq xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 b4 f4 c0 1d fe ff[ 	]*vcvttpd2uqq xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 30[ 	]*vcvttpd2uqq xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 72 7f[ 	]*vcvttpd2uqq xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 b2 00 08 00 00[ 	]*vcvttpd2uqq xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 72 80[ 	]*vcvttpd2uqq xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 0f 78 b2 f0 f7 ff ff[ 	]*vcvttpd2uqq xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 72 7f[ 	]*vcvttpd2uqq xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 b2 00 04 00 00[ 	]*vcvttpd2uqq xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 72 80[ 	]*vcvttpd2uqq xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 1f 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 f5[ 	]*vcvttpd2uqq ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd af 78 f5[ 	]*vcvttpd2uqq ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 31[ 	]*vcvttpd2uqq ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 b4 f4 c0 1d fe ff[ 	]*vcvttpd2uqq ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 30[ 	]*vcvttpd2uqq ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 72 7f[ 	]*vcvttpd2uqq ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 b2 00 10 00 00[ 	]*vcvttpd2uqq ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 72 80[ 	]*vcvttpd2uqq ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 2f 78 b2 e0 ef ff ff[ 	]*vcvttpd2uqq ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 72 7f[ 	]*vcvttpd2uqq ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 b2 00 04 00 00[ 	]*vcvttpd2uqq ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 72 80[ 	]*vcvttpd2uqq ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 3f 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a f5[ 	]*vcvttps2qq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 7a f5[ 	]*vcvttps2qq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a 31[ 	]*vcvttps2qq xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a b4 f4 c0 1d fe ff[ 	]*vcvttps2qq xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a 30[ 	]*vcvttps2qq xmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a 72 7f[ 	]*vcvttps2qq xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a b2 00 04 00 00[ 	]*vcvttps2qq xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a 72 80[ 	]*vcvttps2qq xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 7a b2 f8 fb ff ff[ 	]*vcvttps2qq xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a 72 7f[ 	]*vcvttps2qq xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a b2 00 02 00 00[ 	]*vcvttps2qq xmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a 72 80[ 	]*vcvttps2qq xmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a b2 fc fd ff ff[ 	]*vcvttps2qq xmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a 72 7f[ 	]*vcvttps2qq xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a f5[ 	]*vcvttps2qq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 7a f5[ 	]*vcvttps2qq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a 31[ 	]*vcvttps2qq ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a b4 f4 c0 1d fe ff[ 	]*vcvttps2qq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a 30[ 	]*vcvttps2qq ymm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a 72 7f[ 	]*vcvttps2qq ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a b2 00 08 00 00[ 	]*vcvttps2qq ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a 72 80[ 	]*vcvttps2qq ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 7a b2 f0 f7 ff ff[ 	]*vcvttps2qq ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a 72 7f[ 	]*vcvttps2qq ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a b2 00 02 00 00[ 	]*vcvttps2qq ymm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a 72 80[ 	]*vcvttps2qq ymm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a b2 fc fd ff ff[ 	]*vcvttps2qq ymm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7a 72 7f[ 	]*vcvttps2qq ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 f5[ 	]*vcvttps2uqq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 8f 78 f5[ 	]*vcvttps2uqq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 31[ 	]*vcvttps2uqq xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 b4 f4 c0 1d fe ff[ 	]*vcvttps2uqq xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 30[ 	]*vcvttps2uqq xmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 72 7f[ 	]*vcvttps2uqq xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 b2 00 04 00 00[ 	]*vcvttps2uqq xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 72 80[ 	]*vcvttps2uqq xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 0f 78 b2 f8 fb ff ff[ 	]*vcvttps2uqq xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 72 7f[ 	]*vcvttps2uqq xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 b2 00 02 00 00[ 	]*vcvttps2uqq xmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 72 80[ 	]*vcvttps2uqq xmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 b2 fc fd ff ff[ 	]*vcvttps2uqq xmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 72 7f[ 	]*vcvttps2uqq xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 f5[ 	]*vcvttps2uqq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d af 78 f5[ 	]*vcvttps2uqq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 31[ 	]*vcvttps2uqq ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 b4 f4 c0 1d fe ff[ 	]*vcvttps2uqq ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 30[ 	]*vcvttps2uqq ymm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 72 7f[ 	]*vcvttps2uqq ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 b2 00 08 00 00[ 	]*vcvttps2uqq ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 72 80[ 	]*vcvttps2uqq ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 2f 78 b2 f0 f7 ff ff[ 	]*vcvttps2uqq ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 72 7f[ 	]*vcvttps2uqq ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 b2 00 02 00 00[ 	]*vcvttps2uqq ymm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 72 80[ 	]*vcvttps2uqq ymm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 b2 fc fd ff ff[ 	]*vcvttps2uqq ymm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 78 72 7f[ 	]*vcvttps2uqq ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 39 ee[ 	]*vpmovd2m k5,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 39 ee[ 	]*vpmovd2m k5,ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 39 ee[ 	]*vpmovq2m k5,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 39 ee[ 	]*vpmovq2m k5,ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 38 f5[ 	]*vpmovm2d xmm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 38 f5[ 	]*vpmovm2d ymm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 38 f5[ 	]*vpmovm2q xmm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 38 f5[ 	]*vpmovm2q ymm6,k5
#pass
