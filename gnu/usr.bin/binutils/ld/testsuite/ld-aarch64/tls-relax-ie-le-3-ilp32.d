#source: tls-relax-ie-le-3.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0
#notarget: *-*-nto*
#objdump: -dr
#...
 +10000:	d53bd042 	mrs	x2, tpidr_el0
 +10004:	52a0000f 	movz	w15, #0x0, lsl #16
 +10008:	7280010f 	movk	w15, #0x8
 +1000c:	8b0f004f 	add	x15, x2, x15
 +10010:	b94001e0 	ldr	w0, \[x15\]
