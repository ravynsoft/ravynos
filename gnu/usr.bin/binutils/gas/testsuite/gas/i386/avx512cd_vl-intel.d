#as:
#objdump: -dw -Mintel
#name: i386 AVX512CD/VL insns (Intel disassembly)
#source: avx512cd_vl.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 f5[ 	]*vpconflictd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f c4 f5[ 	]*vpconflictd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 31[ 	]*vpconflictd xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictd xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 30[ 	]*vpconflictd xmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 72 7f[ 	]*vpconflictd xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 b2 00 08 00 00[ 	]*vpconflictd xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 72 80[ 	]*vpconflictd xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 b2 f0 f7 ff ff[ 	]*vpconflictd xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 72 7f[ 	]*vpconflictd xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 b2 00 02 00 00[ 	]*vpconflictd xmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 72 80[ 	]*vpconflictd xmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 b2 fc fd ff ff[ 	]*vpconflictd xmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 f5[ 	]*vpconflictd ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af c4 f5[ 	]*vpconflictd ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 31[ 	]*vpconflictd ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictd ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 30[ 	]*vpconflictd ymm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 72 7f[ 	]*vpconflictd ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 b2 00 10 00 00[ 	]*vpconflictd ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 72 80[ 	]*vpconflictd ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 b2 e0 ef ff ff[ 	]*vpconflictd ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 72 7f[ 	]*vpconflictd ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 b2 00 02 00 00[ 	]*vpconflictd ymm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 72 80[ 	]*vpconflictd ymm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 b2 fc fd ff ff[ 	]*vpconflictd ymm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 f5[ 	]*vpconflictq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f c4 f5[ 	]*vpconflictq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 31[ 	]*vpconflictq xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictq xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 30[ 	]*vpconflictq xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 72 7f[ 	]*vpconflictq xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 b2 00 08 00 00[ 	]*vpconflictq xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 72 80[ 	]*vpconflictq xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 b2 f0 f7 ff ff[ 	]*vpconflictq xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 72 7f[ 	]*vpconflictq xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 b2 00 04 00 00[ 	]*vpconflictq xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 72 80[ 	]*vpconflictq xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 b2 f8 fb ff ff[ 	]*vpconflictq xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 f5[ 	]*vpconflictq ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af c4 f5[ 	]*vpconflictq ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 31[ 	]*vpconflictq ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictq ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 30[ 	]*vpconflictq ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 72 7f[ 	]*vpconflictq ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 b2 00 10 00 00[ 	]*vpconflictq ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 72 80[ 	]*vpconflictq ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 b2 e0 ef ff ff[ 	]*vpconflictq ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 72 7f[ 	]*vpconflictq ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 b2 00 04 00 00[ 	]*vpconflictq ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 72 80[ 	]*vpconflictq ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 b2 f8 fb ff ff[ 	]*vpconflictq ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 f5[ 	]*vplzcntd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 44 f5[ 	]*vplzcntd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 31[ 	]*vplzcntd xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntd xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 30[ 	]*vplzcntd xmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 72 7f[ 	]*vplzcntd xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 b2 00 08 00 00[ 	]*vplzcntd xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 72 80[ 	]*vplzcntd xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 b2 f0 f7 ff ff[ 	]*vplzcntd xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 72 7f[ 	]*vplzcntd xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 b2 00 02 00 00[ 	]*vplzcntd xmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 72 80[ 	]*vplzcntd xmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 b2 fc fd ff ff[ 	]*vplzcntd xmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 f5[ 	]*vplzcntd ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 44 f5[ 	]*vplzcntd ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 31[ 	]*vplzcntd ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntd ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 30[ 	]*vplzcntd ymm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 72 7f[ 	]*vplzcntd ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 b2 00 10 00 00[ 	]*vplzcntd ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 72 80[ 	]*vplzcntd ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 b2 e0 ef ff ff[ 	]*vplzcntd ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 72 7f[ 	]*vplzcntd ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 b2 00 02 00 00[ 	]*vplzcntd ymm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 72 80[ 	]*vplzcntd ymm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 b2 fc fd ff ff[ 	]*vplzcntd ymm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 f5[ 	]*vplzcntq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 44 f5[ 	]*vplzcntq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 31[ 	]*vplzcntq xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntq xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 30[ 	]*vplzcntq xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 72 7f[ 	]*vplzcntq xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 b2 00 08 00 00[ 	]*vplzcntq xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 72 80[ 	]*vplzcntq xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 b2 f0 f7 ff ff[ 	]*vplzcntq xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 72 7f[ 	]*vplzcntq xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 b2 00 04 00 00[ 	]*vplzcntq xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 72 80[ 	]*vplzcntq xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 b2 f8 fb ff ff[ 	]*vplzcntq xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 f5[ 	]*vplzcntq ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 44 f5[ 	]*vplzcntq ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 31[ 	]*vplzcntq ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntq ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 30[ 	]*vplzcntq ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 72 7f[ 	]*vplzcntq ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 b2 00 10 00 00[ 	]*vplzcntq ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 72 80[ 	]*vplzcntq ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 b2 e0 ef ff ff[ 	]*vplzcntq ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 72 7f[ 	]*vplzcntq ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 b2 00 04 00 00[ 	]*vplzcntq ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 72 80[ 	]*vplzcntq ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 b2 f8 fb ff ff[ 	]*vplzcntq ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 3a f6[ 	]*vpbroadcastmw2d xmm6,k6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 3a f6[ 	]*vpbroadcastmw2d ymm6,k6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 2a f6[ 	]*vpbroadcastmb2q xmm6,k6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 2a f6[ 	]*vpbroadcastmb2q ymm6,k6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 f5[ 	]*vpconflictd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f c4 f5[ 	]*vpconflictd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 31[ 	]*vpconflictd xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictd xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 30[ 	]*vpconflictd xmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 72 7f[ 	]*vpconflictd xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 b2 00 08 00 00[ 	]*vpconflictd xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 72 80[ 	]*vpconflictd xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f c4 b2 f0 f7 ff ff[ 	]*vpconflictd xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 72 7f[ 	]*vpconflictd xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 b2 00 02 00 00[ 	]*vpconflictd xmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 72 80[ 	]*vpconflictd xmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f c4 b2 fc fd ff ff[ 	]*vpconflictd xmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 f5[ 	]*vpconflictd ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af c4 f5[ 	]*vpconflictd ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 31[ 	]*vpconflictd ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictd ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 30[ 	]*vpconflictd ymm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 72 7f[ 	]*vpconflictd ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 b2 00 10 00 00[ 	]*vpconflictd ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 72 80[ 	]*vpconflictd ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f c4 b2 e0 ef ff ff[ 	]*vpconflictd ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 72 7f[ 	]*vpconflictd ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 b2 00 02 00 00[ 	]*vpconflictd ymm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 72 80[ 	]*vpconflictd ymm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f c4 b2 fc fd ff ff[ 	]*vpconflictd ymm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 f5[ 	]*vpconflictq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f c4 f5[ 	]*vpconflictq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 31[ 	]*vpconflictq xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictq xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 30[ 	]*vpconflictq xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 72 7f[ 	]*vpconflictq xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 b2 00 08 00 00[ 	]*vpconflictq xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 72 80[ 	]*vpconflictq xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f c4 b2 f0 f7 ff ff[ 	]*vpconflictq xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 72 7f[ 	]*vpconflictq xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 b2 00 04 00 00[ 	]*vpconflictq xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 72 80[ 	]*vpconflictq xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f c4 b2 f8 fb ff ff[ 	]*vpconflictq xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 f5[ 	]*vpconflictq ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af c4 f5[ 	]*vpconflictq ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 31[ 	]*vpconflictq ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 b4 f4 c0 1d fe ff[ 	]*vpconflictq ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 30[ 	]*vpconflictq ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 72 7f[ 	]*vpconflictq ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 b2 00 10 00 00[ 	]*vpconflictq ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 72 80[ 	]*vpconflictq ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f c4 b2 e0 ef ff ff[ 	]*vpconflictq ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 72 7f[ 	]*vpconflictq ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 b2 00 04 00 00[ 	]*vpconflictq ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 72 80[ 	]*vpconflictq ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f c4 b2 f8 fb ff ff[ 	]*vpconflictq ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 f5[ 	]*vplzcntd xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 44 f5[ 	]*vplzcntd xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 31[ 	]*vplzcntd xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntd xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 30[ 	]*vplzcntd xmm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 72 7f[ 	]*vplzcntd xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 b2 00 08 00 00[ 	]*vplzcntd xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 72 80[ 	]*vplzcntd xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 44 b2 f0 f7 ff ff[ 	]*vplzcntd xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 72 7f[ 	]*vplzcntd xmm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 b2 00 02 00 00[ 	]*vplzcntd xmm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 72 80[ 	]*vplzcntd xmm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 1f 44 b2 fc fd ff ff[ 	]*vplzcntd xmm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 f5[ 	]*vplzcntd ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 44 f5[ 	]*vplzcntd ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 31[ 	]*vplzcntd ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntd ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 30[ 	]*vplzcntd ymm6\{k7\},DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 72 7f[ 	]*vplzcntd ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 b2 00 10 00 00[ 	]*vplzcntd ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 72 80[ 	]*vplzcntd ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 44 b2 e0 ef ff ff[ 	]*vplzcntd ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 72 7f[ 	]*vplzcntd ymm6\{k7\},DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 b2 00 02 00 00[ 	]*vplzcntd ymm6\{k7\},DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 72 80[ 	]*vplzcntd ymm6\{k7\},DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 3f 44 b2 fc fd ff ff[ 	]*vplzcntd ymm6\{k7\},DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 f5[ 	]*vplzcntq xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 44 f5[ 	]*vplzcntq xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 31[ 	]*vplzcntq xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntq xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 30[ 	]*vplzcntq xmm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 72 7f[ 	]*vplzcntq xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 b2 00 08 00 00[ 	]*vplzcntq xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 72 80[ 	]*vplzcntq xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 44 b2 f0 f7 ff ff[ 	]*vplzcntq xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 72 7f[ 	]*vplzcntq xmm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 b2 00 04 00 00[ 	]*vplzcntq xmm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 72 80[ 	]*vplzcntq xmm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 1f 44 b2 f8 fb ff ff[ 	]*vplzcntq xmm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 f5[ 	]*vplzcntq ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 44 f5[ 	]*vplzcntq ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 31[ 	]*vplzcntq ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 b4 f4 c0 1d fe ff[ 	]*vplzcntq ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 30[ 	]*vplzcntq ymm6\{k7\},QWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 72 7f[ 	]*vplzcntq ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 b2 00 10 00 00[ 	]*vplzcntq ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 72 80[ 	]*vplzcntq ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 44 b2 e0 ef ff ff[ 	]*vplzcntq ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 72 7f[ 	]*vplzcntq ymm6\{k7\},QWORD BCST \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 b2 00 04 00 00[ 	]*vplzcntq ymm6\{k7\},QWORD BCST \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 72 80[ 	]*vplzcntq ymm6\{k7\},QWORD BCST \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 3f 44 b2 f8 fb ff ff[ 	]*vplzcntq ymm6\{k7\},QWORD BCST \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 3a f6[ 	]*vpbroadcastmw2d xmm6,k6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 3a f6[ 	]*vpbroadcastmw2d ymm6,k6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 2a f6[ 	]*vpbroadcastmb2q xmm6,k6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 2a f6[ 	]*vpbroadcastmb2q ymm6,k6
#pass
