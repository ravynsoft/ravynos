#objdump: -dw
#name: i386 arch 12

.*:     file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	0f 0f c1 bb          	pswapd %mm1,%mm0
[ 	]*[a-f0-9]+:	0f da c1             	pminub %mm1,%mm0
#pass
