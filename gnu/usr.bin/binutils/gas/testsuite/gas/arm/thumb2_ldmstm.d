# name: Thumb-2 LDM/STM
# as: -march=armv6t2
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> bc01      	pop	{r0}
0[0-9a-f]+ <[^>]+> f85d 8b04 	ldr.w	r8, \[sp\], #4
0[0-9a-f]+ <[^>]+> f8d1 9000 	ldr.w	r9, \[r1\]
0[0-9a-f]+ <[^>]+> f852 cb04 	ldr.w	ip, \[r2\], #4
0[0-9a-f]+ <[^>]+> f85d 2d04 	ldr.w	r2, \[sp, #-4\]!
0[0-9a-f]+ <[^>]+> f85d 8d04 	ldr.w	r8, \[sp, #-4\]!
0[0-9a-f]+ <[^>]+> f856 4c04 	ldr.w	r4, \[r6, #-4\]
0[0-9a-f]+ <[^>]+> f856 8c04 	ldr.w	r8, \[r6, #-4\]
0[0-9a-f]+ <[^>]+> f852 4d04 	ldr.w	r4, \[r2, #-4\]!
0[0-9a-f]+ <[^>]+> f852 cd04 	ldr.w	ip, \[r2, #-4\]!
0[0-9a-f]+ <[^>]+> b408      	push	{r3}
0[0-9a-f]+ <[^>]+> f84d 9b04 	str.w	r9, \[sp\], #4
0[0-9a-f]+ <[^>]+> f8c3 c000 	str.w	ip, \[r3\]
0[0-9a-f]+ <[^>]+> f844 cb04 	str.w	ip, \[r4\], #4
0[0-9a-f]+ <[^>]+> f84d 3d04 	str.w	r3, \[sp, #-4\]!
0[0-9a-f]+ <[^>]+> f84d 9d04 	str.w	r9, \[sp, #-4\]!
0[0-9a-f]+ <[^>]+> f847 5c04 	str.w	r5, \[r7, #-4\]
0[0-9a-f]+ <[^>]+> f846 cc04 	str.w	ip, \[r6, #-4\]
0[0-9a-f]+ <[^>]+> f846 bd04 	str.w	fp, \[r6, #-4\]!
0[0-9a-f]+ <[^>]+> f845 8d04 	str.w	r8, \[r5, #-4\]!
0[0-9a-f]+ <[^>]+> c80e      	ldmia	r0!, {r1, r2, r3}
0[0-9a-f]+ <[^>]+> c80f      	ldmia	r0, {r0, r1, r2, r3}
0[0-9a-f]+ <[^>]+> c802      	ldmia	r0!, {r1}
0[0-9a-f]+ <[^>]+> e890 0f00 	ldmia.w	r0, {r8, r9, sl, fp}
0[0-9a-f]+ <[^>]+> e8b0 000e 	ldmia.w	r0!, {r1, r2, r3}
0[0-9a-f]+ <[^>]+> e8b0 0f00 	ldmia.w	r0!, {r8, r9, sl, fp}
0[0-9a-f]+ <[^>]+> e8b0 5000 	ldmia.w	r0!, {ip, lr}
0[0-9a-f]+ <[^>]+> e8b0 9000 	ldmia.w	r0!, {ip, pc}
0[0-9a-f]+ <[^>]+> bf08      	it	eq
0[0-9a-f]+ <[^>]+> e8b0 9000 	ldmiaeq.w	r0!, {ip, pc}
0[0-9a-f]+ <[^>]+> c00f      	stmia	r0!, {r0, r1, r2, r3}
0[0-9a-f]+ <[^>]+> c0f0      	stmia	r0!, {r4, r5, r6, r7}
0[0-9a-f]+ <[^>]+> e8a0 00f0 	stmia.w	r0!, {r4, r5, r6, r7}
0[0-9a-f]+ <[^>]+> e8a0 0f00 	stmia.w	r0!, {r8, r9, sl, fp}
0[0-9a-f]+ <[^>]+> e880 000f 	stmia.w	r0, {r0, r1, r2, r3}
0[0-9a-f]+ <[^>]+> e880 0f00 	stmia.w	r0, {r8, r9, sl, fp}
0[0-9a-f]+ <[^>]+> f850 1b04 	ldr.w	r1, \[r0\], #4
0[0-9a-f]+ <[^>]+> f8d0 1000 	ldr.w	r1, \[r0\]
0[0-9a-f]+ <[^>]+> f858 9b04 	ldr.w	r9, \[r8\], #4
0[0-9a-f]+ <[^>]+> f8d8 9000 	ldr.w	r9, \[r8\]
0[0-9a-f]+ <[^>]+> f840 1b04 	str.w	r1, \[r0\], #4
0[0-9a-f]+ <[^>]+> 6001      	str	r1, \[r0, #0\]
0[0-9a-f]+ <[^>]+> 680a      	ldr	r2, \[r1, #0\]
0[0-9a-f]+ <[^>]+> 6807      	ldr	r7, \[r0, #0\]
0[0-9a-f]+ <[^>]+> 9700      	str	r7, \[sp, #0\]
0[0-9a-f]+ <[^>]+> 9000      	str	r0, \[sp, #0\]
0[0-9a-f]+ <[^>]+> 9f00      	ldr	r7, \[sp, #0\]
0[0-9a-f]+ <[^>]+> 9800      	ldr	r0, \[sp, #0\]
0[0-9a-f]+ <[^>]+> f848 9b04 	str.w	r9, \[r8\], #4
0[0-9a-f]+ <[^>]+> f8c8 9000 	str.w	r9, \[r8\]
#pass
