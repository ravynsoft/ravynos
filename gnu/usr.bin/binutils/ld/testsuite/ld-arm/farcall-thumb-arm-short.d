.*:     file format .*

Disassembly of section .text:

00001000 <_start>:
    1000:	f000 f802 	bl	1008 <__bar_from_thumb>
    1004:	0000      	movs	r0, r0
	\.\.\.

00001008 <__bar_from_thumb>:
    1008:	4778      	bx	pc
    100a:	e7fd      	b.n	.+ <.+>
    100c:	ea000400 	b	2014 <bar>
Disassembly of section .foo:

00002014 <bar>:
    2014:	e12fff1e 	bx	lr
