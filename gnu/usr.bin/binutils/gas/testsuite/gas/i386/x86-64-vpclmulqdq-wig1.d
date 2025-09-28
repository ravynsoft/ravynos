#as: -mvexwig=1
#objdump: -dw
#name: x86_64 AVX/VPCLMULQDQ wig insns
#source: x86-64-vpclmulqdq.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*c4 43 b5 44 d0 ab[ 	]*vpclmulqdq \$0xab,%ymm8,%ymm9,%ymm10
[ 	]*[a-f0-9]+:[ 	]*c4 23 b5 44 94 f0 24 01 00 00 7b[ 	]*vpclmulqdq \$0x7b,0x124\(%rax,%r14,8\),%ymm9,%ymm10
[ 	]*[a-f0-9]+:[ 	]*c4 63 b5 44 92 e0 0f 00 00 7b[ 	]*vpclmulqdq \$0x7b,0xfe0\(%rdx\),%ymm9,%ymm10
[ 	]*[a-f0-9]+:[ 	]*c4 43 a5 44 e2 11[ 	]*vpclmulhqhqdq %ymm10,%ymm11,%ymm12
[ 	]*[a-f0-9]+:[ 	]*c4 43 9d 44 eb 01[ 	]*vpclmulhqlqdq %ymm11,%ymm12,%ymm13
[ 	]*[a-f0-9]+:[ 	]*c4 43 95 44 f4 10[ 	]*vpclmullqhqdq %ymm12,%ymm13,%ymm14
[ 	]*[a-f0-9]+:[ 	]*c4 43 8d 44 fd 00[ 	]*vpclmullqlqdq %ymm13,%ymm14,%ymm15
[ 	]*[a-f0-9]+:[ 	]*c4 43 b5 44 d0 ab[ 	]*vpclmulqdq \$0xab,%ymm8,%ymm9,%ymm10
[ 	]*[a-f0-9]+:[ 	]*c4 23 b5 44 94 f0 34 12 00 00 7b[ 	]*vpclmulqdq \$0x7b,0x1234\(%rax,%r14,8\),%ymm9,%ymm10
[ 	]*[a-f0-9]+:[ 	]*c4 63 b5 44 92 e0 0f 00 00 7b[ 	]*vpclmulqdq \$0x7b,0xfe0\(%rdx\),%ymm9,%ymm10
#pass
