#as:
#objdump: -dw -Mintel
#name: i386 AVX512VL/GFNI insns (Intel disassembly)
#source: avx512vl_gfni.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f ce f4 ab[ 	]*vgf2p8affineqb xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f ce f4 ab[ 	]*vgf2p8affineqb xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f ce f4 7b[ 	]*vgf2p8affineqb xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f ce b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineqb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f ce 72 7f 7b[ 	]*vgf2p8affineqb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f ce 72 7f 7b[ 	]*vgf2p8affineqb xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f ce f4 ab[ 	]*vgf2p8affineqb ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af ce f4 ab[ 	]*vgf2p8affineqb ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f ce f4 7b[ 	]*vgf2p8affineqb ymm6\{k7\},ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f ce b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineqb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f ce 72 7f 7b[ 	]*vgf2p8affineqb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f ce 72 7f 7b[ 	]*vgf2p8affineqb ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f cf f4 ab[ 	]*vgf2p8affineinvqb xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f cf f4 ab[ 	]*vgf2p8affineinvqb xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f cf f4 7b[ 	]*vgf2p8affineinvqb xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f cf b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f cf 72 7f 7b[ 	]*vgf2p8affineinvqb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f cf 72 7f 7b[ 	]*vgf2p8affineinvqb xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f cf f4 ab[ 	]*vgf2p8affineinvqb ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af cf f4 ab[ 	]*vgf2p8affineinvqb ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f cf f4 7b[ 	]*vgf2p8affineinvqb ymm6\{k7\},ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f cf b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f cf 72 7f 7b[ 	]*vgf2p8affineinvqb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f cf 72 7f 7b[ 	]*vgf2p8affineinvqb ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f cf f4[ 	]*vgf2p8mulb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f cf f4[ 	]*vgf2p8mulb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f cf b4 f4 c0 1d fe ff[ 	]*vgf2p8mulb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f cf 72 7f[ 	]*vgf2p8mulb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f cf f4[ 	]*vgf2p8mulb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af cf f4[ 	]*vgf2p8mulb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f cf b4 f4 c0 1d fe ff[ 	]*vgf2p8mulb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f cf 72 7f[ 	]*vgf2p8mulb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f ce f4 ab[ 	]*vgf2p8affineqb xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f ce f4 ab[ 	]*vgf2p8affineqb xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f ce b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineqb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f ce 72 7f 7b[ 	]*vgf2p8affineqb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f ce 72 7f 7b[ 	]*vgf2p8affineqb xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f ce f4 ab[ 	]*vgf2p8affineqb ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af ce f4 ab[ 	]*vgf2p8affineqb ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f ce b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineqb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f ce 72 7f 7b[ 	]*vgf2p8affineqb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f ce 72 7f 7b[ 	]*vgf2p8affineqb ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f cf f4 ab[ 	]*vgf2p8affineinvqb xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f cf f4 ab[ 	]*vgf2p8affineinvqb xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f cf b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f cf 72 7f 7b[ 	]*vgf2p8affineinvqb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 1f cf 72 7f 7b[ 	]*vgf2p8affineinvqb xmm6\{k7\},xmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f cf f4 ab[ 	]*vgf2p8affineinvqb ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af cf f4 ab[ 	]*vgf2p8affineinvqb ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f cf b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f cf 30 7b[ 	]*vgf2p8affineinvqb ymm6\{k7\},ymm5,QWORD BCST \[eax\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f cf 72 7f 7b[ 	]*vgf2p8affineinvqb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 3f cf 72 7f 7b[ 	]*vgf2p8affineinvqb ymm6\{k7\},ymm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f cf f4[ 	]*vgf2p8mulb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 8f cf f4[ 	]*vgf2p8mulb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f cf b4 f4 c0 1d fe ff[ 	]*vgf2p8mulb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 0f cf 72 7f[ 	]*vgf2p8mulb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f cf f4[ 	]*vgf2p8mulb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 af cf f4[ 	]*vgf2p8mulb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f cf b4 f4 c0 1d fe ff[ 	]*vgf2p8mulb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 2f cf 72 7f[ 	]*vgf2p8mulb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
#pass
