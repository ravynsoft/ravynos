#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 branch relocation 1
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
[0-9a-f]+ <[^>]*> 17ef      	b	00001000 <bar>
[0-9a-f]+ <[^>]*> 60ee      	bteqz	00001000 <bar>
[0-9a-f]+ <[^>]*> 61ed      	btnez	00001000 <bar>
[0-9a-f]+ <[^>]*> 22ec      	beqz	v0,00001000 <bar>
[0-9a-f]+ <[^>]*> 2aeb      	bnez	v0,00001000 <bar>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
