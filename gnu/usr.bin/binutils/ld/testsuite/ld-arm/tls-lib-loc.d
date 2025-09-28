
.*:     file format elf32-.*arm
architecture: arm.*, flags 0x00000150:
HAS_SYMS, DYNAMIC, D_PAGED
start address 0x.*

Disassembly of section .plt:

[0-9a-f]+ <.plt>:
    [0-9a-f]+:	e52de004 	push	{lr}		@ .*
    [0-9a-f]+:	e59fe004 	ldr	lr, \[pc, #4\]	@ 8128 .*
    [0-9a-f]+:	e08fe00e 	add	lr, pc, lr
    [0-9a-f]+:	e5bef008 	ldr	pc, \[lr, #8\]!
    8128:	000080cc 	.word	0x000080cc
    812c:	e08e0000 	add	r0, lr, r0
    [0-9a-f]+:	e5901004 	ldr	r1, \[r0, #4\]
    [0-9a-f]+:	e12fff11 	bx	r1
    [0-9a-f]+:	e52d2004 	push	{r2}		@ .*
    813c:	e59f200c 	ldr	r2, \[pc, #12\]	@ 8150 .*
    [0-9a-f]+:	e59f100c 	ldr	r1, \[pc, #12\]	@ 8154 .*
    [0-9a-f]+:	e79f2002 	ldr	r2, \[pc, r2\]
    [0-9a-f]+:	e081100f 	add	r1, r1, pc
    [0-9a-f]+:	e12fff12 	bx	r2
    8150:	000080bc 	.word	0x000080bc
    8154:	000080a4 	.word	0x000080a4

Disassembly of section .text:

[0-9a-f]+ <foo>:
    [0-9a-f]+:	e59f0004 	ldr	r0, \[pc, #4\]	@ 8164 .*
    [0-9a-f]+:	fafffff2 	blx	812c <.*>
    [0-9a-f]+:	e1a00000 	nop			@ .*
    8164:	000080a0 	.word	0x000080a0
