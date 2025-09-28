
tmpdir/tls-app-rel-ie:     file format elf32-.*arm.*
architecture: arm.*, flags 0x[0-9a-f]+:
EXEC_P, HAS_SYMS, D_PAGED
start address 0x[0-9a-f]+

Disassembly of section .text:

[0-9a-f]+ <foo>:
    [0-9a-f]+:	e59f0004 	ldr	r0, \[pc, #4\]	@ .*
    [0-9a-f]+:	e79f0000 	ldr	r0, \[pc, r0\]
    [0-9a-f]+:	e1a00000 	nop			@ .*
    [0-9a-f]+:	00008020 	.word	0x00008020

[0-9a-f]+ <bar>:
    [0-9a-f]+:	4801      	ldr	r0, \[pc, #4\]	@ .*
    [0-9a-f]+:	4478      	add	r0, pc
    [0-9a-f]+:	6800      	ldr	r0, \[r0, #0\]
    [0-9a-f]+:	46c0      	nop			@ .*
    [0-9a-f]+:	00008016 	.word	0x00008016
