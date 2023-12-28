#source: ../x86-64-avx-swap.s
#as: -msse2avx
#objdump: -drwMintel
#name: x86-64 (ILP32) AVX swap (Intel mode)

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c5 7d 29 c6          	vmovapd ymm6,ymm8
[ 	]*[a-f0-9]+:	c5 7c 29 c6          	vmovaps ymm6,ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f c6          	vmovdqa ymm6,ymm8
[ 	]*[a-f0-9]+:	c5 7e 7f c6          	vmovdqu ymm6,ymm8
[ 	]*[a-f0-9]+:	c5 7d 11 c6          	vmovupd ymm6,ymm8
[ 	]*[a-f0-9]+:	c5 7c 11 c6          	vmovups ymm6,ymm8
[ 	]*[a-f0-9]+:	c5 79 29 c6          	vmovapd xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 78 29 c6          	vmovaps xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 79 7f c6          	vmovdqa xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 7a 7f c6          	vmovdqu xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 79 d6 c6          	vmovq  xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 4b 11 c6          	vmovsd xmm6,xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 4a 11 c6          	vmovss xmm6,xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 79 11 c6          	vmovupd xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 78 11 c6          	vmovups xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 79 29 c6          	vmovapd xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 78 29 c6          	vmovaps xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 79 7f c6          	vmovdqa xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 7a 7f c6          	vmovdqu xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 79 d6 c6          	vmovq  xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 79 11 c6          	vmovupd xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 78 11 c6          	vmovups xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 4b 11 c2          	vmovsd xmm2,xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 4a 11 c2          	vmovss xmm2,xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 7d 29 c6          	vmovapd ymm6,ymm8
[ 	]*[a-f0-9]+:	c5 7c 29 c6          	vmovaps ymm6,ymm8
[ 	]*[a-f0-9]+:	c5 7d 7f c6          	vmovdqa ymm6,ymm8
[ 	]*[a-f0-9]+:	c5 7e 7f c6          	vmovdqu ymm6,ymm8
[ 	]*[a-f0-9]+:	c5 7d 11 c6          	vmovupd ymm6,ymm8
[ 	]*[a-f0-9]+:	c5 7c 11 c6          	vmovups ymm6,ymm8
[ 	]*[a-f0-9]+:	c5 79 29 c6          	vmovapd xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 78 29 c6          	vmovaps xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 79 7f c6          	vmovdqa xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 7a 7f c6          	vmovdqu xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 79 d6 c6          	vmovq  xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 4b 11 c6          	vmovsd xmm6,xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 4a 11 c6          	vmovss xmm6,xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 79 11 c6          	vmovupd xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 78 11 c6          	vmovups xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 79 29 c6          	vmovapd xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 78 29 c6          	vmovaps xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 79 7f c6          	vmovdqa xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 7a 7f c6          	vmovdqu xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 79 d6 c6          	vmovq  xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 79 11 c6          	vmovupd xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 78 11 c6          	vmovups xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 4b 11 c2          	vmovsd xmm2,xmm6,xmm8
[ 	]*[a-f0-9]+:	c5 4a 11 c2          	vmovss xmm2,xmm6,xmm8
#pass
