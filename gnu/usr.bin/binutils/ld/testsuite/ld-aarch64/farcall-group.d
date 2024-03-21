#name: aarch64-farcall-group
#source: farcall-group.s
#as:
#ld: -Ttext=0x400078
#objdump: -dr
#...

Disassembly of section .text:

0000000000400078 <_start>:
  400078:	95000008 	bl	4400098 <__end_veneer>
	...
 440007c:	(d503201f|1f2003d5) 	.word	0x(d503201f|1f2003d5)
 4400080:	1400000e 	b	44000b8 <__end_veneer\+0x20>
 4400084:	d503201f 	nop

0000000004400088 <___start_veneer>:
 4400088:	90fe0010 	adrp	x16, 400000 <.*>
 440008c:	9101e210 	add	x16, x16, #0x78
 4400090:	d61f0200 	br	x16
 4400094:	00000000 	udf	#0

0000000004400098 <__end_veneer>:
 4400098:	90020010 	adrp	x16, 8400000 <__end_veneer\+0x3ffff68>
 440009c:	9102e210 	add	x16, x16, #0xb8
 44000a0:	d61f0200 	br	x16
	...

00000000084000b8 <end>:
 84000b8:	96fffff4 	bl	4400088 <___start_veneer>
