
tmpdir/fdpic-shared.so:     file format elf32-(little|big)arm
architecture: arm.*, flags 0x00000150:
HAS_SYMS, DYNAMIC, D_PAGED
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

Disassembly of section .text:

.* <my_shared_func1>:
 .*:	e12fff1e 	bx	lr

.* <my_shared_func3>:
 .*:	e3a00000 	mov	r0, #0
 .*:	e12fff1e 	bx	lr

.* <my_shared_func2>:
 .*:	e92d4010 	push	{r4, lr}
 .*:	e1a04009 	mov	r4, r9
 .*:	ebffffef 	bl	.* <.plt>
 .*:	e1a09004 	mov	r9, r4
 .*:	e8bd8010 	pop	{r4, pc}
