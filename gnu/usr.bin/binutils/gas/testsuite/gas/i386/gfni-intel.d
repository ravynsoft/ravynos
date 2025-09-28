#as:
#objdump: -dw -Mintel
#name: i386 GFNI insns (Intel disassembly)
#source: gfni.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*66 0f 38 cf ec[ 	]*gf2p8mulb xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*66 0f 38 cf ac f4 c0 1d fe ff[ 	]*gf2p8mulb xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*66 0f 38 cf aa f0 07 00 00[ 	]*gf2p8mulb xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a ce ec ab[ 	]*gf2p8affineqb xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a ce ac f4 c0 1d fe ff 7b[ 	]*gf2p8affineqb xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a ce aa f0 07 00 00 7b[ 	]*gf2p8affineqb xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a cf ec ab[ 	]*gf2p8affineinvqb xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a cf ac f4 c0 1d fe ff 7b[ 	]*gf2p8affineinvqb xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a cf aa f0 07 00 00 7b[ 	]*gf2p8affineinvqb xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*66 0f 38 cf ec[ 	]*gf2p8mulb xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*66 0f 38 cf ac f4 c0 1d fe ff[ 	]*gf2p8mulb xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*66 0f 38 cf aa f0 07 00 00[ 	]*gf2p8mulb xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a ce ec ab[ 	]*gf2p8affineqb xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a ce ac f4 c0 1d fe ff 7b[ 	]*gf2p8affineqb xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a ce aa f0 07 00 00 7b[ 	]*gf2p8affineqb xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a cf ec ab[ 	]*gf2p8affineinvqb xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a cf ac f4 c0 1d fe ff 7b[ 	]*gf2p8affineinvqb xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*66 0f 3a cf aa f0 07 00 00 7b[ 	]*gf2p8affineinvqb xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
#pass
