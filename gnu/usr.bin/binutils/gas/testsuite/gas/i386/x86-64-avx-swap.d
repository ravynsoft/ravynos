#as: -msse2avx
#objdump: -drw
#name: x86-64 AVX swap

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c5 7d 29 c6          	vmovapd %ymm8,%ymm6
[ 	]*[a-f0-9]+:	c5 7c 29 c6          	vmovaps %ymm8,%ymm6
[ 	]*[a-f0-9]+:	c5 7d 7f c6          	vmovdqa %ymm8,%ymm6
[ 	]*[a-f0-9]+:	c5 7e 7f c6          	vmovdqu %ymm8,%ymm6
[ 	]*[a-f0-9]+:	c5 7d 11 c6          	vmovupd %ymm8,%ymm6
[ 	]*[a-f0-9]+:	c5 7c 11 c6          	vmovups %ymm8,%ymm6
[ 	]*[a-f0-9]+:	c5 79 29 c6          	vmovapd %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 78 29 c6          	vmovaps %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 79 7f c6          	vmovdqa %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 7a 7f c6          	vmovdqu %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 79 d6 c6          	vmovq  %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 4b 11 c6          	vmovsd %xmm8,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 4a 11 c6          	vmovss %xmm8,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 79 11 c6          	vmovupd %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 78 11 c6          	vmovups %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 79 29 c6          	vmovapd %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 78 29 c6          	vmovaps %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 79 7f c6          	vmovdqa %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 7a 7f c6          	vmovdqu %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 79 d6 c6          	vmovq  %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 79 11 c6          	vmovupd %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 78 11 c6          	vmovups %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 4b 11 c2          	vmovsd %xmm8,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 4a 11 c2          	vmovss %xmm8,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 7d 29 c6          	vmovapd %ymm8,%ymm6
[ 	]*[a-f0-9]+:	c5 7c 29 c6          	vmovaps %ymm8,%ymm6
[ 	]*[a-f0-9]+:	c5 7d 7f c6          	vmovdqa %ymm8,%ymm6
[ 	]*[a-f0-9]+:	c5 7e 7f c6          	vmovdqu %ymm8,%ymm6
[ 	]*[a-f0-9]+:	c5 7d 11 c6          	vmovupd %ymm8,%ymm6
[ 	]*[a-f0-9]+:	c5 7c 11 c6          	vmovups %ymm8,%ymm6
[ 	]*[a-f0-9]+:	c5 79 29 c6          	vmovapd %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 78 29 c6          	vmovaps %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 79 7f c6          	vmovdqa %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 7a 7f c6          	vmovdqu %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 79 d6 c6          	vmovq  %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 4b 11 c6          	vmovsd %xmm8,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 4a 11 c6          	vmovss %xmm8,%xmm6,%xmm6
[ 	]*[a-f0-9]+:	c5 79 11 c6          	vmovupd %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 78 11 c6          	vmovups %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 79 29 c6          	vmovapd %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 78 29 c6          	vmovaps %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 79 7f c6          	vmovdqa %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 7a 7f c6          	vmovdqu %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 79 d6 c6          	vmovq  %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 79 11 c6          	vmovupd %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 78 11 c6          	vmovups %xmm8,%xmm6
[ 	]*[a-f0-9]+:	c5 4b 11 c2          	vmovsd %xmm8,%xmm6,%xmm2
[ 	]*[a-f0-9]+:	c5 4a 11 c2          	vmovss %xmm8,%xmm6,%xmm2
#pass
