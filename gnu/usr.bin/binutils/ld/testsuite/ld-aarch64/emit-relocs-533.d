#source: emit-relocs-533.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	798008eb 	ldrsh	x11, \[x7, #4\]
			10000: R_AARCH64_TLSLD_LDST16_DTPREL_LO12	v2
