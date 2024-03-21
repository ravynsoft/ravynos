
.*:     file format .*

Disassembly of section .text:

00001000 <_start>:
    1000:	eb000000 	bl	1008 <__bar_veneer>
    1004:	00000000 	andeq	r0, r0, r0

00001008 <__bar_veneer>:
    1008:	e51ff004 	ldr	pc, \[pc, #-4\]	@ 100c <__bar_veneer\+0x4>
    100c:	02003020 	.word	0x02003020

00001010 <myfunc>:
	...
 2001010:	eb000802 	bl	2003020 <bar>

Disassembly of section .far:

02003020 <bar>:
 2003020:	e12fff1e 	bx	lr
