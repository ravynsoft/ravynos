#objdump: -drw
#name: evex equates

.*: +file format .*

Disassembly of section .text:

0+000 <_start>:
[ 	]*[a-f0-9]+:	62 e1 76 08 58 c8    	vaddss %xmm0,%xmm1,%xmm17
[ 	]*[a-f0-9]+:	62 b1 76 08 58 c1    	vaddss %xmm17,%xmm1,%xmm0
#pass
