#as:
#objdump: -dw -Mintel
#name: i386 AVX512VL/VAES insns (Intel disassembly)
#source: avx512vl_vaes.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 de f4[ 	]*vaesdec xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 de b4 f4 c0 1d fe ff[ 	]*vaesdec xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 de b2 f0 07 00 00[ 	]*vaesdec xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 de f4[ 	]*vaesdec ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 de b4 f4 c0 1d fe ff[ 	]*vaesdec ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 de b2 e0 0f 00 00[ 	]*vaesdec ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 df f4[ 	]*vaesdeclast xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 df b4 f4 c0 1d fe ff[ 	]*vaesdeclast xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 df b2 f0 07 00 00[ 	]*vaesdeclast xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 df f4[ 	]*vaesdeclast ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 df b4 f4 c0 1d fe ff[ 	]*vaesdeclast ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 df b2 e0 0f 00 00[ 	]*vaesdeclast ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dc f4[ 	]*vaesenc xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dc b4 f4 c0 1d fe ff[ 	]*vaesenc xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dc b2 f0 07 00 00[ 	]*vaesenc xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dc f4[ 	]*vaesenc ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dc b4 f4 c0 1d fe ff[ 	]*vaesenc ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dc b2 e0 0f 00 00[ 	]*vaesenc ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dd f4[ 	]*vaesenclast xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dd b4 f4 c0 1d fe ff[ 	]*vaesenclast xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dd b2 f0 07 00 00[ 	]*vaesenclast xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dd f4[ 	]*vaesenclast ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dd b4 f4 c0 1d fe ff[ 	]*vaesenclast ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dd b2 e0 0f 00 00[ 	]*vaesenclast ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 de f4[ 	]*\{evex\} vaesdec xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 de b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdec xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 de 72 7f[ 	]*\{evex\} vaesdec xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 de f4[ 	]*\{evex\} vaesdec ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 de b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdec ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 de 72 7f[ 	]*\{evex\} vaesdec ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 df f4[ 	]*\{evex\} vaesdeclast xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 df b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdeclast xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 df 72 7f[ 	]*\{evex\} vaesdeclast xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 df f4[ 	]*\{evex\} vaesdeclast ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 df b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdeclast ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 df 72 7f[ 	]*\{evex\} vaesdeclast ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 dc f4[ 	]*\{evex\} vaesenc xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 dc b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenc xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 dc 72 7f[ 	]*\{evex\} vaesenc xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 dc f4[ 	]*\{evex\} vaesenc ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 dc b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenc ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 dc 72 7f[ 	]*\{evex\} vaesenc ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 dd f4[ 	]*\{evex\} vaesenclast xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 dd b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenclast xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 dd 72 7f[ 	]*\{evex\} vaesenclast xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 dd f4[ 	]*\{evex\} vaesenclast ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 dd b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenclast ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 dd 72 7f[ 	]*\{evex\} vaesenclast ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 de f4[ 	]*vaesdec xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 de b4 f4 c0 1d fe ff[ 	]*vaesdec xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 de b2 f0 07 00 00[ 	]*vaesdec xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 de f4[ 	]*vaesdec ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 de b4 f4 c0 1d fe ff[ 	]*vaesdec ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 de b2 e0 0f 00 00[ 	]*vaesdec ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 df f4[ 	]*vaesdeclast xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 df b4 f4 c0 1d fe ff[ 	]*vaesdeclast xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 df b2 f0 07 00 00[ 	]*vaesdeclast xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 df f4[ 	]*vaesdeclast ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 df b4 f4 c0 1d fe ff[ 	]*vaesdeclast ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 df b2 e0 0f 00 00[ 	]*vaesdeclast ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dc f4[ 	]*vaesenc xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dc b4 f4 c0 1d fe ff[ 	]*vaesenc xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dc b2 f0 07 00 00[ 	]*vaesenc xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dc f4[ 	]*vaesenc ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dc b4 f4 c0 1d fe ff[ 	]*vaesenc ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dc b2 e0 0f 00 00[ 	]*vaesenc ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dd f4[ 	]*vaesenclast xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dd b4 f4 c0 1d fe ff[ 	]*vaesenclast xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 dd b2 f0 07 00 00[ 	]*vaesenclast xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dd f4[ 	]*vaesenclast ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dd b4 f4 c0 1d fe ff[ 	]*vaesenclast ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 dd b2 e0 0f 00 00[ 	]*vaesenclast ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 de f4[ 	]*\{evex\} vaesdec xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 de b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdec xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 de 72 7f[ 	]*\{evex\} vaesdec xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 de f4[ 	]*\{evex\} vaesdec ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 de b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdec ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 de 72 7f[ 	]*\{evex\} vaesdec ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 df f4[ 	]*\{evex\} vaesdeclast xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 df b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdeclast xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 df 72 7f[ 	]*\{evex\} vaesdeclast xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 df f4[ 	]*\{evex\} vaesdeclast ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 df b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesdeclast ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 df 72 7f[ 	]*\{evex\} vaesdeclast ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 dc f4[ 	]*\{evex\} vaesenc xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 dc b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenc xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 dc 72 7f[ 	]*\{evex\} vaesenc xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 dc f4[ 	]*\{evex\} vaesenc ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 dc b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenc ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 dc 72 7f[ 	]*\{evex\} vaesenc ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 dd f4[ 	]*\{evex\} vaesenclast xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 dd b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenclast xmm6,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 08 dd 72 7f[ 	]*\{evex\} vaesenclast xmm6,xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 dd f4[ 	]*\{evex\} vaesenclast ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 dd b4 f4 c0 1d fe ff[ 	]*\{evex\} vaesenclast ymm6,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 28 dd 72 7f[ 	]*\{evex\} vaesenclast ymm6,ymm5,YMMWORD PTR \[edx\+0xfe0\]
#pass
