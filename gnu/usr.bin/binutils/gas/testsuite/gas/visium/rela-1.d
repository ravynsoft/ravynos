#as:
#objdump: -dr
#name: rela-1

.*: +file format .*

Disassembly of section .text:

0+0000000 <text>:
   0:	f8 00 00 04 	brr     tr,\+4
			0: R_VISIUM_PC16	.text2\+0x10
   4:	00 00 00 00 	nop
   8:	84 a6 00 00 	moviu   r6,0x0000
			8: R_VISIUM_HI16	.text2\+0x10
   c:	84 86 00 10 	movil   r6,0x0010
			c: R_VISIUM_LO16	.text2\+0x10
  10:	ff 86 00 04 	bra     tr,r6,r0
  14:	00 00 00 00 	nop
