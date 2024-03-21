#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 absolute relocation 1
#as: -32 -mips3

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> fd06      	daddiu	s0,6
[0-9a-f]+ <[^>]*> f222 ff94 	daddiu	a0,sp,4660
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
