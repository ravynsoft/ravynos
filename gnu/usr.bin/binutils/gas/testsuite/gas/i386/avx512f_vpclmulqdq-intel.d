#as:
#objdump: -dw -Mintel
#name: i386 AVX512F/VPCLMULQDQ insns (Intel disassembly)
#source: avx512f_vpclmulqdq.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 f3 65 48 44 c9 ab[ 	]*vpclmulqdq zmm1,zmm3,zmm1,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 65 48 44 8c f4 c0 1d fe ff 7b[ 	]*vpclmulqdq zmm1,zmm3,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 65 48 44 4a 7f 7b[ 	]*vpclmulqdq zmm1,zmm3,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 6d 48 44 d9 11[ 	]*vpclmulhqhqdq zmm3,zmm2,zmm1
[ 	]*[a-f0-9]+:[ 	]*62 f3 65 48 44 e2 01[ 	]*vpclmulhqlqdq zmm4,zmm3,zmm2
[ 	]*[a-f0-9]+:[ 	]*62 f3 5d 48 44 eb 10[ 	]*vpclmullqhqdq zmm5,zmm4,zmm3
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 48 44 f4 00[ 	]*vpclmullqlqdq zmm6,zmm5,zmm4
[ 	]*[a-f0-9]+:[ 	]*62 f3 6d 48 44 d2 ab[ 	]*vpclmulqdq zmm2,zmm2,zmm2,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 6d 48 44 94 f4 c0 1d fe ff 7b[ 	]*vpclmulqdq zmm2,zmm2,ZMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 6d 48 44 52 7f 7b[ 	]*vpclmulqdq zmm2,zmm2,ZMMWORD PTR \[edx\+0x1fc0\],0x7b
#pass
