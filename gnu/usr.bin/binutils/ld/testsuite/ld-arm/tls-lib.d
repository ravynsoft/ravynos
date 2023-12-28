
.*:     file format elf32-.*arm.*
architecture: arm.*, flags 0x00000150:
HAS_SYMS, DYNAMIC, D_PAGED
start address 0x.*

Disassembly of section .text:

.* <foo>:
 .*:	e1a00000 	nop			@ \(mov r0, r0\)
 .*:	e1a00000 	nop			@ \(mov r0, r0\)
 .*:	e1a0f00e 	mov	pc, lr
 .*:	00010098 	.word	0x00010098
 .*:	0001008c 	.word	0x0001008c
 .*:	00000004 	.word	0x00000004
