#source: tls-relax-ie-le-3.s
#ld: -T relocs.ld -e0
#objdump: -dr
#...
 +10000:	d53bd042 	mrs	x2, tpidr_el0
 +10004:	d2a0000f 	movz	x15, #0x0, lsl #16
 +10008:	f280020f 	movk	x15, #0x10
 +1000c:	8b0f004f 	add	x15, x2, x15
 +10010:	b94001e0 	ldr	w0, \[x15\]
