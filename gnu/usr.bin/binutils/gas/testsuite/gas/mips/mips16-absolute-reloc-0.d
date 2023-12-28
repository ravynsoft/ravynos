#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 absolute relocation 0
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 6806      	li	s0,6
[0-9a-f]+ <[^>]*> f222 0414 	addiu	a0,sp,4660
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
