.*:     file format .*

Disassembly of section .text:

00001000 <_start>:
    1000:	eb000000 	bl	1008 <__bar_veneer>
    1004:	00000000 	andeq	r0, r0, r0

00001008 <__bar_veneer>:
    1008:	e59fc000 	ldr	ip, \[pc\]	@ 1010 <__bar_veneer\+0x8>
    100c:	e08ff00c 	add	pc, pc, ip
    1010:	0200000c 	.word	0x0200000c
    1014:	00000000 	.word	0x00000000
Disassembly of section .foo:

02001020 <bar>:
 2001020:	e12fff1e 	bx	lr
