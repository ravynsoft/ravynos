#source: emit-relocs-516.s
#target: [check_shared_lib_support]
#ld: -shared -T relocs.ld -e0
#notarget: aarch64-*-*
#objdump: -dr
#...

Disassembly of section .text:

0000000000010000 \<test\>:
   10000:	58000101 	ldr	x1, 10020 \<test\+0x20\>
   10004:	100000e2 	adr	x2, 10020 \<test\+0x20\>
   10008:	8b020021 	add	x1, x1, x2
   1000c:	f2800100 	movk	x0, #0x8
   10010:	f2800300 	movk	x0, #0x18
   10014:	8b000020 	add	x0, x1, x0
   10018:	9400000c 	bl	10048 \<.*\>
   1001c:	d503201f 	nop
   10020:	00000000 	.word	0x00000000
   10024:	0000ffe0 	.word	0x0000ffe0

Disassembly of section .plt:

0000000000010028 \<.plt\>:
   10028:	a9bf7bf0 	stp	x16, x30, \[sp, #-16\]!
   1002c:	90000090 	adrp	x16, 20000 \<_GLOBAL_OFFSET_TABLE_\>
   10030:	f9401e11 	ldr	x17, \[x16, #56\]
   10034:	9100e210 	add	x16, x16, #0x38
   10038:	d61f0220 	br	x17
   1003c:	d503201f 	nop
   10040:	d503201f 	nop
   10044:	d503201f 	nop
   10048:	90000090 	adrp	x16, 20000 \<_GLOBAL_OFFSET_TABLE_\>
   1004c:	f9402211 	ldr	x17, \[x16, #64\]
   10050:	91010210 	add	x16, x16, #0x40
   10054:	d61f0220 	br	x17
