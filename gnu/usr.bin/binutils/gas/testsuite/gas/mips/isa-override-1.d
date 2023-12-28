#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS ISA override code generation
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 8c820000 	lw	v0,0\(a0\)
[0-9a-f]+ <[^>]*> 8c830004 	lw	v1,4\(a0\)
[0-9a-f]+ <[^>]*> 3c0189ab 	lui	at,0x89ab
[0-9a-f]+ <[^>]*> 00411025 	or	v0,v0,at
[0-9a-f]+ <[^>]*> d4820000 	ldc1	\$f2,0\(a0\)
[0-9a-f]+ <[^>]*> 3c013ff0 	lui	at,0x3ff0
[0-9a-f]+ <[^>]*> 44811800 	mtc1	at,\$f3
[0-9a-f]+ <[^>]*> 3c0189ab 	lui	at,0x89ab
[0-9a-f]+ <[^>]*> 44811000 	mtc1	at,\$f2
[0-9a-f]+ <[^>]*> dc820000 	ld	v0,0\(a0\)
[0-9a-f]+ <[^>]*> 340189ab 	li	at,0x89ab
[0-9a-f]+ <[^>]*> 00010c38 	dsll	at,at,0x10
[0-9a-f]+ <[^>]*> 00411025 	or	v0,v0,at
[0-9a-f]+ <[^>]*> 3c029000 	lui	v0,0x9000
[0-9a-f]+ <[^>]*> 00021438 	dsll	v0,v0,0x10
[0-9a-f]+ <[^>]*> 34428000 	ori	v0,v0,0x8000
[0-9a-f]+ <[^>]*> 00021438 	dsll	v0,v0,0x10
[0-9a-f]+ <[^>]*> d4820000 	ldc1	\$f2,0\(a0\)
[0-9a-f]+ <[^>]*> 3c013ff0 	lui	at,0x3ff0
[0-9a-f]+ <[^>]*> 00010c38 	dsll	at,at,0x10
[0-9a-f]+ <[^>]*> 342189ab 	ori	at,at,0x89ab
[0-9a-f]+ <[^>]*> 00010c38 	dsll	at,at,0x10
[0-9a-f]+ <[^>]*> 44a11000 	dmtc1	at,\$f2
[0-9a-f]+ <[^>]*> 8c820000 	lw	v0,0\(a0\)
[0-9a-f]+ <[^>]*> 8c830004 	lw	v1,4\(a0\)
[0-9a-f]+ <[^>]*> 3c0189ab 	lui	at,0x89ab
[0-9a-f]+ <[^>]*> 00411025 	or	v0,v0,at
[0-9a-f]+ <[^>]*> d4820000 	ldc1	\$f2,0\(a0\)
[0-9a-f]+ <[^>]*> 3c013ff0 	lui	at,0x3ff0
[0-9a-f]+ <[^>]*> 44811800 	mtc1	at,\$f3
[0-9a-f]+ <[^>]*> 3c0189ab 	lui	at,0x89ab
[0-9a-f]+ <[^>]*> 44811000 	mtc1	at,\$f2
[0-9a-f]+ <[^>]*> 8c820000 	lw	v0,0\(a0\)
[0-9a-f]+ <[^>]*> 8c830004 	lw	v1,4\(a0\)
[0-9a-f]+ <[^>]*> 3c0189ab 	lui	at,0x89ab
[0-9a-f]+ <[^>]*> 00411025 	or	v0,v0,at
[0-9a-f]+ <[^>]*> d4820000 	ldc1	\$f2,0\(a0\)
[0-9a-f]+ <[^>]*> 3c013ff0 	lui	at,0x3ff0
[0-9a-f]+ <[^>]*> 44811800 	mtc1	at,\$f3
[0-9a-f]+ <[^>]*> 3c0189ab 	lui	at,0x89ab
[0-9a-f]+ <[^>]*> 44811000 	mtc1	at,\$f2
	\.\.\.
