#as:
#objdump: -dw -Mintel
#name: i386 AVX512F/GFNI insns (Intel disassembly)
#source: avx512f_gfni.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 ce f4 ab[ 	]*vgf2p8affineqb zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f ce f4 ab[ 	]*vgf2p8affineqb zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf ce f4 ab[ 	]*vgf2p8affineqb zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 ce b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineqb zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 ce 72 7f 7b[ 	]*vgf2p8affineqb zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 ce 72 7f 7b[ 	]*vgf2p8affineqb zmm6,zmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 cf f4 ab[ 	]*vgf2p8affineinvqb zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f cf f4 ab[ 	]*vgf2p8affineinvqb zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf cf f4 ab[ 	]*vgf2p8affineinvqb zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 cf b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 cf 72 7f 7b[ 	]*vgf2p8affineinvqb zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 cf 72 7f 7b[ 	]*vgf2p8affineinvqb zmm6,zmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 cf f4[ 	]*vgf2p8mulb zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f cf f4[ 	]*vgf2p8mulb zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf cf f4[ 	]*vgf2p8mulb zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 cf b4 f4 c0 1d fe ff[ 	]*vgf2p8mulb zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 cf 72 7f[ 	]*vgf2p8mulb zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 ce f4 ab[ 	]*vgf2p8affineqb zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f ce f4 ab[ 	]*vgf2p8affineqb zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf ce f4 ab[ 	]*vgf2p8affineqb zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 ce b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineqb zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 ce 72 7f 7b[ 	]*vgf2p8affineqb zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 ce 72 7f 7b[ 	]*vgf2p8affineqb zmm6,zmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 cf f4 ab[ 	]*vgf2p8affineinvqb zmm6,zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 4f cf f4 ab[ 	]*vgf2p8affineinvqb zmm6\{k7\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 cf cf f4 ab[ 	]*vgf2p8affineinvqb zmm6\{k7\}\{z\},zmm5,zmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 cf b4 f4 c0 1d fe ff 7b[ 	]*vgf2p8affineinvqb zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 48 cf 72 7f 7b[ 	]*vgf2p8affineinvqb zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 58 cf 72 7f 7b[ 	]*vgf2p8affineinvqb zmm6,zmm5,QWORD BCST \[edx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 cf f4[ 	]*vgf2p8mulb zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 4f cf f4[ 	]*vgf2p8mulb zmm6\{k7\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 cf cf f4[ 	]*vgf2p8mulb zmm6\{k7\}\{z\},zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 cf b4 f4 c0 1d fe ff[ 	]*vgf2p8mulb zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 cf 72 7f[ 	]*vgf2p8mulb zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
#pass
