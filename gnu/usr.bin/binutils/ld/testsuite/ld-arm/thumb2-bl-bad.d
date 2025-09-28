.*:     file format .*

Disassembly of section .text:

00001000 <_start>:
    1000:	f000 e802 	blx	1008 <__bar_veneer>
    1004:	0000      	movs	r0, r0
	\.\.\.

00001008 <__bar_veneer>:
    1008:	e51ff004 	ldr	pc, \[pc, #-4\]	@ 100c <__bar_veneer\+0x4>
    100c:	0100100d 	.word	0x0100100d
Disassembly of section .foo:

0100100c <bar>:
 100100c:	4770      	bx	lr
