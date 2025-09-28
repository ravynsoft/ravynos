#source: emit-relocs-291.s
#ld: -Ttext 0x1000000000000 --defsym tempy=0x1ffffffffffff --defsym tempy2=0x1ffff00000000 -defsym tempy3=0x8 -e0 --emit-relocs
#objdump: -dr

#...
 +1000000000000:	d2dfffe4 	mov	x4, #0xffff00000000        	// #281470681743360
			1000000000000: R_AARCH64_MOVW_PREL_G2	tempy
 +1000000000004:	d2dfffc7 	mov	x7, #0xfffe00000000        	// #281466386776064
			1000000000004: R_AARCH64_MOVW_PREL_G2	tempy2
 +1000000000008:	92dffff1 	mov	x17, #0xffff0000ffffffff    	// #-281470681743361
			1000000000008: R_AARCH64_MOVW_PREL_G2	tempy3
