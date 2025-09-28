#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS CP0 branch instructions
#as: -32
#source: cp0b.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 41000001 	bc0f	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 02108026 	xor	s0,s0,s0
[0-9a-f]+ <[^>]*> 41010001 	bc0t	[0-9a-f]+ <[^>]*>
[0-9a-f]+ <[^>]*> 02108026 	xor	s0,s0,s0
	\.\.\.
