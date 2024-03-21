#as:
#objdump: -dw -Mintel
#name: x86_64 VPCLMULQDQ insns (Intel disassembly)
#source: x86-64-vpclmulqdq.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*c4 43 35 44 d0 ab[ 	]*vpclmulqdq ymm10,ymm9,ymm8,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 23 35 44 94 f0 24 01 00 00 7b[ 	]*vpclmulqdq ymm10,ymm9,YMMWORD PTR \[rax\+r14\*8\+0x124\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 63 35 44 92 e0 0f 00 00 7b[ 	]*vpclmulqdq ymm10,ymm9,YMMWORD PTR \[rdx\+0xfe0\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 43 25 44 e2 11[ 	]*vpclmulhqhqdq ymm12,ymm11,ymm10
[ 	]*[a-f0-9]+:[ 	]*c4 43 1d 44 eb 01[ 	]*vpclmulhqlqdq ymm13,ymm12,ymm11
[ 	]*[a-f0-9]+:[ 	]*c4 43 15 44 f4 10[ 	]*vpclmullqhqdq ymm14,ymm13,ymm12
[ 	]*[a-f0-9]+:[ 	]*c4 43 0d 44 fd 00[ 	]*vpclmullqlqdq ymm15,ymm14,ymm13
[ 	]*[a-f0-9]+:[ 	]*c4 43 35 44 d0 ab[ 	]*vpclmulqdq ymm10,ymm9,ymm8,0xab
[ 	]*[a-f0-9]+:[ 	]*c4 23 35 44 94 f0 34 12 00 00 7b[ 	]*vpclmulqdq ymm10,ymm9,YMMWORD PTR \[rax\+r14\*8\+0x1234\],0x7b
[ 	]*[a-f0-9]+:[ 	]*c4 63 35 44 92 e0 0f 00 00 7b[ 	]*vpclmulqdq ymm10,ymm9,YMMWORD PTR \[rdx\+0xfe0\],0x7b
#pass
