#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512DQ insns (Intel disassembly)
#source: x86-64-avx512dq.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b 31[ 	]*vbroadcastf32x8 zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 1b 31[ 	]*vbroadcastf32x8 zmm30\{k7\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 1b 31[ 	]*vbroadcastf32x8 zmm30\{k7\}\{z\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 1b b4 f0 23 01 00 00[ 	]*vbroadcastf32x8 zmm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b 72 7f[ 	]*vbroadcastf32x8 zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b b2 00 10 00 00[ 	]*vbroadcastf32x8 zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b 72 80[ 	]*vbroadcastf32x8 zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b b2 e0 ef ff ff[ 	]*vbroadcastf32x8 zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a 31[ 	]*vbroadcastf64x2 zmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 1a 31[ 	]*vbroadcastf64x2 zmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd cf 1a 31[ 	]*vbroadcastf64x2 zmm30\{k7\}\{z\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 1a b4 f0 23 01 00 00[ 	]*vbroadcastf64x2 zmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a 72 7f[ 	]*vbroadcastf64x2 zmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 zmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a 72 80[ 	]*vbroadcastf64x2 zmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 zmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b 31[ 	]*vbroadcasti32x8 zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 5b 31[ 	]*vbroadcasti32x8 zmm30\{k7\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 5b 31[ 	]*vbroadcasti32x8 zmm30\{k7\}\{z\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 5b b4 f0 23 01 00 00[ 	]*vbroadcasti32x8 zmm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b 72 7f[ 	]*vbroadcasti32x8 zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b b2 00 10 00 00[ 	]*vbroadcasti32x8 zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b 72 80[ 	]*vbroadcasti32x8 zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b b2 e0 ef ff ff[ 	]*vbroadcasti32x8 zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a 31[ 	]*vbroadcasti64x2 zmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 5a 31[ 	]*vbroadcasti64x2 zmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd cf 5a 31[ 	]*vbroadcasti64x2 zmm30\{k7\}\{z\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 5a b4 f0 23 01 00 00[ 	]*vbroadcasti64x2 zmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a 72 7f[ 	]*vbroadcasti64x2 zmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 zmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a 72 80[ 	]*vbroadcasti64x2 zmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 zmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 19 f7[ 	]*vbroadcastf32x2 zmm30,xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 19 f7[ 	]*vbroadcastf32x2 zmm30\{k7\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 19 f7[ 	]*vbroadcastf32x2 zmm30\{k7\}\{z\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 31[ 	]*vbroadcastf32x2 zmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 19 b4 f0 23 01 00 00[ 	]*vbroadcastf32x2 zmm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 72 7f[ 	]*vbroadcastf32x2 zmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 zmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 72 80[ 	]*vbroadcastf32x2 zmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 zmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 7b f5[ 	]*vcvtpd2qq zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 7b f5[ 	]*vcvtpd2qq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 7b f5[ 	]*vcvtpd2qq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 7b f5[ 	]*vcvtpd2qq zmm30,zmm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 58 7b f5[ 	]*vcvtpd2qq zmm30,zmm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 7b f5[ 	]*vcvtpd2qq zmm30,zmm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 78 7b f5[ 	]*vcvtpd2qq zmm30,zmm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b 31[ 	]*vcvtpd2qq zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 7b b4 f0 23 01 00 00[ 	]*vcvtpd2qq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b 31[ 	]*vcvtpd2qq zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b 72 7f[ 	]*vcvtpd2qq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b b2 00 20 00 00[ 	]*vcvtpd2qq zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b 72 80[ 	]*vcvtpd2qq zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b b2 c0 df ff ff[ 	]*vcvtpd2qq zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b 72 7f[ 	]*vcvtpd2qq zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b b2 00 04 00 00[ 	]*vcvtpd2qq zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b 72 80[ 	]*vcvtpd2qq zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 79 f5[ 	]*vcvtpd2uqq zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 79 f5[ 	]*vcvtpd2uqq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 79 f5[ 	]*vcvtpd2uqq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 79 f5[ 	]*vcvtpd2uqq zmm30,zmm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 58 79 f5[ 	]*vcvtpd2uqq zmm30,zmm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 79 f5[ 	]*vcvtpd2uqq zmm30,zmm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 78 79 f5[ 	]*vcvtpd2uqq zmm30,zmm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 31[ 	]*vcvtpd2uqq zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 79 b4 f0 23 01 00 00[ 	]*vcvtpd2uqq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 31[ 	]*vcvtpd2uqq zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 72 7f[ 	]*vcvtpd2uqq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 b2 00 20 00 00[ 	]*vcvtpd2uqq zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 72 80[ 	]*vcvtpd2uqq zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 b2 c0 df ff ff[ 	]*vcvtpd2uqq zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 72 7f[ 	]*vcvtpd2uqq zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 b2 00 04 00 00[ 	]*vcvtpd2uqq zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 72 80[ 	]*vcvtpd2uqq zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 7b f5[ 	]*vcvtps2qq zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 7b f5[ 	]*vcvtps2qq zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 7b f5[ 	]*vcvtps2qq zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 7b f5[ 	]*vcvtps2qq zmm30,ymm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 58 7b f5[ 	]*vcvtps2qq zmm30,ymm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 38 7b f5[ 	]*vcvtps2qq zmm30,ymm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 78 7b f5[ 	]*vcvtps2qq zmm30,ymm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b 31[ 	]*vcvtps2qq zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 7b b4 f0 23 01 00 00[ 	]*vcvtps2qq zmm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b 31[ 	]*vcvtps2qq zmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b 72 7f[ 	]*vcvtps2qq zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b b2 00 10 00 00[ 	]*vcvtps2qq zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b 72 80[ 	]*vcvtps2qq zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b b2 e0 ef ff ff[ 	]*vcvtps2qq zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b 72 7f[ 	]*vcvtps2qq zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b b2 00 02 00 00[ 	]*vcvtps2qq zmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b 72 80[ 	]*vcvtps2qq zmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b b2 fc fd ff ff[ 	]*vcvtps2qq zmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 79 f5[ 	]*vcvtps2uqq zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 79 f5[ 	]*vcvtps2uqq zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 79 f5[ 	]*vcvtps2uqq zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 79 f5[ 	]*vcvtps2uqq zmm30,ymm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 58 79 f5[ 	]*vcvtps2uqq zmm30,ymm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 38 79 f5[ 	]*vcvtps2uqq zmm30,ymm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 78 79 f5[ 	]*vcvtps2uqq zmm30,ymm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 31[ 	]*vcvtps2uqq zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 79 b4 f0 23 01 00 00[ 	]*vcvtps2uqq zmm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 31[ 	]*vcvtps2uqq zmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 72 7f[ 	]*vcvtps2uqq zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 b2 00 10 00 00[ 	]*vcvtps2uqq zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 72 80[ 	]*vcvtps2uqq zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 b2 e0 ef ff ff[ 	]*vcvtps2uqq zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 72 7f[ 	]*vcvtps2uqq zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 b2 00 02 00 00[ 	]*vcvtps2uqq zmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 72 80[ 	]*vcvtps2uqq zmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 b2 fc fd ff ff[ 	]*vcvtps2uqq zmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 e6 f5[ 	]*vcvtqq2pd zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 4f e6 f5[ 	]*vcvtqq2pd zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe cf e6 f5[ 	]*vcvtqq2pd zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 18 e6 f5[ 	]*vcvtqq2pd zmm30,zmm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 58 e6 f5[ 	]*vcvtqq2pd zmm30,zmm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 38 e6 f5[ 	]*vcvtqq2pd zmm30,zmm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 78 e6 f5[ 	]*vcvtqq2pd zmm30,zmm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 31[ 	]*vcvtqq2pd zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 48 e6 b4 f0 23 01 00 00[ 	]*vcvtqq2pd zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 31[ 	]*vcvtqq2pd zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 72 7f[ 	]*vcvtqq2pd zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 b2 00 20 00 00[ 	]*vcvtqq2pd zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 72 80[ 	]*vcvtqq2pd zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 b2 c0 df ff ff[ 	]*vcvtqq2pd zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 72 7f[ 	]*vcvtqq2pd zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 b2 00 04 00 00[ 	]*vcvtqq2pd zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 72 80[ 	]*vcvtqq2pd zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 48 5b f5[ 	]*vcvtqq2ps ymm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 4f 5b f5[ 	]*vcvtqq2ps ymm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fc cf 5b f5[ 	]*vcvtqq2ps ymm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 18 5b f5[ 	]*vcvtqq2ps ymm30,zmm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 58 5b f5[ 	]*vcvtqq2ps ymm30,zmm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 38 5b f5[ 	]*vcvtqq2ps ymm30,zmm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 78 5b f5[ 	]*vcvtqq2ps ymm30,zmm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b 31[ 	]*vcvtqq2ps ymm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fc 48 5b b4 f0 23 01 00 00[ 	]*vcvtqq2ps ymm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b 31[ 	]*vcvtqq2ps ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b 72 7f[ 	]*vcvtqq2ps ymm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b b2 00 20 00 00[ 	]*vcvtqq2ps ymm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b 72 80[ 	]*vcvtqq2ps ymm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b b2 c0 df ff ff[ 	]*vcvtqq2ps ymm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b 72 7f[ 	]*vcvtqq2ps ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b b2 00 04 00 00[ 	]*vcvtqq2ps ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b 72 80[ 	]*vcvtqq2ps ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 7a f5[ 	]*vcvtuqq2pd zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 4f 7a f5[ 	]*vcvtuqq2pd zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe cf 7a f5[ 	]*vcvtuqq2pd zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 18 7a f5[ 	]*vcvtuqq2pd zmm30,zmm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 58 7a f5[ 	]*vcvtuqq2pd zmm30,zmm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 38 7a f5[ 	]*vcvtuqq2pd zmm30,zmm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 78 7a f5[ 	]*vcvtuqq2pd zmm30,zmm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a 31[ 	]*vcvtuqq2pd zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 48 7a b4 f0 23 01 00 00[ 	]*vcvtuqq2pd zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a 31[ 	]*vcvtuqq2pd zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a 72 7f[ 	]*vcvtuqq2pd zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a b2 00 20 00 00[ 	]*vcvtuqq2pd zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a 72 80[ 	]*vcvtuqq2pd zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a b2 c0 df ff ff[ 	]*vcvtuqq2pd zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a 72 7f[ 	]*vcvtuqq2pd zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a b2 00 04 00 00[ 	]*vcvtuqq2pd zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a 72 80[ 	]*vcvtuqq2pd zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 48 7a f5[ 	]*vcvtuqq2ps ymm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 4f 7a f5[ 	]*vcvtuqq2ps ymm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff cf 7a f5[ 	]*vcvtuqq2ps ymm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 18 7a f5[ 	]*vcvtuqq2ps ymm30,zmm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 58 7a f5[ 	]*vcvtuqq2ps ymm30,zmm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 38 7a f5[ 	]*vcvtuqq2ps ymm30,zmm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 78 7a f5[ 	]*vcvtuqq2ps ymm30,zmm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a 31[ 	]*vcvtuqq2ps ymm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 48 7a b4 f0 23 01 00 00[ 	]*vcvtuqq2ps ymm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a 31[ 	]*vcvtuqq2ps ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a 72 7f[ 	]*vcvtuqq2ps ymm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a b2 00 20 00 00[ 	]*vcvtuqq2ps ymm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a 72 80[ 	]*vcvtuqq2ps ymm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a b2 c0 df ff ff[ 	]*vcvtuqq2ps ymm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a 72 7f[ 	]*vcvtuqq2ps ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a b2 00 04 00 00[ 	]*vcvtuqq2ps ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a 72 80[ 	]*vcvtuqq2ps ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 19 ee ab[ 	]*vextractf64x2 xmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 4f 19 ee ab[ 	]*vextractf64x2 xmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd cf 19 ee ab[ 	]*vextractf64x2 xmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 19 ee 7b[ 	]*vextractf64x2 xmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 1b ee ab[ 	]*vextractf32x8 ymm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 4f 1b ee ab[ 	]*vextractf32x8 ymm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d cf 1b ee ab[ 	]*vextractf32x8 ymm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 1b ee 7b[ 	]*vextractf32x8 ymm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 39 ee ab[ 	]*vextracti64x2 xmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 4f 39 ee ab[ 	]*vextracti64x2 xmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd cf 39 ee ab[ 	]*vextracti64x2 xmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 39 ee 7b[ 	]*vextracti64x2 xmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 3b ee ab[ 	]*vextracti32x8 ymm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 4f 3b ee ab[ 	]*vextracti32x8 ymm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d cf 3b ee ab[ 	]*vextracti32x8 ymm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 3b ee 7b[ 	]*vextracti32x8 ymm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 48 66 ee ab[ 	]*vfpclasspd k5,zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 4f 66 ee ab[ 	]*vfpclasspd k5\{k7\},zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 48 66 ee 7b[ 	]*vfpclasspd k5,zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 29 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 48 66 ac f0 23 01 00 00 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 29 7b[ 	]*vfpclasspd k5,QWORD BCST \[rcx\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 7f 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa 00 20 00 00 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 80 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa c0 df ff ff 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 7f 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx\+0x3f8\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa 00 04 00 00 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx\+0x400\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 80 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx-0x400\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx-0x408\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 48 66 ee ab[ 	]*vfpclassps k5,zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 4f 66 ee ab[ 	]*vfpclassps k5\{k7\},zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 48 66 ee 7b[ 	]*vfpclassps k5,zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 29 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 48 66 ac f0 23 01 00 00 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 29 7b[ 	]*vfpclassps k5,DWORD BCST \[rcx\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 7f 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa 00 20 00 00 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 80 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa c0 df ff ff 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 7f 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx\+0x1fc\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa 00 02 00 00 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx\+0x200\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 80 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx-0x200\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa fc fd ff ff 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx-0x204\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 67 ee ab[ 	]*vfpclasssd k5,xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 0f 67 ee ab[ 	]*vfpclasssd k5\{k7\},xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 67 ee 7b[ 	]*vfpclasssd k5,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 29 7b[ 	]*vfpclasssd k5,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 08 67 ac f0 23 01 00 00 7b[ 	]*vfpclasssd k5,QWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 6a 7f 7b[ 	]*vfpclasssd k5,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 aa 00 04 00 00 7b[ 	]*vfpclasssd k5,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 6a 80 7b[ 	]*vfpclasssd k5,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 aa f8 fb ff ff 7b[ 	]*vfpclasssd k5,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 67 ee ab[ 	]*vfpclassss k5,xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 0f 67 ee ab[ 	]*vfpclassss k5\{k7\},xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 67 ee 7b[ 	]*vfpclassss k5,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 29 7b[ 	]*vfpclassss k5,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 08 67 ac f0 23 01 00 00 7b[ 	]*vfpclassss k5,DWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 6a 7f 7b[ 	]*vfpclassss k5,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 aa 00 02 00 00 7b[ 	]*vfpclassss k5,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 6a 80 7b[ 	]*vfpclassss k5,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 aa fc fd ff ff 7b[ 	]*vfpclassss k5,DWORD PTR \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 18 f4 ab[ 	]*vinsertf64x2 zmm30,zmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 18 f4 ab[ 	]*vinsertf64x2 zmm30\{k7\},zmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 18 f4 ab[ 	]*vinsertf64x2 zmm30\{k7\}\{z\},zmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 18 f4 7b[ 	]*vinsertf64x2 zmm30,zmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 31 7b[ 	]*vinsertf64x2 zmm30,zmm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 18 b4 f0 23 01 00 00 7b[ 	]*vinsertf64x2 zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 72 7f 7b[ 	]*vinsertf64x2 zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 72 80 7b[ 	]*vinsertf64x2 zmm30,zmm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 zmm30,zmm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 1a f4 ab[ 	]*vinsertf32x8 zmm30,zmm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 1a f4 ab[ 	]*vinsertf32x8 zmm30\{k7\},zmm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 1a f4 ab[ 	]*vinsertf32x8 zmm30\{k7\}\{z\},zmm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 1a f4 7b[ 	]*vinsertf32x8 zmm30,zmm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a 31 7b[ 	]*vinsertf32x8 zmm30,zmm29,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 1a b4 f0 23 01 00 00 7b[ 	]*vinsertf32x8 zmm30,zmm29,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a 72 7f 7b[ 	]*vinsertf32x8 zmm30,zmm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a b2 00 10 00 00 7b[ 	]*vinsertf32x8 zmm30,zmm29,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a 72 80 7b[ 	]*vinsertf32x8 zmm30,zmm29,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a b2 e0 ef ff ff 7b[ 	]*vinsertf32x8 zmm30,zmm29,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 38 f4 ab[ 	]*vinserti64x2 zmm30,zmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 38 f4 ab[ 	]*vinserti64x2 zmm30\{k7\},zmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 38 f4 ab[ 	]*vinserti64x2 zmm30\{k7\}\{z\},zmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 38 f4 7b[ 	]*vinserti64x2 zmm30,zmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 31 7b[ 	]*vinserti64x2 zmm30,zmm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 38 b4 f0 23 01 00 00 7b[ 	]*vinserti64x2 zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 72 7f 7b[ 	]*vinserti64x2 zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 72 80 7b[ 	]*vinserti64x2 zmm30,zmm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 zmm30,zmm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 3a f4 ab[ 	]*vinserti32x8 zmm30,zmm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 3a f4 ab[ 	]*vinserti32x8 zmm30\{k7\},zmm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 3a f4 ab[ 	]*vinserti32x8 zmm30\{k7\}\{z\},zmm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 3a f4 7b[ 	]*vinserti32x8 zmm30,zmm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a 31 7b[ 	]*vinserti32x8 zmm30,zmm29,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 3a b4 f0 23 01 00 00 7b[ 	]*vinserti32x8 zmm30,zmm29,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a 72 7f 7b[ 	]*vinserti32x8 zmm30,zmm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a b2 00 10 00 00 7b[ 	]*vinserti32x8 zmm30,zmm29,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a 72 80 7b[ 	]*vinserti32x8 zmm30,zmm29,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a b2 e0 ef ff ff 7b[ 	]*vinserti32x8 zmm30,zmm29,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 59 f7[ 	]*vbroadcasti32x2 zmm30,xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 59 f7[ 	]*vbroadcasti32x2 zmm30\{k7\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 59 f7[ 	]*vbroadcasti32x2 zmm30\{k7\}\{z\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 31[ 	]*vbroadcasti32x2 zmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 59 b4 f0 23 01 00 00[ 	]*vbroadcasti32x2 zmm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 72 7f[ 	]*vbroadcasti32x2 zmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 zmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 72 80[ 	]*vbroadcasti32x2 zmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 zmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 e8 ab[ 	]*vpextrd eax,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 e8 7b[ 	]*vpextrd eax,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 ed 7b[ 	]*vpextrd ebp,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7d 08 16 ed 7b[ 	]*vpextrd r13d,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 29 7b[ 	]*vpextrd DWORD PTR \[rcx\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 08 16 ac f0 23 01 00 00 7b[ 	]*vpextrd DWORD PTR \[rax\+r14\*8\+0x123\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 6a 7f 7b[ 	]*vpextrd DWORD PTR \[rdx\+0x1fc\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 aa 00 02 00 00 7b[ 	]*vpextrd DWORD PTR \[rdx\+0x200\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 6a 80 7b[ 	]*vpextrd DWORD PTR \[rdx-0x200\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 aa fc fd ff ff 7b[ 	]*vpextrd DWORD PTR \[rdx-0x204\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 e8 ab[ 	]*vpextrq rax,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 e8 7b[ 	]*vpextrq rax,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 fd 08 16 e8 7b[ 	]*vpextrq r8,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 29 7b[ 	]*vpextrq QWORD PTR \[rcx\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 16 ac f0 23 01 00 00 7b[ 	]*vpextrq QWORD PTR \[rax\+r14\*8\+0x123\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 6a 7f 7b[ 	]*vpextrq QWORD PTR \[rdx\+0x3f8\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 aa 00 04 00 00 7b[ 	]*vpextrq QWORD PTR \[rdx\+0x400\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 6a 80 7b[ 	]*vpextrq QWORD PTR \[rdx-0x400\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 aa f8 fb ff ff 7b[ 	]*vpextrq QWORD PTR \[rdx-0x408\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 f0 ab[ 	]*vpinsrd xmm30,xmm29,eax,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 f0 7b[ 	]*vpinsrd xmm30,xmm29,eax,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 f5 7b[ 	]*vpinsrd xmm30,xmm29,ebp,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 15 00 22 f5 7b[ 	]*vpinsrd xmm30,xmm29,r13d,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 31 7b[ 	]*vpinsrd xmm30,xmm29,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 22 b4 f0 23 01 00 00 7b[ 	]*vpinsrd xmm30,xmm29,DWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 72 7f 7b[ 	]*vpinsrd xmm30,xmm29,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 b2 00 02 00 00 7b[ 	]*vpinsrd xmm30,xmm29,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 72 80 7b[ 	]*vpinsrd xmm30,xmm29,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 b2 fc fd ff ff 7b[ 	]*vpinsrd xmm30,xmm29,DWORD PTR \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 f0 ab[ 	]*vpinsrq xmm30,xmm29,rax,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 f0 7b[ 	]*vpinsrq xmm30,xmm29,rax,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 95 00 22 f0 7b[ 	]*vpinsrq xmm30,xmm29,r8,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 31 7b[ 	]*vpinsrq xmm30,xmm29,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 22 b4 f0 23 01 00 00 7b[ 	]*vpinsrq xmm30,xmm29,QWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 72 7f 7b[ 	]*vpinsrq xmm30,xmm29,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 b2 00 04 00 00 7b[ 	]*vpinsrq xmm30,xmm29,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 72 80 7b[ 	]*vpinsrq xmm30,xmm29,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 b2 f8 fb ff ff 7b[ 	]*vpinsrq xmm30,xmm29,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 40 f4[ 	]*vpmullq zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 40 f4[ 	]*vpmullq zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 40 f4[ 	]*vpmullq zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 31[ 	]*vpmullq zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 40 b4 f0 23 01 00 00[ 	]*vpmullq zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 31[ 	]*vpmullq zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 72 7f[ 	]*vpmullq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 b2 00 20 00 00[ 	]*vpmullq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 72 80[ 	]*vpmullq zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 b2 c0 df ff ff[ 	]*vpmullq zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 72 7f[ 	]*vpmullq zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 b2 00 04 00 00[ 	]*vpmullq zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 72 80[ 	]*vpmullq zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 b2 f8 fb ff ff[ 	]*vpmullq zmm30,zmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 50 f4 ab[ 	]*vrangepd zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 50 f4 ab[ 	]*vrangepd zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 50 f4 ab[ 	]*vrangepd zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 50 f4 ab[ 	]*vrangepd zmm30,zmm29,zmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 50 f4 7b[ 	]*vrangepd zmm30,zmm29,zmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 50 f4 7b[ 	]*vrangepd zmm30,zmm29,zmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 31 7b[ 	]*vrangepd zmm30,zmm29,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 50 b4 f0 23 01 00 00 7b[ 	]*vrangepd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 31 7b[ 	]*vrangepd zmm30,zmm29,QWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 72 7f 7b[ 	]*vrangepd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 b2 00 20 00 00 7b[ 	]*vrangepd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 72 80 7b[ 	]*vrangepd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 b2 c0 df ff ff 7b[ 	]*vrangepd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 72 7f 7b[ 	]*vrangepd zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 b2 00 04 00 00 7b[ 	]*vrangepd zmm30,zmm29,QWORD BCST \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 72 80 7b[ 	]*vrangepd zmm30,zmm29,QWORD BCST \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 b2 f8 fb ff ff 7b[ 	]*vrangepd zmm30,zmm29,QWORD BCST \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 50 f4 ab[ 	]*vrangeps zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 50 f4 ab[ 	]*vrangeps zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 50 f4 ab[ 	]*vrangeps zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 50 f4 ab[ 	]*vrangeps zmm30,zmm29,zmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 50 f4 7b[ 	]*vrangeps zmm30,zmm29,zmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 50 f4 7b[ 	]*vrangeps zmm30,zmm29,zmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 31 7b[ 	]*vrangeps zmm30,zmm29,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 50 b4 f0 23 01 00 00 7b[ 	]*vrangeps zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 31 7b[ 	]*vrangeps zmm30,zmm29,DWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 72 7f 7b[ 	]*vrangeps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 b2 00 20 00 00 7b[ 	]*vrangeps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 72 80 7b[ 	]*vrangeps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 b2 c0 df ff ff 7b[ 	]*vrangeps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 72 7f 7b[ 	]*vrangeps zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 b2 00 02 00 00 7b[ 	]*vrangeps zmm30,zmm29,DWORD BCST \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 72 80 7b[ 	]*vrangeps zmm30,zmm29,DWORD BCST \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 b2 fc fd ff ff 7b[ 	]*vrangeps zmm30,zmm29,DWORD BCST \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 51 f4 ab[ 	]*vrangesd xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 51 f4 ab[ 	]*vrangesd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 51 f4 ab[ 	]*vrangesd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 51 f4 ab[ 	]*vrangesd xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 51 f4 7b[ 	]*vrangesd xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 51 f4 7b[ 	]*vrangesd xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 31 7b[ 	]*vrangesd xmm30,xmm29,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 51 b4 f0 23 01 00 00 7b[ 	]*vrangesd xmm30,xmm29,QWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 72 7f 7b[ 	]*vrangesd xmm30,xmm29,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 b2 00 04 00 00 7b[ 	]*vrangesd xmm30,xmm29,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 72 80 7b[ 	]*vrangesd xmm30,xmm29,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 b2 f8 fb ff ff 7b[ 	]*vrangesd xmm30,xmm29,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 51 f4 ab[ 	]*vrangess xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 51 f4 ab[ 	]*vrangess xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 51 f4 ab[ 	]*vrangess xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 51 f4 ab[ 	]*vrangess xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 51 f4 7b[ 	]*vrangess xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 51 f4 7b[ 	]*vrangess xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 31 7b[ 	]*vrangess xmm30,xmm29,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 51 b4 f0 23 01 00 00 7b[ 	]*vrangess xmm30,xmm29,DWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 72 7f 7b[ 	]*vrangess xmm30,xmm29,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 b2 00 02 00 00 7b[ 	]*vrangess xmm30,xmm29,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 72 80 7b[ 	]*vrangess xmm30,xmm29,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 b2 fc fd ff ff 7b[ 	]*vrangess xmm30,xmm29,DWORD PTR \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 54 f4[ 	]*vandpd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 54 f4[ 	]*vandpd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 54 f4[ 	]*vandpd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 31[ 	]*vandpd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 54 b4 f0 23 01 00 00[ 	]*vandpd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 31[ 	]*vandpd zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 72 7f[ 	]*vandpd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 b2 00 20 00 00[ 	]*vandpd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 72 80[ 	]*vandpd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 b2 c0 df ff ff[ 	]*vandpd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 72 7f[ 	]*vandpd zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 b2 00 04 00 00[ 	]*vandpd zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 72 80[ 	]*vandpd zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 b2 f8 fb ff ff[ 	]*vandpd zmm30,zmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 54 f4[ 	]*vandps zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 54 f4[ 	]*vandps zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 54 f4[ 	]*vandps zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 31[ 	]*vandps zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 54 b4 f0 23 01 00 00[ 	]*vandps zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 31[ 	]*vandps zmm30,zmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 72 7f[ 	]*vandps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 b2 00 20 00 00[ 	]*vandps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 72 80[ 	]*vandps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 b2 c0 df ff ff[ 	]*vandps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 72 7f[ 	]*vandps zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 b2 00 02 00 00[ 	]*vandps zmm30,zmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 72 80[ 	]*vandps zmm30,zmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 b2 fc fd ff ff[ 	]*vandps zmm30,zmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 55 f4[ 	]*vandnpd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 55 f4[ 	]*vandnpd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 55 f4[ 	]*vandnpd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 31[ 	]*vandnpd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 55 b4 f0 23 01 00 00[ 	]*vandnpd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 31[ 	]*vandnpd zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 72 7f[ 	]*vandnpd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 b2 00 20 00 00[ 	]*vandnpd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 72 80[ 	]*vandnpd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 b2 c0 df ff ff[ 	]*vandnpd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 72 7f[ 	]*vandnpd zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 b2 00 04 00 00[ 	]*vandnpd zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 72 80[ 	]*vandnpd zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 b2 f8 fb ff ff[ 	]*vandnpd zmm30,zmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 55 f4[ 	]*vandnps zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 55 f4[ 	]*vandnps zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 55 f4[ 	]*vandnps zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 31[ 	]*vandnps zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 55 b4 f0 23 01 00 00[ 	]*vandnps zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 31[ 	]*vandnps zmm30,zmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 72 7f[ 	]*vandnps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 b2 00 20 00 00[ 	]*vandnps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 72 80[ 	]*vandnps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 b2 c0 df ff ff[ 	]*vandnps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 72 7f[ 	]*vandnps zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 b2 00 02 00 00[ 	]*vandnps zmm30,zmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 72 80[ 	]*vandnps zmm30,zmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 b2 fc fd ff ff[ 	]*vandnps zmm30,zmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 56 f4[ 	]*vorpd  zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 56 f4[ 	]*vorpd  zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 56 f4[ 	]*vorpd  zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 31[ 	]*vorpd  zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 56 b4 f0 23 01 00 00[ 	]*vorpd  zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 31[ 	]*vorpd  zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 72 7f[ 	]*vorpd  zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 b2 00 20 00 00[ 	]*vorpd  zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 72 80[ 	]*vorpd  zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 b2 c0 df ff ff[ 	]*vorpd  zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 72 7f[ 	]*vorpd  zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 b2 00 04 00 00[ 	]*vorpd  zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 72 80[ 	]*vorpd  zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 b2 f8 fb ff ff[ 	]*vorpd  zmm30,zmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 56 f4[ 	]*vorps  zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 56 f4[ 	]*vorps  zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 56 f4[ 	]*vorps  zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 31[ 	]*vorps  zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 56 b4 f0 23 01 00 00[ 	]*vorps  zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 31[ 	]*vorps  zmm30,zmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 72 7f[ 	]*vorps  zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 b2 00 20 00 00[ 	]*vorps  zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 72 80[ 	]*vorps  zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 b2 c0 df ff ff[ 	]*vorps  zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 72 7f[ 	]*vorps  zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 b2 00 02 00 00[ 	]*vorps  zmm30,zmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 72 80[ 	]*vorps  zmm30,zmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 b2 fc fd ff ff[ 	]*vorps  zmm30,zmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 57 f4[ 	]*vxorpd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 57 f4[ 	]*vxorpd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 57 f4[ 	]*vxorpd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 31[ 	]*vxorpd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 57 b4 f0 23 01 00 00[ 	]*vxorpd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 31[ 	]*vxorpd zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 72 7f[ 	]*vxorpd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 b2 00 20 00 00[ 	]*vxorpd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 72 80[ 	]*vxorpd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 b2 c0 df ff ff[ 	]*vxorpd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 72 7f[ 	]*vxorpd zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 b2 00 04 00 00[ 	]*vxorpd zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 72 80[ 	]*vxorpd zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 b2 f8 fb ff ff[ 	]*vxorpd zmm30,zmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 57 f4[ 	]*vxorps zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 57 f4[ 	]*vxorps zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 57 f4[ 	]*vxorps zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 31[ 	]*vxorps zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 57 b4 f0 23 01 00 00[ 	]*vxorps zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 31[ 	]*vxorps zmm30,zmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 72 7f[ 	]*vxorps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 b2 00 20 00 00[ 	]*vxorps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 72 80[ 	]*vxorps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 b2 c0 df ff ff[ 	]*vxorps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 72 7f[ 	]*vxorps zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 b2 00 02 00 00[ 	]*vxorps zmm30,zmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 72 80[ 	]*vxorps zmm30,zmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 b2 fc fd ff ff[ 	]*vxorps zmm30,zmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 56 f5 ab[ 	]*vreducepd zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 4f 56 f5 ab[ 	]*vreducepd zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd cf 56 f5 ab[ 	]*vreducepd zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 18 56 f5 ab[ 	]*vreducepd zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 56 f5 7b[ 	]*vreducepd zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 18 56 f5 7b[ 	]*vreducepd zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 31 7b[ 	]*vreducepd zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 48 56 b4 f0 23 01 00 00 7b[ 	]*vreducepd zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 31 7b[ 	]*vreducepd zmm30,QWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 72 7f 7b[ 	]*vreducepd zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 b2 00 20 00 00 7b[ 	]*vreducepd zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 72 80 7b[ 	]*vreducepd zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 b2 c0 df ff ff 7b[ 	]*vreducepd zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 72 7f 7b[ 	]*vreducepd zmm30,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 b2 00 04 00 00 7b[ 	]*vreducepd zmm30,QWORD BCST \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 72 80 7b[ 	]*vreducepd zmm30,QWORD BCST \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 b2 f8 fb ff ff 7b[ 	]*vreducepd zmm30,QWORD BCST \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 56 f5 ab[ 	]*vreduceps zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 4f 56 f5 ab[ 	]*vreduceps zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d cf 56 f5 ab[ 	]*vreduceps zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 18 56 f5 ab[ 	]*vreduceps zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 56 f5 7b[ 	]*vreduceps zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 18 56 f5 7b[ 	]*vreduceps zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 31 7b[ 	]*vreduceps zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 48 56 b4 f0 23 01 00 00 7b[ 	]*vreduceps zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 31 7b[ 	]*vreduceps zmm30,DWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 72 7f 7b[ 	]*vreduceps zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 b2 00 20 00 00 7b[ 	]*vreduceps zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 72 80 7b[ 	]*vreduceps zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 b2 c0 df ff ff 7b[ 	]*vreduceps zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 72 7f 7b[ 	]*vreduceps zmm30,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 b2 00 02 00 00 7b[ 	]*vreduceps zmm30,DWORD BCST \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 72 80 7b[ 	]*vreduceps zmm30,DWORD BCST \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 b2 fc fd ff ff 7b[ 	]*vreduceps zmm30,DWORD BCST \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 57 f4 ab[ 	]*vreducesd xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 57 f4 ab[ 	]*vreducesd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 57 f4 ab[ 	]*vreducesd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 57 f4 ab[ 	]*vreducesd xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 57 f4 7b[ 	]*vreducesd xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 57 f4 7b[ 	]*vreducesd xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 31 7b[ 	]*vreducesd xmm30,xmm29,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 57 b4 f0 23 01 00 00 7b[ 	]*vreducesd xmm30,xmm29,QWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 72 7f 7b[ 	]*vreducesd xmm30,xmm29,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 b2 00 04 00 00 7b[ 	]*vreducesd xmm30,xmm29,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 72 80 7b[ 	]*vreducesd xmm30,xmm29,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 b2 f8 fb ff ff 7b[ 	]*vreducesd xmm30,xmm29,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 57 f4 ab[ 	]*vreducess xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 57 f4 ab[ 	]*vreducess xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 57 f4 ab[ 	]*vreducess xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 57 f4 ab[ 	]*vreducess xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 57 f4 7b[ 	]*vreducess xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 57 f4 7b[ 	]*vreducess xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 31 7b[ 	]*vreducess xmm30,xmm29,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 57 b4 f0 23 01 00 00 7b[ 	]*vreducess xmm30,xmm29,DWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 72 7f 7b[ 	]*vreducess xmm30,xmm29,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 b2 00 02 00 00 7b[ 	]*vreducess xmm30,xmm29,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 72 80 7b[ 	]*vreducess xmm30,xmm29,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 b2 fc fd ff ff 7b[ 	]*vreducess xmm30,xmm29,DWORD PTR \[rdx-0x204\],0x7b
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
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 29[ 	]*kmovb  k5,BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*c4 a1 79 90 ac f0 23 01 00 00[ 	]*kmovb  k5,BYTE PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*c5 f9 91 29[ 	]*kmovb  BYTE PTR \[rcx\],k5
[ 	]*[a-f0-9]+:[ 	]*c4 a1 79 91 ac f0 23 01 00 00[ 	]*kmovb  BYTE PTR \[rax\+r14\*8\+0x123\],k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 e8[ 	]*kmovb  k5,eax
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 ed[ 	]*kmovb  k5,ebp
[ 	]*[a-f0-9]+:[ 	]*c4 c1 79 92 ed[ 	]*kmovb  k5,r13d
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 c5[ 	]*kmovb  eax,k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 ed[ 	]*kmovb  ebp,k5
[ 	]*[a-f0-9]+:[ 	]*c5 79 93 ed[ 	]*kmovb  r13d,k5
[ 	]*[a-f0-9]+:[ 	]*c5 cc 4a ef[ 	]*kaddw  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 cd 4a ef[ 	]*kaddb  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 31 ab[ 	]*vextractf64x2 XMMWORD PTR \[rcx\],zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 4f 19 31 ab[ 	]*vextractf64x2 XMMWORD PTR \[rcx\]\{k7\},zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 31 7b[ 	]*vextractf64x2 XMMWORD PTR \[rcx\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 48 19 b4 f0 23 01 00 00 7b[ 	]*vextractf64x2 XMMWORD PTR \[rax\+r14\*8\+0x123\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 72 7f 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx\+0x7f0\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 b2 00 08 00 00 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx\+0x800\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 72 80 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx-0x800\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 b2 f0 f7 ff ff 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx-0x810\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 31 ab[ 	]*vextractf32x8 YMMWORD PTR \[rcx\],zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 4f 1b 31 ab[ 	]*vextractf32x8 YMMWORD PTR \[rcx\]\{k7\},zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 31 7b[ 	]*vextractf32x8 YMMWORD PTR \[rcx\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 48 1b b4 f0 23 01 00 00 7b[ 	]*vextractf32x8 YMMWORD PTR \[rax\+r14\*8\+0x123\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 72 7f 7b[ 	]*vextractf32x8 YMMWORD PTR \[rdx\+0xfe0\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b b2 00 10 00 00 7b[ 	]*vextractf32x8 YMMWORD PTR \[rdx\+0x1000\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 72 80 7b[ 	]*vextractf32x8 YMMWORD PTR \[rdx-0x1000\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b b2 e0 ef ff ff 7b[ 	]*vextractf32x8 YMMWORD PTR \[rdx-0x1020\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 31 ab[ 	]*vextracti64x2 XMMWORD PTR \[rcx\],zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 4f 39 31 ab[ 	]*vextracti64x2 XMMWORD PTR \[rcx\]\{k7\},zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 31 7b[ 	]*vextracti64x2 XMMWORD PTR \[rcx\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 48 39 b4 f0 23 01 00 00 7b[ 	]*vextracti64x2 XMMWORD PTR \[rax\+r14\*8\+0x123\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 72 7f 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx\+0x7f0\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 b2 00 08 00 00 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx\+0x800\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 72 80 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx-0x800\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 b2 f0 f7 ff ff 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx-0x810\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 31 ab[ 	]*vextracti32x8 YMMWORD PTR \[rcx\],zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 4f 3b 31 ab[ 	]*vextracti32x8 YMMWORD PTR \[rcx\]\{k7\},zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 31 7b[ 	]*vextracti32x8 YMMWORD PTR \[rcx\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 48 3b b4 f0 23 01 00 00 7b[ 	]*vextracti32x8 YMMWORD PTR \[rax\+r14\*8\+0x123\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 72 7f 7b[ 	]*vextracti32x8 YMMWORD PTR \[rdx\+0xfe0\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b b2 00 10 00 00 7b[ 	]*vextracti32x8 YMMWORD PTR \[rdx\+0x1000\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 72 80 7b[ 	]*vextracti32x8 YMMWORD PTR \[rdx-0x1000\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b b2 e0 ef ff ff 7b[ 	]*vextracti32x8 YMMWORD PTR \[rdx-0x1020\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 7a f5[ 	]*vcvttpd2qq zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 7a f5[ 	]*vcvttpd2qq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 7a f5[ 	]*vcvttpd2qq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 7a f5[ 	]*vcvttpd2qq zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a 31[ 	]*vcvttpd2qq zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 7a b4 f0 23 01 00 00[ 	]*vcvttpd2qq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a 31[ 	]*vcvttpd2qq zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a 72 7f[ 	]*vcvttpd2qq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a b2 00 20 00 00[ 	]*vcvttpd2qq zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a 72 80[ 	]*vcvttpd2qq zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a b2 c0 df ff ff[ 	]*vcvttpd2qq zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a 72 7f[ 	]*vcvttpd2qq zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a b2 00 04 00 00[ 	]*vcvttpd2qq zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a 72 80[ 	]*vcvttpd2qq zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 78 f5[ 	]*vcvttpd2uqq zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 78 f5[ 	]*vcvttpd2uqq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 78 f5[ 	]*vcvttpd2uqq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 78 f5[ 	]*vcvttpd2uqq zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 31[ 	]*vcvttpd2uqq zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 78 b4 f0 23 01 00 00[ 	]*vcvttpd2uqq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 31[ 	]*vcvttpd2uqq zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 72 7f[ 	]*vcvttpd2uqq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 b2 00 20 00 00[ 	]*vcvttpd2uqq zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 72 80[ 	]*vcvttpd2uqq zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 b2 c0 df ff ff[ 	]*vcvttpd2uqq zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 72 7f[ 	]*vcvttpd2uqq zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 b2 00 04 00 00[ 	]*vcvttpd2uqq zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 72 80[ 	]*vcvttpd2uqq zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 7a f5[ 	]*vcvttps2qq zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 7a f5[ 	]*vcvttps2qq zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 7a f5[ 	]*vcvttps2qq zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 7a f5[ 	]*vcvttps2qq zmm30,ymm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a 31[ 	]*vcvttps2qq zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 7a b4 f0 23 01 00 00[ 	]*vcvttps2qq zmm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a 31[ 	]*vcvttps2qq zmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a 72 7f[ 	]*vcvttps2qq zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a b2 00 10 00 00[ 	]*vcvttps2qq zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a 72 80[ 	]*vcvttps2qq zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a b2 e0 ef ff ff[ 	]*vcvttps2qq zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a 72 7f[ 	]*vcvttps2qq zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a b2 00 02 00 00[ 	]*vcvttps2qq zmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a 72 80[ 	]*vcvttps2qq zmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a b2 fc fd ff ff[ 	]*vcvttps2qq zmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 78 f5[ 	]*vcvttps2uqq zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 78 f5[ 	]*vcvttps2uqq zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 78 f5[ 	]*vcvttps2uqq zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 78 f5[ 	]*vcvttps2uqq zmm30,ymm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 31[ 	]*vcvttps2uqq zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 78 b4 f0 23 01 00 00[ 	]*vcvttps2uqq zmm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 31[ 	]*vcvttps2uqq zmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 72 7f[ 	]*vcvttps2uqq zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 b2 00 10 00 00[ 	]*vcvttps2uqq zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 72 80[ 	]*vcvttps2uqq zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 b2 e0 ef ff ff[ 	]*vcvttps2uqq zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 72 7f[ 	]*vcvttps2uqq zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 b2 00 02 00 00[ 	]*vcvttps2uqq zmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 72 80[ 	]*vcvttps2uqq zmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 b2 fc fd ff ff[ 	]*vcvttps2uqq zmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 48 39 ee[ 	]*vpmovd2m k5,zmm30
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 48 39 ee[ 	]*vpmovq2m k5,zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 38 f5[ 	]*vpmovm2d zmm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 48 38 f5[ 	]*vpmovm2q zmm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b 31[ 	]*vbroadcastf32x8 zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 1b 31[ 	]*vbroadcastf32x8 zmm30\{k7\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 1b 31[ 	]*vbroadcastf32x8 zmm30\{k7\}\{z\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 1b b4 f0 34 12 00 00[ 	]*vbroadcastf32x8 zmm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b 72 7f[ 	]*vbroadcastf32x8 zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b b2 00 10 00 00[ 	]*vbroadcastf32x8 zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b 72 80[ 	]*vbroadcastf32x8 zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 1b b2 e0 ef ff ff[ 	]*vbroadcastf32x8 zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a 31[ 	]*vbroadcastf64x2 zmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 1a 31[ 	]*vbroadcastf64x2 zmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd cf 1a 31[ 	]*vbroadcastf64x2 zmm30\{k7\}\{z\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 1a b4 f0 34 12 00 00[ 	]*vbroadcastf64x2 zmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a 72 7f[ 	]*vbroadcastf64x2 zmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a b2 00 08 00 00[ 	]*vbroadcastf64x2 zmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a 72 80[ 	]*vbroadcastf64x2 zmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 1a b2 f0 f7 ff ff[ 	]*vbroadcastf64x2 zmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b 31[ 	]*vbroadcasti32x8 zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 4f 5b 31[ 	]*vbroadcasti32x8 zmm30\{k7\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d cf 5b 31[ 	]*vbroadcasti32x8 zmm30\{k7\}\{z\},YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 5b b4 f0 34 12 00 00[ 	]*vbroadcasti32x8 zmm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b 72 7f[ 	]*vbroadcasti32x8 zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b b2 00 10 00 00[ 	]*vbroadcasti32x8 zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b 72 80[ 	]*vbroadcasti32x8 zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 5b b2 e0 ef ff ff[ 	]*vbroadcasti32x8 zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a 31[ 	]*vbroadcasti64x2 zmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 4f 5a 31[ 	]*vbroadcasti64x2 zmm30\{k7\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd cf 5a 31[ 	]*vbroadcasti64x2 zmm30\{k7\}\{z\},XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 5a b4 f0 34 12 00 00[ 	]*vbroadcasti64x2 zmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a 72 7f[ 	]*vbroadcasti64x2 zmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a b2 00 08 00 00[ 	]*vbroadcasti64x2 zmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a 72 80[ 	]*vbroadcasti64x2 zmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 5a b2 f0 f7 ff ff[ 	]*vbroadcasti64x2 zmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 19 f7[ 	]*vbroadcastf32x2 zmm30,xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 19 f7[ 	]*vbroadcastf32x2 zmm30\{k7\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 19 f7[ 	]*vbroadcastf32x2 zmm30\{k7\}\{z\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 31[ 	]*vbroadcastf32x2 zmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 19 b4 f0 34 12 00 00[ 	]*vbroadcastf32x2 zmm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 72 7f[ 	]*vbroadcastf32x2 zmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 b2 00 04 00 00[ 	]*vbroadcastf32x2 zmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 72 80[ 	]*vbroadcastf32x2 zmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 19 b2 f8 fb ff ff[ 	]*vbroadcastf32x2 zmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 7b f5[ 	]*vcvtpd2qq zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 7b f5[ 	]*vcvtpd2qq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 7b f5[ 	]*vcvtpd2qq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 7b f5[ 	]*vcvtpd2qq zmm30,zmm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 58 7b f5[ 	]*vcvtpd2qq zmm30,zmm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 7b f5[ 	]*vcvtpd2qq zmm30,zmm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 78 7b f5[ 	]*vcvtpd2qq zmm30,zmm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b 31[ 	]*vcvtpd2qq zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 7b b4 f0 34 12 00 00[ 	]*vcvtpd2qq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b 31[ 	]*vcvtpd2qq zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b 72 7f[ 	]*vcvtpd2qq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b b2 00 20 00 00[ 	]*vcvtpd2qq zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b 72 80[ 	]*vcvtpd2qq zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7b b2 c0 df ff ff[ 	]*vcvtpd2qq zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b 72 7f[ 	]*vcvtpd2qq zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b b2 00 04 00 00[ 	]*vcvtpd2qq zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b 72 80[ 	]*vcvtpd2qq zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7b b2 f8 fb ff ff[ 	]*vcvtpd2qq zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 79 f5[ 	]*vcvtpd2uqq zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 79 f5[ 	]*vcvtpd2uqq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 79 f5[ 	]*vcvtpd2uqq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 79 f5[ 	]*vcvtpd2uqq zmm30,zmm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 58 79 f5[ 	]*vcvtpd2uqq zmm30,zmm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 38 79 f5[ 	]*vcvtpd2uqq zmm30,zmm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 78 79 f5[ 	]*vcvtpd2uqq zmm30,zmm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 31[ 	]*vcvtpd2uqq zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 79 b4 f0 34 12 00 00[ 	]*vcvtpd2uqq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 31[ 	]*vcvtpd2uqq zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 72 7f[ 	]*vcvtpd2uqq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 b2 00 20 00 00[ 	]*vcvtpd2uqq zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 72 80[ 	]*vcvtpd2uqq zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 79 b2 c0 df ff ff[ 	]*vcvtpd2uqq zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 72 7f[ 	]*vcvtpd2uqq zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 b2 00 04 00 00[ 	]*vcvtpd2uqq zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 72 80[ 	]*vcvtpd2uqq zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 79 b2 f8 fb ff ff[ 	]*vcvtpd2uqq zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 7b f5[ 	]*vcvtps2qq zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 7b f5[ 	]*vcvtps2qq zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 7b f5[ 	]*vcvtps2qq zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 7b f5[ 	]*vcvtps2qq zmm30,ymm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 58 7b f5[ 	]*vcvtps2qq zmm30,ymm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 38 7b f5[ 	]*vcvtps2qq zmm30,ymm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 78 7b f5[ 	]*vcvtps2qq zmm30,ymm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b 31[ 	]*vcvtps2qq zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 7b b4 f0 34 12 00 00[ 	]*vcvtps2qq zmm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b 31[ 	]*vcvtps2qq zmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b 72 7f[ 	]*vcvtps2qq zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b b2 00 10 00 00[ 	]*vcvtps2qq zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b 72 80[ 	]*vcvtps2qq zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7b b2 e0 ef ff ff[ 	]*vcvtps2qq zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b 72 7f[ 	]*vcvtps2qq zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b b2 00 02 00 00[ 	]*vcvtps2qq zmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b 72 80[ 	]*vcvtps2qq zmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b b2 fc fd ff ff[ 	]*vcvtps2qq zmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7b 72 7f[ 	]*vcvtps2qq zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 79 f5[ 	]*vcvtps2uqq zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 79 f5[ 	]*vcvtps2uqq zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 79 f5[ 	]*vcvtps2uqq zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 79 f5[ 	]*vcvtps2uqq zmm30,ymm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 58 79 f5[ 	]*vcvtps2uqq zmm30,ymm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 38 79 f5[ 	]*vcvtps2uqq zmm30,ymm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 78 79 f5[ 	]*vcvtps2uqq zmm30,ymm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 31[ 	]*vcvtps2uqq zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 79 b4 f0 34 12 00 00[ 	]*vcvtps2uqq zmm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 31[ 	]*vcvtps2uqq zmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 72 7f[ 	]*vcvtps2uqq zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 b2 00 10 00 00[ 	]*vcvtps2uqq zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 72 80[ 	]*vcvtps2uqq zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 79 b2 e0 ef ff ff[ 	]*vcvtps2uqq zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 72 7f[ 	]*vcvtps2uqq zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 b2 00 02 00 00[ 	]*vcvtps2uqq zmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 72 80[ 	]*vcvtps2uqq zmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 b2 fc fd ff ff[ 	]*vcvtps2uqq zmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 79 72 7f[ 	]*vcvtps2uqq zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 e6 f5[ 	]*vcvtqq2pd zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 4f e6 f5[ 	]*vcvtqq2pd zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe cf e6 f5[ 	]*vcvtqq2pd zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 18 e6 f5[ 	]*vcvtqq2pd zmm30,zmm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 58 e6 f5[ 	]*vcvtqq2pd zmm30,zmm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 38 e6 f5[ 	]*vcvtqq2pd zmm30,zmm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 78 e6 f5[ 	]*vcvtqq2pd zmm30,zmm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 31[ 	]*vcvtqq2pd zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 48 e6 b4 f0 34 12 00 00[ 	]*vcvtqq2pd zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 31[ 	]*vcvtqq2pd zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 72 7f[ 	]*vcvtqq2pd zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 b2 00 20 00 00[ 	]*vcvtqq2pd zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 72 80[ 	]*vcvtqq2pd zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 e6 b2 c0 df ff ff[ 	]*vcvtqq2pd zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 72 7f[ 	]*vcvtqq2pd zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 b2 00 04 00 00[ 	]*vcvtqq2pd zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 72 80[ 	]*vcvtqq2pd zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 e6 b2 f8 fb ff ff[ 	]*vcvtqq2pd zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 48 5b f5[ 	]*vcvtqq2ps ymm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 4f 5b f5[ 	]*vcvtqq2ps ymm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fc cf 5b f5[ 	]*vcvtqq2ps ymm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 18 5b f5[ 	]*vcvtqq2ps ymm30,zmm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 58 5b f5[ 	]*vcvtqq2ps ymm30,zmm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 38 5b f5[ 	]*vcvtqq2ps ymm30,zmm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fc 78 5b f5[ 	]*vcvtqq2ps ymm30,zmm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b 31[ 	]*vcvtqq2ps ymm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fc 48 5b b4 f0 34 12 00 00[ 	]*vcvtqq2ps ymm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b 31[ 	]*vcvtqq2ps ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b 72 7f[ 	]*vcvtqq2ps ymm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b b2 00 20 00 00[ 	]*vcvtqq2ps ymm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b 72 80[ 	]*vcvtqq2ps ymm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 48 5b b2 c0 df ff ff[ 	]*vcvtqq2ps ymm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b 72 7f[ 	]*vcvtqq2ps ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b b2 00 04 00 00[ 	]*vcvtqq2ps ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b 72 80[ 	]*vcvtqq2ps ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fc 58 5b b2 f8 fb ff ff[ 	]*vcvtqq2ps ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 48 7a f5[ 	]*vcvtuqq2pd zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 4f 7a f5[ 	]*vcvtuqq2pd zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe cf 7a f5[ 	]*vcvtuqq2pd zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 18 7a f5[ 	]*vcvtuqq2pd zmm30,zmm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 58 7a f5[ 	]*vcvtuqq2pd zmm30,zmm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 38 7a f5[ 	]*vcvtuqq2pd zmm30,zmm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 fe 78 7a f5[ 	]*vcvtuqq2pd zmm30,zmm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a 31[ 	]*vcvtuqq2pd zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fe 48 7a b4 f0 34 12 00 00[ 	]*vcvtuqq2pd zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a 31[ 	]*vcvtuqq2pd zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a 72 7f[ 	]*vcvtuqq2pd zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a b2 00 20 00 00[ 	]*vcvtuqq2pd zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a 72 80[ 	]*vcvtuqq2pd zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 48 7a b2 c0 df ff ff[ 	]*vcvtuqq2pd zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a 72 7f[ 	]*vcvtuqq2pd zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a b2 00 04 00 00[ 	]*vcvtuqq2pd zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a 72 80[ 	]*vcvtuqq2pd zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fe 58 7a b2 f8 fb ff ff[ 	]*vcvtuqq2pd zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 48 7a f5[ 	]*vcvtuqq2ps ymm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 4f 7a f5[ 	]*vcvtuqq2ps ymm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff cf 7a f5[ 	]*vcvtuqq2ps ymm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 18 7a f5[ 	]*vcvtuqq2ps ymm30,zmm29\{rn-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 58 7a f5[ 	]*vcvtuqq2ps ymm30,zmm29\{ru-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 38 7a f5[ 	]*vcvtuqq2ps ymm30,zmm29\{rd-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 78 7a f5[ 	]*vcvtuqq2ps ymm30,zmm29\{rz-sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a 31[ 	]*vcvtuqq2ps ymm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 48 7a b4 f0 34 12 00 00[ 	]*vcvtuqq2ps ymm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a 31[ 	]*vcvtuqq2ps ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a 72 7f[ 	]*vcvtuqq2ps ymm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a b2 00 20 00 00[ 	]*vcvtuqq2ps ymm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a 72 80[ 	]*vcvtuqq2ps ymm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 48 7a b2 c0 df ff ff[ 	]*vcvtuqq2ps ymm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a 72 7f[ 	]*vcvtuqq2ps ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a b2 00 04 00 00[ 	]*vcvtuqq2ps ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a 72 80[ 	]*vcvtuqq2ps ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 58 7a b2 f8 fb ff ff[ 	]*vcvtuqq2ps ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 19 ee ab[ 	]*vextractf64x2 xmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 4f 19 ee ab[ 	]*vextractf64x2 xmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd cf 19 ee ab[ 	]*vextractf64x2 xmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 19 ee 7b[ 	]*vextractf64x2 xmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 1b ee ab[ 	]*vextractf32x8 ymm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 4f 1b ee ab[ 	]*vextractf32x8 ymm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d cf 1b ee ab[ 	]*vextractf32x8 ymm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 1b ee 7b[ 	]*vextractf32x8 ymm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 39 ee ab[ 	]*vextracti64x2 xmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 4f 39 ee ab[ 	]*vextracti64x2 xmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd cf 39 ee ab[ 	]*vextracti64x2 xmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 39 ee 7b[ 	]*vextracti64x2 xmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 3b ee ab[ 	]*vextracti32x8 ymm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 4f 3b ee ab[ 	]*vextracti32x8 ymm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d cf 3b ee ab[ 	]*vextracti32x8 ymm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 3b ee 7b[ 	]*vextracti32x8 ymm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 48 66 ee ab[ 	]*vfpclasspd k5,zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 4f 66 ee ab[ 	]*vfpclasspd k5\{k7\},zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 48 66 ee 7b[ 	]*vfpclasspd k5,zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 29 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 48 66 ac f0 34 12 00 00 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 29 7b[ 	]*vfpclasspd k5,QWORD BCST \[rcx\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 7f 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa 00 20 00 00 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 6a 80 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 48 66 aa c0 df ff ff 7b[ 	]*vfpclasspd k5,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 7f 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx\+0x3f8\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa 00 04 00 00 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx\+0x400\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 6a 80 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx-0x400\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 58 66 aa f8 fb ff ff 7b[ 	]*vfpclasspd k5,QWORD BCST \[rdx-0x408\]\{1to8\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 48 66 ee ab[ 	]*vfpclassps k5,zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 4f 66 ee ab[ 	]*vfpclassps k5\{k7\},zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 48 66 ee 7b[ 	]*vfpclassps k5,zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 29 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 48 66 ac f0 34 12 00 00 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 29 7b[ 	]*vfpclassps k5,DWORD BCST \[rcx\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 7f 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa 00 20 00 00 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 6a 80 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 48 66 aa c0 df ff ff 7b[ 	]*vfpclassps k5,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 7f 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx\+0x1fc\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa 00 02 00 00 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx\+0x200\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 6a 80 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx-0x200\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 58 66 aa fc fd ff ff 7b[ 	]*vfpclassps k5,DWORD BCST \[rdx-0x204\]\{1to16\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 67 ee ab[ 	]*vfpclasssd k5,xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 0f 67 ee ab[ 	]*vfpclasssd k5\{k7\},xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 fd 08 67 ee 7b[ 	]*vfpclasssd k5,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 29 7b[ 	]*vfpclasssd k5,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 fd 08 67 ac f0 34 12 00 00 7b[ 	]*vfpclasssd k5,QWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 6a 7f 7b[ 	]*vfpclasssd k5,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 aa 00 04 00 00 7b[ 	]*vfpclasssd k5,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 6a 80 7b[ 	]*vfpclasssd k5,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 fd 08 67 aa f8 fb ff ff 7b[ 	]*vfpclasssd k5,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 67 ee ab[ 	]*vfpclassss k5,xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 0f 67 ee ab[ 	]*vfpclassss k5\{k7\},xmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 7d 08 67 ee 7b[ 	]*vfpclassss k5,xmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 29 7b[ 	]*vfpclassss k5,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 7d 08 67 ac f0 34 12 00 00 7b[ 	]*vfpclassss k5,DWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 6a 7f 7b[ 	]*vfpclassss k5,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 aa 00 02 00 00 7b[ 	]*vfpclassss k5,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 6a 80 7b[ 	]*vfpclassss k5,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 7d 08 67 aa fc fd ff ff 7b[ 	]*vfpclassss k5,DWORD PTR \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 18 f4 ab[ 	]*vinsertf64x2 zmm30,zmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 18 f4 ab[ 	]*vinsertf64x2 zmm30\{k7\},zmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 18 f4 ab[ 	]*vinsertf64x2 zmm30\{k7\}\{z\},zmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 18 f4 7b[ 	]*vinsertf64x2 zmm30,zmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 31 7b[ 	]*vinsertf64x2 zmm30,zmm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 18 b4 f0 34 12 00 00 7b[ 	]*vinsertf64x2 zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 72 7f 7b[ 	]*vinsertf64x2 zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 b2 00 08 00 00 7b[ 	]*vinsertf64x2 zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 72 80 7b[ 	]*vinsertf64x2 zmm30,zmm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 18 b2 f0 f7 ff ff 7b[ 	]*vinsertf64x2 zmm30,zmm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 1a f4 ab[ 	]*vinsertf32x8 zmm30,zmm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 1a f4 ab[ 	]*vinsertf32x8 zmm30\{k7\},zmm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 1a f4 ab[ 	]*vinsertf32x8 zmm30\{k7\}\{z\},zmm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 1a f4 7b[ 	]*vinsertf32x8 zmm30,zmm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a 31 7b[ 	]*vinsertf32x8 zmm30,zmm29,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 1a b4 f0 34 12 00 00 7b[ 	]*vinsertf32x8 zmm30,zmm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a 72 7f 7b[ 	]*vinsertf32x8 zmm30,zmm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a b2 00 10 00 00 7b[ 	]*vinsertf32x8 zmm30,zmm29,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a 72 80 7b[ 	]*vinsertf32x8 zmm30,zmm29,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 1a b2 e0 ef ff ff 7b[ 	]*vinsertf32x8 zmm30,zmm29,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 38 f4 ab[ 	]*vinserti64x2 zmm30,zmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 38 f4 ab[ 	]*vinserti64x2 zmm30\{k7\},zmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 38 f4 ab[ 	]*vinserti64x2 zmm30\{k7\}\{z\},zmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 38 f4 7b[ 	]*vinserti64x2 zmm30,zmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 31 7b[ 	]*vinserti64x2 zmm30,zmm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 38 b4 f0 34 12 00 00 7b[ 	]*vinserti64x2 zmm30,zmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 72 7f 7b[ 	]*vinserti64x2 zmm30,zmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 b2 00 08 00 00 7b[ 	]*vinserti64x2 zmm30,zmm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 72 80 7b[ 	]*vinserti64x2 zmm30,zmm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 38 b2 f0 f7 ff ff 7b[ 	]*vinserti64x2 zmm30,zmm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 3a f4 ab[ 	]*vinserti32x8 zmm30,zmm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 3a f4 ab[ 	]*vinserti32x8 zmm30\{k7\},zmm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 3a f4 ab[ 	]*vinserti32x8 zmm30\{k7\}\{z\},zmm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 3a f4 7b[ 	]*vinserti32x8 zmm30,zmm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a 31 7b[ 	]*vinserti32x8 zmm30,zmm29,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 3a b4 f0 34 12 00 00 7b[ 	]*vinserti32x8 zmm30,zmm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a 72 7f 7b[ 	]*vinserti32x8 zmm30,zmm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a b2 00 10 00 00 7b[ 	]*vinserti32x8 zmm30,zmm29,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a 72 80 7b[ 	]*vinserti32x8 zmm30,zmm29,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 3a b2 e0 ef ff ff 7b[ 	]*vinserti32x8 zmm30,zmm29,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 59 f7[ 	]*vbroadcasti32x2 zmm30,xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 59 f7[ 	]*vbroadcasti32x2 zmm30\{k7\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 59 f7[ 	]*vbroadcasti32x2 zmm30\{k7\}\{z\},xmm31
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 31[ 	]*vbroadcasti32x2 zmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 59 b4 f0 34 12 00 00[ 	]*vbroadcasti32x2 zmm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 72 7f[ 	]*vbroadcasti32x2 zmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 b2 00 04 00 00[ 	]*vbroadcasti32x2 zmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 72 80[ 	]*vbroadcasti32x2 zmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 59 b2 f8 fb ff ff[ 	]*vbroadcasti32x2 zmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 e8 ab[ 	]*vpextrd eax,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 e8 7b[ 	]*vpextrd eax,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 ed 7b[ 	]*vpextrd ebp,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 7d 08 16 ed 7b[ 	]*vpextrd r13d,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 29 7b[ 	]*vpextrd DWORD PTR \[rcx\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 08 16 ac f0 34 12 00 00 7b[ 	]*vpextrd DWORD PTR \[rax\+r14\*8\+0x1234\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 6a 7f 7b[ 	]*vpextrd DWORD PTR \[rdx\+0x1fc\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 aa 00 02 00 00 7b[ 	]*vpextrd DWORD PTR \[rdx\+0x200\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 6a 80 7b[ 	]*vpextrd DWORD PTR \[rdx-0x200\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 08 16 aa fc fd ff ff 7b[ 	]*vpextrd DWORD PTR \[rdx-0x204\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 e8 ab[ 	]*vpextrq rax,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 e8 7b[ 	]*vpextrq rax,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 fd 08 16 e8 7b[ 	]*vpextrq r8,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 29 7b[ 	]*vpextrq QWORD PTR \[rcx\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 08 16 ac f0 34 12 00 00 7b[ 	]*vpextrq QWORD PTR \[rax\+r14\*8\+0x1234\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 6a 7f 7b[ 	]*vpextrq QWORD PTR \[rdx\+0x3f8\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 aa 00 04 00 00 7b[ 	]*vpextrq QWORD PTR \[rdx\+0x400\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 6a 80 7b[ 	]*vpextrq QWORD PTR \[rdx-0x400\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 08 16 aa f8 fb ff ff 7b[ 	]*vpextrq QWORD PTR \[rdx-0x408\],xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 f0 ab[ 	]*vpinsrd xmm30,xmm29,eax,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 f0 7b[ 	]*vpinsrd xmm30,xmm29,eax,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 f5 7b[ 	]*vpinsrd xmm30,xmm29,ebp,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 15 00 22 f5 7b[ 	]*vpinsrd xmm30,xmm29,r13d,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 31 7b[ 	]*vpinsrd xmm30,xmm29,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 22 b4 f0 34 12 00 00 7b[ 	]*vpinsrd xmm30,xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 72 7f 7b[ 	]*vpinsrd xmm30,xmm29,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 b2 00 02 00 00 7b[ 	]*vpinsrd xmm30,xmm29,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 72 80 7b[ 	]*vpinsrd xmm30,xmm29,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 22 b2 fc fd ff ff 7b[ 	]*vpinsrd xmm30,xmm29,DWORD PTR \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 f0 ab[ 	]*vpinsrq xmm30,xmm29,rax,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 f0 7b[ 	]*vpinsrq xmm30,xmm29,rax,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 43 95 00 22 f0 7b[ 	]*vpinsrq xmm30,xmm29,r8,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 31 7b[ 	]*vpinsrq xmm30,xmm29,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 22 b4 f0 34 12 00 00 7b[ 	]*vpinsrq xmm30,xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 72 7f 7b[ 	]*vpinsrq xmm30,xmm29,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 b2 00 04 00 00 7b[ 	]*vpinsrq xmm30,xmm29,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 72 80 7b[ 	]*vpinsrq xmm30,xmm29,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 22 b2 f8 fb ff ff 7b[ 	]*vpinsrq xmm30,xmm29,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 40 f4[ 	]*vpmullq zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 40 f4[ 	]*vpmullq zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 40 f4[ 	]*vpmullq zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 31[ 	]*vpmullq zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 40 b4 f0 34 12 00 00[ 	]*vpmullq zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 31[ 	]*vpmullq zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 72 7f[ 	]*vpmullq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 b2 00 20 00 00[ 	]*vpmullq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 72 80[ 	]*vpmullq zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 40 b2 c0 df ff ff[ 	]*vpmullq zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 72 7f[ 	]*vpmullq zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 b2 00 04 00 00[ 	]*vpmullq zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 72 80[ 	]*vpmullq zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 40 b2 f8 fb ff ff[ 	]*vpmullq zmm30,zmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 50 f4 ab[ 	]*vrangepd zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 50 f4 ab[ 	]*vrangepd zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 50 f4 ab[ 	]*vrangepd zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 50 f4 ab[ 	]*vrangepd zmm30,zmm29,zmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 50 f4 7b[ 	]*vrangepd zmm30,zmm29,zmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 50 f4 7b[ 	]*vrangepd zmm30,zmm29,zmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 31 7b[ 	]*vrangepd zmm30,zmm29,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 50 b4 f0 34 12 00 00 7b[ 	]*vrangepd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 31 7b[ 	]*vrangepd zmm30,zmm29,QWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 72 7f 7b[ 	]*vrangepd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 b2 00 20 00 00 7b[ 	]*vrangepd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 72 80 7b[ 	]*vrangepd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 50 b2 c0 df ff ff 7b[ 	]*vrangepd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 72 7f 7b[ 	]*vrangepd zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 b2 00 04 00 00 7b[ 	]*vrangepd zmm30,zmm29,QWORD BCST \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 72 80 7b[ 	]*vrangepd zmm30,zmm29,QWORD BCST \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 50 b2 f8 fb ff ff 7b[ 	]*vrangepd zmm30,zmm29,QWORD BCST \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 50 f4 ab[ 	]*vrangeps zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 47 50 f4 ab[ 	]*vrangeps zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 c7 50 f4 ab[ 	]*vrangeps zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 50 f4 ab[ 	]*vrangeps zmm30,zmm29,zmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 40 50 f4 7b[ 	]*vrangeps zmm30,zmm29,zmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 50 f4 7b[ 	]*vrangeps zmm30,zmm29,zmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 31 7b[ 	]*vrangeps zmm30,zmm29,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 40 50 b4 f0 34 12 00 00 7b[ 	]*vrangeps zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 31 7b[ 	]*vrangeps zmm30,zmm29,DWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 72 7f 7b[ 	]*vrangeps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 b2 00 20 00 00 7b[ 	]*vrangeps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 72 80 7b[ 	]*vrangeps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 40 50 b2 c0 df ff ff 7b[ 	]*vrangeps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 72 7f 7b[ 	]*vrangeps zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 b2 00 02 00 00 7b[ 	]*vrangeps zmm30,zmm29,DWORD BCST \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 72 80 7b[ 	]*vrangeps zmm30,zmm29,DWORD BCST \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 50 50 b2 fc fd ff ff 7b[ 	]*vrangeps zmm30,zmm29,DWORD BCST \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 51 f4 ab[ 	]*vrangesd xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 51 f4 ab[ 	]*vrangesd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 51 f4 ab[ 	]*vrangesd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 51 f4 ab[ 	]*vrangesd xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 51 f4 7b[ 	]*vrangesd xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 51 f4 7b[ 	]*vrangesd xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 31 7b[ 	]*vrangesd xmm30,xmm29,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 51 b4 f0 34 12 00 00 7b[ 	]*vrangesd xmm30,xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 72 7f 7b[ 	]*vrangesd xmm30,xmm29,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 b2 00 04 00 00 7b[ 	]*vrangesd xmm30,xmm29,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 72 80 7b[ 	]*vrangesd xmm30,xmm29,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 51 b2 f8 fb ff ff 7b[ 	]*vrangesd xmm30,xmm29,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 51 f4 ab[ 	]*vrangess xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 51 f4 ab[ 	]*vrangess xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 51 f4 ab[ 	]*vrangess xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 51 f4 ab[ 	]*vrangess xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 51 f4 7b[ 	]*vrangess xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 51 f4 7b[ 	]*vrangess xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 31 7b[ 	]*vrangess xmm30,xmm29,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 51 b4 f0 34 12 00 00 7b[ 	]*vrangess xmm30,xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 72 7f 7b[ 	]*vrangess xmm30,xmm29,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 b2 00 02 00 00 7b[ 	]*vrangess xmm30,xmm29,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 72 80 7b[ 	]*vrangess xmm30,xmm29,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 51 b2 fc fd ff ff 7b[ 	]*vrangess xmm30,xmm29,DWORD PTR \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 54 f4[ 	]*vandpd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 54 f4[ 	]*vandpd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 54 f4[ 	]*vandpd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 31[ 	]*vandpd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 54 b4 f0 34 12 00 00[ 	]*vandpd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 31[ 	]*vandpd zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 72 7f[ 	]*vandpd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 b2 00 20 00 00[ 	]*vandpd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 72 80[ 	]*vandpd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 54 b2 c0 df ff ff[ 	]*vandpd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 72 7f[ 	]*vandpd zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 b2 00 04 00 00[ 	]*vandpd zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 72 80[ 	]*vandpd zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 54 b2 f8 fb ff ff[ 	]*vandpd zmm30,zmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 54 f4[ 	]*vandps zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 54 f4[ 	]*vandps zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 54 f4[ 	]*vandps zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 31[ 	]*vandps zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 54 b4 f0 34 12 00 00[ 	]*vandps zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 31[ 	]*vandps zmm30,zmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 72 7f[ 	]*vandps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 b2 00 20 00 00[ 	]*vandps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 72 80[ 	]*vandps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 54 b2 c0 df ff ff[ 	]*vandps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 72 7f[ 	]*vandps zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 b2 00 02 00 00[ 	]*vandps zmm30,zmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 72 80[ 	]*vandps zmm30,zmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 54 b2 fc fd ff ff[ 	]*vandps zmm30,zmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 55 f4[ 	]*vandnpd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 55 f4[ 	]*vandnpd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 55 f4[ 	]*vandnpd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 31[ 	]*vandnpd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 55 b4 f0 34 12 00 00[ 	]*vandnpd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 31[ 	]*vandnpd zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 72 7f[ 	]*vandnpd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 b2 00 20 00 00[ 	]*vandnpd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 72 80[ 	]*vandnpd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 55 b2 c0 df ff ff[ 	]*vandnpd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 72 7f[ 	]*vandnpd zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 b2 00 04 00 00[ 	]*vandnpd zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 72 80[ 	]*vandnpd zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 55 b2 f8 fb ff ff[ 	]*vandnpd zmm30,zmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 55 f4[ 	]*vandnps zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 55 f4[ 	]*vandnps zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 55 f4[ 	]*vandnps zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 31[ 	]*vandnps zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 55 b4 f0 34 12 00 00[ 	]*vandnps zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 31[ 	]*vandnps zmm30,zmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 72 7f[ 	]*vandnps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 b2 00 20 00 00[ 	]*vandnps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 72 80[ 	]*vandnps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 55 b2 c0 df ff ff[ 	]*vandnps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 72 7f[ 	]*vandnps zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 b2 00 02 00 00[ 	]*vandnps zmm30,zmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 72 80[ 	]*vandnps zmm30,zmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 55 b2 fc fd ff ff[ 	]*vandnps zmm30,zmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 56 f4[ 	]*vorpd  zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 56 f4[ 	]*vorpd  zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 56 f4[ 	]*vorpd  zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 31[ 	]*vorpd  zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 56 b4 f0 34 12 00 00[ 	]*vorpd  zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 31[ 	]*vorpd  zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 72 7f[ 	]*vorpd  zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 b2 00 20 00 00[ 	]*vorpd  zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 72 80[ 	]*vorpd  zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 56 b2 c0 df ff ff[ 	]*vorpd  zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 72 7f[ 	]*vorpd  zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 b2 00 04 00 00[ 	]*vorpd  zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 72 80[ 	]*vorpd  zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 56 b2 f8 fb ff ff[ 	]*vorpd  zmm30,zmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 56 f4[ 	]*vorps  zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 56 f4[ 	]*vorps  zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 56 f4[ 	]*vorps  zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 31[ 	]*vorps  zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 56 b4 f0 34 12 00 00[ 	]*vorps  zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 31[ 	]*vorps  zmm30,zmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 72 7f[ 	]*vorps  zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 b2 00 20 00 00[ 	]*vorps  zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 72 80[ 	]*vorps  zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 56 b2 c0 df ff ff[ 	]*vorps  zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 72 7f[ 	]*vorps  zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 b2 00 02 00 00[ 	]*vorps  zmm30,zmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 72 80[ 	]*vorps  zmm30,zmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 56 b2 fc fd ff ff[ 	]*vorps  zmm30,zmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 95 40 57 f4[ 	]*vxorpd zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 47 57 f4[ 	]*vxorpd zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 95 c7 57 f4[ 	]*vxorpd zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 31[ 	]*vxorpd zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 95 40 57 b4 f0 34 12 00 00[ 	]*vxorpd zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 31[ 	]*vxorpd zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 72 7f[ 	]*vxorpd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 b2 00 20 00 00[ 	]*vxorpd zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 72 80[ 	]*vxorpd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 40 57 b2 c0 df ff ff[ 	]*vxorpd zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 72 7f[ 	]*vxorpd zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 b2 00 04 00 00[ 	]*vxorpd zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 72 80[ 	]*vxorpd zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 95 50 57 b2 f8 fb ff ff[ 	]*vxorpd zmm30,zmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 14 40 57 f4[ 	]*vxorps zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 47 57 f4[ 	]*vxorps zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 14 c7 57 f4[ 	]*vxorps zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 31[ 	]*vxorps zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 14 40 57 b4 f0 34 12 00 00[ 	]*vxorps zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 31[ 	]*vxorps zmm30,zmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 72 7f[ 	]*vxorps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 b2 00 20 00 00[ 	]*vxorps zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 72 80[ 	]*vxorps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 40 57 b2 c0 df ff ff[ 	]*vxorps zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 72 7f[ 	]*vxorps zmm30,zmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 b2 00 02 00 00[ 	]*vxorps zmm30,zmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 72 80[ 	]*vxorps zmm30,zmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 14 50 57 b2 fc fd ff ff[ 	]*vxorps zmm30,zmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 56 f5 ab[ 	]*vreducepd zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 4f 56 f5 ab[ 	]*vreducepd zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd cf 56 f5 ab[ 	]*vreducepd zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 18 56 f5 ab[ 	]*vreducepd zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 48 56 f5 7b[ 	]*vreducepd zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 fd 18 56 f5 7b[ 	]*vreducepd zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 31 7b[ 	]*vreducepd zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 48 56 b4 f0 34 12 00 00 7b[ 	]*vreducepd zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 31 7b[ 	]*vreducepd zmm30,QWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 72 7f 7b[ 	]*vreducepd zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 b2 00 20 00 00 7b[ 	]*vreducepd zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 72 80 7b[ 	]*vreducepd zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 56 b2 c0 df ff ff 7b[ 	]*vreducepd zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 72 7f 7b[ 	]*vreducepd zmm30,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 b2 00 04 00 00 7b[ 	]*vreducepd zmm30,QWORD BCST \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 72 80 7b[ 	]*vreducepd zmm30,QWORD BCST \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 58 56 b2 f8 fb ff ff 7b[ 	]*vreducepd zmm30,QWORD BCST \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 56 f5 ab[ 	]*vreduceps zmm30,zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 4f 56 f5 ab[ 	]*vreduceps zmm30\{k7\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d cf 56 f5 ab[ 	]*vreduceps zmm30\{k7\}\{z\},zmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 18 56 f5 ab[ 	]*vreduceps zmm30,zmm29\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 48 56 f5 7b[ 	]*vreduceps zmm30,zmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 7d 18 56 f5 7b[ 	]*vreduceps zmm30,zmm29\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 31 7b[ 	]*vreduceps zmm30,ZMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 48 56 b4 f0 34 12 00 00 7b[ 	]*vreduceps zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 31 7b[ 	]*vreduceps zmm30,DWORD BCST \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 72 7f 7b[ 	]*vreduceps zmm30,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 b2 00 20 00 00 7b[ 	]*vreduceps zmm30,ZMMWORD PTR \[rdx\+0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 72 80 7b[ 	]*vreduceps zmm30,ZMMWORD PTR \[rdx-0x2000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 56 b2 c0 df ff ff 7b[ 	]*vreduceps zmm30,ZMMWORD PTR \[rdx-0x2040\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 72 7f 7b[ 	]*vreduceps zmm30,DWORD BCST \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 b2 00 02 00 00 7b[ 	]*vreduceps zmm30,DWORD BCST \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 72 80 7b[ 	]*vreduceps zmm30,DWORD BCST \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 58 56 b2 fc fd ff ff 7b[ 	]*vreduceps zmm30,DWORD BCST \[rdx-0x204\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 57 f4 ab[ 	]*vreducesd xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 07 57 f4 ab[ 	]*vreducesd xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 87 57 f4 ab[ 	]*vreducesd xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 57 f4 ab[ 	]*vreducesd xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 00 57 f4 7b[ 	]*vreducesd xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 10 57 f4 7b[ 	]*vreducesd xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 31 7b[ 	]*vreducesd xmm30,xmm29,QWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 95 00 57 b4 f0 34 12 00 00 7b[ 	]*vreducesd xmm30,xmm29,QWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 72 7f 7b[ 	]*vreducesd xmm30,xmm29,QWORD PTR \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 b2 00 04 00 00 7b[ 	]*vreducesd xmm30,xmm29,QWORD PTR \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 72 80 7b[ 	]*vreducesd xmm30,xmm29,QWORD PTR \[rdx-0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 00 57 b2 f8 fb ff ff 7b[ 	]*vreducesd xmm30,xmm29,QWORD PTR \[rdx-0x408\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 57 f4 ab[ 	]*vreducess xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 57 f4 ab[ 	]*vreducess xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 57 f4 ab[ 	]*vreducess xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 57 f4 ab[ 	]*vreducess xmm30,xmm29,xmm28\{sae\},0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 57 f4 7b[ 	]*vreducess xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 10 57 f4 7b[ 	]*vreducess xmm30,xmm29,xmm28\{sae\},0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 31 7b[ 	]*vreducess xmm30,xmm29,DWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 57 b4 f0 34 12 00 00 7b[ 	]*vreducess xmm30,xmm29,DWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 72 7f 7b[ 	]*vreducess xmm30,xmm29,DWORD PTR \[rdx\+0x1fc\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 b2 00 02 00 00 7b[ 	]*vreducess xmm30,xmm29,DWORD PTR \[rdx\+0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 72 80 7b[ 	]*vreducess xmm30,xmm29,DWORD PTR \[rdx-0x200\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 57 b2 fc fd ff ff 7b[ 	]*vreducess xmm30,xmm29,DWORD PTR \[rdx-0x204\],0x7b
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
[ 	]*[a-f0-9]+:[ 	]*c5 f9 90 29[ 	]*kmovb  k5,BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*c4 a1 79 90 ac f0 34 12 00 00[ 	]*kmovb  k5,BYTE PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*c5 f9 91 29[ 	]*kmovb  BYTE PTR \[rcx\],k5
[ 	]*[a-f0-9]+:[ 	]*c4 a1 79 91 ac f0 34 12 00 00[ 	]*kmovb  BYTE PTR \[rax\+r14\*8\+0x1234\],k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 e8[ 	]*kmovb  k5,eax
[ 	]*[a-f0-9]+:[ 	]*c5 f9 92 ed[ 	]*kmovb  k5,ebp
[ 	]*[a-f0-9]+:[ 	]*c4 c1 79 92 ed[ 	]*kmovb  k5,r13d
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 c5[ 	]*kmovb  eax,k5
[ 	]*[a-f0-9]+:[ 	]*c5 f9 93 ed[ 	]*kmovb  ebp,k5
[ 	]*[a-f0-9]+:[ 	]*c5 79 93 ed[ 	]*kmovb  r13d,k5
[ 	]*[a-f0-9]+:[ 	]*c5 cc 4a ef[ 	]*kaddw  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*c5 cd 4a ef[ 	]*kaddb  k5,k6,k7
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 31 ab[ 	]*vextractf64x2 XMMWORD PTR \[rcx\],zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 4f 19 31 ab[ 	]*vextractf64x2 XMMWORD PTR \[rcx\]\{k7\},zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 31 7b[ 	]*vextractf64x2 XMMWORD PTR \[rcx\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 48 19 b4 f0 34 12 00 00 7b[ 	]*vextractf64x2 XMMWORD PTR \[rax\+r14\*8\+0x1234\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 72 7f 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx\+0x7f0\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 b2 00 08 00 00 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx\+0x800\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 72 80 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx-0x800\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 19 b2 f0 f7 ff ff 7b[ 	]*vextractf64x2 XMMWORD PTR \[rdx-0x810\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 31 ab[ 	]*vextractf32x8 YMMWORD PTR \[rcx\],zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 4f 1b 31 ab[ 	]*vextractf32x8 YMMWORD PTR \[rcx\]\{k7\},zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 31 7b[ 	]*vextractf32x8 YMMWORD PTR \[rcx\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 48 1b b4 f0 34 12 00 00 7b[ 	]*vextractf32x8 YMMWORD PTR \[rax\+r14\*8\+0x1234\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 72 7f 7b[ 	]*vextractf32x8 YMMWORD PTR \[rdx\+0xfe0\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b b2 00 10 00 00 7b[ 	]*vextractf32x8 YMMWORD PTR \[rdx\+0x1000\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b 72 80 7b[ 	]*vextractf32x8 YMMWORD PTR \[rdx-0x1000\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 1b b2 e0 ef ff ff 7b[ 	]*vextractf32x8 YMMWORD PTR \[rdx-0x1020\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 31 ab[ 	]*vextracti64x2 XMMWORD PTR \[rcx\],zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 4f 39 31 ab[ 	]*vextracti64x2 XMMWORD PTR \[rcx\]\{k7\},zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 31 7b[ 	]*vextracti64x2 XMMWORD PTR \[rcx\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 fd 48 39 b4 f0 34 12 00 00 7b[ 	]*vextracti64x2 XMMWORD PTR \[rax\+r14\*8\+0x1234\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 72 7f 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx\+0x7f0\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 b2 00 08 00 00 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx\+0x800\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 72 80 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx-0x800\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 fd 48 39 b2 f0 f7 ff ff 7b[ 	]*vextracti64x2 XMMWORD PTR \[rdx-0x810\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 31 ab[ 	]*vextracti32x8 YMMWORD PTR \[rcx\],zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 4f 3b 31 ab[ 	]*vextracti32x8 YMMWORD PTR \[rcx\]\{k7\},zmm30,0xab
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 31 7b[ 	]*vextracti32x8 YMMWORD PTR \[rcx\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 7d 48 3b b4 f0 34 12 00 00 7b[ 	]*vextracti32x8 YMMWORD PTR \[rax\+r14\*8\+0x1234\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 72 7f 7b[ 	]*vextracti32x8 YMMWORD PTR \[rdx\+0xfe0\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b b2 00 10 00 00 7b[ 	]*vextracti32x8 YMMWORD PTR \[rdx\+0x1000\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b 72 80 7b[ 	]*vextracti32x8 YMMWORD PTR \[rdx-0x1000\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 7d 48 3b b2 e0 ef ff ff 7b[ 	]*vextracti32x8 YMMWORD PTR \[rdx-0x1020\],zmm30,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 7a f5[ 	]*vcvttpd2qq zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 7a f5[ 	]*vcvttpd2qq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 7a f5[ 	]*vcvttpd2qq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 7a f5[ 	]*vcvttpd2qq zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a 31[ 	]*vcvttpd2qq zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 7a b4 f0 34 12 00 00[ 	]*vcvttpd2qq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a 31[ 	]*vcvttpd2qq zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a 72 7f[ 	]*vcvttpd2qq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a b2 00 20 00 00[ 	]*vcvttpd2qq zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a 72 80[ 	]*vcvttpd2qq zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 7a b2 c0 df ff ff[ 	]*vcvttpd2qq zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a 72 7f[ 	]*vcvttpd2qq zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a b2 00 04 00 00[ 	]*vcvttpd2qq zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a 72 80[ 	]*vcvttpd2qq zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 7a b2 f8 fb ff ff[ 	]*vcvttpd2qq zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 48 78 f5[ 	]*vcvttpd2uqq zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 4f 78 f5[ 	]*vcvttpd2uqq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd cf 78 f5[ 	]*vcvttpd2uqq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 fd 18 78 f5[ 	]*vcvttpd2uqq zmm30,zmm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 31[ 	]*vcvttpd2uqq zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 fd 48 78 b4 f0 34 12 00 00[ 	]*vcvttpd2uqq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 31[ 	]*vcvttpd2uqq zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 72 7f[ 	]*vcvttpd2uqq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 b2 00 20 00 00[ 	]*vcvttpd2uqq zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 72 80[ 	]*vcvttpd2uqq zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 48 78 b2 c0 df ff ff[ 	]*vcvttpd2uqq zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 72 7f[ 	]*vcvttpd2uqq zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 b2 00 04 00 00[ 	]*vcvttpd2uqq zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 72 80[ 	]*vcvttpd2uqq zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 61 fd 58 78 b2 f8 fb ff ff[ 	]*vcvttpd2uqq zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 7a f5[ 	]*vcvttps2qq zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 7a f5[ 	]*vcvttps2qq zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 7a f5[ 	]*vcvttps2qq zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 7a f5[ 	]*vcvttps2qq zmm30,ymm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a 31[ 	]*vcvttps2qq zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 7a b4 f0 34 12 00 00[ 	]*vcvttps2qq zmm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a 31[ 	]*vcvttps2qq zmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a 72 7f[ 	]*vcvttps2qq zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a b2 00 10 00 00[ 	]*vcvttps2qq zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a 72 80[ 	]*vcvttps2qq zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 7a b2 e0 ef ff ff[ 	]*vcvttps2qq zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a 72 7f[ 	]*vcvttps2qq zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a b2 00 02 00 00[ 	]*vcvttps2qq zmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a 72 80[ 	]*vcvttps2qq zmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a b2 fc fd ff ff[ 	]*vcvttps2qq zmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 7a 72 7f[ 	]*vcvttps2qq zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 48 78 f5[ 	]*vcvttps2uqq zmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 4f 78 f5[ 	]*vcvttps2uqq zmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d cf 78 f5[ 	]*vcvttps2uqq zmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7d 18 78 f5[ 	]*vcvttps2uqq zmm30,ymm29\{sae\}
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 31[ 	]*vcvttps2uqq zmm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7d 48 78 b4 f0 34 12 00 00[ 	]*vcvttps2uqq zmm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 31[ 	]*vcvttps2uqq zmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 72 7f[ 	]*vcvttps2uqq zmm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 b2 00 10 00 00[ 	]*vcvttps2uqq zmm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 72 80[ 	]*vcvttps2uqq zmm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 48 78 b2 e0 ef ff ff[ 	]*vcvttps2uqq zmm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 72 7f[ 	]*vcvttps2uqq zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 b2 00 02 00 00[ 	]*vcvttps2uqq zmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 72 80[ 	]*vcvttps2uqq zmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 b2 fc fd ff ff[ 	]*vcvttps2uqq zmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7d 58 78 72 7f[ 	]*vcvttps2uqq zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 48 39 ee[ 	]*vpmovd2m k5,zmm30
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 48 39 ee[ 	]*vpmovq2m k5,zmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 48 38 f5[ 	]*vpmovm2d zmm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 48 38 f5[ 	]*vpmovm2q zmm30,k5
#pass
