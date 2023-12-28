#source: tls-relax-large-desc-le.s
#ld: -T relocs.ld -e0
#notarget: aarch64_be-*-*
#objdump: -dr
#...

Disassembly of section .text:

0000000000010000 <test>:
  +10000:	58000101 	ldr	x1, 10020 \<test\+0x20\>
  +10004:	100000e2 	adr	x2, 10020 \<test\+0x20\>
  +10008:	8b020032 	add	x18, x1, x2
  +1000c:	d2c00000 	movz	x0, #0x0, lsl #32
  +10010:	f2a00000 	movk	x0, #0x0, lsl #16
  +10014:	f2800200 	movk	x0, #0x10
  +10018:	d503201f 	nop
  +1001c:	d503201f 	nop
  +10020:	0000ffe0 	.word	0x0000ffe0
  +10024:	00000000 	.word	0x00000000
