#source: emit-relocs-515.s
#target: [check_shared_lib_support]
#ld: -shared -T relocs.ld -e0
#notarget: aarch64_be-*-*
#objdump: -dr
#...

Disassembly of section .text:

0000000000010000 \<test\>:
   10000:	580000e1 	ldr	x1, 1001c \<test\+0x1c\>
   10004:	100000c2 	adr	x2, 1001c \<test\+0x1c\>
   10008:	8b020021 	add	x1, x1, x2
   1000c:	d2a00000 	movz	x0, #0x0, lsl #16
   10010:	8b000020 	add	x0, x1, x0
   10014:	9400000c 	bl	10044 \<.*\>
   10018:	d503201f 	nop
   1001c:	0000ffe4 	.word	0x0000ffe4
   10020:	00000000 	.word	0x00000000

Disassembly of section .plt:

0000000000010024 \<.plt\>:
   10024:	a9bf7bf0 	stp	x16, x30, \[sp, #-16\]!
   10028:	90000090 	adrp	x16, 20000 \<_GLOBAL_OFFSET_TABLE_\>
   1002c:	f9401611 	ldr	x17, \[x16, #40\]
   10030:	9100a210 	add	x16, x16, #0x28
   10034:	d61f0220 	br	x17
   10038:	d503201f 	nop
   1003c:	d503201f 	nop
   10040:	d503201f 	nop
   10044:	90000090 	adrp	x16, 20000 \<_GLOBAL_OFFSET_TABLE_\>
   10048:	f9401a11 	ldr	x17, \[x16, #48\]
   1004c:	9100c210 	add	x16, x16, #0x30
   10050:	d61f0220 	br	x17
