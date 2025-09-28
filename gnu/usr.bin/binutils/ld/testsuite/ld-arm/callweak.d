
.*:     file format.*

Disassembly of section .far:

12340000 <[^>]*>:
12340000:	e1a00000 	nop			@ \(mov r0, r0\)
12340004:	01a00000 	moveq	r0, r0

12340008 <[^>]*>:
12340008:	e000      	b.n	1234000c <[^>]*>
1234000a:	bf00      	nop
1234000c:	2000      	movs	r0, #0
1234000e:	e000      	b.n	12340012 <[^>]*>
12340010:	bf00      	nop
12340012:	4770      	bx	lr

