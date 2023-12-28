#source: tls-relax-ie-le.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0
#notarget: *-*-nto*
#objdump: -dr
#...
 +10000:	d53bd041 	mrs	x1, tpidr_el0
 +10004:	52a00000 	movz	w0, #0x0, lsl #16
 +10008:	72800100 	movk	w0, #0x8
 +1000c:	8b000020 	add	x0, x1, x0
 +10010:	b9400000 	ldr	w0, \[x0\]
