#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS DADD immediate expansion for R6
#as: -32
#source: daddi.s

# Check MIPS DADD macro expansion with an immediate operand.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 34018000 	li	at,0x8000
[0-9a-f]+ <[^>]*> 01c1782c 	dadd	t7,t6,at
[0-9a-f]+ <[^>]*> 3c01ffff 	lui	at,0xffff
[0-9a-f]+ <[^>]*> 34217fff 	ori	at,at,0x7fff
[0-9a-f]+ <[^>]*> 0201882c 	dadd	s1,s0,at
[0-9a-f]+ <[^>]*> 34018200 	li	at,0x8200
[0-9a-f]+ <[^>]*> 0241982c 	dadd	s3,s2,at
[0-9a-f]+ <[^>]*> 3c01ffff 	lui	at,0xffff
[0-9a-f]+ <[^>]*> 34217dff 	ori	at,at,0x7dff
[0-9a-f]+ <[^>]*> 0281a82c 	dadd	s5,s4,at
	\.\.\.
