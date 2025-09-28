#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512BITALG insns (Intel disassembly)
#source: x86-64-avx512bitalg.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 92 15 40 8f ec[ 	]*vpshufbitqmb k5,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 92 15 47 8f ec[ 	]*vpshufbitqmb k5\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 b2 15 40 8f ac f0 23 01 00 00[ 	]*vpshufbitqmb k5,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 15 40 8f 6a 7f[ 	]*vpshufbitqmb k5,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 54 f5[ 	]*vpopcntb zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 54 f5[ 	]*vpopcntb zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 54 f5[ 	]*vpopcntb zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 54 b4 f0 23 01 00 00[ 	]*vpopcntb zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 54 72 7f[ 	]*vpopcntb zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 54 f5[ 	]*vpopcntw zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 54 f5[ 	]*vpopcntw zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 54 f5[ 	]*vpopcntw zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 54 b4 f0 23 01 00 00[ 	]*vpopcntw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 54 72 7f[ 	]*vpopcntw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 55 f5[ 	]*vpopcntd zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 55 f5[ 	]*vpopcntd zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 55 f5[ 	]*vpopcntd zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 55 b4 f0 23 01 00 00[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 72 7f[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 72 7f[ 	]*vpopcntd zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 55 f5[ 	]*vpopcntq zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 55 f5[ 	]*vpopcntq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 55 f5[ 	]*vpopcntq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 55 b4 f0 23 01 00 00[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 72 7f[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 72 7f[ 	]*vpopcntq zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 92 15 40 8f ec[ 	]*vpshufbitqmb k5,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 92 15 47 8f ec[ 	]*vpshufbitqmb k5\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 b2 15 40 8f ac f0 34 12 00 00[ 	]*vpshufbitqmb k5,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 15 40 8f 6a 7f[ 	]*vpshufbitqmb k5,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 54 f5[ 	]*vpopcntb zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 54 f5[ 	]*vpopcntb zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 54 f5[ 	]*vpopcntb zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 54 b4 f0 34 12 00 00[ 	]*vpopcntb zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 54 72 7f[ 	]*vpopcntb zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 54 f5[ 	]*vpopcntw zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 54 f5[ 	]*vpopcntw zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 54 f5[ 	]*vpopcntw zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 54 b4 f0 34 12 00 00[ 	]*vpopcntw zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 54 72 7f[ 	]*vpopcntw zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 55 f5[ 	]*vpopcntd zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 55 f5[ 	]*vpopcntd zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 55 f5[ 	]*vpopcntd zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 55 b4 f0 34 12 00 00[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 72 7f[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 72 7f[ 	]*vpopcntd zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 55 f5[ 	]*vpopcntq zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 55 f5[ 	]*vpopcntq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 55 f5[ 	]*vpopcntq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 55 b4 f0 34 12 00 00[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 72 7f[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 72 7f[ 	]*vpopcntq zmm30,QWORD BCST \[rdx\+0x3f8\]
#pass
