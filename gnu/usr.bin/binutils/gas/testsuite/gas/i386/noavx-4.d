#objdump: -drw
#name: i386 .noavx

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	c5 d0 58 e6          	vaddps %xmm6,%xmm5,%xmm4
[ 	]*[a-f0-9]+:	c5 d4 58 e6          	vaddps %ymm6,%ymm5,%ymm4
[ 	]*[a-f0-9]+:	62 f1 54 0f 58 e6    	vaddps %xmm6,%xmm5,%xmm4\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 54 2f 58 e6    	vaddps %ymm6,%ymm5,%ymm4\{%k7\}
[ 	]*[a-f0-9]+:	62 f1 54 48 58 e6    	vaddps %zmm6,%zmm5,%zmm4
[ 	]*[a-f0-9]+:	c5 d2 58 e6          	vaddss %xmm6,%xmm5,%xmm4
[ 	]*[a-f0-9]+:	c5 d0 58 e6          	vaddps %xmm6,%xmm5,%xmm4
[ 	]*[a-f0-9]+:	c5 d4 58 e6          	vaddps %ymm6,%ymm5,%ymm4
[ 	]*[a-f0-9]+:	c5 d2 58 e6          	vaddss %xmm6,%xmm5,%xmm4
#pass
