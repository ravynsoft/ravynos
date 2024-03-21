#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS ISA override code generation
#as: -32
#source: r5900@isa-override-1.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 8c820000 	lw	v0,0\(a0\)
[0-9a-f]+ <[^>]*> 8c830004 	lw	v1,4\(a0\)
[0-9a-f]+ <[^>]*> 3c0189ab 	lui	at,0x89ab
[0-9a-f]+ <[^>]*> 00411025 	or	v0,v0,at
[0-9a-f]+ <[^>]*> dc820000 	.word	0xdc820000
[0-9a-f]+ <[^>]*> 340189ab 	li	at,0x89ab
[0-9a-f]+ <[^>]*> 00010c38 	.word	0x10c38
[0-9a-f]+ <[^>]*> 00411025 	or	v0,v0,at
[0-9a-f]+ <[^>]*> 3c029000 	lui	v0,0x9000
[0-9a-f]+ <[^>]*> 00021438 	.word	0x21438
[0-9a-f]+ <[^>]*> 34428000 	ori	v0,v0,0x8000
[0-9a-f]+ <[^>]*> 00021438 	.word	0x21438
[0-9a-f]+ <[^>]*> 8c820000 	lw	v0,0\(a0\)
[0-9a-f]+ <[^>]*> 8c830004 	lw	v1,4\(a0\)
[0-9a-f]+ <[^>]*> 3c0189ab 	lui	at,0x89ab
[0-9a-f]+ <[^>]*> 00411025 	or	v0,v0,at
[0-9a-f]+ <[^>]*> 8c820000 	lw	v0,0\(a0\)
[0-9a-f]+ <[^>]*> 8c830004 	lw	v1,4\(a0\)
[0-9a-f]+ <[^>]*> 3c0189ab 	lui	at,0x89ab
[0-9a-f]+ <[^>]*> 00411025 	or	v0,v0,at
	\.\.\.
