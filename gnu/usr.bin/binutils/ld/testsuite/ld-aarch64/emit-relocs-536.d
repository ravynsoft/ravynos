#source: emit-relocs-536.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	b98005d6 	ldrsw	x22, \[x14, #4\]
			10000: R_AARCH64_TLSLD_LDST32_DTPREL_LO12_NC	v2
   10004:	b9800628 	ldrsw	x8, \[x17, #4\]
			10004: R_AARCH64_TLSLD_LDST32_DTPREL_LO12_NC	v3
