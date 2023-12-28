#source: tls-relax-ie-le.s
#ld: -T relocs.ld -e0
#objdump: -dr
#...
 +10000:	d53bd041 	mrs	x1, tpidr_el0
 +10004:	d2a00000 	movz	x0, #0x0, lsl #16
 +10008:	f2800200 	movk	x0, #0x10
 +1000c:	8b000020 	add	x0, x1, x0
 +10010:	b9400000 	ldr	w0, \[x0\]
