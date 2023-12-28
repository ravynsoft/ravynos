.*:     file format .*

Disassembly of section .text:

00001000 <_start>:
    1000:	f000 e802 	blx	1008 <__bar_veneer>
    1004:	0000      	movs	r0, r0
	\.\.\.

00001008 <__bar_veneer>:
    1008:	e59fc004 	ldr	ip, \[pc, #4\]	@ 1014 <__bar_veneer\+0xc>
    100c:	e08fc00c 	add	ip, pc, ip
    1010:	e12fff1c 	bx	ip
    1014:	02000001 	.word	0x02000001
Disassembly of section .foo:

02001014 <bar>:
 2001014:	4770      	bx	lr
