
.*: .*file format elf32-(big|little)arm.*

Disassembly of section \.text:

00008000 <_start>:
    8000:	ea000001 	b	800c \<__bx_r14\>
    8004:	ea000003 	b	8018 \<__bx_r0\>
    8008:	0a000002 	beq	8018 \<__bx_r0\>

0000800c <__bx_r14>:
    800c:	e31e0001 	tst	lr, #1
    8010:	01a0f00e 	moveq	pc, lr
    8014:	e12fff1e 	bx	lr

00008018 <__bx_r0>:
    8018:	e3100001 	tst	r0, #1
    801c:	01a0f000 	moveq	pc, r0
    8020:	e12fff10 	bx	r0
