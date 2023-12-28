#source: emit-relocs-272.s
#ld: -T relocs.ld --defsym tempy=0xffffffffffff --defsym tempy2=-12345678912345 --defsym tempy3=-0x1000000000000  -e0 --emit-relocs
#objdump: -dr

#...
 +10000:	8a000000 	and	x0, x0, x0
 +10004:	92400000 	and	x0, x0, #0x1
 +10008:	d2dfffe4 	mov	x4, #0xffff00000000        	// #281470681743360
			10008: R_AARCH64_MOVW_SABS_G2	tempy
 +1000c:	92c16747 	mov	x7, #0xfffff4c5ffffffff    	// #-12343736008705
			1000c: R_AARCH64_MOVW_SABS_G2	tempy2
 +10010:	92dffff1 	mov	x17, #0xffff0000ffffffff    	// #-281470681743361
			10010: R_AARCH64_MOVW_SABS_G2	tempy3

