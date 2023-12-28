#as:
#objdump: -dw -Mintel
#name: i386 AVX512VBMI/VL insns (Intel disassembly)
#source: avx512vbmi_vl.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d f4[ 	]*vpermb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 8d f4[ 	]*vpermb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d 31[ 	]*vpermb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d b4 f4 c0 1d fe ff[ 	]*vpermb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d 72 7f[ 	]*vpermb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d b2 00 08 00 00[ 	]*vpermb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d 72 80[ 	]*vpermb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d b2 f0 f7 ff ff[ 	]*vpermb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d f4[ 	]*vpermb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 8d f4[ 	]*vpermb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d 31[ 	]*vpermb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d b4 f4 c0 1d fe ff[ 	]*vpermb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d 72 7f[ 	]*vpermb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d b2 00 10 00 00[ 	]*vpermb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d 72 80[ 	]*vpermb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d b2 e0 ef ff ff[ 	]*vpermb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 f4[ 	]*vpermi2b xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 75 f4[ 	]*vpermi2b xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 31[ 	]*vpermi2b xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2b xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 72 7f[ 	]*vpermi2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 b2 00 08 00 00[ 	]*vpermi2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 72 80[ 	]*vpermi2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 b2 f0 f7 ff ff[ 	]*vpermi2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 f4[ 	]*vpermi2b ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 75 f4[ 	]*vpermi2b ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 31[ 	]*vpermi2b ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2b ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 72 7f[ 	]*vpermi2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 b2 00 10 00 00[ 	]*vpermi2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 72 80[ 	]*vpermi2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 b2 e0 ef ff ff[ 	]*vpermi2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d f4[ 	]*vpermt2b xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 7d f4[ 	]*vpermt2b xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d 31[ 	]*vpermt2b xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2b xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d 72 7f[ 	]*vpermt2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d b2 00 08 00 00[ 	]*vpermt2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d 72 80[ 	]*vpermt2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d b2 f0 f7 ff ff[ 	]*vpermt2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d f4[ 	]*vpermt2b ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 7d f4[ 	]*vpermt2b ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d 31[ 	]*vpermt2b ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2b ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d 72 7f[ 	]*vpermt2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d b2 00 10 00 00[ 	]*vpermt2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d 72 80[ 	]*vpermt2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d b2 e0 ef ff ff[ 	]*vpermt2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 f4[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 83 f4[ 	]*vpmultishiftqb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 31[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 b4 f4 c0 1d fe ff[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 30[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 72 7f[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 b2 00 08 00 00[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 72 80[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 b2 f0 f7 ff ff[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 72 7f[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 b2 00 04 00 00[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 72 80[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 f4[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 83 f4[ 	]*vpmultishiftqb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 31[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 b4 f4 c0 1d fe ff[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 30[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 72 7f[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 b2 00 10 00 00[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 72 80[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 b2 e0 ef ff ff[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 72 7f[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 b2 00 04 00 00[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 72 80[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d f4[ 	]*vpermb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 8d f4[ 	]*vpermb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d 31[ 	]*vpermb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d b4 f4 c0 1d fe ff[ 	]*vpermb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d 72 7f[ 	]*vpermb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d b2 00 08 00 00[ 	]*vpermb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d 72 80[ 	]*vpermb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8d b2 f0 f7 ff ff[ 	]*vpermb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d f4[ 	]*vpermb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 8d f4[ 	]*vpermb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d 31[ 	]*vpermb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d b4 f4 c0 1d fe ff[ 	]*vpermb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d 72 7f[ 	]*vpermb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d b2 00 10 00 00[ 	]*vpermb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d 72 80[ 	]*vpermb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8d b2 e0 ef ff ff[ 	]*vpermb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 f4[ 	]*vpermi2b xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 75 f4[ 	]*vpermi2b xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 31[ 	]*vpermi2b xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2b xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 72 7f[ 	]*vpermi2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 b2 00 08 00 00[ 	]*vpermi2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 72 80[ 	]*vpermi2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 75 b2 f0 f7 ff ff[ 	]*vpermi2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 f4[ 	]*vpermi2b ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 75 f4[ 	]*vpermi2b ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 31[ 	]*vpermi2b ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2b ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 72 7f[ 	]*vpermi2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 b2 00 10 00 00[ 	]*vpermi2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 72 80[ 	]*vpermi2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 75 b2 e0 ef ff ff[ 	]*vpermi2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d f4[ 	]*vpermt2b xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 7d f4[ 	]*vpermt2b xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d 31[ 	]*vpermt2b xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2b xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d 72 7f[ 	]*vpermt2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d b2 00 08 00 00[ 	]*vpermt2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d 72 80[ 	]*vpermt2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 7d b2 f0 f7 ff ff[ 	]*vpermt2b xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d f4[ 	]*vpermt2b ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 7d f4[ 	]*vpermt2b ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d 31[ 	]*vpermt2b ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2b ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d 72 7f[ 	]*vpermt2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d b2 00 10 00 00[ 	]*vpermt2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d 72 80[ 	]*vpermt2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 7d b2 e0 ef ff ff[ 	]*vpermt2b ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 f4[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 83 f4[ 	]*vpmultishiftqb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 31[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 b4 f4 c0 1d fe ff[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 30[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 72 7f[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 b2 00 08 00 00[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 72 80[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 83 b2 f0 f7 ff ff[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 72 7f[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 b2 00 04 00 00[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 72 80[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 1f 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb xmm6\{k7\},xmm5,QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 f4[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 83 f4[ 	]*vpmultishiftqb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 31[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 b4 f4 c0 1d fe ff[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 30[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 72 7f[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 b2 00 10 00 00[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 72 80[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 83 b2 e0 ef ff ff[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 72 7f[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 b2 00 04 00 00[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 72 80[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 3f 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb ymm6\{k7\},ymm5,QWORD BCST \[edx-0x408\]
#pass
