.*:     file format elf32-.*arm
architecture: arm.*, flags 0x00000150:
HAS_SYMS, DYNAMIC, D_PAGED
start address 0x.*

Disassembly of section .plt:

0000813c <.plt>:
.*:	e52de004 	push	{lr}		@ .*
.*:	e59fe004 	ldr	lr, \[pc, #4\]	@ .*
.*:	e08fe00e 	add	lr, pc, lr
.*:	e5bef008 	ldr	pc, \[lr, #8\]!
.*:	000080f0 	.word	0x000080f0
.*:	e08e0000 	add	r0, lr, r0
.*:	e5901004 	ldr	r1, \[r0, #4\]
.*:	e12fff11 	bx	r1
.*:	e52d2004 	push	{r2}		@ .*
.*:	e59f200c 	ldr	r2, \[pc, #12\]	@ .*
.*:	e59f100c 	ldr	r1, \[pc, #12\]	@ .*
.*:	e79f2002 	ldr	r2, \[pc, r2\]
.*:	e081100f 	add	r1, r1, pc
.*:	e12fff12 	bx	r2
.*:	000080e8 	.word	0x000080e8
.*:	000080c8 	.word	0x000080c8

Disassembly of section .text:

00008180 <text>:
.*:	e59f0004 	ldr	r0, \[pc, #4\]	@ .*
.*:	ebfffff1 	bl	.* <\.plt\+0x14>
.*:	e1a00000 	nop			@ .*
.*:	000080c0 	.word	0x000080c0
.*:	4801      	ldr	r0, \[pc, #4\]	@ .*
.*:	f000 f805 	bl	.* <__unnamed_veneer>
.*:	46c0      	nop			@ .*
.*:	000080b1 	.word	0x000080b1
.*:	00000000 	.word	0x00000000

000081a0 <__unnamed_veneer>:
.*:	4778      	bx	pc
.*:	e7fd      	b.n	.+ <.+>
.*:	e59f1000 	ldr	r1, \[pc\]	@ .*
.*:	e081f00f 	add	pc, r1, pc
.*:	ffffffa0 	.word	0xffffffa0

Disassembly of section .foo:

04001000 <foo>:
.*:	e59f0004 	ldr	r0, \[pc, #4\]	@ .*
.*:	eb000009 	bl	4001030 .*
.*:	e1a00000 	nop			@ .*
.*:	fc00f240 	.word	0xfc00f240
.*:	e59f0004 	ldr	r0, \[pc, #4\]	@ .*
.*:	eb000005 	bl	4001030 .*
.*:	e1a00000 	nop			@ .*
.*:	fc00f238 	.word	0xfc00f238
.*:	4801      	ldr	r0, \[pc, #4\]	@ .*
.*:	f000 f80b 	bl	400103c .*
.*:	46c0      	nop			@ .*
.*:	fc00f221 	.word	0xfc00f221
.*:	00000000 	.word	0x00000000

04001030 <__unnamed_veneer>:
.*:	e59f1000 	ldr	r1, \[pc\]	@ .*
.*:	e08ff001 	add	pc, pc, r1
.*:	fc007114 	.word	0xfc007114

0400103c <__unnamed_veneer>:
.*:	4778      	bx	pc
.*:	e7fd      	b.n	.+ <.+>
.*:	e59f1000 	ldr	r1, \[pc\]	@ .*
.*:	e081f00f 	add	pc, r1, pc
.*:	fc007104 	.word	0xfc007104
.*:	00000000 	.word	0x00000000
