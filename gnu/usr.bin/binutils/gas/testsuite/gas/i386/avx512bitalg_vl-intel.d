#as:
#objdump: -dw -Mintel
#name: i386 AVX512BITALG/VL insns (Intel disassembly)
#source: avx512bitalg_vl.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8f ec[ 	]*vpshufbitqmb k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8f ac f4 c0 1d fe ff[ 	]*vpshufbitqmb k5\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8f 6a 7f[ 	]*vpshufbitqmb k5\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8f ec[ 	]*vpshufbitqmb k5\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8f ac f4 c0 1d fe ff[ 	]*vpshufbitqmb k5\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8f 6a 7f[ 	]*vpshufbitqmb k5\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 54 f5[ 	]*vpopcntb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 54 f5[ 	]*vpopcntb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntb xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 54 72 7f[ 	]*vpopcntb xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 54 f5[ 	]*vpopcntb ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 54 f5[ 	]*vpopcntb ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntb ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 54 72 7f[ 	]*vpopcntb ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 54 f5[ 	]*vpopcntw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 54 f5[ 	]*vpopcntw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 54 72 7f[ 	]*vpopcntw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 54 f5[ 	]*vpopcntw ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 54 f5[ 	]*vpopcntw ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 54 72 7f[ 	]*vpopcntw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 55 f5[ 	]*vpopcntd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 55 f5[ 	]*vpopcntd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 55 72 7f[ 	]*vpopcntd xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 55 72 7f[ 	]*vpopcntd xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 55 f5[ 	]*vpopcntd ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 55 f5[ 	]*vpopcntd ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 55 72 7f[ 	]*vpopcntd ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 55 72 7f[ 	]*vpopcntd ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 55 f5[ 	]*vpopcntq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 55 f5[ 	]*vpopcntq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 55 72 7f[ 	]*vpopcntq xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 55 72 7f[ 	]*vpopcntq xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 55 f5[ 	]*vpopcntq ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 55 f5[ 	]*vpopcntq ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 55 72 7f[ 	]*vpopcntq ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 55 72 7f[ 	]*vpopcntq ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8f ec[ 	]*vpshufbitqmb k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8f ac f4 c0 1d fe ff[ 	]*vpshufbitqmb k5\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 8f 6a 7f[ 	]*vpshufbitqmb k5\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8f ec[ 	]*vpshufbitqmb k5\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8f ac f4 c0 1d fe ff[ 	]*vpshufbitqmb k5\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 8f 6a 7f[ 	]*vpshufbitqmb k5\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 54 f5[ 	]*vpopcntb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 54 f5[ 	]*vpopcntb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntb xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 54 72 7f[ 	]*vpopcntb xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 54 f5[ 	]*vpopcntb ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 54 f5[ 	]*vpopcntb ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntb ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 54 72 7f[ 	]*vpopcntb ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 54 f5[ 	]*vpopcntw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 54 f5[ 	]*vpopcntw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 54 72 7f[ 	]*vpopcntw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 54 f5[ 	]*vpopcntw ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 54 f5[ 	]*vpopcntw ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 54 b4 f4 c0 1d fe ff[ 	]*vpopcntw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 54 72 7f[ 	]*vpopcntw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 55 f5[ 	]*vpopcntd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 55 f5[ 	]*vpopcntd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 55 72 7f[ 	]*vpopcntd xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 55 72 7f[ 	]*vpopcntd xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 55 32[ 	]*vpopcntd xmm6\{k7\},DWORD BCST \[edx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 55 f5[ 	]*vpopcntd ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 55 f5[ 	]*vpopcntd ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntd ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 55 72 7f[ 	]*vpopcntd ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 55 72 7f[ 	]*vpopcntd ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 55 32[ 	]*vpopcntd ymm6\{k7\},DWORD BCST \[edx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 55 f5[ 	]*vpopcntq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 55 f5[ 	]*vpopcntq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 55 72 7f[ 	]*vpopcntq xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 55 72 7f[ 	]*vpopcntq xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 55 32[ 	]*vpopcntq xmm6\{k7\},QWORD BCST \[edx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 55 f5[ 	]*vpopcntq ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 55 f5[ 	]*vpopcntq ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 55 b4 f4 c0 1d fe ff[ 	]*vpopcntq ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 55 72 7f[ 	]*vpopcntq ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 55 72 7f[ 	]*vpopcntq ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 55 32[ 	]*vpopcntq ymm6\{k7\},QWORD BCST \[edx\]
#pass
