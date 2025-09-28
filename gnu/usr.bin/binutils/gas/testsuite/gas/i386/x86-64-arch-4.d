#objdump: -dw
#name: x86-64 arch 4

.*:     file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[0-9a-f]+:[ 	]+0f 01 fe[ 	]+invlpgb
[ 	]*[0-9a-f]+:[ 	]+0f 01 ff[ 	]+tlbsync
[ 	]*[a-f0-9]+:[ 	]*c4 43 35 44 d0 ab[ 	]*vpclmulqdq \$0xab,%ymm8,%ymm9,%ymm10
[ 	]*[a-f0-9]+:[ 	]*c4 23 35 44 94 f0 24 01 00 00 7b[ 	]*vpclmulqdq \$0x7b,0x124\(%rax,%r14,8\),%ymm9,%ymm10
[ 	]*[a-f0-9]+:[ 	]*c4 63 35 44 92 e0 0f 00 00 7b[ 	]*vpclmulqdq \$0x7b,0xfe0\(%rdx\),%ymm9,%ymm10
[ 	]*[a-f0-9]+:[ 	]*c4 43 25 44 e2 11[ 	]*vpclmulhqhqdq %ymm10,%ymm11,%ymm12
[ 	]*[a-f0-9]+:[ 	]*c4 43 1d 44 eb 01[ 	]*vpclmulhqlqdq %ymm11,%ymm12,%ymm13
[ 	]*[a-f0-9]+:[ 	]*c4 43 15 44 f4 10[ 	]*vpclmullqhqdq %ymm12,%ymm13,%ymm14
[ 	]*[a-f0-9]+:[ 	]*c4 43 0d 44 fd 00[ 	]*vpclmullqlqdq %ymm13,%ymm14,%ymm15
[ 	]*[a-f0-9]+:	c4 e2 4d dc d4[ 	]+vaesenc %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d dc 39[ 	]+vaesenc \(%rcx\),%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e2 4d dd d4[ 	]+vaesenclast %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d dd 39[ 	]+vaesenclast \(%rcx\),%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e2 4d de d4[ 	]+vaesdec %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d de 39[ 	]+vaesdec \(%rcx\),%ymm6,%ymm7
[ 	]*[a-f0-9]+:	c4 e2 4d df d4[ 	]+vaesdeclast %ymm4,%ymm6,%ymm2
[ 	]*[a-f0-9]+:	c4 e2 4d df 39[ 	]+vaesdeclast \(%rcx\),%ymm6,%ymm7
[ 	]*[a-f0-9]+:	f3 0f 01 ff[ 	]+psmash
[ 	]*[a-f0-9]+:	f2 0f 01 ff[ 	]+pvalidate
[ 	]*[a-f0-9]+:	f2 0f 01 fe[ 	]+rmpupdate
[ 	]*[a-f0-9]+:	f3 0f 01 fe[ 	]+rmpadjust
[ 	]*[a-f0-9]+:	66 0f 38 82 10[ 	]+invpcid \(%rax\),%rdx
[ 	]*[a-f0-9]+:	0f 01 ee[ 	]+rdpkru
[ 	]*[a-f0-9]+:	0f 01 ef[ 	]+wrpkru
#pass
