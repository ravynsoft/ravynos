#objdump: -dr --prefix-addresses --show-raw-insn -M no-aliases
#name: MIPS NAL instruction/idiom 2
#as: -32
#source: nal.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 04100000 	nal
[0-9a-f]+ <[^>]*> 00000000 	sll	zero,zero,0x0
[0-9a-f]+ <[^>]*> 03e00009 	jalr	zero,ra
[0-9a-f]+ <[^>]*> 00000000 	sll	zero,zero,0x0
	\.\.\.
