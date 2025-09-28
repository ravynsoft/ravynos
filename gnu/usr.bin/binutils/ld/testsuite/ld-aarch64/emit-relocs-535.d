#source: emit-relocs-535.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	b9800661 	ldrsw	x1, \[x19, #4\]
			10000: R_AARCH64_TLSLD_LDST32_DTPREL_LO12	v2
