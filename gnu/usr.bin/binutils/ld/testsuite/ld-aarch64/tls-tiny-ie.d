#source: tls-tiny-ie.s
#target: [check_shared_lib_support]
#ld: -shared -T relocs.ld -e0
#objdump: -dr
#...
 +10000:	d53bd042 	mrs	x2, tpidr_el0
 +10004:	58080020 	ldr	x0, 20008 <.*>
 +10008:	8b000040 	add	x0, x2, x0
 +1000c:	b9400000 	ldr	w0, \[x0\]
