
tmpdir/tls-app-rel-le:     file format elf32-.*arm.*
architecture: arm.*, flags 0x[0-9a-f]+:
EXEC_P, HAS_SYMS, D_PAGED
start address 0x[0-9a-f]+

Disassembly of section .text:

[0-9a-f]+ <foo>:
    [0-9a-f]+:	e1a00000 	nop			@ .*
    [0-9a-f]+:	e1a00000 	nop			@ .*
    [0-9a-f]+:	e1a0f00e 	mov	pc, lr
    [0-9a-f]+:	00000008 	.word	0x00000008
