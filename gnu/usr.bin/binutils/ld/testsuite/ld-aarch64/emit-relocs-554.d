#source: emit-relocs-554.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	798028eb 	ldrsh	x11, \[x7, #20\]
			10000: R_AARCH64_TLSLE_LDST16_TPREL_LO12	v2
