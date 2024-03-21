#source: emit-relocs-287.s
#ld: -T relocs.ld --defsym tempy=0x1ffff --defsym tempy2=0x4 -e0 --emit-relocs
#objdump: -dr

#...
 +10000:	d29fffe4 	mov	x4, #0xffff                	// #65535
			10000: R_AARCH64_MOVW_PREL_G0	tempy
 +10004:	929ffff1 	mov	x17, #0xffffffffffff0000    	// #-65536
			10004: R_AARCH64_MOVW_PREL_G0	tempy2

