#source: tls-relax-large-gd-le.s
#ld: -T relocs.ld -e0
#notarget: aarch64_be-*-*
#objdump: -dr
#...
0000000000010000 <test>:
 +10000:	58000121 	ldr	x1, 10024 <test\+0x24>
 +10004:	10000102 	adr	x2, 10024 <test\+0x24>
 +10008:	8b010041 	add	x1, x2, x1
 +1000c:	d2c00000 	movz	x0, #0x0, lsl #32
 +10010:	f2a00000 	movk	x0, #0x0, lsl #16
 +10014:	f2800200 	movk	x0, #0x10
 +10018:	d53bd041 	mrs	x1, tpidr_el0
 +1001c:	8b000020 	add	x0, x1, x0
 +10020:	b9400000 	ldr	w0, \[x0\]
 +10024:	0000ffdc 	.word	0x0000ffdc
 +10028:	00000000 	.word	0x00000000
