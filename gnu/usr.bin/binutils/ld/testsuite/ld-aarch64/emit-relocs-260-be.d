#source: emit-relocs-260.s
#ld: -T relocs.ld --defsym tempy=0x11012 --defsym tempy2=0x45034 --defsym tempy3=0x1234 --defsym _GOT_=0x10000 -e0 --emit-relocs
#notarget: aarch64-*-*
#objdump: -dr
#...
	+10000: R_AARCH64_PREL32	_GOT_
	+10004: R_AARCH64_PREL64	_GOT_\+0x12
 +10008:	0000000e 	\.word	0x0000000e
 +1000c:	fff404f2 	\.word	0xfff404f2
	+1000c: R_AARCH64_PREL16	_GOT_
	+1000e: R_AARCH64_PREL16	_GOT_\+0x500
 +10010:	8a000000 	and	x0, x0, x0
 +10014:	92400000 	and	x0, x0, #0x1

