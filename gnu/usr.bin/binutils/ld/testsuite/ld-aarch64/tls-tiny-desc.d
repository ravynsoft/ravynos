#source: tls-tiny-desc.s
#target: [check_shared_lib_support]
#ld: -shared -T relocs.ld -e0
#objdump: -dr
#...

Disassembly of section .text:

0000000000010000 \<test\>:
 +10000:	58080141 	ldr	x1, 20028 \<var>
 +10004:	10080120 	adr	x0, 20028 \<var>
 +10008:	d63f0020 	blr	x1

Disassembly of section .plt:

000000000001000c \<.plt\>:
 +1000c:	a9bf7bf0 	stp	x16, x30, \[sp, #-16\]!
 +10010:	90000090 	adrp	x16, 20000 \<_GLOBAL_OFFSET_TABLE_\>
 +10014:	f9401211 	ldr	x17, \[x16, #32\]
 +10018:	91008210 	add	x16, x16, #0x20
 +1001c:	d61f0220 	br	x17
 +10020:	d503201f 	nop
 +10024:	d503201f 	nop
 +10028:	d503201f 	nop
 +1002c:	a9bf0fe2 	stp	x2, x3, \[sp, #-16\]!
 +10030:	90000082 	adrp	x2, 20000 \<_GLOBAL_OFFSET_TABLE_\>
 +10034:	90000083 	adrp	x3, 20000 \<_GLOBAL_OFFSET_TABLE_\>
 +10038:	f9400442 	ldr	x2, \[x2, #8\]
 +1003c:	91004063 	add	x3, x3, #0x10
 +10040:	d61f0040 	br	x2
 +10044:	d503201f 	nop
 +10048:	d503201f 	nop
