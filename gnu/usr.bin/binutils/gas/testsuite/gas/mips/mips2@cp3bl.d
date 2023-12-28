#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS CP3 branch likely instructions
#as: -32
#source: cp3bl.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 4d020001 	bc3fl	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 02108026 	xor	s0,s0,s0
[0-9a-f]+ <[^>]*> 4d030001 	bc3tl	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 02108026 	xor	s0,s0,s0
	\.\.\.
