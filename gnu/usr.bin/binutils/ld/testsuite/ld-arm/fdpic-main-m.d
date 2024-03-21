
tmpdir/fdpic-main:     file format elf32-(little|big)arm
architecture: arm.*, flags 0x00000112:
EXEC_P, HAS_SYMS, D_PAGED
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
    .*:	f8df c00c 	ldr.w	ip, \[pc, #12\]	@ .* <.plt\+0x38>
    .*:	eb0c 0c09 	add.w	ip, ip, r9
    .*:	f8dc 9004 	ldr.w	r9, \[ip, #4\]
    .*:	f8dc f000 	ldr.w	pc, \[ip]
    .*:	00000014 	.word	0x00000014
    .*:	00000008 	.word	0x00000008
    .*:	f85f c008 	ldr.w	ip, \[pc, #-8\]	@ .* <.plt\+0x3c>
    .*:	f84d cd04 	str.w	ip, \[sp, #-4\]!
    .*:	f8d9 c004 	ldr.w	ip, \[r9, #4\]
    .*:	f8d9 f000 	ldr.w	pc, \[r9\]

Disassembly of section .text:

.* <_start>:
    .*:	f000 b800 	b.w	.* <main>

.* <main>:
    .*:	4a11      	ldr	r2, \[pc, #68\]	@ .* <main\+0x48>.*
    .*:	4b12      	ldr	r3, \[pc, #72\]	@ .* <main\+0x4c>.*
    .*:	b570      	push	{r4, r5, r6, lr}
    .*:	f859 5002 	ldr.w	r5, \[r9, r2\]
    .*:	464c      	mov	r4, r9
    .*:	f859 3003 	ldr.w	r3, \[r9, r3\]
    .*:	602b      	str	r3, \[r5, #0\]
    .*:	f7ff ffcb 	bl	.* <.plt>
    .*:	4606      	mov	r6, r0
    .*:	46a1      	mov	r9, r4
    .*:	6828      	ldr	r0, \[r5, #0\]
    .*:	46a1      	mov	r9, r4
    .*:	f7ff ffd9 	bl	.* <.plt\+0x28>
    .*:	4b0b      	ldr	r3, \[pc, #44\]	@ .* <main\+0x50>.*
    .*:	46a1      	mov	r9, r4
    .*:	444b      	add	r3, r9
    .*:	4618      	mov	r0, r3
    .*:	602b      	str	r3, \[r5, #0\]
    .*:	46a1      	mov	r9, r4
    .*:	f7ff ffd1 	bl	.* <.plt\+0x28>
    .*:	4b08      	ldr	r3, \[pc, #32\]	@ .* <main\+0x54>.*
    .*:	46a1      	mov	r9, r4
    .*:	f859 3003 	ldr.w	r3, \[r9, r3\]
    .*:	6818      	ldr	r0, \[r3, #0\]
    .*:	46a1      	mov	r9, r4
    .*:	f7ff ffc9 	bl	.* <.plt\+0x28>
    .*:	4630      	mov	r0, r6
    .*:	46a1      	mov	r9, r4
    .*:	bd70      	pop	{r4, r5, r6, pc}
    .*:	0000002c 	.word	0x0000002c
    .*:	00000024 	.word	0x00000024
    .*:	0000001c 	.word	0x0000001c
    .*:	00000028 	.word	0x00000028

.* <my_local_func>:
    .*:	4770      	bx	lr
    .*:	bf00      	nop
