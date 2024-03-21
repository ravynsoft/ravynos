#as:
#objdump: -dw -Mintel
#name: i386 AVX512IFMA/VL insns (Intel disassembly)
#source: avx512ifma_vl.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 f4[ 	]*vpmadd52luq xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f b4 f4[ 	]*vpmadd52luq xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 31[ 	]*vpmadd52luq xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 b4 f4 c0 1d fe ff[ 	]*vpmadd52luq xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 30[ 	]*vpmadd52luq xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 72 7f[ 	]*vpmadd52luq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 b2 00 08 00 00[ 	]*vpmadd52luq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 72 80[ 	]*vpmadd52luq xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 b2 f0 f7 ff ff[ 	]*vpmadd52luq xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 72 7f[ 	]*vpmadd52luq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 b2 00 04 00 00[ 	]*vpmadd52luq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 72 80[ 	]*vpmadd52luq xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 b2 f8 fb ff ff[ 	]*vpmadd52luq xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 f4[ 	]*vpmadd52luq ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af b4 f4[ 	]*vpmadd52luq ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 31[ 	]*vpmadd52luq ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 b4 f4 c0 1d fe ff[ 	]*vpmadd52luq ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 30[ 	]*vpmadd52luq ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 72 7f[ 	]*vpmadd52luq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 b2 00 10 00 00[ 	]*vpmadd52luq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 72 80[ 	]*vpmadd52luq ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 b2 e0 ef ff ff[ 	]*vpmadd52luq ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 72 7f[ 	]*vpmadd52luq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 b2 00 04 00 00[ 	]*vpmadd52luq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 72 80[ 	]*vpmadd52luq ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 b2 f8 fb ff ff[ 	]*vpmadd52luq ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 f4[ 	]*vpmadd52huq xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f b5 f4[ 	]*vpmadd52huq xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 31[ 	]*vpmadd52huq xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 b4 f4 c0 1d fe ff[ 	]*vpmadd52huq xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 30[ 	]*vpmadd52huq xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 72 7f[ 	]*vpmadd52huq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 b2 00 08 00 00[ 	]*vpmadd52huq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 72 80[ 	]*vpmadd52huq xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 b2 f0 f7 ff ff[ 	]*vpmadd52huq xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 72 7f[ 	]*vpmadd52huq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 b2 00 04 00 00[ 	]*vpmadd52huq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 72 80[ 	]*vpmadd52huq xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 b2 f8 fb ff ff[ 	]*vpmadd52huq xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 f4[ 	]*vpmadd52huq ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af b5 f4[ 	]*vpmadd52huq ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 31[ 	]*vpmadd52huq ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 b4 f4 c0 1d fe ff[ 	]*vpmadd52huq ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 30[ 	]*vpmadd52huq ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 72 7f[ 	]*vpmadd52huq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 b2 00 10 00 00[ 	]*vpmadd52huq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 72 80[ 	]*vpmadd52huq ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 b2 e0 ef ff ff[ 	]*vpmadd52huq ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 72 7f[ 	]*vpmadd52huq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 b2 00 04 00 00[ 	]*vpmadd52huq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 72 80[ 	]*vpmadd52huq ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 b2 f8 fb ff ff[ 	]*vpmadd52huq ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 f4[ 	]*vpmadd52luq xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f b4 f4[ 	]*vpmadd52luq xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 31[ 	]*vpmadd52luq xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 b4 f4 c0 1d fe ff[ 	]*vpmadd52luq xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 30[ 	]*vpmadd52luq xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 72 7f[ 	]*vpmadd52luq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 b2 00 08 00 00[ 	]*vpmadd52luq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 72 80[ 	]*vpmadd52luq xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b4 b2 f0 f7 ff ff[ 	]*vpmadd52luq xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 72 7f[ 	]*vpmadd52luq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 b2 00 04 00 00[ 	]*vpmadd52luq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 72 80[ 	]*vpmadd52luq xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b4 b2 f8 fb ff ff[ 	]*vpmadd52luq xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 f4[ 	]*vpmadd52luq ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af b4 f4[ 	]*vpmadd52luq ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 31[ 	]*vpmadd52luq ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 b4 f4 c0 1d fe ff[ 	]*vpmadd52luq ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 30[ 	]*vpmadd52luq ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 72 7f[ 	]*vpmadd52luq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 b2 00 10 00 00[ 	]*vpmadd52luq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 72 80[ 	]*vpmadd52luq ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b4 b2 e0 ef ff ff[ 	]*vpmadd52luq ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 72 7f[ 	]*vpmadd52luq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 b2 00 04 00 00[ 	]*vpmadd52luq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 72 80[ 	]*vpmadd52luq ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b4 b2 f8 fb ff ff[ 	]*vpmadd52luq ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 f4[ 	]*vpmadd52huq xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f b5 f4[ 	]*vpmadd52huq xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 31[ 	]*vpmadd52huq xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 b4 f4 c0 1d fe ff[ 	]*vpmadd52huq xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 30[ 	]*vpmadd52huq xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 72 7f[ 	]*vpmadd52huq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 b2 00 08 00 00[ 	]*vpmadd52huq xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 72 80[ 	]*vpmadd52huq xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f b5 b2 f0 f7 ff ff[ 	]*vpmadd52huq xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 72 7f[ 	]*vpmadd52huq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 b2 00 04 00 00[ 	]*vpmadd52huq xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 72 80[ 	]*vpmadd52huq xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f b5 b2 f8 fb ff ff[ 	]*vpmadd52huq xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 f4[ 	]*vpmadd52huq ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af b5 f4[ 	]*vpmadd52huq ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 31[ 	]*vpmadd52huq ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 b4 f4 c0 1d fe ff[ 	]*vpmadd52huq ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 30[ 	]*vpmadd52huq ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 72 7f[ 	]*vpmadd52huq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 b2 00 10 00 00[ 	]*vpmadd52huq ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 72 80[ 	]*vpmadd52huq ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f b5 b2 e0 ef ff ff[ 	]*vpmadd52huq ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 72 7f[ 	]*vpmadd52huq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 b2 00 04 00 00[ 	]*vpmadd52huq ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 72 80[ 	]*vpmadd52huq ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f b5 b2 f8 fb ff ff[ 	]*vpmadd52huq ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
#pass
