#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512BW/VL insns (Intel disassembly)
#source: x86-64-avx512bw_vl.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 1c f5[ 	]*vpabsb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 1c f5[ 	]*vpabsb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 1c f5[ 	]*vpabsb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1c 31[ 	]*vpabsb xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 1c b4 f0 23 01 00 00[ 	]*vpabsb xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1c 72 7f[ 	]*vpabsb xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1c b2 00 08 00 00[ 	]*vpabsb xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1c 72 80[ 	]*vpabsb xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1c b2 f0 f7 ff ff[ 	]*vpabsb xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 1c f5[ 	]*vpabsb ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 1c f5[ 	]*vpabsb ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 1c f5[ 	]*vpabsb ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1c 31[ 	]*vpabsb ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 1c b4 f0 23 01 00 00[ 	]*vpabsb ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1c 72 7f[ 	]*vpabsb ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1c b2 00 10 00 00[ 	]*vpabsb ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1c 72 80[ 	]*vpabsb ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1c b2 e0 ef ff ff[ 	]*vpabsb ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 1d f5[ 	]*vpabsw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 1d f5[ 	]*vpabsw xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 1d f5[ 	]*vpabsw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1d 31[ 	]*vpabsw xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 1d b4 f0 23 01 00 00[ 	]*vpabsw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1d 72 7f[ 	]*vpabsw xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1d b2 00 08 00 00[ 	]*vpabsw xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1d 72 80[ 	]*vpabsw xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1d b2 f0 f7 ff ff[ 	]*vpabsw xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 1d f5[ 	]*vpabsw ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 1d f5[ 	]*vpabsw ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 1d f5[ 	]*vpabsw ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1d 31[ 	]*vpabsw ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 1d b4 f0 23 01 00 00[ 	]*vpabsw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1d 72 7f[ 	]*vpabsw ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1d b2 00 10 00 00[ 	]*vpabsw ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1d 72 80[ 	]*vpabsw ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1d b2 e0 ef ff ff[ 	]*vpabsw ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 6b f4[ 	]*vpackssdw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 6b f4[ 	]*vpackssdw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 6b f4[ 	]*vpackssdw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 6b 31[ 	]*vpackssdw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 6b b4 f0 23 01 00 00[ 	]*vpackssdw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 10 6b 31[ 	]*vpackssdw xmm30,xmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 6b 72 7f[ 	]*vpackssdw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 6b b2 00 08 00 00[ 	]*vpackssdw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 6b 72 80[ 	]*vpackssdw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 6b b2 f0 f7 ff ff[ 	]*vpackssdw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 10 6b 72 7f[ 	]*vpackssdw xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 10 6b b2 00 02 00 00[ 	]*vpackssdw xmm30,xmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 10 6b 72 80[ 	]*vpackssdw xmm30,xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 10 6b b2 fc fd ff ff[ 	]*vpackssdw xmm30,xmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 6b f4[ 	]*vpackssdw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 6b f4[ 	]*vpackssdw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 6b f4[ 	]*vpackssdw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 6b 31[ 	]*vpackssdw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 6b b4 f0 23 01 00 00[ 	]*vpackssdw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 30 6b 31[ 	]*vpackssdw ymm30,ymm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 6b 72 7f[ 	]*vpackssdw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 6b b2 00 10 00 00[ 	]*vpackssdw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 6b 72 80[ 	]*vpackssdw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 6b b2 e0 ef ff ff[ 	]*vpackssdw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 30 6b 72 7f[ 	]*vpackssdw ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 30 6b b2 00 02 00 00[ 	]*vpackssdw ymm30,ymm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 30 6b 72 80[ 	]*vpackssdw ymm30,ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 30 6b b2 fc fd ff ff[ 	]*vpackssdw ymm30,ymm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 63 f4[ 	]*vpacksswb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 63 f4[ 	]*vpacksswb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 63 f4[ 	]*vpacksswb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 63 31[ 	]*vpacksswb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 63 b4 f0 23 01 00 00[ 	]*vpacksswb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 63 72 7f[ 	]*vpacksswb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 63 b2 00 08 00 00[ 	]*vpacksswb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 63 72 80[ 	]*vpacksswb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 63 b2 f0 f7 ff ff[ 	]*vpacksswb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 63 f4[ 	]*vpacksswb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 63 f4[ 	]*vpacksswb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 63 f4[ 	]*vpacksswb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 63 31[ 	]*vpacksswb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 63 b4 f0 23 01 00 00[ 	]*vpacksswb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 63 72 7f[ 	]*vpacksswb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 63 b2 00 10 00 00[ 	]*vpacksswb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 63 72 80[ 	]*vpacksswb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 63 b2 e0 ef ff ff[ 	]*vpacksswb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 2b f4[ 	]*vpackusdw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 2b f4[ 	]*vpackusdw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 2b f4[ 	]*vpackusdw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 2b 31[ 	]*vpackusdw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 2b b4 f0 23 01 00 00[ 	]*vpackusdw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 2b 31[ 	]*vpackusdw xmm30,xmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 2b 72 7f[ 	]*vpackusdw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 2b b2 00 08 00 00[ 	]*vpackusdw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 2b 72 80[ 	]*vpackusdw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 2b b2 f0 f7 ff ff[ 	]*vpackusdw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 2b 72 7f[ 	]*vpackusdw xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 2b b2 00 02 00 00[ 	]*vpackusdw xmm30,xmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 2b 72 80[ 	]*vpackusdw xmm30,xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 2b b2 fc fd ff ff[ 	]*vpackusdw xmm30,xmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 2b f4[ 	]*vpackusdw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 2b f4[ 	]*vpackusdw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 2b f4[ 	]*vpackusdw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 2b 31[ 	]*vpackusdw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 2b b4 f0 23 01 00 00[ 	]*vpackusdw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 2b 31[ 	]*vpackusdw ymm30,ymm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 2b 72 7f[ 	]*vpackusdw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 2b b2 00 10 00 00[ 	]*vpackusdw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 2b 72 80[ 	]*vpackusdw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 2b b2 e0 ef ff ff[ 	]*vpackusdw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 2b 72 7f[ 	]*vpackusdw ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 2b b2 00 02 00 00[ 	]*vpackusdw ymm30,ymm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 2b 72 80[ 	]*vpackusdw ymm30,ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 2b b2 fc fd ff ff[ 	]*vpackusdw ymm30,ymm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 67 f4[ 	]*vpackuswb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 67 f4[ 	]*vpackuswb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 67 f4[ 	]*vpackuswb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 67 31[ 	]*vpackuswb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 67 b4 f0 23 01 00 00[ 	]*vpackuswb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 67 72 7f[ 	]*vpackuswb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 67 b2 00 08 00 00[ 	]*vpackuswb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 67 72 80[ 	]*vpackuswb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 67 b2 f0 f7 ff ff[ 	]*vpackuswb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 67 f4[ 	]*vpackuswb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 67 f4[ 	]*vpackuswb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 67 f4[ 	]*vpackuswb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 67 31[ 	]*vpackuswb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 67 b4 f0 23 01 00 00[ 	]*vpackuswb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 67 72 7f[ 	]*vpackuswb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 67 b2 00 10 00 00[ 	]*vpackuswb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 67 72 80[ 	]*vpackuswb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 67 b2 e0 ef ff ff[ 	]*vpackuswb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 fc f4[ 	]*vpaddb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 fc f4[ 	]*vpaddb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 fc f4[ 	]*vpaddb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fc 31[ 	]*vpaddb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 fc b4 f0 23 01 00 00[ 	]*vpaddb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fc 72 7f[ 	]*vpaddb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fc b2 00 08 00 00[ 	]*vpaddb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fc 72 80[ 	]*vpaddb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fc b2 f0 f7 ff ff[ 	]*vpaddb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 fc f4[ 	]*vpaddb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 fc f4[ 	]*vpaddb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 fc f4[ 	]*vpaddb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fc 31[ 	]*vpaddb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 fc b4 f0 23 01 00 00[ 	]*vpaddb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fc 72 7f[ 	]*vpaddb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fc b2 00 10 00 00[ 	]*vpaddb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fc 72 80[ 	]*vpaddb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fc b2 e0 ef ff ff[ 	]*vpaddb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 ec f4[ 	]*vpaddsb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 ec f4[ 	]*vpaddsb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 ec f4[ 	]*vpaddsb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ec 31[ 	]*vpaddsb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 ec b4 f0 23 01 00 00[ 	]*vpaddsb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ec 72 7f[ 	]*vpaddsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ec b2 00 08 00 00[ 	]*vpaddsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ec 72 80[ 	]*vpaddsb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ec b2 f0 f7 ff ff[ 	]*vpaddsb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 ec f4[ 	]*vpaddsb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 ec f4[ 	]*vpaddsb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 ec f4[ 	]*vpaddsb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ec 31[ 	]*vpaddsb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 ec b4 f0 23 01 00 00[ 	]*vpaddsb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ec 72 7f[ 	]*vpaddsb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ec b2 00 10 00 00[ 	]*vpaddsb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ec 72 80[ 	]*vpaddsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ec b2 e0 ef ff ff[ 	]*vpaddsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 ed f4[ 	]*vpaddsw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 ed f4[ 	]*vpaddsw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 ed f4[ 	]*vpaddsw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ed 31[ 	]*vpaddsw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 ed b4 f0 23 01 00 00[ 	]*vpaddsw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ed 72 7f[ 	]*vpaddsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ed b2 00 08 00 00[ 	]*vpaddsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ed 72 80[ 	]*vpaddsw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ed b2 f0 f7 ff ff[ 	]*vpaddsw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 ed f4[ 	]*vpaddsw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 ed f4[ 	]*vpaddsw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 ed f4[ 	]*vpaddsw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ed 31[ 	]*vpaddsw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 ed b4 f0 23 01 00 00[ 	]*vpaddsw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ed 72 7f[ 	]*vpaddsw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ed b2 00 10 00 00[ 	]*vpaddsw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ed 72 80[ 	]*vpaddsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ed b2 e0 ef ff ff[ 	]*vpaddsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 dc f4[ 	]*vpaddusb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 dc f4[ 	]*vpaddusb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 dc f4[ 	]*vpaddusb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dc 31[ 	]*vpaddusb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 dc b4 f0 23 01 00 00[ 	]*vpaddusb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dc 72 7f[ 	]*vpaddusb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dc b2 00 08 00 00[ 	]*vpaddusb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dc 72 80[ 	]*vpaddusb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dc b2 f0 f7 ff ff[ 	]*vpaddusb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 dc f4[ 	]*vpaddusb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 dc f4[ 	]*vpaddusb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 dc f4[ 	]*vpaddusb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dc 31[ 	]*vpaddusb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 dc b4 f0 23 01 00 00[ 	]*vpaddusb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dc 72 7f[ 	]*vpaddusb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dc b2 00 10 00 00[ 	]*vpaddusb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dc 72 80[ 	]*vpaddusb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dc b2 e0 ef ff ff[ 	]*vpaddusb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 dd f4[ 	]*vpaddusw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 dd f4[ 	]*vpaddusw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 dd f4[ 	]*vpaddusw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dd 31[ 	]*vpaddusw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 dd b4 f0 23 01 00 00[ 	]*vpaddusw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dd 72 7f[ 	]*vpaddusw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dd b2 00 08 00 00[ 	]*vpaddusw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dd 72 80[ 	]*vpaddusw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dd b2 f0 f7 ff ff[ 	]*vpaddusw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 dd f4[ 	]*vpaddusw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 dd f4[ 	]*vpaddusw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 dd f4[ 	]*vpaddusw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dd 31[ 	]*vpaddusw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 dd b4 f0 23 01 00 00[ 	]*vpaddusw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dd 72 7f[ 	]*vpaddusw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dd b2 00 10 00 00[ 	]*vpaddusw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dd 72 80[ 	]*vpaddusw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dd b2 e0 ef ff ff[ 	]*vpaddusw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 fd f4[ 	]*vpaddw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 fd f4[ 	]*vpaddw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 fd f4[ 	]*vpaddw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fd 31[ 	]*vpaddw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 fd b4 f0 23 01 00 00[ 	]*vpaddw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fd 72 7f[ 	]*vpaddw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fd b2 00 08 00 00[ 	]*vpaddw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fd 72 80[ 	]*vpaddw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fd b2 f0 f7 ff ff[ 	]*vpaddw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 fd f4[ 	]*vpaddw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 fd f4[ 	]*vpaddw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 fd f4[ 	]*vpaddw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fd 31[ 	]*vpaddw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 fd b4 f0 23 01 00 00[ 	]*vpaddw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fd 72 7f[ 	]*vpaddw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fd b2 00 10 00 00[ 	]*vpaddw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fd 72 80[ 	]*vpaddw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fd b2 e0 ef ff ff[ 	]*vpaddw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 0f f4 ab[ 	]*vpalignr xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 0f f4 ab[ 	]*vpalignr xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 0f f4 ab[ 	]*vpalignr xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 0f f4 7b[ 	]*vpalignr xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 0f 31 7b[ 	]*vpalignr xmm30,xmm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 0f b4 f0 23 01 00 00 7b[ 	]*vpalignr xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 0f 72 7f 7b[ 	]*vpalignr xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 0f b2 00 08 00 00 7b[ 	]*vpalignr xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 0f 72 80 7b[ 	]*vpalignr xmm30,xmm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 0f b2 f0 f7 ff ff 7b[ 	]*vpalignr xmm30,xmm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 0f f4 ab[ 	]*vpalignr ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 0f f4 ab[ 	]*vpalignr ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 0f f4 ab[ 	]*vpalignr ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 0f f4 7b[ 	]*vpalignr ymm30,ymm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 0f 31 7b[ 	]*vpalignr ymm30,ymm29,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 0f b4 f0 23 01 00 00 7b[ 	]*vpalignr ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 0f 72 7f 7b[ 	]*vpalignr ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 0f b2 00 10 00 00 7b[ 	]*vpalignr ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 0f 72 80 7b[ 	]*vpalignr ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 0f b2 e0 ef ff ff 7b[ 	]*vpalignr ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 e0 f4[ 	]*vpavgb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 e0 f4[ 	]*vpavgb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 e0 f4[ 	]*vpavgb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e0 31[ 	]*vpavgb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 e0 b4 f0 23 01 00 00[ 	]*vpavgb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e0 72 7f[ 	]*vpavgb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e0 b2 00 08 00 00[ 	]*vpavgb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e0 72 80[ 	]*vpavgb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e0 b2 f0 f7 ff ff[ 	]*vpavgb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 e0 f4[ 	]*vpavgb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 e0 f4[ 	]*vpavgb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 e0 f4[ 	]*vpavgb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e0 31[ 	]*vpavgb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 e0 b4 f0 23 01 00 00[ 	]*vpavgb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e0 72 7f[ 	]*vpavgb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e0 b2 00 10 00 00[ 	]*vpavgb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e0 72 80[ 	]*vpavgb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e0 b2 e0 ef ff ff[ 	]*vpavgb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 e3 f4[ 	]*vpavgw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 e3 f4[ 	]*vpavgw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 e3 f4[ 	]*vpavgw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e3 31[ 	]*vpavgw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 e3 b4 f0 23 01 00 00[ 	]*vpavgw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e3 72 7f[ 	]*vpavgw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e3 b2 00 08 00 00[ 	]*vpavgw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e3 72 80[ 	]*vpavgw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e3 b2 f0 f7 ff ff[ 	]*vpavgw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 e3 f4[ 	]*vpavgw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 e3 f4[ 	]*vpavgw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 e3 f4[ 	]*vpavgw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e3 31[ 	]*vpavgw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 e3 b4 f0 23 01 00 00[ 	]*vpavgw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e3 72 7f[ 	]*vpavgw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e3 b2 00 10 00 00[ 	]*vpavgw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e3 72 80[ 	]*vpavgw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e3 b2 e0 ef ff ff[ 	]*vpavgw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 66 f4[ 	]*vpblendmb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 66 f4[ 	]*vpblendmb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 66 f4[ 	]*vpblendmb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 66 31[ 	]*vpblendmb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 66 b4 f0 23 01 00 00[ 	]*vpblendmb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 66 72 7f[ 	]*vpblendmb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 66 b2 00 08 00 00[ 	]*vpblendmb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 66 72 80[ 	]*vpblendmb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 66 b2 f0 f7 ff ff[ 	]*vpblendmb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 66 f4[ 	]*vpblendmb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 66 f4[ 	]*vpblendmb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 66 f4[ 	]*vpblendmb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 66 31[ 	]*vpblendmb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 66 b4 f0 23 01 00 00[ 	]*vpblendmb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 66 72 7f[ 	]*vpblendmb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 66 b2 00 10 00 00[ 	]*vpblendmb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 66 72 80[ 	]*vpblendmb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 66 b2 e0 ef ff ff[ 	]*vpblendmb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 78 f5[ 	]*vpbroadcastb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 78 f5[ 	]*vpbroadcastb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 78 f5[ 	]*vpbroadcastb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 78 31[ 	]*vpbroadcastb xmm30,BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 78 b4 f0 23 01 00 00[ 	]*vpbroadcastb xmm30,BYTE PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 78 72 7f[ 	]*vpbroadcastb xmm30,BYTE PTR \[rdx\+0x7f\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 78 b2 80 00 00 00[ 	]*vpbroadcastb xmm30,BYTE PTR \[rdx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 78 72 80[ 	]*vpbroadcastb xmm30,BYTE PTR \[rdx-0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 78 b2 7f ff ff ff[ 	]*vpbroadcastb xmm30,BYTE PTR \[rdx-0x81\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 78 f5[ 	]*vpbroadcastb ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 78 f5[ 	]*vpbroadcastb ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 78 f5[ 	]*vpbroadcastb ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 78 31[ 	]*vpbroadcastb ymm30,BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 78 b4 f0 23 01 00 00[ 	]*vpbroadcastb ymm30,BYTE PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 78 72 7f[ 	]*vpbroadcastb ymm30,BYTE PTR \[rdx\+0x7f\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 78 b2 80 00 00 00[ 	]*vpbroadcastb ymm30,BYTE PTR \[rdx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 78 72 80[ 	]*vpbroadcastb ymm30,BYTE PTR \[rdx-0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 78 b2 7f ff ff ff[ 	]*vpbroadcastb ymm30,BYTE PTR \[rdx-0x81\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 7a f0[ 	]*vpbroadcastb xmm30,eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 0f 7a f0[ 	]*vpbroadcastb xmm30\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 8f 7a f0[ 	]*vpbroadcastb xmm30\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 7a f0[ 	]*vpbroadcastb ymm30,eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 2f 7a f0[ 	]*vpbroadcastb ymm30\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d af 7a f0[ 	]*vpbroadcastb ymm30\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 79 f5[ 	]*vpbroadcastw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 79 f5[ 	]*vpbroadcastw xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 79 f5[ 	]*vpbroadcastw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 79 31[ 	]*vpbroadcastw xmm30,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 79 b4 f0 23 01 00 00[ 	]*vpbroadcastw xmm30,WORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 79 72 7f[ 	]*vpbroadcastw xmm30,WORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 79 b2 00 01 00 00[ 	]*vpbroadcastw xmm30,WORD PTR \[rdx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 79 72 80[ 	]*vpbroadcastw xmm30,WORD PTR \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 79 b2 fe fe ff ff[ 	]*vpbroadcastw xmm30,WORD PTR \[rdx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 79 f5[ 	]*vpbroadcastw ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 79 f5[ 	]*vpbroadcastw ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 79 f5[ 	]*vpbroadcastw ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 79 31[ 	]*vpbroadcastw ymm30,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 79 b4 f0 23 01 00 00[ 	]*vpbroadcastw ymm30,WORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 79 72 7f[ 	]*vpbroadcastw ymm30,WORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 79 b2 00 01 00 00[ 	]*vpbroadcastw ymm30,WORD PTR \[rdx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 79 72 80[ 	]*vpbroadcastw ymm30,WORD PTR \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 79 b2 fe fe ff ff[ 	]*vpbroadcastw ymm30,WORD PTR \[rdx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 7b f0[ 	]*vpbroadcastw xmm30,eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 0f 7b f0[ 	]*vpbroadcastw xmm30\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 8f 7b f0[ 	]*vpbroadcastw xmm30\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 7b f0[ 	]*vpbroadcastw ymm30,eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 2f 7b f0[ 	]*vpbroadcastw ymm30\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d af 7b f0[ 	]*vpbroadcastw ymm30\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 74 ed[ 	]*vpcmpeqb k5,xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 07 74 ed[ 	]*vpcmpeqb k5\{k7\},xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 74 29[ 	]*vpcmpeqb k5,xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 74 ac f0 23 01 00 00[ 	]*vpcmpeqb k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 74 6a 7f[ 	]*vpcmpeqb k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 74 aa 00 08 00 00[ 	]*vpcmpeqb k5,xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 74 6a 80[ 	]*vpcmpeqb k5,xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 74 aa f0 f7 ff ff[ 	]*vpcmpeqb k5,xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 74 ed[ 	]*vpcmpeqb k5,ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 27 74 ed[ 	]*vpcmpeqb k5\{k7\},ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 74 29[ 	]*vpcmpeqb k5,ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 74 ac f0 23 01 00 00[ 	]*vpcmpeqb k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 74 6a 7f[ 	]*vpcmpeqb k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 74 aa 00 10 00 00[ 	]*vpcmpeqb k5,ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 74 6a 80[ 	]*vpcmpeqb k5,ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 74 aa e0 ef ff ff[ 	]*vpcmpeqb k5,ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 75 ed[ 	]*vpcmpeqw k5,xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 07 75 ed[ 	]*vpcmpeqw k5\{k7\},xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 75 29[ 	]*vpcmpeqw k5,xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 75 ac f0 23 01 00 00[ 	]*vpcmpeqw k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 75 6a 7f[ 	]*vpcmpeqw k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 75 aa 00 08 00 00[ 	]*vpcmpeqw k5,xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 75 6a 80[ 	]*vpcmpeqw k5,xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 75 aa f0 f7 ff ff[ 	]*vpcmpeqw k5,xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 75 ed[ 	]*vpcmpeqw k5,ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 27 75 ed[ 	]*vpcmpeqw k5\{k7\},ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 75 29[ 	]*vpcmpeqw k5,ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 75 ac f0 23 01 00 00[ 	]*vpcmpeqw k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 75 6a 7f[ 	]*vpcmpeqw k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 75 aa 00 10 00 00[ 	]*vpcmpeqw k5,ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 75 6a 80[ 	]*vpcmpeqw k5,ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 75 aa e0 ef ff ff[ 	]*vpcmpeqw k5,ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 64 ed[ 	]*vpcmpgtb k5,xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 07 64 ed[ 	]*vpcmpgtb k5\{k7\},xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 64 29[ 	]*vpcmpgtb k5,xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 64 ac f0 23 01 00 00[ 	]*vpcmpgtb k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 64 6a 7f[ 	]*vpcmpgtb k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 64 aa 00 08 00 00[ 	]*vpcmpgtb k5,xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 64 6a 80[ 	]*vpcmpgtb k5,xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 64 aa f0 f7 ff ff[ 	]*vpcmpgtb k5,xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 64 ed[ 	]*vpcmpgtb k5,ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 27 64 ed[ 	]*vpcmpgtb k5\{k7\},ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 64 29[ 	]*vpcmpgtb k5,ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 64 ac f0 23 01 00 00[ 	]*vpcmpgtb k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 64 6a 7f[ 	]*vpcmpgtb k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 64 aa 00 10 00 00[ 	]*vpcmpgtb k5,ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 64 6a 80[ 	]*vpcmpgtb k5,ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 64 aa e0 ef ff ff[ 	]*vpcmpgtb k5,ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 65 ed[ 	]*vpcmpgtw k5,xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 07 65 ed[ 	]*vpcmpgtw k5\{k7\},xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 65 29[ 	]*vpcmpgtw k5,xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 65 ac f0 23 01 00 00[ 	]*vpcmpgtw k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 65 6a 7f[ 	]*vpcmpgtw k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 65 aa 00 08 00 00[ 	]*vpcmpgtw k5,xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 65 6a 80[ 	]*vpcmpgtw k5,xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 65 aa f0 f7 ff ff[ 	]*vpcmpgtw k5,xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 65 ed[ 	]*vpcmpgtw k5,ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 27 65 ed[ 	]*vpcmpgtw k5\{k7\},ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 65 29[ 	]*vpcmpgtw k5,ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 65 ac f0 23 01 00 00[ 	]*vpcmpgtw k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 65 6a 7f[ 	]*vpcmpgtw k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 65 aa 00 10 00 00[ 	]*vpcmpgtw k5,ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 65 6a 80[ 	]*vpcmpgtw k5,ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 65 aa e0 ef ff ff[ 	]*vpcmpgtw k5,ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 66 f4[ 	]*vpblendmw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 66 f4[ 	]*vpblendmw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 66 f4[ 	]*vpblendmw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 66 31[ 	]*vpblendmw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 66 b4 f0 23 01 00 00[ 	]*vpblendmw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 66 72 7f[ 	]*vpblendmw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 66 b2 00 08 00 00[ 	]*vpblendmw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 66 72 80[ 	]*vpblendmw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 66 b2 f0 f7 ff ff[ 	]*vpblendmw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 66 f4[ 	]*vpblendmw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 66 f4[ 	]*vpblendmw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 66 f4[ 	]*vpblendmw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 66 31[ 	]*vpblendmw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 66 b4 f0 23 01 00 00[ 	]*vpblendmw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 66 72 7f[ 	]*vpblendmw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 66 b2 00 10 00 00[ 	]*vpblendmw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 66 72 80[ 	]*vpblendmw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 66 b2 e0 ef ff ff[ 	]*vpblendmw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 04 f4[ 	]*vpmaddubsw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 04 f4[ 	]*vpmaddubsw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 04 f4[ 	]*vpmaddubsw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 04 31[ 	]*vpmaddubsw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 04 b4 f0 23 01 00 00[ 	]*vpmaddubsw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 04 72 7f[ 	]*vpmaddubsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 04 b2 00 08 00 00[ 	]*vpmaddubsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 04 72 80[ 	]*vpmaddubsw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 04 b2 f0 f7 ff ff[ 	]*vpmaddubsw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 04 f4[ 	]*vpmaddubsw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 04 f4[ 	]*vpmaddubsw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 04 f4[ 	]*vpmaddubsw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 04 31[ 	]*vpmaddubsw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 04 b4 f0 23 01 00 00[ 	]*vpmaddubsw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 04 72 7f[ 	]*vpmaddubsw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 04 b2 00 10 00 00[ 	]*vpmaddubsw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 04 72 80[ 	]*vpmaddubsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 04 b2 e0 ef ff ff[ 	]*vpmaddubsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 f5 f4[ 	]*vpmaddwd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 f5 f4[ 	]*vpmaddwd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 f5 f4[ 	]*vpmaddwd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f5 31[ 	]*vpmaddwd xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 f5 b4 f0 23 01 00 00[ 	]*vpmaddwd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f5 72 7f[ 	]*vpmaddwd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f5 b2 00 08 00 00[ 	]*vpmaddwd xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f5 72 80[ 	]*vpmaddwd xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f5 b2 f0 f7 ff ff[ 	]*vpmaddwd xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 f5 f4[ 	]*vpmaddwd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 f5 f4[ 	]*vpmaddwd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 f5 f4[ 	]*vpmaddwd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f5 31[ 	]*vpmaddwd ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 f5 b4 f0 23 01 00 00[ 	]*vpmaddwd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f5 72 7f[ 	]*vpmaddwd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f5 b2 00 10 00 00[ 	]*vpmaddwd ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f5 72 80[ 	]*vpmaddwd ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f5 b2 e0 ef ff ff[ 	]*vpmaddwd ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 3c f4[ 	]*vpmaxsb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 3c f4[ 	]*vpmaxsb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 3c f4[ 	]*vpmaxsb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3c 31[ 	]*vpmaxsb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 3c b4 f0 23 01 00 00[ 	]*vpmaxsb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3c 72 7f[ 	]*vpmaxsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3c b2 00 08 00 00[ 	]*vpmaxsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3c 72 80[ 	]*vpmaxsb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3c b2 f0 f7 ff ff[ 	]*vpmaxsb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 3c f4[ 	]*vpmaxsb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 3c f4[ 	]*vpmaxsb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 3c f4[ 	]*vpmaxsb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3c 31[ 	]*vpmaxsb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 3c b4 f0 23 01 00 00[ 	]*vpmaxsb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3c 72 7f[ 	]*vpmaxsb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3c b2 00 10 00 00[ 	]*vpmaxsb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3c 72 80[ 	]*vpmaxsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3c b2 e0 ef ff ff[ 	]*vpmaxsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 ee f4[ 	]*vpmaxsw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 ee f4[ 	]*vpmaxsw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 ee f4[ 	]*vpmaxsw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ee 31[ 	]*vpmaxsw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 ee b4 f0 23 01 00 00[ 	]*vpmaxsw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ee 72 7f[ 	]*vpmaxsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ee b2 00 08 00 00[ 	]*vpmaxsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ee 72 80[ 	]*vpmaxsw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ee b2 f0 f7 ff ff[ 	]*vpmaxsw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 ee f4[ 	]*vpmaxsw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 ee f4[ 	]*vpmaxsw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 ee f4[ 	]*vpmaxsw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ee 31[ 	]*vpmaxsw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 ee b4 f0 23 01 00 00[ 	]*vpmaxsw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ee 72 7f[ 	]*vpmaxsw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ee b2 00 10 00 00[ 	]*vpmaxsw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ee 72 80[ 	]*vpmaxsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ee b2 e0 ef ff ff[ 	]*vpmaxsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 de f4[ 	]*vpmaxub xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 de f4[ 	]*vpmaxub xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 de f4[ 	]*vpmaxub xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 de 31[ 	]*vpmaxub xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 de b4 f0 23 01 00 00[ 	]*vpmaxub xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 de 72 7f[ 	]*vpmaxub xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 de b2 00 08 00 00[ 	]*vpmaxub xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 de 72 80[ 	]*vpmaxub xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 de b2 f0 f7 ff ff[ 	]*vpmaxub xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 de f4[ 	]*vpmaxub ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 de f4[ 	]*vpmaxub ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 de f4[ 	]*vpmaxub ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 de 31[ 	]*vpmaxub ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 de b4 f0 23 01 00 00[ 	]*vpmaxub ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 de 72 7f[ 	]*vpmaxub ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 de b2 00 10 00 00[ 	]*vpmaxub ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 de 72 80[ 	]*vpmaxub ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 de b2 e0 ef ff ff[ 	]*vpmaxub ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 3e f4[ 	]*vpmaxuw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 3e f4[ 	]*vpmaxuw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 3e f4[ 	]*vpmaxuw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3e 31[ 	]*vpmaxuw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 3e b4 f0 23 01 00 00[ 	]*vpmaxuw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3e 72 7f[ 	]*vpmaxuw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3e b2 00 08 00 00[ 	]*vpmaxuw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3e 72 80[ 	]*vpmaxuw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3e b2 f0 f7 ff ff[ 	]*vpmaxuw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 3e f4[ 	]*vpmaxuw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 3e f4[ 	]*vpmaxuw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 3e f4[ 	]*vpmaxuw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3e 31[ 	]*vpmaxuw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 3e b4 f0 23 01 00 00[ 	]*vpmaxuw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3e 72 7f[ 	]*vpmaxuw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3e b2 00 10 00 00[ 	]*vpmaxuw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3e 72 80[ 	]*vpmaxuw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3e b2 e0 ef ff ff[ 	]*vpmaxuw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 38 f4[ 	]*vpminsb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 38 f4[ 	]*vpminsb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 38 f4[ 	]*vpminsb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 38 31[ 	]*vpminsb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 38 b4 f0 23 01 00 00[ 	]*vpminsb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 38 72 7f[ 	]*vpminsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 38 b2 00 08 00 00[ 	]*vpminsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 38 72 80[ 	]*vpminsb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 38 b2 f0 f7 ff ff[ 	]*vpminsb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 38 f4[ 	]*vpminsb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 38 f4[ 	]*vpminsb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 38 f4[ 	]*vpminsb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 38 31[ 	]*vpminsb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 38 b4 f0 23 01 00 00[ 	]*vpminsb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 38 72 7f[ 	]*vpminsb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 38 b2 00 10 00 00[ 	]*vpminsb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 38 72 80[ 	]*vpminsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 38 b2 e0 ef ff ff[ 	]*vpminsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 ea f4[ 	]*vpminsw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 ea f4[ 	]*vpminsw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 ea f4[ 	]*vpminsw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ea 31[ 	]*vpminsw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 ea b4 f0 23 01 00 00[ 	]*vpminsw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ea 72 7f[ 	]*vpminsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ea b2 00 08 00 00[ 	]*vpminsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ea 72 80[ 	]*vpminsw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ea b2 f0 f7 ff ff[ 	]*vpminsw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 ea f4[ 	]*vpminsw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 ea f4[ 	]*vpminsw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 ea f4[ 	]*vpminsw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ea 31[ 	]*vpminsw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 ea b4 f0 23 01 00 00[ 	]*vpminsw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ea 72 7f[ 	]*vpminsw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ea b2 00 10 00 00[ 	]*vpminsw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ea 72 80[ 	]*vpminsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ea b2 e0 ef ff ff[ 	]*vpminsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 da f4[ 	]*vpminub xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 da f4[ 	]*vpminub xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 da f4[ 	]*vpminub xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 da 31[ 	]*vpminub xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 da b4 f0 23 01 00 00[ 	]*vpminub xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 da 72 7f[ 	]*vpminub xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 da b2 00 08 00 00[ 	]*vpminub xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 da 72 80[ 	]*vpminub xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 da b2 f0 f7 ff ff[ 	]*vpminub xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 da f4[ 	]*vpminub ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 da f4[ 	]*vpminub ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 da f4[ 	]*vpminub ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 da 31[ 	]*vpminub ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 da b4 f0 23 01 00 00[ 	]*vpminub ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 da 72 7f[ 	]*vpminub ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 da b2 00 10 00 00[ 	]*vpminub ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 da 72 80[ 	]*vpminub ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 da b2 e0 ef ff ff[ 	]*vpminub ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 3a f4[ 	]*vpminuw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 3a f4[ 	]*vpminuw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 3a f4[ 	]*vpminuw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3a 31[ 	]*vpminuw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 3a b4 f0 23 01 00 00[ 	]*vpminuw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3a 72 7f[ 	]*vpminuw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3a b2 00 08 00 00[ 	]*vpminuw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3a 72 80[ 	]*vpminuw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3a b2 f0 f7 ff ff[ 	]*vpminuw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 3a f4[ 	]*vpminuw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 3a f4[ 	]*vpminuw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 3a f4[ 	]*vpminuw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3a 31[ 	]*vpminuw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 3a b4 f0 23 01 00 00[ 	]*vpminuw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3a 72 7f[ 	]*vpminuw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3a b2 00 10 00 00[ 	]*vpminuw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3a 72 80[ 	]*vpminuw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3a b2 e0 ef ff ff[ 	]*vpminuw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 20 f5[ 	]*vpmovsxbw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 20 f5[ 	]*vpmovsxbw xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 20 f5[ 	]*vpmovsxbw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 20 31[ 	]*vpmovsxbw xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 20 b4 f0 23 01 00 00[ 	]*vpmovsxbw xmm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 20 72 7f[ 	]*vpmovsxbw xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 20 b2 00 04 00 00[ 	]*vpmovsxbw xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 20 72 80[ 	]*vpmovsxbw xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 20 b2 f8 fb ff ff[ 	]*vpmovsxbw xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 20 f5[ 	]*vpmovsxbw ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 20 f5[ 	]*vpmovsxbw ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 20 f5[ 	]*vpmovsxbw ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 20 31[ 	]*vpmovsxbw ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 20 b4 f0 23 01 00 00[ 	]*vpmovsxbw ymm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 20 72 7f[ 	]*vpmovsxbw ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 20 b2 00 08 00 00[ 	]*vpmovsxbw ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 20 72 80[ 	]*vpmovsxbw ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 20 b2 f0 f7 ff ff[ 	]*vpmovsxbw ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 30 f5[ 	]*vpmovzxbw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 30 f5[ 	]*vpmovzxbw xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 30 f5[ 	]*vpmovzxbw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 30 31[ 	]*vpmovzxbw xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 30 b4 f0 23 01 00 00[ 	]*vpmovzxbw xmm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 30 72 7f[ 	]*vpmovzxbw xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 30 b2 00 04 00 00[ 	]*vpmovzxbw xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 30 72 80[ 	]*vpmovzxbw xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 30 b2 f8 fb ff ff[ 	]*vpmovzxbw xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 30 f5[ 	]*vpmovzxbw ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 30 f5[ 	]*vpmovzxbw ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 30 f5[ 	]*vpmovzxbw ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 30 31[ 	]*vpmovzxbw ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 30 b4 f0 23 01 00 00[ 	]*vpmovzxbw ymm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 30 72 7f[ 	]*vpmovzxbw ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 30 b2 00 08 00 00[ 	]*vpmovzxbw ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 30 72 80[ 	]*vpmovzxbw ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 30 b2 f0 f7 ff ff[ 	]*vpmovzxbw ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 0b f4[ 	]*vpmulhrsw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 0b f4[ 	]*vpmulhrsw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 0b f4[ 	]*vpmulhrsw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 0b 31[ 	]*vpmulhrsw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 0b b4 f0 23 01 00 00[ 	]*vpmulhrsw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 0b 72 7f[ 	]*vpmulhrsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 0b b2 00 08 00 00[ 	]*vpmulhrsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 0b 72 80[ 	]*vpmulhrsw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 0b b2 f0 f7 ff ff[ 	]*vpmulhrsw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 0b f4[ 	]*vpmulhrsw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 0b f4[ 	]*vpmulhrsw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 0b f4[ 	]*vpmulhrsw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 0b 31[ 	]*vpmulhrsw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 0b b4 f0 23 01 00 00[ 	]*vpmulhrsw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 0b 72 7f[ 	]*vpmulhrsw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 0b b2 00 10 00 00[ 	]*vpmulhrsw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 0b 72 80[ 	]*vpmulhrsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 0b b2 e0 ef ff ff[ 	]*vpmulhrsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 e4 f4[ 	]*vpmulhuw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 e4 f4[ 	]*vpmulhuw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 e4 f4[ 	]*vpmulhuw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e4 31[ 	]*vpmulhuw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 e4 b4 f0 23 01 00 00[ 	]*vpmulhuw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e4 72 7f[ 	]*vpmulhuw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e4 b2 00 08 00 00[ 	]*vpmulhuw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e4 72 80[ 	]*vpmulhuw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e4 b2 f0 f7 ff ff[ 	]*vpmulhuw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 e4 f4[ 	]*vpmulhuw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 e4 f4[ 	]*vpmulhuw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 e4 f4[ 	]*vpmulhuw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e4 31[ 	]*vpmulhuw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 e4 b4 f0 23 01 00 00[ 	]*vpmulhuw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e4 72 7f[ 	]*vpmulhuw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e4 b2 00 10 00 00[ 	]*vpmulhuw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e4 72 80[ 	]*vpmulhuw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e4 b2 e0 ef ff ff[ 	]*vpmulhuw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 e5 f4[ 	]*vpmulhw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 e5 f4[ 	]*vpmulhw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 e5 f4[ 	]*vpmulhw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e5 31[ 	]*vpmulhw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 e5 b4 f0 23 01 00 00[ 	]*vpmulhw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e5 72 7f[ 	]*vpmulhw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e5 b2 00 08 00 00[ 	]*vpmulhw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e5 72 80[ 	]*vpmulhw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e5 b2 f0 f7 ff ff[ 	]*vpmulhw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 e5 f4[ 	]*vpmulhw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 e5 f4[ 	]*vpmulhw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 e5 f4[ 	]*vpmulhw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e5 31[ 	]*vpmulhw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 e5 b4 f0 23 01 00 00[ 	]*vpmulhw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e5 72 7f[ 	]*vpmulhw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e5 b2 00 10 00 00[ 	]*vpmulhw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e5 72 80[ 	]*vpmulhw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e5 b2 e0 ef ff ff[ 	]*vpmulhw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 d5 f4[ 	]*vpmullw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 d5 f4[ 	]*vpmullw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 d5 f4[ 	]*vpmullw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d5 31[ 	]*vpmullw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 d5 b4 f0 23 01 00 00[ 	]*vpmullw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d5 72 7f[ 	]*vpmullw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d5 b2 00 08 00 00[ 	]*vpmullw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d5 72 80[ 	]*vpmullw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d5 b2 f0 f7 ff ff[ 	]*vpmullw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 d5 f4[ 	]*vpmullw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 d5 f4[ 	]*vpmullw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 d5 f4[ 	]*vpmullw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d5 31[ 	]*vpmullw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 d5 b4 f0 23 01 00 00[ 	]*vpmullw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d5 72 7f[ 	]*vpmullw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d5 b2 00 10 00 00[ 	]*vpmullw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d5 72 80[ 	]*vpmullw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d5 b2 e0 ef ff ff[ 	]*vpmullw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 f6 f4[ 	]*vpsadbw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f6 31[ 	]*vpsadbw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 f6 b4 f0 23 01 00 00[ 	]*vpsadbw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f6 72 7f[ 	]*vpsadbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f6 b2 00 08 00 00[ 	]*vpsadbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f6 72 80[ 	]*vpsadbw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f6 b2 f0 f7 ff ff[ 	]*vpsadbw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 f6 f4[ 	]*vpsadbw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f6 31[ 	]*vpsadbw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 f6 b4 f0 23 01 00 00[ 	]*vpsadbw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f6 72 7f[ 	]*vpsadbw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f6 b2 00 10 00 00[ 	]*vpsadbw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f6 72 80[ 	]*vpsadbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f6 b2 e0 ef ff ff[ 	]*vpsadbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 00 f4[ 	]*vpshufb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 00 f4[ 	]*vpshufb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 00 f4[ 	]*vpshufb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 00 31[ 	]*vpshufb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 00 b4 f0 23 01 00 00[ 	]*vpshufb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 00 72 7f[ 	]*vpshufb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 00 b2 00 08 00 00[ 	]*vpshufb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 00 72 80[ 	]*vpshufb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 00 b2 f0 f7 ff ff[ 	]*vpshufb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 00 f4[ 	]*vpshufb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 00 f4[ 	]*vpshufb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 00 f4[ 	]*vpshufb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 00 31[ 	]*vpshufb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 00 b4 f0 23 01 00 00[ 	]*vpshufb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 00 72 7f[ 	]*vpshufb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 00 b2 00 10 00 00[ 	]*vpshufb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 00 72 80[ 	]*vpshufb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 00 b2 e0 ef ff ff[ 	]*vpshufb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 08 70 f5 ab[ 	]*vpshufhw xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 0f 70 f5 ab[ 	]*vpshufhw xmm30\{k7\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 8f 70 f5 ab[ 	]*vpshufhw xmm30\{k7\}\{z\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 08 70 f5 7b[ 	]*vpshufhw xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 08 70 31 7b[ 	]*vpshufhw xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 7e 08 70 b4 f0 23 01 00 00 7b[ 	]*vpshufhw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 08 70 72 7f 7b[ 	]*vpshufhw xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 08 70 b2 00 08 00 00 7b[ 	]*vpshufhw xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 08 70 72 80 7b[ 	]*vpshufhw xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 08 70 b2 f0 f7 ff ff 7b[ 	]*vpshufhw xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 28 70 f5 ab[ 	]*vpshufhw ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 2f 70 f5 ab[ 	]*vpshufhw ymm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e af 70 f5 ab[ 	]*vpshufhw ymm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 28 70 f5 7b[ 	]*vpshufhw ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 28 70 31 7b[ 	]*vpshufhw ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 7e 28 70 b4 f0 23 01 00 00 7b[ 	]*vpshufhw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 28 70 72 7f 7b[ 	]*vpshufhw ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 28 70 b2 00 10 00 00 7b[ 	]*vpshufhw ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 28 70 72 80 7b[ 	]*vpshufhw ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 28 70 b2 e0 ef ff ff 7b[ 	]*vpshufhw ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 08 70 f5 ab[ 	]*vpshuflw xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 0f 70 f5 ab[ 	]*vpshuflw xmm30\{k7\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 8f 70 f5 ab[ 	]*vpshuflw xmm30\{k7\}\{z\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 08 70 f5 7b[ 	]*vpshuflw xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 70 31 7b[ 	]*vpshuflw xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 08 70 b4 f0 23 01 00 00 7b[ 	]*vpshuflw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 70 72 7f 7b[ 	]*vpshuflw xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 70 b2 00 08 00 00 7b[ 	]*vpshuflw xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 70 72 80 7b[ 	]*vpshuflw xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 70 b2 f0 f7 ff ff 7b[ 	]*vpshuflw xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 28 70 f5 ab[ 	]*vpshuflw ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 2f 70 f5 ab[ 	]*vpshuflw ymm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f af 70 f5 ab[ 	]*vpshuflw ymm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 28 70 f5 7b[ 	]*vpshuflw ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 70 31 7b[ 	]*vpshuflw ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 28 70 b4 f0 23 01 00 00 7b[ 	]*vpshuflw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 70 72 7f 7b[ 	]*vpshuflw ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 70 b2 00 10 00 00 7b[ 	]*vpshuflw ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 70 72 80 7b[ 	]*vpshuflw ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 70 b2 e0 ef ff ff 7b[ 	]*vpshuflw ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 f1 f4[ 	]*vpsllw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 f1 f4[ 	]*vpsllw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 f1 f4[ 	]*vpsllw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f1 31[ 	]*vpsllw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 f1 b4 f0 23 01 00 00[ 	]*vpsllw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f1 72 7f[ 	]*vpsllw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f1 b2 00 08 00 00[ 	]*vpsllw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f1 72 80[ 	]*vpsllw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f1 b2 f0 f7 ff ff[ 	]*vpsllw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 f1 f4[ 	]*vpsllw ymm30,ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 f1 f4[ 	]*vpsllw ymm30\{k7\},ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 f1 f4[ 	]*vpsllw ymm30\{k7\}\{z\},ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f1 31[ 	]*vpsllw ymm30,ymm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 f1 b4 f0 23 01 00 00[ 	]*vpsllw ymm30,ymm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f1 72 7f[ 	]*vpsllw ymm30,ymm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f1 b2 00 08 00 00[ 	]*vpsllw ymm30,ymm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f1 72 80[ 	]*vpsllw ymm30,ymm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f1 b2 f0 f7 ff ff[ 	]*vpsllw ymm30,ymm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 e1 f4[ 	]*vpsraw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 e1 f4[ 	]*vpsraw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 e1 f4[ 	]*vpsraw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e1 31[ 	]*vpsraw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 e1 b4 f0 23 01 00 00[ 	]*vpsraw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e1 72 7f[ 	]*vpsraw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e1 b2 00 08 00 00[ 	]*vpsraw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e1 72 80[ 	]*vpsraw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e1 b2 f0 f7 ff ff[ 	]*vpsraw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 e1 f4[ 	]*vpsraw ymm30,ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 e1 f4[ 	]*vpsraw ymm30\{k7\},ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 e1 f4[ 	]*vpsraw ymm30\{k7\}\{z\},ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e1 31[ 	]*vpsraw ymm30,ymm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 e1 b4 f0 23 01 00 00[ 	]*vpsraw ymm30,ymm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e1 72 7f[ 	]*vpsraw ymm30,ymm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e1 b2 00 08 00 00[ 	]*vpsraw ymm30,ymm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e1 72 80[ 	]*vpsraw ymm30,ymm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e1 b2 f0 f7 ff ff[ 	]*vpsraw ymm30,ymm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 d1 f4[ 	]*vpsrlw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 d1 f4[ 	]*vpsrlw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 d1 f4[ 	]*vpsrlw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d1 31[ 	]*vpsrlw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 d1 b4 f0 23 01 00 00[ 	]*vpsrlw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d1 72 7f[ 	]*vpsrlw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d1 b2 00 08 00 00[ 	]*vpsrlw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d1 72 80[ 	]*vpsrlw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d1 b2 f0 f7 ff ff[ 	]*vpsrlw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 d1 f4[ 	]*vpsrlw ymm30,ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 d1 f4[ 	]*vpsrlw ymm30\{k7\},ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 d1 f4[ 	]*vpsrlw ymm30\{k7\}\{z\},ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d1 31[ 	]*vpsrlw ymm30,ymm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 d1 b4 f0 23 01 00 00[ 	]*vpsrlw ymm30,ymm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d1 72 7f[ 	]*vpsrlw ymm30,ymm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d1 b2 00 08 00 00[ 	]*vpsrlw ymm30,ymm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d1 72 80[ 	]*vpsrlw ymm30,ymm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d1 b2 f0 f7 ff ff[ 	]*vpsrlw ymm30,ymm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 73 dd ab[ 	]*vpsrldq xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 73 dd 7b[ 	]*vpsrldq xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 19 7b[ 	]*vpsrldq xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 73 9c f0 23 01 00 00 7b[ 	]*vpsrldq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 5a 7f 7b[ 	]*vpsrldq xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 9a 00 08 00 00 7b[ 	]*vpsrldq xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 5a 80 7b[ 	]*vpsrldq xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 9a f0 f7 ff ff 7b[ 	]*vpsrldq xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 73 dd ab[ 	]*vpsrldq ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 73 dd 7b[ 	]*vpsrldq ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 19 7b[ 	]*vpsrldq ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 73 9c f0 23 01 00 00 7b[ 	]*vpsrldq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 5a 7f 7b[ 	]*vpsrldq ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 9a 00 10 00 00 7b[ 	]*vpsrldq ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 5a 80 7b[ 	]*vpsrldq ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 9a e0 ef ff ff 7b[ 	]*vpsrldq ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 71 d5 ab[ 	]*vpsrlw xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 07 71 d5 ab[ 	]*vpsrlw xmm30\{k7\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 87 71 d5 ab[ 	]*vpsrlw xmm30\{k7\}\{z\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 71 d5 7b[ 	]*vpsrlw xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 11 7b[ 	]*vpsrlw xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 71 94 f0 23 01 00 00 7b[ 	]*vpsrlw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 52 7f 7b[ 	]*vpsrlw xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 92 00 08 00 00 7b[ 	]*vpsrlw xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 52 80 7b[ 	]*vpsrlw xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 92 f0 f7 ff ff 7b[ 	]*vpsrlw xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 71 d5 ab[ 	]*vpsrlw ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 27 71 d5 ab[ 	]*vpsrlw ymm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d a7 71 d5 ab[ 	]*vpsrlw ymm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 71 d5 7b[ 	]*vpsrlw ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 11 7b[ 	]*vpsrlw ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 71 94 f0 23 01 00 00 7b[ 	]*vpsrlw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 52 7f 7b[ 	]*vpsrlw ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 92 00 10 00 00 7b[ 	]*vpsrlw ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 52 80 7b[ 	]*vpsrlw ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 92 e0 ef ff ff 7b[ 	]*vpsrlw ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 71 e5 ab[ 	]*vpsraw xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 07 71 e5 ab[ 	]*vpsraw xmm30\{k7\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 87 71 e5 ab[ 	]*vpsraw xmm30\{k7\}\{z\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 71 e5 7b[ 	]*vpsraw xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 21 7b[ 	]*vpsraw xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 71 a4 f0 23 01 00 00 7b[ 	]*vpsraw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 62 7f 7b[ 	]*vpsraw xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 a2 00 08 00 00 7b[ 	]*vpsraw xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 62 80 7b[ 	]*vpsraw xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 a2 f0 f7 ff ff 7b[ 	]*vpsraw xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 71 e5 ab[ 	]*vpsraw ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 27 71 e5 ab[ 	]*vpsraw ymm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d a7 71 e5 ab[ 	]*vpsraw ymm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 71 e5 7b[ 	]*vpsraw ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 21 7b[ 	]*vpsraw ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 71 a4 f0 23 01 00 00 7b[ 	]*vpsraw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 62 7f 7b[ 	]*vpsraw ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 a2 00 10 00 00 7b[ 	]*vpsraw ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 62 80 7b[ 	]*vpsraw ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 a2 e0 ef ff ff 7b[ 	]*vpsraw ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 10 f4[ 	]*vpsrlvw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 10 f4[ 	]*vpsrlvw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 10 f4[ 	]*vpsrlvw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 10 31[ 	]*vpsrlvw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 10 b4 f0 23 01 00 00[ 	]*vpsrlvw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 10 72 7f[ 	]*vpsrlvw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 10 b2 00 08 00 00[ 	]*vpsrlvw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 10 72 80[ 	]*vpsrlvw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 10 b2 f0 f7 ff ff[ 	]*vpsrlvw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 10 f4[ 	]*vpsrlvw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 10 f4[ 	]*vpsrlvw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 10 f4[ 	]*vpsrlvw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 10 31[ 	]*vpsrlvw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 10 b4 f0 23 01 00 00[ 	]*vpsrlvw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 10 72 7f[ 	]*vpsrlvw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 10 b2 00 10 00 00[ 	]*vpsrlvw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 10 72 80[ 	]*vpsrlvw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 10 b2 e0 ef ff ff[ 	]*vpsrlvw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 11 f4[ 	]*vpsravw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 11 f4[ 	]*vpsravw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 11 f4[ 	]*vpsravw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 11 31[ 	]*vpsravw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 11 b4 f0 23 01 00 00[ 	]*vpsravw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 11 72 7f[ 	]*vpsravw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 11 b2 00 08 00 00[ 	]*vpsravw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 11 72 80[ 	]*vpsravw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 11 b2 f0 f7 ff ff[ 	]*vpsravw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 11 f4[ 	]*vpsravw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 11 f4[ 	]*vpsravw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 11 f4[ 	]*vpsravw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 11 31[ 	]*vpsravw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 11 b4 f0 23 01 00 00[ 	]*vpsravw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 11 72 7f[ 	]*vpsravw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 11 b2 00 10 00 00[ 	]*vpsravw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 11 72 80[ 	]*vpsravw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 11 b2 e0 ef ff ff[ 	]*vpsravw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 f8 f4[ 	]*vpsubb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 f8 f4[ 	]*vpsubb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 f8 f4[ 	]*vpsubb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f8 31[ 	]*vpsubb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 f8 b4 f0 23 01 00 00[ 	]*vpsubb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f8 72 7f[ 	]*vpsubb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f8 b2 00 08 00 00[ 	]*vpsubb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f8 72 80[ 	]*vpsubb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f8 b2 f0 f7 ff ff[ 	]*vpsubb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 f8 f4[ 	]*vpsubb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 f8 f4[ 	]*vpsubb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 f8 f4[ 	]*vpsubb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f8 31[ 	]*vpsubb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 f8 b4 f0 23 01 00 00[ 	]*vpsubb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f8 72 7f[ 	]*vpsubb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f8 b2 00 10 00 00[ 	]*vpsubb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f8 72 80[ 	]*vpsubb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f8 b2 e0 ef ff ff[ 	]*vpsubb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 e8 f4[ 	]*vpsubsb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 e8 f4[ 	]*vpsubsb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 e8 f4[ 	]*vpsubsb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e8 31[ 	]*vpsubsb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 e8 b4 f0 23 01 00 00[ 	]*vpsubsb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e8 72 7f[ 	]*vpsubsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e8 b2 00 08 00 00[ 	]*vpsubsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e8 72 80[ 	]*vpsubsb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e8 b2 f0 f7 ff ff[ 	]*vpsubsb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 e8 f4[ 	]*vpsubsb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 e8 f4[ 	]*vpsubsb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 e8 f4[ 	]*vpsubsb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e8 31[ 	]*vpsubsb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 e8 b4 f0 23 01 00 00[ 	]*vpsubsb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e8 72 7f[ 	]*vpsubsb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e8 b2 00 10 00 00[ 	]*vpsubsb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e8 72 80[ 	]*vpsubsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e8 b2 e0 ef ff ff[ 	]*vpsubsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 e9 f4[ 	]*vpsubsw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 e9 f4[ 	]*vpsubsw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 e9 f4[ 	]*vpsubsw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e9 31[ 	]*vpsubsw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 e9 b4 f0 23 01 00 00[ 	]*vpsubsw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e9 72 7f[ 	]*vpsubsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e9 b2 00 08 00 00[ 	]*vpsubsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e9 72 80[ 	]*vpsubsw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e9 b2 f0 f7 ff ff[ 	]*vpsubsw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 e9 f4[ 	]*vpsubsw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 e9 f4[ 	]*vpsubsw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 e9 f4[ 	]*vpsubsw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e9 31[ 	]*vpsubsw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 e9 b4 f0 23 01 00 00[ 	]*vpsubsw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e9 72 7f[ 	]*vpsubsw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e9 b2 00 10 00 00[ 	]*vpsubsw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e9 72 80[ 	]*vpsubsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e9 b2 e0 ef ff ff[ 	]*vpsubsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 d8 f4[ 	]*vpsubusb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 d8 f4[ 	]*vpsubusb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 d8 f4[ 	]*vpsubusb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d8 31[ 	]*vpsubusb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 d8 b4 f0 23 01 00 00[ 	]*vpsubusb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d8 72 7f[ 	]*vpsubusb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d8 b2 00 08 00 00[ 	]*vpsubusb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d8 72 80[ 	]*vpsubusb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d8 b2 f0 f7 ff ff[ 	]*vpsubusb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 d8 f4[ 	]*vpsubusb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 d8 f4[ 	]*vpsubusb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 d8 f4[ 	]*vpsubusb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d8 31[ 	]*vpsubusb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 d8 b4 f0 23 01 00 00[ 	]*vpsubusb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d8 72 7f[ 	]*vpsubusb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d8 b2 00 10 00 00[ 	]*vpsubusb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d8 72 80[ 	]*vpsubusb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d8 b2 e0 ef ff ff[ 	]*vpsubusb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 d9 f4[ 	]*vpsubusw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 d9 f4[ 	]*vpsubusw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 d9 f4[ 	]*vpsubusw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d9 31[ 	]*vpsubusw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 d9 b4 f0 23 01 00 00[ 	]*vpsubusw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d9 72 7f[ 	]*vpsubusw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d9 b2 00 08 00 00[ 	]*vpsubusw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d9 72 80[ 	]*vpsubusw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d9 b2 f0 f7 ff ff[ 	]*vpsubusw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 d9 f4[ 	]*vpsubusw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 d9 f4[ 	]*vpsubusw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 d9 f4[ 	]*vpsubusw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d9 31[ 	]*vpsubusw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 d9 b4 f0 23 01 00 00[ 	]*vpsubusw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d9 72 7f[ 	]*vpsubusw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d9 b2 00 10 00 00[ 	]*vpsubusw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d9 72 80[ 	]*vpsubusw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d9 b2 e0 ef ff ff[ 	]*vpsubusw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 f9 f4[ 	]*vpsubw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 f9 f4[ 	]*vpsubw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 f9 f4[ 	]*vpsubw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f9 31[ 	]*vpsubw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 f9 b4 f0 23 01 00 00[ 	]*vpsubw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f9 72 7f[ 	]*vpsubw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f9 b2 00 08 00 00[ 	]*vpsubw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f9 72 80[ 	]*vpsubw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f9 b2 f0 f7 ff ff[ 	]*vpsubw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 f9 f4[ 	]*vpsubw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 f9 f4[ 	]*vpsubw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 f9 f4[ 	]*vpsubw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f9 31[ 	]*vpsubw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 f9 b4 f0 23 01 00 00[ 	]*vpsubw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f9 72 7f[ 	]*vpsubw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f9 b2 00 10 00 00[ 	]*vpsubw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f9 72 80[ 	]*vpsubw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f9 b2 e0 ef ff ff[ 	]*vpsubw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 68 f4[ 	]*vpunpckhbw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 68 f4[ 	]*vpunpckhbw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 68 f4[ 	]*vpunpckhbw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 68 31[ 	]*vpunpckhbw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 68 b4 f0 23 01 00 00[ 	]*vpunpckhbw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 68 72 7f[ 	]*vpunpckhbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 68 b2 00 08 00 00[ 	]*vpunpckhbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 68 72 80[ 	]*vpunpckhbw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 68 b2 f0 f7 ff ff[ 	]*vpunpckhbw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 68 f4[ 	]*vpunpckhbw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 68 f4[ 	]*vpunpckhbw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 68 f4[ 	]*vpunpckhbw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 68 31[ 	]*vpunpckhbw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 68 b4 f0 23 01 00 00[ 	]*vpunpckhbw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 68 72 7f[ 	]*vpunpckhbw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 68 b2 00 10 00 00[ 	]*vpunpckhbw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 68 72 80[ 	]*vpunpckhbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 68 b2 e0 ef ff ff[ 	]*vpunpckhbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 69 f4[ 	]*vpunpckhwd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 69 f4[ 	]*vpunpckhwd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 69 f4[ 	]*vpunpckhwd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 69 31[ 	]*vpunpckhwd xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 69 b4 f0 23 01 00 00[ 	]*vpunpckhwd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 69 72 7f[ 	]*vpunpckhwd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 69 b2 00 08 00 00[ 	]*vpunpckhwd xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 69 72 80[ 	]*vpunpckhwd xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 69 b2 f0 f7 ff ff[ 	]*vpunpckhwd xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 69 f4[ 	]*vpunpckhwd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 69 f4[ 	]*vpunpckhwd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 69 f4[ 	]*vpunpckhwd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 69 31[ 	]*vpunpckhwd ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 69 b4 f0 23 01 00 00[ 	]*vpunpckhwd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 69 72 7f[ 	]*vpunpckhwd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 69 b2 00 10 00 00[ 	]*vpunpckhwd ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 69 72 80[ 	]*vpunpckhwd ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 69 b2 e0 ef ff ff[ 	]*vpunpckhwd ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 60 f4[ 	]*vpunpcklbw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 60 f4[ 	]*vpunpcklbw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 60 f4[ 	]*vpunpcklbw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 60 31[ 	]*vpunpcklbw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 60 b4 f0 23 01 00 00[ 	]*vpunpcklbw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 60 72 7f[ 	]*vpunpcklbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 60 b2 00 08 00 00[ 	]*vpunpcklbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 60 72 80[ 	]*vpunpcklbw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 60 b2 f0 f7 ff ff[ 	]*vpunpcklbw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 60 f4[ 	]*vpunpcklbw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 60 f4[ 	]*vpunpcklbw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 60 f4[ 	]*vpunpcklbw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 60 31[ 	]*vpunpcklbw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 60 b4 f0 23 01 00 00[ 	]*vpunpcklbw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 60 72 7f[ 	]*vpunpcklbw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 60 b2 00 10 00 00[ 	]*vpunpcklbw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 60 72 80[ 	]*vpunpcklbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 60 b2 e0 ef ff ff[ 	]*vpunpcklbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 61 f4[ 	]*vpunpcklwd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 61 f4[ 	]*vpunpcklwd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 61 f4[ 	]*vpunpcklwd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 61 31[ 	]*vpunpcklwd xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 61 b4 f0 23 01 00 00[ 	]*vpunpcklwd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 61 72 7f[ 	]*vpunpcklwd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 61 b2 00 08 00 00[ 	]*vpunpcklwd xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 61 72 80[ 	]*vpunpcklwd xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 61 b2 f0 f7 ff ff[ 	]*vpunpcklwd xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 61 f4[ 	]*vpunpcklwd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 61 f4[ 	]*vpunpcklwd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 61 f4[ 	]*vpunpcklwd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 61 31[ 	]*vpunpcklwd ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 61 b4 f0 23 01 00 00[ 	]*vpunpcklwd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 61 72 7f[ 	]*vpunpcklwd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 61 b2 00 10 00 00[ 	]*vpunpcklwd ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 61 72 80[ 	]*vpunpcklwd ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 61 b2 e0 ef ff ff[ 	]*vpunpcklwd ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 08 30 ee[ 	]*vpmovwb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 0f 30 ee[ 	]*vpmovwb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 8f 30 ee[ 	]*vpmovwb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 28 30 ee[ 	]*vpmovwb xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 2f 30 ee[ 	]*vpmovwb xmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e af 30 ee[ 	]*vpmovwb xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 08 20 ee[ 	]*vpmovswb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 0f 20 ee[ 	]*vpmovswb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 8f 20 ee[ 	]*vpmovswb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 28 20 ee[ 	]*vpmovswb xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 2f 20 ee[ 	]*vpmovswb xmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e af 20 ee[ 	]*vpmovswb xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 08 10 ee[ 	]*vpmovuswb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 0f 10 ee[ 	]*vpmovuswb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 8f 10 ee[ 	]*vpmovuswb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 28 10 ee[ 	]*vpmovuswb xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 2f 10 ee[ 	]*vpmovuswb xmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e af 10 ee[ 	]*vpmovuswb xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 42 f4 ab[ 	]*vdbpsadbw xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 42 f4 ab[ 	]*vdbpsadbw xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 42 f4 ab[ 	]*vdbpsadbw xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 42 f4 7b[ 	]*vdbpsadbw xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 42 31 7b[ 	]*vdbpsadbw xmm30,xmm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 42 b4 f0 23 01 00 00 7b[ 	]*vdbpsadbw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 42 72 7f 7b[ 	]*vdbpsadbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 42 b2 00 08 00 00 7b[ 	]*vdbpsadbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 42 72 80 7b[ 	]*vdbpsadbw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 42 b2 f0 f7 ff ff 7b[ 	]*vdbpsadbw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 42 f4 ab[ 	]*vdbpsadbw ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 42 f4 ab[ 	]*vdbpsadbw ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 42 f4 ab[ 	]*vdbpsadbw ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 42 f4 7b[ 	]*vdbpsadbw ymm30,ymm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 42 31 7b[ 	]*vdbpsadbw ymm30,ymm29,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 42 b4 f0 23 01 00 00 7b[ 	]*vdbpsadbw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 42 72 7f 7b[ 	]*vdbpsadbw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 42 b2 00 10 00 00 7b[ 	]*vdbpsadbw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 42 72 80 7b[ 	]*vdbpsadbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 42 b2 e0 ef ff ff 7b[ 	]*vdbpsadbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 8d f4[ 	]*vpermw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 8d f4[ 	]*vpermw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 8d f4[ 	]*vpermw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 8d 31[ 	]*vpermw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 8d b4 f0 23 01 00 00[ 	]*vpermw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 8d 72 7f[ 	]*vpermw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 8d b2 00 08 00 00[ 	]*vpermw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 8d 72 80[ 	]*vpermw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 8d b2 f0 f7 ff ff[ 	]*vpermw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 8d f4[ 	]*vpermw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 8d f4[ 	]*vpermw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 8d f4[ 	]*vpermw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 8d 31[ 	]*vpermw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 8d b4 f0 23 01 00 00[ 	]*vpermw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 8d 72 7f[ 	]*vpermw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 8d b2 00 10 00 00[ 	]*vpermw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 8d 72 80[ 	]*vpermw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 8d b2 e0 ef ff ff[ 	]*vpermw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 7d f4[ 	]*vpermt2w xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 7d f4[ 	]*vpermt2w xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 7d f4[ 	]*vpermt2w xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 7d 31[ 	]*vpermt2w xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 7d b4 f0 23 01 00 00[ 	]*vpermt2w xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 7d 72 7f[ 	]*vpermt2w xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 7d b2 00 08 00 00[ 	]*vpermt2w xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 7d 72 80[ 	]*vpermt2w xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 7d b2 f0 f7 ff ff[ 	]*vpermt2w xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 7d f4[ 	]*vpermt2w ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 7d f4[ 	]*vpermt2w ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 7d f4[ 	]*vpermt2w ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 7d 31[ 	]*vpermt2w ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 7d b4 f0 23 01 00 00[ 	]*vpermt2w ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 7d 72 7f[ 	]*vpermt2w ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 7d b2 00 10 00 00[ 	]*vpermt2w ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 7d 72 80[ 	]*vpermt2w ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 7d b2 e0 ef ff ff[ 	]*vpermt2w ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 73 fd ab[ 	]*vpslldq xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 73 fd 7b[ 	]*vpslldq xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 39 7b[ 	]*vpslldq xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 73 bc f0 23 01 00 00 7b[ 	]*vpslldq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 7a 7f 7b[ 	]*vpslldq xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 ba 00 08 00 00 7b[ 	]*vpslldq xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 7a 80 7b[ 	]*vpslldq xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 ba f0 f7 ff ff 7b[ 	]*vpslldq xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 73 fd ab[ 	]*vpslldq ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 73 fd 7b[ 	]*vpslldq ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 39 7b[ 	]*vpslldq ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 73 bc f0 23 01 00 00 7b[ 	]*vpslldq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 7a 7f 7b[ 	]*vpslldq ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 ba 00 10 00 00 7b[ 	]*vpslldq ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 7a 80 7b[ 	]*vpslldq ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 ba e0 ef ff ff 7b[ 	]*vpslldq ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 71 f5 ab[ 	]*vpsllw xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 07 71 f5 ab[ 	]*vpsllw xmm30\{k7\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 87 71 f5 ab[ 	]*vpsllw xmm30\{k7\}\{z\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 71 f5 7b[ 	]*vpsllw xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 31 7b[ 	]*vpsllw xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 71 b4 f0 23 01 00 00 7b[ 	]*vpsllw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 72 7f 7b[ 	]*vpsllw xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 b2 00 08 00 00 7b[ 	]*vpsllw xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 72 80 7b[ 	]*vpsllw xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 b2 f0 f7 ff ff 7b[ 	]*vpsllw xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 71 f5 ab[ 	]*vpsllw ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 27 71 f5 ab[ 	]*vpsllw ymm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d a7 71 f5 ab[ 	]*vpsllw ymm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 71 f5 7b[ 	]*vpsllw ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 31 7b[ 	]*vpsllw ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 71 b4 f0 23 01 00 00 7b[ 	]*vpsllw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 72 7f 7b[ 	]*vpsllw ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 b2 00 10 00 00 7b[ 	]*vpsllw ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 72 80 7b[ 	]*vpsllw ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 b2 e0 ef ff ff 7b[ 	]*vpsllw ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 12 f4[ 	]*vpsllvw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 12 f4[ 	]*vpsllvw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 12 f4[ 	]*vpsllvw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 12 31[ 	]*vpsllvw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 12 b4 f0 23 01 00 00[ 	]*vpsllvw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 12 72 7f[ 	]*vpsllvw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 12 b2 00 08 00 00[ 	]*vpsllvw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 12 72 80[ 	]*vpsllvw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 12 b2 f0 f7 ff ff[ 	]*vpsllvw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 12 f4[ 	]*vpsllvw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 12 f4[ 	]*vpsllvw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 12 f4[ 	]*vpsllvw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 12 31[ 	]*vpsllvw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 12 b4 f0 23 01 00 00[ 	]*vpsllvw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 12 72 7f[ 	]*vpsllvw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 12 b2 00 10 00 00[ 	]*vpsllvw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 12 72 80[ 	]*vpsllvw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 12 b2 e0 ef ff ff[ 	]*vpsllvw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 08 6f f5[ 	]*vmovdqu8 xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 0f 6f f5[ 	]*vmovdqu8 xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 8f 6f f5[ 	]*vmovdqu8 xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 6f 31[ 	]*vmovdqu8 xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 08 6f b4 f0 23 01 00 00[ 	]*vmovdqu8 xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 6f 72 7f[ 	]*vmovdqu8 xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 6f b2 00 08 00 00[ 	]*vmovdqu8 xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 6f 72 80[ 	]*vmovdqu8 xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 6f b2 f0 f7 ff ff[ 	]*vmovdqu8 xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 28 6f f5[ 	]*vmovdqu8 ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 2f 6f f5[ 	]*vmovdqu8 ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7f af 6f f5[ 	]*vmovdqu8 ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 6f 31[ 	]*vmovdqu8 ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 28 6f b4 f0 23 01 00 00[ 	]*vmovdqu8 ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 6f 72 7f[ 	]*vmovdqu8 ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 6f b2 00 10 00 00[ 	]*vmovdqu8 ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 6f 72 80[ 	]*vmovdqu8 ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 6f b2 e0 ef ff ff[ 	]*vmovdqu8 ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 6f f5[ 	]*vmovdqu16 xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 6f f5[ 	]*vmovdqu16 xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 6f f5[ 	]*vmovdqu16 xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 6f 31[ 	]*vmovdqu16 xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 08 6f b4 f0 23 01 00 00[ 	]*vmovdqu16 xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 6f 72 7f[ 	]*vmovdqu16 xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 6f b2 00 08 00 00[ 	]*vmovdqu16 xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 6f 72 80[ 	]*vmovdqu16 xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 6f b2 f0 f7 ff ff[ 	]*vmovdqu16 xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 6f f5[ 	]*vmovdqu16 ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 6f f5[ 	]*vmovdqu16 ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 6f f5[ 	]*vmovdqu16 ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 6f 31[ 	]*vmovdqu16 ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 28 6f b4 f0 23 01 00 00[ 	]*vmovdqu16 ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 6f 72 7f[ 	]*vmovdqu16 ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 6f b2 00 10 00 00[ 	]*vmovdqu16 ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 6f 72 80[ 	]*vmovdqu16 ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 6f b2 e0 ef ff ff[ 	]*vmovdqu16 ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 30 31[ 	]*vpmovwb QWORD PTR \[rcx\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 0f 30 31[ 	]*vpmovwb QWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 08 30 b4 f0 23 01 00 00[ 	]*vpmovwb QWORD PTR \[rax\+r14\*8\+0x123\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 30 72 7f[ 	]*vpmovwb QWORD PTR \[rdx\+0x3f8\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 30 b2 00 04 00 00[ 	]*vpmovwb QWORD PTR \[rdx\+0x400\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 30 72 80[ 	]*vpmovwb QWORD PTR \[rdx-0x400\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 30 b2 f8 fb ff ff[ 	]*vpmovwb QWORD PTR \[rdx-0x408\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 30 31[ 	]*vpmovwb XMMWORD PTR \[rcx\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 2f 30 31[ 	]*vpmovwb XMMWORD PTR \[rcx\]\{k7\},ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 28 30 b4 f0 23 01 00 00[ 	]*vpmovwb XMMWORD PTR \[rax\+r14\*8\+0x123\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 30 72 7f[ 	]*vpmovwb XMMWORD PTR \[rdx\+0x7f0\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 30 b2 00 08 00 00[ 	]*vpmovwb XMMWORD PTR \[rdx\+0x800\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 30 72 80[ 	]*vpmovwb XMMWORD PTR \[rdx-0x800\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 30 b2 f0 f7 ff ff[ 	]*vpmovwb XMMWORD PTR \[rdx-0x810\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 20 31[ 	]*vpmovswb QWORD PTR \[rcx\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 0f 20 31[ 	]*vpmovswb QWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 08 20 b4 f0 23 01 00 00[ 	]*vpmovswb QWORD PTR \[rax\+r14\*8\+0x123\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 20 72 7f[ 	]*vpmovswb QWORD PTR \[rdx\+0x3f8\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 20 b2 00 04 00 00[ 	]*vpmovswb QWORD PTR \[rdx\+0x400\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 20 72 80[ 	]*vpmovswb QWORD PTR \[rdx-0x400\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 20 b2 f8 fb ff ff[ 	]*vpmovswb QWORD PTR \[rdx-0x408\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 20 31[ 	]*vpmovswb XMMWORD PTR \[rcx\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 2f 20 31[ 	]*vpmovswb XMMWORD PTR \[rcx\]\{k7\},ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 28 20 b4 f0 23 01 00 00[ 	]*vpmovswb XMMWORD PTR \[rax\+r14\*8\+0x123\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 20 72 7f[ 	]*vpmovswb XMMWORD PTR \[rdx\+0x7f0\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 20 b2 00 08 00 00[ 	]*vpmovswb XMMWORD PTR \[rdx\+0x800\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 20 72 80[ 	]*vpmovswb XMMWORD PTR \[rdx-0x800\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 20 b2 f0 f7 ff ff[ 	]*vpmovswb XMMWORD PTR \[rdx-0x810\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 10 31[ 	]*vpmovuswb QWORD PTR \[rcx\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 0f 10 31[ 	]*vpmovuswb QWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 08 10 b4 f0 23 01 00 00[ 	]*vpmovuswb QWORD PTR \[rax\+r14\*8\+0x123\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 10 72 7f[ 	]*vpmovuswb QWORD PTR \[rdx\+0x3f8\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 10 b2 00 04 00 00[ 	]*vpmovuswb QWORD PTR \[rdx\+0x400\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 10 72 80[ 	]*vpmovuswb QWORD PTR \[rdx-0x400\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 10 b2 f8 fb ff ff[ 	]*vpmovuswb QWORD PTR \[rdx-0x408\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 10 31[ 	]*vpmovuswb XMMWORD PTR \[rcx\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 2f 10 31[ 	]*vpmovuswb XMMWORD PTR \[rcx\]\{k7\},ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 28 10 b4 f0 23 01 00 00[ 	]*vpmovuswb XMMWORD PTR \[rax\+r14\*8\+0x123\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 10 72 7f[ 	]*vpmovuswb XMMWORD PTR \[rdx\+0x7f0\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 10 b2 00 08 00 00[ 	]*vpmovuswb XMMWORD PTR \[rdx\+0x800\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 10 72 80[ 	]*vpmovuswb XMMWORD PTR \[rdx-0x800\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 10 b2 f0 f7 ff ff[ 	]*vpmovuswb XMMWORD PTR \[rdx-0x810\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 7f 31[ 	]*vmovdqu8 XMMWORD PTR \[rcx\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 0f 7f 31[ 	]*vmovdqu8 XMMWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 08 7f b4 f0 23 01 00 00[ 	]*vmovdqu8 XMMWORD PTR \[rax\+r14\*8\+0x123\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 7f 72 7f[ 	]*vmovdqu8 XMMWORD PTR \[rdx\+0x7f0\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 7f b2 00 08 00 00[ 	]*vmovdqu8 XMMWORD PTR \[rdx\+0x800\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 7f 72 80[ 	]*vmovdqu8 XMMWORD PTR \[rdx-0x800\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 7f b2 f0 f7 ff ff[ 	]*vmovdqu8 XMMWORD PTR \[rdx-0x810\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 7f 31[ 	]*vmovdqu8 YMMWORD PTR \[rcx\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 2f 7f 31[ 	]*vmovdqu8 YMMWORD PTR \[rcx\]\{k7\},ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 28 7f b4 f0 23 01 00 00[ 	]*vmovdqu8 YMMWORD PTR \[rax\+r14\*8\+0x123\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 7f 72 7f[ 	]*vmovdqu8 YMMWORD PTR \[rdx\+0xfe0\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 7f b2 00 10 00 00[ 	]*vmovdqu8 YMMWORD PTR \[rdx\+0x1000\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 7f 72 80[ 	]*vmovdqu8 YMMWORD PTR \[rdx-0x1000\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 7f b2 e0 ef ff ff[ 	]*vmovdqu8 YMMWORD PTR \[rdx-0x1020\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7f 31[ 	]*vmovdqu16 XMMWORD PTR \[rcx\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 0f 7f 31[ 	]*vmovdqu16 XMMWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 08 7f b4 f0 23 01 00 00[ 	]*vmovdqu16 XMMWORD PTR \[rax\+r14\*8\+0x123\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7f 72 7f[ 	]*vmovdqu16 XMMWORD PTR \[rdx\+0x7f0\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7f b2 00 08 00 00[ 	]*vmovdqu16 XMMWORD PTR \[rdx\+0x800\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7f 72 80[ 	]*vmovdqu16 XMMWORD PTR \[rdx-0x800\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7f b2 f0 f7 ff ff[ 	]*vmovdqu16 XMMWORD PTR \[rdx-0x810\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7f 31[ 	]*vmovdqu16 YMMWORD PTR \[rcx\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 2f 7f 31[ 	]*vmovdqu16 YMMWORD PTR \[rcx\]\{k7\},ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 28 7f b4 f0 23 01 00 00[ 	]*vmovdqu16 YMMWORD PTR \[rax\+r14\*8\+0x123\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7f 72 7f[ 	]*vmovdqu16 YMMWORD PTR \[rdx\+0xfe0\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7f b2 00 10 00 00[ 	]*vmovdqu16 YMMWORD PTR \[rdx\+0x1000\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7f 72 80[ 	]*vmovdqu16 YMMWORD PTR \[rdx-0x1000\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7f b2 e0 ef ff ff[ 	]*vmovdqu16 YMMWORD PTR \[rdx-0x1020\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 75 f4[ 	]*vpermi2w xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 75 f4[ 	]*vpermi2w xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 75 f4[ 	]*vpermi2w xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 75 31[ 	]*vpermi2w xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 75 b4 f0 23 01 00 00[ 	]*vpermi2w xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 75 72 7f[ 	]*vpermi2w xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 75 b2 00 08 00 00[ 	]*vpermi2w xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 75 72 80[ 	]*vpermi2w xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 75 b2 f0 f7 ff ff[ 	]*vpermi2w xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 75 f4[ 	]*vpermi2w ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 75 f4[ 	]*vpermi2w ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 75 f4[ 	]*vpermi2w ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 75 31[ 	]*vpermi2w ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 75 b4 f0 23 01 00 00[ 	]*vpermi2w ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 75 72 7f[ 	]*vpermi2w ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 75 b2 00 10 00 00[ 	]*vpermi2w ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 75 72 80[ 	]*vpermi2w ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 75 b2 e0 ef ff ff[ 	]*vpermi2w ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 92 0d 00 26 ed[ 	]*vptestmb k5,xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 92 0d 07 26 ed[ 	]*vptestmb k5\{k7\},xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 00 26 29[ 	]*vptestmb k5,xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 0d 00 26 ac f0 23 01 00 00[ 	]*vptestmb k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 00 26 6a 7f[ 	]*vptestmb k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 00 26 aa 00 08 00 00[ 	]*vptestmb k5,xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 00 26 6a 80[ 	]*vptestmb k5,xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 00 26 aa f0 f7 ff ff[ 	]*vptestmb k5,xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 92 0d 20 26 ed[ 	]*vptestmb k5,ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 92 0d 27 26 ed[ 	]*vptestmb k5\{k7\},ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 20 26 29[ 	]*vptestmb k5,ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 0d 20 26 ac f0 23 01 00 00[ 	]*vptestmb k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 20 26 6a 7f[ 	]*vptestmb k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 20 26 aa 00 10 00 00[ 	]*vptestmb k5,ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 20 26 6a 80[ 	]*vptestmb k5,ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 20 26 aa e0 ef ff ff[ 	]*vptestmb k5,ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 92 8d 00 26 ed[ 	]*vptestmw k5,xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 92 8d 07 26 ed[ 	]*vptestmw k5\{k7\},xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 00 26 29[ 	]*vptestmw k5,xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 8d 00 26 ac f0 23 01 00 00[ 	]*vptestmw k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 00 26 6a 7f[ 	]*vptestmw k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 00 26 aa 00 08 00 00[ 	]*vptestmw k5,xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 00 26 6a 80[ 	]*vptestmw k5,xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 00 26 aa f0 f7 ff ff[ 	]*vptestmw k5,xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 92 8d 20 26 ed[ 	]*vptestmw k5,ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 92 8d 27 26 ed[ 	]*vptestmw k5\{k7\},ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 20 26 29[ 	]*vptestmw k5,ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 8d 20 26 ac f0 23 01 00 00[ 	]*vptestmw k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 20 26 6a 7f[ 	]*vptestmw k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 20 26 aa 00 10 00 00[ 	]*vptestmw k5,ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 20 26 6a 80[ 	]*vptestmw k5,ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 20 26 aa e0 ef ff ff[ 	]*vptestmw k5,ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 08 29 ee[ 	]*vpmovb2m k5,xmm30
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 28 29 ee[ 	]*vpmovb2m k5,ymm30
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 08 29 ee[ 	]*vpmovw2m k5,xmm30
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 28 29 ee[ 	]*vpmovw2m k5,ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 28 f5[ 	]*vpmovm2b xmm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 28 f5[ 	]*vpmovm2b ymm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 08 28 f5[ 	]*vpmovm2w xmm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 28 28 f5[ 	]*vpmovm2w ymm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 92 16 00 26 ec[ 	]*vptestnmb k5,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 92 16 07 26 ec[ 	]*vptestnmb k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 00 26 29[ 	]*vptestnmb k5,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 16 00 26 ac f0 23 01 00 00[ 	]*vptestnmb k5,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 00 26 6a 7f[ 	]*vptestnmb k5,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 00 26 aa 00 08 00 00[ 	]*vptestnmb k5,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 00 26 6a 80[ 	]*vptestnmb k5,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 00 26 aa f0 f7 ff ff[ 	]*vptestnmb k5,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 92 16 20 26 ec[ 	]*vptestnmb k5,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 92 16 27 26 ec[ 	]*vptestnmb k5\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 20 26 29[ 	]*vptestnmb k5,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 16 20 26 ac f0 23 01 00 00[ 	]*vptestnmb k5,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 20 26 6a 7f[ 	]*vptestnmb k5,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 20 26 aa 00 10 00 00[ 	]*vptestnmb k5,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 20 26 6a 80[ 	]*vptestnmb k5,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 20 26 aa e0 ef ff ff[ 	]*vptestnmb k5,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 92 96 00 26 ec[ 	]*vptestnmw k5,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 92 96 07 26 ec[ 	]*vptestnmw k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 00 26 29[ 	]*vptestnmw k5,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 96 00 26 ac f0 23 01 00 00[ 	]*vptestnmw k5,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 00 26 6a 7f[ 	]*vptestnmw k5,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 00 26 aa 00 08 00 00[ 	]*vptestnmw k5,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 00 26 6a 80[ 	]*vptestnmw k5,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 00 26 aa f0 f7 ff ff[ 	]*vptestnmw k5,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 92 96 20 26 ec[ 	]*vptestnmw k5,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 92 96 27 26 ec[ 	]*vptestnmw k5\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 20 26 29[ 	]*vptestnmw k5,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 96 20 26 ac f0 23 01 00 00[ 	]*vptestnmw k5,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 20 26 6a 7f[ 	]*vptestnmw k5,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 20 26 aa 00 10 00 00[ 	]*vptestnmw k5,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 20 26 6a 80[ 	]*vptestnmw k5,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 20 26 aa e0 ef ff ff[ 	]*vptestnmw k5,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 00 3f ed ab[ 	]*vpcmpb k5,xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 07 3f ed ab[ 	]*vpcmpb k5\{k7\},xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 00 3f ed 7b[ 	]*vpcmpb k5,xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3f 29 7b[ 	]*vpcmpb k5,xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 0d 00 3f ac f0 23 01 00 00 7b[ 	]*vpcmpb k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3f 6a 7f 7b[ 	]*vpcmpb k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3f aa 00 08 00 00 7b[ 	]*vpcmpb k5,xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3f 6a 80 7b[ 	]*vpcmpb k5,xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3f aa f0 f7 ff ff 7b[ 	]*vpcmpb k5,xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 20 3f ed ab[ 	]*vpcmpb k5,ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 27 3f ed ab[ 	]*vpcmpb k5\{k7\},ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 20 3f ed 7b[ 	]*vpcmpb k5,ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3f 29 7b[ 	]*vpcmpb k5,ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 0d 20 3f ac f0 23 01 00 00 7b[ 	]*vpcmpb k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3f 6a 7f 7b[ 	]*vpcmpb k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3f aa 00 10 00 00 7b[ 	]*vpcmpb k5,ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3f 6a 80 7b[ 	]*vpcmpb k5,ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3f aa e0 ef ff ff 7b[ 	]*vpcmpb k5,ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 00 3f ed ab[ 	]*vpcmpw k5,xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 07 3f ed ab[ 	]*vpcmpw k5\{k7\},xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 00 3f ed 7b[ 	]*vpcmpw k5,xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3f 29 7b[ 	]*vpcmpw k5,xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 8d 00 3f ac f0 23 01 00 00 7b[ 	]*vpcmpw k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3f 6a 7f 7b[ 	]*vpcmpw k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3f aa 00 08 00 00 7b[ 	]*vpcmpw k5,xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3f 6a 80 7b[ 	]*vpcmpw k5,xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3f aa f0 f7 ff ff 7b[ 	]*vpcmpw k5,xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 20 3f ed ab[ 	]*vpcmpw k5,ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 27 3f ed ab[ 	]*vpcmpw k5\{k7\},ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 20 3f ed 7b[ 	]*vpcmpw k5,ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3f 29 7b[ 	]*vpcmpw k5,ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 8d 20 3f ac f0 23 01 00 00 7b[ 	]*vpcmpw k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3f 6a 7f 7b[ 	]*vpcmpw k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3f aa 00 10 00 00 7b[ 	]*vpcmpw k5,ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3f 6a 80 7b[ 	]*vpcmpw k5,ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3f aa e0 ef ff ff 7b[ 	]*vpcmpw k5,ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 00 3e ed ab[ 	]*vpcmpub k5,xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 07 3e ed ab[ 	]*vpcmpub k5\{k7\},xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 00 3e ed 7b[ 	]*vpcmpub k5,xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3e 29 7b[ 	]*vpcmpub k5,xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 0d 00 3e ac f0 23 01 00 00 7b[ 	]*vpcmpub k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3e 6a 7f 7b[ 	]*vpcmpub k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3e aa 00 08 00 00 7b[ 	]*vpcmpub k5,xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3e 6a 80 7b[ 	]*vpcmpub k5,xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3e aa f0 f7 ff ff 7b[ 	]*vpcmpub k5,xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 20 3e ed ab[ 	]*vpcmpub k5,ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 27 3e ed ab[ 	]*vpcmpub k5\{k7\},ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 20 3e ed 7b[ 	]*vpcmpub k5,ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3e 29 7b[ 	]*vpcmpub k5,ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 0d 20 3e ac f0 23 01 00 00 7b[ 	]*vpcmpub k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3e 6a 7f 7b[ 	]*vpcmpub k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3e aa 00 10 00 00 7b[ 	]*vpcmpub k5,ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3e 6a 80 7b[ 	]*vpcmpub k5,ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3e aa e0 ef ff ff 7b[ 	]*vpcmpub k5,ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 00 3e ed ab[ 	]*vpcmpuw k5,xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 07 3e ed ab[ 	]*vpcmpuw k5\{k7\},xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 00 3e ed 7b[ 	]*vpcmpuw k5,xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3e 29 7b[ 	]*vpcmpuw k5,xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 8d 00 3e ac f0 23 01 00 00 7b[ 	]*vpcmpuw k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3e 6a 7f 7b[ 	]*vpcmpuw k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3e aa 00 08 00 00 7b[ 	]*vpcmpuw k5,xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3e 6a 80 7b[ 	]*vpcmpuw k5,xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3e aa f0 f7 ff ff 7b[ 	]*vpcmpuw k5,xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 20 3e ed ab[ 	]*vpcmpuw k5,ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 27 3e ed ab[ 	]*vpcmpuw k5\{k7\},ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 20 3e ed 7b[ 	]*vpcmpuw k5,ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3e 29 7b[ 	]*vpcmpuw k5,ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 8d 20 3e ac f0 23 01 00 00 7b[ 	]*vpcmpuw k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3e 6a 7f 7b[ 	]*vpcmpuw k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3e aa 00 10 00 00 7b[ 	]*vpcmpuw k5,ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3e 6a 80 7b[ 	]*vpcmpuw k5,ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3e aa e0 ef ff ff 7b[ 	]*vpcmpuw k5,ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 1c f5[ 	]*vpabsb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 1c f5[ 	]*vpabsb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 1c f5[ 	]*vpabsb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1c 31[ 	]*vpabsb xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 1c b4 f0 34 12 00 00[ 	]*vpabsb xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1c 72 7f[ 	]*vpabsb xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1c b2 00 08 00 00[ 	]*vpabsb xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1c 72 80[ 	]*vpabsb xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1c b2 f0 f7 ff ff[ 	]*vpabsb xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 1c f5[ 	]*vpabsb ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 1c f5[ 	]*vpabsb ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 1c f5[ 	]*vpabsb ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1c 31[ 	]*vpabsb ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 1c b4 f0 34 12 00 00[ 	]*vpabsb ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1c 72 7f[ 	]*vpabsb ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1c b2 00 10 00 00[ 	]*vpabsb ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1c 72 80[ 	]*vpabsb ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1c b2 e0 ef ff ff[ 	]*vpabsb ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 1d f5[ 	]*vpabsw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 1d f5[ 	]*vpabsw xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 1d f5[ 	]*vpabsw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1d 31[ 	]*vpabsw xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 1d b4 f0 34 12 00 00[ 	]*vpabsw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1d 72 7f[ 	]*vpabsw xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1d b2 00 08 00 00[ 	]*vpabsw xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1d 72 80[ 	]*vpabsw xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 1d b2 f0 f7 ff ff[ 	]*vpabsw xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 1d f5[ 	]*vpabsw ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 1d f5[ 	]*vpabsw ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 1d f5[ 	]*vpabsw ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1d 31[ 	]*vpabsw ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 1d b4 f0 34 12 00 00[ 	]*vpabsw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1d 72 7f[ 	]*vpabsw ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1d b2 00 10 00 00[ 	]*vpabsw ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1d 72 80[ 	]*vpabsw ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 1d b2 e0 ef ff ff[ 	]*vpabsw ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 6b f4[ 	]*vpackssdw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 6b f4[ 	]*vpackssdw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 6b f4[ 	]*vpackssdw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 6b 31[ 	]*vpackssdw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 6b b4 f0 34 12 00 00[ 	]*vpackssdw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 10 6b 31[ 	]*vpackssdw xmm30,xmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 6b 72 7f[ 	]*vpackssdw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 6b b2 00 08 00 00[ 	]*vpackssdw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 6b 72 80[ 	]*vpackssdw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 6b b2 f0 f7 ff ff[ 	]*vpackssdw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 10 6b 72 7f[ 	]*vpackssdw xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 10 6b b2 00 02 00 00[ 	]*vpackssdw xmm30,xmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 10 6b 72 80[ 	]*vpackssdw xmm30,xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 10 6b b2 fc fd ff ff[ 	]*vpackssdw xmm30,xmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 6b f4[ 	]*vpackssdw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 6b f4[ 	]*vpackssdw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 6b f4[ 	]*vpackssdw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 6b 31[ 	]*vpackssdw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 6b b4 f0 34 12 00 00[ 	]*vpackssdw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 30 6b 31[ 	]*vpackssdw ymm30,ymm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 6b 72 7f[ 	]*vpackssdw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 6b b2 00 10 00 00[ 	]*vpackssdw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 6b 72 80[ 	]*vpackssdw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 6b b2 e0 ef ff ff[ 	]*vpackssdw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 30 6b 72 7f[ 	]*vpackssdw ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 30 6b b2 00 02 00 00[ 	]*vpackssdw ymm30,ymm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 30 6b 72 80[ 	]*vpackssdw ymm30,ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 30 6b b2 fc fd ff ff[ 	]*vpackssdw ymm30,ymm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 63 f4[ 	]*vpacksswb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 63 f4[ 	]*vpacksswb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 63 f4[ 	]*vpacksswb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 63 31[ 	]*vpacksswb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 63 b4 f0 34 12 00 00[ 	]*vpacksswb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 63 72 7f[ 	]*vpacksswb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 63 b2 00 08 00 00[ 	]*vpacksswb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 63 72 80[ 	]*vpacksswb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 63 b2 f0 f7 ff ff[ 	]*vpacksswb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 63 f4[ 	]*vpacksswb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 63 f4[ 	]*vpacksswb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 63 f4[ 	]*vpacksswb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 63 31[ 	]*vpacksswb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 63 b4 f0 34 12 00 00[ 	]*vpacksswb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 63 72 7f[ 	]*vpacksswb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 63 b2 00 10 00 00[ 	]*vpacksswb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 63 72 80[ 	]*vpacksswb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 63 b2 e0 ef ff ff[ 	]*vpacksswb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 2b f4[ 	]*vpackusdw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 2b f4[ 	]*vpackusdw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 2b f4[ 	]*vpackusdw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 2b 31[ 	]*vpackusdw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 2b b4 f0 34 12 00 00[ 	]*vpackusdw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 2b 31[ 	]*vpackusdw xmm30,xmm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 2b 72 7f[ 	]*vpackusdw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 2b b2 00 08 00 00[ 	]*vpackusdw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 2b 72 80[ 	]*vpackusdw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 2b b2 f0 f7 ff ff[ 	]*vpackusdw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 2b 72 7f[ 	]*vpackusdw xmm30,xmm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 2b b2 00 02 00 00[ 	]*vpackusdw xmm30,xmm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 2b 72 80[ 	]*vpackusdw xmm30,xmm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 10 2b b2 fc fd ff ff[ 	]*vpackusdw xmm30,xmm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 2b f4[ 	]*vpackusdw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 2b f4[ 	]*vpackusdw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 2b f4[ 	]*vpackusdw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 2b 31[ 	]*vpackusdw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 2b b4 f0 34 12 00 00[ 	]*vpackusdw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 2b 31[ 	]*vpackusdw ymm30,ymm29,DWORD BCST \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 2b 72 7f[ 	]*vpackusdw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 2b b2 00 10 00 00[ 	]*vpackusdw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 2b 72 80[ 	]*vpackusdw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 2b b2 e0 ef ff ff[ 	]*vpackusdw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 2b 72 7f[ 	]*vpackusdw ymm30,ymm29,DWORD BCST \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 2b b2 00 02 00 00[ 	]*vpackusdw ymm30,ymm29,DWORD BCST \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 2b 72 80[ 	]*vpackusdw ymm30,ymm29,DWORD BCST \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 30 2b b2 fc fd ff ff[ 	]*vpackusdw ymm30,ymm29,DWORD BCST \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 67 f4[ 	]*vpackuswb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 67 f4[ 	]*vpackuswb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 67 f4[ 	]*vpackuswb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 67 31[ 	]*vpackuswb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 67 b4 f0 34 12 00 00[ 	]*vpackuswb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 67 72 7f[ 	]*vpackuswb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 67 b2 00 08 00 00[ 	]*vpackuswb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 67 72 80[ 	]*vpackuswb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 67 b2 f0 f7 ff ff[ 	]*vpackuswb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 67 f4[ 	]*vpackuswb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 67 f4[ 	]*vpackuswb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 67 f4[ 	]*vpackuswb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 67 31[ 	]*vpackuswb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 67 b4 f0 34 12 00 00[ 	]*vpackuswb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 67 72 7f[ 	]*vpackuswb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 67 b2 00 10 00 00[ 	]*vpackuswb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 67 72 80[ 	]*vpackuswb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 67 b2 e0 ef ff ff[ 	]*vpackuswb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 fc f4[ 	]*vpaddb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 fc f4[ 	]*vpaddb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 fc f4[ 	]*vpaddb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fc 31[ 	]*vpaddb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 fc b4 f0 34 12 00 00[ 	]*vpaddb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fc 72 7f[ 	]*vpaddb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fc b2 00 08 00 00[ 	]*vpaddb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fc 72 80[ 	]*vpaddb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fc b2 f0 f7 ff ff[ 	]*vpaddb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 fc f4[ 	]*vpaddb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 fc f4[ 	]*vpaddb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 fc f4[ 	]*vpaddb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fc 31[ 	]*vpaddb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 fc b4 f0 34 12 00 00[ 	]*vpaddb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fc 72 7f[ 	]*vpaddb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fc b2 00 10 00 00[ 	]*vpaddb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fc 72 80[ 	]*vpaddb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fc b2 e0 ef ff ff[ 	]*vpaddb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 ec f4[ 	]*vpaddsb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 ec f4[ 	]*vpaddsb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 ec f4[ 	]*vpaddsb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ec 31[ 	]*vpaddsb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 ec b4 f0 34 12 00 00[ 	]*vpaddsb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ec 72 7f[ 	]*vpaddsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ec b2 00 08 00 00[ 	]*vpaddsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ec 72 80[ 	]*vpaddsb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ec b2 f0 f7 ff ff[ 	]*vpaddsb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 ec f4[ 	]*vpaddsb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 ec f4[ 	]*vpaddsb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 ec f4[ 	]*vpaddsb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ec 31[ 	]*vpaddsb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 ec b4 f0 34 12 00 00[ 	]*vpaddsb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ec 72 7f[ 	]*vpaddsb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ec b2 00 10 00 00[ 	]*vpaddsb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ec 72 80[ 	]*vpaddsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ec b2 e0 ef ff ff[ 	]*vpaddsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 ed f4[ 	]*vpaddsw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 ed f4[ 	]*vpaddsw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 ed f4[ 	]*vpaddsw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ed 31[ 	]*vpaddsw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 ed b4 f0 34 12 00 00[ 	]*vpaddsw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ed 72 7f[ 	]*vpaddsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ed b2 00 08 00 00[ 	]*vpaddsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ed 72 80[ 	]*vpaddsw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ed b2 f0 f7 ff ff[ 	]*vpaddsw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 ed f4[ 	]*vpaddsw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 ed f4[ 	]*vpaddsw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 ed f4[ 	]*vpaddsw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ed 31[ 	]*vpaddsw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 ed b4 f0 34 12 00 00[ 	]*vpaddsw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ed 72 7f[ 	]*vpaddsw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ed b2 00 10 00 00[ 	]*vpaddsw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ed 72 80[ 	]*vpaddsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ed b2 e0 ef ff ff[ 	]*vpaddsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 dc f4[ 	]*vpaddusb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 dc f4[ 	]*vpaddusb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 dc f4[ 	]*vpaddusb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dc 31[ 	]*vpaddusb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 dc b4 f0 34 12 00 00[ 	]*vpaddusb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dc 72 7f[ 	]*vpaddusb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dc b2 00 08 00 00[ 	]*vpaddusb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dc 72 80[ 	]*vpaddusb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dc b2 f0 f7 ff ff[ 	]*vpaddusb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 dc f4[ 	]*vpaddusb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 dc f4[ 	]*vpaddusb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 dc f4[ 	]*vpaddusb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dc 31[ 	]*vpaddusb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 dc b4 f0 34 12 00 00[ 	]*vpaddusb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dc 72 7f[ 	]*vpaddusb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dc b2 00 10 00 00[ 	]*vpaddusb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dc 72 80[ 	]*vpaddusb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dc b2 e0 ef ff ff[ 	]*vpaddusb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 dd f4[ 	]*vpaddusw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 dd f4[ 	]*vpaddusw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 dd f4[ 	]*vpaddusw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dd 31[ 	]*vpaddusw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 dd b4 f0 34 12 00 00[ 	]*vpaddusw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dd 72 7f[ 	]*vpaddusw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dd b2 00 08 00 00[ 	]*vpaddusw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dd 72 80[ 	]*vpaddusw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 dd b2 f0 f7 ff ff[ 	]*vpaddusw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 dd f4[ 	]*vpaddusw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 dd f4[ 	]*vpaddusw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 dd f4[ 	]*vpaddusw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dd 31[ 	]*vpaddusw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 dd b4 f0 34 12 00 00[ 	]*vpaddusw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dd 72 7f[ 	]*vpaddusw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dd b2 00 10 00 00[ 	]*vpaddusw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dd 72 80[ 	]*vpaddusw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 dd b2 e0 ef ff ff[ 	]*vpaddusw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 fd f4[ 	]*vpaddw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 fd f4[ 	]*vpaddw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 fd f4[ 	]*vpaddw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fd 31[ 	]*vpaddw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 fd b4 f0 34 12 00 00[ 	]*vpaddw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fd 72 7f[ 	]*vpaddw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fd b2 00 08 00 00[ 	]*vpaddw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fd 72 80[ 	]*vpaddw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 fd b2 f0 f7 ff ff[ 	]*vpaddw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 fd f4[ 	]*vpaddw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 fd f4[ 	]*vpaddw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 fd f4[ 	]*vpaddw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fd 31[ 	]*vpaddw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 fd b4 f0 34 12 00 00[ 	]*vpaddw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fd 72 7f[ 	]*vpaddw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fd b2 00 10 00 00[ 	]*vpaddw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fd 72 80[ 	]*vpaddw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 fd b2 e0 ef ff ff[ 	]*vpaddw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 0f f4 ab[ 	]*vpalignr xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 0f f4 ab[ 	]*vpalignr xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 0f f4 ab[ 	]*vpalignr xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 0f f4 7b[ 	]*vpalignr xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 0f 31 7b[ 	]*vpalignr xmm30,xmm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 0f b4 f0 34 12 00 00 7b[ 	]*vpalignr xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 0f 72 7f 7b[ 	]*vpalignr xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 0f b2 00 08 00 00 7b[ 	]*vpalignr xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 0f 72 80 7b[ 	]*vpalignr xmm30,xmm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 0f b2 f0 f7 ff ff 7b[ 	]*vpalignr xmm30,xmm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 0f f4 ab[ 	]*vpalignr ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 0f f4 ab[ 	]*vpalignr ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 0f f4 ab[ 	]*vpalignr ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 0f f4 7b[ 	]*vpalignr ymm30,ymm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 0f 31 7b[ 	]*vpalignr ymm30,ymm29,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 0f b4 f0 34 12 00 00 7b[ 	]*vpalignr ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 0f 72 7f 7b[ 	]*vpalignr ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 0f b2 00 10 00 00 7b[ 	]*vpalignr ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 0f 72 80 7b[ 	]*vpalignr ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 0f b2 e0 ef ff ff 7b[ 	]*vpalignr ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 e0 f4[ 	]*vpavgb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 e0 f4[ 	]*vpavgb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 e0 f4[ 	]*vpavgb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e0 31[ 	]*vpavgb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 e0 b4 f0 34 12 00 00[ 	]*vpavgb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e0 72 7f[ 	]*vpavgb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e0 b2 00 08 00 00[ 	]*vpavgb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e0 72 80[ 	]*vpavgb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e0 b2 f0 f7 ff ff[ 	]*vpavgb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 e0 f4[ 	]*vpavgb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 e0 f4[ 	]*vpavgb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 e0 f4[ 	]*vpavgb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e0 31[ 	]*vpavgb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 e0 b4 f0 34 12 00 00[ 	]*vpavgb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e0 72 7f[ 	]*vpavgb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e0 b2 00 10 00 00[ 	]*vpavgb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e0 72 80[ 	]*vpavgb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e0 b2 e0 ef ff ff[ 	]*vpavgb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 e3 f4[ 	]*vpavgw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 e3 f4[ 	]*vpavgw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 e3 f4[ 	]*vpavgw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e3 31[ 	]*vpavgw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 e3 b4 f0 34 12 00 00[ 	]*vpavgw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e3 72 7f[ 	]*vpavgw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e3 b2 00 08 00 00[ 	]*vpavgw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e3 72 80[ 	]*vpavgw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e3 b2 f0 f7 ff ff[ 	]*vpavgw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 e3 f4[ 	]*vpavgw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 e3 f4[ 	]*vpavgw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 e3 f4[ 	]*vpavgw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e3 31[ 	]*vpavgw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 e3 b4 f0 34 12 00 00[ 	]*vpavgw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e3 72 7f[ 	]*vpavgw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e3 b2 00 10 00 00[ 	]*vpavgw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e3 72 80[ 	]*vpavgw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e3 b2 e0 ef ff ff[ 	]*vpavgw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 66 f4[ 	]*vpblendmb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 66 f4[ 	]*vpblendmb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 66 f4[ 	]*vpblendmb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 66 31[ 	]*vpblendmb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 66 b4 f0 34 12 00 00[ 	]*vpblendmb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 66 72 7f[ 	]*vpblendmb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 66 b2 00 08 00 00[ 	]*vpblendmb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 66 72 80[ 	]*vpblendmb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 66 b2 f0 f7 ff ff[ 	]*vpblendmb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 66 f4[ 	]*vpblendmb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 66 f4[ 	]*vpblendmb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 66 f4[ 	]*vpblendmb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 66 31[ 	]*vpblendmb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 66 b4 f0 34 12 00 00[ 	]*vpblendmb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 66 72 7f[ 	]*vpblendmb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 66 b2 00 10 00 00[ 	]*vpblendmb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 66 72 80[ 	]*vpblendmb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 66 b2 e0 ef ff ff[ 	]*vpblendmb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 78 f5[ 	]*vpbroadcastb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 78 f5[ 	]*vpbroadcastb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 78 f5[ 	]*vpbroadcastb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 78 31[ 	]*vpbroadcastb xmm30,BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 78 b4 f0 34 12 00 00[ 	]*vpbroadcastb xmm30,BYTE PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 78 72 7f[ 	]*vpbroadcastb xmm30,BYTE PTR \[rdx\+0x7f\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 78 b2 80 00 00 00[ 	]*vpbroadcastb xmm30,BYTE PTR \[rdx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 78 72 80[ 	]*vpbroadcastb xmm30,BYTE PTR \[rdx-0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 78 b2 7f ff ff ff[ 	]*vpbroadcastb xmm30,BYTE PTR \[rdx-0x81\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 78 f5[ 	]*vpbroadcastb ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 78 f5[ 	]*vpbroadcastb ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 78 f5[ 	]*vpbroadcastb ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 78 31[ 	]*vpbroadcastb ymm30,BYTE PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 78 b4 f0 34 12 00 00[ 	]*vpbroadcastb ymm30,BYTE PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 78 72 7f[ 	]*vpbroadcastb ymm30,BYTE PTR \[rdx\+0x7f\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 78 b2 80 00 00 00[ 	]*vpbroadcastb ymm30,BYTE PTR \[rdx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 78 72 80[ 	]*vpbroadcastb ymm30,BYTE PTR \[rdx-0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 78 b2 7f ff ff ff[ 	]*vpbroadcastb ymm30,BYTE PTR \[rdx-0x81\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 7a f0[ 	]*vpbroadcastb xmm30,eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 0f 7a f0[ 	]*vpbroadcastb xmm30\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 8f 7a f0[ 	]*vpbroadcastb xmm30\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 7a f0[ 	]*vpbroadcastb ymm30,eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 2f 7a f0[ 	]*vpbroadcastb ymm30\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d af 7a f0[ 	]*vpbroadcastb ymm30\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 79 f5[ 	]*vpbroadcastw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 79 f5[ 	]*vpbroadcastw xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 79 f5[ 	]*vpbroadcastw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 79 31[ 	]*vpbroadcastw xmm30,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 79 b4 f0 34 12 00 00[ 	]*vpbroadcastw xmm30,WORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 79 72 7f[ 	]*vpbroadcastw xmm30,WORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 79 b2 00 01 00 00[ 	]*vpbroadcastw xmm30,WORD PTR \[rdx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 79 72 80[ 	]*vpbroadcastw xmm30,WORD PTR \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 79 b2 fe fe ff ff[ 	]*vpbroadcastw xmm30,WORD PTR \[rdx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 79 f5[ 	]*vpbroadcastw ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 79 f5[ 	]*vpbroadcastw ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 79 f5[ 	]*vpbroadcastw ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 79 31[ 	]*vpbroadcastw ymm30,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 79 b4 f0 34 12 00 00[ 	]*vpbroadcastw ymm30,WORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 79 72 7f[ 	]*vpbroadcastw ymm30,WORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 79 b2 00 01 00 00[ 	]*vpbroadcastw ymm30,WORD PTR \[rdx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 79 72 80[ 	]*vpbroadcastw ymm30,WORD PTR \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 79 b2 fe fe ff ff[ 	]*vpbroadcastw ymm30,WORD PTR \[rdx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 7b f0[ 	]*vpbroadcastw xmm30,eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 0f 7b f0[ 	]*vpbroadcastw xmm30\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 8f 7b f0[ 	]*vpbroadcastw xmm30\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 7b f0[ 	]*vpbroadcastw ymm30,eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 2f 7b f0[ 	]*vpbroadcastw ymm30\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 62 7d af 7b f0[ 	]*vpbroadcastw ymm30\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 74 ed[ 	]*vpcmpeqb k5,xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 07 74 ed[ 	]*vpcmpeqb k5\{k7\},xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 74 29[ 	]*vpcmpeqb k5,xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 74 ac f0 34 12 00 00[ 	]*vpcmpeqb k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 74 6a 7f[ 	]*vpcmpeqb k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 74 aa 00 08 00 00[ 	]*vpcmpeqb k5,xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 74 6a 80[ 	]*vpcmpeqb k5,xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 74 aa f0 f7 ff ff[ 	]*vpcmpeqb k5,xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 74 ed[ 	]*vpcmpeqb k5,ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 27 74 ed[ 	]*vpcmpeqb k5\{k7\},ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 74 29[ 	]*vpcmpeqb k5,ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 74 ac f0 34 12 00 00[ 	]*vpcmpeqb k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 74 6a 7f[ 	]*vpcmpeqb k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 74 aa 00 10 00 00[ 	]*vpcmpeqb k5,ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 74 6a 80[ 	]*vpcmpeqb k5,ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 74 aa e0 ef ff ff[ 	]*vpcmpeqb k5,ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 75 ed[ 	]*vpcmpeqw k5,xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 07 75 ed[ 	]*vpcmpeqw k5\{k7\},xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 75 29[ 	]*vpcmpeqw k5,xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 75 ac f0 34 12 00 00[ 	]*vpcmpeqw k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 75 6a 7f[ 	]*vpcmpeqw k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 75 aa 00 08 00 00[ 	]*vpcmpeqw k5,xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 75 6a 80[ 	]*vpcmpeqw k5,xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 75 aa f0 f7 ff ff[ 	]*vpcmpeqw k5,xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 75 ed[ 	]*vpcmpeqw k5,ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 27 75 ed[ 	]*vpcmpeqw k5\{k7\},ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 75 29[ 	]*vpcmpeqw k5,ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 75 ac f0 34 12 00 00[ 	]*vpcmpeqw k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 75 6a 7f[ 	]*vpcmpeqw k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 75 aa 00 10 00 00[ 	]*vpcmpeqw k5,ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 75 6a 80[ 	]*vpcmpeqw k5,ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 75 aa e0 ef ff ff[ 	]*vpcmpeqw k5,ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 64 ed[ 	]*vpcmpgtb k5,xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 07 64 ed[ 	]*vpcmpgtb k5\{k7\},xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 64 29[ 	]*vpcmpgtb k5,xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 64 ac f0 34 12 00 00[ 	]*vpcmpgtb k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 64 6a 7f[ 	]*vpcmpgtb k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 64 aa 00 08 00 00[ 	]*vpcmpgtb k5,xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 64 6a 80[ 	]*vpcmpgtb k5,xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 64 aa f0 f7 ff ff[ 	]*vpcmpgtb k5,xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 64 ed[ 	]*vpcmpgtb k5,ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 27 64 ed[ 	]*vpcmpgtb k5\{k7\},ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 64 29[ 	]*vpcmpgtb k5,ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 64 ac f0 34 12 00 00[ 	]*vpcmpgtb k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 64 6a 7f[ 	]*vpcmpgtb k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 64 aa 00 10 00 00[ 	]*vpcmpgtb k5,ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 64 6a 80[ 	]*vpcmpgtb k5,ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 64 aa e0 ef ff ff[ 	]*vpcmpgtb k5,ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 65 ed[ 	]*vpcmpgtw k5,xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 07 65 ed[ 	]*vpcmpgtw k5\{k7\},xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 65 29[ 	]*vpcmpgtw k5,xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 65 ac f0 34 12 00 00[ 	]*vpcmpgtw k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 65 6a 7f[ 	]*vpcmpgtw k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 65 aa 00 08 00 00[ 	]*vpcmpgtw k5,xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 65 6a 80[ 	]*vpcmpgtw k5,xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 65 aa f0 f7 ff ff[ 	]*vpcmpgtw k5,xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 65 ed[ 	]*vpcmpgtw k5,ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 27 65 ed[ 	]*vpcmpgtw k5\{k7\},ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 65 29[ 	]*vpcmpgtw k5,ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 65 ac f0 34 12 00 00[ 	]*vpcmpgtw k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 65 6a 7f[ 	]*vpcmpgtw k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 65 aa 00 10 00 00[ 	]*vpcmpgtw k5,ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 65 6a 80[ 	]*vpcmpgtw k5,ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 65 aa e0 ef ff ff[ 	]*vpcmpgtw k5,ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 66 f4[ 	]*vpblendmw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 66 f4[ 	]*vpblendmw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 66 f4[ 	]*vpblendmw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 66 31[ 	]*vpblendmw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 66 b4 f0 34 12 00 00[ 	]*vpblendmw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 66 72 7f[ 	]*vpblendmw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 66 b2 00 08 00 00[ 	]*vpblendmw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 66 72 80[ 	]*vpblendmw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 66 b2 f0 f7 ff ff[ 	]*vpblendmw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 66 f4[ 	]*vpblendmw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 66 f4[ 	]*vpblendmw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 66 f4[ 	]*vpblendmw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 66 31[ 	]*vpblendmw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 66 b4 f0 34 12 00 00[ 	]*vpblendmw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 66 72 7f[ 	]*vpblendmw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 66 b2 00 10 00 00[ 	]*vpblendmw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 66 72 80[ 	]*vpblendmw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 66 b2 e0 ef ff ff[ 	]*vpblendmw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 04 f4[ 	]*vpmaddubsw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 04 f4[ 	]*vpmaddubsw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 04 f4[ 	]*vpmaddubsw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 04 31[ 	]*vpmaddubsw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 04 b4 f0 34 12 00 00[ 	]*vpmaddubsw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 04 72 7f[ 	]*vpmaddubsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 04 b2 00 08 00 00[ 	]*vpmaddubsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 04 72 80[ 	]*vpmaddubsw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 04 b2 f0 f7 ff ff[ 	]*vpmaddubsw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 04 f4[ 	]*vpmaddubsw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 04 f4[ 	]*vpmaddubsw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 04 f4[ 	]*vpmaddubsw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 04 31[ 	]*vpmaddubsw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 04 b4 f0 34 12 00 00[ 	]*vpmaddubsw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 04 72 7f[ 	]*vpmaddubsw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 04 b2 00 10 00 00[ 	]*vpmaddubsw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 04 72 80[ 	]*vpmaddubsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 04 b2 e0 ef ff ff[ 	]*vpmaddubsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 f5 f4[ 	]*vpmaddwd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 f5 f4[ 	]*vpmaddwd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 f5 f4[ 	]*vpmaddwd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f5 31[ 	]*vpmaddwd xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 f5 b4 f0 34 12 00 00[ 	]*vpmaddwd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f5 72 7f[ 	]*vpmaddwd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f5 b2 00 08 00 00[ 	]*vpmaddwd xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f5 72 80[ 	]*vpmaddwd xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f5 b2 f0 f7 ff ff[ 	]*vpmaddwd xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 f5 f4[ 	]*vpmaddwd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 f5 f4[ 	]*vpmaddwd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 f5 f4[ 	]*vpmaddwd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f5 31[ 	]*vpmaddwd ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 f5 b4 f0 34 12 00 00[ 	]*vpmaddwd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f5 72 7f[ 	]*vpmaddwd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f5 b2 00 10 00 00[ 	]*vpmaddwd ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f5 72 80[ 	]*vpmaddwd ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f5 b2 e0 ef ff ff[ 	]*vpmaddwd ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 3c f4[ 	]*vpmaxsb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 3c f4[ 	]*vpmaxsb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 3c f4[ 	]*vpmaxsb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3c 31[ 	]*vpmaxsb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 3c b4 f0 34 12 00 00[ 	]*vpmaxsb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3c 72 7f[ 	]*vpmaxsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3c b2 00 08 00 00[ 	]*vpmaxsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3c 72 80[ 	]*vpmaxsb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3c b2 f0 f7 ff ff[ 	]*vpmaxsb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 3c f4[ 	]*vpmaxsb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 3c f4[ 	]*vpmaxsb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 3c f4[ 	]*vpmaxsb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3c 31[ 	]*vpmaxsb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 3c b4 f0 34 12 00 00[ 	]*vpmaxsb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3c 72 7f[ 	]*vpmaxsb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3c b2 00 10 00 00[ 	]*vpmaxsb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3c 72 80[ 	]*vpmaxsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3c b2 e0 ef ff ff[ 	]*vpmaxsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 ee f4[ 	]*vpmaxsw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 ee f4[ 	]*vpmaxsw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 ee f4[ 	]*vpmaxsw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ee 31[ 	]*vpmaxsw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 ee b4 f0 34 12 00 00[ 	]*vpmaxsw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ee 72 7f[ 	]*vpmaxsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ee b2 00 08 00 00[ 	]*vpmaxsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ee 72 80[ 	]*vpmaxsw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ee b2 f0 f7 ff ff[ 	]*vpmaxsw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 ee f4[ 	]*vpmaxsw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 ee f4[ 	]*vpmaxsw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 ee f4[ 	]*vpmaxsw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ee 31[ 	]*vpmaxsw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 ee b4 f0 34 12 00 00[ 	]*vpmaxsw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ee 72 7f[ 	]*vpmaxsw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ee b2 00 10 00 00[ 	]*vpmaxsw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ee 72 80[ 	]*vpmaxsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ee b2 e0 ef ff ff[ 	]*vpmaxsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 de f4[ 	]*vpmaxub xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 de f4[ 	]*vpmaxub xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 de f4[ 	]*vpmaxub xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 de 31[ 	]*vpmaxub xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 de b4 f0 34 12 00 00[ 	]*vpmaxub xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 de 72 7f[ 	]*vpmaxub xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 de b2 00 08 00 00[ 	]*vpmaxub xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 de 72 80[ 	]*vpmaxub xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 de b2 f0 f7 ff ff[ 	]*vpmaxub xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 de f4[ 	]*vpmaxub ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 de f4[ 	]*vpmaxub ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 de f4[ 	]*vpmaxub ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 de 31[ 	]*vpmaxub ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 de b4 f0 34 12 00 00[ 	]*vpmaxub ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 de 72 7f[ 	]*vpmaxub ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 de b2 00 10 00 00[ 	]*vpmaxub ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 de 72 80[ 	]*vpmaxub ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 de b2 e0 ef ff ff[ 	]*vpmaxub ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 3e f4[ 	]*vpmaxuw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 3e f4[ 	]*vpmaxuw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 3e f4[ 	]*vpmaxuw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3e 31[ 	]*vpmaxuw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 3e b4 f0 34 12 00 00[ 	]*vpmaxuw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3e 72 7f[ 	]*vpmaxuw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3e b2 00 08 00 00[ 	]*vpmaxuw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3e 72 80[ 	]*vpmaxuw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3e b2 f0 f7 ff ff[ 	]*vpmaxuw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 3e f4[ 	]*vpmaxuw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 3e f4[ 	]*vpmaxuw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 3e f4[ 	]*vpmaxuw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3e 31[ 	]*vpmaxuw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 3e b4 f0 34 12 00 00[ 	]*vpmaxuw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3e 72 7f[ 	]*vpmaxuw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3e b2 00 10 00 00[ 	]*vpmaxuw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3e 72 80[ 	]*vpmaxuw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3e b2 e0 ef ff ff[ 	]*vpmaxuw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 38 f4[ 	]*vpminsb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 38 f4[ 	]*vpminsb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 38 f4[ 	]*vpminsb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 38 31[ 	]*vpminsb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 38 b4 f0 34 12 00 00[ 	]*vpminsb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 38 72 7f[ 	]*vpminsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 38 b2 00 08 00 00[ 	]*vpminsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 38 72 80[ 	]*vpminsb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 38 b2 f0 f7 ff ff[ 	]*vpminsb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 38 f4[ 	]*vpminsb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 38 f4[ 	]*vpminsb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 38 f4[ 	]*vpminsb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 38 31[ 	]*vpminsb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 38 b4 f0 34 12 00 00[ 	]*vpminsb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 38 72 7f[ 	]*vpminsb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 38 b2 00 10 00 00[ 	]*vpminsb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 38 72 80[ 	]*vpminsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 38 b2 e0 ef ff ff[ 	]*vpminsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 ea f4[ 	]*vpminsw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 ea f4[ 	]*vpminsw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 ea f4[ 	]*vpminsw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ea 31[ 	]*vpminsw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 ea b4 f0 34 12 00 00[ 	]*vpminsw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ea 72 7f[ 	]*vpminsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ea b2 00 08 00 00[ 	]*vpminsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ea 72 80[ 	]*vpminsw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 ea b2 f0 f7 ff ff[ 	]*vpminsw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 ea f4[ 	]*vpminsw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 ea f4[ 	]*vpminsw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 ea f4[ 	]*vpminsw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ea 31[ 	]*vpminsw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 ea b4 f0 34 12 00 00[ 	]*vpminsw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ea 72 7f[ 	]*vpminsw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ea b2 00 10 00 00[ 	]*vpminsw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ea 72 80[ 	]*vpminsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 ea b2 e0 ef ff ff[ 	]*vpminsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 da f4[ 	]*vpminub xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 da f4[ 	]*vpminub xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 da f4[ 	]*vpminub xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 da 31[ 	]*vpminub xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 da b4 f0 34 12 00 00[ 	]*vpminub xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 da 72 7f[ 	]*vpminub xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 da b2 00 08 00 00[ 	]*vpminub xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 da 72 80[ 	]*vpminub xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 da b2 f0 f7 ff ff[ 	]*vpminub xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 da f4[ 	]*vpminub ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 da f4[ 	]*vpminub ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 da f4[ 	]*vpminub ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 da 31[ 	]*vpminub ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 da b4 f0 34 12 00 00[ 	]*vpminub ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 da 72 7f[ 	]*vpminub ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 da b2 00 10 00 00[ 	]*vpminub ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 da 72 80[ 	]*vpminub ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 da b2 e0 ef ff ff[ 	]*vpminub ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 3a f4[ 	]*vpminuw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 3a f4[ 	]*vpminuw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 3a f4[ 	]*vpminuw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3a 31[ 	]*vpminuw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 3a b4 f0 34 12 00 00[ 	]*vpminuw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3a 72 7f[ 	]*vpminuw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3a b2 00 08 00 00[ 	]*vpminuw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3a 72 80[ 	]*vpminuw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 3a b2 f0 f7 ff ff[ 	]*vpminuw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 3a f4[ 	]*vpminuw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 3a f4[ 	]*vpminuw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 3a f4[ 	]*vpminuw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3a 31[ 	]*vpminuw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 3a b4 f0 34 12 00 00[ 	]*vpminuw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3a 72 7f[ 	]*vpminuw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3a b2 00 10 00 00[ 	]*vpminuw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3a 72 80[ 	]*vpminuw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 3a b2 e0 ef ff ff[ 	]*vpminuw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 20 f5[ 	]*vpmovsxbw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 20 f5[ 	]*vpmovsxbw xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 20 f5[ 	]*vpmovsxbw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 20 31[ 	]*vpmovsxbw xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 20 b4 f0 34 12 00 00[ 	]*vpmovsxbw xmm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 20 72 7f[ 	]*vpmovsxbw xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 20 b2 00 04 00 00[ 	]*vpmovsxbw xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 20 72 80[ 	]*vpmovsxbw xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 20 b2 f8 fb ff ff[ 	]*vpmovsxbw xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 20 f5[ 	]*vpmovsxbw ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 20 f5[ 	]*vpmovsxbw ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 20 f5[ 	]*vpmovsxbw ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 20 31[ 	]*vpmovsxbw ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 20 b4 f0 34 12 00 00[ 	]*vpmovsxbw ymm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 20 72 7f[ 	]*vpmovsxbw ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 20 b2 00 08 00 00[ 	]*vpmovsxbw ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 20 72 80[ 	]*vpmovsxbw ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 20 b2 f0 f7 ff ff[ 	]*vpmovsxbw ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 08 30 f5[ 	]*vpmovzxbw xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 0f 30 f5[ 	]*vpmovzxbw xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 8f 30 f5[ 	]*vpmovzxbw xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 30 31[ 	]*vpmovzxbw xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 08 30 b4 f0 34 12 00 00[ 	]*vpmovzxbw xmm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 30 72 7f[ 	]*vpmovzxbw xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 30 b2 00 04 00 00[ 	]*vpmovzxbw xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 30 72 80[ 	]*vpmovzxbw xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 08 30 b2 f8 fb ff ff[ 	]*vpmovzxbw xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 28 30 f5[ 	]*vpmovzxbw ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d 2f 30 f5[ 	]*vpmovzxbw ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7d af 30 f5[ 	]*vpmovzxbw ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 30 31[ 	]*vpmovzxbw ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 7d 28 30 b4 f0 34 12 00 00[ 	]*vpmovzxbw ymm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 30 72 7f[ 	]*vpmovzxbw ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 30 b2 00 08 00 00[ 	]*vpmovzxbw ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 30 72 80[ 	]*vpmovzxbw ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7d 28 30 b2 f0 f7 ff ff[ 	]*vpmovzxbw ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 0b f4[ 	]*vpmulhrsw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 0b f4[ 	]*vpmulhrsw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 0b f4[ 	]*vpmulhrsw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 0b 31[ 	]*vpmulhrsw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 0b b4 f0 34 12 00 00[ 	]*vpmulhrsw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 0b 72 7f[ 	]*vpmulhrsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 0b b2 00 08 00 00[ 	]*vpmulhrsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 0b 72 80[ 	]*vpmulhrsw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 0b b2 f0 f7 ff ff[ 	]*vpmulhrsw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 0b f4[ 	]*vpmulhrsw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 0b f4[ 	]*vpmulhrsw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 0b f4[ 	]*vpmulhrsw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 0b 31[ 	]*vpmulhrsw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 0b b4 f0 34 12 00 00[ 	]*vpmulhrsw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 0b 72 7f[ 	]*vpmulhrsw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 0b b2 00 10 00 00[ 	]*vpmulhrsw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 0b 72 80[ 	]*vpmulhrsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 0b b2 e0 ef ff ff[ 	]*vpmulhrsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 e4 f4[ 	]*vpmulhuw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 e4 f4[ 	]*vpmulhuw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 e4 f4[ 	]*vpmulhuw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e4 31[ 	]*vpmulhuw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 e4 b4 f0 34 12 00 00[ 	]*vpmulhuw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e4 72 7f[ 	]*vpmulhuw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e4 b2 00 08 00 00[ 	]*vpmulhuw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e4 72 80[ 	]*vpmulhuw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e4 b2 f0 f7 ff ff[ 	]*vpmulhuw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 e4 f4[ 	]*vpmulhuw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 e4 f4[ 	]*vpmulhuw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 e4 f4[ 	]*vpmulhuw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e4 31[ 	]*vpmulhuw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 e4 b4 f0 34 12 00 00[ 	]*vpmulhuw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e4 72 7f[ 	]*vpmulhuw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e4 b2 00 10 00 00[ 	]*vpmulhuw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e4 72 80[ 	]*vpmulhuw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e4 b2 e0 ef ff ff[ 	]*vpmulhuw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 e5 f4[ 	]*vpmulhw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 e5 f4[ 	]*vpmulhw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 e5 f4[ 	]*vpmulhw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e5 31[ 	]*vpmulhw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 e5 b4 f0 34 12 00 00[ 	]*vpmulhw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e5 72 7f[ 	]*vpmulhw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e5 b2 00 08 00 00[ 	]*vpmulhw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e5 72 80[ 	]*vpmulhw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e5 b2 f0 f7 ff ff[ 	]*vpmulhw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 e5 f4[ 	]*vpmulhw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 e5 f4[ 	]*vpmulhw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 e5 f4[ 	]*vpmulhw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e5 31[ 	]*vpmulhw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 e5 b4 f0 34 12 00 00[ 	]*vpmulhw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e5 72 7f[ 	]*vpmulhw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e5 b2 00 10 00 00[ 	]*vpmulhw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e5 72 80[ 	]*vpmulhw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e5 b2 e0 ef ff ff[ 	]*vpmulhw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 d5 f4[ 	]*vpmullw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 d5 f4[ 	]*vpmullw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 d5 f4[ 	]*vpmullw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d5 31[ 	]*vpmullw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 d5 b4 f0 34 12 00 00[ 	]*vpmullw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d5 72 7f[ 	]*vpmullw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d5 b2 00 08 00 00[ 	]*vpmullw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d5 72 80[ 	]*vpmullw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d5 b2 f0 f7 ff ff[ 	]*vpmullw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 d5 f4[ 	]*vpmullw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 d5 f4[ 	]*vpmullw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 d5 f4[ 	]*vpmullw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d5 31[ 	]*vpmullw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 d5 b4 f0 34 12 00 00[ 	]*vpmullw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d5 72 7f[ 	]*vpmullw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d5 b2 00 10 00 00[ 	]*vpmullw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d5 72 80[ 	]*vpmullw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d5 b2 e0 ef ff ff[ 	]*vpmullw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 f6 f4[ 	]*vpsadbw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f6 31[ 	]*vpsadbw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 f6 b4 f0 34 12 00 00[ 	]*vpsadbw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f6 72 7f[ 	]*vpsadbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f6 b2 00 08 00 00[ 	]*vpsadbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f6 72 80[ 	]*vpsadbw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f6 b2 f0 f7 ff ff[ 	]*vpsadbw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 f6 f4[ 	]*vpsadbw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f6 31[ 	]*vpsadbw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 f6 b4 f0 34 12 00 00[ 	]*vpsadbw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f6 72 7f[ 	]*vpsadbw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f6 b2 00 10 00 00[ 	]*vpsadbw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f6 72 80[ 	]*vpsadbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f6 b2 e0 ef ff ff[ 	]*vpsadbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 00 00 f4[ 	]*vpshufb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 07 00 f4[ 	]*vpshufb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 87 00 f4[ 	]*vpshufb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 00 31[ 	]*vpshufb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 00 00 b4 f0 34 12 00 00[ 	]*vpshufb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 00 72 7f[ 	]*vpshufb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 00 b2 00 08 00 00[ 	]*vpshufb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 00 72 80[ 	]*vpshufb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 00 00 b2 f0 f7 ff ff[ 	]*vpshufb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 15 20 00 f4[ 	]*vpshufb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 27 00 f4[ 	]*vpshufb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 a7 00 f4[ 	]*vpshufb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 00 31[ 	]*vpshufb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 15 20 00 b4 f0 34 12 00 00[ 	]*vpshufb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 00 72 7f[ 	]*vpshufb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 00 b2 00 10 00 00[ 	]*vpshufb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 00 72 80[ 	]*vpshufb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 20 00 b2 e0 ef ff ff[ 	]*vpshufb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 08 70 f5 ab[ 	]*vpshufhw xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 0f 70 f5 ab[ 	]*vpshufhw xmm30\{k7\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 8f 70 f5 ab[ 	]*vpshufhw xmm30\{k7\}\{z\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 08 70 f5 7b[ 	]*vpshufhw xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 08 70 31 7b[ 	]*vpshufhw xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 7e 08 70 b4 f0 34 12 00 00 7b[ 	]*vpshufhw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 08 70 72 7f 7b[ 	]*vpshufhw xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 08 70 b2 00 08 00 00 7b[ 	]*vpshufhw xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 08 70 72 80 7b[ 	]*vpshufhw xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 08 70 b2 f0 f7 ff ff 7b[ 	]*vpshufhw xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 28 70 f5 ab[ 	]*vpshufhw ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 2f 70 f5 ab[ 	]*vpshufhw ymm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e af 70 f5 ab[ 	]*vpshufhw ymm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7e 28 70 f5 7b[ 	]*vpshufhw ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 28 70 31 7b[ 	]*vpshufhw ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 7e 28 70 b4 f0 34 12 00 00 7b[ 	]*vpshufhw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 28 70 72 7f 7b[ 	]*vpshufhw ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 28 70 b2 00 10 00 00 7b[ 	]*vpshufhw ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 28 70 72 80 7b[ 	]*vpshufhw ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7e 28 70 b2 e0 ef ff ff 7b[ 	]*vpshufhw ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 08 70 f5 ab[ 	]*vpshuflw xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 0f 70 f5 ab[ 	]*vpshuflw xmm30\{k7\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 8f 70 f5 ab[ 	]*vpshuflw xmm30\{k7\}\{z\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 08 70 f5 7b[ 	]*vpshuflw xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 70 31 7b[ 	]*vpshuflw xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 08 70 b4 f0 34 12 00 00 7b[ 	]*vpshuflw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 70 72 7f 7b[ 	]*vpshuflw xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 70 b2 00 08 00 00 7b[ 	]*vpshuflw xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 70 72 80 7b[ 	]*vpshuflw xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 70 b2 f0 f7 ff ff 7b[ 	]*vpshuflw xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 28 70 f5 ab[ 	]*vpshuflw ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 2f 70 f5 ab[ 	]*vpshuflw ymm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f af 70 f5 ab[ 	]*vpshuflw ymm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 28 70 f5 7b[ 	]*vpshuflw ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 70 31 7b[ 	]*vpshuflw ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 28 70 b4 f0 34 12 00 00 7b[ 	]*vpshuflw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 70 72 7f 7b[ 	]*vpshuflw ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 70 b2 00 10 00 00 7b[ 	]*vpshuflw ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 70 72 80 7b[ 	]*vpshuflw ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 70 b2 e0 ef ff ff 7b[ 	]*vpshuflw ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 f1 f4[ 	]*vpsllw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 f1 f4[ 	]*vpsllw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 f1 f4[ 	]*vpsllw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f1 31[ 	]*vpsllw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 f1 b4 f0 34 12 00 00[ 	]*vpsllw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f1 72 7f[ 	]*vpsllw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f1 b2 00 08 00 00[ 	]*vpsllw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f1 72 80[ 	]*vpsllw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f1 b2 f0 f7 ff ff[ 	]*vpsllw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 f1 f4[ 	]*vpsllw ymm30,ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 f1 f4[ 	]*vpsllw ymm30\{k7\},ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 f1 f4[ 	]*vpsllw ymm30\{k7\}\{z\},ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f1 31[ 	]*vpsllw ymm30,ymm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 f1 b4 f0 34 12 00 00[ 	]*vpsllw ymm30,ymm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f1 72 7f[ 	]*vpsllw ymm30,ymm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f1 b2 00 08 00 00[ 	]*vpsllw ymm30,ymm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f1 72 80[ 	]*vpsllw ymm30,ymm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f1 b2 f0 f7 ff ff[ 	]*vpsllw ymm30,ymm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 e1 f4[ 	]*vpsraw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 e1 f4[ 	]*vpsraw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 e1 f4[ 	]*vpsraw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e1 31[ 	]*vpsraw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 e1 b4 f0 34 12 00 00[ 	]*vpsraw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e1 72 7f[ 	]*vpsraw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e1 b2 00 08 00 00[ 	]*vpsraw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e1 72 80[ 	]*vpsraw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e1 b2 f0 f7 ff ff[ 	]*vpsraw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 e1 f4[ 	]*vpsraw ymm30,ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 e1 f4[ 	]*vpsraw ymm30\{k7\},ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 e1 f4[ 	]*vpsraw ymm30\{k7\}\{z\},ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e1 31[ 	]*vpsraw ymm30,ymm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 e1 b4 f0 34 12 00 00[ 	]*vpsraw ymm30,ymm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e1 72 7f[ 	]*vpsraw ymm30,ymm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e1 b2 00 08 00 00[ 	]*vpsraw ymm30,ymm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e1 72 80[ 	]*vpsraw ymm30,ymm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e1 b2 f0 f7 ff ff[ 	]*vpsraw ymm30,ymm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 d1 f4[ 	]*vpsrlw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 d1 f4[ 	]*vpsrlw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 d1 f4[ 	]*vpsrlw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d1 31[ 	]*vpsrlw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 d1 b4 f0 34 12 00 00[ 	]*vpsrlw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d1 72 7f[ 	]*vpsrlw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d1 b2 00 08 00 00[ 	]*vpsrlw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d1 72 80[ 	]*vpsrlw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d1 b2 f0 f7 ff ff[ 	]*vpsrlw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 d1 f4[ 	]*vpsrlw ymm30,ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 d1 f4[ 	]*vpsrlw ymm30\{k7\},ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 d1 f4[ 	]*vpsrlw ymm30\{k7\}\{z\},ymm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d1 31[ 	]*vpsrlw ymm30,ymm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 d1 b4 f0 34 12 00 00[ 	]*vpsrlw ymm30,ymm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d1 72 7f[ 	]*vpsrlw ymm30,ymm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d1 b2 00 08 00 00[ 	]*vpsrlw ymm30,ymm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d1 72 80[ 	]*vpsrlw ymm30,ymm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d1 b2 f0 f7 ff ff[ 	]*vpsrlw ymm30,ymm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 73 dd ab[ 	]*vpsrldq xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 73 dd 7b[ 	]*vpsrldq xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 19 7b[ 	]*vpsrldq xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 73 9c f0 34 12 00 00 7b[ 	]*vpsrldq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 5a 7f 7b[ 	]*vpsrldq xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 9a 00 08 00 00 7b[ 	]*vpsrldq xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 5a 80 7b[ 	]*vpsrldq xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 9a f0 f7 ff ff 7b[ 	]*vpsrldq xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 73 dd ab[ 	]*vpsrldq ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 73 dd 7b[ 	]*vpsrldq ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 19 7b[ 	]*vpsrldq ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 73 9c f0 34 12 00 00 7b[ 	]*vpsrldq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 5a 7f 7b[ 	]*vpsrldq ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 9a 00 10 00 00 7b[ 	]*vpsrldq ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 5a 80 7b[ 	]*vpsrldq ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 9a e0 ef ff ff 7b[ 	]*vpsrldq ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 71 d5 ab[ 	]*vpsrlw xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 07 71 d5 ab[ 	]*vpsrlw xmm30\{k7\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 87 71 d5 ab[ 	]*vpsrlw xmm30\{k7\}\{z\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 71 d5 7b[ 	]*vpsrlw xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 11 7b[ 	]*vpsrlw xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 71 94 f0 34 12 00 00 7b[ 	]*vpsrlw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 52 7f 7b[ 	]*vpsrlw xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 92 00 08 00 00 7b[ 	]*vpsrlw xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 52 80 7b[ 	]*vpsrlw xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 92 f0 f7 ff ff 7b[ 	]*vpsrlw xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 71 d5 ab[ 	]*vpsrlw ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 27 71 d5 ab[ 	]*vpsrlw ymm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d a7 71 d5 ab[ 	]*vpsrlw ymm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 71 d5 7b[ 	]*vpsrlw ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 11 7b[ 	]*vpsrlw ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 71 94 f0 34 12 00 00 7b[ 	]*vpsrlw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 52 7f 7b[ 	]*vpsrlw ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 92 00 10 00 00 7b[ 	]*vpsrlw ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 52 80 7b[ 	]*vpsrlw ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 92 e0 ef ff ff 7b[ 	]*vpsrlw ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 71 e5 ab[ 	]*vpsraw xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 07 71 e5 ab[ 	]*vpsraw xmm30\{k7\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 87 71 e5 ab[ 	]*vpsraw xmm30\{k7\}\{z\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 71 e5 7b[ 	]*vpsraw xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 21 7b[ 	]*vpsraw xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 71 a4 f0 34 12 00 00 7b[ 	]*vpsraw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 62 7f 7b[ 	]*vpsraw xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 a2 00 08 00 00 7b[ 	]*vpsraw xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 62 80 7b[ 	]*vpsraw xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 a2 f0 f7 ff ff 7b[ 	]*vpsraw xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 71 e5 ab[ 	]*vpsraw ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 27 71 e5 ab[ 	]*vpsraw ymm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d a7 71 e5 ab[ 	]*vpsraw ymm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 71 e5 7b[ 	]*vpsraw ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 21 7b[ 	]*vpsraw ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 71 a4 f0 34 12 00 00 7b[ 	]*vpsraw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 62 7f 7b[ 	]*vpsraw ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 a2 00 10 00 00 7b[ 	]*vpsraw ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 62 80 7b[ 	]*vpsraw ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 a2 e0 ef ff ff 7b[ 	]*vpsraw ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 10 f4[ 	]*vpsrlvw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 10 f4[ 	]*vpsrlvw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 10 f4[ 	]*vpsrlvw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 10 31[ 	]*vpsrlvw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 10 b4 f0 34 12 00 00[ 	]*vpsrlvw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 10 72 7f[ 	]*vpsrlvw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 10 b2 00 08 00 00[ 	]*vpsrlvw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 10 72 80[ 	]*vpsrlvw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 10 b2 f0 f7 ff ff[ 	]*vpsrlvw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 10 f4[ 	]*vpsrlvw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 10 f4[ 	]*vpsrlvw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 10 f4[ 	]*vpsrlvw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 10 31[ 	]*vpsrlvw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 10 b4 f0 34 12 00 00[ 	]*vpsrlvw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 10 72 7f[ 	]*vpsrlvw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 10 b2 00 10 00 00[ 	]*vpsrlvw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 10 72 80[ 	]*vpsrlvw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 10 b2 e0 ef ff ff[ 	]*vpsrlvw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 11 f4[ 	]*vpsravw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 11 f4[ 	]*vpsravw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 11 f4[ 	]*vpsravw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 11 31[ 	]*vpsravw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 11 b4 f0 34 12 00 00[ 	]*vpsravw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 11 72 7f[ 	]*vpsravw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 11 b2 00 08 00 00[ 	]*vpsravw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 11 72 80[ 	]*vpsravw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 11 b2 f0 f7 ff ff[ 	]*vpsravw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 11 f4[ 	]*vpsravw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 11 f4[ 	]*vpsravw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 11 f4[ 	]*vpsravw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 11 31[ 	]*vpsravw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 11 b4 f0 34 12 00 00[ 	]*vpsravw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 11 72 7f[ 	]*vpsravw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 11 b2 00 10 00 00[ 	]*vpsravw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 11 72 80[ 	]*vpsravw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 11 b2 e0 ef ff ff[ 	]*vpsravw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 f8 f4[ 	]*vpsubb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 f8 f4[ 	]*vpsubb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 f8 f4[ 	]*vpsubb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f8 31[ 	]*vpsubb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 f8 b4 f0 34 12 00 00[ 	]*vpsubb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f8 72 7f[ 	]*vpsubb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f8 b2 00 08 00 00[ 	]*vpsubb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f8 72 80[ 	]*vpsubb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f8 b2 f0 f7 ff ff[ 	]*vpsubb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 f8 f4[ 	]*vpsubb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 f8 f4[ 	]*vpsubb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 f8 f4[ 	]*vpsubb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f8 31[ 	]*vpsubb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 f8 b4 f0 34 12 00 00[ 	]*vpsubb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f8 72 7f[ 	]*vpsubb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f8 b2 00 10 00 00[ 	]*vpsubb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f8 72 80[ 	]*vpsubb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f8 b2 e0 ef ff ff[ 	]*vpsubb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 e8 f4[ 	]*vpsubsb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 e8 f4[ 	]*vpsubsb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 e8 f4[ 	]*vpsubsb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e8 31[ 	]*vpsubsb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 e8 b4 f0 34 12 00 00[ 	]*vpsubsb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e8 72 7f[ 	]*vpsubsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e8 b2 00 08 00 00[ 	]*vpsubsb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e8 72 80[ 	]*vpsubsb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e8 b2 f0 f7 ff ff[ 	]*vpsubsb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 e8 f4[ 	]*vpsubsb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 e8 f4[ 	]*vpsubsb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 e8 f4[ 	]*vpsubsb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e8 31[ 	]*vpsubsb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 e8 b4 f0 34 12 00 00[ 	]*vpsubsb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e8 72 7f[ 	]*vpsubsb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e8 b2 00 10 00 00[ 	]*vpsubsb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e8 72 80[ 	]*vpsubsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e8 b2 e0 ef ff ff[ 	]*vpsubsb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 e9 f4[ 	]*vpsubsw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 e9 f4[ 	]*vpsubsw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 e9 f4[ 	]*vpsubsw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e9 31[ 	]*vpsubsw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 e9 b4 f0 34 12 00 00[ 	]*vpsubsw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e9 72 7f[ 	]*vpsubsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e9 b2 00 08 00 00[ 	]*vpsubsw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e9 72 80[ 	]*vpsubsw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 e9 b2 f0 f7 ff ff[ 	]*vpsubsw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 e9 f4[ 	]*vpsubsw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 e9 f4[ 	]*vpsubsw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 e9 f4[ 	]*vpsubsw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e9 31[ 	]*vpsubsw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 e9 b4 f0 34 12 00 00[ 	]*vpsubsw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e9 72 7f[ 	]*vpsubsw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e9 b2 00 10 00 00[ 	]*vpsubsw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e9 72 80[ 	]*vpsubsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 e9 b2 e0 ef ff ff[ 	]*vpsubsw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 d8 f4[ 	]*vpsubusb xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 d8 f4[ 	]*vpsubusb xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 d8 f4[ 	]*vpsubusb xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d8 31[ 	]*vpsubusb xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 d8 b4 f0 34 12 00 00[ 	]*vpsubusb xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d8 72 7f[ 	]*vpsubusb xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d8 b2 00 08 00 00[ 	]*vpsubusb xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d8 72 80[ 	]*vpsubusb xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d8 b2 f0 f7 ff ff[ 	]*vpsubusb xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 d8 f4[ 	]*vpsubusb ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 d8 f4[ 	]*vpsubusb ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 d8 f4[ 	]*vpsubusb ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d8 31[ 	]*vpsubusb ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 d8 b4 f0 34 12 00 00[ 	]*vpsubusb ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d8 72 7f[ 	]*vpsubusb ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d8 b2 00 10 00 00[ 	]*vpsubusb ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d8 72 80[ 	]*vpsubusb ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d8 b2 e0 ef ff ff[ 	]*vpsubusb ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 d9 f4[ 	]*vpsubusw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 d9 f4[ 	]*vpsubusw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 d9 f4[ 	]*vpsubusw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d9 31[ 	]*vpsubusw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 d9 b4 f0 34 12 00 00[ 	]*vpsubusw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d9 72 7f[ 	]*vpsubusw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d9 b2 00 08 00 00[ 	]*vpsubusw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d9 72 80[ 	]*vpsubusw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 d9 b2 f0 f7 ff ff[ 	]*vpsubusw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 d9 f4[ 	]*vpsubusw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 d9 f4[ 	]*vpsubusw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 d9 f4[ 	]*vpsubusw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d9 31[ 	]*vpsubusw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 d9 b4 f0 34 12 00 00[ 	]*vpsubusw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d9 72 7f[ 	]*vpsubusw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d9 b2 00 10 00 00[ 	]*vpsubusw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d9 72 80[ 	]*vpsubusw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 d9 b2 e0 ef ff ff[ 	]*vpsubusw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 f9 f4[ 	]*vpsubw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 f9 f4[ 	]*vpsubw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 f9 f4[ 	]*vpsubw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f9 31[ 	]*vpsubw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 f9 b4 f0 34 12 00 00[ 	]*vpsubw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f9 72 7f[ 	]*vpsubw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f9 b2 00 08 00 00[ 	]*vpsubw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f9 72 80[ 	]*vpsubw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 f9 b2 f0 f7 ff ff[ 	]*vpsubw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 f9 f4[ 	]*vpsubw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 f9 f4[ 	]*vpsubw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 f9 f4[ 	]*vpsubw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f9 31[ 	]*vpsubw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 f9 b4 f0 34 12 00 00[ 	]*vpsubw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f9 72 7f[ 	]*vpsubw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f9 b2 00 10 00 00[ 	]*vpsubw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f9 72 80[ 	]*vpsubw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 f9 b2 e0 ef ff ff[ 	]*vpsubw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 68 f4[ 	]*vpunpckhbw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 68 f4[ 	]*vpunpckhbw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 68 f4[ 	]*vpunpckhbw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 68 31[ 	]*vpunpckhbw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 68 b4 f0 34 12 00 00[ 	]*vpunpckhbw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 68 72 7f[ 	]*vpunpckhbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 68 b2 00 08 00 00[ 	]*vpunpckhbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 68 72 80[ 	]*vpunpckhbw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 68 b2 f0 f7 ff ff[ 	]*vpunpckhbw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 68 f4[ 	]*vpunpckhbw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 68 f4[ 	]*vpunpckhbw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 68 f4[ 	]*vpunpckhbw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 68 31[ 	]*vpunpckhbw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 68 b4 f0 34 12 00 00[ 	]*vpunpckhbw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 68 72 7f[ 	]*vpunpckhbw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 68 b2 00 10 00 00[ 	]*vpunpckhbw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 68 72 80[ 	]*vpunpckhbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 68 b2 e0 ef ff ff[ 	]*vpunpckhbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 69 f4[ 	]*vpunpckhwd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 69 f4[ 	]*vpunpckhwd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 69 f4[ 	]*vpunpckhwd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 69 31[ 	]*vpunpckhwd xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 69 b4 f0 34 12 00 00[ 	]*vpunpckhwd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 69 72 7f[ 	]*vpunpckhwd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 69 b2 00 08 00 00[ 	]*vpunpckhwd xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 69 72 80[ 	]*vpunpckhwd xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 69 b2 f0 f7 ff ff[ 	]*vpunpckhwd xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 69 f4[ 	]*vpunpckhwd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 69 f4[ 	]*vpunpckhwd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 69 f4[ 	]*vpunpckhwd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 69 31[ 	]*vpunpckhwd ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 69 b4 f0 34 12 00 00[ 	]*vpunpckhwd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 69 72 7f[ 	]*vpunpckhwd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 69 b2 00 10 00 00[ 	]*vpunpckhwd ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 69 72 80[ 	]*vpunpckhwd ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 69 b2 e0 ef ff ff[ 	]*vpunpckhwd ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 60 f4[ 	]*vpunpcklbw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 60 f4[ 	]*vpunpcklbw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 60 f4[ 	]*vpunpcklbw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 60 31[ 	]*vpunpcklbw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 60 b4 f0 34 12 00 00[ 	]*vpunpcklbw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 60 72 7f[ 	]*vpunpcklbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 60 b2 00 08 00 00[ 	]*vpunpcklbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 60 72 80[ 	]*vpunpcklbw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 60 b2 f0 f7 ff ff[ 	]*vpunpcklbw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 60 f4[ 	]*vpunpcklbw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 60 f4[ 	]*vpunpcklbw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 60 f4[ 	]*vpunpcklbw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 60 31[ 	]*vpunpcklbw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 60 b4 f0 34 12 00 00[ 	]*vpunpcklbw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 60 72 7f[ 	]*vpunpcklbw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 60 b2 00 10 00 00[ 	]*vpunpcklbw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 60 72 80[ 	]*vpunpcklbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 60 b2 e0 ef ff ff[ 	]*vpunpcklbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 00 61 f4[ 	]*vpunpcklwd xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 07 61 f4[ 	]*vpunpcklwd xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 87 61 f4[ 	]*vpunpcklwd xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 61 31[ 	]*vpunpcklwd xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 00 61 b4 f0 34 12 00 00[ 	]*vpunpcklwd xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 61 72 7f[ 	]*vpunpcklwd xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 61 b2 00 08 00 00[ 	]*vpunpcklwd xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 61 72 80[ 	]*vpunpcklwd xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 00 61 b2 f0 f7 ff ff[ 	]*vpunpcklwd xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 15 20 61 f4[ 	]*vpunpcklwd ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 27 61 f4[ 	]*vpunpcklwd ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 01 15 a7 61 f4[ 	]*vpunpcklwd ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 61 31[ 	]*vpunpcklwd ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 15 20 61 b4 f0 34 12 00 00[ 	]*vpunpcklwd ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 61 72 7f[ 	]*vpunpcklwd ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 61 b2 00 10 00 00[ 	]*vpunpcklwd ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 61 72 80[ 	]*vpunpcklwd ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 15 20 61 b2 e0 ef ff ff[ 	]*vpunpcklwd ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 08 30 ee[ 	]*vpmovwb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 0f 30 ee[ 	]*vpmovwb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 8f 30 ee[ 	]*vpmovwb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 28 30 ee[ 	]*vpmovwb xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 2f 30 ee[ 	]*vpmovwb xmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e af 30 ee[ 	]*vpmovwb xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 08 20 ee[ 	]*vpmovswb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 0f 20 ee[ 	]*vpmovswb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 8f 20 ee[ 	]*vpmovswb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 28 20 ee[ 	]*vpmovswb xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 2f 20 ee[ 	]*vpmovswb xmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e af 20 ee[ 	]*vpmovswb xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 08 10 ee[ 	]*vpmovuswb xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 0f 10 ee[ 	]*vpmovuswb xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 8f 10 ee[ 	]*vpmovuswb xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 28 10 ee[ 	]*vpmovuswb xmm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e 2f 10 ee[ 	]*vpmovuswb xmm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 02 7e af 10 ee[ 	]*vpmovuswb xmm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 42 f4 ab[ 	]*vdbpsadbw xmm30,xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 07 42 f4 ab[ 	]*vdbpsadbw xmm30\{k7\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 87 42 f4 ab[ 	]*vdbpsadbw xmm30\{k7\}\{z\},xmm29,xmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 00 42 f4 7b[ 	]*vdbpsadbw xmm30,xmm29,xmm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 42 31 7b[ 	]*vdbpsadbw xmm30,xmm29,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 00 42 b4 f0 34 12 00 00 7b[ 	]*vdbpsadbw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 42 72 7f 7b[ 	]*vdbpsadbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 42 b2 00 08 00 00 7b[ 	]*vdbpsadbw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 42 72 80 7b[ 	]*vdbpsadbw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 00 42 b2 f0 f7 ff ff 7b[ 	]*vdbpsadbw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 42 f4 ab[ 	]*vdbpsadbw ymm30,ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 27 42 f4 ab[ 	]*vdbpsadbw ymm30\{k7\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 a7 42 f4 ab[ 	]*vdbpsadbw ymm30\{k7\}\{z\},ymm29,ymm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 15 20 42 f4 7b[ 	]*vdbpsadbw ymm30,ymm29,ymm28,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 42 31 7b[ 	]*vdbpsadbw ymm30,ymm29,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 23 15 20 42 b4 f0 34 12 00 00 7b[ 	]*vdbpsadbw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 42 72 7f 7b[ 	]*vdbpsadbw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 42 b2 00 10 00 00 7b[ 	]*vdbpsadbw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 42 72 80 7b[ 	]*vdbpsadbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 15 20 42 b2 e0 ef ff ff 7b[ 	]*vdbpsadbw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 8d f4[ 	]*vpermw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 8d f4[ 	]*vpermw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 8d f4[ 	]*vpermw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 8d 31[ 	]*vpermw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 8d b4 f0 34 12 00 00[ 	]*vpermw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 8d 72 7f[ 	]*vpermw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 8d b2 00 08 00 00[ 	]*vpermw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 8d 72 80[ 	]*vpermw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 8d b2 f0 f7 ff ff[ 	]*vpermw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 8d f4[ 	]*vpermw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 8d f4[ 	]*vpermw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 8d f4[ 	]*vpermw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 8d 31[ 	]*vpermw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 8d b4 f0 34 12 00 00[ 	]*vpermw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 8d 72 7f[ 	]*vpermw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 8d b2 00 10 00 00[ 	]*vpermw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 8d 72 80[ 	]*vpermw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 8d b2 e0 ef ff ff[ 	]*vpermw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 7d f4[ 	]*vpermt2w xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 7d f4[ 	]*vpermt2w xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 7d f4[ 	]*vpermt2w xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 7d 31[ 	]*vpermt2w xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 7d b4 f0 34 12 00 00[ 	]*vpermt2w xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 7d 72 7f[ 	]*vpermt2w xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 7d b2 00 08 00 00[ 	]*vpermt2w xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 7d 72 80[ 	]*vpermt2w xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 7d b2 f0 f7 ff ff[ 	]*vpermt2w xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 7d f4[ 	]*vpermt2w ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 7d f4[ 	]*vpermt2w ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 7d f4[ 	]*vpermt2w ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 7d 31[ 	]*vpermt2w ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 7d b4 f0 34 12 00 00[ 	]*vpermt2w ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 7d 72 7f[ 	]*vpermt2w ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 7d b2 00 10 00 00[ 	]*vpermt2w ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 7d 72 80[ 	]*vpermt2w ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 7d b2 e0 ef ff ff[ 	]*vpermt2w ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 73 fd ab[ 	]*vpslldq xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 73 fd 7b[ 	]*vpslldq xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 39 7b[ 	]*vpslldq xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 73 bc f0 34 12 00 00 7b[ 	]*vpslldq xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 7a 7f 7b[ 	]*vpslldq xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 ba 00 08 00 00 7b[ 	]*vpslldq xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 7a 80 7b[ 	]*vpslldq xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 73 ba f0 f7 ff ff 7b[ 	]*vpslldq xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 73 fd ab[ 	]*vpslldq ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 73 fd 7b[ 	]*vpslldq ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 39 7b[ 	]*vpslldq ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 73 bc f0 34 12 00 00 7b[ 	]*vpslldq ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 7a 7f 7b[ 	]*vpslldq ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 ba 00 10 00 00 7b[ 	]*vpslldq ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 7a 80 7b[ 	]*vpslldq ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 73 ba e0 ef ff ff 7b[ 	]*vpslldq ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 71 f5 ab[ 	]*vpsllw xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 07 71 f5 ab[ 	]*vpsllw xmm30\{k7\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 87 71 f5 ab[ 	]*vpsllw xmm30\{k7\}\{z\},xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 00 71 f5 7b[ 	]*vpsllw xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 31 7b[ 	]*vpsllw xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 00 71 b4 f0 34 12 00 00 7b[ 	]*vpsllw xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 72 7f 7b[ 	]*vpsllw xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 b2 00 08 00 00 7b[ 	]*vpsllw xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 72 80 7b[ 	]*vpsllw xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 00 71 b2 f0 f7 ff ff 7b[ 	]*vpsllw xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 71 f5 ab[ 	]*vpsllw ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 27 71 f5 ab[ 	]*vpsllw ymm30\{k7\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d a7 71 f5 ab[ 	]*vpsllw ymm30\{k7\}\{z\},ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 91 0d 20 71 f5 7b[ 	]*vpsllw ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 31 7b[ 	]*vpsllw ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b1 0d 20 71 b4 f0 34 12 00 00 7b[ 	]*vpsllw ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 72 7f 7b[ 	]*vpsllw ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 b2 00 10 00 00 7b[ 	]*vpsllw ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 72 80 7b[ 	]*vpsllw ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 0d 20 71 b2 e0 ef ff ff 7b[ 	]*vpsllw ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 12 f4[ 	]*vpsllvw xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 12 f4[ 	]*vpsllvw xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 12 f4[ 	]*vpsllvw xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 12 31[ 	]*vpsllvw xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 12 b4 f0 34 12 00 00[ 	]*vpsllvw xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 12 72 7f[ 	]*vpsllvw xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 12 b2 00 08 00 00[ 	]*vpsllvw xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 12 72 80[ 	]*vpsllvw xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 12 b2 f0 f7 ff ff[ 	]*vpsllvw xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 12 f4[ 	]*vpsllvw ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 12 f4[ 	]*vpsllvw ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 12 f4[ 	]*vpsllvw ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 12 31[ 	]*vpsllvw ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 12 b4 f0 34 12 00 00[ 	]*vpsllvw ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 12 72 7f[ 	]*vpsllvw ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 12 b2 00 10 00 00[ 	]*vpsllvw ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 12 72 80[ 	]*vpsllvw ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 12 b2 e0 ef ff ff[ 	]*vpsllvw ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 08 6f f5[ 	]*vmovdqu8 xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 0f 6f f5[ 	]*vmovdqu8 xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 8f 6f f5[ 	]*vmovdqu8 xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 6f 31[ 	]*vmovdqu8 xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 08 6f b4 f0 34 12 00 00[ 	]*vmovdqu8 xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 6f 72 7f[ 	]*vmovdqu8 xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 6f b2 00 08 00 00[ 	]*vmovdqu8 xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 6f 72 80[ 	]*vmovdqu8 xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 6f b2 f0 f7 ff ff[ 	]*vmovdqu8 xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 28 6f f5[ 	]*vmovdqu8 ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7f 2f 6f f5[ 	]*vmovdqu8 ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 7f af 6f f5[ 	]*vmovdqu8 ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 6f 31[ 	]*vmovdqu8 ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 28 6f b4 f0 34 12 00 00[ 	]*vmovdqu8 ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 6f 72 7f[ 	]*vmovdqu8 ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 6f b2 00 10 00 00[ 	]*vmovdqu8 ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 6f 72 80[ 	]*vmovdqu8 ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 6f b2 e0 ef ff ff[ 	]*vmovdqu8 ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 08 6f f5[ 	]*vmovdqu16 xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 0f 6f f5[ 	]*vmovdqu16 xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 8f 6f f5[ 	]*vmovdqu16 xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 6f 31[ 	]*vmovdqu16 xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 08 6f b4 f0 34 12 00 00[ 	]*vmovdqu16 xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 6f 72 7f[ 	]*vmovdqu16 xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 6f b2 00 08 00 00[ 	]*vmovdqu16 xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 6f 72 80[ 	]*vmovdqu16 xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 6f b2 f0 f7 ff ff[ 	]*vmovdqu16 xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 28 6f f5[ 	]*vmovdqu16 ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff 2f 6f f5[ 	]*vmovdqu16 ymm30\{k7\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 01 ff af 6f f5[ 	]*vmovdqu16 ymm30\{k7\}\{z\},ymm29
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 6f 31[ 	]*vmovdqu16 ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 28 6f b4 f0 34 12 00 00[ 	]*vmovdqu16 ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 6f 72 7f[ 	]*vmovdqu16 ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 6f b2 00 10 00 00[ 	]*vmovdqu16 ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 6f 72 80[ 	]*vmovdqu16 ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 6f b2 e0 ef ff ff[ 	]*vmovdqu16 ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 30 31[ 	]*vpmovwb QWORD PTR \[rcx\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 0f 30 31[ 	]*vpmovwb QWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 08 30 b4 f0 34 12 00 00[ 	]*vpmovwb QWORD PTR \[rax\+r14\*8\+0x1234\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 30 72 7f[ 	]*vpmovwb QWORD PTR \[rdx\+0x3f8\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 30 b2 00 04 00 00[ 	]*vpmovwb QWORD PTR \[rdx\+0x400\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 30 72 80[ 	]*vpmovwb QWORD PTR \[rdx-0x400\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 30 b2 f8 fb ff ff[ 	]*vpmovwb QWORD PTR \[rdx-0x408\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 30 31[ 	]*vpmovwb XMMWORD PTR \[rcx\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 2f 30 31[ 	]*vpmovwb XMMWORD PTR \[rcx\]\{k7\},ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 28 30 b4 f0 34 12 00 00[ 	]*vpmovwb XMMWORD PTR \[rax\+r14\*8\+0x1234\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 30 72 7f[ 	]*vpmovwb XMMWORD PTR \[rdx\+0x7f0\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 30 b2 00 08 00 00[ 	]*vpmovwb XMMWORD PTR \[rdx\+0x800\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 30 72 80[ 	]*vpmovwb XMMWORD PTR \[rdx-0x800\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 30 b2 f0 f7 ff ff[ 	]*vpmovwb XMMWORD PTR \[rdx-0x810\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 20 31[ 	]*vpmovswb QWORD PTR \[rcx\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 0f 20 31[ 	]*vpmovswb QWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 08 20 b4 f0 34 12 00 00[ 	]*vpmovswb QWORD PTR \[rax\+r14\*8\+0x1234\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 20 72 7f[ 	]*vpmovswb QWORD PTR \[rdx\+0x3f8\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 20 b2 00 04 00 00[ 	]*vpmovswb QWORD PTR \[rdx\+0x400\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 20 72 80[ 	]*vpmovswb QWORD PTR \[rdx-0x400\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 20 b2 f8 fb ff ff[ 	]*vpmovswb QWORD PTR \[rdx-0x408\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 20 31[ 	]*vpmovswb XMMWORD PTR \[rcx\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 2f 20 31[ 	]*vpmovswb XMMWORD PTR \[rcx\]\{k7\},ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 28 20 b4 f0 34 12 00 00[ 	]*vpmovswb XMMWORD PTR \[rax\+r14\*8\+0x1234\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 20 72 7f[ 	]*vpmovswb XMMWORD PTR \[rdx\+0x7f0\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 20 b2 00 08 00 00[ 	]*vpmovswb XMMWORD PTR \[rdx\+0x800\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 20 72 80[ 	]*vpmovswb XMMWORD PTR \[rdx-0x800\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 20 b2 f0 f7 ff ff[ 	]*vpmovswb XMMWORD PTR \[rdx-0x810\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 10 31[ 	]*vpmovuswb QWORD PTR \[rcx\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 0f 10 31[ 	]*vpmovuswb QWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 08 10 b4 f0 34 12 00 00[ 	]*vpmovuswb QWORD PTR \[rax\+r14\*8\+0x1234\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 10 72 7f[ 	]*vpmovuswb QWORD PTR \[rdx\+0x3f8\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 10 b2 00 04 00 00[ 	]*vpmovuswb QWORD PTR \[rdx\+0x400\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 10 72 80[ 	]*vpmovuswb QWORD PTR \[rdx-0x400\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 10 b2 f8 fb ff ff[ 	]*vpmovuswb QWORD PTR \[rdx-0x408\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 10 31[ 	]*vpmovuswb XMMWORD PTR \[rcx\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 2f 10 31[ 	]*vpmovuswb XMMWORD PTR \[rcx\]\{k7\},ymm30
[ 	]*[a-f0-9]+:[ 	]*62 22 7e 28 10 b4 f0 34 12 00 00[ 	]*vpmovuswb XMMWORD PTR \[rax\+r14\*8\+0x1234\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 10 72 7f[ 	]*vpmovuswb XMMWORD PTR \[rdx\+0x7f0\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 10 b2 00 08 00 00[ 	]*vpmovuswb XMMWORD PTR \[rdx\+0x800\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 10 72 80[ 	]*vpmovuswb XMMWORD PTR \[rdx-0x800\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 10 b2 f0 f7 ff ff[ 	]*vpmovuswb XMMWORD PTR \[rdx-0x810\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 7f 31[ 	]*vmovdqu8 XMMWORD PTR \[rcx\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 0f 7f 31[ 	]*vmovdqu8 XMMWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 08 7f b4 f0 34 12 00 00[ 	]*vmovdqu8 XMMWORD PTR \[rax\+r14\*8\+0x1234\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 7f 72 7f[ 	]*vmovdqu8 XMMWORD PTR \[rdx\+0x7f0\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 7f b2 00 08 00 00[ 	]*vmovdqu8 XMMWORD PTR \[rdx\+0x800\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 7f 72 80[ 	]*vmovdqu8 XMMWORD PTR \[rdx-0x800\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 08 7f b2 f0 f7 ff ff[ 	]*vmovdqu8 XMMWORD PTR \[rdx-0x810\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 7f 31[ 	]*vmovdqu8 YMMWORD PTR \[rcx\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 2f 7f 31[ 	]*vmovdqu8 YMMWORD PTR \[rcx\]\{k7\},ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 7f 28 7f b4 f0 34 12 00 00[ 	]*vmovdqu8 YMMWORD PTR \[rax\+r14\*8\+0x1234\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 7f 72 7f[ 	]*vmovdqu8 YMMWORD PTR \[rdx\+0xfe0\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 7f b2 00 10 00 00[ 	]*vmovdqu8 YMMWORD PTR \[rdx\+0x1000\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 7f 72 80[ 	]*vmovdqu8 YMMWORD PTR \[rdx-0x1000\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 7f 28 7f b2 e0 ef ff ff[ 	]*vmovdqu8 YMMWORD PTR \[rdx-0x1020\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7f 31[ 	]*vmovdqu16 XMMWORD PTR \[rcx\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 0f 7f 31[ 	]*vmovdqu16 XMMWORD PTR \[rcx\]\{k7\},xmm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 08 7f b4 f0 34 12 00 00[ 	]*vmovdqu16 XMMWORD PTR \[rax\+r14\*8\+0x1234\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7f 72 7f[ 	]*vmovdqu16 XMMWORD PTR \[rdx\+0x7f0\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7f b2 00 08 00 00[ 	]*vmovdqu16 XMMWORD PTR \[rdx\+0x800\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7f 72 80[ 	]*vmovdqu16 XMMWORD PTR \[rdx-0x800\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 08 7f b2 f0 f7 ff ff[ 	]*vmovdqu16 XMMWORD PTR \[rdx-0x810\],xmm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7f 31[ 	]*vmovdqu16 YMMWORD PTR \[rcx\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 2f 7f 31[ 	]*vmovdqu16 YMMWORD PTR \[rcx\]\{k7\},ymm30
[ 	]*[a-f0-9]+:[ 	]*62 21 ff 28 7f b4 f0 34 12 00 00[ 	]*vmovdqu16 YMMWORD PTR \[rax\+r14\*8\+0x1234\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7f 72 7f[ 	]*vmovdqu16 YMMWORD PTR \[rdx\+0xfe0\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7f b2 00 10 00 00[ 	]*vmovdqu16 YMMWORD PTR \[rdx\+0x1000\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7f 72 80[ 	]*vmovdqu16 YMMWORD PTR \[rdx-0x1000\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 61 ff 28 7f b2 e0 ef ff ff[ 	]*vmovdqu16 YMMWORD PTR \[rdx-0x1020\],ymm30
[ 	]*[a-f0-9]+:[ 	]*62 02 95 00 75 f4[ 	]*vpermi2w xmm30,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 07 75 f4[ 	]*vpermi2w xmm30\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 87 75 f4[ 	]*vpermi2w xmm30\{k7\}\{z\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 75 31[ 	]*vpermi2w xmm30,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 00 75 b4 f0 34 12 00 00[ 	]*vpermi2w xmm30,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 75 72 7f[ 	]*vpermi2w xmm30,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 75 b2 00 08 00 00[ 	]*vpermi2w xmm30,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 75 72 80[ 	]*vpermi2w xmm30,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 00 75 b2 f0 f7 ff ff[ 	]*vpermi2w xmm30,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 95 20 75 f4[ 	]*vpermi2w ymm30,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 27 75 f4[ 	]*vpermi2w ymm30\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 02 95 a7 75 f4[ 	]*vpermi2w ymm30\{k7\}\{z\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 75 31[ 	]*vpermi2w ymm30,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 95 20 75 b4 f0 34 12 00 00[ 	]*vpermi2w ymm30,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 75 72 7f[ 	]*vpermi2w ymm30,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 75 b2 00 10 00 00[ 	]*vpermi2w ymm30,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 75 72 80[ 	]*vpermi2w ymm30,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 62 95 20 75 b2 e0 ef ff ff[ 	]*vpermi2w ymm30,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 92 0d 00 26 ed[ 	]*vptestmb k5,xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 92 0d 07 26 ed[ 	]*vptestmb k5\{k7\},xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 00 26 29[ 	]*vptestmb k5,xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 0d 00 26 ac f0 34 12 00 00[ 	]*vptestmb k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 00 26 6a 7f[ 	]*vptestmb k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 00 26 aa 00 08 00 00[ 	]*vptestmb k5,xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 00 26 6a 80[ 	]*vptestmb k5,xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 00 26 aa f0 f7 ff ff[ 	]*vptestmb k5,xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 92 0d 20 26 ed[ 	]*vptestmb k5,ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 92 0d 27 26 ed[ 	]*vptestmb k5\{k7\},ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 20 26 29[ 	]*vptestmb k5,ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 0d 20 26 ac f0 34 12 00 00[ 	]*vptestmb k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 20 26 6a 7f[ 	]*vptestmb k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 20 26 aa 00 10 00 00[ 	]*vptestmb k5,ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 20 26 6a 80[ 	]*vptestmb k5,ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 0d 20 26 aa e0 ef ff ff[ 	]*vptestmb k5,ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 92 8d 00 26 ed[ 	]*vptestmw k5,xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 92 8d 07 26 ed[ 	]*vptestmw k5\{k7\},xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 00 26 29[ 	]*vptestmw k5,xmm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 8d 00 26 ac f0 34 12 00 00[ 	]*vptestmw k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 00 26 6a 7f[ 	]*vptestmw k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 00 26 aa 00 08 00 00[ 	]*vptestmw k5,xmm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 00 26 6a 80[ 	]*vptestmw k5,xmm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 00 26 aa f0 f7 ff ff[ 	]*vptestmw k5,xmm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 92 8d 20 26 ed[ 	]*vptestmw k5,ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 92 8d 27 26 ed[ 	]*vptestmw k5\{k7\},ymm30,ymm29
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 20 26 29[ 	]*vptestmw k5,ymm30,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 8d 20 26 ac f0 34 12 00 00[ 	]*vptestmw k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 20 26 6a 7f[ 	]*vptestmw k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 20 26 aa 00 10 00 00[ 	]*vptestmw k5,ymm30,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 20 26 6a 80[ 	]*vptestmw k5,ymm30,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 8d 20 26 aa e0 ef ff ff[ 	]*vptestmw k5,ymm30,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 08 29 ee[ 	]*vpmovb2m k5,xmm30
[ 	]*[a-f0-9]+:[ 	]*62 92 7e 28 29 ee[ 	]*vpmovb2m k5,ymm30
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 08 29 ee[ 	]*vpmovw2m k5,xmm30
[ 	]*[a-f0-9]+:[ 	]*62 92 fe 28 29 ee[ 	]*vpmovw2m k5,ymm30
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 08 28 f5[ 	]*vpmovm2b xmm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 7e 28 28 f5[ 	]*vpmovm2b ymm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 08 28 f5[ 	]*vpmovm2w xmm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 62 fe 28 28 f5[ 	]*vpmovm2w ymm30,k5
[ 	]*[a-f0-9]+:[ 	]*62 92 16 00 26 ec[ 	]*vptestnmb k5,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 92 16 07 26 ec[ 	]*vptestnmb k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 00 26 29[ 	]*vptestnmb k5,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 16 00 26 ac f0 34 12 00 00[ 	]*vptestnmb k5,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 00 26 6a 7f[ 	]*vptestnmb k5,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 00 26 aa 00 08 00 00[ 	]*vptestnmb k5,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 00 26 6a 80[ 	]*vptestnmb k5,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 00 26 aa f0 f7 ff ff[ 	]*vptestnmb k5,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 92 16 20 26 ec[ 	]*vptestnmb k5,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 92 16 27 26 ec[ 	]*vptestnmb k5\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 20 26 29[ 	]*vptestnmb k5,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 16 20 26 ac f0 34 12 00 00[ 	]*vptestnmb k5,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 20 26 6a 7f[ 	]*vptestnmb k5,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 20 26 aa 00 10 00 00[ 	]*vptestnmb k5,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 20 26 6a 80[ 	]*vptestnmb k5,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 16 20 26 aa e0 ef ff ff[ 	]*vptestnmb k5,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 92 96 00 26 ec[ 	]*vptestnmw k5,xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 92 96 07 26 ec[ 	]*vptestnmw k5\{k7\},xmm29,xmm28
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 00 26 29[ 	]*vptestnmw k5,xmm29,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 96 00 26 ac f0 34 12 00 00[ 	]*vptestnmw k5,xmm29,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 00 26 6a 7f[ 	]*vptestnmw k5,xmm29,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 00 26 aa 00 08 00 00[ 	]*vptestnmw k5,xmm29,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 00 26 6a 80[ 	]*vptestnmw k5,xmm29,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 00 26 aa f0 f7 ff ff[ 	]*vptestnmw k5,xmm29,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 92 96 20 26 ec[ 	]*vptestnmw k5,ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 92 96 27 26 ec[ 	]*vptestnmw k5\{k7\},ymm29,ymm28
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 20 26 29[ 	]*vptestnmw k5,ymm29,YMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 b2 96 20 26 ac f0 34 12 00 00[ 	]*vptestnmw k5,ymm29,YMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 20 26 6a 7f[ 	]*vptestnmw k5,ymm29,YMMWORD PTR \[rdx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 20 26 aa 00 10 00 00[ 	]*vptestnmw k5,ymm29,YMMWORD PTR \[rdx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 20 26 6a 80[ 	]*vptestnmw k5,ymm29,YMMWORD PTR \[rdx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 96 20 26 aa e0 ef ff ff[ 	]*vptestnmw k5,ymm29,YMMWORD PTR \[rdx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 00 3f ed ab[ 	]*vpcmpb k5,xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 07 3f ed ab[ 	]*vpcmpb k5\{k7\},xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 00 3f ed 7b[ 	]*vpcmpb k5,xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3f 29 7b[ 	]*vpcmpb k5,xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 0d 00 3f ac f0 34 12 00 00 7b[ 	]*vpcmpb k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3f 6a 7f 7b[ 	]*vpcmpb k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3f aa 00 08 00 00 7b[ 	]*vpcmpb k5,xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3f 6a 80 7b[ 	]*vpcmpb k5,xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3f aa f0 f7 ff ff 7b[ 	]*vpcmpb k5,xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 20 3f ed ab[ 	]*vpcmpb k5,ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 27 3f ed ab[ 	]*vpcmpb k5\{k7\},ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 20 3f ed 7b[ 	]*vpcmpb k5,ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3f 29 7b[ 	]*vpcmpb k5,ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 0d 20 3f ac f0 34 12 00 00 7b[ 	]*vpcmpb k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3f 6a 7f 7b[ 	]*vpcmpb k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3f aa 00 10 00 00 7b[ 	]*vpcmpb k5,ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3f 6a 80 7b[ 	]*vpcmpb k5,ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3f aa e0 ef ff ff 7b[ 	]*vpcmpb k5,ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 00 3f ed ab[ 	]*vpcmpw k5,xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 07 3f ed ab[ 	]*vpcmpw k5\{k7\},xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 00 3f ed 7b[ 	]*vpcmpw k5,xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3f 29 7b[ 	]*vpcmpw k5,xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 8d 00 3f ac f0 34 12 00 00 7b[ 	]*vpcmpw k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3f 6a 7f 7b[ 	]*vpcmpw k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3f aa 00 08 00 00 7b[ 	]*vpcmpw k5,xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3f 6a 80 7b[ 	]*vpcmpw k5,xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3f aa f0 f7 ff ff 7b[ 	]*vpcmpw k5,xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 20 3f ed ab[ 	]*vpcmpw k5,ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 27 3f ed ab[ 	]*vpcmpw k5\{k7\},ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 20 3f ed 7b[ 	]*vpcmpw k5,ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3f 29 7b[ 	]*vpcmpw k5,ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 8d 20 3f ac f0 34 12 00 00 7b[ 	]*vpcmpw k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3f 6a 7f 7b[ 	]*vpcmpw k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3f aa 00 10 00 00 7b[ 	]*vpcmpw k5,ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3f 6a 80 7b[ 	]*vpcmpw k5,ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3f aa e0 ef ff ff 7b[ 	]*vpcmpw k5,ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 00 3e ed ab[ 	]*vpcmpub k5,xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 07 3e ed ab[ 	]*vpcmpub k5\{k7\},xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 00 3e ed 7b[ 	]*vpcmpub k5,xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3e 29 7b[ 	]*vpcmpub k5,xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 0d 00 3e ac f0 34 12 00 00 7b[ 	]*vpcmpub k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3e 6a 7f 7b[ 	]*vpcmpub k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3e aa 00 08 00 00 7b[ 	]*vpcmpub k5,xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3e 6a 80 7b[ 	]*vpcmpub k5,xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 00 3e aa f0 f7 ff ff 7b[ 	]*vpcmpub k5,xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 20 3e ed ab[ 	]*vpcmpub k5,ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 27 3e ed ab[ 	]*vpcmpub k5\{k7\},ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 0d 20 3e ed 7b[ 	]*vpcmpub k5,ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3e 29 7b[ 	]*vpcmpub k5,ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 0d 20 3e ac f0 34 12 00 00 7b[ 	]*vpcmpub k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3e 6a 7f 7b[ 	]*vpcmpub k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3e aa 00 10 00 00 7b[ 	]*vpcmpub k5,ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3e 6a 80 7b[ 	]*vpcmpub k5,ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 0d 20 3e aa e0 ef ff ff 7b[ 	]*vpcmpub k5,ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 00 3e ed ab[ 	]*vpcmpuw k5,xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 07 3e ed ab[ 	]*vpcmpuw k5\{k7\},xmm30,xmm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 00 3e ed 7b[ 	]*vpcmpuw k5,xmm30,xmm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3e 29 7b[ 	]*vpcmpuw k5,xmm30,XMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 8d 00 3e ac f0 34 12 00 00 7b[ 	]*vpcmpuw k5,xmm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3e 6a 7f 7b[ 	]*vpcmpuw k5,xmm30,XMMWORD PTR \[rdx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3e aa 00 08 00 00 7b[ 	]*vpcmpuw k5,xmm30,XMMWORD PTR \[rdx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3e 6a 80 7b[ 	]*vpcmpuw k5,xmm30,XMMWORD PTR \[rdx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 00 3e aa f0 f7 ff ff 7b[ 	]*vpcmpuw k5,xmm30,XMMWORD PTR \[rdx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 20 3e ed ab[ 	]*vpcmpuw k5,ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 27 3e ed ab[ 	]*vpcmpuw k5\{k7\},ymm30,ymm29,0xab
[ 	]*[a-f0-9]+:[ 	]*62 93 8d 20 3e ed 7b[ 	]*vpcmpuw k5,ymm30,ymm29,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3e 29 7b[ 	]*vpcmpuw k5,ymm30,YMMWORD PTR \[rcx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 b3 8d 20 3e ac f0 34 12 00 00 7b[ 	]*vpcmpuw k5,ymm30,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3e 6a 7f 7b[ 	]*vpcmpuw k5,ymm30,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3e aa 00 10 00 00 7b[ 	]*vpcmpuw k5,ymm30,YMMWORD PTR \[rdx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3e 6a 80 7b[ 	]*vpcmpuw k5,ymm30,YMMWORD PTR \[rdx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 8d 20 3e aa e0 ef ff ff 7b[ 	]*vpcmpuw k5,ymm30,YMMWORD PTR \[rdx-0x1020\],0x7b
#pass
