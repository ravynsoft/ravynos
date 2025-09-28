#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512CD/VL insns (Intel disassembly)
#source: x86-64-avx512cd_vl.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 c4 f5[ 	]*vpconflictd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f c4 f5[ 	]*vpconflictd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f c4 f5[ 	]*vpconflictd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 31[ 	]*vpconflictd xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 c4 b4 f0 23 01 00 00[ 	]*vpconflictd xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 31[ 	]*vpconflictd xmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 72 7f[ 	]*vpconflictd xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 b2 00 08 00 00[ 	]*vpconflictd xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 72 80[ 	]*vpconflictd xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 b2 f0 f7 ff ff[ 	]*vpconflictd xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 72 7f[ 	]*vpconflictd xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 b2 00 02 00 00[ 	]*vpconflictd xmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 72 80[ 	]*vpconflictd xmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 b2 fc fd ff ff[ 	]*vpconflictd xmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 c4 f5[ 	]*vpconflictd ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f c4 f5[ 	]*vpconflictd ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af c4 f5[ 	]*vpconflictd ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 31[ 	]*vpconflictd ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 c4 b4 f0 23 01 00 00[ 	]*vpconflictd ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 31[ 	]*vpconflictd ymm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 72 7f[ 	]*vpconflictd ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 b2 00 10 00 00[ 	]*vpconflictd ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 72 80[ 	]*vpconflictd ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 b2 e0 ef ff ff[ 	]*vpconflictd ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 72 7f[ 	]*vpconflictd ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 b2 00 02 00 00[ 	]*vpconflictd ymm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 72 80[ 	]*vpconflictd ymm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 b2 fc fd ff ff[ 	]*vpconflictd ymm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 c4 f5[ 	]*vpconflictq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f c4 f5[ 	]*vpconflictq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f c4 f5[ 	]*vpconflictq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 31[ 	]*vpconflictq xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 c4 b4 f0 23 01 00 00[ 	]*vpconflictq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 31[ 	]*vpconflictq xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 72 7f[ 	]*vpconflictq xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 b2 00 08 00 00[ 	]*vpconflictq xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 72 80[ 	]*vpconflictq xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 b2 f0 f7 ff ff[ 	]*vpconflictq xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 72 7f[ 	]*vpconflictq xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 b2 00 04 00 00[ 	]*vpconflictq xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 72 80[ 	]*vpconflictq xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 b2 f8 fb ff ff[ 	]*vpconflictq xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 c4 f5[ 	]*vpconflictq ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f c4 f5[ 	]*vpconflictq ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af c4 f5[ 	]*vpconflictq ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 31[ 	]*vpconflictq ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 c4 b4 f0 23 01 00 00[ 	]*vpconflictq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 31[ 	]*vpconflictq ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 72 7f[ 	]*vpconflictq ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 b2 00 10 00 00[ 	]*vpconflictq ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 72 80[ 	]*vpconflictq ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 b2 e0 ef ff ff[ 	]*vpconflictq ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 72 7f[ 	]*vpconflictq ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 b2 00 04 00 00[ 	]*vpconflictq ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 72 80[ 	]*vpconflictq ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 b2 f8 fb ff ff[ 	]*vpconflictq ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 44 f5[ 	]*vplzcntd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 44 f5[ 	]*vplzcntd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 44 f5[ 	]*vplzcntd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 31[ 	]*vplzcntd xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 44 b4 f0 23 01 00 00[ 	]*vplzcntd xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 31[ 	]*vplzcntd xmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 72 7f[ 	]*vplzcntd xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 b2 00 08 00 00[ 	]*vplzcntd xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 72 80[ 	]*vplzcntd xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 b2 f0 f7 ff ff[ 	]*vplzcntd xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 72 7f[ 	]*vplzcntd xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 b2 00 02 00 00[ 	]*vplzcntd xmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 72 80[ 	]*vplzcntd xmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 b2 fc fd ff ff[ 	]*vplzcntd xmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 44 f5[ 	]*vplzcntd ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 44 f5[ 	]*vplzcntd ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 44 f5[ 	]*vplzcntd ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 31[ 	]*vplzcntd ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 44 b4 f0 23 01 00 00[ 	]*vplzcntd ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 31[ 	]*vplzcntd ymm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 72 7f[ 	]*vplzcntd ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 b2 00 10 00 00[ 	]*vplzcntd ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 72 80[ 	]*vplzcntd ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 b2 e0 ef ff ff[ 	]*vplzcntd ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 72 7f[ 	]*vplzcntd ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 b2 00 02 00 00[ 	]*vplzcntd ymm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 72 80[ 	]*vplzcntd ymm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 b2 fc fd ff ff[ 	]*vplzcntd ymm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 44 f5[ 	]*vplzcntq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 44 f5[ 	]*vplzcntq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 44 f5[ 	]*vplzcntq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 31[ 	]*vplzcntq xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 44 b4 f0 23 01 00 00[ 	]*vplzcntq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 31[ 	]*vplzcntq xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 72 7f[ 	]*vplzcntq xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 b2 00 08 00 00[ 	]*vplzcntq xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 72 80[ 	]*vplzcntq xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 b2 f0 f7 ff ff[ 	]*vplzcntq xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 72 7f[ 	]*vplzcntq xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 b2 00 04 00 00[ 	]*vplzcntq xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 72 80[ 	]*vplzcntq xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 b2 f8 fb ff ff[ 	]*vplzcntq xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 44 f5[ 	]*vplzcntq ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 44 f5[ 	]*vplzcntq ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 44 f5[ 	]*vplzcntq ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 31[ 	]*vplzcntq ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 44 b4 f0 23 01 00 00[ 	]*vplzcntq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 31[ 	]*vplzcntq ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 72 7f[ 	]*vplzcntq ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 b2 00 10 00 00[ 	]*vplzcntq ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 72 80[ 	]*vplzcntq ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 b2 e0 ef ff ff[ 	]*vplzcntq ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 72 7f[ 	]*vplzcntq ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 b2 00 04 00 00[ 	]*vplzcntq ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 72 80[ 	]*vplzcntq ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 b2 f8 fb ff ff[ 	]*vplzcntq ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 3a f6[ 	]*vpbroadcastmw2d xmm30,k6
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 3a f6[ 	]*vpbroadcastmw2d ymm30,k6
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 08 2a f6[ 	]*vpbroadcastmb2q xmm30,k6
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 28 2a f6[ 	]*vpbroadcastmb2q ymm30,k6
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 c4 f5[ 	]*vpconflictd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f c4 f5[ 	]*vpconflictd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f c4 f5[ 	]*vpconflictd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 31[ 	]*vpconflictd xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 c4 b4 f0 34 12 00 00[ 	]*vpconflictd xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 31[ 	]*vpconflictd xmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 72 7f[ 	]*vpconflictd xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 b2 00 08 00 00[ 	]*vpconflictd xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 72 80[ 	]*vpconflictd xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 c4 b2 f0 f7 ff ff[ 	]*vpconflictd xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 72 7f[ 	]*vpconflictd xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 b2 00 02 00 00[ 	]*vpconflictd xmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 72 80[ 	]*vpconflictd xmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 c4 b2 fc fd ff ff[ 	]*vpconflictd xmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 c4 f5[ 	]*vpconflictd ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f c4 f5[ 	]*vpconflictd ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af c4 f5[ 	]*vpconflictd ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 31[ 	]*vpconflictd ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 c4 b4 f0 34 12 00 00[ 	]*vpconflictd ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 31[ 	]*vpconflictd ymm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 72 7f[ 	]*vpconflictd ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 b2 00 10 00 00[ 	]*vpconflictd ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 72 80[ 	]*vpconflictd ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 c4 b2 e0 ef ff ff[ 	]*vpconflictd ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 72 7f[ 	]*vpconflictd ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 b2 00 02 00 00[ 	]*vpconflictd ymm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 72 80[ 	]*vpconflictd ymm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 c4 b2 fc fd ff ff[ 	]*vpconflictd ymm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 c4 f5[ 	]*vpconflictq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f c4 f5[ 	]*vpconflictq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f c4 f5[ 	]*vpconflictq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 31[ 	]*vpconflictq xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 c4 b4 f0 34 12 00 00[ 	]*vpconflictq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 31[ 	]*vpconflictq xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 72 7f[ 	]*vpconflictq xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 b2 00 08 00 00[ 	]*vpconflictq xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 72 80[ 	]*vpconflictq xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 c4 b2 f0 f7 ff ff[ 	]*vpconflictq xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 72 7f[ 	]*vpconflictq xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 b2 00 04 00 00[ 	]*vpconflictq xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 72 80[ 	]*vpconflictq xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 c4 b2 f8 fb ff ff[ 	]*vpconflictq xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 c4 f5[ 	]*vpconflictq ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f c4 f5[ 	]*vpconflictq ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af c4 f5[ 	]*vpconflictq ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 31[ 	]*vpconflictq ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 c4 b4 f0 34 12 00 00[ 	]*vpconflictq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 31[ 	]*vpconflictq ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 72 7f[ 	]*vpconflictq ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 b2 00 10 00 00[ 	]*vpconflictq ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 72 80[ 	]*vpconflictq ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 c4 b2 e0 ef ff ff[ 	]*vpconflictq ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 72 7f[ 	]*vpconflictq ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 b2 00 04 00 00[ 	]*vpconflictq ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 72 80[ 	]*vpconflictq ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 c4 b2 f8 fb ff ff[ 	]*vpconflictq ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 44 f5[ 	]*vplzcntd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 44 f5[ 	]*vplzcntd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 44 f5[ 	]*vplzcntd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 31[ 	]*vplzcntd xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 44 b4 f0 34 12 00 00[ 	]*vplzcntd xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 31[ 	]*vplzcntd xmm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 72 7f[ 	]*vplzcntd xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 b2 00 08 00 00[ 	]*vplzcntd xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 72 80[ 	]*vplzcntd xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 44 b2 f0 f7 ff ff[ 	]*vplzcntd xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 72 7f[ 	]*vplzcntd xmm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 b2 00 02 00 00[ 	]*vplzcntd xmm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 72 80[ 	]*vplzcntd xmm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 18 44 b2 fc fd ff ff[ 	]*vplzcntd xmm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 44 f5[ 	]*vplzcntd ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 44 f5[ 	]*vplzcntd ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 44 f5[ 	]*vplzcntd ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 31[ 	]*vplzcntd ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 44 b4 f0 34 12 00 00[ 	]*vplzcntd ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 31[ 	]*vplzcntd ymm30,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 72 7f[ 	]*vplzcntd ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 b2 00 10 00 00[ 	]*vplzcntd ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 72 80[ 	]*vplzcntd ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 44 b2 e0 ef ff ff[ 	]*vplzcntd ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 72 7f[ 	]*vplzcntd ymm30,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 b2 00 02 00 00[ 	]*vplzcntd ymm30,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 72 80[ 	]*vplzcntd ymm30,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 38 44 b2 fc fd ff ff[ 	]*vplzcntd ymm30,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 44 f5[ 	]*vplzcntq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 44 f5[ 	]*vplzcntq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 44 f5[ 	]*vplzcntq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 31[ 	]*vplzcntq xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 44 b4 f0 34 12 00 00[ 	]*vplzcntq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 31[ 	]*vplzcntq xmm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 72 7f[ 	]*vplzcntq xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 b2 00 08 00 00[ 	]*vplzcntq xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 72 80[ 	]*vplzcntq xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 44 b2 f0 f7 ff ff[ 	]*vplzcntq xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 72 7f[ 	]*vplzcntq xmm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 b2 00 04 00 00[ 	]*vplzcntq xmm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 72 80[ 	]*vplzcntq xmm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 18 44 b2 f8 fb ff ff[ 	]*vplzcntq xmm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 44 f5[ 	]*vplzcntq ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 44 f5[ 	]*vplzcntq ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 44 f5[ 	]*vplzcntq ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 31[ 	]*vplzcntq ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 44 b4 f0 34 12 00 00[ 	]*vplzcntq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 31[ 	]*vplzcntq ymm30,QWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 72 7f[ 	]*vplzcntq ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 b2 00 10 00 00[ 	]*vplzcntq ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 72 80[ 	]*vplzcntq ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 44 b2 e0 ef ff ff[ 	]*vplzcntq ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 72 7f[ 	]*vplzcntq ymm30,QWORD BCST \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 b2 00 04 00 00[ 	]*vplzcntq ymm30,QWORD BCST \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 72 80[ 	]*vplzcntq ymm30,QWORD BCST \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 38 44 b2 f8 fb ff ff[ 	]*vplzcntq ymm30,QWORD BCST \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 3a f6[ 	]*vpbroadcastmw2d xmm30,k6
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 3a f6[ 	]*vpbroadcastmw2d ymm30,k6
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 08 2a f6[ 	]*vpbroadcastmb2q xmm30,k6
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 28 2a f6[ 	]*vpbroadcastmb2q ymm30,k6
#pass
