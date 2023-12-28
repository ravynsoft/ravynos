#source: tls-relax-gdesc-ie-2.s
#ld: -T relocs.ld -e0 tmpdir/tls-sharedlib.so
#objdump: -dr
#...
 +10000:	90000080 	adrp	x0, 20000 <_GLOBAL_OFFSET_TABLE_>
 +10004:	d503201f 	nop
 +10008:	f9400400 	ldr	x0, \[x0, #8\]
 +1000c:	d503201f 	nop
 +10010:	d503201f 	nop
 +10014:	d503201f 	nop
 +10018:	d503201f 	nop
 +1001c:	d503201f 	nop
 +10020:	d503201f 	nop
 +10024:	d503201f 	nop
 +10028:	d503201f 	nop
 +1002c:	d53bd041 	mrs	x1, tpidr_el0
 +10030:	8b000020 	add	x0, x1, x0
 +10034:	b9400000 	ldr	w0, \[x0\]
