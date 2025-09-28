#source: tls-relax-ie-le-2.s
#ld: -T relocs.ld -e0
#objdump: -dr
#...
 +10000:	d53bd041 	mrs	x1, tpidr_el0
 +10004:	d503201f 	nop
 +10008:	d503201f 	nop
 +1000c:	d2a00000 	movz	x0, #0x0, lsl #16
 +10010:	d503201f 	nop
 +10014:	d503201f 	nop
 +10018:	d503201f 	nop
 +1001c:	f2800200 	movk	x0, #0x10
 +10020:	d503201f 	nop
 +10024:	8b000020 	add	x0, x1, x0
 +10028:	d503201f 	nop
 +1002c:	d503201f 	nop
 +10030:	b9400000 	ldr	w0, \[x0\]
