#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative relocation 4
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> fe48      	dla	v0,00001020 <bar>
[0-9a-f]+ <[^>]*> fc44      	ld	v0,00001020 <bar>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
	\.\.\.
