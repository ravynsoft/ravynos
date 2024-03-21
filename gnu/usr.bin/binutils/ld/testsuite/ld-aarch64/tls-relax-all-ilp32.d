#source: tls-relax-all.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0
#notarget: *-*-nto*
#objdump: -dr
#...
 +10000:	a9bf7bfd 	stp	x29, x30, \[sp, #-16\]!
 +10004:	910003fd 	mov	x29, sp
 +10008:	52a00000 	movz	w0, #0x0, lsl #16
 +1000c:	72800100 	movk	w0, #0x8
 +10010:	d503201f 	nop
 +10014:	d503201f 	nop
 +10018:	d53bd041 	mrs	x1, tpidr_el0
 +1001c:	8b000020 	add	x0, x1, x0
 +10020:	b9400001 	ldr	w1, \[x0\]
 +10024:	52a00000 	movz	w0, #0x0, lsl #16
 +10028:	72800180 	movk	w0, #0xc
 +1002c:	d503201f 	nop
 +10030:	d503201f 	nop
 +10034:	d53bd042 	mrs	x2, tpidr_el0
 +10038:	8b000040 	add	x0, x2, x0
 +1003c:	b9400000 	ldr	w0, \[x0\]
 +10040:	0b000021 	add	w1, w1, w0
 +10044:	52a00000 	movz	w0, #0x0, lsl #16
 +10048:	72800200 	movk	w0, #0x10
 +1004c:	d53bd041 	mrs	x1, tpidr_el0
 +10050:	0b000020 	add	w0, w1, w0
 +10054:	b9400000 	ldr	w0, \[x0\]
 +10058:	0b000021 	add	w1, w1, w0
 +1005c:	52a00000 	movz	w0, #0x0, lsl #16
 +10060:	72800280 	movk	w0, #0x14
 +10064:	d53bd041 	mrs	x1, tpidr_el0
 +10068:	0b000020 	add	w0, w1, w0
 +1006c:	b9400000 	ldr	w0, \[x0\]
 +10070:	0b000021 	add	w1, w1, w0
 +10074:	d53bd042 	mrs	x2, tpidr_el0
 +10078:	52a00000 	movz	w0, #0x0, lsl #16
 +1007c:	72800300 	movk	w0, #0x18
 +10080:	8b000040 	add	x0, x2, x0
 +10084:	b9400000 	ldr	w0, \[x0\]
 +10088:	0b000020 	add	w0, w1, w0
