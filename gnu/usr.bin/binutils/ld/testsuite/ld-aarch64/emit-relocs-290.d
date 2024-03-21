#source: emit-relocs-290.s
#ld: -Ttext=0x100000000 --defsym tempy=0x200000000 --defsym tempy2=0x1ffff0000 --defsym tempy3=0x8 -e0 --emit-relocs
#objdump: -dr

#...
 +100000000:	f2a00004 	movk	x4, #0x0, lsl #16
			100000000: R_AARCH64_MOVW_PREL_G1_NC	tempy
 +100000004:	f2bfffc7 	movk	x7, #0xfffe, lsl #16
			100000004: R_AARCH64_MOVW_PREL_G1_NC	tempy2
 +100000008:	f2a00011 	movk	x17, #0x0, lsl #16
			100000008: R_AARCH64_MOVW_PREL_G1_NC	tempy3
