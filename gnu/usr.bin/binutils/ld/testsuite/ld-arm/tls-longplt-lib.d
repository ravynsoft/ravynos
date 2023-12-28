.*:     file format elf32-.*arm
architecture: armv6t2, flags 0x00000150:
HAS_SYMS, DYNAMIC, D_PAGED
start address 0x.*

Disassembly of section .plt:

00008170 <.plt>:
.*:	e52de004 	push	{lr}		@ .*
.*:	e59fe004 	ldr	lr, \[pc, #4\]	@ .*
.*:	e08fe00e 	add	lr, pc, lr
.*:	e5bef008 	ldr	pc, \[lr, #8\]!
.*:	000080e0 	.word	0x000080e0
.*:	e08e0000 	add	r0, lr, r0
.*:	e5901004 	ldr	r1, \[r0, #4\]
.*:	e12fff11 	bx	r1
.*:	e52d2004 	push	{r2}		@ .*
.*:	e59f200c 	ldr	r2, \[pc, #12\]	@ .*
.*:	e59f100c 	ldr	r1, \[pc, #12\]	@ .*
.*:	e79f2002 	ldr	r2, \[pc, r2\]
.*:	e081100f 	add	r1, r1, pc
.*:	e12fff12 	bx	r2
.*:	000080d8 	.word	0x000080d8
.*:	000080b8 	.word	0x000080b8

Disassembly of section .text:

000081b0 <text>:
.*:	e59f0004 	ldr	r0, \[pc, #4\]	@ .*
.*:	fafffff2 	blx	.* <\.plt\+0x14>
.*:	e1a00000 	nop			@ .*
.*:	000080b4 	.word	0x000080b4
.*:	4801      	ldr	r0, \[pc, #4\]	@ .*
.*:	f7ff efe0 	blx	.* <\.plt\+0x14>
.*:	bf00      	nop
.*:	000080a5 	.word	0x000080a5

Disassembly of section .foo:

04001000 <foo>:
.*:	e59f0004 	ldr	r0, \[pc, #4\]	@ .*
.*:	fa000009 	blx	4001030 .*
.*:	e1a00000 	nop			@ .*
.*:	fc00f264 	.word	0xfc00f264
.*:	e59f0004 	ldr	r0, \[pc, #4\]	@ .*
.*:	fa000005 	blx	4001030 .*
.*:	e1a00000 	nop			@ .*
.*:	fc00f25c 	.word	0xfc00f25c
.*:	4801      	ldr	r0, \[pc, #4\]	@ .*
.*:	f000 e806 	blx	4001030 .*
.*:	bf00      	nop
.*:	fc00f245 	.word	0xfc00f245
.*:	00000000 	.word	0x00000000

04001030 <__unnamed_veneer>:
.*:	e59f1000 	ldr	r1, \[pc\]	@ .*
.*:	e08ff001 	add	pc, pc, r1
.*:	fc007148 	.word	0xfc007148
.*:	00000000 	.word	0x00000000
