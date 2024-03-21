#source: emit-relocs-532.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#...
0000000000010000 <.text>:
   10000:	3980109d 	ldrsb	x29, \[x4, #4\]
			10000: R_AARCH64_TLSLD_LDST8_DTPREL_LO12_NC	v2
   10004:	398020f2 	ldrsb	x18, \[x7, #8\]
			10004: R_AARCH64_TLSLD_LDST8_DTPREL_LO12_NC	v3
