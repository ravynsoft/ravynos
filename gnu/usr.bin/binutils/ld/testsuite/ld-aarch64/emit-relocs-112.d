#source: emit-relocs-552.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0 --emit-relocs
#notarget: *-*-nto*
#objdump: -dr
#...
00010000 <.text>:
   10000:	39803115 	ldrsb	x21, \[x8, #12\]
			10000: R_AARCH64_P32_TLSLE_LDST8_TPREL_LO12	v2
