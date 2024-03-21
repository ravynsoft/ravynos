#source: emit-relocs-271.s
#ld: -T relocs.ld --defsym tempy=0xffffffff --defsym tempy2=0x674500 --defsym tempy3=-0x100000000  -e0 --emit-relocs
#objdump: -dr

#...
 +10000:	8a000000 	and	x0, x0, x0
 +10004:	92400000 	and	x0, x0, #0x1
 +10008:	d2bfffe4 	mov	x4, #0xffff0000            	// #4294901760
			10008: R_AARCH64_MOVW_SABS_G1	tempy
 +1000c:	d2a00ce7 	mov	x7, #0x670000              	// #6750208
			1000c: R_AARCH64_MOVW_SABS_G1	tempy2
 +10010:	92bffff1 	mov	x17, #0xffffffff0000ffff    	// #-4294901761
			10010: R_AARCH64_MOVW_SABS_G1	tempy3
