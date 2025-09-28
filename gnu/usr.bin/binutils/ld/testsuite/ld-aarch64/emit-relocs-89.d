#source: emit-relocs-89.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -e0 --emit-relocs
#notarget: *-*-nto*
#objdump: -dr
#...
.* <.text>:
  .*:	f2800015 	movk	x21, #0x0
			.*: R_AARCH64_P32_TLSLD_MOVW_DTPREL_G0_NC	v1
