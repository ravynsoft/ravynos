#source: emit-relocs-555.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0 --emit-relocs
#notarget: *-*-nto*
#objdump: -dr
#...
00010000 <.text>:
   10000:	798019d6 	ldrsh	x22, \[x14, #12\]
			10000: R_AARCH64_P32_TLSLE_LDST16_TPREL_LO12_NC	v2
   10004:	79872a28 	ldrsh	x8, \[x17, #916\]
			10004: R_AARCH64_P32_TLSLE_LDST16_TPREL_LO12_NC	v3
