.*:     file format .*

Disassembly of section .text:

00001000 <_start>:
    1000:	f000 f802 	bl	1008 <__bar_veneer>
    1004:	0000      	movs	r0, r0
	...

00001008 <__bar_veneer>:
    1008:	b401      	push	{r0}
    100a:	4802      	ldr	r0, \[pc, #8\]	@ \(1014 <__bar_veneer\+0xc>\)
    100c:	46fc      	mov	ip, pc
    100e:	4484      	add	ip, r0
    1010:	bc01      	pop	{r0}
    1012:	4760      	bx	ip
    1014:	02000005 	.word	0x02000005

Disassembly of section .foo:

02001014 <bar>:
 2001014:	4770      	bx	lr
