.*:     file format .*

Disassembly of section .text:

01c01010 <_start>:
 1c01010:	f300 f802 	bl	1f01018 <__bar_from_thumb>
	\.\.\.
 1f01014:	f000 f806 	bl	1f01024 <__bar_from_thumb>

01f01018 <__bar_from_thumb>:
 1f01018:	4778      	bx	pc
 1f0101a:	e7fd      	b.n	.+ <.+>
 1f0101c:	e51ff004 	ldr	pc, \[pc, #-4\]	@ 1f01020 <__bar_from_thumb\+0x8>
 1f01020:	02001014 	.word	0x02001014

01f01024 <__bar_from_thumb>:
 1f01024:	4778      	bx	pc
 1f01026:	e7fd      	b.n	.+ <.+>
 1f01028:	ea03fff9 	b	2001014 <bar>
 1f0102c:	00000000 	andeq	r0, r0, r0

Disassembly of section .foo:

02001014 <bar>:
 2001014:	e12fff1e 	bx	lr
