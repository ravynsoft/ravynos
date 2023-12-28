#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative relocation with addend 5
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
	\.\.\.
[0-9a-f]+ <[^>]*> f202 fe54 	dla	v0,00002234 <foo\+0x1214>
[0-9a-f]+ <[^>]*> f202 fc54 	ld	v0,00002234 <foo\+0x1214>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
