#source: emit-relocs-293.s
#ld: -Ttext 0x1000000000000 --defsym tempy=0x2000000000000000 --defsym tempy2=0x8 -e0 --emit-relocs
#objdump: -dr

#...
 +1000000000000:	d2e3ffe4 	mov	x4, #0x1fff000000000000    	// #2305561534236983296
			1000000000000: R_AARCH64_MOVW_PREL_G3	tempy
 +1000000000004:	92e00007 	movn	x7, #0x0, lsl #48
			1000000000004: R_AARCH64_MOVW_PREL_G3	tempy2
