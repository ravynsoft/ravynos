#objdump: -dr --prefix-addresses --show-raw-insn
#source: r5.s
#name: Test MIPS32r5 instructions

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 0001f37c 	eretnc
	\.\.\.
