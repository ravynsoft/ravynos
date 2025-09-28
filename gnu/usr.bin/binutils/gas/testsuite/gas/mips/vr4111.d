#objdump: -drz
#name: MIPS VR4111
#as: -march=vr4111

.*: +file format .*mips.*

Disassembly of section \.text:
0+000 <\.text>:
 + 0:	00850029 	dmadd16	a0,a1
 + 4:	00000000 	nop
 + 8:	00000000 	nop
 + c:	00a60028 	madd16	a1,a2
 +10:	00000000 	nop
 +14:	00000000 	nop
#pass
