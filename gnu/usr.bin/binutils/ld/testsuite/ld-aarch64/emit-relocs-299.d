#source: emit-relocs-299.s
#ld: -T relocs.ld --defsym tempy=0x11030 --defsym tempy2=0x45fa0 --defsym tempy3=0x1230  -e0 --emit-relocs
#objdump: -dr
#...
 +10000:	8a000000 	and	x0, x0, x0
 +10004:	92400000 	and	x0, x0, #0x1
 +10008:	3dc00c64 	ldr	q4, \[x3,.*
	+10008: R_AARCH64_LDST128_ABS_LO12_NC	tempy
 +1000c:	3dc3e867 	ldr	q7, \[x3,.*
	+1000c: R_AARCH64_LDST128_ABS_LO12_NC	tempy2
 +10010:	3dc08c71 	ldr	q17, \[x3,.*
	+10010: R_AARCH64_LDST128_ABS_LO12_NC	tempy3
