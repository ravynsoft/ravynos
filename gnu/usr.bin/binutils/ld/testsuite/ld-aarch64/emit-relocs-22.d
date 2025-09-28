#source: emit-relocs-22.s
#as: -mabi=ilp32
#ld: -m [aarch64_choose_ilp32_emul] -T relocs-ilp32.ld --defsym foo1=0x12345 --defsym foo2=0x1234 -e0 --emit-relocs
#notarget: *-*-nto*
#objdump: -dr

.*: +file format .*


Disassembly of section \.text:

.* <\.text>:
 +10000:	528468ad 	mov	w13, #0x2345                	// #9029
	+10000: R_AARCH64_P32_MOVW_PREL_G0	foo1
 +10004:	129db9f1 	mov	w17, #0xffff1230            	// #-60880
	+10004: R_AARCH64_P32_MOVW_PREL_G0	foo2
