.*:     file format elf32-.*arm
architecture: armv6t2, flags 0x00000112:
EXEC_P, HAS_SYMS, D_PAGED
start address 0x.*

Disassembly of section .plt:

0000819c <.plt>:
    819c:	e52de004 	push	{lr}		@ .*
    81a0:	e59fe004 	ldr	lr, \[pc, #4\]	@ .*
    81a4:	e08fe00e 	add	lr, pc, lr
    81a8:	e5bef008 	ldr	pc, \[lr, #8\]!
    81ac:	00008100 	.word	0x00008100
    81b0:	e08e0000 	add	r0, lr, r0
    81b4:	e5901004 	ldr	r1, \[r0, #4]
    81b8:	e12fff11 	bx	r1
    81bc:	e52d2004 	push	{r2}		@ .*
    81c0:	e59f200c 	ldr	r2, \[pc, #12\]	@ .*
    81c4:	e59f100c 	ldr	r1, \[pc, #12\]	@ .*
    81c8:	e79f2002 	ldr	r2, \[pc, r2\]
    81cc:	e081100f 	add	r1, r1, pc
    81d0:	e12fff12 	bx	r2
    81d4:	000080f4 	.word	0x000080f4
    81d8:	000080d8 	.word	0x000080d8

Disassembly of section .text:

000081dc <text>:
    81dc:	e59f0004 	ldr	r0, \[pc, #4\]	@ .*
    81e0:	fafffff2 	blx	81b0 .*
    81e4:	e1a00000 	nop			@ .*
    81e8:	000080d4 	.word	0x000080d4
    81ec:	4801      	ldr	r0, \[pc, #4\]	@ .*
    81ee:	f7ff efe0 	blx	81b0 .*
    81f2:	bf00      	nop
    81f4:	000080c5 	.word	0x000080c5

Disassembly of section .foo:

04001000 <foo>:
 4001000:	e59f0004 	ldr	r0, \[pc, #4\]	@ .*
 4001004:	e79f0000 	ldr	r0, \[pc, r0\]
 4001008:	e1a00000 	nop			@ .*
 400100c:	fc00f2b4 	.word	0xfc00f2b4
 4001010:	e59f0004 	ldr	r0, \[pc, #4\]	@ .*
 4001014:	fa000005 	blx	4001030 .*
 4001018:	e1a00000 	nop			@ .*
 400101c:	fc00f2a0 	.word	0xfc00f2a0
 4001020:	4801      	ldr	r0, \[pc, #4\]	@ .*
 4001022:	f000 f809 	bl	4001038 .*
 4001026:	bf00      	nop
 4001028:	fc00f291 	.word	0xfc00f291
 400102c:	00000000 	.word	0x00000000

04001030 <__unnamed_veneer>:
 4001030:	e51ff004 	ldr	pc, \[pc, #-4\]	@ .*
 4001034:	000081b0 	.word	0x000081b0

04001038 <__unnamed_veneer>:
 4001038:	4778      	bx	pc
 400103a:	e7fd      	b.n	.+ <.+>
 400103c:	e51ff004 	ldr	pc, \[pc, #-4\]	@ .*
 4001040:	000081b0 	.word	0x000081b0
 4001044:	00000000 	.word	0x00000000
