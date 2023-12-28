#source: emit-relocs-559.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	f9400c82 	ldr	x2, \[x4, #24\]
			10000: R_AARCH64_TLSLE_LDST64_TPREL_LO12_NC	v2
   10004:	f9400e2e 	ldr	x14, \[x17, #24\]
			10004: R_AARCH64_TLSLE_LDST64_TPREL_LO12_NC	v3
