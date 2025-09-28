#as:
#objdump: -dw -Mintel
#name: i386 AVX512F/VAES insns (Intel disassembly)
#source: avx512f_vaes.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 de f4[ 	]*vaesdec zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 de b4 f4 c0 1d fe ff[ 	]*vaesdec zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 de 72 7f[ 	]*vaesdec zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 df f4[ 	]*vaesdeclast zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 df b4 f4 c0 1d fe ff[ 	]*vaesdeclast zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 df 72 7f[ 	]*vaesdeclast zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dc f4[ 	]*vaesenc zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dc b4 f4 c0 1d fe ff[ 	]*vaesenc zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dc 72 7f[ 	]*vaesenc zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dd f4[ 	]*vaesenclast zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dd b4 f4 c0 1d fe ff[ 	]*vaesenclast zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dd 72 7f[ 	]*vaesenclast zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 de f4[ 	]*vaesdec zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 de b4 f4 c0 1d fe ff[ 	]*vaesdec zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 de 72 7f[ 	]*vaesdec zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 df f4[ 	]*vaesdeclast zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 df b4 f4 c0 1d fe ff[ 	]*vaesdeclast zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 df 72 7f[ 	]*vaesdeclast zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dc f4[ 	]*vaesenc zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dc b4 f4 c0 1d fe ff[ 	]*vaesenc zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dc 72 7f[ 	]*vaesenc zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dd f4[ 	]*vaesenclast zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dd b4 f4 c0 1d fe ff[ 	]*vaesenclast zmm6,zmm5,ZMMWORD PTR \[esp\+esi\*8-0x1e240\]
[ 	]*[a-f0-9]+:[ 	]*62 f2 55 48 dd 72 7f[ 	]*vaesenclast zmm6,zmm5,ZMMWORD PTR \[edx\+0x1fc0\]
#pass
