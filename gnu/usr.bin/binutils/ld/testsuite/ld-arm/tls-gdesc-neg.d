
.*:     file format .*
architecture: armv.*, flags 0x[0-9a-f]+:
HAS_SYMS, DYNAMIC, D_PAGED
start address 0x[0-9a-f]+

Disassembly of section .plt:

00008164 <.plt>:
    8164:	e52de004 	push	{lr}		@ .*
    8168:	e59fe004 	ldr	lr, \[pc, #4\]	@ .*
    816c:	e08fe00e 	add	lr, pc, lr
    8170:	e5bef008 	ldr	pc, \[lr, #8\]!
    8174:	000080d8 	.word	0x000080d8
    8178:	e08e0000 	add	r0, lr, r0
    817c:	e5901004 	ldr	r1, \[r0, #4\]
    8180:	e12fff11 	bx	r1
    8184:	e52d2004 	push	{r2}		@ .*
    8188:	e59f200c 	ldr	r2, \[pc, #12\]	@ .*
    818c:	e59f100c 	ldr	r1, \[pc, #12\]	@ .*
    8190:	e79f2002 	ldr	r2, \[pc, r2\]
    8194:	e081100f 	add	r1, r1, pc
    8198:	e12fff12 	bx	r2
    819c:	000080c8 	.word	0x000080c8
    81a0:	000080b0 	.word	0x000080b0

Disassembly of section .text:

000081a4 <foo>:
    81a4:	e59f0000 	ldr	r0, \[pc\]	@ .*
    81a8:	ea000000 	b	81b0 <foo\+0xc>
    81ac:	000080a4 	.word	0x000080a4
    81b0:	fafffff0 	blx	8178 <.plt\+0x14>

000081b4 <bar>:
    81b4:	4800      	ldr	r0, \[pc, #0\]	@ .*
    81b6:	e001      	b.n	81bc <bar\+0x8>
    81b8:	00008097 	.word	0x00008097
    81bc:	f7ff efdc 	blx	8178 <.plt\+0x14>
