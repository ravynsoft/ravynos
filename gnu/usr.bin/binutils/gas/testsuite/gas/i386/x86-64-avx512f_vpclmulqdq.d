#as:
#objdump: -dw
#name: x86_64 AVX512F/VPCLMULQDQ insns
#source: x86-64-avx512f_vpclmulqdq.s

.*: +file format .*


Disassembly of section \.text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*62 03 45 40 44 d0 ab[ 	]*vpclmulqdq \$0xab,%zmm24,%zmm23,%zmm26
[ 	]*[a-f0-9]+:[ 	]*62 23 45 40 44 94 f0 23 01 00 00 7b[ 	]*vpclmulqdq \$0x7b,0x123\(%rax,%r14,8\),%zmm23,%zmm26
[ 	]*[a-f0-9]+:[ 	]*62 63 45 40 44 52 7f 7b[ 	]*vpclmulqdq \$0x7b,0x1fc0\(%rdx\),%zmm23,%zmm26
[ 	]*[a-f0-9]+:[ 	]*62 a3 55 40 44 f4 11[ 	]*vpclmulhqhqdq %zmm20,%zmm21,%zmm22
[ 	]*[a-f0-9]+:[ 	]*62 a3 4d 40 44 fd 01[ 	]*vpclmulhqlqdq %zmm21,%zmm22,%zmm23
[ 	]*[a-f0-9]+:[ 	]*62 23 45 40 44 c6 10[ 	]*vpclmullqhqdq %zmm22,%zmm23,%zmm24
[ 	]*[a-f0-9]+:[ 	]*62 23 3d 40 44 cf 00[ 	]*vpclmullqlqdq %zmm23,%zmm24,%zmm25
[ 	]*[a-f0-9]+:[ 	]*62 83 55 40 44 eb ab[ 	]*vpclmulqdq \$0xab,%zmm27,%zmm21,%zmm21
[ 	]*[a-f0-9]+:[ 	]*62 a3 55 40 44 ac f0 34 12 00 00 7b[ 	]*vpclmulqdq \$0x7b,0x1234\(%rax,%r14,8\),%zmm21,%zmm21
[ 	]*[a-f0-9]+:[ 	]*62 e3 55 40 44 6a 7f 7b[ 	]*vpclmulqdq \$0x7b,0x1fc0\(%rdx\),%zmm21,%zmm21
#pass
