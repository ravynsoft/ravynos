#source: emit-relocs-558.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	f9400d20 	ldr	x0, \[x9, #24\]
			10000: R_AARCH64_TLSLE_LDST64_TPREL_LO12	v2
