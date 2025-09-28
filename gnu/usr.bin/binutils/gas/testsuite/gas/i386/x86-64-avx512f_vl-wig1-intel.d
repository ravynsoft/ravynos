#as: -mevexwig=1
#objdump: -dw -Mintel
#name: x86_64 AVX512F/VL wig insns (Intel disassembly)
#source: x86-64-avx512f_vl-wig.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 21 f5[ 	]*vpmovsxbd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 21 f5[ 	]*vpmovsxbd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 21 f5[ 	]*vpmovsxbd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 31[ 	]*vpmovsxbd xmm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 21 b4 f0 23 01 00 00[ 	]*vpmovsxbd xmm30,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 72 7f[ 	]*vpmovsxbd xmm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 b2 00 02 00 00[ 	]*vpmovsxbd xmm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 72 80[ 	]*vpmovsxbd xmm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 b2 fc fd ff ff[ 	]*vpmovsxbd xmm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 21 f5[ 	]*vpmovsxbd ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 21 f5[ 	]*vpmovsxbd ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 21 f5[ 	]*vpmovsxbd ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 31[ 	]*vpmovsxbd ymm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 21 b4 f0 23 01 00 00[ 	]*vpmovsxbd ymm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 72 7f[ 	]*vpmovsxbd ymm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 b2 00 04 00 00[ 	]*vpmovsxbd ymm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 72 80[ 	]*vpmovsxbd ymm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 b2 f8 fb ff ff[ 	]*vpmovsxbd ymm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 22 f5[ 	]*vpmovsxbq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 22 f5[ 	]*vpmovsxbq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 22 f5[ 	]*vpmovsxbq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 31[ 	]*vpmovsxbq xmm30,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 22 b4 f0 23 01 00 00[ 	]*vpmovsxbq xmm30,WORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 72 7f[ 	]*vpmovsxbq xmm30,WORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 b2 00 01 00 00[ 	]*vpmovsxbq xmm30,WORD PTR \[rdx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 72 80[ 	]*vpmovsxbq xmm30,WORD PTR \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 b2 fe fe ff ff[ 	]*vpmovsxbq xmm30,WORD PTR \[rdx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 22 f5[ 	]*vpmovsxbq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 22 f5[ 	]*vpmovsxbq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 22 f5[ 	]*vpmovsxbq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 31[ 	]*vpmovsxbq ymm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 22 b4 f0 23 01 00 00[ 	]*vpmovsxbq ymm30,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 72 7f[ 	]*vpmovsxbq ymm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 b2 00 02 00 00[ 	]*vpmovsxbq ymm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 72 80[ 	]*vpmovsxbq ymm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 b2 fc fd ff ff[ 	]*vpmovsxbq ymm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 23 f5[ 	]*vpmovsxwd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 23 f5[ 	]*vpmovsxwd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 23 f5[ 	]*vpmovsxwd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 31[ 	]*vpmovsxwd xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 23 b4 f0 23 01 00 00[ 	]*vpmovsxwd xmm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 72 7f[ 	]*vpmovsxwd xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 b2 00 04 00 00[ 	]*vpmovsxwd xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 72 80[ 	]*vpmovsxwd xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 b2 f8 fb ff ff[ 	]*vpmovsxwd xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 23 f5[ 	]*vpmovsxwd ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 23 f5[ 	]*vpmovsxwd ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 23 f5[ 	]*vpmovsxwd ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 31[ 	]*vpmovsxwd ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 23 b4 f0 23 01 00 00[ 	]*vpmovsxwd ymm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 72 7f[ 	]*vpmovsxwd ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 b2 00 08 00 00[ 	]*vpmovsxwd ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 72 80[ 	]*vpmovsxwd ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 b2 f0 f7 ff ff[ 	]*vpmovsxwd ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 24 f5[ 	]*vpmovsxwq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 24 f5[ 	]*vpmovsxwq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 24 f5[ 	]*vpmovsxwq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 31[ 	]*vpmovsxwq xmm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 24 b4 f0 23 01 00 00[ 	]*vpmovsxwq xmm30,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 72 7f[ 	]*vpmovsxwq xmm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 b2 00 02 00 00[ 	]*vpmovsxwq xmm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 72 80[ 	]*vpmovsxwq xmm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 b2 fc fd ff ff[ 	]*vpmovsxwq xmm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 24 f5[ 	]*vpmovsxwq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 24 f5[ 	]*vpmovsxwq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 24 f5[ 	]*vpmovsxwq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 31[ 	]*vpmovsxwq ymm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 24 b4 f0 23 01 00 00[ 	]*vpmovsxwq ymm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 72 7f[ 	]*vpmovsxwq ymm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 b2 00 04 00 00[ 	]*vpmovsxwq ymm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 72 80[ 	]*vpmovsxwq ymm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 b2 f8 fb ff ff[ 	]*vpmovsxwq ymm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 31 f5[ 	]*vpmovzxbd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 31 f5[ 	]*vpmovzxbd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 31 f5[ 	]*vpmovzxbd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 31[ 	]*vpmovzxbd xmm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 31 b4 f0 23 01 00 00[ 	]*vpmovzxbd xmm30,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 72 7f[ 	]*vpmovzxbd xmm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 b2 00 02 00 00[ 	]*vpmovzxbd xmm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 72 80[ 	]*vpmovzxbd xmm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 b2 fc fd ff ff[ 	]*vpmovzxbd xmm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 31 f5[ 	]*vpmovzxbd ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 31 f5[ 	]*vpmovzxbd ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 31 f5[ 	]*vpmovzxbd ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 31[ 	]*vpmovzxbd ymm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 31 b4 f0 23 01 00 00[ 	]*vpmovzxbd ymm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 72 7f[ 	]*vpmovzxbd ymm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 b2 00 04 00 00[ 	]*vpmovzxbd ymm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 72 80[ 	]*vpmovzxbd ymm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 b2 f8 fb ff ff[ 	]*vpmovzxbd ymm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 32 f5[ 	]*vpmovzxbq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 32 f5[ 	]*vpmovzxbq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 32 f5[ 	]*vpmovzxbq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 31[ 	]*vpmovzxbq xmm30,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 32 b4 f0 23 01 00 00[ 	]*vpmovzxbq xmm30,WORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 72 7f[ 	]*vpmovzxbq xmm30,WORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 b2 00 01 00 00[ 	]*vpmovzxbq xmm30,WORD PTR \[rdx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 72 80[ 	]*vpmovzxbq xmm30,WORD PTR \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 b2 fe fe ff ff[ 	]*vpmovzxbq xmm30,WORD PTR \[rdx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 32 f5[ 	]*vpmovzxbq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 32 f5[ 	]*vpmovzxbq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 32 f5[ 	]*vpmovzxbq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 31[ 	]*vpmovzxbq ymm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 32 b4 f0 23 01 00 00[ 	]*vpmovzxbq ymm30,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 72 7f[ 	]*vpmovzxbq ymm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 b2 00 02 00 00[ 	]*vpmovzxbq ymm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 72 80[ 	]*vpmovzxbq ymm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 b2 fc fd ff ff[ 	]*vpmovzxbq ymm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 33 f5[ 	]*vpmovzxwd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 33 f5[ 	]*vpmovzxwd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 33 f5[ 	]*vpmovzxwd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 31[ 	]*vpmovzxwd xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 33 b4 f0 23 01 00 00[ 	]*vpmovzxwd xmm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 72 7f[ 	]*vpmovzxwd xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 b2 00 04 00 00[ 	]*vpmovzxwd xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 72 80[ 	]*vpmovzxwd xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 b2 f8 fb ff ff[ 	]*vpmovzxwd xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 33 f5[ 	]*vpmovzxwd ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 33 f5[ 	]*vpmovzxwd ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 33 f5[ 	]*vpmovzxwd ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 31[ 	]*vpmovzxwd ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 33 b4 f0 23 01 00 00[ 	]*vpmovzxwd ymm30,XMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 72 7f[ 	]*vpmovzxwd ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 b2 00 08 00 00[ 	]*vpmovzxwd ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 72 80[ 	]*vpmovzxwd ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 b2 f0 f7 ff ff[ 	]*vpmovzxwd ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 34 f5[ 	]*vpmovzxwq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 34 f5[ 	]*vpmovzxwq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 34 f5[ 	]*vpmovzxwq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 31[ 	]*vpmovzxwq xmm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 34 b4 f0 23 01 00 00[ 	]*vpmovzxwq xmm30,DWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 72 7f[ 	]*vpmovzxwq xmm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 b2 00 02 00 00[ 	]*vpmovzxwq xmm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 72 80[ 	]*vpmovzxwq xmm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 b2 fc fd ff ff[ 	]*vpmovzxwq xmm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 34 f5[ 	]*vpmovzxwq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 34 f5[ 	]*vpmovzxwq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 34 f5[ 	]*vpmovzxwq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 31[ 	]*vpmovzxwq ymm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 34 b4 f0 23 01 00 00[ 	]*vpmovzxwq ymm30,QWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 72 7f[ 	]*vpmovzxwq ymm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 b2 00 04 00 00[ 	]*vpmovzxwq ymm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 72 80[ 	]*vpmovzxwq ymm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 b2 f8 fb ff ff[ 	]*vpmovzxwq ymm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 21 f5[ 	]*vpmovsxbd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 21 f5[ 	]*vpmovsxbd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 21 f5[ 	]*vpmovsxbd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 31[ 	]*vpmovsxbd xmm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 21 b4 f0 34 12 00 00[ 	]*vpmovsxbd xmm30,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 72 7f[ 	]*vpmovsxbd xmm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 b2 00 02 00 00[ 	]*vpmovsxbd xmm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 72 80[ 	]*vpmovsxbd xmm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 21 b2 fc fd ff ff[ 	]*vpmovsxbd xmm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 21 f5[ 	]*vpmovsxbd ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 21 f5[ 	]*vpmovsxbd ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 21 f5[ 	]*vpmovsxbd ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 31[ 	]*vpmovsxbd ymm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 21 b4 f0 34 12 00 00[ 	]*vpmovsxbd ymm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 72 7f[ 	]*vpmovsxbd ymm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 b2 00 04 00 00[ 	]*vpmovsxbd ymm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 72 80[ 	]*vpmovsxbd ymm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 21 b2 f8 fb ff ff[ 	]*vpmovsxbd ymm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 22 f5[ 	]*vpmovsxbq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 22 f5[ 	]*vpmovsxbq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 22 f5[ 	]*vpmovsxbq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 31[ 	]*vpmovsxbq xmm30,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 22 b4 f0 34 12 00 00[ 	]*vpmovsxbq xmm30,WORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 72 7f[ 	]*vpmovsxbq xmm30,WORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 b2 00 01 00 00[ 	]*vpmovsxbq xmm30,WORD PTR \[rdx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 72 80[ 	]*vpmovsxbq xmm30,WORD PTR \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 22 b2 fe fe ff ff[ 	]*vpmovsxbq xmm30,WORD PTR \[rdx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 22 f5[ 	]*vpmovsxbq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 22 f5[ 	]*vpmovsxbq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 22 f5[ 	]*vpmovsxbq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 31[ 	]*vpmovsxbq ymm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 22 b4 f0 34 12 00 00[ 	]*vpmovsxbq ymm30,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 72 7f[ 	]*vpmovsxbq ymm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 b2 00 02 00 00[ 	]*vpmovsxbq ymm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 72 80[ 	]*vpmovsxbq ymm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 22 b2 fc fd ff ff[ 	]*vpmovsxbq ymm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 23 f5[ 	]*vpmovsxwd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 23 f5[ 	]*vpmovsxwd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 23 f5[ 	]*vpmovsxwd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 31[ 	]*vpmovsxwd xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 23 b4 f0 34 12 00 00[ 	]*vpmovsxwd xmm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 72 7f[ 	]*vpmovsxwd xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 b2 00 04 00 00[ 	]*vpmovsxwd xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 72 80[ 	]*vpmovsxwd xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 23 b2 f8 fb ff ff[ 	]*vpmovsxwd xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 23 f5[ 	]*vpmovsxwd ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 23 f5[ 	]*vpmovsxwd ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 23 f5[ 	]*vpmovsxwd ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 31[ 	]*vpmovsxwd ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 23 b4 f0 34 12 00 00[ 	]*vpmovsxwd ymm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 72 7f[ 	]*vpmovsxwd ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 b2 00 08 00 00[ 	]*vpmovsxwd ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 72 80[ 	]*vpmovsxwd ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 23 b2 f0 f7 ff ff[ 	]*vpmovsxwd ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 24 f5[ 	]*vpmovsxwq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 24 f5[ 	]*vpmovsxwq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 24 f5[ 	]*vpmovsxwq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 31[ 	]*vpmovsxwq xmm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 24 b4 f0 34 12 00 00[ 	]*vpmovsxwq xmm30,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 72 7f[ 	]*vpmovsxwq xmm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 b2 00 02 00 00[ 	]*vpmovsxwq xmm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 72 80[ 	]*vpmovsxwq xmm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 24 b2 fc fd ff ff[ 	]*vpmovsxwq xmm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 24 f5[ 	]*vpmovsxwq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 24 f5[ 	]*vpmovsxwq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 24 f5[ 	]*vpmovsxwq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 31[ 	]*vpmovsxwq ymm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 24 b4 f0 34 12 00 00[ 	]*vpmovsxwq ymm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 72 7f[ 	]*vpmovsxwq ymm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 b2 00 04 00 00[ 	]*vpmovsxwq ymm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 72 80[ 	]*vpmovsxwq ymm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 24 b2 f8 fb ff ff[ 	]*vpmovsxwq ymm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 31 f5[ 	]*vpmovzxbd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 31 f5[ 	]*vpmovzxbd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 31 f5[ 	]*vpmovzxbd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 31[ 	]*vpmovzxbd xmm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 31 b4 f0 34 12 00 00[ 	]*vpmovzxbd xmm30,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 72 7f[ 	]*vpmovzxbd xmm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 b2 00 02 00 00[ 	]*vpmovzxbd xmm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 72 80[ 	]*vpmovzxbd xmm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 31 b2 fc fd ff ff[ 	]*vpmovzxbd xmm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 31 f5[ 	]*vpmovzxbd ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 31 f5[ 	]*vpmovzxbd ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 31 f5[ 	]*vpmovzxbd ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 31[ 	]*vpmovzxbd ymm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 31 b4 f0 34 12 00 00[ 	]*vpmovzxbd ymm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 72 7f[ 	]*vpmovzxbd ymm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 b2 00 04 00 00[ 	]*vpmovzxbd ymm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 72 80[ 	]*vpmovzxbd ymm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 31 b2 f8 fb ff ff[ 	]*vpmovzxbd ymm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 32 f5[ 	]*vpmovzxbq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 32 f5[ 	]*vpmovzxbq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 32 f5[ 	]*vpmovzxbq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 31[ 	]*vpmovzxbq xmm30,WORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 32 b4 f0 34 12 00 00[ 	]*vpmovzxbq xmm30,WORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 72 7f[ 	]*vpmovzxbq xmm30,WORD PTR \[rdx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 b2 00 01 00 00[ 	]*vpmovzxbq xmm30,WORD PTR \[rdx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 72 80[ 	]*vpmovzxbq xmm30,WORD PTR \[rdx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 32 b2 fe fe ff ff[ 	]*vpmovzxbq xmm30,WORD PTR \[rdx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 32 f5[ 	]*vpmovzxbq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 32 f5[ 	]*vpmovzxbq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 32 f5[ 	]*vpmovzxbq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 31[ 	]*vpmovzxbq ymm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 32 b4 f0 34 12 00 00[ 	]*vpmovzxbq ymm30,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 72 7f[ 	]*vpmovzxbq ymm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 b2 00 02 00 00[ 	]*vpmovzxbq ymm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 72 80[ 	]*vpmovzxbq ymm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 32 b2 fc fd ff ff[ 	]*vpmovzxbq ymm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 33 f5[ 	]*vpmovzxwd xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 33 f5[ 	]*vpmovzxwd xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 33 f5[ 	]*vpmovzxwd xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 31[ 	]*vpmovzxwd xmm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 33 b4 f0 34 12 00 00[ 	]*vpmovzxwd xmm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 72 7f[ 	]*vpmovzxwd xmm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 b2 00 04 00 00[ 	]*vpmovzxwd xmm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 72 80[ 	]*vpmovzxwd xmm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 33 b2 f8 fb ff ff[ 	]*vpmovzxwd xmm30,QWORD PTR \[rdx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 33 f5[ 	]*vpmovzxwd ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 33 f5[ 	]*vpmovzxwd ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 33 f5[ 	]*vpmovzxwd ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 31[ 	]*vpmovzxwd ymm30,XMMWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 33 b4 f0 34 12 00 00[ 	]*vpmovzxwd ymm30,XMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 72 7f[ 	]*vpmovzxwd ymm30,XMMWORD PTR \[rdx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 b2 00 08 00 00[ 	]*vpmovzxwd ymm30,XMMWORD PTR \[rdx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 72 80[ 	]*vpmovzxwd ymm30,XMMWORD PTR \[rdx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 33 b2 f0 f7 ff ff[ 	]*vpmovzxwd ymm30,XMMWORD PTR \[rdx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 08 34 f5[ 	]*vpmovzxwq xmm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 0f 34 f5[ 	]*vpmovzxwq xmm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 8f 34 f5[ 	]*vpmovzxwq xmm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 31[ 	]*vpmovzxwq xmm30,DWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 08 34 b4 f0 34 12 00 00[ 	]*vpmovzxwq xmm30,DWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 72 7f[ 	]*vpmovzxwq xmm30,DWORD PTR \[rdx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 b2 00 02 00 00[ 	]*vpmovzxwq xmm30,DWORD PTR \[rdx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 72 80[ 	]*vpmovzxwq xmm30,DWORD PTR \[rdx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 08 34 b2 fc fd ff ff[ 	]*vpmovzxwq xmm30,DWORD PTR \[rdx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 28 34 f5[ 	]*vpmovzxwq ymm30,xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd 2f 34 f5[ 	]*vpmovzxwq ymm30\{k7\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 02 fd af 34 f5[ 	]*vpmovzxwq ymm30\{k7\}\{z\},xmm29
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 31[ 	]*vpmovzxwq ymm30,QWORD PTR \[rcx\]
[ 	]*[a-f0-9]+:[ 	]*62 22 fd 28 34 b4 f0 34 12 00 00[ 	]*vpmovzxwq ymm30,QWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 72 7f[ 	]*vpmovzxwq ymm30,QWORD PTR \[rdx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 b2 00 04 00 00[ 	]*vpmovzxwq ymm30,QWORD PTR \[rdx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 72 80[ 	]*vpmovzxwq ymm30,QWORD PTR \[rdx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 62 fd 28 34 b2 f8 fb ff ff[ 	]*vpmovzxwq ymm30,QWORD PTR \[rdx-0x408\]
#pass
