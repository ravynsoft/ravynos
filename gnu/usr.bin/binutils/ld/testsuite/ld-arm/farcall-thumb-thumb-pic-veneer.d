.*:     file format .*

Disassembly of section .text:

00001000 <_start>:
    1000:	f000 f802 	bl	1008 <__bar_veneer>
    1004:	0000      	movs	r0, r0
	...

00001008 <__bar_veneer>:
    1008:	4778      	bx	pc
    100a:	e7fd      	b.n	.+ <.+>
    100c:	e59fc004 	ldr	ip, \[pc, #4\]	@ 1018 <__bar_veneer\+0x10>
    1010:	e08fc00c 	add	ip, pc, ip
    1014:	e12fff1c 	bx	ip
    1018:	01fffffd 	.word	0x01fffffd
    101c:	00000000 	.word	0x00000000

Disassembly of section .foo:

02001014 <bar>:
 2001014:	4770      	bx	lr
