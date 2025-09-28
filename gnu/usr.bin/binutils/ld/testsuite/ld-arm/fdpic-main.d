
tmpdir/fdpic-main:     file format elf32-(little|big)arm
architecture: arm.*, flags 0x00000112:
EXEC_P, HAS_SYMS, D_PAGED
start address 0x.*

Disassembly of section .plt:

.* <.plt>:
    .*:	e59fc008 	ldr	ip, \[pc, #8\]	@ .* <.plt\+0x10>
    .*:	e08cc009 	add	ip, ip, r9
    .*:	e59c9004 	ldr	r9, \[ip, #4\]
    .*:	e59cf000 	ldr	pc, \[ip\]
    .*:	0000000c 	.word	0x0000000c
    .*:	00000000 	.word	0x00000000
    .*:	e51fc00c 	ldr	ip, \[pc, #-12\]	@ .* <.plt\+0x14>
    .*:	e92d1000 	stmfd	sp!, {ip}
    .*:	e599c004 	ldr	ip, \[r9, #4\]
    .*:	e599f000 	ldr	pc, \[r9\]
    .*:	e59fc008 	ldr	ip, \[pc, #8\]	@ .* <.plt\+0x38>
    .*:	e08cc009 	add	ip, ip, r9
    .*:	e59c9004 	ldr	r9, \[ip, #4\]
    .*:	e59cf000 	ldr	pc, \[ip]
    .*:	00000014 	.word	0x00000014
    .*:	00000008 	.word	0x00000008
    .*:	e51fc00c 	ldr	ip, \[pc, #-12\]	@ .* <.plt\+0x3c>
    .*:	e92d1000 	stmfd	sp!, {ip}
    .*:	e599c004 	ldr	ip, \[r9, #4\]
    .*:	e599f000 	ldr	pc, \[r9\]

Disassembly of section .text:

.* <_start>:
    .*:	eaffffff 	b	.* <main>

.* <main>:
    .*:	e59f206c 	ldr	r2, \[pc, #108\]	@ .* <main\+0x74>
    .*:	e59f306c 	ldr	r3, \[pc, #108\]	@ .* <main\+0x78>
    .*:	e92d4070 	push	{r4, r5, r6, lr}
    .*:	e7995002 	ldr	r5, \[r9, r2\]
    .*:	e1a04009 	mov	r4, r9
    .*:	e7993003 	ldr	r3, \[r9, r3\]
    .*:	e5853000 	str	r3, \[r5\]
    .*:	ebffffe2 	bl	.* <.plt>
    .*:	e1a06000 	mov	r6, r0
    .*:	e1a09004 	mov	r9, r4
    .*:	e5950000 	ldr	r0, \[r5\]
    .*:	e1a09004 	mov	r9, r4
    .*:	ebffffe7 	bl	.* <.plt\+0x28>
    .*:	e59f3040 	ldr	r3, \[pc, #64\]	@ .* <main\+0x7c>
    .*:	e1a09004 	mov	r9, r4
    .*:	e0833009 	add	r3, r3, r9
    .*:	e1a00003 	mov	r0, r3
    .*:	e5853000 	str	r3, \[r5\]
    .*:	e1a09004 	mov	r9, r4
    .*:	ebffffe0 	bl	.* <.plt\+0x28>
    .*:	e59f3028 	ldr	r3, \[pc, #40\]	@ .* <main\+0x80>
    .*:	e1a09004 	mov	r9, r4
    .*:	e7993003 	ldr	r3, \[r9, r3\]
    .*:	e5930000 	ldr	r0, \[r3\]
    .*:	e1a09004 	mov	r9, r4
    .*:	ebffffda 	bl	.* <.plt\+0x28>
    .*:	e1a00006 	mov	r0, r6
    .*:	e1a09004 	mov	r9, r4
    .*:	e8bd8070 	pop	{r4, r5, r6, pc}
    .*:	0000002c 	.word	0x0000002c
    .*:	00000024 	.word	0x00000024
    .*:	0000001c 	.word	0x0000001c
    .*:	00000028 	.word	0x00000028

.* <my_local_func>:
    .*:	e12fff1e 	bx	lr
