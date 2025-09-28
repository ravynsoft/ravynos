#source: emit-relocs-23.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld --defsym foo=0x12345678 -e0 --emit-relocs
#notarget: *-*-nto*
#objdump: -dr

.*: +file format .*


Disassembly of section \.text:

.* <\.text>:
 +10000:	728acf0d 	movk	w13, #0x5678
	+10000: R_AARCH64_P32_MOVW_PREL_G0_NC	foo
 +10004:	52a24671 	mov	w17, #0x12330000            	// #305332224
	+10004: R_AARCH64_P32_MOVW_PREL_G1	foo
