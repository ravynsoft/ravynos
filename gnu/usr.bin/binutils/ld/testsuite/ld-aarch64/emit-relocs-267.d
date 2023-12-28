#source: emit-relocs-267.s
#ld: -T relocs.ld --defsym tempy=0x63001000 --defsym tempy2=0x4500000000 --defsym tempy3=0x1234567812345  -e0 --emit-relocs
#error: .*truncated.*tempy3.*
#objdump: -dr
#...
 +10000:	8a000000 	and	x0, x0, x0
 +10004:	92400000 	and	x0, x0, #0x1
 +10008:	d2c00004 	movz	x4, #0x0, lsl #32
	+10008: R_AARCH64_MOVW_UABS_G2	tempy
 +1000c:	d2c008a7 	movz	x7, #0x45, lsl #32
	+1000c: R_AARCH64_MOVW_UABS_G2	tempy2
 +10010:	d2c468b1 	movz	x17, #0x2345, lsl #32
	+10010: R_AARCH64_MOVW_UABS_G2	tempy3

