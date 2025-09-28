#source: tls-tiny-gd.s
#target: [check_shared_lib_support]
#ld: -shared -T relocs.ld -e0
#objdump: -dr
#...

Disassembly of section .text:

0000000000010000 \<test\>:
 +10000:	10080040 	adr	x0, 20008 \<var>
 +10004:	9400000a 	bl	1002c \<.*>
 +10008:	d503201f 	nop

Disassembly of section .plt:

000000000001000c \<.plt\>:
 +1000c:	a9bf7bf0 	stp	x16, x30, \[sp, #-16\]!
 +10010:	90000090 	adrp	x16, 20000 \<_GLOBAL_OFFSET_TABLE_\>
 +10014:	f9401611 	ldr	x17, \[x16, #40\]
 +10018:	9100a210 	add	x16, x16, #0x28
 +1001c:	d61f0220 	br	x17
 +10020:	d503201f 	nop
 +10024:	d503201f 	nop
 +10028:	d503201f 	nop
 +1002c:	90000090 	adrp	x16, 20000 \<_GLOBAL_OFFSET_TABLE_\>
 +10030:	f9401a11 	ldr	x17, \[x16, #48\]
 +10034:	9100c210 	add	x16, x16, #0x30
 +10038:	d61f0220 	br	x17
