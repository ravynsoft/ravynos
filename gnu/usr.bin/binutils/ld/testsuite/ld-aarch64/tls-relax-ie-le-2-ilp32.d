#source: tls-relax-ie-le-2.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0
#notarget: *-*-nto*
#objdump: -dr
#...
 +10000:	d53bd041 	mrs	x1, tpidr_el0
 +10004:	d503201f 	nop
 +10008:	d503201f 	nop
 +1000c:	52a00000 	movz	w0, #0x0, lsl #16
 +10010:	d503201f 	nop
 +10014:	d503201f 	nop
 +10018:	d503201f 	nop
 +1001c:	72800100 	movk	w0, #0x8
 +10020:	d503201f 	nop
 +10024:	8b000020 	add	x0, x1, x0
 +10028:	d503201f 	nop
 +1002c:	d503201f 	nop
 +10030:	b9400000 	ldr	w0, \[x0\]
