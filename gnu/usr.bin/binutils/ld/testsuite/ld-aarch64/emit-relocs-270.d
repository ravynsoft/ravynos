#source: emit-relocs-270.s
#ld: -T relocs.ld --defsym tempy=0xffff --defsym tempy2=0x4500 --defsym tempy3=-0x10000  -e0 --emit-relocs
#objdump: -dr

#...
 +10000:	8a000000 	and	x0, x0, x0
 +10004:	92400000 	and	x0, x0, #0x1
 +10008:	d29fffe4 	mov	x4, #0xffff                	// #65535
			10008: R_AARCH64_MOVW_SABS_G0	tempy
 +1000c:	d288a007 	mov	x7, #0x4500                	// #17664
			1000c: R_AARCH64_MOVW_SABS_G0	tempy2
 +10010:	929ffff1 	mov	x17, #0xffffffffffff0000    	// #-65536
			10010: R_AARCH64_MOVW_SABS_G0	tempy3
