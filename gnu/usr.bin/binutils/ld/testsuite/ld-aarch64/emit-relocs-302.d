#source: emit-relocs-302.s
#ld: -T relocs.ld --defsym globala=0x11000 --defsym globalb=0x45000 --defsym globalc=0x1234  -e0 --emit-relocs
#notarget: aarch64_be-*-*
#objdump: -dr

.*: +file format .*

Disassembly of section .text:

0000000000010000 <\.text>:
   10000:	580000e1 	ldr	x1, 1001c <\.text\+0x1c>
   10004:	100000c2 	adr	x2, 1001c <\.text\+0x1c>
   10008:	8b010041 	add	x1, x2, x1
   1000c:	d2a00000 	movz	x0, #0x0, lsl #16
			1000c: R_AARCH64_MOVW_GOTOFF_G1	globala
   10010:	d2a00000 	movz	x0, #0x0, lsl #16
			10010: R_AARCH64_MOVW_GOTOFF_G1	globalb
   10014:	d2a00000 	movz	x0, #0x0, lsl #16
			10014: R_AARCH64_MOVW_GOTOFF_G1	globalc
   10018:	f8606820 	ldr	x0, \[x1, x0\]
   1001c:	0000ffe4 	\.word	0x0000ffe4
			1001c: R_AARCH64_PREL64	_GLOBAL_OFFSET_TABLE_
   10020:	00000000 	\.word	0x00000000
