#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512VBMI/VL insns (Intel disassembly)
#source: x86-64-avx512vbmi_vl.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 8d f4[ 	]*vpermb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 8d f4[ 	]*vpermb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 8d f4[ 	]*vpermb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d 31[ 	]*vpermb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 8d b4 f0 23 01 00 00[ 	]*vpermb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d 72 7f[ 	]*vpermb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d b2 00 08 00 00[ 	]*vpermb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d 72 80[ 	]*vpermb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d b2 f0 f7 ff ff[ 	]*vpermb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 8d f4[ 	]*vpermb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 8d f4[ 	]*vpermb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 8d f4[ 	]*vpermb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d 31[ 	]*vpermb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 8d b4 f0 23 01 00 00[ 	]*vpermb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d 72 7f[ 	]*vpermb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d b2 00 10 00 00[ 	]*vpermb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d 72 80[ 	]*vpermb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d b2 e0 ef ff ff[ 	]*vpermb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 75 f4[ 	]*vpermi2b xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 75 f4[ 	]*vpermi2b xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 75 f4[ 	]*vpermi2b xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 31[ 	]*vpermi2b xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 75 b4 f0 23 01 00 00[ 	]*vpermi2b xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 72 7f[ 	]*vpermi2b xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 b2 00 08 00 00[ 	]*vpermi2b xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 72 80[ 	]*vpermi2b xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 b2 f0 f7 ff ff[ 	]*vpermi2b xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 75 f4[ 	]*vpermi2b ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 75 f4[ 	]*vpermi2b ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 75 f4[ 	]*vpermi2b ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 31[ 	]*vpermi2b ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 75 b4 f0 23 01 00 00[ 	]*vpermi2b ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 72 7f[ 	]*vpermi2b ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 b2 00 10 00 00[ 	]*vpermi2b ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 72 80[ 	]*vpermi2b ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 b2 e0 ef ff ff[ 	]*vpermi2b ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 7d f4[ 	]*vpermt2b xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 7d f4[ 	]*vpermt2b xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 7d f4[ 	]*vpermt2b xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d 31[ 	]*vpermt2b xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 7d b4 f0 23 01 00 00[ 	]*vpermt2b xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d 72 7f[ 	]*vpermt2b xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d b2 00 08 00 00[ 	]*vpermt2b xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d 72 80[ 	]*vpermt2b xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d b2 f0 f7 ff ff[ 	]*vpermt2b xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 7d f4[ 	]*vpermt2b ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 7d f4[ 	]*vpermt2b ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 7d f4[ 	]*vpermt2b ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d 31[ 	]*vpermt2b ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 7d b4 f0 23 01 00 00[ 	]*vpermt2b ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d 72 7f[ 	]*vpermt2b ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d b2 00 10 00 00[ 	]*vpermt2b ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d 72 80[ 	]*vpermt2b ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d b2 e0 ef ff ff[ 	]*vpermt2b ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 83 f4[ 	]*vpmultishiftqb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 83 f4[ 	]*vpmultishiftqb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 83 f4[ 	]*vpmultishiftqb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 31[ 	]*vpmultishiftqb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 83 b4 f0 23 01 00 00[ 	]*vpmultishiftqb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 31[ 	]*vpmultishiftqb xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 72 7f[ 	]*vpmultishiftqb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 b2 00 08 00 00[ 	]*vpmultishiftqb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 72 80[ 	]*vpmultishiftqb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 b2 f0 f7 ff ff[ 	]*vpmultishiftqb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 72 7f[ 	]*vpmultishiftqb xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 b2 00 04 00 00[ 	]*vpmultishiftqb xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 72 80[ 	]*vpmultishiftqb xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 83 f4[ 	]*vpmultishiftqb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 83 f4[ 	]*vpmultishiftqb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 83 f4[ 	]*vpmultishiftqb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 31[ 	]*vpmultishiftqb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 83 b4 f0 23 01 00 00[ 	]*vpmultishiftqb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 31[ 	]*vpmultishiftqb ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 72 7f[ 	]*vpmultishiftqb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 b2 00 10 00 00[ 	]*vpmultishiftqb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 72 80[ 	]*vpmultishiftqb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 b2 e0 ef ff ff[ 	]*vpmultishiftqb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 72 7f[ 	]*vpmultishiftqb ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 b2 00 04 00 00[ 	]*vpmultishiftqb ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 72 80[ 	]*vpmultishiftqb ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb ymm30,ymm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 8d f4[ 	]*vpermb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 8d f4[ 	]*vpermb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 8d f4[ 	]*vpermb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d 31[ 	]*vpermb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 8d b4 f0 34 12 00 00[ 	]*vpermb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d 72 7f[ 	]*vpermb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d b2 00 08 00 00[ 	]*vpermb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d 72 80[ 	]*vpermb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 8d b2 f0 f7 ff ff[ 	]*vpermb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 8d f4[ 	]*vpermb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 8d f4[ 	]*vpermb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 8d f4[ 	]*vpermb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d 31[ 	]*vpermb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 8d b4 f0 34 12 00 00[ 	]*vpermb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d 72 7f[ 	]*vpermb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d b2 00 10 00 00[ 	]*vpermb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d 72 80[ 	]*vpermb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 8d b2 e0 ef ff ff[ 	]*vpermb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 75 f4[ 	]*vpermi2b xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 75 f4[ 	]*vpermi2b xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 75 f4[ 	]*vpermi2b xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 31[ 	]*vpermi2b xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 75 b4 f0 34 12 00 00[ 	]*vpermi2b xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 72 7f[ 	]*vpermi2b xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 b2 00 08 00 00[ 	]*vpermi2b xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 72 80[ 	]*vpermi2b xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 75 b2 f0 f7 ff ff[ 	]*vpermi2b xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 75 f4[ 	]*vpermi2b ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 75 f4[ 	]*vpermi2b ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 75 f4[ 	]*vpermi2b ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 31[ 	]*vpermi2b ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 75 b4 f0 34 12 00 00[ 	]*vpermi2b ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 72 7f[ 	]*vpermi2b ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 b2 00 10 00 00[ 	]*vpermi2b ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 72 80[ 	]*vpermi2b ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 75 b2 e0 ef ff ff[ 	]*vpermi2b ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 7d f4[ 	]*vpermt2b xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 7d f4[ 	]*vpermt2b xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 7d f4[ 	]*vpermt2b xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d 31[ 	]*vpermt2b xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 7d b4 f0 34 12 00 00[ 	]*vpermt2b xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d 72 7f[ 	]*vpermt2b xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d b2 00 08 00 00[ 	]*vpermt2b xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d 72 80[ 	]*vpermt2b xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 7d b2 f0 f7 ff ff[ 	]*vpermt2b xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 7d f4[ 	]*vpermt2b ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 7d f4[ 	]*vpermt2b ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 7d f4[ 	]*vpermt2b ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d 31[ 	]*vpermt2b ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 7d b4 f0 34 12 00 00[ 	]*vpermt2b ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d 72 7f[ 	]*vpermt2b ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d b2 00 10 00 00[ 	]*vpermt2b ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d 72 80[ 	]*vpermt2b ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 7d b2 e0 ef ff ff[ 	]*vpermt2b ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 83 f4[ 	]*vpmultishiftqb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 83 f4[ 	]*vpmultishiftqb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 83 f4[ 	]*vpmultishiftqb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 31[ 	]*vpmultishiftqb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 83 b4 f0 34 12 00 00[ 	]*vpmultishiftqb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 31[ 	]*vpmultishiftqb xmm30,xmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 72 7f[ 	]*vpmultishiftqb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 b2 00 08 00 00[ 	]*vpmultishiftqb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 72 80[ 	]*vpmultishiftqb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 83 b2 f0 f7 ff ff[ 	]*vpmultishiftqb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 72 7f[ 	]*vpmultishiftqb xmm30,xmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 b2 00 04 00 00[ 	]*vpmultishiftqb xmm30,xmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 72 80[ 	]*vpmultishiftqb xmm30,xmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 10 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb xmm30,xmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 83 f4[ 	]*vpmultishiftqb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 83 f4[ 	]*vpmultishiftqb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 83 f4[ 	]*vpmultishiftqb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 31[ 	]*vpmultishiftqb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 83 b4 f0 34 12 00 00[ 	]*vpmultishiftqb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 31[ 	]*vpmultishiftqb ymm30,ymm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 72 7f[ 	]*vpmultishiftqb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 b2 00 10 00 00[ 	]*vpmultishiftqb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 72 80[ 	]*vpmultishiftqb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 83 b2 e0 ef ff ff[ 	]*vpmultishiftqb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 72 7f[ 	]*vpmultishiftqb ymm30,ymm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 b2 00 04 00 00[ 	]*vpmultishiftqb ymm30,ymm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 72 80[ 	]*vpmultishiftqb ymm30,ymm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 30 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb ymm30,ymm29,QWORD BCST \[rdx-0x408\]
#pass
