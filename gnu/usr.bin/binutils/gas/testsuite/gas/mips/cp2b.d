#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS CP2 branch instructions
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 49000001 	bc2f	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 02108026 	xor	s0,s0,s0
[0-9a-f]+ <[^>]*> 49010001 	bc2t	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 02108026 	xor	s0,s0,s0
	\.\.\.
