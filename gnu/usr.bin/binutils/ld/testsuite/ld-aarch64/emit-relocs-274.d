#source: emit-relocs-274.s
#ld: -T relocs.ld --defsym tempy=0x11000 --defsym tempy2=0x45000 --defsym tempy3=0x1234 -e0 --emit-relocs
#objdump: -dr
#...
 +10000:	8a000000 	and	x0, x0, x0
 +10004:	92400000 	and	x0, x0, #0x1
 +10008:	10007fc2 	adr	x2, .*
	+10008: R_AARCH64_ADR_PREL_LO21	tempy
 +1000c:	101a7fa7 	adr	x7, 45000 .*
	+1000c: R_AARCH64_ADR_PREL_LO21	tempy2
 +10010:	10f89131 	adr	x17, .*
	+10010: R_AARCH64_ADR_PREL_LO21	tempy3
