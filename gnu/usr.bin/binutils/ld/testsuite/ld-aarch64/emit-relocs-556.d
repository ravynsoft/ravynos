#source: emit-relocs-556.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	b9801661 	ldrsw	x1, \[x19, #20\]
			10000: R_AARCH64_TLSLE_LDST32_TPREL_LO12	v2
