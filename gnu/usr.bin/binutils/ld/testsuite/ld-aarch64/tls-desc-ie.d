#source: tls-desc-ie.s
#target: [check_shared_lib_support]
#ld: -shared -T relocs.ld -e0
#objdump: -dr
#...
 +10000:	90000080 	adrp	x0, 20000 <_GLOBAL_OFFSET_TABLE_>
 +10004:	91004000 	add	x0, x0, #0x10
 +10008:	94000016 	bl	10060 <.*>
 +1000c:	d503201f 	nop
 +10010:	90000080 	adrp	x0, 20000 <_GLOBAL_OFFSET_TABLE_>
 +10014:	f9400400 	ldr	x0, \[x0, #8\]
 +10018:	d503201f 	nop
 +1001c:	d503201f 	nop
 +10020:	d53bd041 	mrs	x1, tpidr_el0
 +10024:	8b000020 	add	x0, x1, x0
 +10028:	d53bd042 	mrs	x2, tpidr_el0
 +1002c:	90000080 	adrp	x0, 20000 <_GLOBAL_OFFSET_TABLE_>
 +10030:	f9400400 	ldr	x0, \[x0, #8\]
 +10034:	8b000040 	add	x0, x2, x0
 +10038:	b9400000 	ldr	w0, \[x0\]
 +1003c:	0b000020 	add	w0, w1, w0

Disassembly of section .plt:

0000000000010040 <.plt>:
 +10040:	a9bf7bf0 	stp	x16, x30, \[sp, #-16\]!
 +10044:	90000090 	adrp	x16, 20000 <_GLOBAL_OFFSET_TABLE_>
 +10048:	f9401a11 	ldr	x17, \[x16, #48\]
 +1004c:	9100c210 	add	x16, x16, #0x30
 +10050:	d61f0220 	br	x17
 +10054:	d503201f 	nop
 +10058:	d503201f 	nop
 +1005c:	d503201f 	nop
 +10060:	90000090 	adrp	x16, 20000 <_GLOBAL_OFFSET_TABLE_>
 +10064:	f9401e11 	ldr	x17, \[x16, #56\]
 +10068:	9100e210 	add	x16, x16, #0x38
 +1006c:	d61f0220 	br	x17
