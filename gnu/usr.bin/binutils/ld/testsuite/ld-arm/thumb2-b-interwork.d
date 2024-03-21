
.*thumb2-b-interwork:     file format elf32-.*arm.*

Disassembly of section .text:

[0-9a-f]+ <_start>:
 +[0-9a-f]+:	f000 b802 	b.w	[0-9a-f]+ <__bar_from_thumb>

[0-9a-f]+ <bar>:
 +[0-9a-f]+:	e12fff1e 	bx	lr

[0-9a-f]+ <__bar_from_thumb>:
 +[0-9a-f]+:	4778      	bx	pc
 +[0-9a-f]+:	e7fd      	b.n	.+ <.+>
 +[0-9a-f]+:	eafffffc 	b	[0-9a-f]+ <bar>

