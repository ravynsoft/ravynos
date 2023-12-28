#source: emit-relocs-553.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0 --emit-relocs
#notarget: *-*-nto*
#objdump: -dr
#...
00010000 <.text>:
   10000:	3980309d 	ldrsb	x29, \[x4, #12\]
			10000: R_AARCH64_P32_TLSLE_LDST8_TPREL_LO12_NC	v2
   10004:	398040f2 	ldrsb	x18, \[x7, #16\]
			10004: R_AARCH64_P32_TLSLE_LDST8_TPREL_LO12_NC	v3
