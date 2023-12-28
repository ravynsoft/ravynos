#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512/VPOPCNTDQ insns (Intel disassembly)
#source: x86-64-avx512_vpopcntdq.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 55 f5[ 	]*vpopcntd zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 55 f5[ 	]*vpopcntd zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 55 f5[ 	]*vpopcntd zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 31[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 55 b4 f0 23 01 00 00[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 31[ 	]*vpopcntd zmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 72 7f[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 b2 00 20 00 00[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 72 80[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 b2 c0 df ff ff[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 72 7f[ 	]*vpopcntd zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 b2 00 02 00 00[ 	]*vpopcntd zmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 72 80[ 	]*vpopcntd zmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 b2 fc fd ff ff[ 	]*vpopcntd zmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 55 f5[ 	]*vpopcntq zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 55 f5[ 	]*vpopcntq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 55 f5[ 	]*vpopcntq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 31[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 55 b4 f0 23 01 00 00[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 31[ 	]*vpopcntq zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 72 7f[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 b2 00 20 00 00[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 72 80[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 b2 c0 df ff ff[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 72 7f[ 	]*vpopcntq zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 b2 00 04 00 00[ 	]*vpopcntq zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 72 80[ 	]*vpopcntq zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 b2 f8 fb ff ff[ 	]*vpopcntq zmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 48 55 f5[ 	]*vpopcntd zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 4f 55 f5[ 	]*vpopcntd zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d cf 55 f5[ 	]*vpopcntd zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 31[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 48 55 b4 f0 34 12 00 00[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 31[ 	]*vpopcntd zmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 72 7f[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 b2 00 20 00 00[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 72 80[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 48 55 b2 c0 df ff ff[ 	]*vpopcntd zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 72 7f[ 	]*vpopcntd zmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 b2 00 02 00 00[ 	]*vpopcntd zmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 72 80[ 	]*vpopcntd zmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 58 55 b2 fc fd ff ff[ 	]*vpopcntd zmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 48 55 f5[ 	]*vpopcntq zmm30,zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 4f 55 f5[ 	]*vpopcntq zmm30\{k7\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd cf 55 f5[ 	]*vpopcntq zmm30\{k7\}\{z\},zmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 31[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 48 55 b4 f0 34 12 00 00[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 31[ 	]*vpopcntq zmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 72 7f[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 b2 00 20 00 00[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rdx\+0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 72 80[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rdx-0x2000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 48 55 b2 c0 df ff ff[ 	]*vpopcntq zmm30,ZMMWORD PTR \[rdx-0x2040\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 72 7f[ 	]*vpopcntq zmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 b2 00 04 00 00[ 	]*vpopcntq zmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 72 80[ 	]*vpopcntq zmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 58 55 b2 f8 fb ff ff[ 	]*vpopcntq zmm30,QWORD BCST \[rdx-0x408\]
#pass
