#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS CP3 branch instructions
#as: -32
#source: cp3b.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 4d000001 	bc3f	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 02108026 	xor	s0,s0,s0
[0-9a-f]+ <[^>]*> 4d010001 	bc3t	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 02108026 	xor	s0,s0,s0
	\.\.\.
