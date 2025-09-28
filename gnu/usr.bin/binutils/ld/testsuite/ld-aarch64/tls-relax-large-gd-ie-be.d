#source: tls-relax-large-gd-ie.s
#ld: -T relocs.ld -e0
#notarget: aarch64-*-*
#objdump: -dr
#...
0000000000010000 <test>:
 +10000:	58000121 	ldr	x1, 10024 <test\+0x24>
 +10004:	10000102 	adr	x2, 10024 <test\+0x24>
 +10008:	8b010041 	add	x1, x2, x1
 +1000c:	d2a00000 	movz	x0, #0x0, lsl #16
 +10010:	f2800100 	movk	x0, #0x8
 +10014:	58000000 	ldr	x0, 10014 <test\+0x14>
 +10018:	d53bd041 	mrs	x1, tpidr_el0
 +1001c:	8b000020 	add	x0, x1, x0
 +10020:	b9400000 	ldr	w0, \[x0\]
 +10024:	00000000 	.word	0x00000000
 +10028:	0000ffdc 	.word	0x0000ffdc
