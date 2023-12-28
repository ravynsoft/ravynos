#source: emit-relocs-559.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld -e0 --emit-relocs
#notarget: *-*-nto*
#objdump: -dr
#...
00010000 <.text>:
   10000:	f9400882 	ldr	x2, \[x4, #16\]
			10000: R_AARCH64_P32_TLSLE_LDST64_TPREL_LO12_NC	v2
   10004:	f9400a2e 	ldr	x14, \[x17, #16\]
			10004: R_AARCH64_P32_TLSLE_LDST64_TPREL_LO12_NC	v3
