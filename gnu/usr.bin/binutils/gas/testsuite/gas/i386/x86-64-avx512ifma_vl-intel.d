#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512IFMA/VL insns (Intel disassembly)
#source: x86-64-avx512ifma_vl.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 b4 f4[ 	]*vpmadd52luq xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 b4 f4[ 	]*vpmadd52luq xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 b4 f4[ 	]*vpmadd52luq xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 31[ 	]*vpmadd52luq xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 b4 b4 f0 23 01 00 00[ 	]*vpmadd52luq xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 31[ 	]*vpmadd52luq xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 72 7f[ 	]*vpmadd52luq xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 b2 00 08 00 00[ 	]*vpmadd52luq xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 72 80[ 	]*vpmadd52luq xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 b2 f0 f7 ff ff[ 	]*vpmadd52luq xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 72 7f[ 	]*vpmadd52luq xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 b2 00 04 00 00[ 	]*vpmadd52luq xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 72 80[ 	]*vpmadd52luq xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 b4 f4[ 	]*vpmadd52luq ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 b4 f4[ 	]*vpmadd52luq ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 b4 f4[ 	]*vpmadd52luq ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 31[ 	]*vpmadd52luq ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 b4 b4 f0 23 01 00 00[ 	]*vpmadd52luq ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 31[ 	]*vpmadd52luq ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 72 7f[ 	]*vpmadd52luq ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 b2 00 10 00 00[ 	]*vpmadd52luq ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 72 80[ 	]*vpmadd52luq ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 b2 e0 ef ff ff[ 	]*vpmadd52luq ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 72 7f[ 	]*vpmadd52luq ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 b2 00 04 00 00[ 	]*vpmadd52luq ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 72 80[ 	]*vpmadd52luq ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq ymm30,ymm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 b5 f4[ 	]*vpmadd52huq xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 b5 f4[ 	]*vpmadd52huq xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 b5 f4[ 	]*vpmadd52huq xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 31[ 	]*vpmadd52huq xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 b5 b4 f0 23 01 00 00[ 	]*vpmadd52huq xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 31[ 	]*vpmadd52huq xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 72 7f[ 	]*vpmadd52huq xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 b2 00 08 00 00[ 	]*vpmadd52huq xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 72 80[ 	]*vpmadd52huq xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 b2 f0 f7 ff ff[ 	]*vpmadd52huq xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 72 7f[ 	]*vpmadd52huq xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 b2 00 04 00 00[ 	]*vpmadd52huq xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 72 80[ 	]*vpmadd52huq xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 b5 f4[ 	]*vpmadd52huq ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 b5 f4[ 	]*vpmadd52huq ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 b5 f4[ 	]*vpmadd52huq ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 31[ 	]*vpmadd52huq ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 b5 b4 f0 23 01 00 00[ 	]*vpmadd52huq ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 31[ 	]*vpmadd52huq ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 72 7f[ 	]*vpmadd52huq ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 b2 00 10 00 00[ 	]*vpmadd52huq ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 72 80[ 	]*vpmadd52huq ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 b2 e0 ef ff ff[ 	]*vpmadd52huq ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 72 7f[ 	]*vpmadd52huq ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 b2 00 04 00 00[ 	]*vpmadd52huq ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 72 80[ 	]*vpmadd52huq ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq ymm30,ymm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 b4 f4[ 	]*vpmadd52luq xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 b4 f4[ 	]*vpmadd52luq xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 b4 f4[ 	]*vpmadd52luq xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 31[ 	]*vpmadd52luq xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 b4 b4 f0 34 12 00 00[ 	]*vpmadd52luq xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 31[ 	]*vpmadd52luq xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 72 7f[ 	]*vpmadd52luq xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 b2 00 08 00 00[ 	]*vpmadd52luq xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 72 80[ 	]*vpmadd52luq xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b4 b2 f0 f7 ff ff[ 	]*vpmadd52luq xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 72 7f[ 	]*vpmadd52luq xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 b2 00 04 00 00[ 	]*vpmadd52luq xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 72 80[ 	]*vpmadd52luq xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 b4 f4[ 	]*vpmadd52luq ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 b4 f4[ 	]*vpmadd52luq ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 b4 f4[ 	]*vpmadd52luq ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 31[ 	]*vpmadd52luq ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 b4 b4 f0 34 12 00 00[ 	]*vpmadd52luq ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 31[ 	]*vpmadd52luq ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 72 7f[ 	]*vpmadd52luq ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 b2 00 10 00 00[ 	]*vpmadd52luq ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 72 80[ 	]*vpmadd52luq ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b4 b2 e0 ef ff ff[ 	]*vpmadd52luq ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 72 7f[ 	]*vpmadd52luq ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 b2 00 04 00 00[ 	]*vpmadd52luq ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 72 80[ 	]*vpmadd52luq ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq ymm30,ymm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 b5 f4[ 	]*vpmadd52huq xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 b5 f4[ 	]*vpmadd52huq xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 b5 f4[ 	]*vpmadd52huq xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 31[ 	]*vpmadd52huq xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 b5 b4 f0 34 12 00 00[ 	]*vpmadd52huq xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 31[ 	]*vpmadd52huq xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 72 7f[ 	]*vpmadd52huq xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 b2 00 08 00 00[ 	]*vpmadd52huq xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 72 80[ 	]*vpmadd52huq xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 b5 b2 f0 f7 ff ff[ 	]*vpmadd52huq xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 72 7f[ 	]*vpmadd52huq xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 b2 00 04 00 00[ 	]*vpmadd52huq xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 72 80[ 	]*vpmadd52huq xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 b5 f4[ 	]*vpmadd52huq ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 b5 f4[ 	]*vpmadd52huq ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 b5 f4[ 	]*vpmadd52huq ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 31[ 	]*vpmadd52huq ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 b5 b4 f0 34 12 00 00[ 	]*vpmadd52huq ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 31[ 	]*vpmadd52huq ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 72 7f[ 	]*vpmadd52huq ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 b2 00 10 00 00[ 	]*vpmadd52huq ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 72 80[ 	]*vpmadd52huq ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 b5 b2 e0 ef ff ff[ 	]*vpmadd52huq ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 72 7f[ 	]*vpmadd52huq ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 b2 00 04 00 00[ 	]*vpmadd52huq ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 72 80[ 	]*vpmadd52huq ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq ymm30,ymm29,QWORD BCST \[rdx-0x408\]
#pass
