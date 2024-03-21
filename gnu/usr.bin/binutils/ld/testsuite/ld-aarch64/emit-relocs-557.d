#source: emit-relocs-557.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	b98015d6 	ldrsw	x22, \[x14, #20\]
			10000: R_AARCH64_TLSLE_LDST32_TPREL_LO12_NC	v2
   10004:	b9801628 	ldrsw	x8, \[x17, #20\]
			10004: R_AARCH64_TLSLE_LDST32_TPREL_LO12_NC	v3
