#source: emit-relocs-257.s
#ld: -T relocs.ld --defsym tempy=0x11012 --defsym tempy2=0x45034 --defsym tempy3=0x1234  -e0 --emit-relocs
#notarget: aarch64-*-*
#objdump: -dr
#...
 +10000:	00011012 	\.word	0x00011012
	+10000: R_AARCH64_ABS32	tempy
 +10004:	00000000 	\.word	0x00000000
	+10004: R_AARCH64_ABS64	tempy2
 +10008:	00045034 	\.word	0x00045034
 +1000c:	1234123c 	\.word	0x1234123c
	+1000c: R_AARCH64_ABS16	tempy3
	+1000e: R_AARCH64_ABS16	tempy3\+0x8
 +10010:	8a000000 	and	x0, x0, x0
 +10014:	92400000 	and	x0, x0, #0x1

