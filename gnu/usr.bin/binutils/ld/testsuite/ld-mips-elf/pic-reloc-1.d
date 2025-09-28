#name: MIPS PIC relocation 1
#ld: -shared -T pic-reloc-absolute-hi.ld
#objdump: -d --prefix-addresses --show-raw-insn
#target: [check_shared_lib_support]
#source: pic-reloc-lui.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 3c021234 	lui	v0,0x1234
[0-9a-f]+ <[^>]*> 24425678 	addiu	v0,v0,22136
[0-9a-f]+ <[^>]*> 3c021234 	lui	v0,0x1234
[0-9a-f]+ <[^>]*> 24425678 	addiu	v0,v0,22136
	\.\.\.
