#as:
#objdump: -dw -Mintel
#name: i386 AVX512VL/VPCLMULQDQ insns (Intel disassembly)
#source: avx512vl_vpclmulqdq.s

.*: +file format .*


Disassembly of section \.text:

00000000 <_start>:
[ 	]*[a-f0-9]+:[ 	]*c4 e3 69 44 da ab[ 	]*vpclmulqdq xmm3,xmm2,xmm2,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 69 44 9c f4 c0 1d fe ff 7b[ 	]*vpclmulqdq xmm3,xmm2,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 69 44 9a f0 07 00 00 7b[ 	]*vpclmulqdq xmm3,xmm2,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 55 44 e1 ab[ 	]*vpclmulqdq ymm4,ymm5,ymm1,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 55 44 a4 f4 c0 1d fe ff 7b[ 	]*vpclmulqdq ymm4,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 55 44 a2 e0 0f 00 00 7b[ 	]*vpclmulqdq ymm4,ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 6d 08 44 da ab[ 	]*\{evex\} vpclmulqdq xmm3,xmm2,xmm2,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 6d 08 44 9c f4 c0 1d fe ff 7b[ 	]*\{evex\} vpclmulqdq xmm3,xmm2,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 6d 08 44 5a 7f 7b[ 	]*\{evex\} vpclmulqdq xmm3,xmm2,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 28 44 e1 ab[ 	]*\{evex\} vpclmulqdq ymm4,ymm5,ymm1,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 28 44 a4 f4 c0 1d fe ff 7b[ 	]*\{evex\} vpclmulqdq ymm4,ymm5,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 28 44 62 7f 7b[ 	]*\{evex\} vpclmulqdq ymm4,ymm5,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 65 08 44 e2 11[ 	]*\{evex\} vpclmulhqhqdq xmm4,xmm3,xmm2
[ 	]*[a-f0-9]+:[ 	]*62 f3 5d 08 44 eb 01[ 	]*\{evex\} vpclmulhqlqdq xmm5,xmm4,xmm3
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 08 44 f4 10[ 	]*\{evex\} vpclmullqhqdq xmm6,xmm5,xmm4
[ 	]*[a-f0-9]+:[ 	]*62 f3 4d 08 44 fd 00[ 	]*\{evex\} vpclmullqlqdq xmm7,xmm6,xmm5
[ 	]*[a-f0-9]+:[ 	]*62 f3 6d 28 44 d9 11[ 	]*\{evex\} vpclmulhqhqdq ymm3,ymm2,ymm1
[ 	]*[a-f0-9]+:[ 	]*62 f3 65 28 44 e2 01[ 	]*\{evex\} vpclmulhqlqdq ymm4,ymm3,ymm2
[ 	]*[a-f0-9]+:[ 	]*62 f3 5d 28 44 eb 10[ 	]*\{evex\} vpclmullqhqdq ymm5,ymm4,ymm3
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 28 44 f4 00[ 	]*\{evex\} vpclmullqlqdq ymm6,ymm5,ymm4
[ 	]*[a-f0-9]+:[ 	]*c4 e3 51 44 db ab[ 	]*vpclmulqdq xmm3,xmm5,xmm3,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 51 44 9c f4 c0 1d fe ff 7b[ 	]*vpclmulqdq xmm3,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 51 44 9a f0 07 00 00 7b[ 	]*vpclmulqdq xmm3,xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 6d 44 d2 ab[ 	]*vpclmulqdq ymm2,ymm2,ymm2,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 e3 6d 44 94 f4 c0 1d fe ff 7b[ 	]*vpclmulqdq ymm2,ymm2,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 e3 6d 44 92 e0 0f 00 00 7b[ 	]*vpclmulqdq ymm2,ymm2,YMMWORD PTR \[edx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 08 44 db ab[ 	]*\{evex\} vpclmulqdq xmm3,xmm5,xmm3,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 08 44 9c f4 c0 1d fe ff 7b[ 	]*\{evex\} vpclmulqdq xmm3,xmm5,XMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 55 08 44 5a 7f 7b[ 	]*\{evex\} vpclmulqdq xmm3,xmm5,XMMWORD PTR \[edx\+0x7f0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 6d 28 44 d2 ab[ 	]*\{evex\} vpclmulqdq ymm2,ymm2,ymm2,0xab
[ 	]*[a-f0-9]+:[ 	]*62 f3 6d 28 44 94 f4 c0 1d fe ff 7b[ 	]*\{evex\} vpclmulqdq ymm2,ymm2,YMMWORD PTR \[esp\+esi\*8-0x1e240\],0x7b
[ 	]*[a-f0-9]+:[ 	]*62 f3 6d 28 44 52 7f 7b[ 	]*\{evex\} vpclmulqdq ymm2,ymm2,YMMWORD PTR \[edx\+0xfe0\],0x7b
#pass
