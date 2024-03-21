.*:     file format .*

Disassembly of section .text:

01c01010 <_start>:
 1c01010:	f300 e802 	blx	1f01018 <__bar_from_thumb>
	\.\.\.
 1f01014:	f0ff effe 	blx	2001014 <bar>

01f01018 <__bar_from_thumb>:
 1f01018:	e51ff004 	ldr	pc, \[pc, #-4\]	@ 1f0101c <__bar_from_thumb\+0x4>
 1f0101c:	02001014 	.word	0x02001014
Disassembly of section .foo:

02001014 <bar>:
 2001014:	e12fff1e 	bx	lr
