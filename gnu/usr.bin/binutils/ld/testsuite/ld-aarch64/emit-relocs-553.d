#source: emit-relocs-553.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	3980509d 	ldrsb	x29, \[x4, #20\]
			10000: R_AARCH64_TLSLE_LDST8_TPREL_LO12_NC	v2
   10004:	398060f2 	ldrsb	x18, \[x7, #24\]
			10004: R_AARCH64_TLSLE_LDST8_TPREL_LO12_NC	v3
