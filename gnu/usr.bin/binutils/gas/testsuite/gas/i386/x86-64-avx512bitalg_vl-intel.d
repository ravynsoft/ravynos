#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512BITALG/VL insns (Intel disassembly)
#source: x86-64-avx512bitalg_vl.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 92 15 00 8f ec[ 	]*vpshufbitqmb k5,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 92 15 07 8f ec[ 	]*vpshufbitqmb k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 b2 15 00 8f ac f0 23 01 00 00[ 	]*vpshufbitqmb k5,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 15 00 8f 6a 7f[ 	]*vpshufbitqmb k5,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 92 15 20 8f ec[ 	]*vpshufbitqmb k5,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 92 15 27 8f ec[ 	]*vpshufbitqmb k5\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 b2 15 20 8f ac f0 23 01 00 00[ 	]*vpshufbitqmb k5,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 15 20 8f 6a 7f[ 	]*vpshufbitqmb k5,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 54 f5[ 	]*vpopcntb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 54 f5[ 	]*vpopcntb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 54 f5[ 	]*vpopcntb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 54 b4 f0 23 01 00 00[ 	]*vpopcntb xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 54 72 7f[ 	]*vpopcntb xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 54 f5[ 	]*vpopcntb ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 54 f5[ 	]*vpopcntb ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 54 f5[ 	]*vpopcntb ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 54 b4 f0 23 01 00 00[ 	]*vpopcntb ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 54 72 7f[ 	]*vpopcntb ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 54 f5[ 	]*vpopcntw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 54 f5[ 	]*vpopcntw xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 54 f5[ 	]*vpopcntw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 54 b4 f0 23 01 00 00[ 	]*vpopcntw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 54 72 7f[ 	]*vpopcntw xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 54 f5[ 	]*vpopcntw ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 54 f5[ 	]*vpopcntw ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 54 f5[ 	]*vpopcntw ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 54 b4 f0 23 01 00 00[ 	]*vpopcntw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 54 72 7f[ 	]*vpopcntw ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 55 f5[ 	]*vpopcntd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 55 f5[ 	]*vpopcntd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 55 f5[ 	]*vpopcntd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 55 b4 f0 23 01 00 00[ 	]*vpopcntd xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 55 72 7f[ 	]*vpopcntd xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 55 72 7f[ 	]*vpopcntd xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 55 f5[ 	]*vpopcntd ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 55 f5[ 	]*vpopcntd ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 55 f5[ 	]*vpopcntd ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 55 b4 f0 23 01 00 00[ 	]*vpopcntd ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 55 72 7f[ 	]*vpopcntd ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 55 72 7f[ 	]*vpopcntd ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 55 f5[ 	]*vpopcntq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 55 f5[ 	]*vpopcntq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 55 f5[ 	]*vpopcntq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 55 b4 f0 23 01 00 00[ 	]*vpopcntq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 55 72 7f[ 	]*vpopcntq xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 55 72 7f[ 	]*vpopcntq xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 55 f5[ 	]*vpopcntq ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 55 f5[ 	]*vpopcntq ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 55 f5[ 	]*vpopcntq ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 55 b4 f0 23 01 00 00[ 	]*vpopcntq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 55 72 7f[ 	]*vpopcntq ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 55 72 7f[ 	]*vpopcntq ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 92 15 00 8f ec[ 	]*vpshufbitqmb k5,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 92 15 07 8f ec[ 	]*vpshufbitqmb k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 b2 15 00 8f ac f0 34 12 00 00[ 	]*vpshufbitqmb k5,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 15 00 8f 6a 7f[ 	]*vpshufbitqmb k5,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 92 15 20 8f ec[ 	]*vpshufbitqmb k5,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 92 15 27 8f ec[ 	]*vpshufbitqmb k5\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 b2 15 20 8f ac f0 34 12 00 00[ 	]*vpshufbitqmb k5,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 15 20 8f 6a 7f[ 	]*vpshufbitqmb k5,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 54 f5[ 	]*vpopcntb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 54 f5[ 	]*vpopcntb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 54 f5[ 	]*vpopcntb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 54 b4 f0 34 12 00 00[ 	]*vpopcntb xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 54 72 7f[ 	]*vpopcntb xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 54 f5[ 	]*vpopcntb ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 54 f5[ 	]*vpopcntb ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 54 f5[ 	]*vpopcntb ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 54 b4 f0 34 12 00 00[ 	]*vpopcntb ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 54 72 7f[ 	]*vpopcntb ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 54 f5[ 	]*vpopcntw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 54 f5[ 	]*vpopcntw xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 54 f5[ 	]*vpopcntw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 54 b4 f0 34 12 00 00[ 	]*vpopcntw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 54 72 7f[ 	]*vpopcntw xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 54 f5[ 	]*vpopcntw ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 54 f5[ 	]*vpopcntw ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 54 f5[ 	]*vpopcntw ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 54 b4 f0 34 12 00 00[ 	]*vpopcntw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 54 72 7f[ 	]*vpopcntw ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 55 f5[ 	]*vpopcntd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 55 f5[ 	]*vpopcntd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 55 f5[ 	]*vpopcntd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 55 b4 f0 34 12 00 00[ 	]*vpopcntd xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 55 72 7f[ 	]*vpopcntd xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 55 72 7f[ 	]*vpopcntd xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 55 f5[ 	]*vpopcntd ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 55 f5[ 	]*vpopcntd ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 55 f5[ 	]*vpopcntd ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 55 b4 f0 34 12 00 00[ 	]*vpopcntd ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 55 72 7f[ 	]*vpopcntd ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 55 72 7f[ 	]*vpopcntd ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 55 f5[ 	]*vpopcntq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 55 f5[ 	]*vpopcntq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 55 f5[ 	]*vpopcntq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 55 b4 f0 34 12 00 00[ 	]*vpopcntq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 55 72 7f[ 	]*vpopcntq xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 55 72 7f[ 	]*vpopcntq xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 55 f5[ 	]*vpopcntq ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 55 f5[ 	]*vpopcntq ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 55 f5[ 	]*vpopcntq ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 55 b4 f0 34 12 00 00[ 	]*vpopcntq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 55 72 7f[ 	]*vpopcntq ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 55 72 7f[ 	]*vpopcntq ymm30,QWORD BCST \[rdx\+0x3f8\]
#pass
