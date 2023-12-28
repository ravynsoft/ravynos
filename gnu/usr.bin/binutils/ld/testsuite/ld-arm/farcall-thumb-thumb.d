.*:     file format .*

Disassembly of section .text:

00001000 <_start>:
    1000:	f000 f802 	bl	1008 <__bar_veneer>
    1004:	0000      	movs	r0, r0
	\.\.\.

00001008 <__bar_veneer>:
    1008:	4778      	bx	pc
    100a:	e7fd      	b.n	.+ <.+>
    100c:	e59fc000 	ldr	ip, \[pc\]	@ 1014 <__bar_veneer\+0xc>
    1010:	e12fff1c 	bx	ip
    1014:	02001015 	.word	0x02001015
Disassembly of section .foo:

02001014 <bar>:
 2001014:	4770      	bx	lr
