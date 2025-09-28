#source: tls-relax-gd-le.s
#ld: -T relocs.ld -e0
#objdump: -dr
#...
 +10000:	d2a00000 	movz	x0, #0x0, lsl #16
 +10004:	f2800200 	movk	x0, #0x10
 +10008:	d53bd041 	mrs	x1, tpidr_el0
 +1000c:	8b000020 	add	x0, x1, x0
 +10010:	b9400000 	ldr	w0, \[x0\]
