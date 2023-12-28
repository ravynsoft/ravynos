#source: tls-large-desc.s
#target: [check_shared_lib_support]
#ld: -shared -T relocs.ld -e0
#notarget: aarch64_be-*-*
#objdump: -dr
#...

Disassembly of section .text:

0000000000010000 \<test\>:
  +10000:	58000101 	ldr	x1, 10020 \<test\+0x20\>
  +10004:	100000e2 	adr	x2, 10020 \<test\+0x20\>
  +10008:	8b020032 	add	x18, x1, x2
  +1000c:	d2a00000 	movz	x0, #0x0, lsl #16
  +10010:	f2800500 	movk	x0, #0x28
  +10014:	f8606a41 	ldr	x1, \[x18, x0\]
  +10018:	8b000240 	add	x0, x18, x0
  +1001c:	d63f0020 	blr	x1
  +10020:	0000ffe0 	.word	0x0000ffe0
  +10024:	00000000 	.word	0x00000000

Disassembly of section .plt:

0000000000010028 <.plt>:
  +10028:	a9bf7bf0 	stp	x16, x30, \[sp, #-16\]!
  +1002c:	90000090 	adrp	x16, 20000 \<_GLOBAL_OFFSET_TABLE_\>
  +10030:	f9401211 	ldr	x17, \[x16, #32\]
  +10034:	91008210 	add	x16, x16, #0x20
  +10038:	d61f0220 	br	x17
  +1003c:	d503201f 	nop
  +10040:	d503201f 	nop
  +10044:	d503201f 	nop
  +10048:	a9bf0fe2 	stp	x2, x3, \[sp, #-16\]!
  +1004c:	90000082 	adrp	x2, 20000 \<_GLOBAL_OFFSET_TABLE_\>
  +10050:	90000083 	adrp	x3, 20000 \<_GLOBAL_OFFSET_TABLE_\>
  +10054:	f9400442 	ldr	x2, \[x2, #8\]
  +10058:	91004063 	add	x3, x3, #0x10
  +1005c:	d61f0040 	br	x2
  +10060:	d503201f 	nop
  +10064:	d503201f 	nop
