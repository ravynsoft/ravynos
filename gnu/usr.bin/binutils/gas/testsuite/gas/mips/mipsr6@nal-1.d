#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS NAL instruction/idiom 1
#as: -32
#source: nal.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 04100000 	nal
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 03e00009 	jr	ra
[0-9a-f]+ <[^>]*> 00000000 	nop
	\.\.\.
