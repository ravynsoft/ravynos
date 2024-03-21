#source: emit-relocs-310.s
#ld: -T relocs.ld --defsym globala=0x11000 --defsym globalb=0x45000 --defsym globalc=0x1234  -e0 --emit-relocs
#notarget: aarch64_be-*-*
#objdump: -dr

.*: +file format .*

Disassembly of section .text:

0000000000010000 <\.text>:
   10000:	580000c1 	ldr	x1, 10018 <\.text\+0x18>
   10004:	100000a2 	adr	x2, 10018 <\.text\+0x18>
   10008:	8b010041 	add	x1, x2, x1
   1000c:	f9400820 	ldr	x0, \[x1, #16\]
			1000c: R_AARCH64_LD64_GOTOFF_LO15	globala
   10010:	f9400c20 	ldr	x0, \[x1, #24\]
			10010: R_AARCH64_LD64_GOTOFF_LO15	globalb
   10014:	f9400420 	ldr	x0, \[x1, #8\]
			10014: R_AARCH64_LD64_GOTOFF_LO15	globalc
   10018:	0000ffe8 	.word	0x0000ffe8
			10018: R_AARCH64_PREL64	_GLOBAL_OFFSET_TABLE_
   1001c:	00000000 	.word	0x00000000
