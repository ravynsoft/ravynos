#source: emit-relocs-558.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0 --emit-relocs
#notarget: *-*-nto*
#objdump: -dr
#...
00010000 <.text>:
   10000:	f9400920 	ldr	x0, \[x9, #16\]
			10000: R_AARCH64_P32_TLSLE_LDST64_TPREL_LO12	v2
