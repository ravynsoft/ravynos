
tmpdir/tls-lib2inline.so:     file format elf32-.*arm
architecture: arm.*, flags 0x[0-9a-f]+:
HAS_SYMS, DYNAMIC, D_PAGED
start address 0x[0-9a-f]+

Disassembly of section .plt:

[0-9a-f]+ <.plt>:
    [0-9a-f]+:	e52de004 	push	{lr}		@ .*
    [0-9a-f]+:	e59fe004 	ldr	lr, \[pc, #4\]	@ .*
    [0-9a-f]+:	e08fe00e 	add	lr, pc, lr
    [0-9a-f]+:	e5bef008 	ldr	pc, \[lr, #8\]!
    [0-9a-f]+:	000080e4 	.word	0x000080e4
    [0-9a-f]+:	e08e0000 	add	r0, lr, r0
    [0-9a-f]+:	e5901004 	ldr	r1, \[r0, #4\]
    [0-9a-f]+:	e12fff11 	bx	r1
    [0-9a-f]+:	e52d2004 	push	{r2}		@ .*
    [0-9a-f]+:	e59f200c 	ldr	r2, \[pc, #12\]	@ .*
    [0-9a-f]+:	e59f100c 	ldr	r1, \[pc, #12\]	@ .*
    [0-9a-f]+:	e79f2002 	ldr	r2, \[pc, r2\]
    [0-9a-f]+:	e081100f 	add	r1, r1, pc
    [0-9a-f]+:	e12fff12 	bx	r2
    [0-9a-f]+:	000080d4 	.word	0x000080d4
    [0-9a-f]+:	000080bc 	.word	0x000080bc

Disassembly of section .text:

[0-9a-f]+ <foo>:
    [0-9a-f]+:	e59f000c 	ldr	r0, \[pc, #12\]	@ .*
    [0-9a-f]+:	e08f0000 	add	r0, pc, r0
    [0-9a-f]+:	e5901004 	ldr	r1, \[r0, #4\]
    [0-9a-f]+:	e12fff31 	blx	r1
    [0-9a-f]+:	e1a00000 	nop			@ .*
    [0-9a-f]+:	000080b4 	.word	0x000080b4

[0-9a-f]+ <bar>:
    [0-9a-f]+:	4802      	ldr	r0, \[pc, #8\]	@ .*
    [0-9a-f]+:	4478      	add	r0, pc
    [0-9a-f]+:	6841      	ldr	r1, \[r0, #4\]
    [0-9a-f]+:	4788      	blx	r1
    [0-9a-f]+:	46c0      	nop			@ .*
    [0-9a-f]+:	46c0      	nop			@ .*
    [0-9a-f]+:	000080a2 	.word	0x000080a2
