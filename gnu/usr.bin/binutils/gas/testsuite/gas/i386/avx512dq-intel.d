#as:
#objdump: -dw -Mintel
#name: i386 AVX512DQ insns (Intel disassembly)
#source: avx512dq.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b 31[ 	]*vbroadcastf32x8 zmm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 1b 31[ 	]*vbroadcastf32x8 zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 1b 31[ 	]*vbroadcastf32x8 zmm6\{k7\}\{z\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b b4 f4 c0 1d fe ff[ 	]*vbroadcastf32x8 zmm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b 72 7f[ 	]*vbroadcastf32x8 zmm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b b2 00 10 00 00[ 	]*vbroadcastf32x8 zmm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b 72 80[ 	]*vbroadcastf32x8 zmm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b b2 e0 ef ff ff[ 	]*vbroadcastf32x8 zmm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a 31[ 	]*vbroadcastf64x2 zmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 1a 31[ 	]*vbroadcastf64x2 zmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 1a 31[ 	]*vbroadcastf64x2 zmm6\{k7\}\{z\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a b4 f4 c0 1d fe ff[ 	]*vbroadcastf64x2 zmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a 72 7f[ 	]*vbroadcastf64x2 zmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 zmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a 72 80[ 	]*vbroadcastf64x2 zmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 zmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b 31[ 	]*vbroadcasti32x8 zmm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 5b 31[ 	]*vbroadcasti32x8 zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 5b 31[ 	]*vbroadcasti32x8 zmm6\{k7\}\{z\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x8 zmm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b 72 7f[ 	]*vbroadcasti32x8 zmm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b b2 00 10 00 00[ 	]*vbroadcasti32x8 zmm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b 72 80[ 	]*vbroadcasti32x8 zmm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b b2 e0 ef ff ff[ 	]*vbroadcasti32x8 zmm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a 31[ 	]*vbroadcasti64x2 zmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 5a 31[ 	]*vbroadcasti64x2 zmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 5a 31[ 	]*vbroadcasti64x2 zmm6\{k7\}\{z\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a b4 f4 c0 1d fe ff[ 	]*vbroadcasti64x2 zmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a 72 7f[ 	]*vbroadcasti64x2 zmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 zmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a 72 80[ 	]*vbroadcasti64x2 zmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 zmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 f7[ 	]*vbroadcastf32x2 zmm6,xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 19 f7[ 	]*vbroadcastf32x2 zmm6\{k7\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 19 f7[ 	]*vbroadcastf32x2 zmm6\{k7\}\{z\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 31[ 	]*vbroadcastf32x2 zmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 b4 f4 c0 1d fe ff[ 	]*vbroadcastf32x2 zmm6,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 72 7f[ 	]*vbroadcastf32x2 zmm6,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 zmm6,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 72 80[ 	]*vbroadcastf32x2 zmm6,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 zmm6,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b f5[ 	]*vcvtpd2qq zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 7b f5[ 	]*vcvtpd2qq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 7b f5[ 	]*vcvtpd2qq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 7b f5[ 	]*vcvtpd2qq zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b f5[ 	]*vcvtpd2qq zmm6,zmm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 38 7b f5[ 	]*vcvtpd2qq zmm6,zmm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 78 7b f5[ 	]*vcvtpd2qq zmm6,zmm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b 31[ 	]*vcvtpd2qq zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b b4 f4 c0 1d fe ff[ 	]*vcvtpd2qq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b 30[ 	]*vcvtpd2qq zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b 72 7f[ 	]*vcvtpd2qq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b b2 00 20 00 00[ 	]*vcvtpd2qq zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b 72 80[ 	]*vcvtpd2qq zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b b2 c0 df ff ff[ 	]*vcvtpd2qq zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b 72 7f[ 	]*vcvtpd2qq zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b b2 00 04 00 00[ 	]*vcvtpd2qq zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b 72 80[ 	]*vcvtpd2qq zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 f5[ 	]*vcvtpd2uqq zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 79 f5[ 	]*vcvtpd2uqq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 79 f5[ 	]*vcvtpd2uqq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 79 f5[ 	]*vcvtpd2uqq zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 f5[ 	]*vcvtpd2uqq zmm6,zmm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 38 79 f5[ 	]*vcvtpd2uqq zmm6,zmm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 78 79 f5[ 	]*vcvtpd2uqq zmm6,zmm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 31[ 	]*vcvtpd2uqq zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 b4 f4 c0 1d fe ff[ 	]*vcvtpd2uqq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 30[ 	]*vcvtpd2uqq zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 72 7f[ 	]*vcvtpd2uqq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 b2 00 20 00 00[ 	]*vcvtpd2uqq zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 72 80[ 	]*vcvtpd2uqq zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 b2 c0 df ff ff[ 	]*vcvtpd2uqq zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 72 7f[ 	]*vcvtpd2uqq zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 b2 00 04 00 00[ 	]*vcvtpd2uqq zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 72 80[ 	]*vcvtpd2uqq zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b f5[ 	]*vcvtps2qq zmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 7b f5[ 	]*vcvtps2qq zmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b f5[ 	]*vcvtps2qq zmm6\{k7\},ymm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b f5[ 	]*vcvtps2qq zmm6\{k7\},ymm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b f5[ 	]*vcvtps2qq zmm6\{k7\},ymm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 7f 7b f5[ 	]*vcvtps2qq zmm6\{k7\},ymm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b 31[ 	]*vcvtps2qq zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b b4 f4 c0 1d fe ff[ 	]*vcvtps2qq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b 30[ 	]*vcvtps2qq zmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b 72 7f[ 	]*vcvtps2qq zmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b b2 00 10 00 00[ 	]*vcvtps2qq zmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b 72 80[ 	]*vcvtps2qq zmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b b2 e0 ef ff ff[ 	]*vcvtps2qq zmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b 72 7f[ 	]*vcvtps2qq zmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b b2 00 02 00 00[ 	]*vcvtps2qq zmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b 72 80[ 	]*vcvtps2qq zmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b b2 fc fd ff ff[ 	]*vcvtps2qq zmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 f5[ 	]*vcvtps2uqq zmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 79 f5[ 	]*vcvtps2uqq zmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 f5[ 	]*vcvtps2uqq zmm6\{k7\},ymm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 f5[ 	]*vcvtps2uqq zmm6\{k7\},ymm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 f5[ 	]*vcvtps2uqq zmm6\{k7\},ymm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 7f 79 f5[ 	]*vcvtps2uqq zmm6\{k7\},ymm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 31[ 	]*vcvtps2uqq zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 b4 f4 c0 1d fe ff[ 	]*vcvtps2uqq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 30[ 	]*vcvtps2uqq zmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 72 7f[ 	]*vcvtps2uqq zmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 b2 00 10 00 00[ 	]*vcvtps2uqq zmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 72 80[ 	]*vcvtps2uqq zmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 b2 e0 ef ff ff[ 	]*vcvtps2uqq zmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 72 7f[ 	]*vcvtps2uqq zmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 b2 00 02 00 00[ 	]*vcvtps2uqq zmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 72 80[ 	]*vcvtps2uqq zmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 b2 fc fd ff ff[ 	]*vcvtps2uqq zmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 f5[ 	]*vcvtqq2pd zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 4f e6 f5[ 	]*vcvtqq2pd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe cf e6 f5[ 	]*vcvtqq2pd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 18 e6 f5[ 	]*vcvtqq2pd zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 f5[ 	]*vcvtqq2pd zmm6,zmm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 38 e6 f5[ 	]*vcvtqq2pd zmm6,zmm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 78 e6 f5[ 	]*vcvtqq2pd zmm6,zmm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 31[ 	]*vcvtqq2pd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 b4 f4 c0 1d fe ff[ 	]*vcvtqq2pd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 30[ 	]*vcvtqq2pd zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 72 7f[ 	]*vcvtqq2pd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 b2 00 20 00 00[ 	]*vcvtqq2pd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 72 80[ 	]*vcvtqq2pd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 b2 c0 df ff ff[ 	]*vcvtqq2pd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 72 7f[ 	]*vcvtqq2pd zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 b2 00 04 00 00[ 	]*vcvtqq2pd zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 72 80[ 	]*vcvtqq2pd zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b f5[ 	]*vcvtqq2ps ymm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc cf 5b f5[ 	]*vcvtqq2ps ymm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b f5[ 	]*vcvtqq2ps ymm6\{k7\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b f5[ 	]*vcvtqq2ps ymm6\{k7\},zmm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b f5[ 	]*vcvtqq2ps ymm6\{k7\},zmm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 7f 5b f5[ 	]*vcvtqq2ps ymm6\{k7\},zmm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b 31[ 	]*vcvtqq2ps ymm6\{k7\},ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b b4 f4 c0 1d fe ff[ 	]*vcvtqq2ps ymm6\{k7\},ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b 30[ 	]*vcvtqq2ps ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b 72 7f[ 	]*vcvtqq2ps ymm6\{k7\},ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b b2 00 20 00 00[ 	]*vcvtqq2ps ymm6\{k7\},ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b 72 80[ 	]*vcvtqq2ps ymm6\{k7\},ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b b2 c0 df ff ff[ 	]*vcvtqq2ps ymm6\{k7\},ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b 72 7f[ 	]*vcvtqq2ps ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b b2 00 04 00 00[ 	]*vcvtqq2ps ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b 72 80[ 	]*vcvtqq2ps ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a f5[ 	]*vcvtuqq2pd zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 4f 7a f5[ 	]*vcvtuqq2pd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe cf 7a f5[ 	]*vcvtuqq2pd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 18 7a f5[ 	]*vcvtuqq2pd zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a f5[ 	]*vcvtuqq2pd zmm6,zmm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 38 7a f5[ 	]*vcvtuqq2pd zmm6,zmm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 78 7a f5[ 	]*vcvtuqq2pd zmm6,zmm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a 31[ 	]*vcvtuqq2pd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2pd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a 30[ 	]*vcvtuqq2pd zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a 72 7f[ 	]*vcvtuqq2pd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a b2 00 20 00 00[ 	]*vcvtuqq2pd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a 72 80[ 	]*vcvtuqq2pd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a b2 c0 df ff ff[ 	]*vcvtuqq2pd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a 72 7f[ 	]*vcvtuqq2pd zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a b2 00 04 00 00[ 	]*vcvtuqq2pd zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a 72 80[ 	]*vcvtuqq2pd zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a f5[ 	]*vcvtuqq2ps ymm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff cf 7a f5[ 	]*vcvtuqq2ps ymm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a f5[ 	]*vcvtuqq2ps ymm6\{k7\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a f5[ 	]*vcvtuqq2ps ymm6\{k7\},zmm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a f5[ 	]*vcvtuqq2ps ymm6\{k7\},zmm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 7f 7a f5[ 	]*vcvtuqq2ps ymm6\{k7\},zmm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a 31[ 	]*vcvtuqq2ps ymm6\{k7\},ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2ps ymm6\{k7\},ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a 30[ 	]*vcvtuqq2ps ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a 72 7f[ 	]*vcvtuqq2ps ymm6\{k7\},ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a b2 00 20 00 00[ 	]*vcvtuqq2ps ymm6\{k7\},ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a 72 80[ 	]*vcvtuqq2ps ymm6\{k7\},ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a b2 c0 df ff ff[ 	]*vcvtuqq2ps ymm6\{k7\},ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a 72 7f[ 	]*vcvtuqq2ps ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a b2 00 04 00 00[ 	]*vcvtuqq2ps ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a 72 80[ 	]*vcvtuqq2ps ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 19 ee ab[ 	]*vextractf64x2 xmm6\{k7\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd cf 19 ee ab[ 	]*vextractf64x2 xmm6\{k7\}\{z\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 19 ee 7b[ 	]*vextractf64x2 xmm6\{k7\},zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 1b ee ab[ 	]*vextractf32x8 ymm6\{k7\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d cf 1b ee ab[ 	]*vextractf32x8 ymm6\{k7\}\{z\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 1b ee 7b[ 	]*vextractf32x8 ymm6\{k7\},zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 39 ee ab[ 	]*vextracti64x2 xmm6\{k7\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd cf 39 ee ab[ 	]*vextracti64x2 xmm6\{k7\}\{z\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 39 ee 7b[ 	]*vextracti64x2 xmm6\{k7\},zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 3b ee ab[ 	]*vextracti32x8 ymm6\{k7\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d cf 3b ee ab[ 	]*vextracti32x8 ymm6\{k7\}\{z\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 3b ee 7b[ 	]*vextracti32x8 ymm6\{k7\},zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 ee ab[ 	]*vfpclasspd k5,zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 66 ee ab[ 	]*vfpclasspd k5\{k7\},zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 ee 7b[ 	]*vfpclasspd k5,zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 29 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 28 7b[ 	]*vfpclasspd k5,QWORD BCST \[eax\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 7f 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa 00 20 00 00 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[edx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 80 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[edx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa c0 df ff ff 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[edx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 7f 7b[ 	]*vfpclasspd k5,QWORD BCST \[edx\+0x3f8\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa 00 04 00 00 7b[ 	]*vfpclasspd k5,QWORD BCST \[edx\+0x400\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 80 7b[ 	]*vfpclasspd k5,QWORD BCST \[edx-0x400\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd k5,QWORD BCST \[edx-0x408\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 ee ab[ 	]*vfpclassps k5,zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 66 ee ab[ 	]*vfpclassps k5\{k7\},zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 ee 7b[ 	]*vfpclassps k5,zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 29 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 28 7b[ 	]*vfpclassps k5,DWORD BCST \[eax\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 7f 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa 00 20 00 00 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[edx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 80 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[edx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa c0 df ff ff 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[edx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 7f 7b[ 	]*vfpclassps k5,DWORD BCST \[edx\+0x1fc\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa 00 02 00 00 7b[ 	]*vfpclassps k5,DWORD BCST \[edx\+0x200\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 80 7b[ 	]*vfpclassps k5,DWORD BCST \[edx-0x200\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa fc fd ff ff 7b[ 	]*vfpclassps k5,DWORD BCST \[edx-0x204\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 ee ab[ 	]*vfpclasssd k5\{k7\},xmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 ee 7b[ 	]*vfpclasssd k5\{k7\},xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 29 7b[ 	]*vfpclasssd k5\{k7\},QWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 ac f4 c0 1d fe ff 7b[ 	]*vfpclasssd k5\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 6a 7f 7b[ 	]*vfpclasssd k5\{k7\},QWORD PTR \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 aa 00 04 00 00 7b[ 	]*vfpclasssd k5\{k7\},QWORD PTR \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 6a 80 7b[ 	]*vfpclasssd k5\{k7\},QWORD PTR \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 aa f8 fb ff ff 7b[ 	]*vfpclasssd k5\{k7\},QWORD PTR \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 ee ab[ 	]*vfpclassss k5\{k7\},xmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 ee 7b[ 	]*vfpclassss k5\{k7\},xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 29 7b[ 	]*vfpclassss k5\{k7\},DWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 ac f4 c0 1d fe ff 7b[ 	]*vfpclassss k5\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 6a 7f 7b[ 	]*vfpclassss k5\{k7\},DWORD PTR \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 aa 00 02 00 00 7b[ 	]*vfpclassss k5\{k7\},DWORD PTR \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 6a 80 7b[ 	]*vfpclassss k5\{k7\},DWORD PTR \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 aa fc fd ff ff 7b[ 	]*vfpclassss k5\{k7\},DWORD PTR \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 f4 ab[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 18 f4 ab[ 	]*vinsertf64x2 zmm6\{k7\}\{z\},zmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 f4 7b[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 31 7b[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 b4 f4 c0 1d fe ff 7b[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 72 7f 7b[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 72 80 7b[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a f4 ab[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 1a f4 ab[ 	]*vinsertf32x8 zmm6\{k7\}\{z\},zmm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a f4 7b[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a 31 7b[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a b4 f4 c0 1d fe ff 7b[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a 72 7f 7b[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a b2 00 10 00 00 7b[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a 72 80 7b[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a b2 e0 ef ff ff 7b[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 f4 ab[ 	]*vinserti64x2 zmm6\{k7\},zmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 38 f4 ab[ 	]*vinserti64x2 zmm6\{k7\}\{z\},zmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 f4 7b[ 	]*vinserti64x2 zmm6\{k7\},zmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 31 7b[ 	]*vinserti64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 b4 f4 c0 1d fe ff 7b[ 	]*vinserti64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 72 7f 7b[ 	]*vinserti64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 72 80 7b[ 	]*vinserti64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a f4 ab[ 	]*vinserti32x8 zmm6\{k7\},zmm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 3a f4 ab[ 	]*vinserti32x8 zmm6\{k7\}\{z\},zmm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a f4 7b[ 	]*vinserti32x8 zmm6\{k7\},zmm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a 31 7b[ 	]*vinserti32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a b4 f4 c0 1d fe ff 7b[ 	]*vinserti32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a 72 7f 7b[ 	]*vinserti32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a b2 00 10 00 00 7b[ 	]*vinserti32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a 72 80 7b[ 	]*vinserti32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a b2 e0 ef ff ff 7b[ 	]*vinserti32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 f7[ 	]*vbroadcasti32x2 zmm6,xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 59 f7[ 	]*vbroadcasti32x2 zmm6\{k7\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 59 f7[ 	]*vbroadcasti32x2 zmm6\{k7\}\{z\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 31[ 	]*vbroadcasti32x2 zmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x2 zmm6,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 72 7f[ 	]*vbroadcasti32x2 zmm6,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 zmm6,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 72 80[ 	]*vbroadcasti32x2 zmm6,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 zmm6,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 f4[ 	]*vpmullq zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 40 f4[ 	]*vpmullq zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 40 f4[ 	]*vpmullq zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 31[ 	]*vpmullq zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 b4 f4 c0 1d fe ff[ 	]*vpmullq zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 30[ 	]*vpmullq zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 72 7f[ 	]*vpmullq zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 b2 00 20 00 00[ 	]*vpmullq zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 72 80[ 	]*vpmullq zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 b2 c0 df ff ff[ 	]*vpmullq zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 72 7f[ 	]*vpmullq zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 b2 00 04 00 00[ 	]*vpmullq zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 72 80[ 	]*vpmullq zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 b2 f8 fb ff ff[ 	]*vpmullq zmm6,zmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 f4 ab[ 	]*vrangepd zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 50 f4 ab[ 	]*vrangepd zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 50 f4 ab[ 	]*vrangepd zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 18 50 f4 ab[ 	]*vrangepd zmm6,zmm5,zmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 f4 7b[ 	]*vrangepd zmm6,zmm5,zmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 18 50 f4 7b[ 	]*vrangepd zmm6,zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 31 7b[ 	]*vrangepd zmm6,zmm5,ZMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangepd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 30 7b[ 	]*vrangepd zmm6,zmm5,QWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 72 7f 7b[ 	]*vrangepd zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 b2 00 20 00 00 7b[ 	]*vrangepd zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 72 80 7b[ 	]*vrangepd zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 b2 c0 df ff ff 7b[ 	]*vrangepd zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 72 7f 7b[ 	]*vrangepd zmm6,zmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 b2 00 04 00 00 7b[ 	]*vrangepd zmm6,zmm5,QWORD BCST \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 72 80 7b[ 	]*vrangepd zmm6,zmm5,QWORD BCST \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 b2 f8 fb ff ff 7b[ 	]*vrangepd zmm6,zmm5,QWORD BCST \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 f4 ab[ 	]*vrangeps zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 50 f4 ab[ 	]*vrangeps zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 50 f4 ab[ 	]*vrangeps zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 18 50 f4 ab[ 	]*vrangeps zmm6,zmm5,zmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 f4 7b[ 	]*vrangeps zmm6,zmm5,zmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 18 50 f4 7b[ 	]*vrangeps zmm6,zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 31 7b[ 	]*vrangeps zmm6,zmm5,ZMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangeps zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 30 7b[ 	]*vrangeps zmm6,zmm5,DWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 72 7f 7b[ 	]*vrangeps zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 b2 00 20 00 00 7b[ 	]*vrangeps zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 72 80 7b[ 	]*vrangeps zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 b2 c0 df ff ff 7b[ 	]*vrangeps zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 72 7f 7b[ 	]*vrangeps zmm6,zmm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 b2 00 02 00 00 7b[ 	]*vrangeps zmm6,zmm5,DWORD BCST \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 72 80 7b[ 	]*vrangeps zmm6,zmm5,DWORD BCST \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 b2 fc fd ff ff 7b[ 	]*vrangeps zmm6,zmm5,DWORD BCST \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 f4 ab[ 	]*vrangesd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 51 f4 ab[ 	]*vrangesd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 51 f4 ab[ 	]*vrangesd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 f4 7b[ 	]*vrangesd xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 51 f4 7b[ 	]*vrangesd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 31 7b[ 	]*vrangesd xmm6\{k7\},xmm5,QWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 b4 f4 c0 1d fe ff 7b[ 	]*vrangesd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 72 7f 7b[ 	]*vrangesd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 b2 00 04 00 00 7b[ 	]*vrangesd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 72 80 7b[ 	]*vrangesd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 b2 f8 fb ff ff 7b[ 	]*vrangesd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 f4 ab[ 	]*vrangess xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 51 f4 ab[ 	]*vrangess xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 51 f4 ab[ 	]*vrangess xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 f4 7b[ 	]*vrangess xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 51 f4 7b[ 	]*vrangess xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 31 7b[ 	]*vrangess xmm6\{k7\},xmm5,DWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 b4 f4 c0 1d fe ff 7b[ 	]*vrangess xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 72 7f 7b[ 	]*vrangess xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 b2 00 02 00 00 7b[ 	]*vrangess xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 72 80 7b[ 	]*vrangess xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 b2 fc fd ff ff 7b[ 	]*vrangess xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 f4[ 	]*vandpd zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 54 f4[ 	]*vandpd zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 54 f4[ 	]*vandpd zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 31[ 	]*vandpd zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 b4 f4 c0 1d fe ff[ 	]*vandpd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 30[ 	]*vandpd zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 72 7f[ 	]*vandpd zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 b2 00 20 00 00[ 	]*vandpd zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 72 80[ 	]*vandpd zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 b2 c0 df ff ff[ 	]*vandpd zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 72 7f[ 	]*vandpd zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 b2 00 04 00 00[ 	]*vandpd zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 72 80[ 	]*vandpd zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 b2 f8 fb ff ff[ 	]*vandpd zmm6,zmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 f4[ 	]*vandps zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 54 f4[ 	]*vandps zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 54 f4[ 	]*vandps zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 31[ 	]*vandps zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 b4 f4 c0 1d fe ff[ 	]*vandps zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 30[ 	]*vandps zmm6,zmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 72 7f[ 	]*vandps zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 b2 00 20 00 00[ 	]*vandps zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 72 80[ 	]*vandps zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 b2 c0 df ff ff[ 	]*vandps zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 72 7f[ 	]*vandps zmm6,zmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 b2 00 02 00 00[ 	]*vandps zmm6,zmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 72 80[ 	]*vandps zmm6,zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 b2 fc fd ff ff[ 	]*vandps zmm6,zmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 f4[ 	]*vandnpd zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 55 f4[ 	]*vandnpd zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 55 f4[ 	]*vandnpd zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 31[ 	]*vandnpd zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 b4 f4 c0 1d fe ff[ 	]*vandnpd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 30[ 	]*vandnpd zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 72 7f[ 	]*vandnpd zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 b2 00 20 00 00[ 	]*vandnpd zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 72 80[ 	]*vandnpd zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 b2 c0 df ff ff[ 	]*vandnpd zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 72 7f[ 	]*vandnpd zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 b2 00 04 00 00[ 	]*vandnpd zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 72 80[ 	]*vandnpd zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 b2 f8 fb ff ff[ 	]*vandnpd zmm6,zmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 f4[ 	]*vandnps zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 55 f4[ 	]*vandnps zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 55 f4[ 	]*vandnps zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 31[ 	]*vandnps zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 b4 f4 c0 1d fe ff[ 	]*vandnps zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 30[ 	]*vandnps zmm6,zmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 72 7f[ 	]*vandnps zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 b2 00 20 00 00[ 	]*vandnps zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 72 80[ 	]*vandnps zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 b2 c0 df ff ff[ 	]*vandnps zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 72 7f[ 	]*vandnps zmm6,zmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 b2 00 02 00 00[ 	]*vandnps zmm6,zmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 72 80[ 	]*vandnps zmm6,zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 b2 fc fd ff ff[ 	]*vandnps zmm6,zmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 f4[ 	]*vorpd  zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 56 f4[ 	]*vorpd  zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 56 f4[ 	]*vorpd  zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 31[ 	]*vorpd  zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 b4 f4 c0 1d fe ff[ 	]*vorpd  zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 30[ 	]*vorpd  zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 72 7f[ 	]*vorpd  zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 b2 00 20 00 00[ 	]*vorpd  zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 72 80[ 	]*vorpd  zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 b2 c0 df ff ff[ 	]*vorpd  zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 72 7f[ 	]*vorpd  zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 b2 00 04 00 00[ 	]*vorpd  zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 72 80[ 	]*vorpd  zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 b2 f8 fb ff ff[ 	]*vorpd  zmm6,zmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 f4[ 	]*vorps  zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 56 f4[ 	]*vorps  zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 56 f4[ 	]*vorps  zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 31[ 	]*vorps  zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 b4 f4 c0 1d fe ff[ 	]*vorps  zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 30[ 	]*vorps  zmm6,zmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 72 7f[ 	]*vorps  zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 b2 00 20 00 00[ 	]*vorps  zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 72 80[ 	]*vorps  zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 b2 c0 df ff ff[ 	]*vorps  zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 72 7f[ 	]*vorps  zmm6,zmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 b2 00 02 00 00[ 	]*vorps  zmm6,zmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 72 80[ 	]*vorps  zmm6,zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 b2 fc fd ff ff[ 	]*vorps  zmm6,zmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 f4[ 	]*vxorpd zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 57 f4[ 	]*vxorpd zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 57 f4[ 	]*vxorpd zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 31[ 	]*vxorpd zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 b4 f4 c0 1d fe ff[ 	]*vxorpd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 30[ 	]*vxorpd zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 72 7f[ 	]*vxorpd zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 b2 00 20 00 00[ 	]*vxorpd zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 72 80[ 	]*vxorpd zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 b2 c0 df ff ff[ 	]*vxorpd zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 72 7f[ 	]*vxorpd zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 b2 00 04 00 00[ 	]*vxorpd zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 72 80[ 	]*vxorpd zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 b2 f8 fb ff ff[ 	]*vxorpd zmm6,zmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 f4[ 	]*vxorps zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 57 f4[ 	]*vxorps zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 57 f4[ 	]*vxorps zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 31[ 	]*vxorps zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 b4 f4 c0 1d fe ff[ 	]*vxorps zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 30[ 	]*vxorps zmm6,zmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 72 7f[ 	]*vxorps zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 b2 00 20 00 00[ 	]*vxorps zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 72 80[ 	]*vxorps zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 b2 c0 df ff ff[ 	]*vxorps zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 72 7f[ 	]*vxorps zmm6,zmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 b2 00 02 00 00[ 	]*vxorps zmm6,zmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 72 80[ 	]*vxorps zmm6,zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 b2 fc fd ff ff[ 	]*vxorps zmm6,zmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 f5 ab[ 	]*vreducepd zmm6,zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 56 f5 ab[ 	]*vreducepd zmm6\{k7\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd cf 56 f5 ab[ 	]*vreducepd zmm6\{k7\}\{z\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 56 f5 ab[ 	]*vreducepd zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 f5 7b[ 	]*vreducepd zmm6,zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 56 f5 7b[ 	]*vreducepd zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 31 7b[ 	]*vreducepd zmm6,ZMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 b4 f4 c0 1d fe ff 7b[ 	]*vreducepd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 30 7b[ 	]*vreducepd zmm6,QWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 72 7f 7b[ 	]*vreducepd zmm6,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 b2 00 20 00 00 7b[ 	]*vreducepd zmm6,ZMMWORD PTR \[edx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 72 80 7b[ 	]*vreducepd zmm6,ZMMWORD PTR \[edx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 b2 c0 df ff ff 7b[ 	]*vreducepd zmm6,ZMMWORD PTR \[edx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 72 7f 7b[ 	]*vreducepd zmm6,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 b2 00 04 00 00 7b[ 	]*vreducepd zmm6,QWORD BCST \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 72 80 7b[ 	]*vreducepd zmm6,QWORD BCST \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 b2 f8 fb ff ff 7b[ 	]*vreducepd zmm6,QWORD BCST \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 f5 ab[ 	]*vreduceps zmm6,zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 56 f5 ab[ 	]*vreduceps zmm6\{k7\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d cf 56 f5 ab[ 	]*vreduceps zmm6\{k7\}\{z\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 56 f5 ab[ 	]*vreduceps zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 f5 7b[ 	]*vreduceps zmm6,zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 56 f5 7b[ 	]*vreduceps zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 31 7b[ 	]*vreduceps zmm6,ZMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 b4 f4 c0 1d fe ff 7b[ 	]*vreduceps zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 30 7b[ 	]*vreduceps zmm6,DWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 72 7f 7b[ 	]*vreduceps zmm6,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 b2 00 20 00 00 7b[ 	]*vreduceps zmm6,ZMMWORD PTR \[edx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 72 80 7b[ 	]*vreduceps zmm6,ZMMWORD PTR \[edx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 b2 c0 df ff ff 7b[ 	]*vreduceps zmm6,ZMMWORD PTR \[edx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 72 7f 7b[ 	]*vreduceps zmm6,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 b2 00 02 00 00 7b[ 	]*vreduceps zmm6,DWORD BCST \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 72 80 7b[ 	]*vreduceps zmm6,DWORD BCST \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 b2 fc fd ff ff 7b[ 	]*vreduceps zmm6,DWORD BCST \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 f4 ab[ 	]*vreducesd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 57 f4 ab[ 	]*vreducesd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 57 f4 ab[ 	]*vreducesd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 f4 7b[ 	]*vreducesd xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 57 f4 7b[ 	]*vreducesd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 31 7b[ 	]*vreducesd xmm6\{k7\},xmm5,QWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 b4 f4 c0 1d fe ff 7b[ 	]*vreducesd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 72 7f 7b[ 	]*vreducesd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 b2 00 04 00 00 7b[ 	]*vreducesd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 72 80 7b[ 	]*vreducesd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 b2 f8 fb ff ff 7b[ 	]*vreducesd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 f4 ab[ 	]*vreducess xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 57 f4 ab[ 	]*vreducess xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 57 f4 ab[ 	]*vreducess xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 f4 7b[ 	]*vreducess xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 57 f4 7b[ 	]*vreducess xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 31 7b[ 	]*vreducess xmm6\{k7\},xmm5,DWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 b4 f4 c0 1d fe ff 7b[ 	]*vreducess xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 72 7f 7b[ 	]*vreducess xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 b2 00 02 00 00 7b[ 	]*vreducess xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 72 80 7b[ 	]*vreducess xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 b2 fc fd ff ff 7b[ 	]*vreducess xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c5 cd 41 ef[ 	]*kandb  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 cd 42 ef[ 	]*kandnb k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 cd 45 ef[ 	]*korb   k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 cd 46 ef[ 	]*kxnorb k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 cd 47 ef[ 	]*kxorb  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 f9 44 ee[ 	]*knotb  k5,k6
[ 	]*[a-f0-9]+:[ 	]*c5 f9 98 ee[ 	]*kortestb k5,k6
[ 	]*[a-f0-9]+:[ 	]*c5 f8 99 ee[ 	]*ktestw k5,k6
[ 	]*[a-f0-9]+:[ 	]*c5 f9 99 ee[ 	]*ktestb k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 30 ee ab[ 	]*kshiftrb k5,k6,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 30 ee 7b[ 	]*kshiftrb k5,k6,0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 32 ee ab[ 	]*kshiftlb k5,k6,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 32 ee 7b[ 	]*kshiftlb k5,k6,0x7b
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 ee[ 	]*kmovb  k5,k6
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 29[ 	]*kmovb  k5,BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 ac f4 c0 1d fe ff[ 	]*kmovb  k5,BYTE PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c5 f9 91 29[ 	]*kmovb  BYTE PTR \[ecx\],k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 91 ac f4 c0 1d fe ff[ 	]*kmovb  BYTE PTR \[esp\+esi\*8-0x1e240\],k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 e8[ 	]*kmovb  k5,eax
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 ed[ 	]*kmovb  k5,ebp
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 c5[ 	]*kmovb  eax,k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 ed[ 	]*kmovb  ebp,k5
[ 	]*[a-f0-9]+:[ 	]*c5 cc 4a ef[ 	]*kaddw  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 cd 4a ef[ 	]*kaddb  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 31 ab[ 	]*vextractf64x2 XMMWORD PTR \[ecx\],zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 19 31 ab[ 	]*vextractf64x2 XMMWORD PTR \[ecx\]\{k7\},zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 31 7b[ 	]*vextractf64x2 XMMWORD PTR \[ecx\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 b4 f4 c0 1d fe ff 7b[ 	]*vextractf64x2 XMMWORD PTR \[esp\+esi\*8-0x1e240\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 72 7f 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx\+0x7f0\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 b2 00 08 00 00 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx\+0x800\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 72 80 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx-0x800\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 b2 f0 f7 ff ff 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx-0x810\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 31 ab[ 	]*vextractf32x8 YMMWORD PTR \[ecx\],zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 1b 31 ab[ 	]*vextractf32x8 YMMWORD PTR \[ecx\]\{k7\},zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 31 7b[ 	]*vextractf32x8 YMMWORD PTR \[ecx\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b b4 f4 c0 1d fe ff 7b[ 	]*vextractf32x8 YMMWORD PTR \[esp\+esi\*8-0x1e240\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 72 7f 7b[ 	]*vextractf32x8 YMMWORD PTR \[edx\+0xfe0\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b b2 00 10 00 00 7b[ 	]*vextractf32x8 YMMWORD PTR \[edx\+0x1000\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 72 80 7b[ 	]*vextractf32x8 YMMWORD PTR \[edx-0x1000\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b b2 e0 ef ff ff 7b[ 	]*vextractf32x8 YMMWORD PTR \[edx-0x1020\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 31 ab[ 	]*vextracti64x2 XMMWORD PTR \[ecx\],zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 39 31 ab[ 	]*vextracti64x2 XMMWORD PTR \[ecx\]\{k7\},zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 31 7b[ 	]*vextracti64x2 XMMWORD PTR \[ecx\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 b4 f4 c0 1d fe ff 7b[ 	]*vextracti64x2 XMMWORD PTR \[esp\+esi\*8-0x1e240\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 72 7f 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx\+0x7f0\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 b2 00 08 00 00 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx\+0x800\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 72 80 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx-0x800\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 b2 f0 f7 ff ff 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx-0x810\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 31 ab[ 	]*vextracti32x8 YMMWORD PTR \[ecx\],zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 3b 31 ab[ 	]*vextracti32x8 YMMWORD PTR \[ecx\]\{k7\},zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 31 7b[ 	]*vextracti32x8 YMMWORD PTR \[ecx\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b b4 f4 c0 1d fe ff 7b[ 	]*vextracti32x8 YMMWORD PTR \[esp\+esi\*8-0x1e240\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 72 7f 7b[ 	]*vextracti32x8 YMMWORD PTR \[edx\+0xfe0\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b b2 00 10 00 00 7b[ 	]*vextracti32x8 YMMWORD PTR \[edx\+0x1000\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 72 80 7b[ 	]*vextracti32x8 YMMWORD PTR \[edx-0x1000\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b b2 e0 ef ff ff 7b[ 	]*vextracti32x8 YMMWORD PTR \[edx-0x1020\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a f5[ 	]*vcvttpd2qq zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 7a f5[ 	]*vcvttpd2qq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 7a f5[ 	]*vcvttpd2qq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 7a f5[ 	]*vcvttpd2qq zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a 31[ 	]*vcvttpd2qq zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a b4 f4 c0 1d fe ff[ 	]*vcvttpd2qq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a 30[ 	]*vcvttpd2qq zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a 72 7f[ 	]*vcvttpd2qq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a b2 00 20 00 00[ 	]*vcvttpd2qq zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a 72 80[ 	]*vcvttpd2qq zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a b2 c0 df ff ff[ 	]*vcvttpd2qq zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a 72 7f[ 	]*vcvttpd2qq zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a b2 00 04 00 00[ 	]*vcvttpd2qq zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a 72 80[ 	]*vcvttpd2qq zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 f5[ 	]*vcvttpd2uqq zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 78 f5[ 	]*vcvttpd2uqq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 78 f5[ 	]*vcvttpd2uqq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 78 f5[ 	]*vcvttpd2uqq zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 31[ 	]*vcvttpd2uqq zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 b4 f4 c0 1d fe ff[ 	]*vcvttpd2uqq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 30[ 	]*vcvttpd2uqq zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 72 7f[ 	]*vcvttpd2uqq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 b2 00 20 00 00[ 	]*vcvttpd2uqq zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 72 80[ 	]*vcvttpd2uqq zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 b2 c0 df ff ff[ 	]*vcvttpd2uqq zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 72 7f[ 	]*vcvttpd2uqq zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 b2 00 04 00 00[ 	]*vcvttpd2uqq zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 72 80[ 	]*vcvttpd2uqq zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a f5[ 	]*vcvttps2qq zmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 7a f5[ 	]*vcvttps2qq zmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a f5[ 	]*vcvttps2qq zmm6\{k7\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a 31[ 	]*vcvttps2qq zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a b4 f4 c0 1d fe ff[ 	]*vcvttps2qq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a 30[ 	]*vcvttps2qq zmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a 72 7f[ 	]*vcvttps2qq zmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a b2 00 10 00 00[ 	]*vcvttps2qq zmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a 72 80[ 	]*vcvttps2qq zmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a b2 e0 ef ff ff[ 	]*vcvttps2qq zmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a 72 7f[ 	]*vcvttps2qq zmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a b2 00 02 00 00[ 	]*vcvttps2qq zmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a 72 80[ 	]*vcvttps2qq zmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a b2 fc fd ff ff[ 	]*vcvttps2qq zmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 f5[ 	]*vcvttps2uqq zmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 78 f5[ 	]*vcvttps2uqq zmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 f5[ 	]*vcvttps2uqq zmm6\{k7\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 31[ 	]*vcvttps2uqq zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 b4 f4 c0 1d fe ff[ 	]*vcvttps2uqq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 30[ 	]*vcvttps2uqq zmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 72 7f[ 	]*vcvttps2uqq zmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 b2 00 10 00 00[ 	]*vcvttps2uqq zmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 72 80[ 	]*vcvttps2uqq zmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 b2 e0 ef ff ff[ 	]*vcvttps2uqq zmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 72 7f[ 	]*vcvttps2uqq zmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 b2 00 02 00 00[ 	]*vcvttps2uqq zmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 72 80[ 	]*vcvttps2uqq zmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 b2 fc fd ff ff[ 	]*vcvttps2uqq zmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 48 39 ee[ 	]*vpmovd2m k5,zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 48 39 ee[ 	]*vpmovq2m k5,zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 48 38 f5[ 	]*vpmovm2d zmm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 48 38 f5[ 	]*vpmovm2q zmm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b 31[ 	]*vbroadcastf32x8 zmm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 1b 31[ 	]*vbroadcastf32x8 zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 1b 31[ 	]*vbroadcastf32x8 zmm6\{k7\}\{z\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b b4 f4 c0 1d fe ff[ 	]*vbroadcastf32x8 zmm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b 72 7f[ 	]*vbroadcastf32x8 zmm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b b2 00 10 00 00[ 	]*vbroadcastf32x8 zmm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b 72 80[ 	]*vbroadcastf32x8 zmm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 1b b2 e0 ef ff ff[ 	]*vbroadcastf32x8 zmm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a 31[ 	]*vbroadcastf64x2 zmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 1a 31[ 	]*vbroadcastf64x2 zmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 1a 31[ 	]*vbroadcastf64x2 zmm6\{k7\}\{z\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a b4 f4 c0 1d fe ff[ 	]*vbroadcastf64x2 zmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a 72 7f[ 	]*vbroadcastf64x2 zmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 zmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a 72 80[ 	]*vbroadcastf64x2 zmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 zmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b 31[ 	]*vbroadcasti32x8 zmm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 5b 31[ 	]*vbroadcasti32x8 zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 5b 31[ 	]*vbroadcasti32x8 zmm6\{k7\}\{z\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x8 zmm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b 72 7f[ 	]*vbroadcasti32x8 zmm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b b2 00 10 00 00[ 	]*vbroadcasti32x8 zmm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b 72 80[ 	]*vbroadcasti32x8 zmm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 5b b2 e0 ef ff ff[ 	]*vbroadcasti32x8 zmm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a 31[ 	]*vbroadcasti64x2 zmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 4f 5a 31[ 	]*vbroadcasti64x2 zmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd cf 5a 31[ 	]*vbroadcasti64x2 zmm6\{k7\}\{z\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a b4 f4 c0 1d fe ff[ 	]*vbroadcasti64x2 zmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a 72 7f[ 	]*vbroadcasti64x2 zmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 zmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a 72 80[ 	]*vbroadcasti64x2 zmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 48 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 zmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 f7[ 	]*vbroadcastf32x2 zmm6,xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 19 f7[ 	]*vbroadcastf32x2 zmm6\{k7\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 19 f7[ 	]*vbroadcastf32x2 zmm6\{k7\}\{z\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 31[ 	]*vbroadcastf32x2 zmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 b4 f4 c0 1d fe ff[ 	]*vbroadcastf32x2 zmm6,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 72 7f[ 	]*vbroadcastf32x2 zmm6,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 zmm6,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 72 80[ 	]*vbroadcastf32x2 zmm6,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 zmm6,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b f5[ 	]*vcvtpd2qq zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 7b f5[ 	]*vcvtpd2qq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 7b f5[ 	]*vcvtpd2qq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 7b f5[ 	]*vcvtpd2qq zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b f5[ 	]*vcvtpd2qq zmm6,zmm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 38 7b f5[ 	]*vcvtpd2qq zmm6,zmm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 78 7b f5[ 	]*vcvtpd2qq zmm6,zmm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b 31[ 	]*vcvtpd2qq zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b b4 f4 c0 1d fe ff[ 	]*vcvtpd2qq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b 30[ 	]*vcvtpd2qq zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b 72 7f[ 	]*vcvtpd2qq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b b2 00 20 00 00[ 	]*vcvtpd2qq zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b 72 80[ 	]*vcvtpd2qq zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7b b2 c0 df ff ff[ 	]*vcvtpd2qq zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b 72 7f[ 	]*vcvtpd2qq zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b b2 00 04 00 00[ 	]*vcvtpd2qq zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b 72 80[ 	]*vcvtpd2qq zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 f5[ 	]*vcvtpd2uqq zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 79 f5[ 	]*vcvtpd2uqq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 79 f5[ 	]*vcvtpd2uqq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 79 f5[ 	]*vcvtpd2uqq zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 f5[ 	]*vcvtpd2uqq zmm6,zmm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 38 79 f5[ 	]*vcvtpd2uqq zmm6,zmm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 78 79 f5[ 	]*vcvtpd2uqq zmm6,zmm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 31[ 	]*vcvtpd2uqq zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 b4 f4 c0 1d fe ff[ 	]*vcvtpd2uqq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 30[ 	]*vcvtpd2uqq zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 72 7f[ 	]*vcvtpd2uqq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 b2 00 20 00 00[ 	]*vcvtpd2uqq zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 72 80[ 	]*vcvtpd2uqq zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 79 b2 c0 df ff ff[ 	]*vcvtpd2uqq zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 72 7f[ 	]*vcvtpd2uqq zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 b2 00 04 00 00[ 	]*vcvtpd2uqq zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 72 80[ 	]*vcvtpd2uqq zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b f5[ 	]*vcvtps2qq zmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 7b f5[ 	]*vcvtps2qq zmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7b f5[ 	]*vcvtps2qq zmm6\{k7\},ymm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b f5[ 	]*vcvtps2qq zmm6\{k7\},ymm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 7b f5[ 	]*vcvtps2qq zmm6\{k7\},ymm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 7f 7b f5[ 	]*vcvtps2qq zmm6\{k7\},ymm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b 31[ 	]*vcvtps2qq zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b b4 f4 c0 1d fe ff[ 	]*vcvtps2qq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b 30[ 	]*vcvtps2qq zmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b 72 7f[ 	]*vcvtps2qq zmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b b2 00 10 00 00[ 	]*vcvtps2qq zmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b 72 80[ 	]*vcvtps2qq zmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7b b2 e0 ef ff ff[ 	]*vcvtps2qq zmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b 72 7f[ 	]*vcvtps2qq zmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b b2 00 02 00 00[ 	]*vcvtps2qq zmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b 72 80[ 	]*vcvtps2qq zmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b b2 fc fd ff ff[ 	]*vcvtps2qq zmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7b 72 7f[ 	]*vcvtps2qq zmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 f5[ 	]*vcvtps2uqq zmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 79 f5[ 	]*vcvtps2uqq zmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 79 f5[ 	]*vcvtps2uqq zmm6\{k7\},ymm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 f5[ 	]*vcvtps2uqq zmm6\{k7\},ymm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 3f 79 f5[ 	]*vcvtps2uqq zmm6\{k7\},ymm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 7f 79 f5[ 	]*vcvtps2uqq zmm6\{k7\},ymm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 31[ 	]*vcvtps2uqq zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 b4 f4 c0 1d fe ff[ 	]*vcvtps2uqq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 30[ 	]*vcvtps2uqq zmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 72 7f[ 	]*vcvtps2uqq zmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 b2 00 10 00 00[ 	]*vcvtps2uqq zmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 72 80[ 	]*vcvtps2uqq zmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 79 b2 e0 ef ff ff[ 	]*vcvtps2uqq zmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 72 7f[ 	]*vcvtps2uqq zmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 b2 00 02 00 00[ 	]*vcvtps2uqq zmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 72 80[ 	]*vcvtps2uqq zmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 b2 fc fd ff ff[ 	]*vcvtps2uqq zmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 79 72 7f[ 	]*vcvtps2uqq zmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 f5[ 	]*vcvtqq2pd zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 4f e6 f5[ 	]*vcvtqq2pd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe cf e6 f5[ 	]*vcvtqq2pd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 18 e6 f5[ 	]*vcvtqq2pd zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 f5[ 	]*vcvtqq2pd zmm6,zmm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 38 e6 f5[ 	]*vcvtqq2pd zmm6,zmm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 78 e6 f5[ 	]*vcvtqq2pd zmm6,zmm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 31[ 	]*vcvtqq2pd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 b4 f4 c0 1d fe ff[ 	]*vcvtqq2pd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 30[ 	]*vcvtqq2pd zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 72 7f[ 	]*vcvtqq2pd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 b2 00 20 00 00[ 	]*vcvtqq2pd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 72 80[ 	]*vcvtqq2pd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 e6 b2 c0 df ff ff[ 	]*vcvtqq2pd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 72 7f[ 	]*vcvtqq2pd zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 b2 00 04 00 00[ 	]*vcvtqq2pd zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 72 80[ 	]*vcvtqq2pd zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b f5[ 	]*vcvtqq2ps ymm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc cf 5b f5[ 	]*vcvtqq2ps ymm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 1f 5b f5[ 	]*vcvtqq2ps ymm6\{k7\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b f5[ 	]*vcvtqq2ps ymm6\{k7\},zmm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 3f 5b f5[ 	]*vcvtqq2ps ymm6\{k7\},zmm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 7f 5b f5[ 	]*vcvtqq2ps ymm6\{k7\},zmm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b 31[ 	]*vcvtqq2ps ymm6\{k7\},ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b b4 f4 c0 1d fe ff[ 	]*vcvtqq2ps ymm6\{k7\},ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b 30[ 	]*vcvtqq2ps ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b 72 7f[ 	]*vcvtqq2ps ymm6\{k7\},ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b b2 00 20 00 00[ 	]*vcvtqq2ps ymm6\{k7\},ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b 72 80[ 	]*vcvtqq2ps ymm6\{k7\},ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 4f 5b b2 c0 df ff ff[ 	]*vcvtqq2ps ymm6\{k7\},ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b 72 7f[ 	]*vcvtqq2ps ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b b2 00 04 00 00[ 	]*vcvtqq2ps ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b 72 80[ 	]*vcvtqq2ps ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fc 5f 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a f5[ 	]*vcvtuqq2pd zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 4f 7a f5[ 	]*vcvtuqq2pd zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe cf 7a f5[ 	]*vcvtuqq2pd zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 18 7a f5[ 	]*vcvtuqq2pd zmm6,zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a f5[ 	]*vcvtuqq2pd zmm6,zmm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 38 7a f5[ 	]*vcvtuqq2pd zmm6,zmm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 78 7a f5[ 	]*vcvtuqq2pd zmm6,zmm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a 31[ 	]*vcvtuqq2pd zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2pd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a 30[ 	]*vcvtuqq2pd zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a 72 7f[ 	]*vcvtuqq2pd zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a b2 00 20 00 00[ 	]*vcvtuqq2pd zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a 72 80[ 	]*vcvtuqq2pd zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 48 7a b2 c0 df ff ff[ 	]*vcvtuqq2pd zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a 72 7f[ 	]*vcvtuqq2pd zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a b2 00 04 00 00[ 	]*vcvtuqq2pd zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a 72 80[ 	]*vcvtuqq2pd zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 58 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a f5[ 	]*vcvtuqq2ps ymm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff cf 7a f5[ 	]*vcvtuqq2ps ymm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 1f 7a f5[ 	]*vcvtuqq2ps ymm6\{k7\},zmm5\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a f5[ 	]*vcvtuqq2ps ymm6\{k7\},zmm5\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 3f 7a f5[ 	]*vcvtuqq2ps ymm6\{k7\},zmm5\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 7f 7a f5[ 	]*vcvtuqq2ps ymm6\{k7\},zmm5\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a 31[ 	]*vcvtuqq2ps ymm6\{k7\},ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a b4 f4 c0 1d fe ff[ 	]*vcvtuqq2ps ymm6\{k7\},ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a 30[ 	]*vcvtuqq2ps ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a 72 7f[ 	]*vcvtuqq2ps ymm6\{k7\},ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a b2 00 20 00 00[ 	]*vcvtuqq2ps ymm6\{k7\},ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a 72 80[ 	]*vcvtuqq2ps ymm6\{k7\},ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 4f 7a b2 c0 df ff ff[ 	]*vcvtuqq2ps ymm6\{k7\},ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a 72 7f[ 	]*vcvtuqq2ps ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a b2 00 04 00 00[ 	]*vcvtuqq2ps ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a 72 80[ 	]*vcvtuqq2ps ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 5f 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 19 ee ab[ 	]*vextractf64x2 xmm6\{k7\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd cf 19 ee ab[ 	]*vextractf64x2 xmm6\{k7\}\{z\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 19 ee 7b[ 	]*vextractf64x2 xmm6\{k7\},zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 1b ee ab[ 	]*vextractf32x8 ymm6\{k7\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d cf 1b ee ab[ 	]*vextractf32x8 ymm6\{k7\}\{z\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 1b ee 7b[ 	]*vextractf32x8 ymm6\{k7\},zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 39 ee ab[ 	]*vextracti64x2 xmm6\{k7\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd cf 39 ee ab[ 	]*vextracti64x2 xmm6\{k7\}\{z\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 39 ee 7b[ 	]*vextracti64x2 xmm6\{k7\},zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 3b ee ab[ 	]*vextracti32x8 ymm6\{k7\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d cf 3b ee ab[ 	]*vextracti32x8 ymm6\{k7\}\{z\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 3b ee 7b[ 	]*vextracti32x8 ymm6\{k7\},zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 ee ab[ 	]*vfpclasspd k5,zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 66 ee ab[ 	]*vfpclasspd k5\{k7\},zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 ee 7b[ 	]*vfpclasspd k5,zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 29 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 28 7b[ 	]*vfpclasspd k5,QWORD BCST \[eax\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 7f 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa 00 20 00 00 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[edx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 80 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[edx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa c0 df ff ff 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[edx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 7f 7b[ 	]*vfpclasspd k5,QWORD BCST \[edx\+0x3f8\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa 00 04 00 00 7b[ 	]*vfpclasspd k5,QWORD BCST \[edx\+0x400\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 80 7b[ 	]*vfpclasspd k5,QWORD BCST \[edx-0x400\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd k5,QWORD BCST \[edx-0x408\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 ee ab[ 	]*vfpclassps k5,zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 66 ee ab[ 	]*vfpclassps k5\{k7\},zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 ee 7b[ 	]*vfpclassps k5,zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 29 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 ac f4 c0 1d fe ff 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 28 7b[ 	]*vfpclassps k5,DWORD BCST \[eax\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 7f 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa 00 20 00 00 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[edx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 80 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[edx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa c0 df ff ff 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[edx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 7f 7b[ 	]*vfpclassps k5,DWORD BCST \[edx\+0x1fc\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa 00 02 00 00 7b[ 	]*vfpclassps k5,DWORD BCST \[edx\+0x200\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 80 7b[ 	]*vfpclassps k5,DWORD BCST \[edx-0x200\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa fc fd ff ff 7b[ 	]*vfpclassps k5,DWORD BCST \[edx-0x204\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 ee ab[ 	]*vfpclasssd k5\{k7\},xmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 ee 7b[ 	]*vfpclasssd k5\{k7\},xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 29 7b[ 	]*vfpclasssd k5\{k7\},QWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 ac f4 c0 1d fe ff 7b[ 	]*vfpclasssd k5\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 6a 7f 7b[ 	]*vfpclasssd k5\{k7\},QWORD PTR \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 aa 00 04 00 00 7b[ 	]*vfpclasssd k5\{k7\},QWORD PTR \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 6a 80 7b[ 	]*vfpclasssd k5\{k7\},QWORD PTR \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 0f 67 aa f8 fb ff ff 7b[ 	]*vfpclasssd k5\{k7\},QWORD PTR \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 ee ab[ 	]*vfpclassss k5\{k7\},xmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 ee 7b[ 	]*vfpclassss k5\{k7\},xmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 29 7b[ 	]*vfpclassss k5\{k7\},DWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 ac f4 c0 1d fe ff 7b[ 	]*vfpclassss k5\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 6a 7f 7b[ 	]*vfpclassss k5\{k7\},DWORD PTR \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 aa 00 02 00 00 7b[ 	]*vfpclassss k5\{k7\},DWORD PTR \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 6a 80 7b[ 	]*vfpclassss k5\{k7\},DWORD PTR \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 0f 67 aa fc fd ff ff 7b[ 	]*vfpclassss k5\{k7\},DWORD PTR \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 f4 ab[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 18 f4 ab[ 	]*vinsertf64x2 zmm6\{k7\}\{z\},zmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 f4 7b[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 31 7b[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 b4 f4 c0 1d fe ff 7b[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 72 7f 7b[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 72 80 7b[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a f4 ab[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 1a f4 ab[ 	]*vinsertf32x8 zmm6\{k7\}\{z\},zmm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a f4 7b[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a 31 7b[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a b4 f4 c0 1d fe ff 7b[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a 72 7f 7b[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a b2 00 10 00 00 7b[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a 72 80 7b[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 1a b2 e0 ef ff ff 7b[ 	]*vinsertf32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 f4 ab[ 	]*vinserti64x2 zmm6\{k7\},zmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 38 f4 ab[ 	]*vinserti64x2 zmm6\{k7\}\{z\},zmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 f4 7b[ 	]*vinserti64x2 zmm6\{k7\},zmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 31 7b[ 	]*vinserti64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 b4 f4 c0 1d fe ff 7b[ 	]*vinserti64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 72 7f 7b[ 	]*vinserti64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 72 80 7b[ 	]*vinserti64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 zmm6\{k7\},zmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a f4 ab[ 	]*vinserti32x8 zmm6\{k7\},zmm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 3a f4 ab[ 	]*vinserti32x8 zmm6\{k7\}\{z\},zmm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a f4 7b[ 	]*vinserti32x8 zmm6\{k7\},zmm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a 31 7b[ 	]*vinserti32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a b4 f4 c0 1d fe ff 7b[ 	]*vinserti32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a 72 7f 7b[ 	]*vinserti32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a b2 00 10 00 00 7b[ 	]*vinserti32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a 72 80 7b[ 	]*vinserti32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 3a b2 e0 ef ff ff 7b[ 	]*vinserti32x8 zmm6\{k7\},zmm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 f7[ 	]*vbroadcasti32x2 zmm6,xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 4f 59 f7[ 	]*vbroadcasti32x2 zmm6\{k7\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d cf 59 f7[ 	]*vbroadcasti32x2 zmm6\{k7\}\{z\},xmm7
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 31[ 	]*vbroadcasti32x2 zmm6,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 b4 f4 c0 1d fe ff[ 	]*vbroadcasti32x2 zmm6,QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 72 7f[ 	]*vbroadcasti32x2 zmm6,QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 zmm6,QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 72 80[ 	]*vbroadcasti32x2 zmm6,QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 48 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 zmm6,QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 f4[ 	]*vpmullq zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 4f 40 f4[ 	]*vpmullq zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 cf 40 f4[ 	]*vpmullq zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 31[ 	]*vpmullq zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 b4 f4 c0 1d fe ff[ 	]*vpmullq zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 30[ 	]*vpmullq zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 72 7f[ 	]*vpmullq zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 b2 00 20 00 00[ 	]*vpmullq zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 72 80[ 	]*vpmullq zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 48 40 b2 c0 df ff ff[ 	]*vpmullq zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 72 7f[ 	]*vpmullq zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 b2 00 04 00 00[ 	]*vpmullq zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 72 80[ 	]*vpmullq zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 58 40 b2 f8 fb ff ff[ 	]*vpmullq zmm6,zmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 f4 ab[ 	]*vrangepd zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f 50 f4 ab[ 	]*vrangepd zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf 50 f4 ab[ 	]*vrangepd zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 18 50 f4 ab[ 	]*vrangepd zmm6,zmm5,zmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 f4 7b[ 	]*vrangepd zmm6,zmm5,zmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 18 50 f4 7b[ 	]*vrangepd zmm6,zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 31 7b[ 	]*vrangepd zmm6,zmm5,ZMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangepd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 30 7b[ 	]*vrangepd zmm6,zmm5,QWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 72 7f 7b[ 	]*vrangepd zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 b2 00 20 00 00 7b[ 	]*vrangepd zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 72 80 7b[ 	]*vrangepd zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 50 b2 c0 df ff ff 7b[ 	]*vrangepd zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 72 7f 7b[ 	]*vrangepd zmm6,zmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 b2 00 04 00 00 7b[ 	]*vrangepd zmm6,zmm5,QWORD BCST \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 72 80 7b[ 	]*vrangepd zmm6,zmm5,QWORD BCST \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 50 b2 f8 fb ff ff 7b[ 	]*vrangepd zmm6,zmm5,QWORD BCST \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 f4 ab[ 	]*vrangeps zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 4f 50 f4 ab[ 	]*vrangeps zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 cf 50 f4 ab[ 	]*vrangeps zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 18 50 f4 ab[ 	]*vrangeps zmm6,zmm5,zmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 f4 7b[ 	]*vrangeps zmm6,zmm5,zmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 18 50 f4 7b[ 	]*vrangeps zmm6,zmm5,zmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 31 7b[ 	]*vrangeps zmm6,zmm5,ZMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 b4 f4 c0 1d fe ff 7b[ 	]*vrangeps zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 30 7b[ 	]*vrangeps zmm6,zmm5,DWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 72 7f 7b[ 	]*vrangeps zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 b2 00 20 00 00 7b[ 	]*vrangeps zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 72 80 7b[ 	]*vrangeps zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 50 b2 c0 df ff ff 7b[ 	]*vrangeps zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 72 7f 7b[ 	]*vrangeps zmm6,zmm5,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 b2 00 02 00 00 7b[ 	]*vrangeps zmm6,zmm5,DWORD BCST \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 72 80 7b[ 	]*vrangeps zmm6,zmm5,DWORD BCST \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 58 50 b2 fc fd ff ff 7b[ 	]*vrangeps zmm6,zmm5,DWORD BCST \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 f4 ab[ 	]*vrangesd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 51 f4 ab[ 	]*vrangesd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 51 f4 ab[ 	]*vrangesd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 f4 7b[ 	]*vrangesd xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 51 f4 7b[ 	]*vrangesd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 31 7b[ 	]*vrangesd xmm6\{k7\},xmm5,QWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 b4 f4 c0 1d fe ff 7b[ 	]*vrangesd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 72 7f 7b[ 	]*vrangesd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 b2 00 04 00 00 7b[ 	]*vrangesd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 72 80 7b[ 	]*vrangesd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 51 b2 f8 fb ff ff 7b[ 	]*vrangesd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 f4 ab[ 	]*vrangess xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 51 f4 ab[ 	]*vrangess xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 51 f4 ab[ 	]*vrangess xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 f4 7b[ 	]*vrangess xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 51 f4 7b[ 	]*vrangess xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 31 7b[ 	]*vrangess xmm6\{k7\},xmm5,DWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 b4 f4 c0 1d fe ff 7b[ 	]*vrangess xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 72 7f 7b[ 	]*vrangess xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 b2 00 02 00 00 7b[ 	]*vrangess xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 72 80 7b[ 	]*vrangess xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 51 b2 fc fd ff ff 7b[ 	]*vrangess xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 f4[ 	]*vandpd zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 54 f4[ 	]*vandpd zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 54 f4[ 	]*vandpd zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 31[ 	]*vandpd zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 b4 f4 c0 1d fe ff[ 	]*vandpd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 30[ 	]*vandpd zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 72 7f[ 	]*vandpd zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 b2 00 20 00 00[ 	]*vandpd zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 72 80[ 	]*vandpd zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 54 b2 c0 df ff ff[ 	]*vandpd zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 72 7f[ 	]*vandpd zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 b2 00 04 00 00[ 	]*vandpd zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 72 80[ 	]*vandpd zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 54 b2 f8 fb ff ff[ 	]*vandpd zmm6,zmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 f4[ 	]*vandps zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 54 f4[ 	]*vandps zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 54 f4[ 	]*vandps zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 31[ 	]*vandps zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 b4 f4 c0 1d fe ff[ 	]*vandps zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 30[ 	]*vandps zmm6,zmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 72 7f[ 	]*vandps zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 b2 00 20 00 00[ 	]*vandps zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 72 80[ 	]*vandps zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 54 b2 c0 df ff ff[ 	]*vandps zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 72 7f[ 	]*vandps zmm6,zmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 b2 00 02 00 00[ 	]*vandps zmm6,zmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 72 80[ 	]*vandps zmm6,zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 54 b2 fc fd ff ff[ 	]*vandps zmm6,zmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 f4[ 	]*vandnpd zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 55 f4[ 	]*vandnpd zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 55 f4[ 	]*vandnpd zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 31[ 	]*vandnpd zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 b4 f4 c0 1d fe ff[ 	]*vandnpd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 30[ 	]*vandnpd zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 72 7f[ 	]*vandnpd zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 b2 00 20 00 00[ 	]*vandnpd zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 72 80[ 	]*vandnpd zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 55 b2 c0 df ff ff[ 	]*vandnpd zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 72 7f[ 	]*vandnpd zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 b2 00 04 00 00[ 	]*vandnpd zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 72 80[ 	]*vandnpd zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 55 b2 f8 fb ff ff[ 	]*vandnpd zmm6,zmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 f4[ 	]*vandnps zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 55 f4[ 	]*vandnps zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 55 f4[ 	]*vandnps zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 31[ 	]*vandnps zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 b4 f4 c0 1d fe ff[ 	]*vandnps zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 30[ 	]*vandnps zmm6,zmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 72 7f[ 	]*vandnps zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 b2 00 20 00 00[ 	]*vandnps zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 72 80[ 	]*vandnps zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 55 b2 c0 df ff ff[ 	]*vandnps zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 72 7f[ 	]*vandnps zmm6,zmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 b2 00 02 00 00[ 	]*vandnps zmm6,zmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 72 80[ 	]*vandnps zmm6,zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 55 b2 fc fd ff ff[ 	]*vandnps zmm6,zmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 f4[ 	]*vorpd  zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 56 f4[ 	]*vorpd  zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 56 f4[ 	]*vorpd  zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 31[ 	]*vorpd  zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 b4 f4 c0 1d fe ff[ 	]*vorpd  zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 30[ 	]*vorpd  zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 72 7f[ 	]*vorpd  zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 b2 00 20 00 00[ 	]*vorpd  zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 72 80[ 	]*vorpd  zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 56 b2 c0 df ff ff[ 	]*vorpd  zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 72 7f[ 	]*vorpd  zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 b2 00 04 00 00[ 	]*vorpd  zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 72 80[ 	]*vorpd  zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 56 b2 f8 fb ff ff[ 	]*vorpd  zmm6,zmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 f4[ 	]*vorps  zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 56 f4[ 	]*vorps  zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 56 f4[ 	]*vorps  zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 31[ 	]*vorps  zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 b4 f4 c0 1d fe ff[ 	]*vorps  zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 30[ 	]*vorps  zmm6,zmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 72 7f[ 	]*vorps  zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 b2 00 20 00 00[ 	]*vorps  zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 72 80[ 	]*vorps  zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 56 b2 c0 df ff ff[ 	]*vorps  zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 72 7f[ 	]*vorps  zmm6,zmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 b2 00 02 00 00[ 	]*vorps  zmm6,zmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 72 80[ 	]*vorps  zmm6,zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 56 b2 fc fd ff ff[ 	]*vorps  zmm6,zmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 f4[ 	]*vxorpd zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 4f 57 f4[ 	]*vxorpd zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 cf 57 f4[ 	]*vxorpd zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 31[ 	]*vxorpd zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 b4 f4 c0 1d fe ff[ 	]*vxorpd zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 30[ 	]*vxorpd zmm6,zmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 72 7f[ 	]*vxorpd zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 b2 00 20 00 00[ 	]*vxorpd zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 72 80[ 	]*vxorpd zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 48 57 b2 c0 df ff ff[ 	]*vxorpd zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 72 7f[ 	]*vxorpd zmm6,zmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 b2 00 04 00 00[ 	]*vxorpd zmm6,zmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 72 80[ 	]*vxorpd zmm6,zmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 58 57 b2 f8 fb ff ff[ 	]*vxorpd zmm6,zmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 f4[ 	]*vxorps zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 4f 57 f4[ 	]*vxorps zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 cf 57 f4[ 	]*vxorps zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 31[ 	]*vxorps zmm6,zmm5,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 b4 f4 c0 1d fe ff[ 	]*vxorps zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 30[ 	]*vxorps zmm6,zmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 72 7f[ 	]*vxorps zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 b2 00 20 00 00[ 	]*vxorps zmm6,zmm5,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 72 80[ 	]*vxorps zmm6,zmm5,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 48 57 b2 c0 df ff ff[ 	]*vxorps zmm6,zmm5,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 72 7f[ 	]*vxorps zmm6,zmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 b2 00 02 00 00[ 	]*vxorps zmm6,zmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 72 80[ 	]*vxorps zmm6,zmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 54 58 57 b2 fc fd ff ff[ 	]*vxorps zmm6,zmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 f5 ab[ 	]*vreducepd zmm6,zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 56 f5 ab[ 	]*vreducepd zmm6\{k7\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd cf 56 f5 ab[ 	]*vreducepd zmm6\{k7\}\{z\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 56 f5 ab[ 	]*vreducepd zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 f5 7b[ 	]*vreducepd zmm6,zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 18 56 f5 7b[ 	]*vreducepd zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 31 7b[ 	]*vreducepd zmm6,ZMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 b4 f4 c0 1d fe ff 7b[ 	]*vreducepd zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 30 7b[ 	]*vreducepd zmm6,QWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 72 7f 7b[ 	]*vreducepd zmm6,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 b2 00 20 00 00 7b[ 	]*vreducepd zmm6,ZMMWORD PTR \[edx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 72 80 7b[ 	]*vreducepd zmm6,ZMMWORD PTR \[edx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 56 b2 c0 df ff ff 7b[ 	]*vreducepd zmm6,ZMMWORD PTR \[edx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 72 7f 7b[ 	]*vreducepd zmm6,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 b2 00 04 00 00 7b[ 	]*vreducepd zmm6,QWORD BCST \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 72 80 7b[ 	]*vreducepd zmm6,QWORD BCST \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 56 b2 f8 fb ff ff 7b[ 	]*vreducepd zmm6,QWORD BCST \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 f5 ab[ 	]*vreduceps zmm6,zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 56 f5 ab[ 	]*vreduceps zmm6\{k7\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d cf 56 f5 ab[ 	]*vreduceps zmm6\{k7\}\{z\},zmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 56 f5 ab[ 	]*vreduceps zmm6,zmm5\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 f5 7b[ 	]*vreduceps zmm6,zmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 18 56 f5 7b[ 	]*vreduceps zmm6,zmm5\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 31 7b[ 	]*vreduceps zmm6,ZMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 b4 f4 c0 1d fe ff 7b[ 	]*vreduceps zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 30 7b[ 	]*vreduceps zmm6,DWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 72 7f 7b[ 	]*vreduceps zmm6,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 b2 00 20 00 00 7b[ 	]*vreduceps zmm6,ZMMWORD PTR \[edx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 72 80 7b[ 	]*vreduceps zmm6,ZMMWORD PTR \[edx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 56 b2 c0 df ff ff 7b[ 	]*vreduceps zmm6,ZMMWORD PTR \[edx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 72 7f 7b[ 	]*vreduceps zmm6,DWORD BCST \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 b2 00 02 00 00 7b[ 	]*vreduceps zmm6,DWORD BCST \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 72 80 7b[ 	]*vreduceps zmm6,DWORD BCST \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 56 b2 fc fd ff ff 7b[ 	]*vreduceps zmm6,DWORD BCST \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 f4 ab[ 	]*vreducesd xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 57 f4 ab[ 	]*vreducesd xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 57 f4 ab[ 	]*vreducesd xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 f4 7b[ 	]*vreducesd xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f 57 f4 7b[ 	]*vreducesd xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 31 7b[ 	]*vreducesd xmm6\{k7\},xmm5,QWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 b4 f4 c0 1d fe ff 7b[ 	]*vreducesd xmm6\{k7\},xmm5,QWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 72 7f 7b[ 	]*vreducesd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 b2 00 04 00 00 7b[ 	]*vreducesd xmm6\{k7\},xmm5,QWORD PTR \[edx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 72 80 7b[ 	]*vreducesd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 57 b2 f8 fb ff ff 7b[ 	]*vreducesd xmm6\{k7\},xmm5,QWORD PTR \[edx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 f4 ab[ 	]*vreducess xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 57 f4 ab[ 	]*vreducess xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 57 f4 ab[ 	]*vreducess xmm6\{k7\},xmm5,xmm4\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 f4 7b[ 	]*vreducess xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 1f 57 f4 7b[ 	]*vreducess xmm6\{k7\},xmm5,xmm4\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 31 7b[ 	]*vreducess xmm6\{k7\},xmm5,DWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 b4 f4 c0 1d fe ff 7b[ 	]*vreducess xmm6\{k7\},xmm5,DWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 72 7f 7b[ 	]*vreducess xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 b2 00 02 00 00 7b[ 	]*vreducess xmm6\{k7\},xmm5,DWORD PTR \[edx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 72 80 7b[ 	]*vreducess xmm6\{k7\},xmm5,DWORD PTR \[edx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 57 b2 fc fd ff ff 7b[ 	]*vreducess xmm6\{k7\},xmm5,DWORD PTR \[edx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c5 cd 41 ef[ 	]*kandb  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 cd 42 ef[ 	]*kandnb k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 cd 45 ef[ 	]*korb   k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 cd 46 ef[ 	]*kxnorb k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 cd 47 ef[ 	]*kxorb  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 f9 44 ee[ 	]*knotb  k5,k6
[ 	]*[a-f0-9]+:[ 	]*c5 f9 98 ee[ 	]*kortestb k5,k6
[ 	]*[a-f0-9]+:[ 	]*c5 f8 99 ee[ 	]*ktestw k5,k6
[ 	]*[a-f0-9]+:[ 	]*c5 f9 99 ee[ 	]*ktestb k5,k6
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 30 ee ab[ 	]*kshiftrb k5,k6,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 30 ee 7b[ 	]*kshiftrb k5,k6,0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 32 ee ab[ 	]*kshiftlb k5,k6,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 79 32 ee 7b[ 	]*kshiftlb k5,k6,0x7b
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 ee[ 	]*kmovb  k5,k6
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 29[ 	]*kmovb  k5,BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 ac f4 c0 1d fe ff[ 	]*kmovb  k5,BYTE PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c5 f9 91 29[ 	]*kmovb  BYTE PTR \[ecx\],k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 91 ac f4 c0 1d fe ff[ 	]*kmovb  BYTE PTR \[esp\+esi\*8-0x1e240\],k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 e8[ 	]*kmovb  k5,eax
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 ed[ 	]*kmovb  k5,ebp
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 c5[ 	]*kmovb  eax,k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 ed[ 	]*kmovb  ebp,k5
[ 	]*[a-f0-9]+:[ 	]*c5 cc 4a ef[ 	]*kaddw  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 cd 4a ef[ 	]*kaddb  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 31 ab[ 	]*vextractf64x2 XMMWORD PTR \[ecx\],zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 19 31 ab[ 	]*vextractf64x2 XMMWORD PTR \[ecx\]\{k7\},zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 31 7b[ 	]*vextractf64x2 XMMWORD PTR \[ecx\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 b4 f4 c0 1d fe ff 7b[ 	]*vextractf64x2 XMMWORD PTR \[esp\+esi\*8-0x1e240\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 72 7f 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx\+0x7f0\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 b2 00 08 00 00 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx\+0x800\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 72 80 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx-0x800\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 19 b2 f0 f7 ff ff 7b[ 	]*vextractf64x2 XMMWORD PTR \[edx-0x810\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 31 ab[ 	]*vextractf32x8 YMMWORD PTR \[ecx\],zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 1b 31 ab[ 	]*vextractf32x8 YMMWORD PTR \[ecx\]\{k7\},zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 31 7b[ 	]*vextractf32x8 YMMWORD PTR \[ecx\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b b4 f4 c0 1d fe ff 7b[ 	]*vextractf32x8 YMMWORD PTR \[esp\+esi\*8-0x1e240\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 72 7f 7b[ 	]*vextractf32x8 YMMWORD PTR \[edx\+0xfe0\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b b2 00 10 00 00 7b[ 	]*vextractf32x8 YMMWORD PTR \[edx\+0x1000\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b 72 80 7b[ 	]*vextractf32x8 YMMWORD PTR \[edx-0x1000\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 1b b2 e0 ef ff ff 7b[ 	]*vextractf32x8 YMMWORD PTR \[edx-0x1020\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 31 ab[ 	]*vextracti64x2 XMMWORD PTR \[ecx\],zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 4f 39 31 ab[ 	]*vextracti64x2 XMMWORD PTR \[ecx\]\{k7\},zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 31 7b[ 	]*vextracti64x2 XMMWORD PTR \[ecx\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 b4 f4 c0 1d fe ff 7b[ 	]*vextracti64x2 XMMWORD PTR \[esp\+esi\*8-0x1e240\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 72 7f 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx\+0x7f0\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 b2 00 08 00 00 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx\+0x800\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 72 80 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx-0x800\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 39 b2 f0 f7 ff ff 7b[ 	]*vextracti64x2 XMMWORD PTR \[edx-0x810\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 31 ab[ 	]*vextracti32x8 YMMWORD PTR \[ecx\],zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 4f 3b 31 ab[ 	]*vextracti32x8 YMMWORD PTR \[ecx\]\{k7\},zmm6,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 31 7b[ 	]*vextracti32x8 YMMWORD PTR \[ecx\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b b4 f4 c0 1d fe ff 7b[ 	]*vextracti32x8 YMMWORD PTR \[esp\+esi\*8-0x1e240\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 72 7f 7b[ 	]*vextracti32x8 YMMWORD PTR \[edx\+0xfe0\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b b2 00 10 00 00 7b[ 	]*vextracti32x8 YMMWORD PTR \[edx\+0x1000\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b 72 80 7b[ 	]*vextracti32x8 YMMWORD PTR \[edx-0x1000\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 3b b2 e0 ef ff ff 7b[ 	]*vextracti32x8 YMMWORD PTR \[edx-0x1020\],zmm6,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a f5[ 	]*vcvttpd2qq zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 7a f5[ 	]*vcvttpd2qq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 7a f5[ 	]*vcvttpd2qq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 7a f5[ 	]*vcvttpd2qq zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a 31[ 	]*vcvttpd2qq zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a b4 f4 c0 1d fe ff[ 	]*vcvttpd2qq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a 30[ 	]*vcvttpd2qq zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a 72 7f[ 	]*vcvttpd2qq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a b2 00 20 00 00[ 	]*vcvttpd2qq zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a 72 80[ 	]*vcvttpd2qq zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 7a b2 c0 df ff ff[ 	]*vcvttpd2qq zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a 72 7f[ 	]*vcvttpd2qq zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a b2 00 04 00 00[ 	]*vcvttpd2qq zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a 72 80[ 	]*vcvttpd2qq zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 f5[ 	]*vcvttpd2uqq zmm6,zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 4f 78 f5[ 	]*vcvttpd2uqq zmm6\{k7\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd cf 78 f5[ 	]*vcvttpd2uqq zmm6\{k7\}\{z\},zmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 18 78 f5[ 	]*vcvttpd2uqq zmm6,zmm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 31[ 	]*vcvttpd2uqq zmm6,ZMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 b4 f4 c0 1d fe ff[ 	]*vcvttpd2uqq zmm6,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 30[ 	]*vcvttpd2uqq zmm6,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 72 7f[ 	]*vcvttpd2uqq zmm6,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 b2 00 20 00 00[ 	]*vcvttpd2uqq zmm6,ZMMWORD PTR \[edx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 72 80[ 	]*vcvttpd2uqq zmm6,ZMMWORD PTR \[edx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 48 78 b2 c0 df ff ff[ 	]*vcvttpd2uqq zmm6,ZMMWORD PTR \[edx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 72 7f[ 	]*vcvttpd2uqq zmm6,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 b2 00 04 00 00[ 	]*vcvttpd2uqq zmm6,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 72 80[ 	]*vcvttpd2uqq zmm6,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fd 58 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq zmm6,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a f5[ 	]*vcvttps2qq zmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 7a f5[ 	]*vcvttps2qq zmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 7a f5[ 	]*vcvttps2qq zmm6\{k7\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a 31[ 	]*vcvttps2qq zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a b4 f4 c0 1d fe ff[ 	]*vcvttps2qq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a 30[ 	]*vcvttps2qq zmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a 72 7f[ 	]*vcvttps2qq zmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a b2 00 10 00 00[ 	]*vcvttps2qq zmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a 72 80[ 	]*vcvttps2qq zmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 7a b2 e0 ef ff ff[ 	]*vcvttps2qq zmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a 72 7f[ 	]*vcvttps2qq zmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a b2 00 02 00 00[ 	]*vcvttps2qq zmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a 72 80[ 	]*vcvttps2qq zmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a b2 fc fd ff ff[ 	]*vcvttps2qq zmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 7a 72 7f[ 	]*vcvttps2qq zmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 f5[ 	]*vcvttps2uqq zmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d cf 78 f5[ 	]*vcvttps2uqq zmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 1f 78 f5[ 	]*vcvttps2uqq zmm6\{k7\},ymm5\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 31[ 	]*vcvttps2uqq zmm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 b4 f4 c0 1d fe ff[ 	]*vcvttps2uqq zmm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 30[ 	]*vcvttps2uqq zmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 72 7f[ 	]*vcvttps2uqq zmm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 b2 00 10 00 00[ 	]*vcvttps2uqq zmm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 72 80[ 	]*vcvttps2uqq zmm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 4f 78 b2 e0 ef ff ff[ 	]*vcvttps2uqq zmm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 72 7f[ 	]*vcvttps2uqq zmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 b2 00 02 00 00[ 	]*vcvttps2uqq zmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 72 80[ 	]*vcvttps2uqq zmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 b2 fc fd ff ff[ 	]*vcvttps2uqq zmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7d 5f 78 72 7f[ 	]*vcvttps2uqq zmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 48 39 ee[ 	]*vpmovd2m k5,zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 48 39 ee[ 	]*vpmovq2m k5,zmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 48 38 f5[ 	]*vpmovm2d zmm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 48 38 f5[ 	]*vpmovm2q zmm6,k5
#pass
