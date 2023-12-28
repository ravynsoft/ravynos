#as: -mevexwig=1
#objdump: -dw -Mintel
#name: i386 AVX512BW/VL wig insns (Intel disassembly)
#source: avx512bw_vl-wig.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1c f5[ 	]*vpabsb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 1c f5[ 	]*vpabsb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1c 31[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1c b4 f4 c0 1d fe ff[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1c 72 7f[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1c b2 00 08 00 00[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1c 72 80[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1c b2 f0 f7 ff ff[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1c f5[ 	]*vpabsb ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 1c f5[ 	]*vpabsb ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1c 31[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1c b4 f4 c0 1d fe ff[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1c 72 7f[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1c b2 00 10 00 00[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1c 72 80[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1c b2 e0 ef ff ff[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1d f5[ 	]*vpabsw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 1d f5[ 	]*vpabsw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1d 31[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1d b4 f4 c0 1d fe ff[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1d 72 7f[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1d b2 00 08 00 00[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1d 72 80[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1d b2 f0 f7 ff ff[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1d f5[ 	]*vpabsw ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 1d f5[ 	]*vpabsw ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1d 31[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1d b4 f4 c0 1d fe ff[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1d 72 7f[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1d b2 00 10 00 00[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1d 72 80[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1d b2 e0 ef ff ff[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 63 f4[ 	]*vpacksswb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 63 f4[ 	]*vpacksswb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 63 31[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 63 b4 f4 c0 1d fe ff[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 63 72 7f[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 63 b2 00 08 00 00[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 63 72 80[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 63 b2 f0 f7 ff ff[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 63 f4[ 	]*vpacksswb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 63 f4[ 	]*vpacksswb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 63 31[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 63 b4 f4 c0 1d fe ff[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 63 72 7f[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 63 b2 00 10 00 00[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 63 72 80[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 63 b2 e0 ef ff ff[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 67 f4[ 	]*vpackuswb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 67 f4[ 	]*vpackuswb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 67 31[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 67 b4 f4 c0 1d fe ff[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 67 72 7f[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 67 b2 00 08 00 00[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 67 72 80[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 67 b2 f0 f7 ff ff[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 67 f4[ 	]*vpackuswb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 67 f4[ 	]*vpackuswb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 67 31[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 67 b4 f4 c0 1d fe ff[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 67 72 7f[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 67 b2 00 10 00 00[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 67 72 80[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 67 b2 e0 ef ff ff[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fc f4[ 	]*vpaddb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f fc f4[ 	]*vpaddb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fc 31[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fc b4 f4 c0 1d fe ff[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fc 72 7f[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fc b2 00 08 00 00[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fc 72 80[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fc b2 f0 f7 ff ff[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fc f4[ 	]*vpaddb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af fc f4[ 	]*vpaddb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fc 31[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fc b4 f4 c0 1d fe ff[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fc 72 7f[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fc b2 00 10 00 00[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fc 72 80[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fc b2 e0 ef ff ff[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ec f4[ 	]*vpaddsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f ec f4[ 	]*vpaddsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ec 31[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ec b4 f4 c0 1d fe ff[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ec 72 7f[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ec b2 00 08 00 00[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ec 72 80[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ec b2 f0 f7 ff ff[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ec f4[ 	]*vpaddsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af ec f4[ 	]*vpaddsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ec 31[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ec b4 f4 c0 1d fe ff[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ec 72 7f[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ec b2 00 10 00 00[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ec 72 80[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ec b2 e0 ef ff ff[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ed f4[ 	]*vpaddsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f ed f4[ 	]*vpaddsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ed 31[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ed b4 f4 c0 1d fe ff[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ed 72 7f[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ed b2 00 08 00 00[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ed 72 80[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ed b2 f0 f7 ff ff[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ed f4[ 	]*vpaddsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af ed f4[ 	]*vpaddsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ed 31[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ed b4 f4 c0 1d fe ff[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ed 72 7f[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ed b2 00 10 00 00[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ed 72 80[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ed b2 e0 ef ff ff[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dc f4[ 	]*vpaddusb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f dc f4[ 	]*vpaddusb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dc 31[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dc b4 f4 c0 1d fe ff[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dc 72 7f[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dc b2 00 08 00 00[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dc 72 80[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dc b2 f0 f7 ff ff[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dc f4[ 	]*vpaddusb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af dc f4[ 	]*vpaddusb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dc 31[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dc b4 f4 c0 1d fe ff[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dc 72 7f[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dc b2 00 10 00 00[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dc 72 80[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dc b2 e0 ef ff ff[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dd f4[ 	]*vpaddusw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f dd f4[ 	]*vpaddusw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dd 31[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dd b4 f4 c0 1d fe ff[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dd 72 7f[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dd b2 00 08 00 00[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dd 72 80[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dd b2 f0 f7 ff ff[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dd f4[ 	]*vpaddusw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af dd f4[ 	]*vpaddusw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dd 31[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dd b4 f4 c0 1d fe ff[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dd 72 7f[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dd b2 00 10 00 00[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dd 72 80[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dd b2 e0 ef ff ff[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fd f4[ 	]*vpaddw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f fd f4[ 	]*vpaddw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fd 31[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fd b4 f4 c0 1d fe ff[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fd 72 7f[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fd b2 00 08 00 00[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fd 72 80[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fd b2 f0 f7 ff ff[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fd f4[ 	]*vpaddw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af fd f4[ 	]*vpaddw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fd 31[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fd b4 f4 c0 1d fe ff[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fd 72 7f[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fd b2 00 10 00 00[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fd 72 80[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fd b2 e0 ef ff ff[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f f4 ab[ 	]*vpalignr xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 0f f4 ab[ 	]*vpalignr xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f f4 7b[ 	]*vpalignr xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f 31 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f b4 f4 c0 1d fe ff 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f 72 7f 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f b2 00 08 00 00 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f 72 80 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f b2 f0 f7 ff ff 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f f4 ab[ 	]*vpalignr ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 0f f4 ab[ 	]*vpalignr ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f f4 7b[ 	]*vpalignr ymm6\{k7\},ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f 31 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f b4 f4 c0 1d fe ff 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f 72 7f 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f b2 00 10 00 00 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f 72 80 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f b2 e0 ef ff ff 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e0 f4[ 	]*vpavgb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f e0 f4[ 	]*vpavgb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e0 31[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e0 b4 f4 c0 1d fe ff[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e0 72 7f[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e0 b2 00 08 00 00[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e0 72 80[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e0 b2 f0 f7 ff ff[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e0 f4[ 	]*vpavgb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af e0 f4[ 	]*vpavgb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e0 31[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e0 b4 f4 c0 1d fe ff[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e0 72 7f[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e0 b2 00 10 00 00[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e0 72 80[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e0 b2 e0 ef ff ff[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e3 f4[ 	]*vpavgw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f e3 f4[ 	]*vpavgw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e3 31[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e3 b4 f4 c0 1d fe ff[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e3 72 7f[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e3 b2 00 08 00 00[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e3 72 80[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e3 b2 f0 f7 ff ff[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e3 f4[ 	]*vpavgw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af e3 f4[ 	]*vpavgw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e3 31[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e3 b4 f4 c0 1d fe ff[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e3 72 7f[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e3 b2 00 10 00 00[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e3 72 80[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e3 b2 e0 ef ff ff[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 74 ed[ 	]*vpcmpeqb k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 74 29[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 74 ac f4 c0 1d fe ff[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 74 6a 7f[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 74 aa 00 08 00 00[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 74 6a 80[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 74 aa f0 f7 ff ff[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 74 ed[ 	]*vpcmpeqb k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 74 29[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 74 ac f4 c0 1d fe ff[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 74 6a 7f[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 74 aa 00 10 00 00[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 74 6a 80[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 74 aa e0 ef ff ff[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 75 ed[ 	]*vpcmpeqw k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 75 29[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 75 ac f4 c0 1d fe ff[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 75 6a 7f[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 75 aa 00 08 00 00[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 75 6a 80[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 75 aa f0 f7 ff ff[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 75 ed[ 	]*vpcmpeqw k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 75 29[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 75 ac f4 c0 1d fe ff[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 75 6a 7f[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 75 aa 00 10 00 00[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 75 6a 80[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 75 aa e0 ef ff ff[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 64 ed[ 	]*vpcmpgtb k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 64 29[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 64 ac f4 c0 1d fe ff[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 64 6a 7f[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 64 aa 00 08 00 00[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 64 6a 80[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 64 aa f0 f7 ff ff[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 64 ed[ 	]*vpcmpgtb k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 64 29[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 64 ac f4 c0 1d fe ff[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 64 6a 7f[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 64 aa 00 10 00 00[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 64 6a 80[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 64 aa e0 ef ff ff[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 65 ed[ 	]*vpcmpgtw k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 65 29[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 65 ac f4 c0 1d fe ff[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 65 6a 7f[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 65 aa 00 08 00 00[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 65 6a 80[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 65 aa f0 f7 ff ff[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 65 ed[ 	]*vpcmpgtw k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 65 29[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 65 ac f4 c0 1d fe ff[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 65 6a 7f[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 65 aa 00 10 00 00[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 65 6a 80[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 65 aa e0 ef ff ff[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 04 f4[ 	]*vpmaddubsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 04 f4[ 	]*vpmaddubsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 04 31[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 04 b4 f4 c0 1d fe ff[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 04 72 7f[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 04 b2 00 08 00 00[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 04 72 80[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 04 b2 f0 f7 ff ff[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 04 f4[ 	]*vpmaddubsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 04 f4[ 	]*vpmaddubsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 04 31[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 04 b4 f4 c0 1d fe ff[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 04 72 7f[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 04 b2 00 10 00 00[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 04 72 80[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 04 b2 e0 ef ff ff[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f5 f4[ 	]*vpmaddwd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f f5 f4[ 	]*vpmaddwd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f5 31[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f5 b4 f4 c0 1d fe ff[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f5 72 7f[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f5 b2 00 08 00 00[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f5 72 80[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f5 b2 f0 f7 ff ff[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f5 f4[ 	]*vpmaddwd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af f5 f4[ 	]*vpmaddwd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f5 31[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f5 b4 f4 c0 1d fe ff[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f5 72 7f[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f5 b2 00 10 00 00[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f5 72 80[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f5 b2 e0 ef ff ff[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3c f4[ 	]*vpmaxsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 3c f4[ 	]*vpmaxsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3c 31[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3c b4 f4 c0 1d fe ff[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3c 72 7f[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3c b2 00 08 00 00[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3c 72 80[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3c b2 f0 f7 ff ff[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3c f4[ 	]*vpmaxsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 3c f4[ 	]*vpmaxsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3c 31[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3c b4 f4 c0 1d fe ff[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3c 72 7f[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3c b2 00 10 00 00[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3c 72 80[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3c b2 e0 ef ff ff[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ee f4[ 	]*vpmaxsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f ee f4[ 	]*vpmaxsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ee 31[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ee b4 f4 c0 1d fe ff[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ee 72 7f[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ee b2 00 08 00 00[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ee 72 80[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ee b2 f0 f7 ff ff[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ee f4[ 	]*vpmaxsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af ee f4[ 	]*vpmaxsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ee 31[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ee b4 f4 c0 1d fe ff[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ee 72 7f[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ee b2 00 10 00 00[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ee 72 80[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ee b2 e0 ef ff ff[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f de f4[ 	]*vpmaxub xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f de f4[ 	]*vpmaxub xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f de 31[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f de b4 f4 c0 1d fe ff[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f de 72 7f[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f de b2 00 08 00 00[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f de 72 80[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f de b2 f0 f7 ff ff[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f de f4[ 	]*vpmaxub ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af de f4[ 	]*vpmaxub ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f de 31[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f de b4 f4 c0 1d fe ff[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f de 72 7f[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f de b2 00 10 00 00[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f de 72 80[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f de b2 e0 ef ff ff[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3e f4[ 	]*vpmaxuw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 3e f4[ 	]*vpmaxuw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3e 31[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3e b4 f4 c0 1d fe ff[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3e 72 7f[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3e b2 00 08 00 00[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3e 72 80[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3e b2 f0 f7 ff ff[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3e f4[ 	]*vpmaxuw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 3e f4[ 	]*vpmaxuw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3e 31[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3e b4 f4 c0 1d fe ff[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3e 72 7f[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3e b2 00 10 00 00[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3e 72 80[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3e b2 e0 ef ff ff[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 38 f4[ 	]*vpminsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 38 f4[ 	]*vpminsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 38 31[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 38 b4 f4 c0 1d fe ff[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 38 72 7f[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 38 b2 00 08 00 00[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 38 72 80[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 38 b2 f0 f7 ff ff[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 38 f4[ 	]*vpminsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 38 f4[ 	]*vpminsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 38 31[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 38 b4 f4 c0 1d fe ff[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 38 72 7f[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 38 b2 00 10 00 00[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 38 72 80[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 38 b2 e0 ef ff ff[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ea f4[ 	]*vpminsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f ea f4[ 	]*vpminsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ea 31[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ea b4 f4 c0 1d fe ff[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ea 72 7f[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ea b2 00 08 00 00[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ea 72 80[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ea b2 f0 f7 ff ff[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ea f4[ 	]*vpminsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af ea f4[ 	]*vpminsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ea 31[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ea b4 f4 c0 1d fe ff[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ea 72 7f[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ea b2 00 10 00 00[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ea 72 80[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ea b2 e0 ef ff ff[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f da f4[ 	]*vpminub xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f da f4[ 	]*vpminub xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f da 31[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f da b4 f4 c0 1d fe ff[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f da 72 7f[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f da b2 00 08 00 00[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f da 72 80[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f da b2 f0 f7 ff ff[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f da f4[ 	]*vpminub ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af da f4[ 	]*vpminub ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f da 31[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f da b4 f4 c0 1d fe ff[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f da 72 7f[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f da b2 00 10 00 00[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f da 72 80[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f da b2 e0 ef ff ff[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3a f4[ 	]*vpminuw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 3a f4[ 	]*vpminuw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3a 31[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3a b4 f4 c0 1d fe ff[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3a 72 7f[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3a b2 00 08 00 00[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3a 72 80[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3a b2 f0 f7 ff ff[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3a f4[ 	]*vpminuw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 3a f4[ 	]*vpminuw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3a 31[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3a b4 f4 c0 1d fe ff[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3a 72 7f[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3a b2 00 10 00 00[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3a 72 80[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3a b2 e0 ef ff ff[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 20 f5[ 	]*vpmovsxbw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 20 f5[ 	]*vpmovsxbw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 20 31[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 20 b4 f4 c0 1d fe ff[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 20 72 7f[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 20 b2 00 04 00 00[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 20 72 80[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 20 b2 f8 fb ff ff[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 20 f5[ 	]*vpmovsxbw ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 20 f5[ 	]*vpmovsxbw ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 20 31[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 20 b4 f4 c0 1d fe ff[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 20 72 7f[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 20 b2 00 08 00 00[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 20 72 80[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 20 b2 f0 f7 ff ff[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 30 f5[ 	]*vpmovzxbw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 30 f5[ 	]*vpmovzxbw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 30 31[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 30 b4 f4 c0 1d fe ff[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 30 72 7f[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 30 b2 00 04 00 00[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 30 72 80[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 30 b2 f8 fb ff ff[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 30 f5[ 	]*vpmovzxbw ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 30 f5[ 	]*vpmovzxbw ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 30 31[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 30 b4 f4 c0 1d fe ff[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 30 72 7f[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 30 b2 00 08 00 00[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 30 72 80[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 30 b2 f0 f7 ff ff[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 0b f4[ 	]*vpmulhrsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 0b f4[ 	]*vpmulhrsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 0b 31[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 0b b4 f4 c0 1d fe ff[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 0b 72 7f[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 0b b2 00 08 00 00[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 0b 72 80[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 0b b2 f0 f7 ff ff[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 0b f4[ 	]*vpmulhrsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 0b f4[ 	]*vpmulhrsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 0b 31[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 0b b4 f4 c0 1d fe ff[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 0b 72 7f[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 0b b2 00 10 00 00[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 0b 72 80[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 0b b2 e0 ef ff ff[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e4 f4[ 	]*vpmulhuw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f e4 f4[ 	]*vpmulhuw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e4 31[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e4 b4 f4 c0 1d fe ff[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e4 72 7f[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e4 b2 00 08 00 00[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e4 72 80[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e4 b2 f0 f7 ff ff[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e4 f4[ 	]*vpmulhuw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af e4 f4[ 	]*vpmulhuw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e4 31[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e4 b4 f4 c0 1d fe ff[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e4 72 7f[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e4 b2 00 10 00 00[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e4 72 80[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e4 b2 e0 ef ff ff[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e5 f4[ 	]*vpmulhw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f e5 f4[ 	]*vpmulhw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e5 31[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e5 b4 f4 c0 1d fe ff[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e5 72 7f[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e5 b2 00 08 00 00[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e5 72 80[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e5 b2 f0 f7 ff ff[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e5 f4[ 	]*vpmulhw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af e5 f4[ 	]*vpmulhw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e5 31[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e5 b4 f4 c0 1d fe ff[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e5 72 7f[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e5 b2 00 10 00 00[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e5 72 80[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e5 b2 e0 ef ff ff[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d5 f4[ 	]*vpmullw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f d5 f4[ 	]*vpmullw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d5 31[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d5 b4 f4 c0 1d fe ff[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d5 72 7f[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d5 b2 00 08 00 00[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d5 72 80[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d5 b2 f0 f7 ff ff[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d5 f4[ 	]*vpmullw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af d5 f4[ 	]*vpmullw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d5 31[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d5 b4 f4 c0 1d fe ff[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d5 72 7f[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d5 b2 00 10 00 00[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d5 72 80[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d5 b2 e0 ef ff ff[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 00 f4[ 	]*vpshufb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 00 f4[ 	]*vpshufb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 00 31[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 00 b4 f4 c0 1d fe ff[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 00 72 7f[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 00 b2 00 08 00 00[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 00 72 80[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 00 b2 f0 f7 ff ff[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 00 f4[ 	]*vpshufb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 00 f4[ 	]*vpshufb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 00 31[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 00 b4 f4 c0 1d fe ff[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 00 72 7f[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 00 b2 00 10 00 00[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 00 72 80[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 00 b2 e0 ef ff ff[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 f5 ab[ 	]*vpshufhw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 8f 70 f5 ab[ 	]*vpshufhw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 f5 7b[ 	]*vpshufhw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 31 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 72 7f 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 b2 00 08 00 00 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 72 80 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 b2 f0 f7 ff ff 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 f5 ab[ 	]*vpshufhw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe af 70 f5 ab[ 	]*vpshufhw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 f5 7b[ 	]*vpshufhw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 31 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 72 7f 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 b2 00 10 00 00 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 72 80 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 b2 e0 ef ff ff 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 f5 ab[ 	]*vpshuflw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 70 f5 ab[ 	]*vpshuflw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 f5 7b[ 	]*vpshuflw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 31 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 72 7f 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 b2 00 08 00 00 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 72 80 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 b2 f0 f7 ff ff 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 f5 ab[ 	]*vpshuflw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 70 f5 ab[ 	]*vpshuflw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 f5 7b[ 	]*vpshuflw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 31 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 72 7f 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 b2 00 10 00 00 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 72 80 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 b2 e0 ef ff ff 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f1 f4[ 	]*vpsllw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f f1 f4[ 	]*vpsllw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f1 31[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f1 b4 f4 c0 1d fe ff[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f1 72 7f[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f1 b2 00 08 00 00[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f1 72 80[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f1 b2 f0 f7 ff ff[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f1 f4[ 	]*vpsllw ymm6\{k7\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af f1 f4[ 	]*vpsllw ymm6\{k7\}\{z\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f1 31[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f1 b4 f4 c0 1d fe ff[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f1 72 7f[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f1 b2 00 08 00 00[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f1 72 80[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f1 b2 f0 f7 ff ff[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e1 f4[ 	]*vpsraw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f e1 f4[ 	]*vpsraw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e1 31[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e1 b4 f4 c0 1d fe ff[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e1 72 7f[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e1 b2 00 08 00 00[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e1 72 80[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e1 b2 f0 f7 ff ff[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e1 f4[ 	]*vpsraw ymm6\{k7\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af e1 f4[ 	]*vpsraw ymm6\{k7\}\{z\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e1 31[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e1 b4 f4 c0 1d fe ff[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e1 72 7f[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e1 b2 00 08 00 00[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e1 72 80[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e1 b2 f0 f7 ff ff[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d1 f4[ 	]*vpsrlw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f d1 f4[ 	]*vpsrlw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d1 31[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d1 b4 f4 c0 1d fe ff[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d1 72 7f[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d1 b2 00 08 00 00[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d1 72 80[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d1 b2 f0 f7 ff ff[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d1 f4[ 	]*vpsrlw ymm6\{k7\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af d1 f4[ 	]*vpsrlw ymm6\{k7\}\{z\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d1 31[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d1 b4 f4 c0 1d fe ff[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d1 72 7f[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d1 b2 00 08 00 00[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d1 72 80[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d1 b2 f0 f7 ff ff[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 d5 ab[ 	]*vpsrlw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 8f 71 d5 ab[ 	]*vpsrlw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 d5 7b[ 	]*vpsrlw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 11 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 94 f4 c0 1d fe ff 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 52 7f 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 92 00 08 00 00 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 52 80 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 92 f0 f7 ff ff 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 d5 ab[ 	]*vpsrlw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd af 71 d5 ab[ 	]*vpsrlw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 d5 7b[ 	]*vpsrlw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 11 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 94 f4 c0 1d fe ff 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 52 7f 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 92 00 10 00 00 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 52 80 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 92 e0 ef ff ff 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 e5 ab[ 	]*vpsraw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 8f 71 e5 ab[ 	]*vpsraw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 e5 7b[ 	]*vpsraw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 21 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 a4 f4 c0 1d fe ff 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 62 7f 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 a2 00 08 00 00 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 62 80 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 a2 f0 f7 ff ff 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 e5 ab[ 	]*vpsraw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd af 71 e5 ab[ 	]*vpsraw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 e5 7b[ 	]*vpsraw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 21 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 a4 f4 c0 1d fe ff 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 62 7f 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 a2 00 10 00 00 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 62 80 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 a2 e0 ef ff ff 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f8 f4[ 	]*vpsubb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f f8 f4[ 	]*vpsubb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f8 31[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f8 b4 f4 c0 1d fe ff[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f8 72 7f[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f8 b2 00 08 00 00[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f8 72 80[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f8 b2 f0 f7 ff ff[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f8 f4[ 	]*vpsubb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af f8 f4[ 	]*vpsubb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f8 31[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f8 b4 f4 c0 1d fe ff[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f8 72 7f[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f8 b2 00 10 00 00[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f8 72 80[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f8 b2 e0 ef ff ff[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e8 f4[ 	]*vpsubsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f e8 f4[ 	]*vpsubsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e8 31[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e8 b4 f4 c0 1d fe ff[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e8 72 7f[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e8 b2 00 08 00 00[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e8 72 80[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e8 b2 f0 f7 ff ff[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e8 f4[ 	]*vpsubsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af e8 f4[ 	]*vpsubsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e8 31[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e8 b4 f4 c0 1d fe ff[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e8 72 7f[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e8 b2 00 10 00 00[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e8 72 80[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e8 b2 e0 ef ff ff[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e9 f4[ 	]*vpsubsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f e9 f4[ 	]*vpsubsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e9 31[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e9 b4 f4 c0 1d fe ff[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e9 72 7f[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e9 b2 00 08 00 00[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e9 72 80[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e9 b2 f0 f7 ff ff[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e9 f4[ 	]*vpsubsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af e9 f4[ 	]*vpsubsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e9 31[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e9 b4 f4 c0 1d fe ff[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e9 72 7f[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e9 b2 00 10 00 00[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e9 72 80[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e9 b2 e0 ef ff ff[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d8 f4[ 	]*vpsubusb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f d8 f4[ 	]*vpsubusb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d8 31[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d8 b4 f4 c0 1d fe ff[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d8 72 7f[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d8 b2 00 08 00 00[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d8 72 80[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d8 b2 f0 f7 ff ff[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d8 f4[ 	]*vpsubusb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af d8 f4[ 	]*vpsubusb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d8 31[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d8 b4 f4 c0 1d fe ff[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d8 72 7f[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d8 b2 00 10 00 00[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d8 72 80[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d8 b2 e0 ef ff ff[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d9 f4[ 	]*vpsubusw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f d9 f4[ 	]*vpsubusw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d9 31[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d9 b4 f4 c0 1d fe ff[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d9 72 7f[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d9 b2 00 08 00 00[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d9 72 80[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d9 b2 f0 f7 ff ff[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d9 f4[ 	]*vpsubusw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af d9 f4[ 	]*vpsubusw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d9 31[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d9 b4 f4 c0 1d fe ff[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d9 72 7f[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d9 b2 00 10 00 00[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d9 72 80[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d9 b2 e0 ef ff ff[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f9 f4[ 	]*vpsubw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f f9 f4[ 	]*vpsubw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f9 31[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f9 b4 f4 c0 1d fe ff[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f9 72 7f[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f9 b2 00 08 00 00[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f9 72 80[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f9 b2 f0 f7 ff ff[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f9 f4[ 	]*vpsubw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af f9 f4[ 	]*vpsubw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f9 31[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f9 b4 f4 c0 1d fe ff[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f9 72 7f[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f9 b2 00 10 00 00[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f9 72 80[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f9 b2 e0 ef ff ff[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 68 f4[ 	]*vpunpckhbw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 68 f4[ 	]*vpunpckhbw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 68 31[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 68 b4 f4 c0 1d fe ff[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 68 72 7f[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 68 b2 00 08 00 00[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 68 72 80[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 68 b2 f0 f7 ff ff[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 68 f4[ 	]*vpunpckhbw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 68 f4[ 	]*vpunpckhbw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 68 31[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 68 b4 f4 c0 1d fe ff[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 68 72 7f[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 68 b2 00 10 00 00[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 68 72 80[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 68 b2 e0 ef ff ff[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 69 f4[ 	]*vpunpckhwd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 69 f4[ 	]*vpunpckhwd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 69 31[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 69 b4 f4 c0 1d fe ff[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 69 72 7f[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 69 b2 00 08 00 00[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 69 72 80[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 69 b2 f0 f7 ff ff[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 69 f4[ 	]*vpunpckhwd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 69 f4[ 	]*vpunpckhwd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 69 31[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 69 b4 f4 c0 1d fe ff[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 69 72 7f[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 69 b2 00 10 00 00[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 69 72 80[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 69 b2 e0 ef ff ff[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 60 f4[ 	]*vpunpcklbw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 60 f4[ 	]*vpunpcklbw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 60 31[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 60 b4 f4 c0 1d fe ff[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 60 72 7f[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 60 b2 00 08 00 00[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 60 72 80[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 60 b2 f0 f7 ff ff[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 60 f4[ 	]*vpunpcklbw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 60 f4[ 	]*vpunpcklbw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 60 31[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 60 b4 f4 c0 1d fe ff[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 60 72 7f[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 60 b2 00 10 00 00[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 60 72 80[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 60 b2 e0 ef ff ff[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 61 f4[ 	]*vpunpcklwd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 61 f4[ 	]*vpunpcklwd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 61 31[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 61 b4 f4 c0 1d fe ff[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 61 72 7f[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 61 b2 00 08 00 00[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 61 72 80[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 61 b2 f0 f7 ff ff[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 61 f4[ 	]*vpunpcklwd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 61 f4[ 	]*vpunpcklwd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 61 31[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 61 b4 f4 c0 1d fe ff[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 61 72 7f[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 61 b2 00 10 00 00[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 61 72 80[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 61 b2 e0 ef ff ff[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 f5 ab[ 	]*vpsllw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 8f 71 f5 ab[ 	]*vpsllw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 f5 7b[ 	]*vpsllw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 31 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 72 7f 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 b2 00 08 00 00 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 72 80 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 b2 f0 f7 ff ff 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 f5 ab[ 	]*vpsllw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd af 71 f5 ab[ 	]*vpsllw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 f5 7b[ 	]*vpsllw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 31 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 72 7f 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 b2 00 10 00 00 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 72 80 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 b2 e0 ef ff ff 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1c f5[ 	]*vpabsb xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 1c f5[ 	]*vpabsb xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1c 31[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1c b4 f4 c0 1d fe ff[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1c 72 7f[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1c b2 00 08 00 00[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1c 72 80[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1c b2 f0 f7 ff ff[ 	]*vpabsb xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1c f5[ 	]*vpabsb ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 1c f5[ 	]*vpabsb ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1c 31[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1c b4 f4 c0 1d fe ff[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1c 72 7f[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1c b2 00 10 00 00[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1c 72 80[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1c b2 e0 ef ff ff[ 	]*vpabsb ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1d f5[ 	]*vpabsw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 1d f5[ 	]*vpabsw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1d 31[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1d b4 f4 c0 1d fe ff[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1d 72 7f[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1d b2 00 08 00 00[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1d 72 80[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 1d b2 f0 f7 ff ff[ 	]*vpabsw xmm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1d f5[ 	]*vpabsw ymm6\{k7\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 1d f5[ 	]*vpabsw ymm6\{k7\}\{z\},ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1d 31[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1d b4 f4 c0 1d fe ff[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1d 72 7f[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1d b2 00 10 00 00[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1d 72 80[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 1d b2 e0 ef ff ff[ 	]*vpabsw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 63 f4[ 	]*vpacksswb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 63 f4[ 	]*vpacksswb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 63 31[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 63 b4 f4 c0 1d fe ff[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 63 72 7f[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 63 b2 00 08 00 00[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 63 72 80[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 63 b2 f0 f7 ff ff[ 	]*vpacksswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 63 f4[ 	]*vpacksswb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 63 f4[ 	]*vpacksswb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 63 31[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 63 b4 f4 c0 1d fe ff[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 63 72 7f[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 63 b2 00 10 00 00[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 63 72 80[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 63 b2 e0 ef ff ff[ 	]*vpacksswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 67 f4[ 	]*vpackuswb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 67 f4[ 	]*vpackuswb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 67 31[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 67 b4 f4 c0 1d fe ff[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 67 72 7f[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 67 b2 00 08 00 00[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 67 72 80[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 67 b2 f0 f7 ff ff[ 	]*vpackuswb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 67 f4[ 	]*vpackuswb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 67 f4[ 	]*vpackuswb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 67 31[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 67 b4 f4 c0 1d fe ff[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 67 72 7f[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 67 b2 00 10 00 00[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 67 72 80[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 67 b2 e0 ef ff ff[ 	]*vpackuswb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fc f4[ 	]*vpaddb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f fc f4[ 	]*vpaddb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fc 31[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fc b4 f4 c0 1d fe ff[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fc 72 7f[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fc b2 00 08 00 00[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fc 72 80[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fc b2 f0 f7 ff ff[ 	]*vpaddb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fc f4[ 	]*vpaddb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af fc f4[ 	]*vpaddb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fc 31[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fc b4 f4 c0 1d fe ff[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fc 72 7f[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fc b2 00 10 00 00[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fc 72 80[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fc b2 e0 ef ff ff[ 	]*vpaddb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ec f4[ 	]*vpaddsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f ec f4[ 	]*vpaddsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ec 31[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ec b4 f4 c0 1d fe ff[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ec 72 7f[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ec b2 00 08 00 00[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ec 72 80[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ec b2 f0 f7 ff ff[ 	]*vpaddsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ec f4[ 	]*vpaddsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af ec f4[ 	]*vpaddsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ec 31[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ec b4 f4 c0 1d fe ff[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ec 72 7f[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ec b2 00 10 00 00[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ec 72 80[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ec b2 e0 ef ff ff[ 	]*vpaddsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ed f4[ 	]*vpaddsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f ed f4[ 	]*vpaddsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ed 31[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ed b4 f4 c0 1d fe ff[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ed 72 7f[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ed b2 00 08 00 00[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ed 72 80[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ed b2 f0 f7 ff ff[ 	]*vpaddsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ed f4[ 	]*vpaddsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af ed f4[ 	]*vpaddsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ed 31[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ed b4 f4 c0 1d fe ff[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ed 72 7f[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ed b2 00 10 00 00[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ed 72 80[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ed b2 e0 ef ff ff[ 	]*vpaddsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dc f4[ 	]*vpaddusb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f dc f4[ 	]*vpaddusb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dc 31[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dc b4 f4 c0 1d fe ff[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dc 72 7f[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dc b2 00 08 00 00[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dc 72 80[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dc b2 f0 f7 ff ff[ 	]*vpaddusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dc f4[ 	]*vpaddusb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af dc f4[ 	]*vpaddusb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dc 31[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dc b4 f4 c0 1d fe ff[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dc 72 7f[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dc b2 00 10 00 00[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dc 72 80[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dc b2 e0 ef ff ff[ 	]*vpaddusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dd f4[ 	]*vpaddusw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f dd f4[ 	]*vpaddusw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dd 31[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dd b4 f4 c0 1d fe ff[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dd 72 7f[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dd b2 00 08 00 00[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dd 72 80[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f dd b2 f0 f7 ff ff[ 	]*vpaddusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dd f4[ 	]*vpaddusw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af dd f4[ 	]*vpaddusw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dd 31[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dd b4 f4 c0 1d fe ff[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dd 72 7f[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dd b2 00 10 00 00[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dd 72 80[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f dd b2 e0 ef ff ff[ 	]*vpaddusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fd f4[ 	]*vpaddw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f fd f4[ 	]*vpaddw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fd 31[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fd b4 f4 c0 1d fe ff[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fd 72 7f[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fd b2 00 08 00 00[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fd 72 80[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f fd b2 f0 f7 ff ff[ 	]*vpaddw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fd f4[ 	]*vpaddw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af fd f4[ 	]*vpaddw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fd 31[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fd b4 f4 c0 1d fe ff[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fd 72 7f[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fd b2 00 10 00 00[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fd 72 80[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f fd b2 e0 ef ff ff[ 	]*vpaddw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f f4 ab[ 	]*vpalignr xmm6\{k7\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 8f 0f f4 ab[ 	]*vpalignr xmm6\{k7\}\{z\},xmm5,xmm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f f4 7b[ 	]*vpalignr xmm6\{k7\},xmm5,xmm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f 31 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f b4 f4 c0 1d fe ff 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f 72 7f 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f b2 00 08 00 00 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f 72 80 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 0f 0f b2 f0 f7 ff ff 7b[ 	]*vpalignr xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f f4 ab[ 	]*vpalignr ymm6\{k7\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 af 0f f4 ab[ 	]*vpalignr ymm6\{k7\}\{z\},ymm5,ymm4,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f f4 7b[ 	]*vpalignr ymm6\{k7\},ymm5,ymm4,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f 31 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f b4 f4 c0 1d fe ff 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f 72 7f 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f b2 00 10 00 00 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f 72 80 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 d5 2f 0f b2 e0 ef ff ff 7b[ 	]*vpalignr ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e0 f4[ 	]*vpavgb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f e0 f4[ 	]*vpavgb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e0 31[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e0 b4 f4 c0 1d fe ff[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e0 72 7f[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e0 b2 00 08 00 00[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e0 72 80[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e0 b2 f0 f7 ff ff[ 	]*vpavgb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e0 f4[ 	]*vpavgb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af e0 f4[ 	]*vpavgb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e0 31[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e0 b4 f4 c0 1d fe ff[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e0 72 7f[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e0 b2 00 10 00 00[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e0 72 80[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e0 b2 e0 ef ff ff[ 	]*vpavgb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e3 f4[ 	]*vpavgw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f e3 f4[ 	]*vpavgw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e3 31[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e3 b4 f4 c0 1d fe ff[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e3 72 7f[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e3 b2 00 08 00 00[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e3 72 80[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e3 b2 f0 f7 ff ff[ 	]*vpavgw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e3 f4[ 	]*vpavgw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af e3 f4[ 	]*vpavgw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e3 31[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e3 b4 f4 c0 1d fe ff[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e3 72 7f[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e3 b2 00 10 00 00[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e3 72 80[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e3 b2 e0 ef ff ff[ 	]*vpavgw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 74 ed[ 	]*vpcmpeqb k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 74 29[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 74 ac f4 c0 1d fe ff[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 74 6a 7f[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 74 aa 00 08 00 00[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 74 6a 80[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 74 aa f0 f7 ff ff[ 	]*vpcmpeqb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 74 ed[ 	]*vpcmpeqb k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 74 29[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 74 ac f4 c0 1d fe ff[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 74 6a 7f[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 74 aa 00 10 00 00[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 74 6a 80[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 74 aa e0 ef ff ff[ 	]*vpcmpeqb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 75 ed[ 	]*vpcmpeqw k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 75 29[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 75 ac f4 c0 1d fe ff[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 75 6a 7f[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 75 aa 00 08 00 00[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 75 6a 80[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 75 aa f0 f7 ff ff[ 	]*vpcmpeqw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 75 ed[ 	]*vpcmpeqw k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 75 29[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 75 ac f4 c0 1d fe ff[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 75 6a 7f[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 75 aa 00 10 00 00[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 75 6a 80[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 75 aa e0 ef ff ff[ 	]*vpcmpeqw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 64 ed[ 	]*vpcmpgtb k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 64 29[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 64 ac f4 c0 1d fe ff[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 64 6a 7f[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 64 aa 00 08 00 00[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 64 6a 80[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 64 aa f0 f7 ff ff[ 	]*vpcmpgtb k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 64 ed[ 	]*vpcmpgtb k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 64 29[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 64 ac f4 c0 1d fe ff[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 64 6a 7f[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 64 aa 00 10 00 00[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 64 6a 80[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 64 aa e0 ef ff ff[ 	]*vpcmpgtb k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 65 ed[ 	]*vpcmpgtw k5\{k7\},xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 65 29[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 65 ac f4 c0 1d fe ff[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 65 6a 7f[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 65 aa 00 08 00 00[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 65 6a 80[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 65 aa f0 f7 ff ff[ 	]*vpcmpgtw k5\{k7\},xmm6,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 65 ed[ 	]*vpcmpgtw k5\{k7\},ymm6,ymm5
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 65 29[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 65 ac f4 c0 1d fe ff[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 65 6a 7f[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 65 aa 00 10 00 00[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 65 6a 80[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 65 aa e0 ef ff ff[ 	]*vpcmpgtw k5\{k7\},ymm6,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 04 f4[ 	]*vpmaddubsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 04 f4[ 	]*vpmaddubsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 04 31[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 04 b4 f4 c0 1d fe ff[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 04 72 7f[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 04 b2 00 08 00 00[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 04 72 80[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 04 b2 f0 f7 ff ff[ 	]*vpmaddubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 04 f4[ 	]*vpmaddubsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 04 f4[ 	]*vpmaddubsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 04 31[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 04 b4 f4 c0 1d fe ff[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 04 72 7f[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 04 b2 00 10 00 00[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 04 72 80[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 04 b2 e0 ef ff ff[ 	]*vpmaddubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f5 f4[ 	]*vpmaddwd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f f5 f4[ 	]*vpmaddwd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f5 31[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f5 b4 f4 c0 1d fe ff[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f5 72 7f[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f5 b2 00 08 00 00[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f5 72 80[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f5 b2 f0 f7 ff ff[ 	]*vpmaddwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f5 f4[ 	]*vpmaddwd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af f5 f4[ 	]*vpmaddwd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f5 31[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f5 b4 f4 c0 1d fe ff[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f5 72 7f[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f5 b2 00 10 00 00[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f5 72 80[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f5 b2 e0 ef ff ff[ 	]*vpmaddwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3c f4[ 	]*vpmaxsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 3c f4[ 	]*vpmaxsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3c 31[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3c b4 f4 c0 1d fe ff[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3c 72 7f[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3c b2 00 08 00 00[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3c 72 80[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3c b2 f0 f7 ff ff[ 	]*vpmaxsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3c f4[ 	]*vpmaxsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 3c f4[ 	]*vpmaxsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3c 31[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3c b4 f4 c0 1d fe ff[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3c 72 7f[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3c b2 00 10 00 00[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3c 72 80[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3c b2 e0 ef ff ff[ 	]*vpmaxsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ee f4[ 	]*vpmaxsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f ee f4[ 	]*vpmaxsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ee 31[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ee b4 f4 c0 1d fe ff[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ee 72 7f[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ee b2 00 08 00 00[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ee 72 80[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ee b2 f0 f7 ff ff[ 	]*vpmaxsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ee f4[ 	]*vpmaxsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af ee f4[ 	]*vpmaxsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ee 31[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ee b4 f4 c0 1d fe ff[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ee 72 7f[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ee b2 00 10 00 00[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ee 72 80[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ee b2 e0 ef ff ff[ 	]*vpmaxsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f de f4[ 	]*vpmaxub xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f de f4[ 	]*vpmaxub xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f de 31[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f de b4 f4 c0 1d fe ff[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f de 72 7f[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f de b2 00 08 00 00[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f de 72 80[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f de b2 f0 f7 ff ff[ 	]*vpmaxub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f de f4[ 	]*vpmaxub ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af de f4[ 	]*vpmaxub ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f de 31[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f de b4 f4 c0 1d fe ff[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f de 72 7f[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f de b2 00 10 00 00[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f de 72 80[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f de b2 e0 ef ff ff[ 	]*vpmaxub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3e f4[ 	]*vpmaxuw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 3e f4[ 	]*vpmaxuw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3e 31[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3e b4 f4 c0 1d fe ff[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3e 72 7f[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3e b2 00 08 00 00[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3e 72 80[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3e b2 f0 f7 ff ff[ 	]*vpmaxuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3e f4[ 	]*vpmaxuw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 3e f4[ 	]*vpmaxuw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3e 31[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3e b4 f4 c0 1d fe ff[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3e 72 7f[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3e b2 00 10 00 00[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3e 72 80[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3e b2 e0 ef ff ff[ 	]*vpmaxuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 38 f4[ 	]*vpminsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 38 f4[ 	]*vpminsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 38 31[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 38 b4 f4 c0 1d fe ff[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 38 72 7f[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 38 b2 00 08 00 00[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 38 72 80[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 38 b2 f0 f7 ff ff[ 	]*vpminsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 38 f4[ 	]*vpminsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 38 f4[ 	]*vpminsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 38 31[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 38 b4 f4 c0 1d fe ff[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 38 72 7f[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 38 b2 00 10 00 00[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 38 72 80[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 38 b2 e0 ef ff ff[ 	]*vpminsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ea f4[ 	]*vpminsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f ea f4[ 	]*vpminsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ea 31[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ea b4 f4 c0 1d fe ff[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ea 72 7f[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ea b2 00 08 00 00[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ea 72 80[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f ea b2 f0 f7 ff ff[ 	]*vpminsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ea f4[ 	]*vpminsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af ea f4[ 	]*vpminsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ea 31[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ea b4 f4 c0 1d fe ff[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ea 72 7f[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ea b2 00 10 00 00[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ea 72 80[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f ea b2 e0 ef ff ff[ 	]*vpminsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f da f4[ 	]*vpminub xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f da f4[ 	]*vpminub xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f da 31[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f da b4 f4 c0 1d fe ff[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f da 72 7f[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f da b2 00 08 00 00[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f da 72 80[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f da b2 f0 f7 ff ff[ 	]*vpminub xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f da f4[ 	]*vpminub ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af da f4[ 	]*vpminub ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f da 31[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f da b4 f4 c0 1d fe ff[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f da 72 7f[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f da b2 00 10 00 00[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f da 72 80[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f da b2 e0 ef ff ff[ 	]*vpminub ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3a f4[ 	]*vpminuw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 3a f4[ 	]*vpminuw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3a 31[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3a b4 f4 c0 1d fe ff[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3a 72 7f[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3a b2 00 08 00 00[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3a 72 80[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 3a b2 f0 f7 ff ff[ 	]*vpminuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3a f4[ 	]*vpminuw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 3a f4[ 	]*vpminuw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3a 31[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3a b4 f4 c0 1d fe ff[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3a 72 7f[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3a b2 00 10 00 00[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3a 72 80[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 3a b2 e0 ef ff ff[ 	]*vpminuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 20 f5[ 	]*vpmovsxbw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 20 f5[ 	]*vpmovsxbw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 20 31[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 20 b4 f4 c0 1d fe ff[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 20 72 7f[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 20 b2 00 04 00 00[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 20 72 80[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 20 b2 f8 fb ff ff[ 	]*vpmovsxbw xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 20 f5[ 	]*vpmovsxbw ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 20 f5[ 	]*vpmovsxbw ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 20 31[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 20 b4 f4 c0 1d fe ff[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 20 72 7f[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 20 b2 00 08 00 00[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 20 72 80[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 20 b2 f0 f7 ff ff[ 	]*vpmovsxbw ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 30 f5[ 	]*vpmovzxbw xmm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 8f 30 f5[ 	]*vpmovzxbw xmm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 30 31[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 30 b4 f4 c0 1d fe ff[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 30 72 7f[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx\+0x3f8\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 30 b2 00 04 00 00[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx\+0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 30 72 80[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx-0x400\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 0f 30 b2 f8 fb ff ff[ 	]*vpmovzxbw xmm6\{k7\},QWORD PTR \[edx-0x408\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 30 f5[ 	]*vpmovzxbw ymm6\{k7\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd af 30 f5[ 	]*vpmovzxbw ymm6\{k7\}\{z\},xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 30 31[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 30 b4 f4 c0 1d fe ff[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 30 72 7f[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 30 b2 00 08 00 00[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 30 72 80[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 fd 2f 30 b2 f0 f7 ff ff[ 	]*vpmovzxbw ymm6\{k7\},XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 0b f4[ 	]*vpmulhrsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 0b f4[ 	]*vpmulhrsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 0b 31[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 0b b4 f4 c0 1d fe ff[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 0b 72 7f[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 0b b2 00 08 00 00[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 0b 72 80[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 0b b2 f0 f7 ff ff[ 	]*vpmulhrsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 0b f4[ 	]*vpmulhrsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 0b f4[ 	]*vpmulhrsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 0b 31[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 0b b4 f4 c0 1d fe ff[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 0b 72 7f[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 0b b2 00 10 00 00[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 0b 72 80[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 0b b2 e0 ef ff ff[ 	]*vpmulhrsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e4 f4[ 	]*vpmulhuw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f e4 f4[ 	]*vpmulhuw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e4 31[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e4 b4 f4 c0 1d fe ff[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e4 72 7f[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e4 b2 00 08 00 00[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e4 72 80[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e4 b2 f0 f7 ff ff[ 	]*vpmulhuw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e4 f4[ 	]*vpmulhuw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af e4 f4[ 	]*vpmulhuw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e4 31[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e4 b4 f4 c0 1d fe ff[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e4 72 7f[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e4 b2 00 10 00 00[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e4 72 80[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e4 b2 e0 ef ff ff[ 	]*vpmulhuw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e5 f4[ 	]*vpmulhw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f e5 f4[ 	]*vpmulhw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e5 31[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e5 b4 f4 c0 1d fe ff[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e5 72 7f[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e5 b2 00 08 00 00[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e5 72 80[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e5 b2 f0 f7 ff ff[ 	]*vpmulhw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e5 f4[ 	]*vpmulhw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af e5 f4[ 	]*vpmulhw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e5 31[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e5 b4 f4 c0 1d fe ff[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e5 72 7f[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e5 b2 00 10 00 00[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e5 72 80[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e5 b2 e0 ef ff ff[ 	]*vpmulhw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d5 f4[ 	]*vpmullw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f d5 f4[ 	]*vpmullw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d5 31[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d5 b4 f4 c0 1d fe ff[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d5 72 7f[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d5 b2 00 08 00 00[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d5 72 80[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d5 b2 f0 f7 ff ff[ 	]*vpmullw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d5 f4[ 	]*vpmullw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af d5 f4[ 	]*vpmullw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d5 31[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d5 b4 f4 c0 1d fe ff[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d5 72 7f[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d5 b2 00 10 00 00[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d5 72 80[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d5 b2 e0 ef ff ff[ 	]*vpmullw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 00 f4[ 	]*vpshufb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 8f 00 f4[ 	]*vpshufb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 00 31[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 00 b4 f4 c0 1d fe ff[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 00 72 7f[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 00 b2 00 08 00 00[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 00 72 80[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 0f 00 b2 f0 f7 ff ff[ 	]*vpshufb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 00 f4[ 	]*vpshufb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 af 00 f4[ 	]*vpshufb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 00 31[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 00 b4 f4 c0 1d fe ff[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 00 72 7f[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 00 b2 00 10 00 00[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 00 72 80[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 d5 2f 00 b2 e0 ef ff ff[ 	]*vpshufb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 f5 ab[ 	]*vpshufhw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 8f 70 f5 ab[ 	]*vpshufhw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 f5 7b[ 	]*vpshufhw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 31 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 72 7f 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 b2 00 08 00 00 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 72 80 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 0f 70 b2 f0 f7 ff ff 7b[ 	]*vpshufhw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 f5 ab[ 	]*vpshufhw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe af 70 f5 ab[ 	]*vpshufhw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 f5 7b[ 	]*vpshufhw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 31 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 72 7f 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 b2 00 10 00 00 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 72 80 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 fe 2f 70 b2 e0 ef ff ff 7b[ 	]*vpshufhw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 f5 ab[ 	]*vpshuflw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 8f 70 f5 ab[ 	]*vpshuflw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 f5 7b[ 	]*vpshuflw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 31 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 72 7f 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 b2 00 08 00 00 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 72 80 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 0f 70 b2 f0 f7 ff ff 7b[ 	]*vpshuflw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 f5 ab[ 	]*vpshuflw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff af 70 f5 ab[ 	]*vpshuflw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 f5 7b[ 	]*vpshuflw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 31 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 b4 f4 c0 1d fe ff 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 72 7f 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 b2 00 10 00 00 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 72 80 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 ff 2f 70 b2 e0 ef ff ff 7b[ 	]*vpshuflw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f1 f4[ 	]*vpsllw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f f1 f4[ 	]*vpsllw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f1 31[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f1 b4 f4 c0 1d fe ff[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f1 72 7f[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f1 b2 00 08 00 00[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f1 72 80[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f1 b2 f0 f7 ff ff[ 	]*vpsllw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f1 f4[ 	]*vpsllw ymm6\{k7\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af f1 f4[ 	]*vpsllw ymm6\{k7\}\{z\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f1 31[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f1 b4 f4 c0 1d fe ff[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f1 72 7f[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f1 b2 00 08 00 00[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f1 72 80[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f1 b2 f0 f7 ff ff[ 	]*vpsllw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e1 f4[ 	]*vpsraw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f e1 f4[ 	]*vpsraw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e1 31[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e1 b4 f4 c0 1d fe ff[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e1 72 7f[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e1 b2 00 08 00 00[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e1 72 80[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e1 b2 f0 f7 ff ff[ 	]*vpsraw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e1 f4[ 	]*vpsraw ymm6\{k7\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af e1 f4[ 	]*vpsraw ymm6\{k7\}\{z\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e1 31[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e1 b4 f4 c0 1d fe ff[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e1 72 7f[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e1 b2 00 08 00 00[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e1 72 80[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e1 b2 f0 f7 ff ff[ 	]*vpsraw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d1 f4[ 	]*vpsrlw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f d1 f4[ 	]*vpsrlw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d1 31[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d1 b4 f4 c0 1d fe ff[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d1 72 7f[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d1 b2 00 08 00 00[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d1 72 80[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d1 b2 f0 f7 ff ff[ 	]*vpsrlw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d1 f4[ 	]*vpsrlw ymm6\{k7\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af d1 f4[ 	]*vpsrlw ymm6\{k7\}\{z\},ymm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d1 31[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d1 b4 f4 c0 1d fe ff[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d1 72 7f[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d1 b2 00 08 00 00[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d1 72 80[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d1 b2 f0 f7 ff ff[ 	]*vpsrlw ymm6\{k7\},ymm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 d5 ab[ 	]*vpsrlw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 8f 71 d5 ab[ 	]*vpsrlw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 d5 7b[ 	]*vpsrlw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 11 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 94 f4 c0 1d fe ff 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 52 7f 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 92 00 08 00 00 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 52 80 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 92 f0 f7 ff ff 7b[ 	]*vpsrlw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 d5 ab[ 	]*vpsrlw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd af 71 d5 ab[ 	]*vpsrlw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 d5 7b[ 	]*vpsrlw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 11 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 94 f4 c0 1d fe ff 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 52 7f 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 92 00 10 00 00 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 52 80 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 92 e0 ef ff ff 7b[ 	]*vpsrlw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 e5 ab[ 	]*vpsraw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 8f 71 e5 ab[ 	]*vpsraw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 e5 7b[ 	]*vpsraw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 21 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 a4 f4 c0 1d fe ff 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 62 7f 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 a2 00 08 00 00 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 62 80 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 a2 f0 f7 ff ff 7b[ 	]*vpsraw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 e5 ab[ 	]*vpsraw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd af 71 e5 ab[ 	]*vpsraw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 e5 7b[ 	]*vpsraw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 21 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 a4 f4 c0 1d fe ff 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 62 7f 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 a2 00 10 00 00 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 62 80 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 a2 e0 ef ff ff 7b[ 	]*vpsraw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f8 f4[ 	]*vpsubb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f f8 f4[ 	]*vpsubb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f8 31[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f8 b4 f4 c0 1d fe ff[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f8 72 7f[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f8 b2 00 08 00 00[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f8 72 80[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f8 b2 f0 f7 ff ff[ 	]*vpsubb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f8 f4[ 	]*vpsubb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af f8 f4[ 	]*vpsubb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f8 31[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f8 b4 f4 c0 1d fe ff[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f8 72 7f[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f8 b2 00 10 00 00[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f8 72 80[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f8 b2 e0 ef ff ff[ 	]*vpsubb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e8 f4[ 	]*vpsubsb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f e8 f4[ 	]*vpsubsb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e8 31[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e8 b4 f4 c0 1d fe ff[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e8 72 7f[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e8 b2 00 08 00 00[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e8 72 80[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e8 b2 f0 f7 ff ff[ 	]*vpsubsb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e8 f4[ 	]*vpsubsb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af e8 f4[ 	]*vpsubsb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e8 31[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e8 b4 f4 c0 1d fe ff[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e8 72 7f[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e8 b2 00 10 00 00[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e8 72 80[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e8 b2 e0 ef ff ff[ 	]*vpsubsb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e9 f4[ 	]*vpsubsw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f e9 f4[ 	]*vpsubsw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e9 31[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e9 b4 f4 c0 1d fe ff[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e9 72 7f[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e9 b2 00 08 00 00[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e9 72 80[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f e9 b2 f0 f7 ff ff[ 	]*vpsubsw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e9 f4[ 	]*vpsubsw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af e9 f4[ 	]*vpsubsw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e9 31[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e9 b4 f4 c0 1d fe ff[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e9 72 7f[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e9 b2 00 10 00 00[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e9 72 80[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f e9 b2 e0 ef ff ff[ 	]*vpsubsw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d8 f4[ 	]*vpsubusb xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f d8 f4[ 	]*vpsubusb xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d8 31[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d8 b4 f4 c0 1d fe ff[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d8 72 7f[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d8 b2 00 08 00 00[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d8 72 80[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d8 b2 f0 f7 ff ff[ 	]*vpsubusb xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d8 f4[ 	]*vpsubusb ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af d8 f4[ 	]*vpsubusb ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d8 31[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d8 b4 f4 c0 1d fe ff[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d8 72 7f[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d8 b2 00 10 00 00[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d8 72 80[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d8 b2 e0 ef ff ff[ 	]*vpsubusb ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d9 f4[ 	]*vpsubusw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f d9 f4[ 	]*vpsubusw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d9 31[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d9 b4 f4 c0 1d fe ff[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d9 72 7f[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d9 b2 00 08 00 00[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d9 72 80[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f d9 b2 f0 f7 ff ff[ 	]*vpsubusw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d9 f4[ 	]*vpsubusw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af d9 f4[ 	]*vpsubusw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d9 31[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d9 b4 f4 c0 1d fe ff[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d9 72 7f[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d9 b2 00 10 00 00[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d9 72 80[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f d9 b2 e0 ef ff ff[ 	]*vpsubusw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f9 f4[ 	]*vpsubw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f f9 f4[ 	]*vpsubw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f9 31[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f9 b4 f4 c0 1d fe ff[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f9 72 7f[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f9 b2 00 08 00 00[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f9 72 80[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f f9 b2 f0 f7 ff ff[ 	]*vpsubw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f9 f4[ 	]*vpsubw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af f9 f4[ 	]*vpsubw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f9 31[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f9 b4 f4 c0 1d fe ff[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f9 72 7f[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f9 b2 00 10 00 00[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f9 72 80[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f f9 b2 e0 ef ff ff[ 	]*vpsubw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 68 f4[ 	]*vpunpckhbw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 68 f4[ 	]*vpunpckhbw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 68 31[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 68 b4 f4 c0 1d fe ff[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 68 72 7f[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 68 b2 00 08 00 00[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 68 72 80[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 68 b2 f0 f7 ff ff[ 	]*vpunpckhbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 68 f4[ 	]*vpunpckhbw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 68 f4[ 	]*vpunpckhbw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 68 31[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 68 b4 f4 c0 1d fe ff[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 68 72 7f[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 68 b2 00 10 00 00[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 68 72 80[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 68 b2 e0 ef ff ff[ 	]*vpunpckhbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 69 f4[ 	]*vpunpckhwd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 69 f4[ 	]*vpunpckhwd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 69 31[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 69 b4 f4 c0 1d fe ff[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 69 72 7f[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 69 b2 00 08 00 00[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 69 72 80[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 69 b2 f0 f7 ff ff[ 	]*vpunpckhwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 69 f4[ 	]*vpunpckhwd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 69 f4[ 	]*vpunpckhwd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 69 31[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 69 b4 f4 c0 1d fe ff[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 69 72 7f[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 69 b2 00 10 00 00[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 69 72 80[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 69 b2 e0 ef ff ff[ 	]*vpunpckhwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 60 f4[ 	]*vpunpcklbw xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 60 f4[ 	]*vpunpcklbw xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 60 31[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 60 b4 f4 c0 1d fe ff[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 60 72 7f[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 60 b2 00 08 00 00[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 60 72 80[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 60 b2 f0 f7 ff ff[ 	]*vpunpcklbw xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 60 f4[ 	]*vpunpcklbw ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 60 f4[ 	]*vpunpcklbw ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 60 31[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 60 b4 f4 c0 1d fe ff[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 60 72 7f[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 60 b2 00 10 00 00[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 60 72 80[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 60 b2 e0 ef ff ff[ 	]*vpunpcklbw ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 61 f4[ 	]*vpunpcklwd xmm6\{k7\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 8f 61 f4[ 	]*vpunpcklwd xmm6\{k7\}\{z\},xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 61 31[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 61 b4 f4 c0 1d fe ff[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 61 72 7f[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x7f0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 61 b2 00 08 00 00[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx\+0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 61 72 80[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x800\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 0f 61 b2 f0 f7 ff ff[ 	]*vpunpcklwd xmm6\{k7\},xmm5,XMMWORD PTR \[edx-0x810\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 61 f4[ 	]*vpunpcklwd ymm6\{k7\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 af 61 f4[ 	]*vpunpcklwd ymm6\{k7\}\{z\},ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 61 31[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 61 b4 f4 c0 1d fe ff[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 61 72 7f[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0xfe0\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 61 b2 00 10 00 00[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx\+0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 61 72 80[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1000\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 d5 2f 61 b2 e0 ef ff ff[ 	]*vpunpcklwd ymm6\{k7\},ymm5,YMMWORD PTR \[edx-0x1020\]
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 f5 ab[ 	]*vpsllw xmm6\{k7\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 8f 71 f5 ab[ 	]*vpsllw xmm6\{k7\}\{z\},xmm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 f5 7b[ 	]*vpsllw xmm6\{k7\},xmm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 31 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 72 7f 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 b2 00 08 00 00 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx\+0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 72 80 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx-0x800\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 0f 71 b2 f0 f7 ff ff 7b[ 	]*vpsllw xmm6\{k7\},XMMWORD PTR \[edx-0x810\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 f5 ab[ 	]*vpsllw ymm6\{k7\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd af 71 f5 ab[ 	]*vpsllw ymm6\{k7\}\{z\},ymm5,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 f5 7b[ 	]*vpsllw ymm6\{k7\},ymm5,0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 31 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[ecx\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 b4 f4 c0 1d fe ff 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 72 7f 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 b2 00 10 00 00 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx\+0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 72 80 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx-0x1000\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f1 cd 2f 71 b2 e0 ef ff ff 7b[ 	]*vpsllw ymm6\{k7\},YMMWORD PTR \[edx-0x1020\],0x7b
#pass
