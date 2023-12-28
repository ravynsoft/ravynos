#source: tls-relax-gdesc-le.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0
#notarget: *-*-nto*
#objdump: -dr
#...
 +10000:	52a00000 	movz	w0, #0x0, lsl #16
 +10004:	72800100 	movk	w0, #0x8
 +10008:	d503201f 	nop
 +1000c:	d503201f 	nop
 +10010:	d53bd041 	mrs	x1, tpidr_el0
 +10014:	8b000020 	add	x0, x1, x0
 +10018:	b9400000 	ldr	w0, \[x0\]
