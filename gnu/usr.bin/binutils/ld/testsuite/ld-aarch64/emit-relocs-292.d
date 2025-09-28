#source: emit-relocs-292.s
#ld: -Ttext 0x1000000000000 --defsym tempy=0x2000000000000 --defsym tempy2=0x1ffff00000004 --defsym tempy3=0x4 -e0 --emit-relocs
#objdump: -dr

#...
 +1000000000000:	f2c00004 	movk	x4, #0x0, lsl #32
			1000000000000: R_AARCH64_MOVW_PREL_G2_NC	tempy
 +1000000000004:	f2dfffe7 	movk	x7, #0xffff, lsl #32
			1000000000004: R_AARCH64_MOVW_PREL_G2_NC	tempy2
 +1000000000008:	f2dffff1 	movk	x17, #0xffff, lsl #32
			1000000000008: R_AARCH64_MOVW_PREL_G2_NC	tempy3
