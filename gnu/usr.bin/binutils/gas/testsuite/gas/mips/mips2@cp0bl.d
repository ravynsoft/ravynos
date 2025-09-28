#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS CP0 branch likely instructions
#as: -32
#source: cp0bl.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 41020001 	bc0fl	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 02108026 	xor	s0,s0,s0
[0-9a-f]+ <[^>]*> 41030001 	bc0tl	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 02108026 	xor	s0,s0,s0
	\.\.\.
