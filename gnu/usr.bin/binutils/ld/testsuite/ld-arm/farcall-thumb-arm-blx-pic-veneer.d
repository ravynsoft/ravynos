.*:     file format .*

Disassembly of section .text:

01c01010 <_start>:
 1c01010:	f300 e802 	blx	1f01018 <__bar_from_thumb>
	\.\.\.
 1f01014:	f0ff effe 	blx	2001014 <bar>

01f01018 <__bar_from_thumb>:
 1f01018:	e59fc000 	ldr	ip, \[pc\]	@ 1f01020 <__bar_from_thumb\+0x8>
 1f0101c:	e08ff00c 	add	pc, pc, ip
 1f01020:	000ffff0 	.word	0x000ffff0
 1f01024:	00000000 	.word	0x00000000
Disassembly of section .foo:

02001014 <bar>:
 2001014:	e12fff1e 	bx	lr
