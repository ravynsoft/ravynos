#source: emit-relocs-264.s
#ld: -T relocs.ld --defsym tempy=0x11000 --defsym tempy2=0x45000 --defsym tempy3=0x1234  -e0 --emit-relocs
#objdump: -dr -Mno-aliases
#...
 +10000:	8a000000 	and	x0, x0, x0
 +10004:	92400000 	and	x0, x0, #0x1
 +10008:	d2820004 	movz	x4, #0x1000
	+10008: R_AARCH64_MOVW_UABS_G0_NC	tempy
 +1000c:	d28a0007 	movz	x7, #0x5000
	+1000c: R_AARCH64_MOVW_UABS_G0_NC	tempy2
 +10010:	d2824691 	movz	x17, #0x1234
	+10010: R_AARCH64_MOVW_UABS_G0_NC	tempy3

