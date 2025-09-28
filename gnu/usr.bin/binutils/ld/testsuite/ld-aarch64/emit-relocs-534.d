#source: emit-relocs-534.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	798009d6 	ldrsh	x22, \[x14, #4\]
			10000: R_AARCH64_TLSLD_LDST16_DTPREL_LO12_NC	v2
   10004:	79871a28 	ldrsh	x8, \[x17, #908\]
			10004: R_AARCH64_TLSLD_LDST16_DTPREL_LO12_NC	v3
