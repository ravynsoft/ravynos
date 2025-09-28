#source: emit-relocs-289.s
#ld: -Ttext 0x100000000 --defsym tempy=0x1ffffffff --defsym tempy2=0x4 -e0 --emit-relocs
#objdump: -dr

#...
 +100000000:	d2bfffe4 	mov	x4, #0xffff0000            	// #4294901760
			100000000: R_AARCH64_MOVW_PREL_G1	tempy
 +100000004:	92bffff1 	mov	x17, #0xffffffff0000ffff    	// #-4294901761
			100000004: R_AARCH64_MOVW_PREL_G1	tempy2

