#source: emit-relocs-556.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0 --emit-relocs
#notarget: *-*-nto*
#objdump: -dr
#...
00010000 <.text>:
   10000:	b9800e61 	ldrsw	x1, \[x19, #12\]
			10000: R_AARCH64_P32_TLSLE_LDST32_TPREL_LO12	v2
