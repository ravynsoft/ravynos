#source: emit-relocs-269.s
#ld: -T relocs.ld --defsym tempy=0x6300100100100100 --defsym tempy2=0xf00df00df00df00d --defsym tempy3=0x1234567812345  -e0 --emit-relocs
#objdump: -dr -Mno-aliases
#...
 +10000:	8a000000 	and	x0, x0, x0
 +10004:	92400000 	and	x0, x0, #0x1
 +10008:	d2ec6004 	movz	x4, #0x6300, lsl #48
	+10008: R_AARCH64_MOVW_UABS_G3	tempy
 +1000c:	d2fe01a7 	movz	x7, #0xf00d, lsl #48
	+1000c: R_AARCH64_MOVW_UABS_G3	tempy2
 +10010:	d2e00031 	movz	x17, #0x1, lsl #48
	+10010: R_AARCH64_MOVW_UABS_G3	tempy3

