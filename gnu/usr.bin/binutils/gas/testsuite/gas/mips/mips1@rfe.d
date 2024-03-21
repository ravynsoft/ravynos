#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS RFE instruction
#as: -32
#source: rfe.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 42000010 	rfe
	\.\.\.
