#as: -mevexwig=1
#objdump: -dw -Mintel
#name: i386 AVX512F/VL wig insns (Intel disassembly)
#source: avx512f_vl-wig.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 f5[ 	]*vpmovsxbd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 21 f5[ 	]*vpmovsxbd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 31[ 	]*vpmovsxbd xmm6\{k7\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 b4 f4 c0 1d fe ff[ 	]*vpmovsxbd xmm6\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 72 7f[ 	]*vpmovsxbd xmm6\{k7\},DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 b2 00 02 00 00[ 	]*vpmovsxbd xmm6\{k7\},DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 72 80[ 	]*vpmovsxbd xmm6\{k7\},DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 b2 fc fd ff ff[ 	]*vpmovsxbd xmm6\{k7\},DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 f5[ 	]*vpmovsxbd ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 21 f5[ 	]*vpmovsxbd ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 31[ 	]*vpmovsxbd ymm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 b4 f4 c0 1d fe ff[ 	]*vpmovsxbd ymm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 72 7f[ 	]*vpmovsxbd ymm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 b2 00 04 00 00[ 	]*vpmovsxbd ymm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 72 80[ 	]*vpmovsxbd ymm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 b2 f8 fb ff ff[ 	]*vpmovsxbd ymm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 f5[ 	]*vpmovsxbq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 22 f5[ 	]*vpmovsxbq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 31[ 	]*vpmovsxbq xmm6\{k7\},WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 b4 f4 c0 1d fe ff[ 	]*vpmovsxbq xmm6\{k7\},WORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 72 7f[ 	]*vpmovsxbq xmm6\{k7\},WORD PTR \[edx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 b2 00 01 00 00[ 	]*vpmovsxbq xmm6\{k7\},WORD PTR \[edx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 72 80[ 	]*vpmovsxbq xmm6\{k7\},WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 b2 fe fe ff ff[ 	]*vpmovsxbq xmm6\{k7\},WORD PTR \[edx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 f5[ 	]*vpmovsxbq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 22 f5[ 	]*vpmovsxbq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 31[ 	]*vpmovsxbq ymm6\{k7\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 b4 f4 c0 1d fe ff[ 	]*vpmovsxbq ymm6\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 72 7f[ 	]*vpmovsxbq ymm6\{k7\},DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 b2 00 02 00 00[ 	]*vpmovsxbq ymm6\{k7\},DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 72 80[ 	]*vpmovsxbq ymm6\{k7\},DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 b2 fc fd ff ff[ 	]*vpmovsxbq ymm6\{k7\},DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 f5[ 	]*vpmovsxwd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 23 f5[ 	]*vpmovsxwd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 31[ 	]*vpmovsxwd xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 b4 f4 c0 1d fe ff[ 	]*vpmovsxwd xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 72 7f[ 	]*vpmovsxwd xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 b2 00 04 00 00[ 	]*vpmovsxwd xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 72 80[ 	]*vpmovsxwd xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 b2 f8 fb ff ff[ 	]*vpmovsxwd xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 f5[ 	]*vpmovsxwd ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 23 f5[ 	]*vpmovsxwd ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 31[ 	]*vpmovsxwd ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 b4 f4 c0 1d fe ff[ 	]*vpmovsxwd ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 72 7f[ 	]*vpmovsxwd ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 b2 00 08 00 00[ 	]*vpmovsxwd ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 72 80[ 	]*vpmovsxwd ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 b2 f0 f7 ff ff[ 	]*vpmovsxwd ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 f5[ 	]*vpmovsxwq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 24 f5[ 	]*vpmovsxwq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 31[ 	]*vpmovsxwq xmm6\{k7\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 b4 f4 c0 1d fe ff[ 	]*vpmovsxwq xmm6\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 72 7f[ 	]*vpmovsxwq xmm6\{k7\},DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 b2 00 02 00 00[ 	]*vpmovsxwq xmm6\{k7\},DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 72 80[ 	]*vpmovsxwq xmm6\{k7\},DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 b2 fc fd ff ff[ 	]*vpmovsxwq xmm6\{k7\},DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 f5[ 	]*vpmovsxwq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 24 f5[ 	]*vpmovsxwq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 31[ 	]*vpmovsxwq ymm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 b4 f4 c0 1d fe ff[ 	]*vpmovsxwq ymm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 72 7f[ 	]*vpmovsxwq ymm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 b2 00 04 00 00[ 	]*vpmovsxwq ymm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 72 80[ 	]*vpmovsxwq ymm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 b2 f8 fb ff ff[ 	]*vpmovsxwq ymm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 f5[ 	]*vpmovzxbd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 31 f5[ 	]*vpmovzxbd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 31[ 	]*vpmovzxbd xmm6\{k7\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 b4 f4 c0 1d fe ff[ 	]*vpmovzxbd xmm6\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 72 7f[ 	]*vpmovzxbd xmm6\{k7\},DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 b2 00 02 00 00[ 	]*vpmovzxbd xmm6\{k7\},DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 72 80[ 	]*vpmovzxbd xmm6\{k7\},DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 b2 fc fd ff ff[ 	]*vpmovzxbd xmm6\{k7\},DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 f5[ 	]*vpmovzxbd ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 31 f5[ 	]*vpmovzxbd ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 31[ 	]*vpmovzxbd ymm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 b4 f4 c0 1d fe ff[ 	]*vpmovzxbd ymm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 72 7f[ 	]*vpmovzxbd ymm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 b2 00 04 00 00[ 	]*vpmovzxbd ymm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 72 80[ 	]*vpmovzxbd ymm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 b2 f8 fb ff ff[ 	]*vpmovzxbd ymm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 f5[ 	]*vpmovzxbq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 32 f5[ 	]*vpmovzxbq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 31[ 	]*vpmovzxbq xmm6\{k7\},WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 b4 f4 c0 1d fe ff[ 	]*vpmovzxbq xmm6\{k7\},WORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 72 7f[ 	]*vpmovzxbq xmm6\{k7\},WORD PTR \[edx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 b2 00 01 00 00[ 	]*vpmovzxbq xmm6\{k7\},WORD PTR \[edx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 72 80[ 	]*vpmovzxbq xmm6\{k7\},WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 b2 fe fe ff ff[ 	]*vpmovzxbq xmm6\{k7\},WORD PTR \[edx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 f5[ 	]*vpmovzxbq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 32 f5[ 	]*vpmovzxbq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 31[ 	]*vpmovzxbq ymm6\{k7\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 b4 f4 c0 1d fe ff[ 	]*vpmovzxbq ymm6\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 72 7f[ 	]*vpmovzxbq ymm6\{k7\},DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 b2 00 02 00 00[ 	]*vpmovzxbq ymm6\{k7\},DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 72 80[ 	]*vpmovzxbq ymm6\{k7\},DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 b2 fc fd ff ff[ 	]*vpmovzxbq ymm6\{k7\},DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 f5[ 	]*vpmovzxwd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 33 f5[ 	]*vpmovzxwd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 31[ 	]*vpmovzxwd xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 b4 f4 c0 1d fe ff[ 	]*vpmovzxwd xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 72 7f[ 	]*vpmovzxwd xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 b2 00 04 00 00[ 	]*vpmovzxwd xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 72 80[ 	]*vpmovzxwd xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 b2 f8 fb ff ff[ 	]*vpmovzxwd xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 f5[ 	]*vpmovzxwd ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 33 f5[ 	]*vpmovzxwd ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 31[ 	]*vpmovzxwd ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 b4 f4 c0 1d fe ff[ 	]*vpmovzxwd ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 72 7f[ 	]*vpmovzxwd ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 b2 00 08 00 00[ 	]*vpmovzxwd ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 72 80[ 	]*vpmovzxwd ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 b2 f0 f7 ff ff[ 	]*vpmovzxwd ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 f5[ 	]*vpmovzxwq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 34 f5[ 	]*vpmovzxwq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 31[ 	]*vpmovzxwq xmm6\{k7\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 b4 f4 c0 1d fe ff[ 	]*vpmovzxwq xmm6\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 72 7f[ 	]*vpmovzxwq xmm6\{k7\},DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 b2 00 02 00 00[ 	]*vpmovzxwq xmm6\{k7\},DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 72 80[ 	]*vpmovzxwq xmm6\{k7\},DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 b2 fc fd ff ff[ 	]*vpmovzxwq xmm6\{k7\},DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 f5[ 	]*vpmovzxwq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 34 f5[ 	]*vpmovzxwq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 31[ 	]*vpmovzxwq ymm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 b4 f4 c0 1d fe ff[ 	]*vpmovzxwq ymm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 72 7f[ 	]*vpmovzxwq ymm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 b2 00 04 00 00[ 	]*vpmovzxwq ymm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 72 80[ 	]*vpmovzxwq ymm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 b2 f8 fb ff ff[ 	]*vpmovzxwq ymm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 f5[ 	]*vpmovsxbd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 21 f5[ 	]*vpmovsxbd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 31[ 	]*vpmovsxbd xmm6\{k7\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 b4 f4 c0 1d fe ff[ 	]*vpmovsxbd xmm6\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 72 7f[ 	]*vpmovsxbd xmm6\{k7\},DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 b2 00 02 00 00[ 	]*vpmovsxbd xmm6\{k7\},DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 72 80[ 	]*vpmovsxbd xmm6\{k7\},DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 21 b2 fc fd ff ff[ 	]*vpmovsxbd xmm6\{k7\},DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 f5[ 	]*vpmovsxbd ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 21 f5[ 	]*vpmovsxbd ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 31[ 	]*vpmovsxbd ymm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 b4 f4 c0 1d fe ff[ 	]*vpmovsxbd ymm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 72 7f[ 	]*vpmovsxbd ymm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 b2 00 04 00 00[ 	]*vpmovsxbd ymm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 72 80[ 	]*vpmovsxbd ymm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 21 b2 f8 fb ff ff[ 	]*vpmovsxbd ymm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 f5[ 	]*vpmovsxbq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 22 f5[ 	]*vpmovsxbq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 31[ 	]*vpmovsxbq xmm6\{k7\},WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 b4 f4 c0 1d fe ff[ 	]*vpmovsxbq xmm6\{k7\},WORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 72 7f[ 	]*vpmovsxbq xmm6\{k7\},WORD PTR \[edx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 b2 00 01 00 00[ 	]*vpmovsxbq xmm6\{k7\},WORD PTR \[edx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 72 80[ 	]*vpmovsxbq xmm6\{k7\},WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 22 b2 fe fe ff ff[ 	]*vpmovsxbq xmm6\{k7\},WORD PTR \[edx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 f5[ 	]*vpmovsxbq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 22 f5[ 	]*vpmovsxbq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 31[ 	]*vpmovsxbq ymm6\{k7\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 b4 f4 c0 1d fe ff[ 	]*vpmovsxbq ymm6\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 72 7f[ 	]*vpmovsxbq ymm6\{k7\},DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 b2 00 02 00 00[ 	]*vpmovsxbq ymm6\{k7\},DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 72 80[ 	]*vpmovsxbq ymm6\{k7\},DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 22 b2 fc fd ff ff[ 	]*vpmovsxbq ymm6\{k7\},DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 f5[ 	]*vpmovsxwd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 23 f5[ 	]*vpmovsxwd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 31[ 	]*vpmovsxwd xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 b4 f4 c0 1d fe ff[ 	]*vpmovsxwd xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 72 7f[ 	]*vpmovsxwd xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 b2 00 04 00 00[ 	]*vpmovsxwd xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 72 80[ 	]*vpmovsxwd xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 23 b2 f8 fb ff ff[ 	]*vpmovsxwd xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 f5[ 	]*vpmovsxwd ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 23 f5[ 	]*vpmovsxwd ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 31[ 	]*vpmovsxwd ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 b4 f4 c0 1d fe ff[ 	]*vpmovsxwd ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 72 7f[ 	]*vpmovsxwd ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 b2 00 08 00 00[ 	]*vpmovsxwd ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 72 80[ 	]*vpmovsxwd ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 23 b2 f0 f7 ff ff[ 	]*vpmovsxwd ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 f5[ 	]*vpmovsxwq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 24 f5[ 	]*vpmovsxwq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 31[ 	]*vpmovsxwq xmm6\{k7\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 b4 f4 c0 1d fe ff[ 	]*vpmovsxwq xmm6\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 72 7f[ 	]*vpmovsxwq xmm6\{k7\},DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 b2 00 02 00 00[ 	]*vpmovsxwq xmm6\{k7\},DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 72 80[ 	]*vpmovsxwq xmm6\{k7\},DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 24 b2 fc fd ff ff[ 	]*vpmovsxwq xmm6\{k7\},DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 f5[ 	]*vpmovsxwq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 24 f5[ 	]*vpmovsxwq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 31[ 	]*vpmovsxwq ymm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 b4 f4 c0 1d fe ff[ 	]*vpmovsxwq ymm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 72 7f[ 	]*vpmovsxwq ymm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 b2 00 04 00 00[ 	]*vpmovsxwq ymm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 72 80[ 	]*vpmovsxwq ymm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 24 b2 f8 fb ff ff[ 	]*vpmovsxwq ymm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 f5[ 	]*vpmovzxbd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 31 f5[ 	]*vpmovzxbd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 31[ 	]*vpmovzxbd xmm6\{k7\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 b4 f4 c0 1d fe ff[ 	]*vpmovzxbd xmm6\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 72 7f[ 	]*vpmovzxbd xmm6\{k7\},DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 b2 00 02 00 00[ 	]*vpmovzxbd xmm6\{k7\},DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 72 80[ 	]*vpmovzxbd xmm6\{k7\},DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 31 b2 fc fd ff ff[ 	]*vpmovzxbd xmm6\{k7\},DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 f5[ 	]*vpmovzxbd ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 31 f5[ 	]*vpmovzxbd ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 31[ 	]*vpmovzxbd ymm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 b4 f4 c0 1d fe ff[ 	]*vpmovzxbd ymm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 72 7f[ 	]*vpmovzxbd ymm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 b2 00 04 00 00[ 	]*vpmovzxbd ymm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 72 80[ 	]*vpmovzxbd ymm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 31 b2 f8 fb ff ff[ 	]*vpmovzxbd ymm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 f5[ 	]*vpmovzxbq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 32 f5[ 	]*vpmovzxbq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 31[ 	]*vpmovzxbq xmm6\{k7\},WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 b4 f4 c0 1d fe ff[ 	]*vpmovzxbq xmm6\{k7\},WORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 72 7f[ 	]*vpmovzxbq xmm6\{k7\},WORD PTR \[edx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 b2 00 01 00 00[ 	]*vpmovzxbq xmm6\{k7\},WORD PTR \[edx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 72 80[ 	]*vpmovzxbq xmm6\{k7\},WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 32 b2 fe fe ff ff[ 	]*vpmovzxbq xmm6\{k7\},WORD PTR \[edx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 f5[ 	]*vpmovzxbq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 32 f5[ 	]*vpmovzxbq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 31[ 	]*vpmovzxbq ymm6\{k7\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 b4 f4 c0 1d fe ff[ 	]*vpmovzxbq ymm6\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 72 7f[ 	]*vpmovzxbq ymm6\{k7\},DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 b2 00 02 00 00[ 	]*vpmovzxbq ymm6\{k7\},DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 72 80[ 	]*vpmovzxbq ymm6\{k7\},DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 32 b2 fc fd ff ff[ 	]*vpmovzxbq ymm6\{k7\},DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 f5[ 	]*vpmovzxwd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 33 f5[ 	]*vpmovzxwd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 31[ 	]*vpmovzxwd xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 b4 f4 c0 1d fe ff[ 	]*vpmovzxwd xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 72 7f[ 	]*vpmovzxwd xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 b2 00 04 00 00[ 	]*vpmovzxwd xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 72 80[ 	]*vpmovzxwd xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 33 b2 f8 fb ff ff[ 	]*vpmovzxwd xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 f5[ 	]*vpmovzxwd ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 33 f5[ 	]*vpmovzxwd ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 31[ 	]*vpmovzxwd ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 b4 f4 c0 1d fe ff[ 	]*vpmovzxwd ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 72 7f[ 	]*vpmovzxwd ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 b2 00 08 00 00[ 	]*vpmovzxwd ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 72 80[ 	]*vpmovzxwd ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 33 b2 f0 f7 ff ff[ 	]*vpmovzxwd ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 f5[ 	]*vpmovzxwq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 34 f5[ 	]*vpmovzxwq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 31[ 	]*vpmovzxwq xmm6\{k7\},DWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 b4 f4 c0 1d fe ff[ 	]*vpmovzxwq xmm6\{k7\},DWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 72 7f[ 	]*vpmovzxwq xmm6\{k7\},DWORD PTR \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 b2 00 02 00 00[ 	]*vpmovzxwq xmm6\{k7\},DWORD PTR \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 72 80[ 	]*vpmovzxwq xmm6\{k7\},DWORD PTR \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 34 b2 fc fd ff ff[ 	]*vpmovzxwq xmm6\{k7\},DWORD PTR \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 f5[ 	]*vpmovzxwq ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 34 f5[ 	]*vpmovzxwq ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 31[ 	]*vpmovzxwq ymm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 b4 f4 c0 1d fe ff[ 	]*vpmovzxwq ymm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 72 7f[ 	]*vpmovzxwq ymm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 b2 00 04 00 00[ 	]*vpmovzxwq ymm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 72 80[ 	]*vpmovzxwq ymm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 34 b2 f8 fb ff ff[ 	]*vpmovzxwq ymm6\{k7\},QWORD PTR \[edx-0x408\]
#pass
