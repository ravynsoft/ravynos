#source: tls-tiny-desc-le.s
#ld: -T relocs.ld -e0
#objdump: -dr
#...

Disassembly of section .text:

0000000000010000 \<test\>:
 +10000:	d2a00000 	movz	x0, #0x0, lsl #16
 +10004:	f2800200 	movk	x0, #0x10
 +10008:	d503201f 	nop
