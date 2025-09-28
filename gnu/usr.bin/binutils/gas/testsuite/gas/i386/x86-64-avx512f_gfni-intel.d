#as:
#objdump: -dw -Mintel
#name: x86_64 AVX512F/GFNI insns (Intel disassembly)
#source: x86-64-avx512f_gfni.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 ce f4 ab[ 	]*vgf2p8affineqb zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 ce f4 ab[ 	]*vgf2p8affineqb zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 ce f4 ab[ 	]*vgf2p8affineqb zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 ce b4 f0 23 01 00 00 7b[ 	]*vgf2p8affineqb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 ce 72 7f 7b[ 	]*vgf2p8affineqb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 ce 72 7f 7b[ 	]*vgf2p8affineqb zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 cf f4 ab[ 	]*vgf2p8affineinvqb zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 cf f4 ab[ 	]*vgf2p8affineinvqb zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 cf f4 ab[ 	]*vgf2p8affineinvqb zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 cf b4 f0 23 01 00 00 7b[ 	]*vgf2p8affineinvqb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 cf 72 7f 7b[ 	]*vgf2p8affineinvqb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 cf 72 7f 7b[ 	]*vgf2p8affineinvqb zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 cf f4[ 	]*vgf2p8mulb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 cf f4[ 	]*vgf2p8mulb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 cf f4[ 	]*vgf2p8mulb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 cf b4 f0 23 01 00 00[ 	]*vgf2p8mulb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x123\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 cf 72 7f[ 	]*vgf2p8mulb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 ce f4 ab[ 	]*vgf2p8affineqb zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 ce f4 ab[ 	]*vgf2p8affineqb zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 ce f4 ab[ 	]*vgf2p8affineqb zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 ce b4 f0 34 12 00 00 7b[ 	]*vgf2p8affineqb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 ce 72 7f 7b[ 	]*vgf2p8affineqb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 ce 72 7f 7b[ 	]*vgf2p8affineqb zmm30,zmm29,QWORD BCST \[rdx\+0x3f8\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 03 95 40 cf f4 ab[ 	]*vgf2p8affineinvqb zmm30,zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 47 cf f4 ab[ 	]*vgf2p8affineinvqb zmm30\{k7\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 03 95 c7 cf f4 ab[ 	]*vgf2p8affineinvqb zmm30\{k7\}\{z\},zmm29,zmm28,0xab
[ 	]*[a-f0-9]+:[ 	]*62 23 95 40 cf b4 f0 34 12 00 00 7b[ 	]*vgf2p8affineinvqb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 40 cf 72 7f 7b[ 	]*vgf2p8affineinvqb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 63 95 50 cf b2 00 04 00 00 7b[ 	]*vgf2p8affineinvqb zmm30,zmm29,QWORD BCST \[rdx\+0x400\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 02 15 40 cf f4[ 	]*vgf2p8mulb zmm30,zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 47 cf f4[ 	]*vgf2p8mulb zmm30\{k7\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 02 15 c7 cf f4[ 	]*vgf2p8mulb zmm30\{k7\}\{z\},zmm29,zmm28
[ 	]*[a-f0-9]+:[ 	]*62 22 15 40 cf b4 f0 34 12 00 00[ 	]*vgf2p8mulb zmm30,zmm29,ZMMWORD PTR \[rax\+r14\*8\+0x1234\]
[ 	]*[a-f0-9]+:[ 	]*62 62 15 40 cf 72 7f[ 	]*vgf2p8mulb zmm30,zmm29,ZMMWORD PTR \[rdx\+0x1fc0\]
#pass
