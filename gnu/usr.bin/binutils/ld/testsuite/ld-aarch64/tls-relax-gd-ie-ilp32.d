#source: tls-relax-gd-ie.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0 tmpdir/tls-sharedlib-ilp32.so
#notarget: *-*-nto*
#objdump: -dr
#...
 +10000:	90000080 	adrp	x0, 20000 <_GLOBAL_OFFSET_TABLE_>
 +10004:	b9400400 	ldr	w0, \[x0, #4\]
 +10008:	d53bd041 	mrs	x1, tpidr_el0
 +1000c:	0b000020 	add	w0, w1, w0
 +10010:	b9400000 	ldr	w0, \[x0\]
