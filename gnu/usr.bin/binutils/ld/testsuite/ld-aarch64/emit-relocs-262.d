#source: emit-relocs-262.s
#ld: -T relocs.ld --defsym tempy=0x11012 --defsym tempy2=0x45034 --defsym tempy3=0x1234 --defsym _GOT_=0x20000 -e0 --emit-relocs
#error: .*truncated.*
#objdump: -dr
#...
 +10000:	00011012 	\.word	0x00011012
	+10000: R_AARCH64_PREL32	tempy
 +10004:	00045034 	\.word	0x00045034
	+10004: R_AARCH64_PREL64	tempy2
 +10008:	00000000 	\.word	0x00000000
 +1000c:	123c1234 	\.word	0x123c1234
	+1000c: R_AARCH64_PREL16	tempy3
	+1000e: R_AARCH64_PREL16	tempy3\+0x8
 +10010:	8a000000 	and	x0, x0, x0
 +10014:	92400000 	and	x0, x0, #0x1

