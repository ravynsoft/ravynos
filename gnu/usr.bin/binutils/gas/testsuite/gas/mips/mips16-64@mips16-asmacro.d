#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS16 ASMACRO instruction
#as: -32 -I$srcdir/$subdir

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> f000      	extend	0x0
[0-9a-f]+ <[^>]*> e000      	daddu	s0,s0
[0-9a-f]+ <[^>]*> f0a4      	extend	0xa4
[0-9a-f]+ <[^>]*> e341      	addu	s0,v1,v0
[0-9a-f]+ <[^>]*> f0e0      	extend	0xe0
[0-9a-f]+ <[^>]*> e71f      	subu	a3,s0
[0-9a-f]+ <[^>]*> f501      	extend	0x501
[0-9a-f]+ <[^>]*> e264      	daddu	s1,v0,v1
[0-9a-f]+ <[^>]*> f71f      	extend	0x71f
[0-9a-f]+ <[^>]*> e0e0      	daddu	s0,a3
[0-9a-f]+ <[^>]*> f7ff      	extend	0x7ff
[0-9a-f]+ <[^>]*> e7ff      	subu	a3,a3
	\.\.\.
