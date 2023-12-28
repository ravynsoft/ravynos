#source: emit-relocs-288.s
#ld: -T relocs.ld --defsym tempy=0x1ffff --defsym tempy2=0x20000 --defsym tempy3=0x0 -e0 --emit-relocs
#objdump: -dr

#...
 +10000:	f29fffe4 	movk	x4, #0xffff
			10000: R_AARCH64_MOVW_PREL_G0_NC	tempy
 +10004:	f29fff87 	movk	x7, #0xfffc
			10004: R_AARCH64_MOVW_PREL_G0_NC	tempy2
 +10008:	f29fff11 	movk	x17, #0xfff8
			10008: R_AARCH64_MOVW_PREL_G0_NC	tempy3
