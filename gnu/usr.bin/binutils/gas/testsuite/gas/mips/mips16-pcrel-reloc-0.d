#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative relocation 0
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 0a08      	la	v0,00001020 <bar>
[0-9a-f]+ <[^>]*> b208      	lw	v0,00001020 <bar>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
	\.\.\.
