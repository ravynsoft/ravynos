#source: thumb-plt.s
#name: Thumb only PLT and GOT
#ld: -shared -e0 -z max-page-size=0x10000
#objdump: -dr
#skip: *-*-pe *-*-wince *-*-vxworks armeb-*-* *-*-gnueabihf

.*: +file format .*arm.*


Disassembly of section \.plt:

00000110 <\.plt>:
 110:	b500      	push	{lr}
 112:	f8df e008 	ldr.w	lr, \[pc, #8\]	@ 11c <\.plt\+0xc>
 116:	44fe      	add	lr, pc
 118:	f85e ff08 	ldr.w	pc, \[lr, #8\]!
 11c:	000100(.+) 	\.word	0x000100\1

00000120 <foo@plt>:
 120:	f240 0c.+ 	movw	ip, #[0-9]+	@ 0x.+
 124:	f2c0 0c01 	movt	ip, #1
 128:	44fc      	add	ip, pc
 12a:	f8dc f000 	ldr.w	pc, \[ip\]
 12e:	e7fc      	b.n	12a <foo@plt\+0xa>

Disassembly of section .text:

00000130 <bar>:
 130:	b580      	push	{r7, lr}
 132:	af00      	add	r7, sp, #0
 134:	f7ff fff4 	bl	120 <foo@plt>
 138:	4603      	mov	r3, r0
 13a:	4618      	mov	r0, r3
 13c:	bd80      	pop	{r7, pc}
