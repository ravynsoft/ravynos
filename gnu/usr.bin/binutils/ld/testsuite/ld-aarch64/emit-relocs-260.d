#source: emit-relocs-260.s
#ld: -T relocs.ld --defsym tempy=0x11012 --defsym tempy2=0x45034 --defsym tempy3=0x1234 --defsym _GOT_=0x10000 -e0 --emit-relocs
#notarget: aarch64_be-*-*
#objdump: -dr
#...
 +10000:	00000000 	\.word	0x00000000
	+10000: R_AARCH64_PREL32	_GOT_
 +10004:	0000000e 	\.word	0x0000000e
	+10004: R_AARCH64_PREL64	_GOT_\+0x12
 +10008:	00000000 	\.word	0x00000000
 +1000c:	04f2fff4 	\.word	0x04f2fff4
	+1000c: R_AARCH64_PREL16	_GOT_
	+1000e: R_AARCH64_PREL16	_GOT_\+0x500
 +10010:	8a000000 	and	x0, x0, x0
 +10014:	92400000 	and	x0, x0, #0x1

