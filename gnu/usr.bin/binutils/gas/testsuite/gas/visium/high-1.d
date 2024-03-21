#as:
#objdump: -dr
#name: high-1

.*: +file format .*

Disassembly of section .text:

0+0000000 <foo>:
   0:	84 a4 00 01 	moviu   r4,0x0001
			0: R_VISIUM_HI16	.text\+0x10000
   4:	84 84 00 00 	movil   r4,0x0000
			4: R_VISIUM_LO16	.text\+0x10000
   8:	84 a4 12 34 	moviu   r4,0x1234
   c:	84 84 87 65 	movil   r4,0x8765
  10:	04 a4 00 00 	moviu   r4,0x0000
			10: R_VISIUM_HI16	.text\+0x18
  14:	84 84 00 18 	movil   r4,0x0018
			14: R_VISIUM_LO16	.text\+0x18
