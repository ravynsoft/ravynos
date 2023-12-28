#source: emit-relocs-554.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0 --emit-relocs
#notarget: *-*-nto*
#objdump: -dr
#...
00010000 <.text>:
   10000:	798018eb 	ldrsh	x11, \[x7, #12\]
			10000: R_AARCH64_P32_TLSLE_LDST16_TPREL_LO12	v2
