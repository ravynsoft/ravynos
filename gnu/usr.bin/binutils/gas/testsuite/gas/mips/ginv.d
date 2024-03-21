#objdump: -pdr --prefix-addresses --show-raw-insn
#name: MIPS GINV
#as: -mginv -32

# Test GINV instructions.

.*: +file format .*mips.*
#...
ASEs:
#...
	GINV ASE
#...

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 7c40003d 	ginvi	v0
[0-9a-f]+ <[^>]*> 7c6000bd 	ginvt	v1,0x0
[0-9a-f]+ <[^>]*> 7c8001bd 	ginvt	a0,0x1
	\.\.\.
