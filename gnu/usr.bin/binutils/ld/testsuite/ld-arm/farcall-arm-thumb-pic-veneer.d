.*:     file format .*

Disassembly of section .text:

00001000 <_start>:
    1000:	eb000000 	bl	1008 <__bar_from_arm>
    1004:	00000000 	andeq	r0, r0, r0

00001008 <__bar_from_arm>:
    1008:	e59fc004 	ldr	ip, \[pc, #4\]	@ 1014 <__bar_from_arm\+0xc>
    100c:	e08fc00c 	add	ip, pc, ip
    1010:	e12fff1c 	bx	ip
    1014:	02000001 	.word	0x02000001
Disassembly of section .foo:

02001014 <bar>:
 2001014:	4770      	bx	lr
