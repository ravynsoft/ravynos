.*:     file format .*

Disassembly of section .text:

00008000 <_start>:
    8000:	ea000000 	b	8008 <__far_veneer>
    8004:	00000000 	andeq	r0, r0, r0

00008008 <__far_veneer>:
    8008:	e51ff004 	ldr	pc, \[pc, #-4\]	@ 800c <__far_veneer\+0x4>
    800c:	12340000 	\.word	0x12340000

00008010 <after>:
    8010:	11111111 	\.word	0x11111111

Disassembly of section \.far:

12340000 <far>:
12340000:	e12fff1e 	bx	lr
