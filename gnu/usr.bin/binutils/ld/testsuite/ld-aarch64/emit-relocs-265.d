#source: emit-relocs-265.s
#ld: -T relocs.ld --defsym tempy=0x100011000 --defsym tempy2=0x45000 --defsym tempy3=0x1234  -e0 --emit-relocs
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_MOVW_UABS_G1 against symbol `tempy.*
#objdump: -dr
#...
 +10000:	8a000000 	and	x0, x0, x0
 +10004:	92400000 	and	x0, x0, #0x1
 +10008:	d2a00024 	movz	x4, #0x1, lsl #16
	+10008: R_AARCH64_MOVW_UABS_G1	tempy
 +1000c:	d2a00087 	movz	x7, #0x4, lsl #16
	+1000c: R_AARCH64_MOVW_UABS_G1	tempy2
 +10010:	d2a00011 	movz	x17, #0x0, lsl #16
	+10010: R_AARCH64_MOVW_UABS_G1	tempy3

