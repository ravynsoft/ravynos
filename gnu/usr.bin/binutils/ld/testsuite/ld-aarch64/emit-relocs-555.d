#source: emit-relocs-555.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	798029d6 	ldrsh	x22, \[x14, #20\]
			10000: R_AARCH64_TLSLE_LDST16_TPREL_LO12_NC	v2
   10004:	79873a28 	ldrsh	x8, \[x17, #924\]
			10004: R_AARCH64_TLSLE_LDST16_TPREL_LO12_NC	v3
