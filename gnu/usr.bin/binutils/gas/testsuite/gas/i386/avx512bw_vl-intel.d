#as:
#objdump: -dw -Mintel
#name: i386 AVX512BW/VL insns (Intel disassembly)
#source: avx512bw_vl.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c f5[ 	]*vpabsb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 1c f5[ 	]*vpabsb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c 31[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c b4 f4 c0 1d fe ff[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c 72 7f[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c b2 00 08 00 00[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c 72 80[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c b2 f0 f7 ff ff[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c f5[ 	]*vpabsb ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 1c f5[ 	]*vpabsb ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c 31[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c b4 f4 c0 1d fe ff[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c 72 7f[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c b2 00 10 00 00[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c 72 80[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c b2 e0 ef ff ff[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d f5[ 	]*vpabsw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 1d f5[ 	]*vpabsw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d 31[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d b4 f4 c0 1d fe ff[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d 72 7f[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d b2 00 08 00 00[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d 72 80[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d b2 f0 f7 ff ff[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d f5[ 	]*vpabsw ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 1d f5[ 	]*vpabsw ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d 31[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d b4 f4 c0 1d fe ff[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d 72 7f[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d b2 00 10 00 00[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d 72 80[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d b2 e0 ef ff ff[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b f4[ 	]*vpackssdw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 6b f4[ 	]*vpackssdw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b 31[ 	]*vpackssdw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b b4 f4 c0 1d fe ff[ 	]*vpackssdw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b 30[ 	]*vpackssdw xmm6\{k7\},xmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b 72 7f[ 	]*vpackssdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b b2 00 08 00 00[ 	]*vpackssdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b 72 80[ 	]*vpackssdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b b2 f0 f7 ff ff[ 	]*vpackssdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b 72 7f[ 	]*vpackssdw xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b b2 00 02 00 00[ 	]*vpackssdw xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b 72 80[ 	]*vpackssdw xmm6\{k7\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b b2 fc fd ff ff[ 	]*vpackssdw xmm6\{k7\},xmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b f4[ 	]*vpackssdw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 6b f4[ 	]*vpackssdw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b 31[ 	]*vpackssdw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b b4 f4 c0 1d fe ff[ 	]*vpackssdw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b 30[ 	]*vpackssdw ymm6\{k7\},ymm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b 72 7f[ 	]*vpackssdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b b2 00 10 00 00[ 	]*vpackssdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b 72 80[ 	]*vpackssdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b b2 e0 ef ff ff[ 	]*vpackssdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b 72 7f[ 	]*vpackssdw ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b b2 00 02 00 00[ 	]*vpackssdw ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b 72 80[ 	]*vpackssdw ymm6\{k7\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b b2 fc fd ff ff[ 	]*vpackssdw ymm6\{k7\},ymm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 f4[ 	]*vpacksswb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 63 f4[ 	]*vpacksswb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 31[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 b4 f4 c0 1d fe ff[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 72 7f[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 b2 00 08 00 00[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 72 80[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 b2 f0 f7 ff ff[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 f4[ 	]*vpacksswb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 63 f4[ 	]*vpacksswb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 31[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 b4 f4 c0 1d fe ff[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 72 7f[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 b2 00 10 00 00[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 72 80[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 b2 e0 ef ff ff[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b f4[ 	]*vpackusdw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 2b f4[ 	]*vpackusdw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b 31[ 	]*vpackusdw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b b4 f4 c0 1d fe ff[ 	]*vpackusdw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b 30[ 	]*vpackusdw xmm6\{k7\},xmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b 72 7f[ 	]*vpackusdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b b2 00 08 00 00[ 	]*vpackusdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b 72 80[ 	]*vpackusdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b b2 f0 f7 ff ff[ 	]*vpackusdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b 72 7f[ 	]*vpackusdw xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b b2 00 02 00 00[ 	]*vpackusdw xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b 72 80[ 	]*vpackusdw xmm6\{k7\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b b2 fc fd ff ff[ 	]*vpackusdw xmm6\{k7\},xmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b f4[ 	]*vpackusdw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 2b f4[ 	]*vpackusdw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b 31[ 	]*vpackusdw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b b4 f4 c0 1d fe ff[ 	]*vpackusdw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b 30[ 	]*vpackusdw ymm6\{k7\},ymm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b 72 7f[ 	]*vpackusdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b b2 00 10 00 00[ 	]*vpackusdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b 72 80[ 	]*vpackusdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b b2 e0 ef ff ff[ 	]*vpackusdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b 72 7f[ 	]*vpackusdw ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b b2 00 02 00 00[ 	]*vpackusdw ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b 72 80[ 	]*vpackusdw ymm6\{k7\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b b2 fc fd ff ff[ 	]*vpackusdw ymm6\{k7\},ymm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 f4[ 	]*vpackuswb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 67 f4[ 	]*vpackuswb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 31[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 b4 f4 c0 1d fe ff[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 72 7f[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 b2 00 08 00 00[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 72 80[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 b2 f0 f7 ff ff[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 f4[ 	]*vpackuswb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 67 f4[ 	]*vpackuswb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 31[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 b4 f4 c0 1d fe ff[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 72 7f[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 b2 00 10 00 00[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 72 80[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 b2 e0 ef ff ff[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc f4[ 	]*vpaddb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f fc f4[ 	]*vpaddb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc 31[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc b4 f4 c0 1d fe ff[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc 72 7f[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc b2 00 08 00 00[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc 72 80[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc b2 f0 f7 ff ff[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc f4[ 	]*vpaddb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af fc f4[ 	]*vpaddb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc 31[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc b4 f4 c0 1d fe ff[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc 72 7f[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc b2 00 10 00 00[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc 72 80[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc b2 e0 ef ff ff[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec f4[ 	]*vpaddsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ec f4[ 	]*vpaddsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec 31[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec b4 f4 c0 1d fe ff[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec 72 7f[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec b2 00 08 00 00[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec 72 80[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec b2 f0 f7 ff ff[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec f4[ 	]*vpaddsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ec f4[ 	]*vpaddsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec 31[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec b4 f4 c0 1d fe ff[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec 72 7f[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec b2 00 10 00 00[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec 72 80[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec b2 e0 ef ff ff[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed f4[ 	]*vpaddsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ed f4[ 	]*vpaddsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed 31[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed b4 f4 c0 1d fe ff[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed 72 7f[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed b2 00 08 00 00[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed 72 80[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed b2 f0 f7 ff ff[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed f4[ 	]*vpaddsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ed f4[ 	]*vpaddsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed 31[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed b4 f4 c0 1d fe ff[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed 72 7f[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed b2 00 10 00 00[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed 72 80[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed b2 e0 ef ff ff[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc f4[ 	]*vpaddusb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f dc f4[ 	]*vpaddusb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc 31[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc b4 f4 c0 1d fe ff[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc 72 7f[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc b2 00 08 00 00[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc 72 80[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc b2 f0 f7 ff ff[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc f4[ 	]*vpaddusb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af dc f4[ 	]*vpaddusb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc 31[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc b4 f4 c0 1d fe ff[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc 72 7f[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc b2 00 10 00 00[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc 72 80[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc b2 e0 ef ff ff[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd f4[ 	]*vpaddusw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f dd f4[ 	]*vpaddusw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd 31[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd b4 f4 c0 1d fe ff[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd 72 7f[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd b2 00 08 00 00[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd 72 80[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd b2 f0 f7 ff ff[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd f4[ 	]*vpaddusw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af dd f4[ 	]*vpaddusw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd 31[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd b4 f4 c0 1d fe ff[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd 72 7f[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd b2 00 10 00 00[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd 72 80[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd b2 e0 ef ff ff[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd f4[ 	]*vpaddw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f fd f4[ 	]*vpaddw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd 31[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd b4 f4 c0 1d fe ff[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd 72 7f[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd b2 00 08 00 00[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd 72 80[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd b2 f0 f7 ff ff[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd f4[ 	]*vpaddw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af fd f4[ 	]*vpaddw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd 31[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd b4 f4 c0 1d fe ff[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd 72 7f[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd b2 00 10 00 00[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd 72 80[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd b2 e0 ef ff ff[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f f4 ab[ 	]*vpalignr xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 0f f4 ab[ 	]*vpalignr xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f f4 7b[ 	]*vpalignr xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f 31 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f b4 f4 c0 1d fe ff 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f 72 7f 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f b2 00 08 00 00 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f 72 80 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f b2 f0 f7 ff ff 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f f4 ab[ 	]*vpalignr ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 0f f4 ab[ 	]*vpalignr ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f f4 7b[ 	]*vpalignr ymm6\{k7\},ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f 31 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f b4 f4 c0 1d fe ff 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f 72 7f 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f b2 00 10 00 00 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f 72 80 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f b2 e0 ef ff ff 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 f4[ 	]*vpavgb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e0 f4[ 	]*vpavgb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 31[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 b4 f4 c0 1d fe ff[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 72 7f[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 b2 00 08 00 00[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 72 80[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 b2 f0 f7 ff ff[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 f4[ 	]*vpavgb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e0 f4[ 	]*vpavgb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 31[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 b4 f4 c0 1d fe ff[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 72 7f[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 b2 00 10 00 00[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 72 80[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 b2 e0 ef ff ff[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 f4[ 	]*vpavgw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e3 f4[ 	]*vpavgw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 31[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 b4 f4 c0 1d fe ff[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 72 7f[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 b2 00 08 00 00[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 72 80[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 b2 f0 f7 ff ff[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 f4[ 	]*vpavgw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e3 f4[ 	]*vpavgw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 31[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 b4 f4 c0 1d fe ff[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 72 7f[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 b2 00 10 00 00[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 72 80[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 b2 e0 ef ff ff[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 f4[ 	]*vpblendmb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 66 f4[ 	]*vpblendmb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 31[ 	]*vpblendmb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 72 7f[ 	]*vpblendmb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 b2 00 08 00 00[ 	]*vpblendmb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 72 80[ 	]*vpblendmb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 b2 f0 f7 ff ff[ 	]*vpblendmb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 f4[ 	]*vpblendmb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 66 f4[ 	]*vpblendmb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 31[ 	]*vpblendmb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 72 7f[ 	]*vpblendmb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 b2 00 10 00 00[ 	]*vpblendmb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 72 80[ 	]*vpblendmb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 b2 e0 ef ff ff[ 	]*vpblendmb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 f5[ 	]*vpbroadcastb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 78 f5[ 	]*vpbroadcastb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 31[ 	]*vpbroadcastb xmm6\{k7\},BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 b4 f4 c0 1d fe ff[ 	]*vpbroadcastb xmm6\{k7\},BYTE PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 72 7f[ 	]*vpbroadcastb xmm6\{k7\},BYTE PTR \[edx\+0x7f\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 b2 80 00 00 00[ 	]*vpbroadcastb xmm6\{k7\},BYTE PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 72 80[ 	]*vpbroadcastb xmm6\{k7\},BYTE PTR \[edx-0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 b2 7f ff ff ff[ 	]*vpbroadcastb xmm6\{k7\},BYTE PTR \[edx-0x81\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 f5[ 	]*vpbroadcastb ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 78 f5[ 	]*vpbroadcastb ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 31[ 	]*vpbroadcastb ymm6\{k7\},BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 b4 f4 c0 1d fe ff[ 	]*vpbroadcastb ymm6\{k7\},BYTE PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 72 7f[ 	]*vpbroadcastb ymm6\{k7\},BYTE PTR \[edx\+0x7f\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 b2 80 00 00 00[ 	]*vpbroadcastb ymm6\{k7\},BYTE PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 72 80[ 	]*vpbroadcastb ymm6\{k7\},BYTE PTR \[edx-0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 b2 7f ff ff ff[ 	]*vpbroadcastb ymm6\{k7\},BYTE PTR \[edx-0x81\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7a f0[ 	]*vpbroadcastb xmm6\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 7a f0[ 	]*vpbroadcastb xmm6\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7a f5[ 	]*vpbroadcastb xmm6\{k7\},ebp
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7a f0[ 	]*vpbroadcastb ymm6\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 7a f0[ 	]*vpbroadcastb ymm6\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7a f5[ 	]*vpbroadcastb ymm6\{k7\},ebp
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 f5[ 	]*vpbroadcastw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 79 f5[ 	]*vpbroadcastw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 31[ 	]*vpbroadcastw xmm6\{k7\},WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 b4 f4 c0 1d fe ff[ 	]*vpbroadcastw xmm6\{k7\},WORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 72 7f[ 	]*vpbroadcastw xmm6\{k7\},WORD PTR \[edx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 b2 00 01 00 00[ 	]*vpbroadcastw xmm6\{k7\},WORD PTR \[edx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 72 80[ 	]*vpbroadcastw xmm6\{k7\},WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 b2 fe fe ff ff[ 	]*vpbroadcastw xmm6\{k7\},WORD PTR \[edx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 f5[ 	]*vpbroadcastw ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 79 f5[ 	]*vpbroadcastw ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 31[ 	]*vpbroadcastw ymm6\{k7\},WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 b4 f4 c0 1d fe ff[ 	]*vpbroadcastw ymm6\{k7\},WORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 72 7f[ 	]*vpbroadcastw ymm6\{k7\},WORD PTR \[edx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 b2 00 01 00 00[ 	]*vpbroadcastw ymm6\{k7\},WORD PTR \[edx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 72 80[ 	]*vpbroadcastw ymm6\{k7\},WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 b2 fe fe ff ff[ 	]*vpbroadcastw ymm6\{k7\},WORD PTR \[edx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7b f0[ 	]*vpbroadcastw xmm6\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 7b f0[ 	]*vpbroadcastw xmm6\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7b f5[ 	]*vpbroadcastw xmm6\{k7\},ebp
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7b f0[ 	]*vpbroadcastw ymm6\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 7b f0[ 	]*vpbroadcastw ymm6\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7b f5[ 	]*vpbroadcastw ymm6\{k7\},ebp
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 ed[ 	]*vpcmpeqb k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 29[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 ac f4 c0 1d fe ff[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 6a 7f[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 aa 00 08 00 00[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 6a 80[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 aa f0 f7 ff ff[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 ed[ 	]*vpcmpeqb k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 29[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 ac f4 c0 1d fe ff[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 6a 7f[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 aa 00 10 00 00[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 6a 80[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 aa e0 ef ff ff[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 ed[ 	]*vpcmpeqw k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 29[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 ac f4 c0 1d fe ff[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 6a 7f[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 aa 00 08 00 00[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 6a 80[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 aa f0 f7 ff ff[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 ed[ 	]*vpcmpeqw k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 29[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 ac f4 c0 1d fe ff[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 6a 7f[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 aa 00 10 00 00[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 6a 80[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 aa e0 ef ff ff[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 ed[ 	]*vpcmpgtb k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 29[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 ac f4 c0 1d fe ff[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 6a 7f[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 aa 00 08 00 00[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 6a 80[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 aa f0 f7 ff ff[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 ed[ 	]*vpcmpgtb k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 29[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 ac f4 c0 1d fe ff[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 6a 7f[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 aa 00 10 00 00[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 6a 80[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 aa e0 ef ff ff[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 ed[ 	]*vpcmpgtw k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 29[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 ac f4 c0 1d fe ff[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 6a 7f[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 aa 00 08 00 00[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 6a 80[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 aa f0 f7 ff ff[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 ed[ 	]*vpcmpgtw k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 29[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 ac f4 c0 1d fe ff[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 6a 7f[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 aa 00 10 00 00[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 6a 80[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 aa e0 ef ff ff[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 f4[ 	]*vpblendmw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 66 f4[ 	]*vpblendmw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 31[ 	]*vpblendmw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 72 7f[ 	]*vpblendmw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 b2 00 08 00 00[ 	]*vpblendmw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 72 80[ 	]*vpblendmw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 b2 f0 f7 ff ff[ 	]*vpblendmw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 f4[ 	]*vpblendmw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 66 f4[ 	]*vpblendmw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 31[ 	]*vpblendmw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 72 7f[ 	]*vpblendmw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 b2 00 10 00 00[ 	]*vpblendmw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 72 80[ 	]*vpblendmw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 b2 e0 ef ff ff[ 	]*vpblendmw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 f4[ 	]*vpmaddubsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 04 f4[ 	]*vpmaddubsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 31[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 b4 f4 c0 1d fe ff[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 72 7f[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 b2 00 08 00 00[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 72 80[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 b2 f0 f7 ff ff[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 f4[ 	]*vpmaddubsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 04 f4[ 	]*vpmaddubsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 31[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 b4 f4 c0 1d fe ff[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 72 7f[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 b2 00 10 00 00[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 72 80[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 b2 e0 ef ff ff[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 f4[ 	]*vpmaddwd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f5 f4[ 	]*vpmaddwd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 31[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 b4 f4 c0 1d fe ff[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 72 7f[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 b2 00 08 00 00[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 72 80[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 b2 f0 f7 ff ff[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 f4[ 	]*vpmaddwd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f5 f4[ 	]*vpmaddwd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 31[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 b4 f4 c0 1d fe ff[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 72 7f[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 b2 00 10 00 00[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 72 80[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 b2 e0 ef ff ff[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c f4[ 	]*vpmaxsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 3c f4[ 	]*vpmaxsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c 31[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c b4 f4 c0 1d fe ff[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c 72 7f[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c b2 00 08 00 00[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c 72 80[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c b2 f0 f7 ff ff[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c f4[ 	]*vpmaxsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 3c f4[ 	]*vpmaxsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c 31[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c b4 f4 c0 1d fe ff[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c 72 7f[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c b2 00 10 00 00[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c 72 80[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c b2 e0 ef ff ff[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee f4[ 	]*vpmaxsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ee f4[ 	]*vpmaxsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee 31[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee b4 f4 c0 1d fe ff[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee 72 7f[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee b2 00 08 00 00[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee 72 80[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee b2 f0 f7 ff ff[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee f4[ 	]*vpmaxsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ee f4[ 	]*vpmaxsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee 31[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee b4 f4 c0 1d fe ff[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee 72 7f[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee b2 00 10 00 00[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee 72 80[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee b2 e0 ef ff ff[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de f4[ 	]*vpmaxub xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f de f4[ 	]*vpmaxub xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de 31[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de b4 f4 c0 1d fe ff[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de 72 7f[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de b2 00 08 00 00[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de 72 80[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de b2 f0 f7 ff ff[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de f4[ 	]*vpmaxub ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af de f4[ 	]*vpmaxub ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de 31[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de b4 f4 c0 1d fe ff[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de 72 7f[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de b2 00 10 00 00[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de 72 80[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de b2 e0 ef ff ff[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e f4[ 	]*vpmaxuw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 3e f4[ 	]*vpmaxuw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e 31[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e b4 f4 c0 1d fe ff[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e 72 7f[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e b2 00 08 00 00[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e 72 80[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e b2 f0 f7 ff ff[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e f4[ 	]*vpmaxuw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 3e f4[ 	]*vpmaxuw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e 31[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e b4 f4 c0 1d fe ff[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e 72 7f[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e b2 00 10 00 00[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e 72 80[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e b2 e0 ef ff ff[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 f4[ 	]*vpminsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 38 f4[ 	]*vpminsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 31[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 b4 f4 c0 1d fe ff[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 72 7f[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 b2 00 08 00 00[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 72 80[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 b2 f0 f7 ff ff[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 f4[ 	]*vpminsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 38 f4[ 	]*vpminsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 31[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 b4 f4 c0 1d fe ff[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 72 7f[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 b2 00 10 00 00[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 72 80[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 b2 e0 ef ff ff[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea f4[ 	]*vpminsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ea f4[ 	]*vpminsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea 31[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea b4 f4 c0 1d fe ff[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea 72 7f[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea b2 00 08 00 00[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea 72 80[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea b2 f0 f7 ff ff[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea f4[ 	]*vpminsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ea f4[ 	]*vpminsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea 31[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea b4 f4 c0 1d fe ff[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea 72 7f[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea b2 00 10 00 00[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea 72 80[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea b2 e0 ef ff ff[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da f4[ 	]*vpminub xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f da f4[ 	]*vpminub xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da 31[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da b4 f4 c0 1d fe ff[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da 72 7f[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da b2 00 08 00 00[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da 72 80[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da b2 f0 f7 ff ff[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da f4[ 	]*vpminub ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af da f4[ 	]*vpminub ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da 31[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da b4 f4 c0 1d fe ff[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da 72 7f[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da b2 00 10 00 00[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da 72 80[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da b2 e0 ef ff ff[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a f4[ 	]*vpminuw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 3a f4[ 	]*vpminuw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a 31[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a b4 f4 c0 1d fe ff[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a 72 7f[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a b2 00 08 00 00[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a 72 80[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a b2 f0 f7 ff ff[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a f4[ 	]*vpminuw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 3a f4[ 	]*vpminuw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a 31[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a b4 f4 c0 1d fe ff[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a 72 7f[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a b2 00 10 00 00[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a 72 80[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a b2 e0 ef ff ff[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 f5[ 	]*vpmovsxbw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 20 f5[ 	]*vpmovsxbw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 31[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 b4 f4 c0 1d fe ff[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 72 7f[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 b2 00 04 00 00[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 72 80[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 b2 f8 fb ff ff[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 f5[ 	]*vpmovsxbw ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 20 f5[ 	]*vpmovsxbw ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 31[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 b4 f4 c0 1d fe ff[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 72 7f[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 b2 00 08 00 00[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 72 80[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 b2 f0 f7 ff ff[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 f5[ 	]*vpmovzxbw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 30 f5[ 	]*vpmovzxbw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 31[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 b4 f4 c0 1d fe ff[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 72 7f[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 b2 00 04 00 00[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 72 80[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 b2 f8 fb ff ff[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 f5[ 	]*vpmovzxbw ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 30 f5[ 	]*vpmovzxbw ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 31[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 b4 f4 c0 1d fe ff[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 72 7f[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 b2 00 08 00 00[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 72 80[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 b2 f0 f7 ff ff[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b f4[ 	]*vpmulhrsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 0b f4[ 	]*vpmulhrsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b 31[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b b4 f4 c0 1d fe ff[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b 72 7f[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b b2 00 08 00 00[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b 72 80[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b b2 f0 f7 ff ff[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b f4[ 	]*vpmulhrsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 0b f4[ 	]*vpmulhrsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b 31[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b b4 f4 c0 1d fe ff[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b 72 7f[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b b2 00 10 00 00[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b 72 80[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b b2 e0 ef ff ff[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 f4[ 	]*vpmulhuw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e4 f4[ 	]*vpmulhuw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 31[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 b4 f4 c0 1d fe ff[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 72 7f[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 b2 00 08 00 00[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 72 80[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 b2 f0 f7 ff ff[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 f4[ 	]*vpmulhuw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e4 f4[ 	]*vpmulhuw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 31[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 b4 f4 c0 1d fe ff[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 72 7f[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 b2 00 10 00 00[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 72 80[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 b2 e0 ef ff ff[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 f4[ 	]*vpmulhw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e5 f4[ 	]*vpmulhw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 31[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 b4 f4 c0 1d fe ff[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 72 7f[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 b2 00 08 00 00[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 72 80[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 b2 f0 f7 ff ff[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 f4[ 	]*vpmulhw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e5 f4[ 	]*vpmulhw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 31[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 b4 f4 c0 1d fe ff[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 72 7f[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 b2 00 10 00 00[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 72 80[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 b2 e0 ef ff ff[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 f4[ 	]*vpmullw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d5 f4[ 	]*vpmullw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 31[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 b4 f4 c0 1d fe ff[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 72 7f[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 b2 00 08 00 00[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 72 80[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 b2 f0 f7 ff ff[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 f4[ 	]*vpmullw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d5 f4[ 	]*vpmullw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 31[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 b4 f4 c0 1d fe ff[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 72 7f[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 b2 00 10 00 00[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 72 80[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 b2 e0 ef ff ff[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 f4[ 	]*vpshufb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 00 f4[ 	]*vpshufb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 31[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 b4 f4 c0 1d fe ff[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 72 7f[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 b2 00 08 00 00[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 72 80[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 b2 f0 f7 ff ff[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 f4[ 	]*vpshufb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 00 f4[ 	]*vpshufb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 31[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 b4 f4 c0 1d fe ff[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 72 7f[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 b2 00 10 00 00[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 72 80[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 b2 e0 ef ff ff[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 f5 ab[ 	]*vpshufhw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 8f 70 f5 ab[ 	]*vpshufhw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 f5 7b[ 	]*vpshufhw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 31 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 72 7f 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 b2 00 08 00 00 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 72 80 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 b2 f0 f7 ff ff 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 f5 ab[ 	]*vpshufhw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e af 70 f5 ab[ 	]*vpshufhw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 f5 7b[ 	]*vpshufhw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 31 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 72 7f 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 b2 00 10 00 00 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 72 80 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 b2 e0 ef ff ff 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 f5 ab[ 	]*vpshuflw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 70 f5 ab[ 	]*vpshuflw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 f5 7b[ 	]*vpshuflw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 31 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 72 7f 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 b2 00 08 00 00 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 72 80 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 b2 f0 f7 ff ff 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 f5 ab[ 	]*vpshuflw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 70 f5 ab[ 	]*vpshuflw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 f5 7b[ 	]*vpshuflw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 31 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 72 7f 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 b2 00 10 00 00 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 72 80 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 b2 e0 ef ff ff 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 f4[ 	]*vpsllw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f1 f4[ 	]*vpsllw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 31[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 b4 f4 c0 1d fe ff[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 72 7f[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 b2 00 08 00 00[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 72 80[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 b2 f0 f7 ff ff[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 f4[ 	]*vpsllw ymm6\{k7\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f1 f4[ 	]*vpsllw ymm6\{k7\}\{z\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 31[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 b4 f4 c0 1d fe ff[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 72 7f[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 b2 00 08 00 00[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 72 80[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 b2 f0 f7 ff ff[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 f4[ 	]*vpsraw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e1 f4[ 	]*vpsraw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 31[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 b4 f4 c0 1d fe ff[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 72 7f[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 b2 00 08 00 00[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 72 80[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 b2 f0 f7 ff ff[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 f4[ 	]*vpsraw ymm6\{k7\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e1 f4[ 	]*vpsraw ymm6\{k7\}\{z\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 31[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 b4 f4 c0 1d fe ff[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 72 7f[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 b2 00 08 00 00[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 72 80[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 b2 f0 f7 ff ff[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 f4[ 	]*vpsrlw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d1 f4[ 	]*vpsrlw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 31[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 b4 f4 c0 1d fe ff[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 72 7f[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 b2 00 08 00 00[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 72 80[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 b2 f0 f7 ff ff[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 f4[ 	]*vpsrlw ymm6\{k7\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d1 f4[ 	]*vpsrlw ymm6\{k7\}\{z\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 31[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 b4 f4 c0 1d fe ff[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 72 7f[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 b2 00 08 00 00[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 72 80[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 b2 f0 f7 ff ff[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 d5 ab[ 	]*vpsrlw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 8f 71 d5 ab[ 	]*vpsrlw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 d5 7b[ 	]*vpsrlw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 11 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 94 f4 c0 1d fe ff 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 52 7f 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 92 00 08 00 00 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 52 80 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 92 f0 f7 ff ff 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 d5 ab[ 	]*vpsrlw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d af 71 d5 ab[ 	]*vpsrlw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 d5 7b[ 	]*vpsrlw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 11 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 94 f4 c0 1d fe ff 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 52 7f 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 92 00 10 00 00 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 52 80 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 92 e0 ef ff ff 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 e5 ab[ 	]*vpsraw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 8f 71 e5 ab[ 	]*vpsraw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 e5 7b[ 	]*vpsraw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 21 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 a4 f4 c0 1d fe ff 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 62 7f 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 a2 00 08 00 00 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 62 80 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 a2 f0 f7 ff ff 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 e5 ab[ 	]*vpsraw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d af 71 e5 ab[ 	]*vpsraw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 e5 7b[ 	]*vpsraw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 21 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 a4 f4 c0 1d fe ff 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 62 7f 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 a2 00 10 00 00 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 62 80 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 a2 e0 ef ff ff 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 f4[ 	]*vpsrlvw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 10 f4[ 	]*vpsrlvw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 31[ 	]*vpsrlvw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 b4 f4 c0 1d fe ff[ 	]*vpsrlvw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 72 7f[ 	]*vpsrlvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 b2 00 08 00 00[ 	]*vpsrlvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 72 80[ 	]*vpsrlvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 b2 f0 f7 ff ff[ 	]*vpsrlvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 f4[ 	]*vpsrlvw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 10 f4[ 	]*vpsrlvw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 31[ 	]*vpsrlvw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 b4 f4 c0 1d fe ff[ 	]*vpsrlvw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 72 7f[ 	]*vpsrlvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 b2 00 10 00 00[ 	]*vpsrlvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 72 80[ 	]*vpsrlvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 b2 e0 ef ff ff[ 	]*vpsrlvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 f4[ 	]*vpsravw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 11 f4[ 	]*vpsravw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 31[ 	]*vpsravw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 b4 f4 c0 1d fe ff[ 	]*vpsravw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 72 7f[ 	]*vpsravw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 b2 00 08 00 00[ 	]*vpsravw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 72 80[ 	]*vpsravw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 b2 f0 f7 ff ff[ 	]*vpsravw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 f4[ 	]*vpsravw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 11 f4[ 	]*vpsravw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 31[ 	]*vpsravw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 b4 f4 c0 1d fe ff[ 	]*vpsravw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 72 7f[ 	]*vpsravw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 b2 00 10 00 00[ 	]*vpsravw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 72 80[ 	]*vpsravw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 b2 e0 ef ff ff[ 	]*vpsravw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 f4[ 	]*vpsubb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f8 f4[ 	]*vpsubb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 31[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 b4 f4 c0 1d fe ff[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 72 7f[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 b2 00 08 00 00[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 72 80[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 b2 f0 f7 ff ff[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 f4[ 	]*vpsubb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f8 f4[ 	]*vpsubb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 31[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 b4 f4 c0 1d fe ff[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 72 7f[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 b2 00 10 00 00[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 72 80[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 b2 e0 ef ff ff[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 f4[ 	]*vpsubsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e8 f4[ 	]*vpsubsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 31[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 b4 f4 c0 1d fe ff[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 72 7f[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 b2 00 08 00 00[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 72 80[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 b2 f0 f7 ff ff[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 f4[ 	]*vpsubsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e8 f4[ 	]*vpsubsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 31[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 b4 f4 c0 1d fe ff[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 72 7f[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 b2 00 10 00 00[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 72 80[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 b2 e0 ef ff ff[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 f4[ 	]*vpsubsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e9 f4[ 	]*vpsubsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 31[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 b4 f4 c0 1d fe ff[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 72 7f[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 b2 00 08 00 00[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 72 80[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 b2 f0 f7 ff ff[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 f4[ 	]*vpsubsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e9 f4[ 	]*vpsubsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 31[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 b4 f4 c0 1d fe ff[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 72 7f[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 b2 00 10 00 00[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 72 80[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 b2 e0 ef ff ff[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 f4[ 	]*vpsubusb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d8 f4[ 	]*vpsubusb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 31[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 b4 f4 c0 1d fe ff[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 72 7f[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 b2 00 08 00 00[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 72 80[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 b2 f0 f7 ff ff[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 f4[ 	]*vpsubusb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d8 f4[ 	]*vpsubusb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 31[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 b4 f4 c0 1d fe ff[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 72 7f[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 b2 00 10 00 00[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 72 80[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 b2 e0 ef ff ff[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 f4[ 	]*vpsubusw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d9 f4[ 	]*vpsubusw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 31[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 b4 f4 c0 1d fe ff[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 72 7f[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 b2 00 08 00 00[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 72 80[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 b2 f0 f7 ff ff[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 f4[ 	]*vpsubusw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d9 f4[ 	]*vpsubusw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 31[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 b4 f4 c0 1d fe ff[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 72 7f[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 b2 00 10 00 00[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 72 80[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 b2 e0 ef ff ff[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 f4[ 	]*vpsubw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f9 f4[ 	]*vpsubw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 31[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 b4 f4 c0 1d fe ff[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 72 7f[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 b2 00 08 00 00[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 72 80[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 b2 f0 f7 ff ff[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 f4[ 	]*vpsubw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f9 f4[ 	]*vpsubw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 31[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 b4 f4 c0 1d fe ff[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 72 7f[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 b2 00 10 00 00[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 72 80[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 b2 e0 ef ff ff[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 f4[ 	]*vpunpckhbw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 68 f4[ 	]*vpunpckhbw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 31[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 b4 f4 c0 1d fe ff[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 72 7f[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 b2 00 08 00 00[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 72 80[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 b2 f0 f7 ff ff[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 f4[ 	]*vpunpckhbw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 68 f4[ 	]*vpunpckhbw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 31[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 b4 f4 c0 1d fe ff[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 72 7f[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 b2 00 10 00 00[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 72 80[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 b2 e0 ef ff ff[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 f4[ 	]*vpunpckhwd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 69 f4[ 	]*vpunpckhwd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 31[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 b4 f4 c0 1d fe ff[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 72 7f[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 b2 00 08 00 00[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 72 80[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 b2 f0 f7 ff ff[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 f4[ 	]*vpunpckhwd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 69 f4[ 	]*vpunpckhwd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 31[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 b4 f4 c0 1d fe ff[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 72 7f[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 b2 00 10 00 00[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 72 80[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 b2 e0 ef ff ff[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 f4[ 	]*vpunpcklbw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 60 f4[ 	]*vpunpcklbw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 31[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 b4 f4 c0 1d fe ff[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 72 7f[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 b2 00 08 00 00[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 72 80[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 b2 f0 f7 ff ff[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 f4[ 	]*vpunpcklbw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 60 f4[ 	]*vpunpcklbw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 31[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 b4 f4 c0 1d fe ff[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 72 7f[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 b2 00 10 00 00[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 72 80[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 b2 e0 ef ff ff[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 f4[ 	]*vpunpcklwd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 61 f4[ 	]*vpunpcklwd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 31[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 b4 f4 c0 1d fe ff[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 72 7f[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 b2 00 08 00 00[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 72 80[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 b2 f0 f7 ff ff[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 f4[ 	]*vpunpcklwd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 61 f4[ 	]*vpunpcklwd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 31[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 b4 f4 c0 1d fe ff[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 72 7f[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 b2 00 10 00 00[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 72 80[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 b2 e0 ef ff ff[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 ee[ 	]*vpmovwb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 8f 30 ee[ 	]*vpmovwb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 ee[ 	]*vpmovwb xmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e af 30 ee[ 	]*vpmovwb xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 ee[ 	]*vpmovswb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 8f 20 ee[ 	]*vpmovswb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 ee[ 	]*vpmovswb xmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e af 20 ee[ 	]*vpmovswb xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 ee[ 	]*vpmovuswb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 8f 10 ee[ 	]*vpmovuswb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 ee[ 	]*vpmovuswb xmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e af 10 ee[ 	]*vpmovuswb xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 f4 ab[ 	]*vdbpsadbw xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 42 f4 ab[ 	]*vdbpsadbw xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 f4 7b[ 	]*vdbpsadbw xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 31 7b[ 	]*vdbpsadbw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 b4 f4 c0 1d fe ff 7b[ 	]*vdbpsadbw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 72 7f 7b[ 	]*vdbpsadbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 b2 00 08 00 00 7b[ 	]*vdbpsadbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 72 80 7b[ 	]*vdbpsadbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 b2 f0 f7 ff ff 7b[ 	]*vdbpsadbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 f4 ab[ 	]*vdbpsadbw ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 42 f4 ab[ 	]*vdbpsadbw ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 f4 7b[ 	]*vdbpsadbw ymm6\{k7\},ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 31 7b[ 	]*vdbpsadbw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 b4 f4 c0 1d fe ff 7b[ 	]*vdbpsadbw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 72 7f 7b[ 	]*vdbpsadbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 b2 00 10 00 00 7b[ 	]*vdbpsadbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 72 80 7b[ 	]*vdbpsadbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 b2 e0 ef ff ff 7b[ 	]*vdbpsadbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d f4[ 	]*vpermw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 8d f4[ 	]*vpermw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d 31[ 	]*vpermw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d b4 f4 c0 1d fe ff[ 	]*vpermw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d 72 7f[ 	]*vpermw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d b2 00 08 00 00[ 	]*vpermw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d 72 80[ 	]*vpermw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d b2 f0 f7 ff ff[ 	]*vpermw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d f4[ 	]*vpermw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 8d f4[ 	]*vpermw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d 31[ 	]*vpermw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d b4 f4 c0 1d fe ff[ 	]*vpermw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d 72 7f[ 	]*vpermw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d b2 00 10 00 00[ 	]*vpermw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d 72 80[ 	]*vpermw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d b2 e0 ef ff ff[ 	]*vpermw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d f4[ 	]*vpermt2w xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 7d f4[ 	]*vpermt2w xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d 31[ 	]*vpermt2w xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2w xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d 72 7f[ 	]*vpermt2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d b2 00 08 00 00[ 	]*vpermt2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d 72 80[ 	]*vpermt2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d b2 f0 f7 ff ff[ 	]*vpermt2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d f4[ 	]*vpermt2w ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 7d f4[ 	]*vpermt2w ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d 31[ 	]*vpermt2w ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2w ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d 72 7f[ 	]*vpermt2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d b2 00 10 00 00[ 	]*vpermt2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d 72 80[ 	]*vpermt2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d b2 e0 ef ff ff[ 	]*vpermt2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 f5 ab[ 	]*vpsllw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 8f 71 f5 ab[ 	]*vpsllw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 f5 7b[ 	]*vpsllw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 31 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 72 7f 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 b2 00 08 00 00 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 72 80 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 b2 f0 f7 ff ff 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 f5 ab[ 	]*vpsllw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d af 71 f5 ab[ 	]*vpsllw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 f5 7b[ 	]*vpsllw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 31 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 72 7f 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 b2 00 10 00 00 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 72 80 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 b2 e0 ef ff ff 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 f4[ 	]*vpsllvw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 12 f4[ 	]*vpsllvw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 31[ 	]*vpsllvw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 b4 f4 c0 1d fe ff[ 	]*vpsllvw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 72 7f[ 	]*vpsllvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 b2 00 08 00 00[ 	]*vpsllvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 72 80[ 	]*vpsllvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 b2 f0 f7 ff ff[ 	]*vpsllvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 f4[ 	]*vpsllvw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 12 f4[ 	]*vpsllvw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 31[ 	]*vpsllvw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 b4 f4 c0 1d fe ff[ 	]*vpsllvw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 72 7f[ 	]*vpsllvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 b2 00 10 00 00[ 	]*vpsllvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 72 80[ 	]*vpsllvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 b2 e0 ef ff ff[ 	]*vpsllvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f f5[ 	]*vmovdqu8 xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 6f f5[ 	]*vmovdqu8 xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f 31[ 	]*vmovdqu8 xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f 72 7f[ 	]*vmovdqu8 xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f b2 00 08 00 00[ 	]*vmovdqu8 xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f 72 80[ 	]*vmovdqu8 xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f b2 f0 f7 ff ff[ 	]*vmovdqu8 xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f f5[ 	]*vmovdqu8 ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 6f f5[ 	]*vmovdqu8 ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f 31[ 	]*vmovdqu8 ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f 72 7f[ 	]*vmovdqu8 ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f b2 00 10 00 00[ 	]*vmovdqu8 ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f 72 80[ 	]*vmovdqu8 ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f b2 e0 ef ff ff[ 	]*vmovdqu8 ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f f5[ 	]*vmovdqu16 xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 6f f5[ 	]*vmovdqu16 xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f 31[ 	]*vmovdqu16 xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f 72 7f[ 	]*vmovdqu16 xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f b2 00 08 00 00[ 	]*vmovdqu16 xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f 72 80[ 	]*vmovdqu16 xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f b2 f0 f7 ff ff[ 	]*vmovdqu16 xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f f5[ 	]*vmovdqu16 ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 6f f5[ 	]*vmovdqu16 ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f 31[ 	]*vmovdqu16 ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f 72 7f[ 	]*vmovdqu16 ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f b2 00 10 00 00[ 	]*vmovdqu16 ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f 72 80[ 	]*vmovdqu16 ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f b2 e0 ef ff ff[ 	]*vmovdqu16 ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 31[ 	]*vpmovwb QWORD PTR \[ecx\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 b4 f4 c0 1d fe ff[ 	]*vpmovwb QWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 72 7f[ 	]*vpmovwb QWORD PTR \[edx\+0x3f8\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 b2 00 04 00 00[ 	]*vpmovwb QWORD PTR \[edx\+0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 72 80[ 	]*vpmovwb QWORD PTR \[edx-0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 b2 f8 fb ff ff[ 	]*vpmovwb QWORD PTR \[edx-0x408\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 31[ 	]*vpmovwb XMMWORD PTR \[ecx\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 b4 f4 c0 1d fe ff[ 	]*vpmovwb XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 72 7f[ 	]*vpmovwb XMMWORD PTR \[edx\+0x7f0\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 b2 00 08 00 00[ 	]*vpmovwb XMMWORD PTR \[edx\+0x800\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 72 80[ 	]*vpmovwb XMMWORD PTR \[edx-0x800\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 b2 f0 f7 ff ff[ 	]*vpmovwb XMMWORD PTR \[edx-0x810\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 31[ 	]*vpmovswb QWORD PTR \[ecx\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 b4 f4 c0 1d fe ff[ 	]*vpmovswb QWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 72 7f[ 	]*vpmovswb QWORD PTR \[edx\+0x3f8\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 b2 00 04 00 00[ 	]*vpmovswb QWORD PTR \[edx\+0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 72 80[ 	]*vpmovswb QWORD PTR \[edx-0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 b2 f8 fb ff ff[ 	]*vpmovswb QWORD PTR \[edx-0x408\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 31[ 	]*vpmovswb XMMWORD PTR \[ecx\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 b4 f4 c0 1d fe ff[ 	]*vpmovswb XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 72 7f[ 	]*vpmovswb XMMWORD PTR \[edx\+0x7f0\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 b2 00 08 00 00[ 	]*vpmovswb XMMWORD PTR \[edx\+0x800\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 72 80[ 	]*vpmovswb XMMWORD PTR \[edx-0x800\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 b2 f0 f7 ff ff[ 	]*vpmovswb XMMWORD PTR \[edx-0x810\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 31[ 	]*vpmovuswb QWORD PTR \[ecx\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 b4 f4 c0 1d fe ff[ 	]*vpmovuswb QWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 72 7f[ 	]*vpmovuswb QWORD PTR \[edx\+0x3f8\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 b2 00 04 00 00[ 	]*vpmovuswb QWORD PTR \[edx\+0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 72 80[ 	]*vpmovuswb QWORD PTR \[edx-0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 b2 f8 fb ff ff[ 	]*vpmovuswb QWORD PTR \[edx-0x408\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 31[ 	]*vpmovuswb XMMWORD PTR \[ecx\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 b4 f4 c0 1d fe ff[ 	]*vpmovuswb XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 72 7f[ 	]*vpmovuswb XMMWORD PTR \[edx\+0x7f0\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 b2 00 08 00 00[ 	]*vpmovuswb XMMWORD PTR \[edx\+0x800\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 72 80[ 	]*vpmovuswb XMMWORD PTR \[edx-0x800\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 b2 f0 f7 ff ff[ 	]*vpmovuswb XMMWORD PTR \[edx-0x810\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f 31[ 	]*vmovdqu8 XMMWORD PTR \[ecx\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f 72 7f[ 	]*vmovdqu8 XMMWORD PTR \[edx\+0x7f0\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f b2 00 08 00 00[ 	]*vmovdqu8 XMMWORD PTR \[edx\+0x800\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f 72 80[ 	]*vmovdqu8 XMMWORD PTR \[edx-0x800\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f b2 f0 f7 ff ff[ 	]*vmovdqu8 XMMWORD PTR \[edx-0x810\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f 31[ 	]*vmovdqu8 YMMWORD PTR \[ecx\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 YMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f 72 7f[ 	]*vmovdqu8 YMMWORD PTR \[edx\+0xfe0\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f b2 00 10 00 00[ 	]*vmovdqu8 YMMWORD PTR \[edx\+0x1000\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f 72 80[ 	]*vmovdqu8 YMMWORD PTR \[edx-0x1000\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f b2 e0 ef ff ff[ 	]*vmovdqu8 YMMWORD PTR \[edx-0x1020\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f 31[ 	]*vmovdqu16 XMMWORD PTR \[ecx\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f 72 7f[ 	]*vmovdqu16 XMMWORD PTR \[edx\+0x7f0\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f b2 00 08 00 00[ 	]*vmovdqu16 XMMWORD PTR \[edx\+0x800\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f 72 80[ 	]*vmovdqu16 XMMWORD PTR \[edx-0x800\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f b2 f0 f7 ff ff[ 	]*vmovdqu16 XMMWORD PTR \[edx-0x810\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f 31[ 	]*vmovdqu16 YMMWORD PTR \[ecx\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 YMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f 72 7f[ 	]*vmovdqu16 YMMWORD PTR \[edx\+0xfe0\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f b2 00 10 00 00[ 	]*vmovdqu16 YMMWORD PTR \[edx\+0x1000\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f 72 80[ 	]*vmovdqu16 YMMWORD PTR \[edx-0x1000\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f b2 e0 ef ff ff[ 	]*vmovdqu16 YMMWORD PTR \[edx-0x1020\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 f4[ 	]*vpermi2w xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 75 f4[ 	]*vpermi2w xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 31[ 	]*vpermi2w xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2w xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 72 7f[ 	]*vpermi2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 b2 00 08 00 00[ 	]*vpermi2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 72 80[ 	]*vpermi2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 b2 f0 f7 ff ff[ 	]*vpermi2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 f4[ 	]*vpermi2w ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 75 f4[ 	]*vpermi2w ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 31[ 	]*vpermi2w ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2w ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 72 7f[ 	]*vpermi2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 b2 00 10 00 00[ 	]*vpermi2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 72 80[ 	]*vpermi2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 b2 e0 ef ff ff[ 	]*vpermi2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 ed[ 	]*vptestmb k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 29[ 	]*vptestmb k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 ac f4 c0 1d fe ff[ 	]*vptestmb k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 6a 7f[ 	]*vptestmb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 aa 00 08 00 00[ 	]*vptestmb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 6a 80[ 	]*vptestmb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 aa f0 f7 ff ff[ 	]*vptestmb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 ed[ 	]*vptestmb k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 29[ 	]*vptestmb k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 ac f4 c0 1d fe ff[ 	]*vptestmb k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 6a 7f[ 	]*vptestmb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 aa 00 10 00 00[ 	]*vptestmb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 6a 80[ 	]*vptestmb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 aa e0 ef ff ff[ 	]*vptestmb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 ed[ 	]*vptestmw k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 29[ 	]*vptestmw k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 ac f4 c0 1d fe ff[ 	]*vptestmw k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 6a 7f[ 	]*vptestmw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 aa 00 08 00 00[ 	]*vptestmw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 6a 80[ 	]*vptestmw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 aa f0 f7 ff ff[ 	]*vptestmw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 ed[ 	]*vptestmw k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 29[ 	]*vptestmw k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 ac f4 c0 1d fe ff[ 	]*vptestmw k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 6a 7f[ 	]*vptestmw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 aa 00 10 00 00[ 	]*vptestmw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 6a 80[ 	]*vptestmw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 aa e0 ef ff ff[ 	]*vptestmw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 29 ee[ 	]*vpmovb2m k5,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 29 ee[ 	]*vpmovb2m k5,ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 29 ee[ 	]*vpmovw2m k5,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 29 ee[ 	]*vpmovw2m k5,ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 28 f5[ 	]*vpmovm2b xmm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 28 f5[ 	]*vpmovm2b ymm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 28 f5[ 	]*vpmovm2w xmm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 28 f5[ 	]*vpmovm2w ymm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 ec[ 	]*vptestnmb k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 29[ 	]*vptestnmb k5\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 ac f4 c0 1d fe ff[ 	]*vptestnmb k5\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 6a 7f[ 	]*vptestnmb k5\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 aa 00 08 00 00[ 	]*vptestnmb k5\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 6a 80[ 	]*vptestnmb k5\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 aa f0 f7 ff ff[ 	]*vptestnmb k5\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 ec[ 	]*vptestnmb k5\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 29[ 	]*vptestnmb k5\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 ac f4 c0 1d fe ff[ 	]*vptestnmb k5\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 6a 7f[ 	]*vptestnmb k5\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 aa 00 10 00 00[ 	]*vptestnmb k5\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 6a 80[ 	]*vptestnmb k5\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 aa e0 ef ff ff[ 	]*vptestnmb k5\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 ec[ 	]*vptestnmw k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 29[ 	]*vptestnmw k5\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 ac f4 c0 1d fe ff[ 	]*vptestnmw k5\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 6a 7f[ 	]*vptestnmw k5\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 aa 00 08 00 00[ 	]*vptestnmw k5\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 6a 80[ 	]*vptestnmw k5\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 aa f0 f7 ff ff[ 	]*vptestnmw k5\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 ec[ 	]*vptestnmw k5\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 29[ 	]*vptestnmw k5\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 ac f4 c0 1d fe ff[ 	]*vptestnmw k5\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 6a 7f[ 	]*vptestnmw k5\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 aa 00 10 00 00[ 	]*vptestnmw k5\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 6a 80[ 	]*vptestnmw k5\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 aa e0 ef ff ff[ 	]*vptestnmw k5\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f ed ab[ 	]*vpcmpb k5\{k7\},xmm6,xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f ed 7b[ 	]*vpcmpb k5\{k7\},xmm6,xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f 29 7b[ 	]*vpcmpb k5\{k7\},xmm6,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpb k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f 6a 7f 7b[ 	]*vpcmpb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f aa 00 08 00 00 7b[ 	]*vpcmpb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f 6a 80 7b[ 	]*vpcmpb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f aa f0 f7 ff ff 7b[ 	]*vpcmpb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f ed ab[ 	]*vpcmpb k5\{k7\},ymm6,ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f ed 7b[ 	]*vpcmpb k5\{k7\},ymm6,ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f 29 7b[ 	]*vpcmpb k5\{k7\},ymm6,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpb k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f 6a 7f 7b[ 	]*vpcmpb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f aa 00 10 00 00 7b[ 	]*vpcmpb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f 6a 80 7b[ 	]*vpcmpb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f aa e0 ef ff ff 7b[ 	]*vpcmpb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f ed 00[ 	]*vpcmpeqb k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f ed 00[ 	]*vpcmpeqb k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f ed 02[ 	]*vpcmpleb k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f 68 7f 02[ 	]*vpcmpleb k5,xmm6,XMMWORD PTR \[eax\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f a8 00 08 00 00 02[ 	]*vpcmpleb k5,xmm6,XMMWORD PTR \[eax\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f ed 02[ 	]*vpcmpleb k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f 68 7f 02[ 	]*vpcmpleb k5,ymm6,YMMWORD PTR \[eax\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f a8 00 10 00 00 02[ 	]*vpcmpleb k5,ymm6,YMMWORD PTR \[eax\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f ed 01[ 	]*vpcmpltb k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f ed 01[ 	]*vpcmpltb k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f ed 04[ 	]*vpcmpneqb k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f ed 04[ 	]*vpcmpneqb k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f ed 06[ 	]*vpcmpnleb k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f ed 06[ 	]*vpcmpnleb k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3f ed 05[ 	]*vpcmpnltb k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3f ed 05[ 	]*vpcmpnltb k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f ed ab[ 	]*vpcmpw k5\{k7\},xmm6,xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f ed 7b[ 	]*vpcmpw k5\{k7\},xmm6,xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f 29 7b[ 	]*vpcmpw k5\{k7\},xmm6,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpw k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f 6a 7f 7b[ 	]*vpcmpw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f aa 00 08 00 00 7b[ 	]*vpcmpw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f 6a 80 7b[ 	]*vpcmpw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f aa f0 f7 ff ff 7b[ 	]*vpcmpw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f ed ab[ 	]*vpcmpw k5\{k7\},ymm6,ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f ed 7b[ 	]*vpcmpw k5\{k7\},ymm6,ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f 29 7b[ 	]*vpcmpw k5\{k7\},ymm6,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpw k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f 6a 7f 7b[ 	]*vpcmpw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f aa 00 10 00 00 7b[ 	]*vpcmpw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f 6a 80 7b[ 	]*vpcmpw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f aa e0 ef ff ff 7b[ 	]*vpcmpw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f ed 00[ 	]*vpcmpeqw k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f ed 00[ 	]*vpcmpeqw k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f ed 02[ 	]*vpcmplew k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f 68 7f 02[ 	]*vpcmplew k5,xmm6,XMMWORD PTR \[eax\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f a8 00 08 00 00 02[ 	]*vpcmplew k5,xmm6,XMMWORD PTR \[eax\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f ed 02[ 	]*vpcmplew k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f 68 7f 02[ 	]*vpcmplew k5,ymm6,YMMWORD PTR \[eax\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f a8 00 10 00 00 02[ 	]*vpcmplew k5,ymm6,YMMWORD PTR \[eax\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f ed 01[ 	]*vpcmpltw k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f ed 01[ 	]*vpcmpltw k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f ed 04[ 	]*vpcmpneqw k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f ed 04[ 	]*vpcmpneqw k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f ed 06[ 	]*vpcmpnlew k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f ed 06[ 	]*vpcmpnlew k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3f ed 05[ 	]*vpcmpnltw k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3f ed 05[ 	]*vpcmpnltw k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e ed ab[ 	]*vpcmpub k5\{k7\},xmm6,xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e ed 7b[ 	]*vpcmpub k5\{k7\},xmm6,xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e 29 7b[ 	]*vpcmpub k5\{k7\},xmm6,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpub k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e 6a 7f 7b[ 	]*vpcmpub k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e aa 00 08 00 00 7b[ 	]*vpcmpub k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e 6a 80 7b[ 	]*vpcmpub k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e aa f0 f7 ff ff 7b[ 	]*vpcmpub k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e ed ab[ 	]*vpcmpub k5\{k7\},ymm6,ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e ed 7b[ 	]*vpcmpub k5\{k7\},ymm6,ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e 29 7b[ 	]*vpcmpub k5\{k7\},ymm6,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpub k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e 6a 7f 7b[ 	]*vpcmpub k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e aa 00 10 00 00 7b[ 	]*vpcmpub k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e 6a 80 7b[ 	]*vpcmpub k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e aa e0 ef ff ff 7b[ 	]*vpcmpub k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3e ed 00[ 	]*vpcmpequb k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3e ed 00[ 	]*vpcmpequb k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3e ed 02[ 	]*vpcmpleub k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3e ed 02[ 	]*vpcmpleub k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3e ed 01[ 	]*vpcmpltub k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3e ed 01[ 	]*vpcmpltub k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3e ed 04[ 	]*vpcmpnequb k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3e ed 04[ 	]*vpcmpnequb k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3e ed 06[ 	]*vpcmpnleub k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3e ed 06[ 	]*vpcmpnleub k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 3e ed 05[ 	]*vpcmpnltub k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 28 3e ed 05[ 	]*vpcmpnltub k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e ed ab[ 	]*vpcmpuw k5\{k7\},xmm6,xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e ed 7b[ 	]*vpcmpuw k5\{k7\},xmm6,xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e 29 7b[ 	]*vpcmpuw k5\{k7\},xmm6,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpuw k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e 6a 7f 7b[ 	]*vpcmpuw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e aa 00 08 00 00 7b[ 	]*vpcmpuw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e 6a 80 7b[ 	]*vpcmpuw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e aa f0 f7 ff ff 7b[ 	]*vpcmpuw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e ed ab[ 	]*vpcmpuw k5\{k7\},ymm6,ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e ed 7b[ 	]*vpcmpuw k5\{k7\},ymm6,ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e 29 7b[ 	]*vpcmpuw k5\{k7\},ymm6,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpuw k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e 6a 7f 7b[ 	]*vpcmpuw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e aa 00 10 00 00 7b[ 	]*vpcmpuw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e 6a 80 7b[ 	]*vpcmpuw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e aa e0 ef ff ff 7b[ 	]*vpcmpuw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3e ed 00[ 	]*vpcmpequw k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3e ed 00[ 	]*vpcmpequw k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3e ed 02[ 	]*vpcmpleuw k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3e ed 02[ 	]*vpcmpleuw k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3e ed 01[ 	]*vpcmpltuw k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3e ed 01[ 	]*vpcmpltuw k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3e ed 04[ 	]*vpcmpnequw k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3e ed 04[ 	]*vpcmpnequw k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3e ed 06[ 	]*vpcmpnleuw k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3e ed 06[ 	]*vpcmpnleuw k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 08 3e ed 05[ 	]*vpcmpnltuw k5,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 28 3e ed 05[ 	]*vpcmpnltuw k5,ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c f5[ 	]*vpabsb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 1c f5[ 	]*vpabsb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c 31[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c b4 f4 c0 1d fe ff[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c 72 7f[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c b2 00 08 00 00[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c 72 80[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1c b2 f0 f7 ff ff[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c f5[ 	]*vpabsb ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 1c f5[ 	]*vpabsb ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c 31[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c b4 f4 c0 1d fe ff[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c 72 7f[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c b2 00 10 00 00[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c 72 80[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1c b2 e0 ef ff ff[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d f5[ 	]*vpabsw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 1d f5[ 	]*vpabsw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d 31[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d b4 f4 c0 1d fe ff[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d 72 7f[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d b2 00 08 00 00[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d 72 80[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 1d b2 f0 f7 ff ff[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d f5[ 	]*vpabsw ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 1d f5[ 	]*vpabsw ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d 31[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d b4 f4 c0 1d fe ff[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d 72 7f[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d b2 00 10 00 00[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d 72 80[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 1d b2 e0 ef ff ff[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b f4[ 	]*vpackssdw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 6b f4[ 	]*vpackssdw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b 31[ 	]*vpackssdw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b b4 f4 c0 1d fe ff[ 	]*vpackssdw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b 30[ 	]*vpackssdw xmm6\{k7\},xmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b 72 7f[ 	]*vpackssdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b b2 00 08 00 00[ 	]*vpackssdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b 72 80[ 	]*vpackssdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 6b b2 f0 f7 ff ff[ 	]*vpackssdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b 72 7f[ 	]*vpackssdw xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b b2 00 02 00 00[ 	]*vpackssdw xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b 72 80[ 	]*vpackssdw xmm6\{k7\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 1f 6b b2 fc fd ff ff[ 	]*vpackssdw xmm6\{k7\},xmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b f4[ 	]*vpackssdw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 6b f4[ 	]*vpackssdw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b 31[ 	]*vpackssdw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b b4 f4 c0 1d fe ff[ 	]*vpackssdw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b 30[ 	]*vpackssdw ymm6\{k7\},ymm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b 72 7f[ 	]*vpackssdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b b2 00 10 00 00[ 	]*vpackssdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b 72 80[ 	]*vpackssdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 6b b2 e0 ef ff ff[ 	]*vpackssdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b 72 7f[ 	]*vpackssdw ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b b2 00 02 00 00[ 	]*vpackssdw ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b 72 80[ 	]*vpackssdw ymm6\{k7\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 3f 6b b2 fc fd ff ff[ 	]*vpackssdw ymm6\{k7\},ymm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 f4[ 	]*vpacksswb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 63 f4[ 	]*vpacksswb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 31[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 b4 f4 c0 1d fe ff[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 72 7f[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 b2 00 08 00 00[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 72 80[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 63 b2 f0 f7 ff ff[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 f4[ 	]*vpacksswb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 63 f4[ 	]*vpacksswb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 31[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 b4 f4 c0 1d fe ff[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 72 7f[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 b2 00 10 00 00[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 72 80[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 63 b2 e0 ef ff ff[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b f4[ 	]*vpackusdw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 2b f4[ 	]*vpackusdw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b 31[ 	]*vpackusdw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b b4 f4 c0 1d fe ff[ 	]*vpackusdw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b 30[ 	]*vpackusdw xmm6\{k7\},xmm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b 72 7f[ 	]*vpackusdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b b2 00 08 00 00[ 	]*vpackusdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b 72 80[ 	]*vpackusdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 2b b2 f0 f7 ff ff[ 	]*vpackusdw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b 72 7f[ 	]*vpackusdw xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b b2 00 02 00 00[ 	]*vpackusdw xmm6\{k7\},xmm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b 72 80[ 	]*vpackusdw xmm6\{k7\},xmm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 1f 2b b2 fc fd ff ff[ 	]*vpackusdw xmm6\{k7\},xmm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b f4[ 	]*vpackusdw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 2b f4[ 	]*vpackusdw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b 31[ 	]*vpackusdw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b b4 f4 c0 1d fe ff[ 	]*vpackusdw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b 30[ 	]*vpackusdw ymm6\{k7\},ymm5,DWORD BCST \[eax\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b 72 7f[ 	]*vpackusdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b b2 00 10 00 00[ 	]*vpackusdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b 72 80[ 	]*vpackusdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 2b b2 e0 ef ff ff[ 	]*vpackusdw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b 72 7f[ 	]*vpackusdw ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x1fc\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b b2 00 02 00 00[ 	]*vpackusdw ymm6\{k7\},ymm5,DWORD BCST \[edx\+0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b 72 80[ 	]*vpackusdw ymm6\{k7\},ymm5,DWORD BCST \[edx-0x200\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 3f 2b b2 fc fd ff ff[ 	]*vpackusdw ymm6\{k7\},ymm5,DWORD BCST \[edx-0x204\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 f4[ 	]*vpackuswb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 67 f4[ 	]*vpackuswb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 31[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 b4 f4 c0 1d fe ff[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 72 7f[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 b2 00 08 00 00[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 72 80[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 67 b2 f0 f7 ff ff[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 f4[ 	]*vpackuswb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 67 f4[ 	]*vpackuswb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 31[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 b4 f4 c0 1d fe ff[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 72 7f[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 b2 00 10 00 00[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 72 80[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 67 b2 e0 ef ff ff[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc f4[ 	]*vpaddb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f fc f4[ 	]*vpaddb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc 31[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc b4 f4 c0 1d fe ff[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc 72 7f[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc b2 00 08 00 00[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc 72 80[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fc b2 f0 f7 ff ff[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc f4[ 	]*vpaddb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af fc f4[ 	]*vpaddb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc 31[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc b4 f4 c0 1d fe ff[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc 72 7f[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc b2 00 10 00 00[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc 72 80[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fc b2 e0 ef ff ff[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec f4[ 	]*vpaddsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ec f4[ 	]*vpaddsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec 31[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec b4 f4 c0 1d fe ff[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec 72 7f[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec b2 00 08 00 00[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec 72 80[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ec b2 f0 f7 ff ff[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec f4[ 	]*vpaddsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ec f4[ 	]*vpaddsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec 31[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec b4 f4 c0 1d fe ff[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec 72 7f[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec b2 00 10 00 00[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec 72 80[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ec b2 e0 ef ff ff[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed f4[ 	]*vpaddsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ed f4[ 	]*vpaddsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed 31[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed b4 f4 c0 1d fe ff[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed 72 7f[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed b2 00 08 00 00[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed 72 80[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ed b2 f0 f7 ff ff[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed f4[ 	]*vpaddsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ed f4[ 	]*vpaddsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed 31[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed b4 f4 c0 1d fe ff[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed 72 7f[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed b2 00 10 00 00[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed 72 80[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ed b2 e0 ef ff ff[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc f4[ 	]*vpaddusb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f dc f4[ 	]*vpaddusb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc 31[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc b4 f4 c0 1d fe ff[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc 72 7f[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc b2 00 08 00 00[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc 72 80[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dc b2 f0 f7 ff ff[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc f4[ 	]*vpaddusb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af dc f4[ 	]*vpaddusb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc 31[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc b4 f4 c0 1d fe ff[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc 72 7f[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc b2 00 10 00 00[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc 72 80[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dc b2 e0 ef ff ff[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd f4[ 	]*vpaddusw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f dd f4[ 	]*vpaddusw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd 31[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd b4 f4 c0 1d fe ff[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd 72 7f[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd b2 00 08 00 00[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd 72 80[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f dd b2 f0 f7 ff ff[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd f4[ 	]*vpaddusw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af dd f4[ 	]*vpaddusw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd 31[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd b4 f4 c0 1d fe ff[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd 72 7f[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd b2 00 10 00 00[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd 72 80[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f dd b2 e0 ef ff ff[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd f4[ 	]*vpaddw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f fd f4[ 	]*vpaddw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd 31[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd b4 f4 c0 1d fe ff[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd 72 7f[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd b2 00 08 00 00[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd 72 80[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f fd b2 f0 f7 ff ff[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd f4[ 	]*vpaddw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af fd f4[ 	]*vpaddw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd 31[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd b4 f4 c0 1d fe ff[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd 72 7f[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd b2 00 10 00 00[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd 72 80[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f fd b2 e0 ef ff ff[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f f4 ab[ 	]*vpalignr xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 0f f4 ab[ 	]*vpalignr xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f f4 7b[ 	]*vpalignr xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f 31 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f b4 f4 c0 1d fe ff 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f 72 7f 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f b2 00 08 00 00 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f 72 80 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 0f b2 f0 f7 ff ff 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f f4 ab[ 	]*vpalignr ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 0f f4 ab[ 	]*vpalignr ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f f4 7b[ 	]*vpalignr ymm6\{k7\},ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f 31 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f b4 f4 c0 1d fe ff 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f 72 7f 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f b2 00 10 00 00 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f 72 80 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 0f b2 e0 ef ff ff 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 f4[ 	]*vpavgb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e0 f4[ 	]*vpavgb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 31[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 b4 f4 c0 1d fe ff[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 72 7f[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 b2 00 08 00 00[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 72 80[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e0 b2 f0 f7 ff ff[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 f4[ 	]*vpavgb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e0 f4[ 	]*vpavgb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 31[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 b4 f4 c0 1d fe ff[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 72 7f[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 b2 00 10 00 00[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 72 80[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e0 b2 e0 ef ff ff[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 f4[ 	]*vpavgw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e3 f4[ 	]*vpavgw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 31[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 b4 f4 c0 1d fe ff[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 72 7f[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 b2 00 08 00 00[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 72 80[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e3 b2 f0 f7 ff ff[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 f4[ 	]*vpavgw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e3 f4[ 	]*vpavgw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 31[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 b4 f4 c0 1d fe ff[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 72 7f[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 b2 00 10 00 00[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 72 80[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e3 b2 e0 ef ff ff[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 f4[ 	]*vpblendmb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 66 f4[ 	]*vpblendmb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 31[ 	]*vpblendmb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 72 7f[ 	]*vpblendmb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 b2 00 08 00 00[ 	]*vpblendmb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 72 80[ 	]*vpblendmb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 66 b2 f0 f7 ff ff[ 	]*vpblendmb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 f4[ 	]*vpblendmb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 66 f4[ 	]*vpblendmb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 31[ 	]*vpblendmb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 72 7f[ 	]*vpblendmb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 b2 00 10 00 00[ 	]*vpblendmb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 72 80[ 	]*vpblendmb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 66 b2 e0 ef ff ff[ 	]*vpblendmb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 f5[ 	]*vpbroadcastb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 78 f5[ 	]*vpbroadcastb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 31[ 	]*vpbroadcastb xmm6\{k7\},BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 b4 f4 c0 1d fe ff[ 	]*vpbroadcastb xmm6\{k7\},BYTE PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 72 7f[ 	]*vpbroadcastb xmm6\{k7\},BYTE PTR \[edx\+0x7f\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 b2 80 00 00 00[ 	]*vpbroadcastb xmm6\{k7\},BYTE PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 72 80[ 	]*vpbroadcastb xmm6\{k7\},BYTE PTR \[edx-0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 78 b2 7f ff ff ff[ 	]*vpbroadcastb xmm6\{k7\},BYTE PTR \[edx-0x81\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 f5[ 	]*vpbroadcastb ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 78 f5[ 	]*vpbroadcastb ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 31[ 	]*vpbroadcastb ymm6\{k7\},BYTE PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 b4 f4 c0 1d fe ff[ 	]*vpbroadcastb ymm6\{k7\},BYTE PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 72 7f[ 	]*vpbroadcastb ymm6\{k7\},BYTE PTR \[edx\+0x7f\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 b2 80 00 00 00[ 	]*vpbroadcastb ymm6\{k7\},BYTE PTR \[edx\+0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 72 80[ 	]*vpbroadcastb ymm6\{k7\},BYTE PTR \[edx-0x80\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 78 b2 7f ff ff ff[ 	]*vpbroadcastb ymm6\{k7\},BYTE PTR \[edx-0x81\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7a f0[ 	]*vpbroadcastb xmm6\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 7a f0[ 	]*vpbroadcastb xmm6\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7a f5[ 	]*vpbroadcastb xmm6\{k7\},ebp
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7a f0[ 	]*vpbroadcastb ymm6\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 7a f0[ 	]*vpbroadcastb ymm6\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7a f5[ 	]*vpbroadcastb ymm6\{k7\},ebp
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 f5[ 	]*vpbroadcastw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 79 f5[ 	]*vpbroadcastw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 31[ 	]*vpbroadcastw xmm6\{k7\},WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 b4 f4 c0 1d fe ff[ 	]*vpbroadcastw xmm6\{k7\},WORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 72 7f[ 	]*vpbroadcastw xmm6\{k7\},WORD PTR \[edx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 b2 00 01 00 00[ 	]*vpbroadcastw xmm6\{k7\},WORD PTR \[edx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 72 80[ 	]*vpbroadcastw xmm6\{k7\},WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 79 b2 fe fe ff ff[ 	]*vpbroadcastw xmm6\{k7\},WORD PTR \[edx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 f5[ 	]*vpbroadcastw ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 79 f5[ 	]*vpbroadcastw ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 31[ 	]*vpbroadcastw ymm6\{k7\},WORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 b4 f4 c0 1d fe ff[ 	]*vpbroadcastw ymm6\{k7\},WORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 72 7f[ 	]*vpbroadcastw ymm6\{k7\},WORD PTR \[edx\+0xfe\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 b2 00 01 00 00[ 	]*vpbroadcastw ymm6\{k7\},WORD PTR \[edx\+0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 72 80[ 	]*vpbroadcastw ymm6\{k7\},WORD PTR \[edx-0x100\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 79 b2 fe fe ff ff[ 	]*vpbroadcastw ymm6\{k7\},WORD PTR \[edx-0x102\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7b f0[ 	]*vpbroadcastw xmm6\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 7b f0[ 	]*vpbroadcastw xmm6\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 7b f5[ 	]*vpbroadcastw xmm6\{k7\},ebp
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7b f0[ 	]*vpbroadcastw ymm6\{k7\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 7b f0[ 	]*vpbroadcastw ymm6\{k7\}\{z\},eax
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 7b f5[ 	]*vpbroadcastw ymm6\{k7\},ebp
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 ed[ 	]*vpcmpeqb k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 29[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 ac f4 c0 1d fe ff[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 6a 7f[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 aa 00 08 00 00[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 6a 80[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 74 aa f0 f7 ff ff[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 ed[ 	]*vpcmpeqb k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 29[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 ac f4 c0 1d fe ff[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 6a 7f[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 aa 00 10 00 00[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 6a 80[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 74 aa e0 ef ff ff[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 ed[ 	]*vpcmpeqw k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 29[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 ac f4 c0 1d fe ff[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 6a 7f[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 aa 00 08 00 00[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 6a 80[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 75 aa f0 f7 ff ff[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 ed[ 	]*vpcmpeqw k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 29[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 ac f4 c0 1d fe ff[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 6a 7f[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 aa 00 10 00 00[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 6a 80[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 75 aa e0 ef ff ff[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 ed[ 	]*vpcmpgtb k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 29[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 ac f4 c0 1d fe ff[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 6a 7f[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 aa 00 08 00 00[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 6a 80[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 64 aa f0 f7 ff ff[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 ed[ 	]*vpcmpgtb k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 29[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 ac f4 c0 1d fe ff[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 6a 7f[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 aa 00 10 00 00[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 6a 80[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 64 aa e0 ef ff ff[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 ed[ 	]*vpcmpgtw k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 29[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 ac f4 c0 1d fe ff[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 6a 7f[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 aa 00 08 00 00[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 6a 80[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 65 aa f0 f7 ff ff[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 ed[ 	]*vpcmpgtw k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 29[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 ac f4 c0 1d fe ff[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 6a 7f[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 aa 00 10 00 00[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 6a 80[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 65 aa e0 ef ff ff[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 f4[ 	]*vpblendmw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 66 f4[ 	]*vpblendmw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 31[ 	]*vpblendmw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 72 7f[ 	]*vpblendmw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 b2 00 08 00 00[ 	]*vpblendmw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 72 80[ 	]*vpblendmw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 66 b2 f0 f7 ff ff[ 	]*vpblendmw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 f4[ 	]*vpblendmw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 66 f4[ 	]*vpblendmw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 31[ 	]*vpblendmw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 b4 f4 c0 1d fe ff[ 	]*vpblendmw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 72 7f[ 	]*vpblendmw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 b2 00 10 00 00[ 	]*vpblendmw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 72 80[ 	]*vpblendmw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 66 b2 e0 ef ff ff[ 	]*vpblendmw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 f4[ 	]*vpmaddubsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 04 f4[ 	]*vpmaddubsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 31[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 b4 f4 c0 1d fe ff[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 72 7f[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 b2 00 08 00 00[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 72 80[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 04 b2 f0 f7 ff ff[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 f4[ 	]*vpmaddubsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 04 f4[ 	]*vpmaddubsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 31[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 b4 f4 c0 1d fe ff[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 72 7f[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 b2 00 10 00 00[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 72 80[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 04 b2 e0 ef ff ff[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 f4[ 	]*vpmaddwd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f5 f4[ 	]*vpmaddwd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 31[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 b4 f4 c0 1d fe ff[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 72 7f[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 b2 00 08 00 00[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 72 80[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f5 b2 f0 f7 ff ff[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 f4[ 	]*vpmaddwd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f5 f4[ 	]*vpmaddwd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 31[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 b4 f4 c0 1d fe ff[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 72 7f[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 b2 00 10 00 00[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 72 80[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f5 b2 e0 ef ff ff[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c f4[ 	]*vpmaxsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 3c f4[ 	]*vpmaxsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c 31[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c b4 f4 c0 1d fe ff[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c 72 7f[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c b2 00 08 00 00[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c 72 80[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3c b2 f0 f7 ff ff[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c f4[ 	]*vpmaxsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 3c f4[ 	]*vpmaxsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c 31[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c b4 f4 c0 1d fe ff[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c 72 7f[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c b2 00 10 00 00[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c 72 80[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3c b2 e0 ef ff ff[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee f4[ 	]*vpmaxsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ee f4[ 	]*vpmaxsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee 31[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee b4 f4 c0 1d fe ff[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee 72 7f[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee b2 00 08 00 00[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee 72 80[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ee b2 f0 f7 ff ff[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee f4[ 	]*vpmaxsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ee f4[ 	]*vpmaxsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee 31[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee b4 f4 c0 1d fe ff[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee 72 7f[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee b2 00 10 00 00[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee 72 80[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ee b2 e0 ef ff ff[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de f4[ 	]*vpmaxub xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f de f4[ 	]*vpmaxub xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de 31[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de b4 f4 c0 1d fe ff[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de 72 7f[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de b2 00 08 00 00[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de 72 80[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f de b2 f0 f7 ff ff[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de f4[ 	]*vpmaxub ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af de f4[ 	]*vpmaxub ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de 31[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de b4 f4 c0 1d fe ff[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de 72 7f[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de b2 00 10 00 00[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de 72 80[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f de b2 e0 ef ff ff[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e f4[ 	]*vpmaxuw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 3e f4[ 	]*vpmaxuw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e 31[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e b4 f4 c0 1d fe ff[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e 72 7f[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e b2 00 08 00 00[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e 72 80[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3e b2 f0 f7 ff ff[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e f4[ 	]*vpmaxuw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 3e f4[ 	]*vpmaxuw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e 31[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e b4 f4 c0 1d fe ff[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e 72 7f[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e b2 00 10 00 00[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e 72 80[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3e b2 e0 ef ff ff[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 f4[ 	]*vpminsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 38 f4[ 	]*vpminsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 31[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 b4 f4 c0 1d fe ff[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 72 7f[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 b2 00 08 00 00[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 72 80[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 38 b2 f0 f7 ff ff[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 f4[ 	]*vpminsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 38 f4[ 	]*vpminsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 31[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 b4 f4 c0 1d fe ff[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 72 7f[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 b2 00 10 00 00[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 72 80[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 38 b2 e0 ef ff ff[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea f4[ 	]*vpminsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f ea f4[ 	]*vpminsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea 31[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea b4 f4 c0 1d fe ff[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea 72 7f[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea b2 00 08 00 00[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea 72 80[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f ea b2 f0 f7 ff ff[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea f4[ 	]*vpminsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af ea f4[ 	]*vpminsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea 31[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea b4 f4 c0 1d fe ff[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea 72 7f[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea b2 00 10 00 00[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea 72 80[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f ea b2 e0 ef ff ff[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da f4[ 	]*vpminub xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f da f4[ 	]*vpminub xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da 31[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da b4 f4 c0 1d fe ff[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da 72 7f[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da b2 00 08 00 00[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da 72 80[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f da b2 f0 f7 ff ff[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da f4[ 	]*vpminub ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af da f4[ 	]*vpminub ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da 31[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da b4 f4 c0 1d fe ff[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da 72 7f[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da b2 00 10 00 00[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da 72 80[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f da b2 e0 ef ff ff[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a f4[ 	]*vpminuw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 3a f4[ 	]*vpminuw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a 31[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a b4 f4 c0 1d fe ff[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a 72 7f[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a b2 00 08 00 00[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a 72 80[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 3a b2 f0 f7 ff ff[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a f4[ 	]*vpminuw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 3a f4[ 	]*vpminuw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a 31[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a b4 f4 c0 1d fe ff[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a 72 7f[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a b2 00 10 00 00[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a 72 80[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 3a b2 e0 ef ff ff[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 f5[ 	]*vpmovsxbw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 20 f5[ 	]*vpmovsxbw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 31[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 b4 f4 c0 1d fe ff[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 72 7f[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 b2 00 04 00 00[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 72 80[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 20 b2 f8 fb ff ff[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 f5[ 	]*vpmovsxbw ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 20 f5[ 	]*vpmovsxbw ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 31[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 b4 f4 c0 1d fe ff[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 72 7f[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 b2 00 08 00 00[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 72 80[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 20 b2 f0 f7 ff ff[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 f5[ 	]*vpmovzxbw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 8f 30 f5[ 	]*vpmovzxbw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 31[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 b4 f4 c0 1d fe ff[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 72 7f[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 b2 00 04 00 00[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 72 80[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 0f 30 b2 f8 fb ff ff[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 f5[ 	]*vpmovzxbw ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d af 30 f5[ 	]*vpmovzxbw ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 31[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 b4 f4 c0 1d fe ff[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 72 7f[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 b2 00 08 00 00[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 72 80[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7d 2f 30 b2 f0 f7 ff ff[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b f4[ 	]*vpmulhrsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 0b f4[ 	]*vpmulhrsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b 31[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b b4 f4 c0 1d fe ff[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b 72 7f[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b b2 00 08 00 00[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b 72 80[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 0b b2 f0 f7 ff ff[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b f4[ 	]*vpmulhrsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 0b f4[ 	]*vpmulhrsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b 31[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b b4 f4 c0 1d fe ff[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b 72 7f[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b b2 00 10 00 00[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b 72 80[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 0b b2 e0 ef ff ff[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 f4[ 	]*vpmulhuw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e4 f4[ 	]*vpmulhuw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 31[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 b4 f4 c0 1d fe ff[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 72 7f[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 b2 00 08 00 00[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 72 80[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e4 b2 f0 f7 ff ff[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 f4[ 	]*vpmulhuw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e4 f4[ 	]*vpmulhuw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 31[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 b4 f4 c0 1d fe ff[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 72 7f[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 b2 00 10 00 00[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 72 80[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e4 b2 e0 ef ff ff[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 f4[ 	]*vpmulhw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e5 f4[ 	]*vpmulhw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 31[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 b4 f4 c0 1d fe ff[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 72 7f[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 b2 00 08 00 00[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 72 80[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e5 b2 f0 f7 ff ff[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 f4[ 	]*vpmulhw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e5 f4[ 	]*vpmulhw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 31[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 b4 f4 c0 1d fe ff[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 72 7f[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 b2 00 10 00 00[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 72 80[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e5 b2 e0 ef ff ff[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 f4[ 	]*vpmullw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d5 f4[ 	]*vpmullw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 31[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 b4 f4 c0 1d fe ff[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 72 7f[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 b2 00 08 00 00[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 72 80[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d5 b2 f0 f7 ff ff[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 f4[ 	]*vpmullw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d5 f4[ 	]*vpmullw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 31[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 b4 f4 c0 1d fe ff[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 72 7f[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 b2 00 10 00 00[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 72 80[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d5 b2 e0 ef ff ff[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 f4[ 	]*vpshufb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f 00 f4[ 	]*vpshufb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 31[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 b4 f4 c0 1d fe ff[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 72 7f[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 b2 00 08 00 00[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 72 80[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f 00 b2 f0 f7 ff ff[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 f4[ 	]*vpshufb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af 00 f4[ 	]*vpshufb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 31[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 b4 f4 c0 1d fe ff[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 72 7f[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 b2 00 10 00 00[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 72 80[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f 00 b2 e0 ef ff ff[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 f5 ab[ 	]*vpshufhw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 8f 70 f5 ab[ 	]*vpshufhw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 f5 7b[ 	]*vpshufhw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 31 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 72 7f 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 b2 00 08 00 00 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 72 80 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 0f 70 b2 f0 f7 ff ff 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 f5 ab[ 	]*vpshufhw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e af 70 f5 ab[ 	]*vpshufhw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 f5 7b[ 	]*vpshufhw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 31 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 72 7f 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 b2 00 10 00 00 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 72 80 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7e 2f 70 b2 e0 ef ff ff 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 f5 ab[ 	]*vpshuflw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 70 f5 ab[ 	]*vpshuflw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 f5 7b[ 	]*vpshuflw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 31 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 72 7f 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 b2 00 08 00 00 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 72 80 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 70 b2 f0 f7 ff ff 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 f5 ab[ 	]*vpshuflw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 70 f5 ab[ 	]*vpshuflw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 f5 7b[ 	]*vpshuflw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 31 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 72 7f 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 b2 00 10 00 00 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 72 80 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 70 b2 e0 ef ff ff 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 f4[ 	]*vpsllw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f1 f4[ 	]*vpsllw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 31[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 b4 f4 c0 1d fe ff[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 72 7f[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 b2 00 08 00 00[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 72 80[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f1 b2 f0 f7 ff ff[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 f4[ 	]*vpsllw ymm6\{k7\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f1 f4[ 	]*vpsllw ymm6\{k7\}\{z\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 31[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 b4 f4 c0 1d fe ff[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 72 7f[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 b2 00 08 00 00[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 72 80[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f1 b2 f0 f7 ff ff[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 f4[ 	]*vpsraw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e1 f4[ 	]*vpsraw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 31[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 b4 f4 c0 1d fe ff[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 72 7f[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 b2 00 08 00 00[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 72 80[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e1 b2 f0 f7 ff ff[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 f4[ 	]*vpsraw ymm6\{k7\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e1 f4[ 	]*vpsraw ymm6\{k7\}\{z\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 31[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 b4 f4 c0 1d fe ff[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 72 7f[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 b2 00 08 00 00[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 72 80[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e1 b2 f0 f7 ff ff[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 f4[ 	]*vpsrlw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d1 f4[ 	]*vpsrlw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 31[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 b4 f4 c0 1d fe ff[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 72 7f[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 b2 00 08 00 00[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 72 80[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d1 b2 f0 f7 ff ff[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 f4[ 	]*vpsrlw ymm6\{k7\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d1 f4[ 	]*vpsrlw ymm6\{k7\}\{z\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 31[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 b4 f4 c0 1d fe ff[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 72 7f[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 b2 00 08 00 00[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 72 80[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d1 b2 f0 f7 ff ff[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 d5 ab[ 	]*vpsrlw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 8f 71 d5 ab[ 	]*vpsrlw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 d5 7b[ 	]*vpsrlw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 11 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 94 f4 c0 1d fe ff 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 52 7f 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 92 00 08 00 00 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 52 80 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 92 f0 f7 ff ff 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 d5 ab[ 	]*vpsrlw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d af 71 d5 ab[ 	]*vpsrlw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 d5 7b[ 	]*vpsrlw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 11 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 94 f4 c0 1d fe ff 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 52 7f 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 92 00 10 00 00 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 52 80 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 92 e0 ef ff ff 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 e5 ab[ 	]*vpsraw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 8f 71 e5 ab[ 	]*vpsraw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 e5 7b[ 	]*vpsraw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 21 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 a4 f4 c0 1d fe ff 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 62 7f 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 a2 00 08 00 00 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 62 80 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 a2 f0 f7 ff ff 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 e5 ab[ 	]*vpsraw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d af 71 e5 ab[ 	]*vpsraw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 e5 7b[ 	]*vpsraw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 21 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 a4 f4 c0 1d fe ff 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 62 7f 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 a2 00 10 00 00 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 62 80 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 a2 e0 ef ff ff 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 f4[ 	]*vpsrlvw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 10 f4[ 	]*vpsrlvw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 31[ 	]*vpsrlvw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 b4 f4 c0 1d fe ff[ 	]*vpsrlvw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 72 7f[ 	]*vpsrlvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 b2 00 08 00 00[ 	]*vpsrlvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 72 80[ 	]*vpsrlvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 10 b2 f0 f7 ff ff[ 	]*vpsrlvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 f4[ 	]*vpsrlvw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 10 f4[ 	]*vpsrlvw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 31[ 	]*vpsrlvw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 b4 f4 c0 1d fe ff[ 	]*vpsrlvw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 72 7f[ 	]*vpsrlvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 b2 00 10 00 00[ 	]*vpsrlvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 72 80[ 	]*vpsrlvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 10 b2 e0 ef ff ff[ 	]*vpsrlvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 f4[ 	]*vpsravw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 11 f4[ 	]*vpsravw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 31[ 	]*vpsravw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 b4 f4 c0 1d fe ff[ 	]*vpsravw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 72 7f[ 	]*vpsravw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 b2 00 08 00 00[ 	]*vpsravw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 72 80[ 	]*vpsravw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 11 b2 f0 f7 ff ff[ 	]*vpsravw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 f4[ 	]*vpsravw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 11 f4[ 	]*vpsravw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 31[ 	]*vpsravw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 b4 f4 c0 1d fe ff[ 	]*vpsravw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 72 7f[ 	]*vpsravw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 b2 00 10 00 00[ 	]*vpsravw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 72 80[ 	]*vpsravw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 11 b2 e0 ef ff ff[ 	]*vpsravw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 f4[ 	]*vpsubb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f8 f4[ 	]*vpsubb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 31[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 b4 f4 c0 1d fe ff[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 72 7f[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 b2 00 08 00 00[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 72 80[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f8 b2 f0 f7 ff ff[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 f4[ 	]*vpsubb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f8 f4[ 	]*vpsubb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 31[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 b4 f4 c0 1d fe ff[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 72 7f[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 b2 00 10 00 00[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 72 80[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f8 b2 e0 ef ff ff[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 f4[ 	]*vpsubsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e8 f4[ 	]*vpsubsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 31[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 b4 f4 c0 1d fe ff[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 72 7f[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 b2 00 08 00 00[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 72 80[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e8 b2 f0 f7 ff ff[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 f4[ 	]*vpsubsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e8 f4[ 	]*vpsubsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 31[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 b4 f4 c0 1d fe ff[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 72 7f[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 b2 00 10 00 00[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 72 80[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e8 b2 e0 ef ff ff[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 f4[ 	]*vpsubsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f e9 f4[ 	]*vpsubsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 31[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 b4 f4 c0 1d fe ff[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 72 7f[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 b2 00 08 00 00[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 72 80[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f e9 b2 f0 f7 ff ff[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 f4[ 	]*vpsubsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af e9 f4[ 	]*vpsubsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 31[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 b4 f4 c0 1d fe ff[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 72 7f[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 b2 00 10 00 00[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 72 80[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f e9 b2 e0 ef ff ff[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 f4[ 	]*vpsubusb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d8 f4[ 	]*vpsubusb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 31[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 b4 f4 c0 1d fe ff[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 72 7f[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 b2 00 08 00 00[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 72 80[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d8 b2 f0 f7 ff ff[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 f4[ 	]*vpsubusb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d8 f4[ 	]*vpsubusb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 31[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 b4 f4 c0 1d fe ff[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 72 7f[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 b2 00 10 00 00[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 72 80[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d8 b2 e0 ef ff ff[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 f4[ 	]*vpsubusw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f d9 f4[ 	]*vpsubusw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 31[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 b4 f4 c0 1d fe ff[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 72 7f[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 b2 00 08 00 00[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 72 80[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f d9 b2 f0 f7 ff ff[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 f4[ 	]*vpsubusw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af d9 f4[ 	]*vpsubusw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 31[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 b4 f4 c0 1d fe ff[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 72 7f[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 b2 00 10 00 00[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 72 80[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f d9 b2 e0 ef ff ff[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 f4[ 	]*vpsubw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f f9 f4[ 	]*vpsubw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 31[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 b4 f4 c0 1d fe ff[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 72 7f[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 b2 00 08 00 00[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 72 80[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f f9 b2 f0 f7 ff ff[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 f4[ 	]*vpsubw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af f9 f4[ 	]*vpsubw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 31[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 b4 f4 c0 1d fe ff[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 72 7f[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 b2 00 10 00 00[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 72 80[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f f9 b2 e0 ef ff ff[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 f4[ 	]*vpunpckhbw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 68 f4[ 	]*vpunpckhbw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 31[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 b4 f4 c0 1d fe ff[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 72 7f[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 b2 00 08 00 00[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 72 80[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 68 b2 f0 f7 ff ff[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 f4[ 	]*vpunpckhbw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 68 f4[ 	]*vpunpckhbw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 31[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 b4 f4 c0 1d fe ff[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 72 7f[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 b2 00 10 00 00[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 72 80[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 68 b2 e0 ef ff ff[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 f4[ 	]*vpunpckhwd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 69 f4[ 	]*vpunpckhwd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 31[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 b4 f4 c0 1d fe ff[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 72 7f[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 b2 00 08 00 00[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 72 80[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 69 b2 f0 f7 ff ff[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 f4[ 	]*vpunpckhwd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 69 f4[ 	]*vpunpckhwd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 31[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 b4 f4 c0 1d fe ff[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 72 7f[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 b2 00 10 00 00[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 72 80[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 69 b2 e0 ef ff ff[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 f4[ 	]*vpunpcklbw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 60 f4[ 	]*vpunpcklbw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 31[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 b4 f4 c0 1d fe ff[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 72 7f[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 b2 00 08 00 00[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 72 80[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 60 b2 f0 f7 ff ff[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 f4[ 	]*vpunpcklbw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 60 f4[ 	]*vpunpcklbw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 31[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 b4 f4 c0 1d fe ff[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 72 7f[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 b2 00 10 00 00[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 72 80[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 60 b2 e0 ef ff ff[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 f4[ 	]*vpunpcklwd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 8f 61 f4[ 	]*vpunpcklwd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 31[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 b4 f4 c0 1d fe ff[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 72 7f[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 b2 00 08 00 00[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 72 80[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 0f 61 b2 f0 f7 ff ff[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 f4[ 	]*vpunpcklwd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 af 61 f4[ 	]*vpunpcklwd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 31[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 b4 f4 c0 1d fe ff[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 72 7f[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 b2 00 10 00 00[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 72 80[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 55 2f 61 b2 e0 ef ff ff[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 ee[ 	]*vpmovwb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 8f 30 ee[ 	]*vpmovwb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 ee[ 	]*vpmovwb xmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e af 30 ee[ 	]*vpmovwb xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 ee[ 	]*vpmovswb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 8f 20 ee[ 	]*vpmovswb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 ee[ 	]*vpmovswb xmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e af 20 ee[ 	]*vpmovswb xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 ee[ 	]*vpmovuswb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 8f 10 ee[ 	]*vpmovuswb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 ee[ 	]*vpmovuswb xmm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e af 10 ee[ 	]*vpmovuswb xmm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 f4 ab[ 	]*vdbpsadbw xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 8f 42 f4 ab[ 	]*vdbpsadbw xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 f4 7b[ 	]*vdbpsadbw xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 31 7b[ 	]*vdbpsadbw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 b4 f4 c0 1d fe ff 7b[ 	]*vdbpsadbw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 72 7f 7b[ 	]*vdbpsadbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 b2 00 08 00 00 7b[ 	]*vdbpsadbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 72 80 7b[ 	]*vdbpsadbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 0f 42 b2 f0 f7 ff ff 7b[ 	]*vdbpsadbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 f4 ab[ 	]*vdbpsadbw ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 af 42 f4 ab[ 	]*vdbpsadbw ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 f4 7b[ 	]*vdbpsadbw ymm6\{k7\},ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 31 7b[ 	]*vdbpsadbw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 b4 f4 c0 1d fe ff 7b[ 	]*vdbpsadbw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 72 7f 7b[ 	]*vdbpsadbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 b2 00 10 00 00 7b[ 	]*vdbpsadbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 72 80 7b[ 	]*vdbpsadbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 2f 42 b2 e0 ef ff ff 7b[ 	]*vdbpsadbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d f4[ 	]*vpermw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 8d f4[ 	]*vpermw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d 31[ 	]*vpermw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d b4 f4 c0 1d fe ff[ 	]*vpermw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d 72 7f[ 	]*vpermw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d b2 00 08 00 00[ 	]*vpermw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d 72 80[ 	]*vpermw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 8d b2 f0 f7 ff ff[ 	]*vpermw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d f4[ 	]*vpermw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 8d f4[ 	]*vpermw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d 31[ 	]*vpermw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d b4 f4 c0 1d fe ff[ 	]*vpermw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d 72 7f[ 	]*vpermw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d b2 00 10 00 00[ 	]*vpermw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d 72 80[ 	]*vpermw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 8d b2 e0 ef ff ff[ 	]*vpermw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d f4[ 	]*vpermt2w xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 7d f4[ 	]*vpermt2w xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d 31[ 	]*vpermt2w xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2w xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d 72 7f[ 	]*vpermt2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d b2 00 08 00 00[ 	]*vpermt2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d 72 80[ 	]*vpermt2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 7d b2 f0 f7 ff ff[ 	]*vpermt2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d f4[ 	]*vpermt2w ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 7d f4[ 	]*vpermt2w ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d 31[ 	]*vpermt2w ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d b4 f4 c0 1d fe ff[ 	]*vpermt2w ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d 72 7f[ 	]*vpermt2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d b2 00 10 00 00[ 	]*vpermt2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d 72 80[ 	]*vpermt2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 7d b2 e0 ef ff ff[ 	]*vpermt2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 f5 ab[ 	]*vpsllw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 8f 71 f5 ab[ 	]*vpsllw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 f5 7b[ 	]*vpsllw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 31 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 72 7f 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 b2 00 08 00 00 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 72 80 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 0f 71 b2 f0 f7 ff ff 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 f5 ab[ 	]*vpsllw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d af 71 f5 ab[ 	]*vpsllw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 f5 7b[ 	]*vpsllw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 31 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 72 7f 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 b2 00 10 00 00 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 72 80 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 4d 2f 71 b2 e0 ef ff ff 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 f4[ 	]*vpsllvw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 12 f4[ 	]*vpsllvw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 31[ 	]*vpsllvw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 b4 f4 c0 1d fe ff[ 	]*vpsllvw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 72 7f[ 	]*vpsllvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 b2 00 08 00 00[ 	]*vpsllvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 72 80[ 	]*vpsllvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 12 b2 f0 f7 ff ff[ 	]*vpsllvw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 f4[ 	]*vpsllvw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 12 f4[ 	]*vpsllvw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 31[ 	]*vpsllvw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 b4 f4 c0 1d fe ff[ 	]*vpsllvw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 72 7f[ 	]*vpsllvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 b2 00 10 00 00[ 	]*vpsllvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 72 80[ 	]*vpsllvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 12 b2 e0 ef ff ff[ 	]*vpsllvw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f f5[ 	]*vmovdqu8 xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 8f 6f f5[ 	]*vmovdqu8 xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f 31[ 	]*vmovdqu8 xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f 72 7f[ 	]*vmovdqu8 xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f b2 00 08 00 00[ 	]*vmovdqu8 xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f 72 80[ 	]*vmovdqu8 xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 6f b2 f0 f7 ff ff[ 	]*vmovdqu8 xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f f5[ 	]*vmovdqu8 ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f af 6f f5[ 	]*vmovdqu8 ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f 31[ 	]*vmovdqu8 ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f 72 7f[ 	]*vmovdqu8 ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f b2 00 10 00 00[ 	]*vmovdqu8 ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f 72 80[ 	]*vmovdqu8 ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 6f b2 e0 ef ff ff[ 	]*vmovdqu8 ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f f5[ 	]*vmovdqu16 xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 6f f5[ 	]*vmovdqu16 xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f 31[ 	]*vmovdqu16 xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f 72 7f[ 	]*vmovdqu16 xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f b2 00 08 00 00[ 	]*vmovdqu16 xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f 72 80[ 	]*vmovdqu16 xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 6f b2 f0 f7 ff ff[ 	]*vmovdqu16 xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f f5[ 	]*vmovdqu16 ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 6f f5[ 	]*vmovdqu16 ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f 31[ 	]*vmovdqu16 ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f 72 7f[ 	]*vmovdqu16 ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f b2 00 10 00 00[ 	]*vmovdqu16 ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f 72 80[ 	]*vmovdqu16 ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 6f b2 e0 ef ff ff[ 	]*vmovdqu16 ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 31[ 	]*vpmovwb QWORD PTR \[ecx\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 b4 f4 c0 1d fe ff[ 	]*vpmovwb QWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 72 7f[ 	]*vpmovwb QWORD PTR \[edx\+0x3f8\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 b2 00 04 00 00[ 	]*vpmovwb QWORD PTR \[edx\+0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 72 80[ 	]*vpmovwb QWORD PTR \[edx-0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 30 b2 f8 fb ff ff[ 	]*vpmovwb QWORD PTR \[edx-0x408\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 31[ 	]*vpmovwb XMMWORD PTR \[ecx\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 b4 f4 c0 1d fe ff[ 	]*vpmovwb XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 72 7f[ 	]*vpmovwb XMMWORD PTR \[edx\+0x7f0\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 b2 00 08 00 00[ 	]*vpmovwb XMMWORD PTR \[edx\+0x800\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 72 80[ 	]*vpmovwb XMMWORD PTR \[edx-0x800\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 30 b2 f0 f7 ff ff[ 	]*vpmovwb XMMWORD PTR \[edx-0x810\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 31[ 	]*vpmovswb QWORD PTR \[ecx\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 b4 f4 c0 1d fe ff[ 	]*vpmovswb QWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 72 7f[ 	]*vpmovswb QWORD PTR \[edx\+0x3f8\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 b2 00 04 00 00[ 	]*vpmovswb QWORD PTR \[edx\+0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 72 80[ 	]*vpmovswb QWORD PTR \[edx-0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 20 b2 f8 fb ff ff[ 	]*vpmovswb QWORD PTR \[edx-0x408\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 31[ 	]*vpmovswb XMMWORD PTR \[ecx\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 b4 f4 c0 1d fe ff[ 	]*vpmovswb XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 72 7f[ 	]*vpmovswb XMMWORD PTR \[edx\+0x7f0\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 b2 00 08 00 00[ 	]*vpmovswb XMMWORD PTR \[edx\+0x800\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 72 80[ 	]*vpmovswb XMMWORD PTR \[edx-0x800\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 20 b2 f0 f7 ff ff[ 	]*vpmovswb XMMWORD PTR \[edx-0x810\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 31[ 	]*vpmovuswb QWORD PTR \[ecx\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 b4 f4 c0 1d fe ff[ 	]*vpmovuswb QWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 72 7f[ 	]*vpmovuswb QWORD PTR \[edx\+0x3f8\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 b2 00 04 00 00[ 	]*vpmovuswb QWORD PTR \[edx\+0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 72 80[ 	]*vpmovuswb QWORD PTR \[edx-0x400\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 0f 10 b2 f8 fb ff ff[ 	]*vpmovuswb QWORD PTR \[edx-0x408\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 31[ 	]*vpmovuswb XMMWORD PTR \[ecx\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 b4 f4 c0 1d fe ff[ 	]*vpmovuswb XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 72 7f[ 	]*vpmovuswb XMMWORD PTR \[edx\+0x7f0\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 b2 00 08 00 00[ 	]*vpmovuswb XMMWORD PTR \[edx\+0x800\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 72 80[ 	]*vpmovuswb XMMWORD PTR \[edx-0x800\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 2f 10 b2 f0 f7 ff ff[ 	]*vpmovuswb XMMWORD PTR \[edx-0x810\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f 31[ 	]*vmovdqu8 XMMWORD PTR \[ecx\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f 72 7f[ 	]*vmovdqu8 XMMWORD PTR \[edx\+0x7f0\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f b2 00 08 00 00[ 	]*vmovdqu8 XMMWORD PTR \[edx\+0x800\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f 72 80[ 	]*vmovdqu8 XMMWORD PTR \[edx-0x800\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 0f 7f b2 f0 f7 ff ff[ 	]*vmovdqu8 XMMWORD PTR \[edx-0x810\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f 31[ 	]*vmovdqu8 YMMWORD PTR \[ecx\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu8 YMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f 72 7f[ 	]*vmovdqu8 YMMWORD PTR \[edx\+0xfe0\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f b2 00 10 00 00[ 	]*vmovdqu8 YMMWORD PTR \[edx\+0x1000\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f 72 80[ 	]*vmovdqu8 YMMWORD PTR \[edx-0x1000\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 7f 2f 7f b2 e0 ef ff ff[ 	]*vmovdqu8 YMMWORD PTR \[edx-0x1020\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f 31[ 	]*vmovdqu16 XMMWORD PTR \[ecx\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 XMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f 72 7f[ 	]*vmovdqu16 XMMWORD PTR \[edx\+0x7f0\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f b2 00 08 00 00[ 	]*vmovdqu16 XMMWORD PTR \[edx\+0x800\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f 72 80[ 	]*vmovdqu16 XMMWORD PTR \[edx-0x800\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 7f b2 f0 f7 ff ff[ 	]*vmovdqu16 XMMWORD PTR \[edx-0x810\]\{k7\},xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f 31[ 	]*vmovdqu16 YMMWORD PTR \[ecx\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f b4 f4 c0 1d fe ff[ 	]*vmovdqu16 YMMWORD PTR \[esp\+esi\*8-0x1e240\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f 72 7f[ 	]*vmovdqu16 YMMWORD PTR \[edx\+0xfe0\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f b2 00 10 00 00[ 	]*vmovdqu16 YMMWORD PTR \[edx\+0x1000\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f 72 80[ 	]*vmovdqu16 YMMWORD PTR \[edx-0x1000\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 7f b2 e0 ef ff ff[ 	]*vmovdqu16 YMMWORD PTR \[edx-0x1020\]\{k7\},ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 f4[ 	]*vpermi2w xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 75 f4[ 	]*vpermi2w xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 31[ 	]*vpermi2w xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2w xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 72 7f[ 	]*vpermi2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 b2 00 08 00 00[ 	]*vpermi2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 72 80[ 	]*vpermi2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 75 b2 f0 f7 ff ff[ 	]*vpermi2w xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 f4[ 	]*vpermi2w ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 75 f4[ 	]*vpermi2w ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 31[ 	]*vpermi2w ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 b4 f4 c0 1d fe ff[ 	]*vpermi2w ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 72 7f[ 	]*vpermi2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 b2 00 10 00 00[ 	]*vpermi2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 72 80[ 	]*vpermi2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 75 b2 e0 ef ff ff[ 	]*vpermi2w ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 ed[ 	]*vptestmb k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 29[ 	]*vptestmb k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 ac f4 c0 1d fe ff[ 	]*vptestmb k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 6a 7f[ 	]*vptestmb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 aa 00 08 00 00[ 	]*vptestmb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 6a 80[ 	]*vptestmb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 0f 26 aa f0 f7 ff ff[ 	]*vptestmb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 ed[ 	]*vptestmb k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 29[ 	]*vptestmb k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 ac f4 c0 1d fe ff[ 	]*vptestmb k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 6a 7f[ 	]*vptestmb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 aa 00 10 00 00[ 	]*vptestmb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 6a 80[ 	]*vptestmb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 4d 2f 26 aa e0 ef ff ff[ 	]*vptestmb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 ed[ 	]*vptestmw k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 29[ 	]*vptestmw k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 ac f4 c0 1d fe ff[ 	]*vptestmw k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 6a 7f[ 	]*vptestmw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 aa 00 08 00 00[ 	]*vptestmw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 6a 80[ 	]*vptestmw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 0f 26 aa f0 f7 ff ff[ 	]*vptestmw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 ed[ 	]*vptestmw k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 29[ 	]*vptestmw k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 ac f4 c0 1d fe ff[ 	]*vptestmw k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 6a 7f[ 	]*vptestmw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 aa 00 10 00 00[ 	]*vptestmw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 6a 80[ 	]*vptestmw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 cd 2f 26 aa e0 ef ff ff[ 	]*vptestmw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 29 ee[ 	]*vpmovb2m k5,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 29 ee[ 	]*vpmovb2m k5,ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 29 ee[ 	]*vpmovw2m k5,xmm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 29 ee[ 	]*vpmovw2m k5,ymm6
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 08 28 f5[ 	]*vpmovm2b xmm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 7e 28 28 f5[ 	]*vpmovm2b ymm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 08 28 f5[ 	]*vpmovm2w xmm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fe 28 28 f5[ 	]*vpmovm2w ymm6,k5
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 ec[ 	]*vptestnmb k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 29[ 	]*vptestnmb k5\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 ac f4 c0 1d fe ff[ 	]*vptestnmb k5\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 6a 7f[ 	]*vptestnmb k5\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 aa 00 08 00 00[ 	]*vptestnmb k5\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 6a 80[ 	]*vptestnmb k5\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 0f 26 aa f0 f7 ff ff[ 	]*vptestnmb k5\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 ec[ 	]*vptestnmb k5\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 29[ 	]*vptestnmb k5\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 ac f4 c0 1d fe ff[ 	]*vptestnmb k5\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 6a 7f[ 	]*vptestnmb k5\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 aa 00 10 00 00[ 	]*vptestnmb k5\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 6a 80[ 	]*vptestnmb k5\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 56 2f 26 aa e0 ef ff ff[ 	]*vptestnmb k5\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 ec[ 	]*vptestnmw k5\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 29[ 	]*vptestnmw k5\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 ac f4 c0 1d fe ff[ 	]*vptestnmw k5\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 6a 7f[ 	]*vptestnmw k5\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 aa 00 08 00 00[ 	]*vptestnmw k5\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 6a 80[ 	]*vptestnmw k5\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 0f 26 aa f0 f7 ff ff[ 	]*vptestnmw k5\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 ec[ 	]*vptestnmw k5\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 29[ 	]*vptestnmw k5\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 ac f4 c0 1d fe ff[ 	]*vptestnmw k5\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 6a 7f[ 	]*vptestnmw k5\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 aa 00 10 00 00[ 	]*vptestnmw k5\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 6a 80[ 	]*vptestnmw k5\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d6 2f 26 aa e0 ef ff ff[ 	]*vptestnmw k5\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f ed ab[ 	]*vpcmpb k5\{k7\},xmm6,xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f ed 7b[ 	]*vpcmpb k5\{k7\},xmm6,xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f 29 7b[ 	]*vpcmpb k5\{k7\},xmm6,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpb k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f 6a 7f 7b[ 	]*vpcmpb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f aa 00 08 00 00 7b[ 	]*vpcmpb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f 6a 80 7b[ 	]*vpcmpb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3f aa f0 f7 ff ff 7b[ 	]*vpcmpb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f ed ab[ 	]*vpcmpb k5\{k7\},ymm6,ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f ed 7b[ 	]*vpcmpb k5\{k7\},ymm6,ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f 29 7b[ 	]*vpcmpb k5\{k7\},ymm6,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpb k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f 6a 7f 7b[ 	]*vpcmpb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f aa 00 10 00 00 7b[ 	]*vpcmpb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f 6a 80 7b[ 	]*vpcmpb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3f aa e0 ef ff ff 7b[ 	]*vpcmpb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f ed ab[ 	]*vpcmpw k5\{k7\},xmm6,xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f ed 7b[ 	]*vpcmpw k5\{k7\},xmm6,xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f 29 7b[ 	]*vpcmpw k5\{k7\},xmm6,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpw k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f 6a 7f 7b[ 	]*vpcmpw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f aa 00 08 00 00 7b[ 	]*vpcmpw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f 6a 80 7b[ 	]*vpcmpw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3f aa f0 f7 ff ff 7b[ 	]*vpcmpw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f ed ab[ 	]*vpcmpw k5\{k7\},ymm6,ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f ed 7b[ 	]*vpcmpw k5\{k7\},ymm6,ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f 29 7b[ 	]*vpcmpw k5\{k7\},ymm6,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f ac f4 c0 1d fe ff 7b[ 	]*vpcmpw k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f 6a 7f 7b[ 	]*vpcmpw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f aa 00 10 00 00 7b[ 	]*vpcmpw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f 6a 80 7b[ 	]*vpcmpw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3f aa e0 ef ff ff 7b[ 	]*vpcmpw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e ed ab[ 	]*vpcmpub k5\{k7\},xmm6,xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e ed 7b[ 	]*vpcmpub k5\{k7\},xmm6,xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e 29 7b[ 	]*vpcmpub k5\{k7\},xmm6,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpub k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e 6a 7f 7b[ 	]*vpcmpub k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e aa 00 08 00 00 7b[ 	]*vpcmpub k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e 6a 80 7b[ 	]*vpcmpub k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 0f 3e aa f0 f7 ff ff 7b[ 	]*vpcmpub k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e ed ab[ 	]*vpcmpub k5\{k7\},ymm6,ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e ed 7b[ 	]*vpcmpub k5\{k7\},ymm6,ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e 29 7b[ 	]*vpcmpub k5\{k7\},ymm6,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpub k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e 6a 7f 7b[ 	]*vpcmpub k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e aa 00 10 00 00 7b[ 	]*vpcmpub k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e 6a 80 7b[ 	]*vpcmpub k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 2f 3e aa e0 ef ff ff 7b[ 	]*vpcmpub k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e ed ab[ 	]*vpcmpuw k5\{k7\},xmm6,xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e ed 7b[ 	]*vpcmpuw k5\{k7\},xmm6,xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e 29 7b[ 	]*vpcmpuw k5\{k7\},xmm6,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpuw k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e 6a 7f 7b[ 	]*vpcmpuw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e aa 00 08 00 00 7b[ 	]*vpcmpuw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e 6a 80 7b[ 	]*vpcmpuw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 0f 3e aa f0 f7 ff ff 7b[ 	]*vpcmpuw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e ed ab[ 	]*vpcmpuw k5\{k7\},ymm6,ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e ed 7b[ 	]*vpcmpuw k5\{k7\},ymm6,ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e 29 7b[ 	]*vpcmpuw k5\{k7\},ymm6,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e ac f4 c0 1d fe ff 7b[ 	]*vpcmpuw k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e 6a 7f 7b[ 	]*vpcmpuw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e aa 00 10 00 00 7b[ 	]*vpcmpuw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e 6a 80 7b[ 	]*vpcmpuw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 cd 2f 3e aa e0 ef ff ff 7b[ 	]*vpcmpuw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\],0x7b
#pass
