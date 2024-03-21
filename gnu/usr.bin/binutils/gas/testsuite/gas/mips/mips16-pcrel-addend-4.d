#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative relocation with addend 4
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f242 fe54 	dla	v0,00002254 <bar\+0x1234>
[0-9a-f]+ <[^>]*> f242 fc54 	ld	v0,00002254 <bar\+0x1234>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
	\.\.\.
