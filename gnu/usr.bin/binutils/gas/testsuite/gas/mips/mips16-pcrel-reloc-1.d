#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative relocation 1
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
	\.\.\.
[0-9a-f]+ <[^>]*> f7ff 0a00 	la	v0,00001000 <bar>
[0-9a-f]+ <[^>]*> f7df b21c 	lw	v0,00001000 <bar>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
