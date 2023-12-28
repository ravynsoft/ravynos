#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS ISA override code generation
#as: -32
#source: isa-override-1.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> fc44 0000 	lw	v0,0\(a0\)
[0-9a-f]+ <[^>]*> fc64 0004 	lw	v1,4\(a0\)
[0-9a-f]+ <[^>]*> 41a1 89ab 	lui	at,0x89ab
[0-9a-f]+ <[^>]*> 0022 1290 	or	v0,v0,at
[0-9a-f]+ <[^>]*> bc44 0000 	ldc1	\$f2,0\(a0\)
[0-9a-f]+ <[^>]*> 41a1 89ab 	lui	at,0x89ab
[0-9a-f]+ <[^>]*> 5422 283b 	mtc1	at,\$f2
[0-9a-f]+ <[^>]*> 41a1 3ff0 	lui	at,0x3ff0
[0-9a-f]+ <[^>]*> 5422 383b 	mthc1	at,\$f2
[0-9a-f]+ <[^>]*> dc44 0000 	ld	v0,0\(a0\)
[0-9a-f]+ <[^>]*> 5020 89ab 	li	at,0x89ab
[0-9a-f]+ <[^>]*> 5821 8000 	dsll	at,at,0x10
[0-9a-f]+ <[^>]*> 0022 1290 	or	v0,v0,at
[0-9a-f]+ <[^>]*> 41a2 9000 	lui	v0,0x9000
[0-9a-f]+ <[^>]*> 5842 8000 	dsll	v0,v0,0x10
[0-9a-f]+ <[^>]*> 5042 8000 	ori	v0,v0,0x8000
[0-9a-f]+ <[^>]*> 5842 8000 	dsll	v0,v0,0x10
[0-9a-f]+ <[^>]*> bc44 0000 	ldc1	\$f2,0\(a0\)
[0-9a-f]+ <[^>]*> 41a1 3ff0 	lui	at,0x3ff0
[0-9a-f]+ <[^>]*> 5821 8000 	dsll	at,at,0x10
[0-9a-f]+ <[^>]*> 5021 89ab 	ori	at,at,0x89ab
[0-9a-f]+ <[^>]*> 5821 8000 	dsll	at,at,0x10
[0-9a-f]+ <[^>]*> 5422 2c3b 	dmtc1	at,\$f2
[0-9a-f]+ <[^>]*> fc44 0000 	lw	v0,0\(a0\)
[0-9a-f]+ <[^>]*> fc64 0004 	lw	v1,4\(a0\)
[0-9a-f]+ <[^>]*> 41a1 89ab 	lui	at,0x89ab
[0-9a-f]+ <[^>]*> 0022 1290 	or	v0,v0,at
[0-9a-f]+ <[^>]*> bc44 0000 	ldc1	\$f2,0\(a0\)
[0-9a-f]+ <[^>]*> 41a1 89ab 	lui	at,0x89ab
[0-9a-f]+ <[^>]*> 5422 283b 	mtc1	at,\$f2
[0-9a-f]+ <[^>]*> 41a1 3ff0 	lui	at,0x3ff0
[0-9a-f]+ <[^>]*> 5422 383b 	mthc1	at,\$f2
[0-9a-f]+ <[^>]*> fc44 0000 	lw	v0,0\(a0\)
[0-9a-f]+ <[^>]*> fc64 0004 	lw	v1,4\(a0\)
[0-9a-f]+ <[^>]*> 41a1 89ab 	lui	at,0x89ab
[0-9a-f]+ <[^>]*> 0022 1290 	or	v0,v0,at
[0-9a-f]+ <[^>]*> bc44 0000 	ldc1	\$f2,0\(a0\)
[0-9a-f]+ <[^>]*> 41a1 89ab 	lui	at,0x89ab
[0-9a-f]+ <[^>]*> 5422 283b 	mtc1	at,\$f2
[0-9a-f]+ <[^>]*> 41a1 3ff0 	lui	at,0x3ff0
[0-9a-f]+ <[^>]*> 5422 383b 	mthc1	at,\$f2
	\.\.\.
