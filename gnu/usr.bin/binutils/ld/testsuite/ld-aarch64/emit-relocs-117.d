#source: emit-relocs-557.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0 --emit-relocs
#notarget: *-*-nto*
#objdump: -dr
#...
00010000 <.text>:
   10000:	b9800dd6 	ldrsw	x22, \[x14, #12\]
			10000: R_AARCH64_P32_TLSLE_LDST32_TPREL_LO12_NC	v2
   10004:	b9800e28 	ldrsw	x8, \[x17, #12\]
			10004: R_AARCH64_P32_TLSLE_LDST32_TPREL_LO12_NC	v3
