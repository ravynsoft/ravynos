
.*:     file format.*

Disassembly of section .text:

00008000 <arm>:
    8000:	e3a00000 	mov	r0, #0
    8004:	e12fff1e 	bx	lr

00008008 <thumb>:
    8008:	46c0      	nop			@ \(mov r8, r8\)
    800a:	4770      	bx	lr
    800c:	f7ff fffc 	bl	8008 <thumb>

00008010 <data>:
    8010:	12345678 	.word	0x12345678
