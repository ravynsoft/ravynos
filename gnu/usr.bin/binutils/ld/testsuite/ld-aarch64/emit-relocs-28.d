#source: emit-relocs-28.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] --defsym globala=0x11000 --defsym globalb=0x45000 --defsym globalc=0x1234  -e0 --emit-relocs
#notarget: *-*-nto*
#objdump: -dr

.*: +file format .*


Disassembly of section .text:

.* <\.text>:
  .*:	.* 	adrp	x2, .* <.*>
			.*: R_AARCH64_P32_ADR_PREL_PG_HI21	_GLOBAL_OFFSET_TABLE_
  .*:	.* 	ldr	x0, \[x2, #.*\]
			.*: R_AARCH64_P32_LD32_GOTPAGE_LO14	globala
  .*:	.* 	ldr	x0, \[x2, #.*\]
			.*: R_AARCH64_P32_LD32_GOTPAGE_LO14	globalb
  .*:	.* 	ldr	x0, \[x2, #.*\]
			.*: R_AARCH64_P32_LD32_GOTPAGE_LO14	globalc
