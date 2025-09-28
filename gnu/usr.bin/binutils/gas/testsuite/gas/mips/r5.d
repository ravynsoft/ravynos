#objdump: -dr --prefix-addresses --show-raw-insn
#name: Test MIPS32r5 instructions

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 42000058 	eretnc
	...
