#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS CP2 branch likely instructions
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 49020001 	bc2fl	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 02108026 	xor	s0,s0,s0
[0-9a-f]+ <[^>]*> 49030001 	bc2tl	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 02108026 	xor	s0,s0,s0
	\.\.\.
