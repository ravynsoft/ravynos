.*:     file format .*

Disassembly of section .text:

01c01010 <_start>:
 1c01010:	f300 f802 	bl	1f01018 <__bar_from_thumb>
	...
 1f01014:	f000 f800 	bl	1f01018 <__bar_from_thumb>

01f01018 <__bar_from_thumb>:
 1f01018:	4778      	bx	pc
 1f0101a:	e7fd      	b.n	.+ <.+>
 1f0101c:	e59fc000 	ldr	ip, \[pc\]	@ 1f01024 <__bar_from_thumb\+0xc>
 1f01020:	e08cf00f 	add	pc, ip, pc
 1f01024:	000fffec 	.word	0x000fffec

Disassembly of section .foo:

02001014 <bar>:
 2001014:	e12fff1e 	bx	lr
