#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512VBMI insns (Intel disassembly)
#source: x86-64-avx512vbmi.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 8d f4[ 	]*vpermb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 8d f4[ 	]*vpermb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 8d f4[ 	]*vpermb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d 31[ 	]*vpermb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 8d b4 f0 23 01 00 00[ 	]*vpermb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d 72 7f[ 	]*vpermb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d b2 00 20 00 00[ 	]*vpermb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d 72 80[ 	]*vpermb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d b2 c0 df ff ff[ 	]*vpermb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 75 f4[ 	]*vpermi2b zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 75 f4[ 	]*vpermi2b zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 75 f4[ 	]*vpermi2b zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 31[ 	]*vpermi2b zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 75 b4 f0 23 01 00 00[ 	]*vpermi2b zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 72 7f[ 	]*vpermi2b zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 b2 00 20 00 00[ 	]*vpermi2b zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 72 80[ 	]*vpermi2b zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 b2 c0 df ff ff[ 	]*vpermi2b zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 7d f4[ 	]*vpermt2b zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 7d f4[ 	]*vpermt2b zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 7d f4[ 	]*vpermt2b zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d 31[ 	]*vpermt2b zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 7d b4 f0 23 01 00 00[ 	]*vpermt2b zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d 72 7f[ 	]*vpermt2b zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d b2 00 20 00 00[ 	]*vpermt2b zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d 72 80[ 	]*vpermt2b zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d b2 c0 df ff ff[ 	]*vpermt2b zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 83 f4[ 	]*vpmultishiftqb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 83 f4[ 	]*vpmultishiftqb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 83 f4[ 	]*vpmultishiftqb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 31[ 	]*vpmultishiftqb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 83 b4 f0 23 01 00 00[ 	]*vpmultishiftqb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 31[ 	]*vpmultishiftqb zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 72 7f[ 	]*vpmultishiftqb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 b2 00 20 00 00[ 	]*vpmultishiftqb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 72 80[ 	]*vpmultishiftqb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 b2 c0 df ff ff[ 	]*vpmultishiftqb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 72 7f[ 	]*vpmultishiftqb zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 b2 00 04 00 00[ 	]*vpmultishiftqb zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 72 80[ 	]*vpmultishiftqb zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb zmm30,zmm29,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 8d f4[ 	]*vpermb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 8d f4[ 	]*vpermb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 8d f4[ 	]*vpermb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d 31[ 	]*vpermb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 8d b4 f0 34 12 00 00[ 	]*vpermb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d 72 7f[ 	]*vpermb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d b2 00 20 00 00[ 	]*vpermb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d 72 80[ 	]*vpermb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 8d b2 c0 df ff ff[ 	]*vpermb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 75 f4[ 	]*vpermi2b zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 75 f4[ 	]*vpermi2b zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 75 f4[ 	]*vpermi2b zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 31[ 	]*vpermi2b zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 75 b4 f0 34 12 00 00[ 	]*vpermi2b zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 72 7f[ 	]*vpermi2b zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 b2 00 20 00 00[ 	]*vpermi2b zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 72 80[ 	]*vpermi2b zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 75 b2 c0 df ff ff[ 	]*vpermi2b zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 7d f4[ 	]*vpermt2b zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 7d f4[ 	]*vpermt2b zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 7d f4[ 	]*vpermt2b zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d 31[ 	]*vpermt2b zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 7d b4 f0 34 12 00 00[ 	]*vpermt2b zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d 72 7f[ 	]*vpermt2b zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d b2 00 20 00 00[ 	]*vpermt2b zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d 72 80[ 	]*vpermt2b zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 7d b2 c0 df ff ff[ 	]*vpermt2b zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 40 83 f4[ 	]*vpmultishiftqb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 47 83 f4[ 	]*vpmultishiftqb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 c7 83 f4[ 	]*vpmultishiftqb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 31[ 	]*vpmultishiftqb zmm30,zmm29,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 40 83 b4 f0 34 12 00 00[ 	]*vpmultishiftqb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 31[ 	]*vpmultishiftqb zmm30,zmm29,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 72 7f[ 	]*vpmultishiftqb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 b2 00 20 00 00[ 	]*vpmultishiftqb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 72 80[ 	]*vpmultishiftqb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 40 83 b2 c0 df ff ff[ 	]*vpmultishiftqb zmm30,zmm29,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 72 7f[ 	]*vpmultishiftqb zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 b2 00 04 00 00[ 	]*vpmultishiftqb zmm30,zmm29,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 72 80[ 	]*vpmultishiftqb zmm30,zmm29,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 50 83 b2 f8 fb ff ff[ 	]*vpmultishiftqb zmm30,zmm29,QWORD BCST \[rdx-0x408\]
#pass
