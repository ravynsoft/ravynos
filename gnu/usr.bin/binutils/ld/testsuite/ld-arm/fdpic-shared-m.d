
tmpdir/fdpic-shared.so:     file format elf32-(little|big)arm
architecture: arm.*, flags 0x00000150:
HAS_SYMS, DYNAMIC, D_PAGED
start address 0x.*

Disassembly of section .plt:

.* <.plt>:
 .*:	f8df c00c 	ldr.w	ip, \[pc, #12\]	@ .* <.plt\+0x10>
 .*:	eb0c 0c09 	add.w	ip, ip, r9
 .*:	f8dc 9004 	ldr.w	r9, \[ip, #4\]
 .*:	f8dc f000 	ldr.w	pc, \[ip\]
 .*:	0000000c 	.word	0x0000000c
 .*:	00000000 	.word	0x00000000
 .*:	f85f c008 	ldr.w	ip, \[pc, #-8\]	@ .* <.plt\+0x14>
 .*:	f84d cd04 	str.w	ip, \[sp, #-4\]!
 .*:	f8d9 c004 	ldr.w	ip, \[r9, #4\]
 .*:	f8d9 f000 	ldr.w	pc, \[r9\]

Disassembly of section .text:

.* <my_shared_func1>:
 .*:	4770      	bx	lr
 .*:	bf00      	nop

.* <my_shared_func3>:
 .*:	f04f 0000 	mov.w	r0, #0
 .*:	4770      	bx	lr
 .*:	bf00      	nop

.* <my_shared_func2>:
 .*:	b510      	push	{r4, lr}
 .*:	464c      	mov	r4, r9
 .*:	f7ff ffe2 	bl	.* <.plt>
 .*:	46a1      	mov	r9, r4
 .*:	bd10      	pop	{r4, pc}
