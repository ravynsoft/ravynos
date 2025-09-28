#objdump: -dw
#name: i386 arch 11

.*:     file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	f3 0f 5e c1          	divss  %xmm1,%xmm0
[ 	]*[a-f0-9]+:	0f da c1             	pminub %mm1,%mm0
#pass
