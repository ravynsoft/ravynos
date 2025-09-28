#source: emit-relocs-270.s
#ld: -T relocs.ld --defsym tempy=0x10012 --defsym tempy2=0x45000 --defsym tempy3=-292  -e0 --emit-relocs
#error: .*truncated.*tempy[12].*
#objdump: -dr

#...
 +10000:	8a000000 	and	x0, x0, x0
 +10004:	92400000 	and	x0, x0, #0x1
 +10008:	d2820244 	movz	x4, #0x1012
	+10008: R_AARCH64_MOVW_SABS_G0	tempy
 +1000c:	d288a007 	movz	x7, #0x4500
	+1000c: R_AARCH64_MOVW_SABS_G0	tempy2
 +10010:	92802471 	movn	x17, #0x123
	+10010: R_AARCH64_MOVW_SABS_G0	tempy3

