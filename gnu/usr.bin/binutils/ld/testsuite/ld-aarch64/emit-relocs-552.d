#source: emit-relocs-552.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	39805115 	ldrsb	x21, \[x8, #20\]
			10000: R_AARCH64_TLSLE_LDST8_TPREL_LO12	v2
