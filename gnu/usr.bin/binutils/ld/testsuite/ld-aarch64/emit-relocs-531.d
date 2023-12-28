#source: emit-relocs-531.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	39801115 	ldrsb	x21, \[x8, #4\]
			10000: R_AARCH64_TLSLD_LDST8_DTPREL_LO12	v2
