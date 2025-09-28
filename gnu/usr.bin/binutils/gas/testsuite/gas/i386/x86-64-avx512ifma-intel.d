#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512IFMA insns (Intel disassembly)
#source: x86-64-avx512ifma.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 b4 f4[ 	]*vpmadd52luq zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 b4 f4[ 	]*vpmadd52luq zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 b4 f4[ 	]*vpmadd52luq zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 31[ 	]*vpmadd52luq zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 b4 b4 f0 23 01 00 00[ 	]*vpmadd52luq zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 31[ 	]*vpmadd52luq zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 72 7f[ 	]*vpmadd52luq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 b2 00 20 00 00[ 	]*vpmadd52luq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 72 80[ 	]*vpmadd52luq zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 b2 c0 df ff ff[ 	]*vpmadd52luq zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 72 7f[ 	]*vpmadd52luq zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 b2 00 04 00 00[ 	]*vpmadd52luq zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 72 80[ 	]*vpmadd52luq zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq zmm30,zmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 b5 f4[ 	]*vpmadd52huq zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 b5 f4[ 	]*vpmadd52huq zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 b5 f4[ 	]*vpmadd52huq zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 31[ 	]*vpmadd52huq zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 b5 b4 f0 23 01 00 00[ 	]*vpmadd52huq zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 31[ 	]*vpmadd52huq zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 72 7f[ 	]*vpmadd52huq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 b2 00 20 00 00[ 	]*vpmadd52huq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 72 80[ 	]*vpmadd52huq zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 b2 c0 df ff ff[ 	]*vpmadd52huq zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 72 7f[ 	]*vpmadd52huq zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 b2 00 04 00 00[ 	]*vpmadd52huq zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 72 80[ 	]*vpmadd52huq zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq zmm30,zmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 b4 f4[ 	]*vpmadd52luq zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 b4 f4[ 	]*vpmadd52luq zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 b4 f4[ 	]*vpmadd52luq zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 31[ 	]*vpmadd52luq zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 b4 b4 f0 34 12 00 00[ 	]*vpmadd52luq zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 31[ 	]*vpmadd52luq zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 72 7f[ 	]*vpmadd52luq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 b2 00 20 00 00[ 	]*vpmadd52luq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 72 80[ 	]*vpmadd52luq zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b4 b2 c0 df ff ff[ 	]*vpmadd52luq zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 72 7f[ 	]*vpmadd52luq zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 b2 00 04 00 00[ 	]*vpmadd52luq zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 72 80[ 	]*vpmadd52luq zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b4 b2 f8 fb ff ff[ 	]*vpmadd52luq zmm30,zmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 b5 f4[ 	]*vpmadd52huq zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 b5 f4[ 	]*vpmadd52huq zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 b5 f4[ 	]*vpmadd52huq zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 31[ 	]*vpmadd52huq zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 b5 b4 f0 34 12 00 00[ 	]*vpmadd52huq zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 31[ 	]*vpmadd52huq zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 72 7f[ 	]*vpmadd52huq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 b2 00 20 00 00[ 	]*vpmadd52huq zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 72 80[ 	]*vpmadd52huq zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 b5 b2 c0 df ff ff[ 	]*vpmadd52huq zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 72 7f[ 	]*vpmadd52huq zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 b2 00 04 00 00[ 	]*vpmadd52huq zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 72 80[ 	]*vpmadd52huq zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 b5 b2 f8 fb ff ff[ 	]*vpmadd52huq zmm30,zmm29,QWORD BCST \[rdx-0x408\]
#pass
