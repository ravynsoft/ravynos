#as:
#objdump: -dw -Mintel
#name: x86_64 AVX/GFNI insns (Intel disassembly)
#source: x86-64-avx_gfni.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 cf f4[ 	]*vgf2p8mulb ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*c4 a2 55 cf b4 f0 c0 1d fe ff[ 	]*vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rax\+r14\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 cf 72 7e[ 	]*vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 ce f4 ab[ 	]*vgf2p8affineqb ymm6,ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d5 ce b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 ce 72 7e 7b[ 	]*vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rdx\+0x7e\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 cf f4 ab[ 	]*vgf2p8affineinvqb ymm6,ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d5 cf b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 cf 72 7e 7b[ 	]*vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rdx\+0x7e\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 cf f4[ 	]*vgf2p8mulb xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*c4 a2 51 cf b4 f0 c0 1d fe ff[ 	]*vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rax\+r14\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 cf 72 7e[ 	]*vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 ce f4 ab[ 	]*vgf2p8affineqb xmm6,xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d1 ce b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 ce 72 7e 7b[ 	]*vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rdx\+0x7e\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 cf f4 ab[ 	]*vgf2p8affineinvqb xmm6,xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d1 cf b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 cf 72 7e 7b[ 	]*vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rdx\+0x7e\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 cf f4[ 	]*vgf2p8mulb ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*c4 a2 55 cf b4 f0 c0 1d fe ff[ 	]*vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rax\+r14\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 55 cf 72 7e[ 	]*vgf2p8mulb ymm6,ymm5,YMMWORD PTR \[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 ce f4 ab[ 	]*vgf2p8affineqb ymm6,ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d5 ce b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 ce 72 7e 7b[ 	]*vgf2p8affineqb ymm6,ymm5,YMMWORD PTR \[rdx\+0x7e\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 cf f4 ab[ 	]*vgf2p8affineinvqb ymm6,ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d5 cf b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d5 cf 72 7e 7b[ 	]*vgf2p8affineinvqb ymm6,ymm5,YMMWORD PTR \[rdx\+0x7e\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 cf f4[ 	]*vgf2p8mulb xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*c4 a2 51 cf b4 f0 c0 1d fe ff[ 	]*vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rax\+r14\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*c4 e2 51 cf 72 7e[ 	]*vgf2p8mulb xmm6,xmm5,XMMWORD PTR \[rdx\+0x7e\]
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 ce f4 ab[ 	]*vgf2p8affineqb xmm6,xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d1 ce b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 ce 72 7e 7b[ 	]*vgf2p8affineqb xmm6,xmm5,XMMWORD PTR \[rdx\+0x7e\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 cf f4 ab[ 	]*vgf2p8affineinvqb xmm6,xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 a3 d1 cf b4 f0 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rax\+r14\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 d1 cf 72 7e 7b[ 	]*vgf2p8affineinvqb xmm6,xmm5,XMMWORD PTR \[rdx\+0x7e\],0x7b
#pass
