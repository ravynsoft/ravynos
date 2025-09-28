#source: emit-relocs-92.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -e0 --emit-relocs
#notarget: *-*-nto*
#objdump: -dr
#...
.* <.text>:
  .*:	11001134 	add	w20, w9, #0x4
			.*: R_AARCH64_P32_TLSLD_ADD_DTPREL_LO12_NC	v2
