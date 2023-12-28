#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS add
#source: add.s
#as: -32

# Test the add macro (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 1084 0000 	addi	a0,a0,0
[0-9a-f]+ <[^>]*> 1084 0001 	addi	a0,a0,1
[0-9a-f]+ <[^>]*> 5020 8000 	li	at,0x8000
[0-9a-f]+ <[^>]*> 0024 2110 	add	a0,a0,at
[0-9a-f]+ <[^>]*> 1084 8000 	addi	a0,a0,-32768
[0-9a-f]+ <[^>]*> 41a1 0001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 0024 2110 	add	a0,a0,at
[0-9a-f]+ <[^>]*> 41a1 0001 	lui	at,0x1
[0-9a-f]+ <[^>]*> 5021 a5a5 	ori	at,at,0xa5a5
[0-9a-f]+ <[^>]*> 0024 2110 	add	a0,a0,at
[0-9a-f]+ <[^>]*> 3084 0001 	addiu	a0,a0,1
	\.\.\.
