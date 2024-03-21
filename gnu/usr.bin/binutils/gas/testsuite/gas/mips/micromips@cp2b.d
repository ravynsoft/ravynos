#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS CP2 branch instructions
#as: -32
#source: cp2b.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 4280 fffe 	bc2f	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 4440      	xor	s0,s0,s0
[0-9a-f]+ <[^>]*> 42a0 fffe 	bc2t	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 4440      	xor	s0,s0,s0
[0-9a-f]+ <[^>]*> 0000 0000 	nop
	\.\.\.
